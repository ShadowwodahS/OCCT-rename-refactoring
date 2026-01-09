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

#include <Adaptor3d_Surface.hxx>

#include <Adaptor3d_Curve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <gp_Ax1.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_Vec.hxx>
#include <Standard_NotImplemented.hxx>

IMPLEMENT_STANDARD_RTTIEXT(SurfaceAdaptor, RefObject)

//=================================================================================================

SurfaceAdaptor::~SurfaceAdaptor() {}

//=================================================================================================

Handle(SurfaceAdaptor) SurfaceAdaptor::ShallowCopy() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::ShallowCopy");
}

//=================================================================================================

Standard_Real SurfaceAdaptor::FirstUParameter() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::FirstUParameter");
}

//=================================================================================================

Standard_Real SurfaceAdaptor::LastUParameter() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::LastUParameter");
}

//=================================================================================================

Standard_Real SurfaceAdaptor::FirstVParameter() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::FirstVParameter");
}

//=================================================================================================

Standard_Real SurfaceAdaptor::LastVParameter() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::LastVParameter");
}

//=================================================================================================

GeomAbs_Shape SurfaceAdaptor::UContinuity() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::UContinuity");
}

//=================================================================================================

GeomAbs_Shape SurfaceAdaptor::VContinuity() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::VContinuity");
}

//=================================================================================================

// Standard_Integer SurfaceAdaptor::NbUIntervals(const GeomAbs_Shape S) const
Standard_Integer SurfaceAdaptor::NbUIntervals(const GeomAbs_Shape) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::NbUIntervals");
}

//=================================================================================================

// Standard_Integer SurfaceAdaptor::NbVIntervals(const GeomAbs_Shape S) const
Standard_Integer SurfaceAdaptor::NbVIntervals(const GeomAbs_Shape) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::NbVIntervals");
}

//=================================================================================================

// void SurfaceAdaptor::UIntervals(TColStd_Array1OfReal& T, const GeomAbs_Shape S) const
void SurfaceAdaptor::UIntervals(TColStd_Array1OfReal&, const GeomAbs_Shape) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::UIntervals");
}

//=================================================================================================

// void SurfaceAdaptor::VIntervals(TColStd_Array1OfReal& T, const GeomAbs_Shape S) const
void SurfaceAdaptor::VIntervals(TColStd_Array1OfReal&, const GeomAbs_Shape) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::VIntervals");
}

//=================================================================================================

// Handle(SurfaceAdaptor) SurfaceAdaptor::UTrim(const Standard_Real First, const Standard_Real
// Last, const Standard_Real Tol) const
Handle(SurfaceAdaptor) SurfaceAdaptor::UTrim(const Standard_Real,
                                                   const Standard_Real,
                                                   const Standard_Real) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::UTrim");
}

//=================================================================================================

// Handle(SurfaceAdaptor) SurfaceAdaptor::VTrim(const Standard_Real First, const Standard_Real
// Last, const Standard_Real Tol) const
Handle(SurfaceAdaptor) SurfaceAdaptor::VTrim(const Standard_Real,
                                                   const Standard_Real,
                                                   const Standard_Real) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::VTrim");
}

//=================================================================================================

Standard_Boolean SurfaceAdaptor::IsUClosed() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::IsUClosed");
}

//=================================================================================================

Standard_Boolean SurfaceAdaptor::IsVClosed() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::IsVClosed");
}

//=================================================================================================

Standard_Boolean SurfaceAdaptor::IsUPeriodic() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::IsUPeriodic");
}

//=================================================================================================

Standard_Real SurfaceAdaptor::UPeriod() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::UPeriod");
}

//=================================================================================================

Standard_Boolean SurfaceAdaptor::IsVPeriodic() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::IsVPeriodic");
}

//=================================================================================================

Standard_Real SurfaceAdaptor::VPeriod() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::VPeriod");
}

//=================================================================================================

// Point3d SurfaceAdaptor::Value(const Standard_Real U, const Standard_Real V) const
Point3d SurfaceAdaptor::Value(const Standard_Real, const Standard_Real) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Value");
}

//=================================================================================================

// void SurfaceAdaptor::D0(const Standard_Real U, const Standard_Real V, Point3d& P) const
void SurfaceAdaptor::D0(const Standard_Real, const Standard_Real, Point3d&) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::D0");
}

//=================================================================================================

// void SurfaceAdaptor::D1(const Standard_Real U, const Standard_Real V, Point3d& P, Vector3d& D1U,
// Vector3d& D1V) const
void SurfaceAdaptor::D1(const Standard_Real,
                           const Standard_Real,
                           Point3d&,
                           Vector3d&,
                           Vector3d&) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::D1");
}

//=================================================================================================

// void SurfaceAdaptor::D2(const Standard_Real U, const Standard_Real V, Point3d& P, Vector3d& D1U,
// Vector3d& D1V, Vector3d& D2U, Vector3d& D2V, Vector3d& D2UV) const
void SurfaceAdaptor::D2(const Standard_Real,
                           const Standard_Real,
                           Point3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::D2");
}

//=================================================================================================

// void SurfaceAdaptor::D3(const Standard_Real U, const Standard_Real V, Point3d& P, Vector3d& D1U,
// Vector3d& D1V, Vector3d& D2U, Vector3d& D2V, Vector3d& D2UV, Vector3d& D3U, Vector3d& D3V, Vector3d& D3UUV,
// Vector3d& D3UVV) const
void SurfaceAdaptor::D3(const Standard_Real,
                           const Standard_Real,
                           Point3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::D3");
}

//=================================================================================================

// Vector3d SurfaceAdaptor::DN(const Standard_Real U, const Standard_Real V, const Standard_Integer
// Nu, const Standard_Integer Nv) const
Vector3d SurfaceAdaptor::DN(const Standard_Real,
                             const Standard_Real,
                             const Standard_Integer,
                             const Standard_Integer) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::DN");
}

//=================================================================================================

// Standard_Real SurfaceAdaptor::UResolution(const Standard_Real R3d) const
Standard_Real SurfaceAdaptor::UResolution(const Standard_Real) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::UResolution");
}

//=================================================================================================

// Standard_Real SurfaceAdaptor::VResolution(const Standard_Real R3d) const
Standard_Real SurfaceAdaptor::VResolution(const Standard_Real) const
{
  throw Standard_NotImplemented("SurfaceAdaptor::VResolution");
}

//=================================================================================================

GeomAbs_SurfaceType SurfaceAdaptor::GetType() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::GetType");
}

//=================================================================================================

gp_Pln SurfaceAdaptor::Plane1() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Plane1");
}

//=================================================================================================

Cylinder1 SurfaceAdaptor::Cylinder() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Cylinder");
}

//=================================================================================================

Cone1 SurfaceAdaptor::Cone() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Cone");
}

//=================================================================================================

Sphere3 SurfaceAdaptor::Sphere() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Sphere");
}

//=================================================================================================

gp_Torus SurfaceAdaptor::Torus() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Torus");
}

//=================================================================================================

Standard_Integer SurfaceAdaptor::UDegree() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::UDegree");
}

//=================================================================================================

Standard_Integer SurfaceAdaptor::NbUPoles() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::NbUPoles");
}

//=================================================================================================

Standard_Integer SurfaceAdaptor::VDegree() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::VDegree");
}

//=================================================================================================

Standard_Integer SurfaceAdaptor::NbVPoles() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::NbVPoles");
}

//=================================================================================================

Standard_Integer SurfaceAdaptor::NbUKnots() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::NbUKnots");
}

//=================================================================================================

Standard_Integer SurfaceAdaptor::NbVKnots() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::NbVKnots");
}

//=================================================================================================

Standard_Boolean SurfaceAdaptor::IsURational() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::IsURational");
}

//=================================================================================================

Standard_Boolean SurfaceAdaptor::IsVRational() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::IsVRational");
}

//=================================================================================================

Handle(Geom_BezierSurface) SurfaceAdaptor::Bezier() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Bezier");
}

//=================================================================================================

Handle(Geom_BSplineSurface) SurfaceAdaptor::BSpline() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::BSpline");
}

//=================================================================================================

Axis3d SurfaceAdaptor::AxeOfRevolution() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::AxeOfRevolution");
}

//=================================================================================================

Dir3d SurfaceAdaptor::Direction() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::Direction");
}

//=================================================================================================

Handle(Curve5) SurfaceAdaptor::BasisCurve() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::BasisCurve");
}

//=================================================================================================

Handle(SurfaceAdaptor) SurfaceAdaptor::BasisSurface() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::BasisSurface");
}

//=================================================================================================

Standard_Real SurfaceAdaptor::OffsetValue() const
{
  throw Standard_NotImplemented("SurfaceAdaptor::OffsetValue");
}
