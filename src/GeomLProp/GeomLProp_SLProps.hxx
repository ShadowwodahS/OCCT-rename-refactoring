// Created on: 1992-03-26
// Created by: Herve LEGRAND
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

#ifndef _GeomLProp_SLProps_HeaderFile
#define _GeomLProp_SLProps_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Integer.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <LProp_Status.hxx>
class Geom_Surface;
class LProp_BadContinuity;
class Standard_DomainError;
class Standard_OutOfRange;
class LProp_NotDefined;
class GeomLProp_SurfaceTool;
class Point3d;
class Vector3d;
class Dir3d;

class GeomLProp_SLProps
{
public:
  DEFINE_STANDARD_ALLOC

  //! Initializes the local properties of the surface <S>
  //! for the parameter values (<U>, <V>).
  //! The current point and the derivatives are
  //! computed at the same time, which allows an
  //! optimization of the computation time.
  //! <N> indicates the maximum number of derivations to
  //! be done (0, 1, or 2). For example, to compute
  //! only the tangent, N should be equal to 1.
  //! <Resolution> is the linear tolerance (it is used to test
  //! if a vector is null).
  Standard_EXPORT GeomLProp_SLProps(const Handle(Geom_Surface)& S,
                                    const Standard_Real         U,
                                    const Standard_Real         V,
                                    const Standard_Integer      N,
                                    const Standard_Real         Resolution);

  //! idem as previous constructor but without setting the value
  //! of parameters <U> and <V>.
  Standard_EXPORT GeomLProp_SLProps(const Handle(Geom_Surface)& S,
                                    const Standard_Integer      N,
                                    const Standard_Real         Resolution);

  //! idem as previous constructor but without setting the value
  //! of parameters <U> and <V> and the surface.
  //! the surface can have an empty constructor.
  Standard_EXPORT GeomLProp_SLProps(const Standard_Integer N, const Standard_Real Resolution);

  //! Initializes the local properties of the surface S
  //! for the new surface.
  Standard_EXPORT void SetSurface(const Handle(Geom_Surface)& S);

  //! Initializes the local properties of the surface S
  //! for the new parameter values (<U>, <V>).
  Standard_EXPORT void SetParameters(const Standard_Real U, const Standard_Real V);

  //! Returns the point.
  Standard_EXPORT const Point3d& Value() const;

  //! Returns the first U derivative.
  //! The derivative is computed if it has not been yet.
  Standard_EXPORT const Vector3d& D1U();

  //! Returns the first V derivative.
  //! The derivative is computed if it has not been yet.
  Standard_EXPORT const Vector3d& D1V();

  //! Returns the second U derivatives
  //! The derivative is computed if it has not been yet.
  Standard_EXPORT const Vector3d& D2U();

  //! Returns the second V derivative.
  //! The derivative is computed if it has not been yet.
  Standard_EXPORT const Vector3d& D2V();

  //! Returns the second UV cross-derivative.
  //! The derivative is computed if it has not been yet.
  Standard_EXPORT const Vector3d& DUV();

  //! returns True if the U tangent is defined.
  //! For example, the tangent is not defined if the
  //! two first U derivatives are null.
  Standard_EXPORT Standard_Boolean IsTangentUDefined();

  //! Returns the tangent direction <D> on the iso-V.
  Standard_EXPORT void TangentU(Dir3d& D);

  //! returns if the V tangent is defined.
  //! For example, the tangent is not defined if the
  //! two first V derivatives are null.
  Standard_EXPORT Standard_Boolean IsTangentVDefined();

  //! Returns the tangent direction <D> on the iso-V.
  Standard_EXPORT void TangentV(Dir3d& D);

  //! Tells if the normal is defined.
  Standard_EXPORT Standard_Boolean IsNormalDefined();

  //! Returns the normal direction.
  Standard_EXPORT const Dir3d& Normal();

  //! returns True if the curvature is defined.
  Standard_EXPORT Standard_Boolean IsCurvatureDefined();

  //! returns True if the point is umbilic (i.e. if the
  //! curvature is constant).
  Standard_EXPORT Standard_Boolean IsUmbilic();

  //! Returns the maximum curvature
  Standard_EXPORT Standard_Real MaxCurvature();

  //! Returns the minimum curvature
  Standard_EXPORT Standard_Real MinCurvature();

  //! Returns the direction of the maximum and minimum curvature
  //! <MaxD> and <MinD>
  Standard_EXPORT void CurvatureDirections(Dir3d& MaxD, Dir3d& MinD);

  //! Returns the mean curvature.
  Standard_EXPORT Standard_Real MeanCurvature();

  //! Returns the Gaussian curvature
  Standard_EXPORT Standard_Real GaussianCurvature();

protected:
private:
  Handle(Geom_Surface) mySurf;
  Standard_Real        myU;
  Standard_Real        myV;
  Standard_Integer     myDerOrder;
  Standard_Integer     myCN;
  Standard_Real        myLinTol;
  Point3d               myPnt;
  Vector3d               myD1u;
  Vector3d               myD1v;
  Vector3d               myD2u;
  Vector3d               myD2v;
  Vector3d               myDuv;
  Dir3d               myNormal;
  Standard_Real        myMinCurv;
  Standard_Real        myMaxCurv;
  Dir3d               myDirMinCurv;
  Dir3d               myDirMaxCurv;
  Standard_Real        myMeanCurv;
  Standard_Real        myGausCurv;
  Standard_Integer     mySignificantFirstDerivativeOrderU;
  Standard_Integer     mySignificantFirstDerivativeOrderV;
  LProp_Status         myUTangentStatus;
  LProp_Status         myVTangentStatus;
  LProp_Status         myNormalStatus;
  LProp_Status         myCurvatureStatus;
};

#endif // _GeomLProp_SLProps_HeaderFile
