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

#include <GC_MakeConicalSurface.hxx>
#include <gce_MakeCone.hxx>
#include <Geom_ConicalSurface.hxx>
#include <gp.hxx>
#include <gp_Ax2.hxx>
#include <gp_Cone.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <Standard_NotImplemented.hxx>
#include <StdFail_NotDone.hxx>

GC_MakeConicalSurface::GC_MakeConicalSurface(const Frame3d&       A2,
                                             const Standard_Real Ang,
                                             const Standard_Real Radius)
{
  if (Radius < 0.)
  {
    TheError = gce_NegativeRadius;
  }
  else if (Ang <= gp1::Resolution() || Ang >= M_PI / 2. - gp1::Resolution())
  {
    TheError = gce_BadAngle;
  }
  else
  {
    TheError = gce_Done;
    TheCone  = new Geom_ConicalSurface(A2, Ang, Radius);
  }
}

GC_MakeConicalSurface::GC_MakeConicalSurface(const Cone1& C)
{
  TheError = gce_Done;
  TheCone  = new Geom_ConicalSurface(C);
}

//=========================================================================
//   Creation of a cone by four points.                                +
//   two first give the axis.                                     +
//   the third gives the base radius.                              +
//   the third and the fourth the half-angle.                          +
//=========================================================================

GC_MakeConicalSurface::GC_MakeConicalSurface(const Point3d& P1,
                                             const Point3d& P2,
                                             const Point3d& P3,
                                             const Point3d& P4)
{
  ConeBuilder C = ConeBuilder(P1, P2, P3, P4);
  TheError       = C.Status();
  if (TheError == gce_Done)
  {
    TheCone = new Geom_ConicalSurface(C.Value());
  }
}

//=========================================================================
//=========================================================================

GC_MakeConicalSurface::GC_MakeConicalSurface(const Point3d&       P1,
                                             const Point3d&       P2,
                                             const Standard_Real R1,
                                             const Standard_Real R2)
{
  ConeBuilder C = ConeBuilder(P1, P2, R1, R2);
  TheError       = C.Status();
  if (TheError == gce_Done)
  {
    TheCone = new Geom_ConicalSurface(C);
  }
}

const Handle(Geom_ConicalSurface)& GC_MakeConicalSurface::Value() const
{
  StdFail_NotDone_Raise_if(TheError != gce_Done, "GC_MakeConicalSurface::Value() - no result");
  return TheCone;
}
