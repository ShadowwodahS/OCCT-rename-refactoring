// Created on: 1994-07-25
// Created by: Remi LEQUETTE
// Copyright (c) 1994-1999 Matra Datavision
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

#include <BRepTest.hxx>
#include <DBRep.hxx>

//=================================================================================================

void BRepTest1::AllCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  DBRep1::BasicCommands(theCommands);
  BRepTest1::BasicCommands(theCommands);
  BRepTest1::CurveCommands(theCommands);
  BRepTest1::Fillet2DCommands(theCommands);
  BRepTest1::SurfaceCommands(theCommands);
  BRepTest1::FillingCommands(theCommands);
  BRepTest1::PrimitiveCommands(theCommands);
  BRepTest1::SweepCommands(theCommands);
  BRepTest1::TopologyCommands(theCommands);
  BRepTest1::FilletCommands(theCommands);
  BRepTest1::ChamferCommands(theCommands);
  BRepTest1::GPropCommands(theCommands);
  BRepTest1::MatCommands(theCommands);
  BRepTest1::DraftAngleCommands(theCommands);
  BRepTest1::FeatureCommands(theCommands);
  BRepTest1::OtherCommands(theCommands);
  BRepTest1::ExtremaCommands(theCommands);
  BRepTest1::CheckCommands(theCommands);
  //  BRepTest1::PlacementCommands(theCommands) ;
  BRepTest1::ProjectionCommands(theCommands);
  BRepTest1::HistoryCommands(theCommands);

  // define the TCL variable Draw_TOPOLOGY
  const char* com = "set Draw_TOPOLOGY 1";
  theCommands.Eval(com);
}
