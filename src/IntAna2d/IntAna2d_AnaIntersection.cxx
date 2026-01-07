// Copyright (c) 1995-1999 Matra Datavision
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

#include <gp_Circ2d.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Hypr2d.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Parab2d.hxx>
#include <IntAna2d_AnaIntersection.hxx>
#include <IntAna2d_Conic.hxx>
#include <StdFail_NotDone.hxx>

AnalyticIntersection2d::AnalyticIntersection2d()
    : done(Standard_False),
      para(Standard_False),
      iden(Standard_False),
      empt(Standard_True),
      nbp(0)
{
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Lin2d& L1, const gp_Lin2d& L2)
{
  Perform(L1, L2);
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Circ2d& C1, const gp_Circ2d& C2)
{
  Perform(C1, C2);
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Lin2d& L, const gp_Circ2d& C)
{
  Perform(L, C);
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Lin2d& L, const Conic2d& Conic)
{
  Perform(L, Conic);
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Parab2d& P, const Conic2d& Conic)
{
  Perform(P, Conic);
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Circ2d& C, const Conic2d& Conic)
{
  Perform(C, Conic);
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Elips2d& E, const Conic2d& Conic)
{
  Perform(E, Conic);
}

AnalyticIntersection2d::AnalyticIntersection2d(const gp_Hypr2d& E, const Conic2d& Conic)
{
  Perform(E, Conic);
}
