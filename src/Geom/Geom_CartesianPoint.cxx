// Created on: 1993-03-10
// Created by: JCV
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

#include <Geom_CartesianPoint.hxx>
#include <Geom_Geometry.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Geom_CartesianPoint, Geom_Point)

typedef Geom_CartesianPoint CartesianPoint;
typedef Axis3d              Ax1;
typedef Frame3d              Ax2;
typedef Vector3d              Vec;
typedef Transform3d             Trsf;

//=================================================================================================

Geom_CartesianPoint::Geom_CartesianPoint(const Point3d& P)
    : gpPnt(P)
{
}

//=================================================================================================

Geom_CartesianPoint::Geom_CartesianPoint(const Standard_Real X,
                                         const Standard_Real Y,
                                         const Standard_Real Z)
    : gpPnt(X, Y, Z)
{
}

//=================================================================================================

Handle(Geometry3) Geom_CartesianPoint::Copy() const
{

  Handle(Geom_CartesianPoint) P;
  P = new CartesianPoint(gpPnt);
  return P;
}

//=================================================================================================

void Geom_CartesianPoint::SetCoord(const Standard_Real X,
                                   const Standard_Real Y,
                                   const Standard_Real Z)
{

  gpPnt.SetCoord(X, Y, Z);
}

//=================================================================================================

void Geom_CartesianPoint::SetPnt(const Point3d& P)
{
  gpPnt = P;
}

//=================================================================================================

void Geom_CartesianPoint::SetX(const Standard_Real X)
{
  gpPnt.SetX(X);
}

//=================================================================================================

void Geom_CartesianPoint::SetY(const Standard_Real Y)
{
  gpPnt.SetY(Y);
}

//=================================================================================================

void Geom_CartesianPoint::SetZ(const Standard_Real Z)
{
  gpPnt.SetZ(Z);
}

//=================================================================================================

void Geom_CartesianPoint::Coord(Standard_Real& X, Standard_Real& Y, Standard_Real& Z) const
{

  gpPnt.Coord(X, Y, Z);
}

//=================================================================================================

Point3d Geom_CartesianPoint::Pnt() const
{
  return gpPnt;
}

//=================================================================================================

Standard_Real Geom_CartesianPoint::X() const
{
  return gpPnt.X();
}

//=================================================================================================

Standard_Real Geom_CartesianPoint::Y() const
{
  return gpPnt.Y();
}

//=================================================================================================

Standard_Real Geom_CartesianPoint::Z() const
{
  return gpPnt.Z();
}

//=================================================================================================

void Geom_CartesianPoint::Transform(const Trsf& T)
{
  gpPnt.Transform(T);
}
