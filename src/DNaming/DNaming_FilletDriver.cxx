// Created on: 2009-05-06
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

#include <BRepAlgo.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <DNaming.hxx>
#include <DNaming_FilletDriver.hxx>
#include <ModelDefinitions.hxx>
#include <Precision.hxx>
#include <Standard_Real.hxx>
#include <Standard_Type.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Real.hxx>
#include <TDF_Label.hxx>
#include <TFunction_Function.hxx>
#include <TFunction_Logbook.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DNaming_FilletDriver, TFunction_Driver)

//=================================================================================================

DNaming_FilletDriver::DNaming_FilletDriver() {}

//=======================================================================
// function : Validate
// purpose  : Validates labels of a function in <log>.
//=======================================================================
void DNaming_FilletDriver::Validate(Handle(TFunction_Logbook)&) const {}

//=======================================================================
// function : MustExecute
// purpose  : Analyse in <log> if the loaded function must be executed
//=======================================================================
Standard_Boolean DNaming_FilletDriver::MustExecute(const Handle(TFunction_Logbook)&) const
{
  return Standard_True;
}

//=======================================================================
// function : Execute
// purpose  : Execute the function and push in <log> the impacted labels
//=======================================================================
Standard_Integer DNaming_FilletDriver::Execute(Handle(TFunction_Logbook)& theLog) const
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
    std::cout << "FilletDriver:: Context is empty" << std::endl;
#endif
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }

  const Standard_Real      aRadius = DNaming1::GetReal(aFunction, FILLET_RADIUS)->Get();
  const ChFi3d_FilletShape aSurfaceType =
    (ChFi3d_FilletShape)DNaming1::GetInteger(aFunction, FILLET_SURFTYPE)->Get();

  if (aRadius < Precision1::Confusion())
  {
    aFunction->SetFailure(WRONG_ARGUMENT);
#ifdef OCCT_DEBUG
    std::cout << "FilletDriver:: Radius < Precision1::Confusion" << std::endl;
#endif
    return -1;
  }

  Handle(TDataStd_UAttribute) aPathObj = DNaming1::GetObjectArg(aFunction, FILLET_PATH);
  Handle(ShapeAttribute)  aPathNS  = DNaming1::GetObjectValue(aPathObj);
  if (aPathNS.IsNull() || aPathNS->IsEmpty())
  {
#ifdef OCCT_DEBUG
    std::cout << "FilletDriver:: Path is empty" << std::endl;
#endif
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }

  TopoShape aPATH    = aPathNS->Get();
  TopoShape aCONTEXT = aContextNS->Get();
  if (aPATH.IsNull() || aCONTEXT.IsNull())
  {
#ifdef OCCT_DEBUG
    std::cout << "FilletDriver:: Path or Context is null" << std::endl;
#endif
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }

  ShapeExplorer     expl;
  TopTools_MapOfShape View;

  BRepFilletAPI_MakeFillet aMkFillet(aCONTEXT, aSurfaceType);

  if (aPATH.ShapeType() != TopAbs_EDGE && aPATH.ShapeType() != TopAbs_FACE)
  {
    aFunction->SetFailure(WRONG_ARGUMENT);
    return -1;
  }

  if (aPATH.ShapeType() == TopAbs_FACE)
  {
    for (expl.Init(aPATH, TopAbs_EDGE); expl.More(); expl.Next())
    {
      const TopoEdge& anEdge = TopoDS::Edge(expl.Current());
      if (!View.Add(anEdge))
        continue;
      else
        aMkFillet.Add(aRadius, anEdge); // Edge
    }
  }
  else
  {
    const TopoEdge& anEdge = TopoDS::Edge(aPATH);
    aMkFillet.Add(aRadius, anEdge); // Edge
  }

  aMkFillet.Build();

  if (!aMkFillet.IsDone())
  {
    aFunction->SetFailure(ALGO_FAILED);
    return -1;
  }
  ShapeList aLarg;
  aLarg.Append(aCONTEXT);
  if (!BRepAlgo1::IsValid(aLarg, aMkFillet.Shape(), Standard_False, Standard_False))
  {
    aFunction->SetFailure(RESULT_NOT_VALID);
    return -1;
  }

  // Naming1
  LoadNamingDS(RESPOSITION(aFunction), aMkFillet, aCONTEXT);

  theLog->SetValid(RESPOSITION(aFunction), Standard_True);
  aFunction->SetFailure(DONE);
  return 0;
}

//=================================================================================================

void DNaming_FilletDriver::LoadNamingDS(const DataLabel&          theResultLabel,
                                        BRepFilletAPI_MakeFillet& theMkFillet,
                                        const TopoShape&       theContext) const
{
  TNaming_Builder aBuilder(theResultLabel);
  TopoShape    aResult = theMkFillet.Shape();

  if (aResult.ShapeType() == TopAbs_COMPOUND)
  {
    if (aResult.NbChildren() == 1)
    {
      TopoDS_Iterator itr(aResult);
      if (itr.More())
        aResult = itr.Value();
    }
  }
  if (aResult.IsNull())
    aBuilder.Generated(aResult);
  else
    aBuilder.Modify(theContext, aResult);

  TopTools_DataMapOfShapeShape SubShapes;
  for (ShapeExplorer Exp(aResult, TopAbs_FACE); Exp.More(); Exp.Next())
  {
    SubShapes.Bind(Exp.Current(), Exp.Current());
  }

  // New faces generated from edges
  TNaming_Builder anEdgeBuilder(theResultLabel.FindChild(1, Standard_True));
  DNaming1::LoadAndOrientGeneratedShapes(theMkFillet,
                                        theContext,
                                        TopAbs_EDGE,
                                        anEdgeBuilder,
                                        SubShapes);

  // Faces of the initial shape modified by theMkFillet
  TNaming_Builder aFacesBuilder(theResultLabel.FindChild(2, Standard_True));
  DNaming1::LoadAndOrientModifiedShapes(theMkFillet,
                                       theContext,
                                       TopAbs_FACE,
                                       aFacesBuilder,
                                       SubShapes);

  // New faces generated from vertices (if exist)
  TNaming_Builder aVFacesBuilder(theResultLabel.FindChild(3, Standard_True));
  DNaming1::LoadAndOrientGeneratedShapes(theMkFillet,
                                        theContext,
                                        TopAbs_VERTEX,
                                        aVFacesBuilder,
                                        SubShapes);

  // Deleted faces of the initial shape
  TNaming_Builder aDelBuilder(theResultLabel.FindChild(4, Standard_True));
  DNaming1::LoadDeletedShapes(theMkFillet, theContext, TopAbs_FACE, aDelBuilder);
}
