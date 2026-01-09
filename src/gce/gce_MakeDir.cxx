// Created on: 1992-09-02
// Created by: Remi GILET
// Copyright (c) 1992-1999 Matra Datavision
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

#include <gce_MakeDir.hxx>
#include <gp.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <StdFail_NotDone.hxx>

//=========================================================================
//   Creation d une direction 3d (Dir) de gp1 a partir de 2 Pnt de gp1.     +
//=========================================================================
DirectionBuilder::DirectionBuilder(const Point3d& P1, const Point3d& P2)
{
  if (P1.Distance(P2) <= gp1::Resolution())
  {
    TheError = gce_ConfusedPoints;
  }
  else
  {
    TheDir   = Dir3d(P2.XYZ() - P1.XYZ());
    TheError = gce_Done;
  }
}

DirectionBuilder::DirectionBuilder(const Coords3d& Coord)
{
  if (Coord.Modulus() <= gp1::Resolution())
  {
    TheError = gce_NullVector;
  }
  else
  {
    TheDir   = Dir3d(Coord);
    TheError = gce_Done;
  }
}

DirectionBuilder::DirectionBuilder(const Vector3d& V)
{
  if (V.Magnitude() <= gp1::Resolution())
  {
    TheError = gce_NullVector;
  }
  else
  {
    TheDir   = Dir3d(V);
    TheError = gce_Done;
  }
}

DirectionBuilder::DirectionBuilder(const Standard_Real Xv, const Standard_Real Yv, const Standard_Real Zv)
{
  if (Xv * Xv + Yv * Yv + Zv * Zv <= gp1::Resolution())
  {
    TheError = gce_NullVector;
  }
  else
  {
    TheDir   = Dir3d(Xv, Yv, Zv);
    TheError = gce_Done;
  }
}

const Dir3d& DirectionBuilder::Value() const
{
  StdFail_NotDone_Raise_if(TheError != gce_Done, "DirectionBuilder::Value() - no result");
  return TheDir;
}

const Dir3d& DirectionBuilder::Operator() const
{
  return Value();
}

DirectionBuilder::operator Dir3d() const
{
  return Value();
}
