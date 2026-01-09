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

#include <GC_MakeLine.hxx>
#include <gce_MakeLin.hxx>
#include <Geom_Line.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <StdFail_NotDone.hxx>

//=========================================================================
//   Constructions of 3d geometrical elements from Geom.
//=========================================================================
GC_MakeLine::GC_MakeLine(const Point3d& P, const Dir3d& V)
{
  TheError = gce_Done;
  TheLine  = new GeomLine(P, V);
}

GC_MakeLine::GC_MakeLine(const Axis3d& A1)
{
  TheError = gce_Done;
  TheLine  = new GeomLine(A1);
}

GC_MakeLine::GC_MakeLine(const gp_Lin& L)
{
  TheError = gce_Done;
  TheLine  = new GeomLine(L);
}

GC_MakeLine::GC_MakeLine(const Point3d& P1, const Point3d& P2)
{
  LineBuilder L(P1, P2);
  TheError = L.Status();
  if (TheError == gce_Done)
  {
    TheLine = new GeomLine(L.Value());
  }
}

GC_MakeLine::GC_MakeLine(const gp_Lin& Lin, const Point3d& Point)
{
  LineBuilder L(Lin, Point);
  TheError = L.Status();
  if (TheError == gce_Done)
  {
    TheLine = new GeomLine(L.Value());
  }
}

const Handle(GeomLine)& GC_MakeLine::Value() const
{
  StdFail_NotDone_Raise_if(TheError != gce_Done, "GC_MakeLine::Value() - no result");
  return TheLine;
}
