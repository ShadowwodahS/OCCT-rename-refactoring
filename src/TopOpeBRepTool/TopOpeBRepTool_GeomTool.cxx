// Created on: 1993-06-24
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

#include <Precision.hxx>
#include <TopOpeBRepTool_GeomTool.hxx>

//=================================================================================================

GeomTool1::GeomTool1(const TopOpeBRepTool_OutCurveType TypeC3D,
                                                 const Standard_Boolean            CompC3D,
                                                 const Standard_Boolean            CompPC1,
                                                 const Standard_Boolean            CompPC2)
    : myTypeC3D(TypeC3D),
      myCompC3D(CompC3D),
      myCompPC1(CompPC1),
      myCompPC2(CompPC2),
      myTol3d(Precision1::Approximation()),
      myTol2d(Precision1::PApproximation()),
      myNbPntMax(30)
{
}

//=================================================================================================

void GeomTool1::Define(const TopOpeBRepTool_OutCurveType TypeC3D,
                                     const Standard_Boolean            CompC3D,
                                     const Standard_Boolean            CompPC1,
                                     const Standard_Boolean            CompPC2)
{
  myTypeC3D = TypeC3D;
  myCompC3D = CompC3D;
  myCompPC1 = CompPC1;
  myCompPC2 = CompPC2;
}

//=================================================================================================

void GeomTool1::Define(const TopOpeBRepTool_OutCurveType TypeC3D)
{
  myTypeC3D = TypeC3D;
}

//=================================================================================================

void GeomTool1::DefineCurves(const Standard_Boolean CompC3D)
{
  myCompC3D = CompC3D;
}

//=================================================================================================

void GeomTool1::DefinePCurves1(const Standard_Boolean CompPC1)
{
  myCompPC1 = CompPC1;
}

//=================================================================================================

void GeomTool1::DefinePCurves2(const Standard_Boolean CompPC2)
{
  myCompPC2 = CompPC2;
}

//=================================================================================================

void GeomTool1::Define(const GeomTool1& GT)
{
  *this = GT;
}

//=================================================================================================

void GeomTool1::GetTolerances(Standard_Real& tol3d, Standard_Real& tol2d) const
{
  tol3d = myTol3d;
  tol2d = myTol2d;
}

//=================================================================================================

void GeomTool1::SetTolerances(const Standard_Real tol3d, const Standard_Real tol2d)
{
  myTol3d = tol3d;
  myTol2d = tol2d;
}

//=================================================================================================

Standard_Integer GeomTool1::NbPntMax() const
{
  return myNbPntMax;
}

//=================================================================================================

void GeomTool1::SetNbPntMax(const Standard_Integer NbPntMax)
{
  myNbPntMax = NbPntMax;
}

//=================================================================================================

Standard_Boolean GeomTool1::CompC3D() const
{
  return myCompC3D;
}

//=================================================================================================

TopOpeBRepTool_OutCurveType GeomTool1::TypeC3D() const
{
  return myTypeC3D;
}

//=================================================================================================

Standard_Boolean GeomTool1::CompPC1() const
{
  return myCompPC1;
}

//=================================================================================================

Standard_Boolean GeomTool1::CompPC2() const
{
  return myCompPC2;
}
