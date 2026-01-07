// Created on: 1993-09-28
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

#ifndef _GeomFill_HeaderFile
#define _GeomFill_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Convert_ParameterisationType.hxx>
#include <Standard_Real.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <Standard_Boolean.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_Array1OfInteger.hxx>
class GeomSurface;
class GeomCurve3d;
class Vector3d;
class Point3d;

//! Tools and Data to filling Surface and Sweep Surfaces
class GeomFill1
{
public:
  DEFINE_STANDARD_ALLOC

  //! Builds a ruled surface between the two curves, Curve1 and Curve2.
  Standard_EXPORT static Handle(GeomSurface) Surface(const Handle(GeomCurve3d)& Curve1,
                                                      const Handle(GeomCurve3d)& Curve2);

  Standard_EXPORT static void GetCircle(const Convert_ParameterisationType TConv,
                                        const Vector3d&                      ns1,
                                        const Vector3d&                      ns2,
                                        const Vector3d&                      nplan,
                                        const Point3d&                      pt1,
                                        const Point3d&                      pt2,
                                        const Standard_Real                Rayon,
                                        const Point3d&                      Center,
                                        TColgp_Array1OfPnt&                Poles,
                                        TColStd_Array1OfReal&              Weigths);

  Standard_EXPORT static Standard_Boolean GetCircle(const Convert_ParameterisationType TConv,
                                                    const Vector3d&                      ns1,
                                                    const Vector3d&                      ns2,
                                                    const Vector3d&                      dn1w,
                                                    const Vector3d&                      dn2w,
                                                    const Vector3d&                      nplan,
                                                    const Vector3d&                      dnplan,
                                                    const Point3d&                      pts1,
                                                    const Point3d&                      pts2,
                                                    const Vector3d&                      tang1,
                                                    const Vector3d&                      tang2,
                                                    const Standard_Real                Rayon,
                                                    const Standard_Real                DRayon,
                                                    const Point3d&                      Center,
                                                    const Vector3d&                      DCenter,
                                                    TColgp_Array1OfPnt&                Poles,
                                                    TColgp_Array1OfVec&                DPoles,
                                                    TColStd_Array1OfReal&              Weigths,
                                                    TColStd_Array1OfReal&              DWeigths);

  Standard_EXPORT static Standard_Boolean GetCircle(const Convert_ParameterisationType TConv,
                                                    const Vector3d&                      ns1,
                                                    const Vector3d&                      ns2,
                                                    const Vector3d&                      dn1w,
                                                    const Vector3d&                      dn2w,
                                                    const Vector3d&                      d2n1w,
                                                    const Vector3d&                      d2n2w,
                                                    const Vector3d&                      nplan,
                                                    const Vector3d&                      dnplan,
                                                    const Vector3d&                      d2nplan,
                                                    const Point3d&                      pts1,
                                                    const Point3d&                      pts2,
                                                    const Vector3d&                      tang1,
                                                    const Vector3d&                      tang2,
                                                    const Vector3d&                      Dtang1,
                                                    const Vector3d&                      Dtang2,
                                                    const Standard_Real                Rayon,
                                                    const Standard_Real                DRayon,
                                                    const Standard_Real                D2Rayon,
                                                    const Point3d&                      Center,
                                                    const Vector3d&                      DCenter,
                                                    const Vector3d&                      D2Center,
                                                    TColgp_Array1OfPnt&                Poles,
                                                    TColgp_Array1OfVec&                DPoles,
                                                    TColgp_Array1OfVec&                D2Poles,
                                                    TColStd_Array1OfReal&              Weigths,
                                                    TColStd_Array1OfReal&              DWeigths,
                                                    TColStd_Array1OfReal&              D2Weigths);

  Standard_EXPORT static void GetShape(const Standard_Real           MaxAng,
                                       Standard_Integer&             NbPoles,
                                       Standard_Integer&             NbKnots,
                                       Standard_Integer&             Degree,
                                       Convert_ParameterisationType& TypeConv);

  Standard_EXPORT static void Knots(const Convert_ParameterisationType TypeConv,
                                    TColStd_Array1OfReal&              TKnots);

  Standard_EXPORT static void Mults(const Convert_ParameterisationType TypeConv,
                                    TColStd_Array1OfInteger&           TMults);

  Standard_EXPORT static void GetMinimalWeights(const Convert_ParameterisationType TConv,
                                                const Standard_Real                AngleMin,
                                                const Standard_Real                AngleMax,
                                                TColStd_Array1OfReal&              Weigths);

  //! Used  by  the  generical classes to determine
  //! Tolerance for approximation
  Standard_EXPORT static Standard_Real GetTolerance(const Convert_ParameterisationType TConv,
                                                    const Standard_Real                AngleMin,
                                                    const Standard_Real                Radius,
                                                    const Standard_Real                AngularTol,
                                                    const Standard_Real                SpatialTol);
};

#endif // _GeomFill_HeaderFile
