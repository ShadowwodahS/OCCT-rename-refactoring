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

#include <gce_MakeHypr.hxx>
#include <gp_Ax2.hxx>
#include <gp_Hypr.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <StdFail_NotDone.hxx>

//=========================================================================
//   Creation d une Hyperbole 3d de gp1 de centre <Center> et de sommets   +
//   <S1> et <S2>.                                                        +
//   <CenterS1> donne le grand axe .                                      +
//   <S1> donne le grand rayon et <S2> le petit rayon.                    +
//=========================================================================
gce_MakeHypr::gce_MakeHypr(const Point3d& S1, const Point3d& S2, const Point3d& Center)
{
  Dir3d        XAxis(gp_XYZ(S1.XYZ() - Center.XYZ()));
  gp_Lin        L(Center, XAxis);
  Standard_Real D = S1.Distance(Center);
  Standard_Real d = L.Distance(S2);
  if (d > D)
  {
    TheError = gce_InvertAxis;
  }
  else
  {
    Dir3d Norm(XAxis.Crossed(Dir3d(gp_XYZ(S2.XYZ() - Center.XYZ()))));
    TheHypr  = gp_Hypr(Frame3d(Center, Norm, XAxis), D, d);
    TheError = gce_Done;
  }
}

gce_MakeHypr::gce_MakeHypr(const Frame3d&       A2,
                           const Standard_Real MajorRadius,
                           const Standard_Real MinorRadius)
{
  if (MajorRadius < MinorRadius)
  {
    TheError = gce_InvertRadius;
  }
  else if (MajorRadius < 0.0)
  {
    TheError = gce_NegativeRadius;
  }
  else
  {
    TheHypr  = gp_Hypr(A2, MajorRadius, MinorRadius);
    TheError = gce_Done;
  }
}

const gp_Hypr& gce_MakeHypr::Value() const
{
  StdFail_NotDone_Raise_if(TheError != gce_Done, "gce_MakeHypr::Value() - no result");
  return TheHypr;
}

const gp_Hypr& gce_MakeHypr::Operator() const
{
  return Value();
}

gce_MakeHypr::operator gp_Hypr() const
{
  return Value();
}
