// Created on: 1994-02-09
// Created by: Jean Yves LEBEY
// Copyright (c) 1994-1999 Matra Datavision
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

#ifndef _TopOpeBRepTool_ShapeTool_HeaderFile
#define _TopOpeBRepTool_ShapeTool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

class TopoShape;
class Point3d;
class GeomCurve3d;
class TopoEdge;
class GeomSurface;
class TopoFace;
class BRepAdaptor_Surface;
class BRepAdaptor_Curve;
class Dir3d;

class ShapeTool
{
public:
  DEFINE_STANDARD_ALLOC

  //! Returns the tolerance of the shape <S>.
  //! If the shape <S> is Null, returns 0.
  Standard_EXPORT static Standard_Real Tolerance(const TopoShape& S);

  //! Returns 3D point of vertex <S>.
  Standard_EXPORT static Point3d Pnt(const TopoShape& S);

  Standard_EXPORT static Handle(GeomCurve3d) BASISCURVE(const Handle(GeomCurve3d)& C);

  Standard_EXPORT static Handle(GeomCurve3d) BASISCURVE(const TopoEdge& E);

  Standard_EXPORT static Handle(GeomSurface) BASISSURFACE(const Handle(GeomSurface)& S);

  Standard_EXPORT static Handle(GeomSurface) BASISSURFACE(const TopoFace& F);

  Standard_EXPORT static void UVBOUNDS(const Handle(GeomSurface)& S,
                                       Standard_Boolean&           UPeri,
                                       Standard_Boolean&           VPeri,
                                       Standard_Real&              Umin,
                                       Standard_Real&              Umax,
                                       Standard_Real&              Vmin,
                                       Standard_Real&              Vmax);

  Standard_EXPORT static void UVBOUNDS(const TopoFace& F,
                                       Standard_Boolean&  UPeri,
                                       Standard_Boolean&  VPeri,
                                       Standard_Real&     Umin,
                                       Standard_Real&     Umax,
                                       Standard_Real&     Vmin,
                                       Standard_Real&     Vmax);

  //! adjust u,v values in UVBounds of the domain of the
  //! geometric shape <S>, according to Uperiodicity and
  //! VPeriodicity of the domain.
  //! <S> is assumed to be a face.
  //! u and/or v is/are not modified when the domain is
  //! not periodic in U and/or V .
  Standard_EXPORT static void AdjustOnPeriodic(const TopoShape& S,
                                               Standard_Real&      u,
                                               Standard_Real&      v);

  //! indicates whether shape S1 is a closing shape on S2 or not.
  Standard_EXPORT static Standard_Boolean Closed(const TopoShape& S1, const TopoShape& S2);

  Standard_EXPORT static Standard_Real PeriodizeParameter(const Standard_Real par,
                                                          const TopoShape& EE,
                                                          const TopoShape& FF);

  Standard_EXPORT static Standard_Boolean ShapesSameOriented(const TopoShape& S1,
                                                             const TopoShape& S2);

  Standard_EXPORT static Standard_Boolean SurfacesSameOriented(const BRepAdaptor_Surface& S1,
                                                               const BRepAdaptor_Surface& S2);

  Standard_EXPORT static Standard_Boolean FacesSameOriented(const TopoShape& F1,
                                                            const TopoShape& F2);

  Standard_EXPORT static Standard_Boolean CurvesSameOriented(const BRepAdaptor_Curve& C1,
                                                             const BRepAdaptor_Curve& C2);

  Standard_EXPORT static Standard_Boolean EdgesSameOriented(const TopoShape& E1,
                                                            const TopoShape& E2);

  //! Compute tangent T, normal N, curvature C at point of parameter
  //! P on curve BRAC. Returns the tolerance indicating if T,N are null.
  Standard_EXPORT static Standard_Real EdgeData(const BRepAdaptor_Curve& BRAC,
                                                const Standard_Real      P,
                                                Dir3d&                  T,
                                                Dir3d&                  N,
                                                Standard_Real&           C);

  //! Same as previous on edge E.
  Standard_EXPORT static Standard_Real EdgeData(const TopoShape& E,
                                                const Standard_Real P,
                                                Dir3d&             T,
                                                Dir3d&             N,
                                                Standard_Real&      C);

  Standard_EXPORT static Standard_Real Resolution3dU(const Handle(GeomSurface)& SU,
                                                     const Standard_Real         Tol2d);

  Standard_EXPORT static Standard_Real Resolution3dV(const Handle(GeomSurface)& SU,
                                                     const Standard_Real         Tol2d);

  Standard_EXPORT static Standard_Real Resolution3d(const Handle(GeomSurface)& SU,
                                                    const Standard_Real         Tol2d);

  Standard_EXPORT static Standard_Real Resolution3d(const TopoFace&  F,
                                                    const Standard_Real Tol2d);

protected:
private:
};

#endif // _TopOpeBRepTool_ShapeTool_HeaderFile
