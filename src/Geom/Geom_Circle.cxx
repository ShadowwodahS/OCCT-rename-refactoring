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

#include <ElCLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Geometry.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_RangeError.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomCircle, Geom_Conic)

typedef GeomCircle Circle;
typedef Frame3d      Ax2;
typedef Point3d      Pnt;
typedef Transform3d     Trsf;
typedef Vector3d      Vec;
typedef Coords3d      XYZ;

//=================================================================================================

Handle(Geometry3) GeomCircle::Copy() const
{

  Handle(GeomCircle) C;
  C = new Circle(pos, radius);
  return C;
}

//=================================================================================================

GeomCircle::GeomCircle(const gp_Circ& C)
    : radius(C.Radius())
{

  pos = C.Position1();
}

//=================================================================================================

GeomCircle::GeomCircle(const Ax2& A2, const Standard_Real R)
    : radius(R)
{

  if (R < 0.0)
    throw Standard_ConstructionError();
  pos = A2;
}

//=================================================================================================

Standard_Boolean GeomCircle::IsClosed() const
{
  return Standard_True;
}

//=================================================================================================

Standard_Boolean GeomCircle::IsPeriodic() const
{
  return Standard_True;
}

//=================================================================================================

Standard_Real GeomCircle::ReversedParameter(const Standard_Real U) const
{
  return (2. * M_PI - U);
}

//=================================================================================================

Standard_Real GeomCircle::Eccentricity() const
{
  return 0.0;
}

//=================================================================================================

Standard_Real GeomCircle::FirstParameter() const
{
  return 0.0;
}

//=================================================================================================

Standard_Real GeomCircle::LastParameter() const
{
  return 2.0 * M_PI;
}

//=================================================================================================

gp_Circ GeomCircle::Circ() const
{
  return gp_Circ(pos, radius);
}

//=================================================================================================

void GeomCircle::SetCirc(const gp_Circ& C)
{

  radius = C.Radius();
  pos    = C.Position1();
}

//=================================================================================================

void GeomCircle::SetRadius(const Standard_Real R)
{

  if (R < 0.0)
    throw Standard_ConstructionError();
  radius = R;
}

//=================================================================================================

Standard_Real GeomCircle::Radius() const
{
  return radius;
}

//=================================================================================================

void GeomCircle::D0(const Standard_Real U, Pnt& P) const
{

  P = ElCLib1::CircleValue(U, pos, radius);
}

//=================================================================================================

void GeomCircle::D1(const Standard_Real U, Pnt& P, Vec& V1) const
{

  ElCLib1::CircleD1(U, pos, radius, P, V1);
}

//=================================================================================================

void GeomCircle::D2(const Standard_Real U, Pnt& P, Vec& V1, Vec& V2) const
{

  ElCLib1::CircleD2(U, pos, radius, P, V1, V2);
}

//=================================================================================================

void GeomCircle::D3(const Standard_Real U, Pnt& P, Vec& V1, Vec& V2, Vec& V3) const
{

  ElCLib1::CircleD3(U, pos, radius, P, V1, V2, V3);
}

//=================================================================================================

Vec GeomCircle::DN(const Standard_Real U, const Standard_Integer N) const
{

  Standard_RangeError_Raise_if(N < 1, " ");
  return ElCLib1::CircleDN(U, pos, radius, N);
}

//=================================================================================================

void GeomCircle::Transform(const Trsf& T)
{

  radius = radius * Abs(T.ScaleFactor());
  pos.Transform(T);
}

//=================================================================================================

void GeomCircle::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, Geom_Conic)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, radius)
}
