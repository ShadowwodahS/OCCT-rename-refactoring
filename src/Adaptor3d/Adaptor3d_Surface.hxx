// Created on: 1993-03-31
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

#ifndef _Adaptor3d_Surface_HeaderFile
#define _Adaptor3d_Surface_HeaderFile

#include <Adaptor3d_Curve.hxx>
#include <GeomAbs_Shape.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <gp_Ax1.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>
#include <TColStd_Array1OfReal.hxx>

class Geom_BezierSurface;
class Geom_BSplineSurface;

DEFINE_STANDARD_HANDLE(SurfaceAdaptor, RefObject)

//! Root class for surfaces on which geometric algorithms work.
//! An adapted surface is an interface between the
//! services provided by a surface and those required of
//! the surface by algorithms which use it.
//! A derived concrete class is provided:
//! GeomAdaptor_Surface for a surface from the Geom package.
//! The  Surface class describes  the standard behaviour
//! of a surface for generic algorithms.
//!
//! The Surface can  be decomposed in intervals of any
//! continuity in U and V using the method NbIntervals.
//! A current interval can be set.
//! Most of the methods apply to the current interval.
//! Warning: All the methods are virtual and implemented with a
//! raise to allow to redefined only the methods really used.
//!
//! Polynomial coefficients of BSpline surfaces used for their evaluation are cached for better
//! performance. Therefore these evaluations are not thread-safe and parallel evaluations need to be
//! prevented.
class SurfaceAdaptor : public RefObject
{
  DEFINE_STANDARD_RTTIEXT(SurfaceAdaptor, RefObject)
public:
  //! Shallow copy of adaptor
  Standard_EXPORT virtual Handle(SurfaceAdaptor) ShallowCopy() const;

  Standard_EXPORT virtual Standard_Real FirstUParameter() const;

  Standard_EXPORT virtual Standard_Real LastUParameter() const;

  Standard_EXPORT virtual Standard_Real FirstVParameter() const;

  Standard_EXPORT virtual Standard_Real LastVParameter() const;

  Standard_EXPORT virtual GeomAbs_Shape UContinuity() const;

  Standard_EXPORT virtual GeomAbs_Shape VContinuity() const;

  //! Returns the number of U intervals for  continuity
  //! <S>. May be one if UContinuity(me) >= <S>
  Standard_EXPORT virtual Standard_Integer NbUIntervals(const GeomAbs_Shape S) const;

  //! Returns the number of V intervals for  continuity
  //! <S>. May be one if VContinuity(me) >= <S>
  Standard_EXPORT virtual Standard_Integer NbVIntervals(const GeomAbs_Shape S) const;

  //! Returns the  intervals with the requested continuity
  //! in the U direction.
  Standard_EXPORT virtual void UIntervals(TColStd_Array1OfReal& T, const GeomAbs_Shape S) const;

  //! Returns the  intervals with the requested continuity
  //! in the V direction.
  Standard_EXPORT virtual void VIntervals(TColStd_Array1OfReal& T, const GeomAbs_Shape S) const;

  //! Returns    a  surface trimmed in the U direction
  //! equivalent   of  <me>  between
  //! parameters <First>  and <Last>. <Tol>  is used  to
  //! test for 3d points confusion.
  //! If <First> >= <Last>
  Standard_EXPORT virtual Handle(SurfaceAdaptor) UTrim(const Standard_Real First,
                                                          const Standard_Real Last,
                                                          const Standard_Real Tol) const;

  //! Returns    a  surface trimmed in the V direction  between
  //! parameters <First>  and <Last>. <Tol>  is used  to
  //! test for 3d points confusion.
  //! If <First> >= <Last>
  Standard_EXPORT virtual Handle(SurfaceAdaptor) VTrim(const Standard_Real First,
                                                          const Standard_Real Last,
                                                          const Standard_Real Tol) const;

  Standard_EXPORT virtual Standard_Boolean IsUClosed() const;

  Standard_EXPORT virtual Standard_Boolean IsVClosed() const;

  Standard_EXPORT virtual Standard_Boolean IsUPeriodic() const;

  Standard_EXPORT virtual Standard_Real UPeriod() const;

  Standard_EXPORT virtual Standard_Boolean IsVPeriodic() const;

  Standard_EXPORT virtual Standard_Real VPeriod() const;

  //! Computes the point of parameters U,V on the surface.
  //! Tip: use GeomLib1::NormEstim() to calculate surface normal at specified (U, V) point.
  Standard_EXPORT virtual Point3d Value(const Standard_Real U, const Standard_Real V) const;

  //! Computes the point of parameters U,V on the surface.
  Standard_EXPORT virtual void D0(const Standard_Real U, const Standard_Real V, Point3d& P) const;

  //! Computes the point  and the first derivatives on the surface.
  //! Raised if the continuity of the current intervals is not C1.
  //!
  //! Tip: use GeomLib1::NormEstim() to calculate surface normal at specified (U, V) point.
  Standard_EXPORT virtual void D1(const Standard_Real U,
                                  const Standard_Real V,
                                  Point3d&             P,
                                  Vector3d&             D1U,
                                  Vector3d&             D1V) const;

  //! Computes   the point,  the  first  and  second
  //! derivatives on the surface.
  //! Raised  if   the   continuity   of the current
  //! intervals is not C2.
  Standard_EXPORT virtual void D2(const Standard_Real U,
                                  const Standard_Real V,
                                  Point3d&             P,
                                  Vector3d&             D1U,
                                  Vector3d&             D1V,
                                  Vector3d&             D2U,
                                  Vector3d&             D2V,
                                  Vector3d&             D2UV) const;

  //! Computes the point,  the first, second and third
  //! derivatives on the surface.
  //! Raised  if   the   continuity   of the current
  //! intervals is not C3.
  Standard_EXPORT virtual void D3(const Standard_Real U,
                                  const Standard_Real V,
                                  Point3d&             P,
                                  Vector3d&             D1U,
                                  Vector3d&             D1V,
                                  Vector3d&             D2U,
                                  Vector3d&             D2V,
                                  Vector3d&             D2UV,
                                  Vector3d&             D3U,
                                  Vector3d&             D3V,
                                  Vector3d&             D3UUV,
                                  Vector3d&             D3UVV) const;

  //! Computes the derivative of order Nu in the direction U and Nv
  //! in the direction V at the point P(U, V).
  //! Raised if the current U  interval is not not CNu
  //! and the current V interval is not CNv.
  //! Raised if Nu + Nv < 1 or Nu < 0 or Nv < 0.
  Standard_EXPORT virtual Vector3d DN(const Standard_Real    U,
                                    const Standard_Real    V,
                                    const Standard_Integer Nu,
                                    const Standard_Integer Nv) const;

  //! Returns the parametric U  resolution corresponding
  //! to the real space resolution <R3d>.
  Standard_EXPORT virtual Standard_Real UResolution(const Standard_Real R3d) const;

  //! Returns the parametric V  resolution corresponding
  //! to the real space resolution <R3d>.
  Standard_EXPORT virtual Standard_Real VResolution(const Standard_Real R3d) const;

  //! Returns the type of the surface : Plane1, Cylinder,
  //! Cone,      Sphere,        Torus,    BezierSurface,
  //! BSplineSurface,               SurfaceOfRevolution,
  //! SurfaceOfExtrusion, OtherSurface
  Standard_EXPORT virtual GeomAbs_SurfaceType GetType() const;

  Standard_EXPORT virtual gp_Pln Plane1() const;

  Standard_EXPORT virtual Cylinder1 Cylinder() const;

  Standard_EXPORT virtual Cone1 Cone() const;

  Standard_EXPORT virtual Sphere3 Sphere() const;

  Standard_EXPORT virtual gp_Torus Torus() const;

  Standard_EXPORT virtual Standard_Integer UDegree() const;

  Standard_EXPORT virtual Standard_Integer NbUPoles() const;

  Standard_EXPORT virtual Standard_Integer VDegree() const;

  Standard_EXPORT virtual Standard_Integer NbVPoles() const;

  Standard_EXPORT virtual Standard_Integer NbUKnots() const;

  Standard_EXPORT virtual Standard_Integer NbVKnots() const;

  Standard_EXPORT virtual Standard_Boolean IsURational() const;

  Standard_EXPORT virtual Standard_Boolean IsVRational() const;

  Standard_EXPORT virtual Handle(Geom_BezierSurface) Bezier() const;

  Standard_EXPORT virtual Handle(Geom_BSplineSurface) BSpline() const;

  Standard_EXPORT virtual Axis3d AxeOfRevolution() const;

  Standard_EXPORT virtual Dir3d Direction() const;

  Standard_EXPORT virtual Handle(Curve5) BasisCurve() const;

  Standard_EXPORT virtual Handle(SurfaceAdaptor) BasisSurface() const;

  Standard_EXPORT virtual Standard_Real OffsetValue() const;
  Standard_EXPORT virtual ~SurfaceAdaptor();
};

#endif // _Adaptor3d_Surface_HeaderFile
