// Created on: 1995-07-18
// Created by: Modelistation
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

#ifndef _Extrema_CurveTool_HeaderFile
#define _Extrema_CurveTool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <GeomAbs_Shape.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <GeomAbs_CurveType.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Parab.hxx>
class Curve5;
class Point3d;
class Vector3d;
class BezierCurve3d;
class BSplineCurve3d;

class CurveTool4
{
public:
  DEFINE_STANDARD_ALLOC

  static Standard_Real FirstParameter(const Curve5& C);

  static Standard_Real LastParameter(const Curve5& C);

  static GeomAbs_Shape Continuity(const Curve5& C);

  //! Returns  the number  of  intervals for  continuity
  //! <S>. May be one if Continuity(me) >= <S>
  static Standard_Integer NbIntervals(Curve5& C, const GeomAbs_Shape S);

  //! Stores in <T> the  parameters bounding the intervals
  //! of continuity <S>.
  //!
  //! The array must provide  enough room to  accommodate
  //! for the parameters. i.e. T.Length() > NbIntervals()
  static void Intervals(Curve5& C, TColStd_Array1OfReal& T, const GeomAbs_Shape S);

  //! Returns the parameters bounding the intervals of subdivision of curve
  //! according to Curvature deflection. Value of deflection is defined in method.
  //!
  Standard_EXPORT static Handle(TColStd_HArray1OfReal) DeflCurvIntervals(const Curve5& C);

  Standard_EXPORT static Standard_Boolean IsPeriodic(const Curve5& C);

  static Standard_Real Period(const Curve5& C);

  static Standard_Real Resolution(const Curve5& C, const Standard_Real R3d);

  static GeomAbs_CurveType GetType(const Curve5& C);

  static Point3d Value(const Curve5& C, const Standard_Real U);

  static void D0(const Curve5& C, const Standard_Real U, Point3d& P);

  static void D1(const Curve5& C, const Standard_Real U, Point3d& P, Vector3d& V);

  static void D2(const Curve5& C,
                 const Standard_Real    U,
                 Point3d&                P,
                 Vector3d&                V1,
                 Vector3d&                V2);

  static void D3(const Curve5& C,
                 const Standard_Real    U,
                 Point3d&                P,
                 Vector3d&                V1,
                 Vector3d&                V2,
                 Vector3d&                V3);

  static Vector3d DN(const Curve5& C, const Standard_Real U, const Standard_Integer N);

  static gp_Lin Line(const Curve5& C);

  static gp_Circ Circle(const Curve5& C);

  static gp_Elips Ellipse(const Curve5& C);

  static gp_Hypr Hyperbola(const Curve5& C);

  static gp_Parab Parabola(const Curve5& C);

  static Standard_Integer Degree(const Curve5& C);

  static Standard_Boolean IsRational(const Curve5& C);

  static Standard_Integer NbPoles(const Curve5& C);

  static Standard_Integer NbKnots(const Curve5& C);

  static Handle(BezierCurve3d) Bezier(const Curve5& C);

  static Handle(BSplineCurve3d) BSpline(const Curve5& C);

protected:
private:
};

#include <Extrema_CurveTool.lxx>

#endif // _Extrema_CurveTool_HeaderFile
