// Created on: 1993-07-01
// Created by: Bruno DUMORTIER
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

#include <Adaptor3d_Curve.hxx>

#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Lin.hxx>
#include <gp_Parab.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Standard_NotImplemented.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Curve5, RefObject)

//=================================================================================================

Curve5::~Curve5() {}

//=================================================================================================

Handle(Curve5) Curve5::ShallowCopy() const
{
  throw Standard_NotImplemented("Curve5::ShallowCopy");
}

//=================================================================================================

Standard_Real Curve5::FirstParameter() const
{
  throw Standard_NotImplemented("Curve5::FirstParameter");
}

//=================================================================================================

Standard_Real Curve5::LastParameter() const
{
  throw Standard_NotImplemented("Curve5::LastParameter");
}

//=================================================================================================

GeomAbs_Shape Curve5::Continuity() const
{
  throw Standard_NotImplemented("Curve5::Continuity");
}

//=================================================================================================

Standard_Integer Curve5::NbIntervals(const GeomAbs_Shape) const
{
  throw Standard_NotImplemented("Curve5::NbIntervals");
}

//=================================================================================================

void Curve5::Intervals(TColStd_Array1OfReal&, const GeomAbs_Shape) const
{
  throw Standard_NotImplemented("Curve5::Intervals");
}

//=================================================================================================

// Handle(Curve5) Curve5::Trim(const Standard_Real First, const Standard_Real
// Last, const Standard_Real Tol) const
Handle(Curve5) Curve5::Trim(const Standard_Real,
                                              const Standard_Real,
                                              const Standard_Real) const
{
  throw Standard_NotImplemented("Curve5::Trim");
}

//=================================================================================================

Standard_Boolean Curve5::IsClosed() const
{
  throw Standard_NotImplemented("Curve5::IsClosed");
}

//=================================================================================================

Standard_Boolean Curve5::IsPeriodic() const
{
  throw Standard_NotImplemented("Curve5::IsPeriodic");
}

//=================================================================================================

Standard_Real Curve5::Period() const
{
  throw Standard_NotImplemented("Curve5::Period");
}

//=================================================================================================

// Point3d Curve5::Value(const Standard_Real U) const
Point3d Curve5::Value(const Standard_Real) const
{
  throw Standard_NotImplemented("Curve5::Value");
}

//=================================================================================================

// void Curve5::D0(const Standard_Real U, Point3d& P) const
void Curve5::D0(const Standard_Real, Point3d&) const
{
  throw Standard_NotImplemented("Curve5::D0");
}

//=================================================================================================

// void Curve5::D1(const Standard_Real U, Point3d& P, Vector3d& V) const
void Curve5::D1(const Standard_Real, Point3d&, Vector3d&) const
{
  throw Standard_NotImplemented("Curve5::D1");
}

//=================================================================================================

// void Curve5::D2(const Standard_Real U, Point3d& P, Vector3d& V1, Vector3d& V2) const
void Curve5::D2(const Standard_Real, Point3d&, Vector3d&, Vector3d&) const
{
  throw Standard_NotImplemented("Curve5::D2");
}

//=================================================================================================

// void Curve5::D3(const Standard_Real U, Point3d& P, Vector3d& V1, Vector3d& V2, Vector3d& V3)
// const
void Curve5::D3(const Standard_Real, Point3d&, Vector3d&, Vector3d&, Vector3d&) const
{
  throw Standard_NotImplemented("Curve5::D3");
}

//=================================================================================================

// Vector3d Curve5::DN(const Standard_Real U, const Standard_Integer N) const
Vector3d Curve5::DN(const Standard_Real, const Standard_Integer) const
{
  throw Standard_NotImplemented("Curve5::DN");
}

//=================================================================================================

// Standard_Real Curve5::Resolution(const Standard_Real R3d) const
Standard_Real Curve5::Resolution(const Standard_Real) const
{
  throw Standard_NotImplemented("Curve5::Resolution");
}

//=================================================================================================

GeomAbs_CurveType Curve5::GetType() const
{
  throw Standard_NotImplemented("Curve5::GetType");
}

//=================================================================================================

gp_Lin Curve5::Line() const
{
  throw Standard_NotImplemented("Curve5::Line");
}

//=================================================================================================

gp_Circ Curve5::Circle() const
{
  throw Standard_NotImplemented("Curve5::Circle");
}

//=================================================================================================

gp_Elips Curve5::Ellipse() const
{
  throw Standard_NotImplemented("Curve5::Ellipse");
}

//=================================================================================================

gp_Hypr Curve5::Hyperbola() const
{
  throw Standard_NotImplemented("Curve5::Hyperbola");
}

//=================================================================================================

gp_Parab Curve5::Parabola() const
{
  throw Standard_NotImplemented("Curve5::Parabola");
}

//=================================================================================================

Standard_Integer Curve5::Degree() const
{
  throw Standard_NotImplemented("Curve5::Degree");
}

//=================================================================================================

Standard_Boolean Curve5::IsRational() const
{
  throw Standard_NotImplemented("Curve5::IsRational");
}

//=================================================================================================

Standard_Integer Curve5::NbPoles() const
{
  throw Standard_NotImplemented("Curve5::NbPoles");
}

//=================================================================================================

Standard_Integer Curve5::NbKnots() const
{
  throw Standard_NotImplemented("Curve5::NbKnots");
}

//=================================================================================================

Handle(BezierCurve3d) Curve5::Bezier() const
{
  throw Standard_NotImplemented("Curve5::Bezier");
}

//=================================================================================================

Handle(BSplineCurve3d) Curve5::BSpline() const
{
  throw Standard_NotImplemented("Curve5::BSpline");
}

//=================================================================================================

Handle(Geom_OffsetCurve) Curve5::OffsetCurve() const
{
  throw Standard_NotImplemented("Curve5::OffsetCurve");
}
