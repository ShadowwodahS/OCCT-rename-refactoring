// Created on: 1992-09-01
// Created by: Modelistation
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

#ifndef _GeomAdaptor_Curve_HeaderFile
#define _GeomAdaptor_Curve_HeaderFile

#include <Adaptor3d_Curve.hxx>
#include <BSplCLib_Cache.hxx>
#include <Geom_Curve.hxx>
#include <GeomAbs_Shape.hxx>
#include <GeomEvaluator_Curve.hxx>
#include <Standard_NullObject.hxx>
#include <Standard_ConstructionError.hxx>

DEFINE_STANDARD_HANDLE(GeomAdaptor_Curve, Curve5)

//! This class provides an interface between the services provided by any
//! curve from the package Geom and those required of the curve by algorithms which use it.
//! Creation of the loaded curve the curve is C1 by piece.
//!
//! Polynomial coefficients of BSpline curves used for their evaluation are
//! cached for better performance. Therefore these evaluations are not
//! thread-safe and parallel evaluations need to be prevented.
class GeomAdaptor_Curve : public Curve5
{
  DEFINE_STANDARD_RTTIEXT(GeomAdaptor_Curve, Curve5)
public:
  GeomAdaptor_Curve()
      : myTypeCurve(GeomAbs_OtherCurve),
        myFirst(0.0),
        myLast(0.0)
  {
  }

  GeomAdaptor_Curve(const Handle(GeomCurve3d)& theCurve) { Load(theCurve); }

  //! Standard_ConstructionError is raised if theUFirst>theULast
  GeomAdaptor_Curve(const Handle(GeomCurve3d)& theCurve,
                    const Standard_Real       theUFirst,
                    const Standard_Real       theULast)
  {
    Load(theCurve, theUFirst, theULast);
  }

  //! Shallow copy of adaptor
  Standard_EXPORT virtual Handle(Curve5) ShallowCopy() const Standard_OVERRIDE;

  //! Reset currently loaded curve (undone Load()).
  Standard_EXPORT void Reset();

  void Load(const Handle(GeomCurve3d)& theCurve)
  {
    if (theCurve.IsNull())
    {
      throw Standard_NullObject();
    }
    load(theCurve, theCurve->FirstParameter(), theCurve->LastParameter());
  }

  //! Standard_ConstructionError is raised if theUFirst>theULast
  void Load(const Handle(GeomCurve3d)& theCurve,
            const Standard_Real       theUFirst,
            const Standard_Real       theULast)
  {
    if (theCurve.IsNull())
    {
      throw Standard_NullObject();
    }
    if (theUFirst > theULast)
    {
      throw Standard_ConstructionError();
    }
    load(theCurve, theUFirst, theULast);
  }

  //! Provides a curve inherited from Hcurve from Adaptor.
  //! This is inherited to provide easy to use constructors.
  const Handle(GeomCurve3d)& Curve() const { return myCurve; }

  virtual Standard_Real FirstParameter() const Standard_OVERRIDE { return myFirst; }

  virtual Standard_Real LastParameter() const Standard_OVERRIDE { return myLast; }

  Standard_EXPORT GeomAbs_Shape Continuity() const Standard_OVERRIDE;

  //! Returns  the number  of  intervals for  continuity
  //! <S>. May be one if Continuity(me) >= <S>
  Standard_EXPORT Standard_Integer NbIntervals(const GeomAbs_Shape S) const Standard_OVERRIDE;

  //! Stores in <T> the  parameters bounding the intervals
  //! of continuity <S>.
  //!
  //! The array must provide  enough room to  accommodate
  //! for the parameters. i.e. T.Length() > NbIntervals()
  Standard_EXPORT void Intervals(TColStd_Array1OfReal& T,
                                 const GeomAbs_Shape   S) const Standard_OVERRIDE;

  //! Returns    a  curve equivalent   of  <me>  between
  //! parameters <First>  and <Last>. <Tol>  is used  to
  //! test for 3d points confusion.
  //! If <First> >= <Last>
  Standard_EXPORT Handle(Curve5) Trim(const Standard_Real First,
                                               const Standard_Real Last,
                                               const Standard_Real Tol) const Standard_OVERRIDE;

  Standard_EXPORT Standard_Boolean IsClosed() const Standard_OVERRIDE;

  Standard_EXPORT Standard_Boolean IsPeriodic() const Standard_OVERRIDE;

  Standard_EXPORT Standard_Real Period() const Standard_OVERRIDE;

  //! Computes the point of parameter U on the curve
  Standard_EXPORT Point3d Value(const Standard_Real U) const Standard_OVERRIDE;

  //! Computes the point of parameter U.
  Standard_EXPORT void D0(const Standard_Real U, Point3d& P) const Standard_OVERRIDE;

  //! Computes the point of parameter U on the curve
  //! with its first derivative.
  //!
  //! Warning : On the specific case of BSplineCurve:
  //! if the curve is cut in interval of continuity at least C1, the
  //! derivatives are computed on the current interval.
  //! else the derivatives are computed on the basis curve.
  Standard_EXPORT void D1(const Standard_Real U, Point3d& P, Vector3d& V) const Standard_OVERRIDE;

  //! Returns the point P of parameter U, the first and second
  //! derivatives V1 and V2.
  //!
  //! Warning : On the specific case of BSplineCurve:
  //! if the curve is cut in interval of continuity at least C2, the
  //! derivatives are computed on the current interval.
  //! else the derivatives are computed on the basis curve.
  Standard_EXPORT void D2(const Standard_Real U,
                          Point3d&             P,
                          Vector3d&             V1,
                          Vector3d&             V2) const Standard_OVERRIDE;

  //! Returns the point P of parameter U, the first, the second
  //! and the third derivative.
  //!
  //! Warning : On the specific case of BSplineCurve:
  //! if the curve is cut in interval of continuity at least C3, the
  //! derivatives are computed on the current interval.
  //! else the derivatives are computed on the basis curve.
  Standard_EXPORT void D3(const Standard_Real U,
                          Point3d&             P,
                          Vector3d&             V1,
                          Vector3d&             V2,
                          Vector3d&             V3) const Standard_OVERRIDE;

  //! The returned vector gives the value of the derivative for the
  //! order of derivation N.
  //! Warning : On the specific case of BSplineCurve:
  //! if the curve is cut in interval of continuity CN, the
  //! derivatives are computed on the current interval.
  //! else the derivatives are computed on the basis curve.
  //! Raised if N < 1.
  Standard_EXPORT Vector3d DN(const Standard_Real    U,
                            const Standard_Integer N) const Standard_OVERRIDE;

  //! returns the parametric resolution
  Standard_EXPORT Standard_Real Resolution(const Standard_Real R3d) const Standard_OVERRIDE;

  virtual GeomAbs_CurveType GetType() const Standard_OVERRIDE { return myTypeCurve; }

  Standard_EXPORT gp_Lin Line() const Standard_OVERRIDE;

  Standard_EXPORT gp_Circ Circle() const Standard_OVERRIDE;

  Standard_EXPORT gp_Elips Ellipse() const Standard_OVERRIDE;

  Standard_EXPORT gp_Hypr Hyperbola() const Standard_OVERRIDE;

  Standard_EXPORT gp_Parab Parabola() const Standard_OVERRIDE;

  //! this should NEVER make a copy
  //! of the underlying curve to read
  //! the relevant information
  Standard_EXPORT Standard_Integer Degree() const Standard_OVERRIDE;

  //! this should NEVER make a copy
  //! of the underlying curve to read
  //! the relevant information
  Standard_EXPORT Standard_Boolean IsRational() const Standard_OVERRIDE;

  //! this should NEVER make a copy
  //! of the underlying curve to read
  //! the relevant information
  Standard_EXPORT Standard_Integer NbPoles() const Standard_OVERRIDE;

  //! this should NEVER make a copy
  //! of the underlying curve to read
  //! the relevant information
  Standard_EXPORT Standard_Integer NbKnots() const Standard_OVERRIDE;

  //! this will NOT make a copy of the
  //! Bezier Curve : If you want to modify
  //! the Curve please make a copy yourself
  //! Also it will NOT trim the surface to
  //! myFirst/Last.
  Standard_EXPORT Handle(BezierCurve3d) Bezier() const Standard_OVERRIDE;

  //! this will NOT make a copy of the
  //! BSpline Curve : If you want to modify
  //! the Curve please make a copy yourself
  //! Also it will NOT trim the surface to
  //! myFirst/Last.
  Standard_EXPORT Handle(BSplineCurve3d) BSpline() const Standard_OVERRIDE;

  Standard_EXPORT Handle(Geom_OffsetCurve) OffsetCurve() const Standard_OVERRIDE;

  friend class GeomAdaptor_Surface;

private:
  Standard_EXPORT GeomAbs_Shape LocalContinuity(const Standard_Real U1,
                                                const Standard_Real U2) const;

  Standard_EXPORT void load(const Handle(GeomCurve3d)& C,
                            const Standard_Real       UFirst,
                            const Standard_Real       ULast);

  //! Check theU relates to start or finish point of B-spline curve and return indices of span the
  //! point is located
  Standard_Boolean IsBoundary(const Standard_Real theU,
                              Standard_Integer&   theSpanStart,
                              Standard_Integer&   theSpanFinish) const;

  //! Rebuilds B-spline cache
  //! \param theParameter the value on the knot axis which identifies the caching span
  void RebuildCache(const Standard_Real theParameter) const;

private:
  Handle(GeomCurve3d) myCurve;
  GeomAbs_CurveType  myTypeCurve;
  Standard_Real      myFirst;
  Standard_Real      myLast;

  Handle(BSplineCurve3d)      myBSplineCurve;    ///< B-spline representation to prevent castings
  mutable Handle(BSplCLib_Cache) myCurveCache;      ///< Cached data for B-spline or Bezier curve
  Handle(Curve6)    myNestedEvaluator; ///< Calculates value of offset curve
};

#endif // _GeomAdaptor_Curve_HeaderFile
