// Created on: 1998-03-23
// Created by: Jean Yves LEBEY
// Copyright (c) 1998-1999 Matra Datavision
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

#ifndef _TopOpeBRepTool_2d_HeaderFile
#define _TopOpeBRepTool_2d_HeaderFile
#include <Geom2d_Curve.hxx>
#include <TopOpeBRepTool_define.hxx>

Standard_EXPORT Standard_Integer FC2D_Prepare(const TopoShape& S1, const TopoShape& S2);
Standard_EXPORT Standard_Boolean FC2D_HasC3D(const TopoEdge& E);
Standard_EXPORT Standard_Boolean FC2D_HasCurveOnSurface(const TopoEdge& E, const TopoFace& F);
Standard_EXPORT Standard_Boolean FC2D_HasOldCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D,
                                                           Standard_Real&        f,
                                                           Standard_Real&        l,
                                                           Standard_Real&        tol);
Standard_EXPORT Standard_Boolean FC2D_HasOldCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D);
Standard_EXPORT Standard_Boolean FC2D_HasNewCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D,
                                                           Standard_Real&        f,
                                                           Standard_Real&        l,
                                                           Standard_Real&        tol);
Standard_EXPORT Standard_Boolean FC2D_HasNewCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D);
Standard_EXPORT Handle(GeomCurve2d) FC2D_CurveOnSurface(
  const TopoEdge&     E,
  const TopoFace&     F,
  Standard_Real&         f,
  Standard_Real&         l,
  Standard_Real&         tol,
  const Standard_Boolean trim3d = Standard_False);
Standard_EXPORT Handle(GeomCurve2d) FC2D_CurveOnSurface(
  const TopoEdge&     E,
  const TopoFace&     F,
  const TopoEdge&     EF,
  Standard_Real&         f,
  Standard_Real&         l,
  Standard_Real&         tol,
  const Standard_Boolean trim3d = Standard_False);
// modified by NIZHNY-MZV
Standard_EXPORT Handle(GeomCurve2d) FC2D_MakeCurveOnSurface(
  const TopoEdge&     E,
  const TopoFace&     F,
  Standard_Real&         f,
  Standard_Real&         l,
  Standard_Real&         tol,
  const Standard_Boolean trim3d = Standard_False);
Standard_EXPORT Handle(GeomCurve2d) FC2D_EditableCurveOnSurface(
  const TopoEdge&     E,
  const TopoFace&     F,
  Standard_Real&         f,
  Standard_Real&         l,
  Standard_Real&         tol,
  const Standard_Boolean trim3d = Standard_False);
Standard_EXPORT Standard_Integer FC2D_AddNewCurveOnSurface(Handle(GeomCurve2d) PC,
                                                           const TopoEdge&   E,
                                                           const TopoFace&   F,
                                                           const Standard_Real& f,
                                                           const Standard_Real& l,
                                                           const Standard_Real& tol);
#endif
