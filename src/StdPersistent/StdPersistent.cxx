// Copyright (c) 2015 OPEN CASCADE SAS
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

#include <StdPersistent.hxx>
#include <StdObjMgt_MapOfInstantiators.hxx>

#include <StdPersistent_TopLoc.hxx>
#include <StdPersistent_Naming.hxx>
#include <StdPersistent_HArray1.hxx>
#include <StdLPersistent_HArray1.hxx>
#include <StdPersistent_DataXtd.hxx>
#include <StdPersistent_DataXtd_Constraint.hxx>
#include <StdPersistent_DataXtd_PatternStd.hxx>
#include <StdPersistent_PPrsStd.hxx>

//=======================================================================
// function : BindTypes
// purpose  : Register types
//=======================================================================
void StdPersistent1::BindTypes(MapOfInstantiators& theMap)
{
  // Non-attribute data
  theMap.Bind<TopLoc::Datum3D1>("PTopLoc_Datum3D");
  theMap.Bind<TopLoc::ItemLocation>("PTopLoc_ItemLocation");
  theMap.Bind<StdPersistent_TopoDS::TShape>("PTopoDS_TShape1");
  theMap.Bind<StdPersistent_HArray1::Shape1>("PTopoDS_HArray1OfShape1");
  theMap.Bind<Naming2::Name>("PNaming_Name");
  theMap.Bind<Naming2::Name_1>("PNaming_Name_1");
  theMap.Bind<Naming2::Name_2>("PNaming_Name_2");

  theMap.Bind<HArray1::Persistent>("PNaming_HArray1OfNamedShape");

  // Attributes
  theMap.Bind<Naming2::NamedShape1>("PNaming_NamedShape");
  theMap.Bind<Naming2::Naming1>("PNaming_Naming");
  theMap.Bind<Naming2::Naming_1>("PNaming_Naming_1");
  theMap.Bind<Naming2::Naming_2>("PNaming_Naming_2");

  theMap.Bind<DataXtd::Shape>("PDataXtd_Shape");
  theMap.Bind<DataXtd::Point>("PDataXtd_Point");
  theMap.Bind<DataXtd::Axis>("PDataXtd_Axis");
  theMap.Bind<DataXtd::Plane1>("PDataXtd_Plane");
  theMap.Bind<DataXtd::Placement>("PDataXtd_Placement");
  theMap.Bind<DataXtd::Geometry1>("PDataXtd_Geometry");
  theMap.Bind<DataXtd::Position1>("PDataXtd_Position");
  theMap.Bind<StdPersistent_DataXtd_Constraint>("PDataXtd_Constraint");
  theMap.Bind<StdPersistent_DataXtd_PatternStd>("PDataXtd_PatternStd");

  theMap.Bind<PPrsStd::AISPresentation>("PPrsStd_AISPresentation");

  theMap.Bind<PPrsStd::AISPresentation_1>("PPrsStd_AISPresentation_1");
}
