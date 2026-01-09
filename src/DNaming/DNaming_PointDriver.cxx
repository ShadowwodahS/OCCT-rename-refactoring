// Created on: 2010-02-25
// Created by: Sergey ZARITCHNY
// Copyright (c) 2010-2014 OPEN CASCADE SAS
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
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <DNaming.hxx>
#include <DNaming_PointDriver.hxx>
#include <gp_Pnt.hxx>
#include <ModelDefinitions.hxx>
#include <Standard_Type.hxx>
#include <TDataStd_Real.hxx>
#include <TDF_Label.hxx>
#include <TFunction_Function.hxx>
#include <TFunction_Logbook.hxx>
#include <TNaming.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DNaming_PointDriver, FunctionDriver1)

// OCCT
//  OCAF
//=================================================================================================

DNaming_PointDriver::DNaming_PointDriver() {}

//=======================================================================
// function : Validate
// purpose  : Validates labels of a function in <log>.
//=======================================================================
void DNaming_PointDriver::Validate(Handle(TFunction_Logbook)&) const {}

//=======================================================================
// function : MustExecute
// purpose  : Analyse in <log> if the loaded function must be executed
//=======================================================================
Standard_Boolean DNaming_PointDriver::MustExecute(const Handle(TFunction_Logbook)&) const
{
  return Standard_True;
}

//=======================================================================
// function : Execute
// purpose  : Execute the function and push in <log> the impacted labels
//=======================================================================
Standard_Integer DNaming_PointDriver::Execute(Handle(TFunction_Logbook)& theLog) const
{
  Handle(TFunction_Function) aFunction;
  Label().FindAttribute(TFunction_Function::GetID(), aFunction);
  if (aFunction.IsNull())
    return -1;

  // perform calculations

  Standard_Real aDX = DNaming1::GetReal(aFunction, PNT_DX)->Get();
  Standard_Real aDY = DNaming1::GetReal(aFunction, PNT_DY)->Get();
  Standard_Real aDZ = DNaming1::GetReal(aFunction, PNT_DZ)->Get();

  Handle(ShapeAttribute) aPrevPnt = DNaming1::GetFunctionResult(aFunction);
  // Save location
  TopLoc_Location aLocation;
  if (!aPrevPnt.IsNull() && !aPrevPnt->IsEmpty())
  {
    aLocation = aPrevPnt->Get().Location();
  }
  Point3d aPoint;
  if (aFunction->GetDriverGUID() == PNTRLT_GUID)
  {
    Handle(TDataStd_UAttribute) aRefPnt   = DNaming1::GetObjectArg(aFunction, PNTRLT_REF);
    Handle(ShapeAttribute)  aRefPntNS = DNaming1::GetObjectValue(aRefPnt);
    if (aRefPntNS.IsNull() || aRefPntNS->IsEmpty())
    {
#ifdef OCCT_DEBUG
      std::cout << "PointDriver:: Ref Point is empty" << std::endl;
#endif
      aFunction->SetFailure(WRONG_ARGUMENT);
      return -1;
    }
    TopoShape  aRefPntShape = aRefPntNS->Get();
    TopoVertex aVertex      = TopoDS::Vertex(aRefPntShape);
    aPoint                     = BRepInspector::Pnt(aVertex);
    aPoint.SetX(aPoint.X() + aDX);
    aPoint.SetY(aPoint.Y() + aDY);
    aPoint.SetZ(aPoint.Z() + aDZ);
  }
  else
    aPoint = Point3d(aDX, aDY, aDZ);

  BRepBuilderAPI_MakeVertex aMakeVertex(aPoint);

  if (!aMakeVertex.IsDone())
  {
    aFunction->SetFailure(ALGO_FAILED);
    return -1;
  }

  // Naming1
  const DataLabel& aResultLabel = RESPOSITION(aFunction);
  TNaming_Builder  aBuilder(aResultLabel);
  aBuilder.Generated(aMakeVertex.Shape());

  // restore location
  if (!aLocation.IsIdentity())
    TNaming1::Displace(aResultLabel, aLocation, Standard_True);

  theLog->SetValid(aResultLabel, Standard_True);

  aFunction->SetFailure(DONE);
  return 0;
}
