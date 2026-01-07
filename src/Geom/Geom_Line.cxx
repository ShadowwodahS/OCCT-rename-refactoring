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
#include <Geom_Line.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <Precision.hxx>
#include <Standard_RangeError.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomLine, GeomCurve3d)

//=================================================================================================

Handle(Geom_Geometry) GeomLine::Copy() const
{

  Handle(GeomLine) L;
  L = new GeomLine(pos);
  return L;
}

//=================================================================================================

GeomLine::GeomLine(const Axis3d& A)
    : pos(A)
{
}

//=================================================================================================

GeomLine::GeomLine(const gp_Lin& L)
    : pos(L.Position())
{
}

//=================================================================================================

GeomLine::GeomLine(const Point3d& P, const Dir3d& V)
    : pos(P, V)
{
}

//=================================================================================================

void GeomLine::Reverse()
{
  pos.Reverse();
}

//=================================================================================================

Standard_Real GeomLine::ReversedParameter(const Standard_Real U) const
{
  return (-U);
}

//=================================================================================================

void GeomLine::SetDirection(const Dir3d& V)
{
  pos.SetDirection(V);
}

//=================================================================================================

void GeomLine::SetLin(const gp_Lin& L)
{
  pos = L.Position();
}

//=================================================================================================

void GeomLine::SetLocation(const Point3d& P)
{
  pos.SetLocation(P);
}

//=================================================================================================

void GeomLine::SetPosition(const Axis3d& A1)
{
  pos = A1;
}

//=================================================================================================

const Axis3d& GeomLine::Position() const
{
  return pos;
}

//=================================================================================================

Standard_Boolean GeomLine::IsClosed() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean GeomLine::IsPeriodic() const
{
  return Standard_False;
}

//=================================================================================================

GeomAbs_Shape GeomLine::Continuity() const
{
  return GeomAbs_CN;
}

//=================================================================================================

Standard_Real GeomLine::FirstParameter() const
{
  return -Precision::Infinite();
}

//=================================================================================================

Standard_Real GeomLine::LastParameter() const
{
  return Precision::Infinite();
}

//=================================================================================================

gp_Lin GeomLine::Lin() const
{
  return gp_Lin(pos);
}

//=================================================================================================

Standard_Boolean GeomLine::IsCN(const Standard_Integer) const
{
  return Standard_True;
}

//=================================================================================================

void GeomLine::Transform(const Transform3d& T)
{
  pos.Transform(T);
}

//=================================================================================================

void GeomLine::D0(const Standard_Real U, Point3d& P) const
{
  P = ElCLib1::LineValue(U, pos);
}

//=================================================================================================

void GeomLine::D1(const Standard_Real U, Point3d& P, Vector3d& V1) const
{

  ElCLib1::LineD1(U, pos, P, V1);
}

//=================================================================================================

void GeomLine::D2(const Standard_Real U, Point3d& P, Vector3d& V1, Vector3d& V2) const
{

  ElCLib1::LineD1(U, pos, P, V1);
  V2.SetCoord(0.0, 0.0, 0.0);
}

//=================================================================================================

void GeomLine::D3(const Standard_Real U, Point3d& P, Vector3d& V1, Vector3d& V2, Vector3d& V3) const
{

  ElCLib1::LineD1(U, pos, P, V1);
  V2.SetCoord(0.0, 0.0, 0.0);
  V3.SetCoord(0.0, 0.0, 0.0);
}

//=================================================================================================

Vector3d GeomLine::DN(const Standard_Real, const Standard_Integer N) const
{

  Standard_RangeError_Raise_if(N <= 0, " ");
  if (N == 1)
    return Vector3d(pos.Direction());
  else
    return Vector3d(0.0, 0.0, 0.0);
}

//=================================================================================================

Standard_Real GeomLine::TransformedParameter(const Standard_Real U, const Transform3d& T) const
{
  if (Precision::IsInfinite(U))
    return U;
  return U * Abs(T.ScaleFactor());
}

//=================================================================================================

Standard_Real GeomLine::ParametricTransformation(const Transform3d& T) const
{
  return Abs(T.ScaleFactor());
}

//=================================================================================================

void GeomLine::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)
  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, GeomCurve3d)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &pos)
}
