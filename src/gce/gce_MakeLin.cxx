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

#include <gce_MakeLin.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <StdFail_NotDone.hxx>

//=========================================================================
//   Creation d une ligne 3d de gp1 a partir d un Ax1 de gp1.               +
//=========================================================================
LineBuilder::LineBuilder(const Axis3d& A1)
{
  TheLin   = gp_Lin(A1);
  TheError = gce_Done;
}

//=========================================================================
//   Creation d une ligne 3d de gp1 a partir de son origine P (Pnt de gp1)  +
//   et d une direction V (Dir de gp1).                                    +
//=========================================================================

LineBuilder::LineBuilder(const Point3d& P, const Dir3d& V)
{
  TheLin   = gp_Lin(P, V);
  TheError = gce_Done;
}

//=========================================================================
//   Creation d une ligne 3d de gp1 passant par les deux points <P1> et    +
//   <P2>.                                                                +
//=========================================================================

LineBuilder::LineBuilder(const Point3d& P1, const Point3d& P2)
{
  if (P1.Distance(P2) >= gp1::Resolution())
  {
    TheLin   = gp_Lin(P1, Dir3d(P2.XYZ() - P1.XYZ()));
    TheError = gce_Done;
  }
  else
  {
    TheError = gce_ConfusedPoints;
  }
}

//=========================================================================
//   Creation d une ligne 3d de gp1 parallele a une autre <Lin> et passant +
//   par le point <P>.                                                    +
//=========================================================================

LineBuilder::LineBuilder(const gp_Lin& Lin, const Point3d& P)
{
  TheLin   = gp_Lin(P, Lin.Direction());
  TheError = gce_Done;
}

const gp_Lin& LineBuilder::Value() const
{
  StdFail_NotDone_Raise_if(TheError != gce_Done, "LineBuilder::Value() - no result");
  return TheLin;
}

const gp_Lin& LineBuilder::Operator() const
{
  return Value();
}

LineBuilder::operator gp_Lin() const
{
  return Value();
}
