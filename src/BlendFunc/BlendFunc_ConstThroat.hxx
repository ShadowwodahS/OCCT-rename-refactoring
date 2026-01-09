// Created by: Julia GERASIMOVA
// Copyright (c) 2015 OPEN CASCADE SAS
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

#ifndef _BlendFunc_ConstThroat_HeaderFile
#define _BlendFunc_ConstThroat_HeaderFile

#include <Adaptor3d_Surface.hxx>
#include <BlendFunc_GenChamfer.hxx>
#include <math_Vector.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColgp_Array1OfVec2d.hxx>

class math_Matrix;

//! Class for a function used to compute a symmetric chamfer
//! with constant throat that is the height of isosceles triangle in section
class BlendFunc_ConstThroat : public BlendFunc_GenChamfer
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT BlendFunc_ConstThroat(const Handle(SurfaceAdaptor)& S1,
                                        const Handle(SurfaceAdaptor)& S2,
                                        const Handle(Curve5)&   C);

  //! computes the values <F> of the Functions for the
  //! variable <X>.
  //! Returns True if the computation was done successfully,
  //! False otherwise.
  Standard_EXPORT Standard_Boolean Value(const math_Vector& X, math_Vector& F) Standard_OVERRIDE;

  //! returns the values <D> of the derivatives for the
  //! variable <X>.
  //! Returns True if the computation was done successfully,
  //! False otherwise.
  Standard_EXPORT Standard_Boolean Derivatives(const math_Vector& X,
                                               math_Matrix&       D) Standard_OVERRIDE;

  Standard_EXPORT void Set(const Standard_Real Param) Standard_OVERRIDE;

  Standard_EXPORT Standard_Boolean IsSolution(const math_Vector&  Sol,
                                              const Standard_Real Tol) Standard_OVERRIDE;

  Standard_EXPORT const Point3d& PointOnS1() const Standard_OVERRIDE;

  Standard_EXPORT const Point3d& PointOnS2() const Standard_OVERRIDE;

  Standard_EXPORT Standard_Boolean IsTangencyPoint() const Standard_OVERRIDE;

  Standard_EXPORT const Vector3d& TangentOnS1() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Vec2d& Tangent2dOnS1() const Standard_OVERRIDE;

  Standard_EXPORT const Vector3d& TangentOnS2() const Standard_OVERRIDE;

  Standard_EXPORT const gp_Vec2d& Tangent2dOnS2() const Standard_OVERRIDE;

  //! Returns the tangent vector at the section,
  //! at the beginning and the end of the section, and
  //! returns the normal (of the surfaces) at
  //! these points.
  Standard_EXPORT void Tangent(const Standard_Real U1,
                               const Standard_Real V1,
                               const Standard_Real U2,
                               const Standard_Real V2,
                               Vector3d&             TgFirst,
                               Vector3d&             TgLast,
                               Vector3d&             NormFirst,
                               Vector3d&             NormLast) const Standard_OVERRIDE;

  //! Sets the throat and the "quadrant".
  Standard_EXPORT void Set(const Standard_Real aThroat,
                           const Standard_Real,
                           const Standard_Integer Choix) Standard_OVERRIDE;

  //! Returns the length of the maximum section
  Standard_EXPORT Standard_Real GetSectionSize() const Standard_OVERRIDE;

protected:
  Point3d           pts1;
  Point3d           pts2;
  Vector3d           d1u1;
  Vector3d           d1v1;
  Vector3d           d1u2;
  Vector3d           d1v2;
  Standard_Boolean istangent;
  Vector3d           tg1;
  gp_Vec2d         tg12d;
  Vector3d           tg2;
  gp_Vec2d         tg22d;
  Standard_Real    param;
  Standard_Real    Throat;

  Point3d        ptgui;
  Vector3d        nplan;
  Standard_Real normtg;
  Standard_Real theD;
  Vector3d        d1gui;
  Vector3d        d2gui;

private:
};

#endif // _BlendFunc_ConstThroat_HeaderFile
