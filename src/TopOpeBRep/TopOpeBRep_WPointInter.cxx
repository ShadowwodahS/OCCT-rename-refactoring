// Created on: 1993-11-10
// Created by: Jean Yves LEBEY
// Copyright (c) 1993-1999 Matra Datavision
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

#include <gp_Pnt2d.hxx>
#include <IntSurf_PntOn2S.hxx>
#include <TopOpeBRep_WPointInter.hxx>

//=================================================================================================

WPointIntersection::WPointIntersection() {}

//=================================================================================================

void WPointIntersection::Set(const PointOn2Surfaces& P)
{
  myPP2S = (PointOn2Surfaces*)&P;
}

//=================================================================================================

void WPointIntersection::ParametersOnS1(Standard_Real& U1, Standard_Real& V1) const
{
  myPP2S->ParametersOnS1(U1, V1);
}

//=================================================================================================

void WPointIntersection::ParametersOnS2(Standard_Real& U2, Standard_Real& V2) const
{
  myPP2S->ParametersOnS2(U2, V2);
}

//=================================================================================================

void WPointIntersection::Parameters(Standard_Real& U1,
                                        Standard_Real& V1,
                                        Standard_Real& U2,
                                        Standard_Real& V2) const
{
  myPP2S->Parameters(U1, V1, U2, V2);
}

//=================================================================================================

gp_Pnt2d WPointIntersection::ValueOnS1() const
{
  Standard_Real u, v;
  myPP2S->ParametersOnS1(u, v);
  return gp_Pnt2d(u, v);
}

//=================================================================================================

gp_Pnt2d WPointIntersection::ValueOnS2() const
{
  Standard_Real u, v;
  myPP2S->ParametersOnS2(u, v);
  return gp_Pnt2d(u, v);
}

//=================================================================================================

const Point3d& WPointIntersection::Value() const
{
  return myPP2S->Value();
}

TopOpeBRep_PPntOn2S WPointIntersection::PPntOn2SDummy() const
{
  return myPP2S;
}
