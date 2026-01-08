// Created on: 2009-06-17
// Created by: Sergey ZARITCHNY
// Copyright (c) 2009-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepCheck_Shell.hxx>
#include <BRepCheck_Wire.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <DNaming.hxx>
#include <DNaming_RevolutionDriver.hxx>
#include <Geom_Line.hxx>
#include <GProp_GProps.hxx>
#include <ModelDefinitions.hxx>
#include <Precision.hxx>
#include <Standard_Real.hxx>
#include <Standard_Type.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Real.hxx>
#include <TDF_Label.hxx>
#include <TDF_TagSource.hxx>
#include <TFunction_Function.hxx>
#include <TFunction_Logbook.hxx>
#include <TNaming.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_MapOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DNaming_RevolutionDriver, TFunction_Driver)

// OCAF
//=================================================================================================

DNaming_RevolutionDriver::DNaming_RevolutionDriver() {}

//=======================================================================
// function : Validate
// purpose  : Validates labels of a function in <theLog>.
//=======================================================================
void DNaming_RevolutionDriver::Validate(Handle(TFunction_Logbook)&) const {}

//=======================================================================
// function : MustExecute
// purpose  : Analyses in <theLog> if the loaded function must be executed
//=======================================================================
Standard_Boolean DNaming_RevolutionDriver::MustExecute(const Handle(TFunction_Logbook)&) const
{
  return Standard_True;
}

//=======================================================================
// function : Execute
// purpose  : Executes the function
//=======================================================================
Standard_Integer DNaming_RevolutionDriver::Execute(Handle(TFunction_Logbook)& theLog) const
{
  Handle(TFunction_Function) aFunction;
  Label().FindAttribute(TFunction_Function::GetID(), aFunction);
  if (aFunction.IsNull())
    return -1;

  // Save location
  Handle(ShapeAttribute) aPrevRevol = DNaming1::GetFunctionResult(aFunction);
  TopLoc_Location            aLocation;
  if (!aPrevRevol.IsNull() && !aPrevRevol->IsEmpty())
  {
    aLocation = aPrevRevol->Get().Location();
  }

  // Basis for Revol
  Handle(TDataStd_UAttribute) aBasObject = DNaming1::GetObjectArg(aFunction, REVOL_BASIS);
  Handle(ShapeAttribute)  aBasisNS   = DNaming1::GetObjectValue(aBasObject);
  if (aBasisNS.IsNull() || aBasisNS->IsEmpty())
  {
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }

  const TopoShape& aBasis = aBasisNS->Get();
  TopoShape        aBASIS;
  if (aBasis.ShapeType() == TopAbs_WIRE)
  {
    Handle(BRepCheck_Wire) aCheck = new BRepCheck_Wire(TopoDS::Wire(aBasis));
    if (aCheck->Closed(Standard_True) == BRepCheck_NoError)
    {
      FaceMaker aMaker(TopoDS::Wire(aBasis), Standard_True); // Makes planar face
      if (aMaker.IsDone())
        aBASIS = aMaker.Face(); // aMaker.Face();
    }
  }
  else if (aBasis.ShapeType() == TopAbs_FACE)
    aBASIS = aBasis;
  if (aBASIS.IsNull())
  {
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }

  Handle(ShapeAttribute) aContextOfBasis;
  Standard_Boolean           anIsAttachment = Standard_False;
  if (DNaming1::IsAttachment(aBasObject))
  {
    aContextOfBasis = DNaming1::GetAttachmentsContext(aBasObject); // a Context of Revolution basis
    if (aContextOfBasis.IsNull() || aContextOfBasis->IsEmpty())
    {
      aFunction->SetFailure(WRONG_ARGUMENT);
      return -1;
    }
    anIsAttachment = Standard_True;
  }

  // Axis
  Handle(TDataStd_UAttribute) anAxObject = DNaming1::GetObjectArg(aFunction, REVOL_AXIS);
  Handle(ShapeAttribute)  anAxNS     = DNaming1::GetObjectValue(anAxObject);
  Axis3d                      anAXIS;
  TopoShape                aTopoDSAxis;
  if (anAxNS.IsNull() || anAxNS->IsEmpty())
  {
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }
  else
  {
    aTopoDSAxis               = anAxNS->Get();
    Standard_Boolean anAxisOK = Standard_False;
    if (!aTopoDSAxis.IsNull())
    {
      if (aTopoDSAxis.ShapeType() == TopAbs_EDGE || aTopoDSAxis.ShapeType() == TopAbs_WIRE)
      {
        if (aTopoDSAxis.ShapeType() == TopAbs_WIRE)
        {
          ShapeExplorer anExplorer(aTopoDSAxis, TopAbs_EDGE);
          aTopoDSAxis = anExplorer.Current();
        }
        const TopoEdge& anEdge = TopoDS::Edge(aTopoDSAxis);
        Standard_Real      aFirst, aLast;
        Handle(GeomCurve3d) aCurve = BRepInspector::Curve(anEdge, aFirst, aLast);
        if (aCurve->IsKind(STANDARD_TYPE(GeomLine)))
        {
          Handle(GeomLine) aLine = Handle(GeomLine)::DownCast(aCurve);
          if (!aLine.IsNull())
          {
            anAXIS   = aLine->Position1();
            anAxisOK = Standard_True;
          }
        }
      }
    }
    if (!anAxisOK)
    {
#ifdef OCCT_DEBUG
      std::cout << "RevolutionDriver:: Axis is not correct" << std::endl;
#endif
      aFunction->SetFailure(WRONG_ARGUMENT);
      return -1;
    }
  }

  if (aFunction->GetDriverGUID() == FULREVOL_GUID)
  {
    BRepPrimAPI_MakeRevol aMakeRevol(aBASIS, anAXIS, Standard_True);
    aMakeRevol.Build();
    if (!aMakeRevol.IsDone())
    {
      aFunction->SetFailure(ALGO_FAILED);
      return -1;
    }
    const TopoShape aResult = aMakeRevol.Shape();
    BRepCheck_Analyzer aCheckAnalyzer(aResult);
    if (!aCheckAnalyzer.IsValid(aResult))
    {
      aFunction->SetFailure(RESULT_NOT_VALID);
      return -1;
    }
    Standard_Boolean aVol = Standard_False;
    if (aResult.ShapeType() == TopAbs_SOLID)
      aVol = Standard_True;
    else if (aResult.ShapeType() == TopAbs_SHELL)
    {
      Handle(BRepCheck_Shell) aCheck = new BRepCheck_Shell(TopoDS::Shell(aResult));
      if (aCheck->Closed() == BRepCheck_NoError)
        aVol = Standard_True;
    }
    if (aVol)
    {
      GeometricProperties aGProp;
      BRepGProp1::VolumeProperties(aResult, aGProp);
      if (aGProp.Mass() <= Precision1::Confusion())
      {
        aFunction->SetFailure(RESULT_NOT_VALID);
        return -1;
      }
    }
    // Naming1
    if (anIsAttachment)
      LoadNamingDS(RESPOSITION(aFunction), aMakeRevol, aBASIS, aContextOfBasis->Get());
    else
      LoadNamingDS(RESPOSITION(aFunction), aMakeRevol, aBASIS, aBASIS);
  }
  else if (aFunction->GetDriverGUID() == SECREVOL_GUID)
  {
    Standard_Real anANGLE = DNaming1::GetReal(aFunction, REVOL_ANGLE)->Get();
    if (anANGLE <= Precision1::Confusion())
    {
      aFunction->SetFailure(WRONG_ARGUMENT);
      return -1;
    }
    // Reverse
    Standard_Integer aRev = DNaming1::GetInteger(aFunction, REVOL_REV)->Get();
    if (aRev)
      anAXIS.Reverse();

    BRepPrimAPI_MakeRevol aMakeRevol(aBASIS, anAXIS, anANGLE, Standard_True);
    aMakeRevol.Build();
    if (!aMakeRevol.IsDone())
    {
      aFunction->SetFailure(ALGO_FAILED);
      return -1;
    }
    const TopoShape aResult = aMakeRevol.Shape();
    BRepCheck_Analyzer aCheckAnalyzer(aResult);
    if (!aCheckAnalyzer.IsValid(aResult))
    {
      aFunction->SetFailure(RESULT_NOT_VALID);
      return -1;
    }
    Standard_Boolean aVol = Standard_False;
    if (aResult.ShapeType() == TopAbs_SOLID)
      aVol = Standard_True;
    else if (aResult.ShapeType() == TopAbs_SHELL)
    {
      Handle(BRepCheck_Shell) aCheck = new BRepCheck_Shell(TopoDS::Shell(aResult));
      if (aCheck->Closed() == BRepCheck_NoError)
        aVol = Standard_True;
    }
    if (aVol)
    {
      GeometricProperties aGProp;
      BRepGProp1::VolumeProperties(aResult, aGProp);
      if (aGProp.Mass() <= Precision1::Confusion())
      {
        aFunction->SetFailure(RESULT_NOT_VALID);
        return -1;
      }
    }

    // Naming1
    if (anIsAttachment)
      LoadNamingDS(RESPOSITION(aFunction), aMakeRevol, aBASIS, aContextOfBasis->Get());
    else
      LoadNamingDS(RESPOSITION(aFunction), aMakeRevol, aBASIS, aBASIS);
  }
  else
  {
    aFunction->SetFailure(UNSUPPORTED_FUNCTION);
    return -1;
  }

  // restore location
  if (!aLocation.IsIdentity())
    TNaming1::Displace(RESPOSITION(aFunction), aLocation, Standard_True);

  theLog->SetValid(RESPOSITION(aFunction), Standard_True);
  aFunction->SetFailure(DONE);
  return 0;
}

//=======================================================================
static void LoadSeamEdge(BRepPrimAPI_MakeRevol& mkRevol,
                         const DataLabel&       ResultLabel,
                         const TopoShape&    ShapeIn)

{
  TopTools_MapOfShape View;
  ShapeExplorer     shapeExplorer(ShapeIn, TopAbs_EDGE);
  Standard_Boolean    isFound(Standard_False);
  for (; shapeExplorer.More(); shapeExplorer.Next())
  {
    const TopoShape& Root = shapeExplorer.Current();
    if (!View.Add(Root))
      continue;
    const ShapeList&        Shapes = mkRevol.Generated(Root);
    TopTools_ListIteratorOfListOfShape ShapesIterator(Shapes);
    for (; ShapesIterator.More(); ShapesIterator.Next())
    {
      TopoShape newShape = ShapesIterator.Value();
      if (newShape.ShapeType() != TopAbs_FACE)
        continue;
      if (!Root.IsSame(newShape))
      {
        ShapeExplorer exp(newShape, TopAbs_EDGE);
        for (; exp.More(); exp.Next())
        {
          if (BRepInspector::IsClosed(TopoDS::Edge(exp.Current()), TopoDS::Face(newShape)))
          {
            TNaming_Builder Builder(ResultLabel.NewChild());
            Builder.Generated(exp.Current());
            isFound = Standard_True;
            break;
          }
        }
        if (isFound)
        {
          isFound = Standard_False;
          break;
        }
      }
    }
  }
}

//=======================================================================
static Standard_Boolean HasDangle(const TopoShape& ShapeIn)
{
  if (ShapeIn.ShapeType() == TopAbs_SOLID)
    return Standard_False;
  else if (ShapeIn.ShapeType() == TopAbs_SHELL)
  {
    Handle(BRepCheck_Shell) aCheck = new BRepCheck_Shell(TopoDS::Shell(ShapeIn));
    return aCheck->Closed() != BRepCheck_NoError;
  }
  else if (ShapeIn.ShapeType() == TopAbs_FACE || ShapeIn.ShapeType() == TopAbs_WIRE
           || ShapeIn.ShapeType() == TopAbs_EDGE || ShapeIn.ShapeType() == TopAbs_VERTEX)
    return Standard_True;
  return Standard_False;
}

//=======================================================================
static void BuildAtomicMap(const TopoShape& S, TopTools_MapOfShape& M)
{
  if (S.ShapeType() == TopAbs_COMPOUND || S.ShapeType() == TopAbs_COMPSOLID)
  {
    TopoDS_Iterator it(S);
    for (; it.More(); it.Next())
    {
      if (it.Value().ShapeType() > TopAbs_COMPSOLID)
        M.Add(it.Value());
      else
        BuildAtomicMap(it.Value(), M);
    }
  }
  else
    M.Add(S);
}

//=================================================================================================

Standard_Boolean HasDangleShapes(const TopoShape& ShapeIn)
{
  if (ShapeIn.ShapeType() == TopAbs_COMPOUND || ShapeIn.ShapeType() == TopAbs_COMPSOLID)
  {
    TopTools_MapOfShape M;
    BuildAtomicMap(ShapeIn, M);
    TopTools_MapIteratorOfMapOfShape it(M);
    for (; it.More(); it.Next())
      if (HasDangle(it.Key1()))
        return Standard_True;
  }
  else
    return HasDangle(ShapeIn);
  return Standard_False;
}

//=================================================================================================

void DNaming_RevolutionDriver::LoadNamingDS(const DataLabel&       theResultLabel,
                                            BRepPrimAPI_MakeRevol& MS,
                                            const TopoShape&    Basis,
                                            const TopoShape&    Context) const
{

  TopTools_DataMapOfShapeShape SubShapes;
  for (ShapeExplorer Exp(MS.Shape(), TopAbs_FACE); Exp.More(); Exp.Next())
  {
    SubShapes.Bind(Exp.Current(), Exp.Current());
  }

  Handle(TDF_TagSource) Tagger = TDF_TagSource::Set(theResultLabel);
  if (Tagger.IsNull())
    return;
  Tagger->Set(0);

  TNaming_Builder Builder(theResultLabel);
  if (Basis.IsEqual(Context))
    Builder.Generated(MS.Shape());
  else
    Builder.Generated(Context, MS.Shape());

  // Insert lateral face : Face from Edge
  TNaming_Builder LateralFaceBuilder(theResultLabel.NewChild());
  DNaming1::LoadAndOrientGeneratedShapes(MS, Basis, TopAbs_EDGE, LateralFaceBuilder, SubShapes);

  // is full
  TopoShape     StartShape = MS.FirstShape();
  TopoShape     EndShape   = MS.LastShape();
  Standard_Boolean isFull(Standard_False);
  if (!StartShape.IsNull() && !EndShape.IsNull())
    isFull = StartShape.IsEqual(EndShape);

  Standard_Boolean hasDangle = HasDangleShapes(MS.Shape());
  Standard_Boolean isBasisClosed(Standard_True);
  TopoVertex    Vfirst, Vlast;
  if (Basis.ShapeType() == TopAbs_WIRE)
  {
    Handle(BRepCheck_Wire) aCheck = new BRepCheck_Wire(TopoDS::Wire(Basis));
    if (aCheck->Closed() != BRepCheck_NoError)
    {
      isBasisClosed = Standard_False; // open
      TopExp1::Vertices(TopoDS::Wire(Basis), Vfirst, Vlast);
    }
  }
  else if (Basis.ShapeType() == TopAbs_EDGE)
  {
    BRepBuilderAPI_MakeWire aMakeWire;
    aMakeWire.Add(TopoDS::Edge(Basis));
    if (aMakeWire.IsDone())
    {
      Handle(BRepCheck_Wire) aCheck = new BRepCheck_Wire(aMakeWire.Wire());
      if (aCheck->Closed() != BRepCheck_NoError)
      {                                 // check for circle case
        isBasisClosed = Standard_False; // open
        TopExp1::Vertices(TopoDS::Edge(Basis), Vfirst, Vlast);
      }
    }
  }
  if (isFull)
  {
    // seam edge
    LoadSeamEdge(MS, theResultLabel, Basis);

    if (hasDangle)
    {
      if (!isBasisClosed)
      {
        // dangle edges
        const ShapeList&        Shapes = MS.Generated(Vfirst);
        TopTools_ListIteratorOfListOfShape it(Shapes);
        for (; it.More(); it.Next())
        {
          if (!BRepInspector::Degenerated(TopoDS::Edge(it.Value())))
          {
            TNaming_Builder aBuilder(theResultLabel.NewChild());
            aBuilder.Generated(Vfirst, it.Value());
          }
#ifdef OCCT_DEBUG
          else
          {
            if (MS.HasDegenerated())
              std::cout << "mkRevol has degenerated" << std::endl;
            std::cout << "BRepInspector found degenerated edge (from Vfirst) TS = "
                      << it.Value().TShape().get() << std::endl;
          }
#endif
        }

        const ShapeList& Shapes2 = MS.Generated(Vlast);
        it.Initialize(Shapes2);
        for (; it.More(); it.Next())
        {
          if (!BRepInspector::Degenerated(TopoDS::Edge(it.Value())))
          {
            TNaming_Builder aBuilder(theResultLabel.NewChild());
            aBuilder.Generated(Vlast, it.Value());
          }
#ifdef OCCT_DEBUG
          else
          {
            if (MS.HasDegenerated())
              std::cout << "mkRevol has degenerated" << std::endl;
            std::cout << "BRepInspector found degenerated edge (from Vlast) TS = "
                      << it.Value().TShape().get() << std::endl;
          }
#endif
        }
      }
    }
  }
  else
  { // if(!isFull)
    // Insert start shape
    if (!StartShape.IsNull())
    {
      if (StartShape.ShapeType() != TopAbs_COMPOUND)
      {
        TNaming_Builder StartBuilder(theResultLabel.NewChild());
        if (SubShapes.IsBound(StartShape))
        {
          StartShape = SubShapes(StartShape);
        }
        StartBuilder.Generated(StartShape);
        if (StartShape.ShapeType() != TopAbs_FACE)
        {
          TopoDS_Iterator it(StartShape);
          for (; it.More(); it.Next())
          {
            TNaming_Builder aBuilder(theResultLabel.NewChild());
            aBuilder.Generated(it.Value());
          }
        }
      }
      else
      {
        TopoDS_Iterator itr(StartShape);
        for (; itr.More(); itr.Next())
        {
          TNaming_Builder StartBuilder(theResultLabel.NewChild());
          StartBuilder.Generated(itr.Value());
        }
      }
    }

    // Insert end shape
    if (!EndShape.IsNull())
    {
      if (EndShape.ShapeType() != TopAbs_COMPOUND)
      {
        TNaming_Builder EndBuilder(theResultLabel.NewChild());
        if (SubShapes.IsBound(EndShape))
        {
          EndShape = SubShapes(EndShape);
        }
        EndBuilder.Generated(EndShape);
        if (EndShape.ShapeType() != TopAbs_FACE)
        {
          TopoDS_Iterator it(EndShape);
          for (; it.More(); it.Next())
          {
            TNaming_Builder aBuilder(theResultLabel.NewChild());
            aBuilder.Generated(it.Value());
          }
        }
      }
      else
      {
        TopoDS_Iterator itr(EndShape);
        for (; itr.More(); itr.Next())
        {
          TNaming_Builder EndBuilder(theResultLabel.NewChild());
          EndBuilder.Generated(itr.Value());
        }
      }
    }
    if (hasDangle)
    {
      if (!isBasisClosed)
      {
        // dangle edges
        const ShapeList&        Shapes = MS.Generated(Vfirst);
        TopTools_ListIteratorOfListOfShape it(Shapes);
        for (; it.More(); it.Next())
        {
          if (!BRepInspector::Degenerated(TopoDS::Edge(it.Value())))
          {
            TNaming_Builder aBuilder(theResultLabel.NewChild());
            aBuilder.Generated(Vfirst, it.Value());
          }
#ifdef OCCT_DEBUG
          else
          {
            if (MS.HasDegenerated())
              std::cout << "mkRevol has degenerated" << std::endl;
            std::cout << "BRepInspector found degenerated edge (from Vfirst) TS = "
                      << it.Value().TShape().get() << std::endl;
          }
#endif
        }

        const ShapeList& Shapes2 = MS.Generated(Vlast);
        it.Initialize(Shapes2);
        for (; it.More(); it.Next())
        {
          if (!BRepInspector::Degenerated(TopoDS::Edge(it.Value())))
          {
            TNaming_Builder aBuilder(theResultLabel.NewChild());
            aBuilder.Generated(Vlast, it.Value());
          }
#ifdef OCCT_DEBUG
          else
          {
            if (MS.HasDegenerated())
              std::cout << "mkRevol has degenerated" << std::endl;
            std::cout << "BRepInspector found degenerated edge (from Vlast) TS = "
                      << it.Value().TShape().get() << std::endl;
          }
#endif
        }
      }
    }
  }
}
