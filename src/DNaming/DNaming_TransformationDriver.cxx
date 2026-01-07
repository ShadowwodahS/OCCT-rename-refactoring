// Created on: 2009-05-07
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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <DNaming.hxx>
#include <DNaming_TransformationDriver.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <ModelDefinitions.hxx>
#include <NCollection_Handle.hxx>
#include <Standard_Type.hxx>
#include <TDataStd_Real.hxx>
#include <TDF_Label.hxx>
#include <TFunction_Function.hxx>
#include <TFunction_Logbook.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_MapOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DNaming_TransformationDriver, TFunction_Driver)

#ifdef _WIN32
  #define EXCEPTION ...
#else
  #define EXCEPTION ExceptionBase const&
#endif

#define FACES_TAG 1
#define EDGES_TAG 2
#define VERTEX_TAG 3

// #define MDTV_DEB_TRSF
#ifdef OCCT_DEBUG_TRSF
  #include <TCollection_AsciiString.hxx>
  #include <BRepTools.hxx>
  #include <TDF_Tool.hxx>

void PrintE(const DataLabel& label)
{
  AsciiString1 entry;
  TDF_Tool::Entry(label, entry);
  std::cout << "LabelEntry = " << entry << std::endl;
}
#endif
//=================================================================================================

DNaming_TransformationDriver::DNaming_TransformationDriver() {}

//=======================================================================
// function : Validate
// purpose  : Validates labels of a function in <log>.
//=======================================================================
void DNaming_TransformationDriver::Validate(Handle(TFunction_Logbook)&) const {}

//=======================================================================
// function : MustExecute
// purpose  : Analyse in <log> if the loaded function must be executed
//=======================================================================
Standard_Boolean DNaming_TransformationDriver::MustExecute(const Handle(TFunction_Logbook)&) const
{
  return Standard_True;
}

//=======================================================================
// function : Execute
// purpose  : Execute the function and push in <log> the impacted labels
//=======================================================================
Standard_Integer DNaming_TransformationDriver::Execute(Handle(TFunction_Logbook)& theLog) const
{
  Handle(TFunction_Function) aFunction;
  Label().FindAttribute(TFunction_Function::GetID(), aFunction);
  if (aFunction.IsNull())
    return -1;

  Handle(TFunction_Function) aPrevFun = DNaming1::GetPrevFunction(aFunction);
  if (aPrevFun.IsNull())
    return -1;
  const DataLabel&           aLab = RESPOSITION(aPrevFun);
  Handle(ShapeAttribute) aContextNS;
  aLab.FindAttribute(ShapeAttribute::GetID(), aContextNS);
  if (aContextNS.IsNull() || aContextNS->IsEmpty())
  {
#ifdef OCCT_DEBUG
    std::cout << "TransformationDriver:: Context is empty" << std::endl;
#endif
    aFunction->SetFailure(WRONG_CONTEXT);
    return -1;
  }
  //
  Transform3d              aTransformation;
  const Standard_GUID& aGUID = aFunction->GetDriverGUID();

  try
  {
    if (aGUID == PTXYZ_GUID)
    {
      Standard_Real aDX = DNaming1::GetReal(aFunction, PTRANSF_DX)->Get();
      Standard_Real aDY = DNaming1::GetReal(aFunction, PTRANSF_DY)->Get();
      Standard_Real aDZ = DNaming1::GetReal(aFunction, PTRANSF_DZ)->Get();
      Vector3d        aVector(aDX, aDY, aDZ);
      aTransformation.SetTranslation(aVector);
    }
    else if (aGUID == PTALINE_GUID)
    {
      Handle(TDataStd_UAttribute) aLineObj = DNaming1::GetObjectArg(aFunction, PTRANSF_LINE);
      Handle(ShapeAttribute)  aLineNS  = DNaming1::GetObjectValue(aLineObj);
      Axis3d                      anAxis;
      if (!DNaming1::ComputeAxis(aLineNS, anAxis))
        throw ExceptionBase();
      Vector3d aVector(anAxis.Direction());
      aVector.Normalize();
      Standard_Real anOffset = DNaming1::GetReal(aFunction, PTRANSF_OFF)->Get();
      aVector *= anOffset;
      aTransformation.SetTranslation(aVector);
    }
    else if (aGUID == PRRLINE_GUID)
    {
      Handle(TDataStd_UAttribute) aLineObj = DNaming1::GetObjectArg(aFunction, PTRANSF_LINE);
      Handle(ShapeAttribute)  aLineNS  = DNaming1::GetObjectValue(aLineObj);
      Axis3d                      anAxis;
      if (!DNaming1::ComputeAxis(aLineNS, anAxis))
        throw ExceptionBase();

      Standard_Real anAngle = DNaming1::GetReal(aFunction, PTRANSF_ANG)->Get();
      aTransformation.SetRotation(anAxis, anAngle);
    }
    else if (aGUID == PMIRR_GUID)
    {
      Handle(TDataStd_UAttribute) aPlaneObj = DNaming1::GetObjectArg(aFunction, PTRANSF_PLANE);
      Handle(ShapeAttribute)  aNS       = DNaming1::GetObjectValue(aPlaneObj);

      if (aNS.IsNull() || aNS->IsEmpty() || aNS->Get().IsNull()
          || aNS->Get().ShapeType() != TopAbs_FACE)
        throw ExceptionBase();
      TopoFace             aFace = TopoDS::Face(aNS->Get());
      Handle(GeomSurface)    aSurf = BRepInspector::Surface(aFace);
      GeomLib_IsPlanarSurface isPlanarSurface(aSurf);
      if (!isPlanarSurface.IsPlanar())
        throw ExceptionBase();
      gp_Pln aPlane     = isPlanarSurface.Plan();
      Frame3d aMirrorAx2 = aPlane.Position().Ax2();
      aTransformation.SetMirror(aMirrorAx2);
    }
    else
    {
      aFunction->SetFailure(UNSUPPORTED_FUNCTION);
      return -1;
    }
  }
  catch (EXCEPTION)
  {
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }
  //

  // Naming
  LoadNamingDS(RESPOSITION(aFunction), aContextNS, aTransformation);

  theLog->SetValid(RESPOSITION(aFunction), Standard_True);
  aFunction->SetFailure(DONE);
  return 0;
}

//=================================================================================
static void BuildMap(const TopTools_MapOfShape&    SMap,
                     BRepBuilderAPI_Transform&     Transformer,
                     TopTools_DataMapOfShapeShape& M)
{
  TopTools_MapIteratorOfMapOfShape anIt(SMap);
  for (; anIt.More(); anIt.Next())
  {
    if (!anIt.Key().IsNull())
    {
      const TopoShape& aS = anIt.Key();
      M.Bind(aS, Transformer.ModifiedShape(aS));
    }
  }
}

//=================================================================================
static void CollectShapes(const TopoShape&             SSh,
                          TopoCompound&                C,
                          TopTools_MapOfShape&            SMap,
                          const DataLabel&                theLab,
                          TopTools_DataMapOfShapeInteger& TagMap,
                          const Standard_Boolean          isPrimitive)
{
  const TopAbs_ShapeEnum aType = SSh.ShapeType();
  ShapeBuilder           aB;
  switch (aType)
  {
    case TopAbs_COMPOUND: {
      TopoDS_Iterator it(SSh);
      for (; it.More(); it.Next())
        CollectShapes(it.Value(), C, SMap, theLab, TagMap, isPrimitive);
    }
    break;
    case TopAbs_COMPSOLID:
    case TopAbs_SOLID:
    case TopAbs_SHELL: {
      ShapeExplorer anEx(SSh, TopAbs_FACE);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
      anEx.Init(SSh, TopAbs_EDGE);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
      anEx.Init(SSh, TopAbs_VERTEX);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
    }
    break;
    case TopAbs_FACE: {
      const Handle(ShapeAttribute) aNamedShape = Tool11::NamedShape(SSh, theLab);
      if (!aNamedShape.IsNull())
        if (SMap.Add(SSh))
          aB.Add(C, SSh);
      ShapeExplorer anEx(SSh, TopAbs_EDGE);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
      anEx.Init(SSh, TopAbs_VERTEX);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
    }
    break;
    case TopAbs_WIRE: {
      ShapeExplorer anEx(SSh, TopAbs_EDGE);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
      anEx.Init(SSh, TopAbs_VERTEX);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
    }
    break;

    case TopAbs_EDGE: {
      const Handle(ShapeAttribute) aNamedShape = Tool11::NamedShape(SSh, theLab);
      if (!aNamedShape.IsNull())
        if (SMap.Add(SSh))
          aB.Add(C, SSh);
      ShapeExplorer anEx(SSh, TopAbs_VERTEX);
      anEx.Init(SSh, TopAbs_VERTEX);
      for (; anEx.More(); anEx.Next())
      {
        const Handle(ShapeAttribute) aNS = Tool11::NamedShape(anEx.Current(), theLab);
        if (aNS.IsNull())
          continue;
        if (SMap.Add(anEx.Current()))
        {
          aB.Add(C, anEx.Current());
          if (isPrimitive)
            TagMap.Bind(anEx.Current(), aNS->Label().Tag());
        }
      }
    }
    break;
    case TopAbs_VERTEX: {
      const Handle(ShapeAttribute) aNS = Tool11::NamedShape(SSh, theLab);
      if (!aNS.IsNull())
        if (SMap.Add(SSh))
        {
          aB.Add(C, SSh);
          //	if(isPrimitive)
          //	  TagMap.Bind(SSh, aNS->Label().Tag());
        }
    }
    break;
    default:
      break;
  }
}

//=================================================================================================

void DNaming_TransformationDriver::LoadNamingDS(const DataLabel&                  theResultLabel,
                                                const Handle(ShapeAttribute)& theSourceNS,
                                                const Transform3d&                    theTrsf) const
{
  if (theSourceNS.IsNull() || theSourceNS->IsEmpty())
    return;
  const TopoShape& aSrcShape = theSourceNS->Get();
  if (aSrcShape.IsNull())
  {
#ifdef OCCT_DEBUG
    std::cout
      << "DNaming_TransformationDriver::LoadNamingDS: The result of the Transform operation is null"
      << std::endl;
#endif
    return;
  }
  Standard_Boolean isPrimitive(Standard_False);
  if (theSourceNS->Evolution() == TNaming_PRIMITIVE)
    isPrimitive = Standard_True;
  const DataLabel& aSrcLabel = theSourceNS->Label();
#ifdef OCCT_DEBUG_TRSF
  std::cout << "TransformationDriver: ";
  PrintE(aSrcLabel);
#endif

  TopoCompound aCompShape;
  ShapeBuilder    aB;
  aB.MakeCompound(aCompShape);
  TopTools_MapOfShape            aSMap;
  TopTools_DataMapOfShapeInteger aTagMap;
  // Collect  shapes
  if (aSMap.Add(aSrcShape))
    aB.Add(aCompShape, aSrcShape);
  CollectShapes(aSrcShape, aCompShape, aSMap, aSrcLabel, aTagMap, isPrimitive);

  // Transform
  BRepBuilderAPI_Transform     aTransformer(aCompShape, theTrsf, Standard_False);
  TopTools_DataMapOfShapeShape aTMap;
  BuildMap(aSMap, aTransformer, aTMap);

  // Load
  TopoShape aNewSh;
  if (aTMap.IsBound(aSrcShape))
    aNewSh = aTMap(aSrcShape);
  if (!aNewSh.IsNull())
  {
    TNaming_Builder aBuilder(theResultLabel);
    aBuilder.Modify(aSrcShape, aNewSh);
    aTMap.UnBind(aSrcShape);
  }

  TopTools_DataMapOfShapeShape SubShapes;
  ShapeExplorer              Exp(aNewSh, TopAbs_FACE);
  for (; Exp.More(); Exp.Next())
  {
    SubShapes.Bind(Exp.Current(), Exp.Current());
  }
  for (Exp.Init(aNewSh, TopAbs_EDGE); Exp.More(); Exp.Next())
  {
    SubShapes.Bind(Exp.Current(), Exp.Current());
  }
  for (Exp.Init(aNewSh, TopAbs_VERTEX); Exp.More(); Exp.Next())
  {
    SubShapes.Bind(Exp.Current(), Exp.Current());
  }

  Standard_Integer                                aNextTag(0);
  TopTools_DataMapIteratorOfDataMapOfShapeInteger it(aTagMap);
  for (; it.More(); it.Next())
  {
    if (it.Value() > aNextTag)
      aNextTag = it.Value();
  }
  NCollection_Handle<TNaming_Builder>           aFBuilder, anEBuilder, aVBuilder;
  TopTools_DataMapIteratorOfDataMapOfShapeShape anIt(aTMap);
  for (; anIt.More(); anIt.Next())
  {
    const TopoShape& aKey     = anIt.Key();
    TopoShape        newShape = anIt.Value();
    if (SubShapes.IsBound(newShape))
    {
      newShape.Orientation((SubShapes(newShape)).Orientation());
    }
    if (isPrimitive)
    {
      if (aTagMap.IsBound(aKey))
      {
        const DataLabel& aLabel = theResultLabel.FindChild(aTagMap.Find(aKey), Standard_True);
        TNaming_Builder  aBuilder(aLabel);
        aBuilder.Modify(aKey, newShape);
      }
      else
      {
        aNextTag++;
        const DataLabel& aLabel = theResultLabel.FindChild(aNextTag, Standard_True);
        TNaming_Builder  aBuilder(aLabel);
        aBuilder.Modify(aKey, newShape);
      }
    }
    else
    {
      if (aKey.ShapeType() == TopAbs_FACE)
      {
        if (aFBuilder.IsNull())
        {
          const DataLabel& aFLabel = theResultLabel.FindChild(FACES_TAG, Standard_True);
          aFBuilder                = new TNaming_Builder(aFLabel);
        }
        aFBuilder->Modify(anIt.Key(), newShape);
      }
      else if (aKey.ShapeType() == TopAbs_EDGE)
      {
        if (anEBuilder.IsNull())
        {
          const DataLabel& aELabel = theResultLabel.FindChild(EDGES_TAG, Standard_True);
          anEBuilder               = new TNaming_Builder(aELabel);
        }
        anEBuilder->Modify(anIt.Key(), newShape);
      }
      else if (aKey.ShapeType() == TopAbs_VERTEX)
      {
        if (aVBuilder.IsNull())
        {
          const DataLabel& aVLabel = theResultLabel.FindChild(VERTEX_TAG, Standard_True);
          aVBuilder                = new TNaming_Builder(aVLabel);
        }
        aVBuilder->Modify(anIt.Key(), newShape);
      }
    }
  }
}
