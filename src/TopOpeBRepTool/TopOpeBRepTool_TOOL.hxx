// Created on: 1998-11-26
// Created by: Xuan PHAM PHU
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

#ifndef _TopOpeBRepTool_TOOL_HeaderFile
#define _TopOpeBRepTool_TOOL_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Integer.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopAbs_State.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
class TopoShape;
class TopoEdge;
class TopoVertex;
class TopoFace;
class gp_Pnt2d;
class TopOpeBRepTool_C2DF;
class Vector3d;
class gp_Dir2d;
class BRepAdaptor_Curve;
class gp_Vec2d;
class Dir3d;
class GeomCurve2d;
class Point3d;

class TOOL1
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT static Standard_Integer OriinSor(
    const TopoShape&    sub,
    const TopoShape&    S,
    const Standard_Boolean checkclo = Standard_False);

  Standard_EXPORT static Standard_Integer OriinSorclosed(const TopoShape& sub,
                                                         const TopoShape& S);

  Standard_EXPORT static Standard_Boolean ClosedE(const TopoEdge& E, TopoVertex& vclo);

  Standard_EXPORT static Standard_Boolean ClosedS(const TopoFace& F);

  Standard_EXPORT static Standard_Boolean IsClosingE(const TopoEdge& E, const TopoFace& F);

  Standard_EXPORT static Standard_Boolean IsClosingE(const TopoEdge&  E,
                                                     const TopoShape& W,
                                                     const TopoFace&  F);

  Standard_EXPORT static void Vertices(const TopoEdge& E, TopTools_Array1OfShape& Vces);

  Standard_EXPORT static TopoVertex Vertex(const Standard_Integer Iv, const TopoEdge& E);

  Standard_EXPORT static Standard_Real ParE(const Standard_Integer Iv, const TopoEdge& E);

  Standard_EXPORT static Standard_Integer OnBoundary(const Standard_Real par, const TopoEdge& E);

  Standard_EXPORT static gp_Pnt2d UVF(const Standard_Real par, const TopOpeBRepTool_C2DF& C2DF);

  Standard_EXPORT static Standard_Boolean ParISO(const gp_Pnt2d&    p2d,
                                                 const TopoEdge& e,
                                                 const TopoFace& f,
                                                 Standard_Real&     pare);

  Standard_EXPORT static Standard_Boolean ParE2d(const gp_Pnt2d&    p2d,
                                                 const TopoEdge& e,
                                                 const TopoFace& f,
                                                 Standard_Real&     par,
                                                 Standard_Real&     dist);

  Standard_EXPORT static Standard_Boolean Getduv(const TopoFace&  f,
                                                 const gp_Pnt2d&     uv,
                                                 const Vector3d&       dir,
                                                 const Standard_Real factor,
                                                 gp_Dir2d&           duv);

  Standard_EXPORT static Standard_Boolean uvApp(const TopoFace&  f,
                                                const TopoEdge&  e,
                                                const Standard_Real par,
                                                const Standard_Real eps,
                                                gp_Pnt2d&           uvapp);

  Standard_EXPORT static Standard_Real TolUV(const TopoFace& F, const Standard_Real tol3d);

  Standard_EXPORT static Standard_Real TolP(const TopoEdge& E, const TopoFace& F);

  Standard_EXPORT static Standard_Real minDUV(const TopoFace& F);

  Standard_EXPORT static Standard_Boolean outUVbounds(const gp_Pnt2d& uv, const TopoFace& F);

  Standard_EXPORT static void stuvF(const gp_Pnt2d&    uv,
                                    const TopoFace& F,
                                    Standard_Integer&  onU,
                                    Standard_Integer&  onV);

  Standard_EXPORT static Standard_Boolean TggeomE(const Standard_Real      par,
                                                  const BRepAdaptor_Curve& BC,
                                                  Vector3d&                  Tg);

  Standard_EXPORT static Standard_Boolean TggeomE(const Standard_Real par,
                                                  const TopoEdge&  E,
                                                  Vector3d&             Tg);

  Standard_EXPORT static Standard_Boolean TgINSIDE(const TopoVertex& v,
                                                   const TopoEdge&   E,
                                                   Vector3d&              Tg,
                                                   Standard_Integer&    OvinE);

  Standard_EXPORT static gp_Vec2d Tg2d(const Standard_Integer     iv,
                                       const TopoEdge&         E,
                                       const TopOpeBRepTool_C2DF& C2DF);

  Standard_EXPORT static gp_Vec2d Tg2dApp(const Standard_Integer     iv,
                                          const TopoEdge&         E,
                                          const TopOpeBRepTool_C2DF& C2DF,
                                          const Standard_Real        factor);

  Standard_EXPORT static gp_Vec2d tryTg2dApp(const Standard_Integer     iv,
                                             const TopoEdge&         E,
                                             const TopOpeBRepTool_C2DF& C2DF,
                                             const Standard_Real        factor);

  Standard_EXPORT static Standard_Boolean XX(const gp_Pnt2d&     uv,
                                             const TopoFace&  f,
                                             const Standard_Real par,
                                             const TopoEdge&  e,
                                             Dir3d&             xx);

  Standard_EXPORT static Standard_Boolean Nt(const gp_Pnt2d&    uv,
                                             const TopoFace& f,
                                             Dir3d&            normt);

  Standard_EXPORT static Standard_Boolean NggeomF(const gp_Pnt2d&    uv,
                                                  const TopoFace& F,
                                                  Vector3d&            ng);

  Standard_EXPORT static Standard_Boolean NgApp(const Standard_Real par,
                                                const TopoEdge&  E,
                                                const TopoFace&  F,
                                                const Standard_Real tola,
                                                Dir3d&             ngApp);

  Standard_EXPORT static Standard_Boolean tryNgApp(const Standard_Real par,
                                                   const TopoEdge&  E,
                                                   const TopoFace&  F,
                                                   const Standard_Real tola,
                                                   Dir3d&             ng);

  Standard_EXPORT static Standard_Integer tryOriEinF(const Standard_Real par,
                                                     const TopoEdge&  E,
                                                     const TopoFace&  F);

  Standard_EXPORT static Standard_Boolean IsQuad(const TopoEdge& E);

  Standard_EXPORT static Standard_Boolean IsQuad(const TopoFace& F);

  Standard_EXPORT static Standard_Boolean CurvE(const TopoEdge&  E,
                                                const Standard_Real par,
                                                const Dir3d&       tg0,
                                                Standard_Real&      Curv);

  Standard_EXPORT static Standard_Boolean CurvF(const TopoFace& F,
                                                const gp_Pnt2d&    uv,
                                                const Dir3d&      tg0,
                                                Standard_Real&     Curv,
                                                Standard_Boolean&  direct);

  Standard_EXPORT static Standard_Boolean UVISO(const Handle(GeomCurve2d)& PC,
                                                Standard_Boolean&           isou,
                                                Standard_Boolean&           isov,
                                                gp_Dir2d&                   d2d,
                                                gp_Pnt2d&                   o2d);

  Standard_EXPORT static Standard_Boolean UVISO(const TopOpeBRepTool_C2DF& C2DF,
                                                Standard_Boolean&          isou,
                                                Standard_Boolean&          isov,
                                                gp_Dir2d&                  d2d,
                                                gp_Pnt2d&                  o2d);

  Standard_EXPORT static Standard_Boolean UVISO(const TopoEdge& E,
                                                const TopoFace& F,
                                                Standard_Boolean&  isou,
                                                Standard_Boolean&  isov,
                                                gp_Dir2d&          d2d,
                                                gp_Pnt2d&          o2d);

  Standard_EXPORT static Standard_Boolean IsonCLO(const Handle(GeomCurve2d)& PC,
                                                  const Standard_Boolean      onU,
                                                  const Standard_Real         xfirst,
                                                  const Standard_Real         xperiod,
                                                  const Standard_Real         xtol);

  Standard_EXPORT static Standard_Boolean IsonCLO(const TopOpeBRepTool_C2DF& C2DF,
                                                  const Standard_Boolean     onU,
                                                  const Standard_Real        xfirst,
                                                  const Standard_Real        xperiod,
                                                  const Standard_Real        xtol);

  Standard_EXPORT static void TrslUV(const gp_Vec2d& t2d, TopOpeBRepTool_C2DF& C2DF);

  Standard_EXPORT static Standard_Boolean TrslUVModifE(const gp_Vec2d&    t2d,
                                                       const TopoFace& F,
                                                       TopoEdge&       E);

  Standard_EXPORT static Standard_Real Matter(const Vector3d& d1,
                                              const Vector3d& d2,
                                              const Vector3d& ref);

  Standard_EXPORT static Standard_Real Matter(const gp_Vec2d& d1, const gp_Vec2d& d2);

  Standard_EXPORT static Standard_Boolean Matter(const Dir3d&       xx1,
                                                 const Dir3d&       nt1,
                                                 const Dir3d&       xx2,
                                                 const Dir3d&       nt2,
                                                 const Standard_Real tola,
                                                 Standard_Real&      Ang);

  Standard_EXPORT static Standard_Boolean Matter(const TopoFace&  f1,
                                                 const TopoFace&  f2,
                                                 const TopoEdge&  e,
                                                 const Standard_Real pare,
                                                 const Standard_Real tola,
                                                 Standard_Real&      Ang);

  Standard_EXPORT static Standard_Boolean MatterKPtg(const TopoFace& f1,
                                                     const TopoFace& f2,
                                                     const TopoEdge& e,
                                                     Standard_Real&     Ang);

  Standard_EXPORT static Standard_Boolean Getstp3dF(const Point3d&      p,
                                                    const TopoFace& f,
                                                    gp_Pnt2d&          uv,
                                                    TopAbs_State&      st);

  Standard_EXPORT static Standard_Boolean SplitE(const TopoEdge&    Eanc,
                                                 ShapeList& Splits);

  Standard_EXPORT static void MkShell(const ShapeList& lF, TopoShape& She);

  Standard_EXPORT static Standard_Boolean Remove(ShapeList& loS,
                                                 const TopoShape&   toremove);

  Standard_EXPORT static Standard_Boolean WireToFace(
    const TopoFace&                        Fref,
    const TopTools_DataMapOfShapeListOfShape& mapWlow,
    ShapeList&                     lFs);

  Standard_EXPORT static Standard_Boolean EdgeONFace(const Standard_Real par,
                                                     const TopoEdge&  ed,
                                                     const gp_Pnt2d&     uv,
                                                     const TopoFace&  fa,
                                                     Standard_Boolean&   isonfa);

protected:
private:
};

#endif // _TopOpeBRepTool_TOOL_HeaderFile
