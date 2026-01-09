// Created on: 1992-10-02
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

#include <GC_MakeCircle.hxx>
#include <gce_MakeCirc.hxx>
#include <Geom_Circle.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <StdFail_NotDone.hxx>

GC_MakeCircle::GC_MakeCircle(const gp_Circ& C)
{
  TheError  = gce_Done;
  TheCircle = new GeomCircle(C);
}

GC_MakeCircle::GC_MakeCircle(const Frame3d& A2, const Standard_Real Radius)
{
  if (Radius < 0.)
  {
    TheError = gce_NegativeRadius;
  }
  else
  {
    TheError  = gce_Done;
    TheCircle = new GeomCircle(gp_Circ(A2, Radius));
  }
}

GC_MakeCircle::GC_MakeCircle(const gp_Circ& Circ, const Point3d& Point)
{
  gp_Circ C = CircleBuilder1(Circ, Point);
  TheCircle = new GeomCircle(C);
  TheError  = gce_Done;
}

GC_MakeCircle::GC_MakeCircle(const gp_Circ& Circ, const Standard_Real Dist)
{
  CircleBuilder1 C = CircleBuilder1(Circ, Dist);
  TheError       = C.Status();
  if (TheError == gce_Done)
  {
    TheCircle = new GeomCircle(C.Value());
  }
}

GC_MakeCircle::GC_MakeCircle(const Point3d& P1, const Point3d& P2, const Point3d& P3)
{
  CircleBuilder1 C = CircleBuilder1(P1, P2, P3);
  TheError       = C.Status();
  if (TheError == gce_Done)
  {
    TheCircle = new GeomCircle(C.Value());
  }
}

GC_MakeCircle::GC_MakeCircle(const Point3d& Point, const Dir3d& Norm, const Standard_Real Radius)
{
  CircleBuilder1 C = CircleBuilder1(Point, Norm, Radius);
  TheError       = C.Status();
  if (TheError == gce_Done)
  {
    TheCircle = new GeomCircle(C.Value());
  }
}

GC_MakeCircle::GC_MakeCircle(const Point3d& Point, const Point3d& PtAxis, const Standard_Real Radius)
{
  CircleBuilder1 C = CircleBuilder1(Point, PtAxis, Radius);
  TheError       = C.Status();
  if (TheError == gce_Done)
  {
    TheCircle = new GeomCircle(C.Value());
  }
}

GC_MakeCircle::GC_MakeCircle(const Axis3d& Axis, const Standard_Real Radius)
{
  CircleBuilder1 C = CircleBuilder1(Axis, Radius);
  TheError       = C.Status();
  if (TheError == gce_Done)
  {
    TheCircle = new GeomCircle(C.Value());
  }
}

const Handle(GeomCircle)& GC_MakeCircle::Value() const
{
  StdFail_NotDone_Raise_if(TheError != gce_Done, "GC_MakeCircle::Value() - no result");
  return TheCircle;
}
