// Created on: 1997-03-27
// Created by: Denis PASCAL
// Copyright (c) 1997-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#include <DDataStd.hxx>
#include <TDataStd.hxx>
#include <TDataStd_Real.hxx>
#include <TDataStd_RealEnum.hxx>
#include <TDataXtd.hxx>
#include <TDataXtd_Constraint.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TNaming_NamedShape.hxx>

//=================================================================================================

void DDataStd1::AllCommands(DrawInterpreter& theCommands)
{
  NamedShapeCommands(theCommands);
  BasicCommands(theCommands);
  DatumCommands(theCommands);
  ConstraintCommands(theCommands);
  ObjectCommands(theCommands);
  DrawDisplayCommands(theCommands);
  NameCommands(theCommands);
  TreeCommands(theCommands);
}

//=================================================================================================

void DDataStd1::DumpConstraint(const Handle(TDataXtd_Constraint)& CTR, Standard_OStream& anOS)
{
  AsciiString1 S;
  Tool3::Entry(CTR->Label(), S);
  anOS << S << " ";
  TDataXtd1::Print(CTR->GetType(), anOS);
  for (Standard_Integer i = 1; i <= CTR->NbGeometries(); i++)
  {
    anOS << " G_" << i << " (";
    Tool3::Entry(CTR->GetGeometry(i)->Label(), S);
    anOS << S << ") ";
  }
  if (CTR->IsPlanar())
  {
    anOS << " P (";
    Tool3::Entry(CTR->GetPlane()->Label(), S);
    anOS << S << ") ";
  }
  if (CTR->IsDimension())
  {
    anOS << " V (";
    Tool3::Entry(CTR->GetValue()->Label(), S);
    anOS << S << ") ";
    Standard_DISABLE_DEPRECATION_WARNINGS TDataStd_RealEnum t = CTR->GetValue()->GetDimension();
    TDataStd1::Print(t, anOS);
    Standard_Real val = CTR->GetValue()->Get();
    if (t == TDataStd_ANGULAR)
      val = (180. * val) / M_PI;
    Standard_ENABLE_DEPRECATION_WARNINGS anOS << " ";
    anOS << val;
  }
  if (!CTR->Verified())
    anOS << " NotVerifed";
}
