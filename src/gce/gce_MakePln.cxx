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

#include <gce_MakePln.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <StdFail_NotDone.hxx>

gce_MakePln::gce_MakePln(const Frame3d& A2)
{
  ThePln   = gp_Pln(Ax3(A2));
  TheError = gce_Done;
}

gce_MakePln::gce_MakePln(const Point3d& P, const Dir3d& V)
{
  ThePln   = gp_Pln(P, V);
  TheError = gce_Done;
}

gce_MakePln::gce_MakePln(const Point3d& P1, const Point3d& P2)
{
  if (P1.Distance(P2) <= gp1::Resolution())
  {
    TheError = gce_ConfusedPoints;
  }
  else
  {
    Dir3d dir(P2.XYZ() - P1.XYZ());
    ThePln   = gp_Pln(P1, dir);
    TheError = gce_Done;
  }
}

gce_MakePln::gce_MakePln(const Standard_Real A,
                         const Standard_Real B,
                         const Standard_Real C,
                         const Standard_Real D)
{
  if (A * A + B * B + C * C <= gp1::Resolution())
  {
    TheError = gce_BadEquation;
  }
  else
  {
    ThePln   = gp_Pln(A, B, C, D);
    TheError = gce_Done;
  }
}

//=========================================================================
//   Creation d un gp_pln passant par trois points.                       +
//=========================================================================

gce_MakePln::gce_MakePln(const Point3d& P1, const Point3d& P2, const Point3d& P3)
{
  Coords3d V1(P2.XYZ() - P1.XYZ());
  Coords3d V2(P3.XYZ() - P1.XYZ());
  Coords3d Norm(V1.Crossed(V2));
  if (Norm.Modulus() < gp1::Resolution())
  {
    TheError = gce_ColinearPoints;
  }
  else
  {
    Dir3d DNorm(Norm);
    Dir3d Dx(V1);
    ThePln   = gp_Pln(Ax3(P1, DNorm, Dx));
    TheError = gce_Done;
  }
}

//=========================================================================
//   Creation d un gp_pln parallele a un autre pln a une distance donnee. +
//=========================================================================

gce_MakePln::gce_MakePln(const gp_Pln& Pl, const Standard_Real Dist)
{
  Point3d Center(Pl.Location().XYZ() + Dist * Coords3d(Pl.Axis().Direction().XYZ()));
  ThePln   = gp_Pln(Ax3(Center, Pl.Axis().Direction(), Pl.XAxis().Direction()));
  TheError = gce_Done;
}

//=========================================================================
//   Creation d un gp_pln parallele a un autre pln passant par un point   +
//   <Point1>.                                                            +
//=========================================================================

gce_MakePln::gce_MakePln(const gp_Pln& Pl, const Point3d& Point)
{
  ThePln   = gp_Pln(Ax3(Point, Pl.Axis().Direction(), Pl.XAxis().Direction()));
  TheError = gce_Done;
}

//=========================================================================
//  Creation d un gp_pln a partir d un Ax1 (Point + Normale).             +
//=========================================================================

gce_MakePln::gce_MakePln(const Axis3d& Axis)
{
  ThePln   = gp_Pln(Axis.Location(), Axis.Direction());
  TheError = gce_Done;
}

//=========================================================================
//  Creation d un gp_pln par un tableau de points.                        +
//=========================================================================

/*gce_MakePln::gce_MakePln(const gp_Array1OfPnt& Pts     ,
                   Standard_Real   ErrMax  ,
                   Standard_Real   ErrMean )
{
  TheError = gce_ConfusedPoints;
}
*/
const gp_Pln& gce_MakePln::Value() const
{
  StdFail_NotDone_Raise_if(TheError != gce_Done, "gce_MakePln::Value() - no result");
  return ThePln;
}

const gp_Pln& gce_MakePln::Operator() const
{
  return Value();
}

gce_MakePln::operator gp_Pln() const
{
  return Value();
}
