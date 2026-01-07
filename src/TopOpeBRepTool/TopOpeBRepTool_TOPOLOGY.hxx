// Created on: 1998-10-06
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

#ifndef _TopOpeBRepTool_TOPOLOGY_HeaderFile
#define _TopOpeBRepTool_TOPOLOGY_HeaderFile

#include <GeomAbs_CurveType.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Wire.hxx>
// #include <BRepAdaptor_Curve2d.hxx>

Standard_EXPORT void FUN_tool_tolUV(const TopoFace& F, Standard_Real& tolu, Standard_Real& tolv);
Standard_EXPORT Standard_Boolean FUN_tool_direct(const TopoFace& F, Standard_Boolean& direct);
// Standard_EXPORT Standard_Boolean FUN_tool_IsUViso(const TopoShape& E,const TopoShape&
// F,Standard_Boolean& isoU,Standard_Boolean& isoV,gp_Dir2d& d2d,gp_Pnt2d& o2d);
Standard_EXPORT Standard_Boolean FUN_tool_bounds(const TopoShape& F,
                                                 Standard_Real&      u1,
                                                 Standard_Real&      u2,
                                                 Standard_Real&      v1,
                                                 Standard_Real&      v2);
Standard_EXPORT Standard_Boolean FUN_tool_geombounds(const TopoFace& F,
                                                     Standard_Real&     u1,
                                                     Standard_Real&     u2,
                                                     Standard_Real&     v1,
                                                     Standard_Real&     v2);
Standard_EXPORT Standard_Boolean FUN_tool_isobounds(const TopoShape& F,
                                                    Standard_Real&      u1,
                                                    Standard_Real&      u2,
                                                    Standard_Real&      v1,
                                                    Standard_Real&      v2);
Standard_EXPORT Standard_Boolean FUN_tool_outbounds(const TopoShape& Sh,
                                                    Standard_Real&      u1,
                                                    Standard_Real&      u2,
                                                    Standard_Real&      v1,
                                                    Standard_Real&      v2,
                                                    Standard_Boolean&   outbounds);

// ----------------------------------------------------------------------
//  project point <P> on geometries (curve <C>,surface <S>)
// ----------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FUN_tool_PinC(const Point3d&            P,
                                               const BRepAdaptor_Curve& BAC,
                                               const Standard_Real      pmin,
                                               const Standard_Real      pmax,
                                               const Standard_Real      tol);
Standard_EXPORT Standard_Boolean FUN_tool_PinC(const Point3d&            P,
                                               const BRepAdaptor_Curve& BAC,
                                               const Standard_Real      tol);

// ----------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FUN_tool_value(const Standard_Real par,
                                                const TopoEdge&  E,
                                                Point3d&             P);
Standard_EXPORT Standard_Boolean FUN_tool_value(const gp_Pnt2d&    UV,
                                                const TopoFace& F,
                                                Point3d&            P);
Standard_EXPORT TopAbs_State     FUN_tool_staPinE(const Point3d&       P,
                                                  const TopoEdge&  E,
                                                  const Standard_Real tol);
Standard_EXPORT TopAbs_State     FUN_tool_staPinE(const Point3d& P, const TopoEdge& E);

// ----------------------------------------------------------------------
//  subshape's orientation :
//  - orientVinE : vertex orientation in edge
//  - orientEinF : edge's orientation in face
//  - tool_orientEinFFORWARD : edge's orientation in face oriented FORWARD
//  - EboundF : true if vertex is oriented (FORWARD,REVERSED) in an edge
// ----------------------------------------------------------------------
Standard_EXPORT Standard_Integer FUN_tool_orientVinE(const TopoVertex& v, const TopoEdge& e);
Standard_EXPORT Standard_Boolean FUN_tool_orientEinF(const TopoEdge&  E,
                                                     const TopoFace&  F,
                                                     TopAbs_Orientation& oriEinF);
Standard_EXPORT Standard_Boolean FUN_tool_orientEinFFORWARD(const TopoEdge&  E,
                                                            const TopoFace&  F,
                                                            TopAbs_Orientation& oriEinF);
Standard_EXPORT Standard_Boolean FUN_tool_EboundF(const TopoEdge& E, const TopoFace& F);

// ----------------------------------------------------------------------
//  derivatives :
// ----------------------------------------------------------------------
Standard_EXPORT Vector3d           FUN_tool_nggeomF(const gp_Pnt2d& p2d, const TopoFace& F);
Standard_EXPORT Standard_Boolean FUN_tool_nggeomF(const Standard_Real& paronE,
                                                  const TopoEdge&   E,
                                                  const TopoFace&   F,
                                                  Vector3d&              nggeomF);
Standard_EXPORT Standard_Boolean FUN_tool_nggeomF(const Standard_Real& paronE,
                                                  const TopoEdge&   E,
                                                  const TopoFace&   F,
                                                  Vector3d&              nggeomF,
                                                  const Standard_Real  tol);
Standard_EXPORT Standard_Boolean FUN_tool_EtgF(const Standard_Real& paronE,
                                               const TopoEdge&   E,
                                               const gp_Pnt2d&      p2d,
                                               const TopoFace&   F,
                                               const Standard_Real  tola);
Standard_EXPORT Standard_Boolean FUN_tool_EtgOOE(const Standard_Real& paronE,
                                                 const TopoEdge&   E,
                                                 const Standard_Real& paronOOE,
                                                 const TopoEdge&   OOE,
                                                 const Standard_Real  tola);

// ----------------------------------------------------------------------
// oriented vectors :
// ----------------------------------------------------------------------
Standard_EXPORT Vector3d           FUN_tool_getgeomxx(const TopoFace&  Fi,
                                                    const TopoEdge&  Ei,
                                                    const Standard_Real parOnEi,
                                                    const Dir3d&       ngFi);
Standard_EXPORT Vector3d           FUN_tool_getgeomxx(const TopoFace&  Fi,
                                                    const TopoEdge&  Ei,
                                                    const Standard_Real parOnEi);
Standard_EXPORT Standard_Boolean FUN_nearestISO(const TopoFace&     F,
                                                const Standard_Real    xpar,
                                                const Standard_Boolean isoU,
                                                Standard_Real&         xinf,
                                                Standard_Real&         xsup);
Standard_EXPORT Standard_Boolean FUN_tool_getxx(const TopoFace&  Fi,
                                                const TopoEdge&  Ei,
                                                const Standard_Real parOnEi,
                                                const Dir3d&       ngFi,
                                                Dir3d&             XX);
Standard_EXPORT Standard_Boolean FUN_tool_getxx(const TopoFace&  Fi,
                                                const TopoEdge&  Ei,
                                                const Standard_Real parOnEi,
                                                Dir3d&             XX);
Standard_EXPORT Standard_Boolean FUN_tool_getdxx(const TopoFace&  F,
                                                 const TopoEdge&  E,
                                                 const Standard_Real parE,
                                                 gp_Vec2d&           XX);
Standard_EXPORT Standard_Boolean FUN_tool_EitangenttoFe(const Dir3d&       ngFe,
                                                        const TopoEdge&  Ei,
                                                        const Standard_Real parOnEi);

// ----------------------------------------------------------------------
// curve type,surface type :
// ----------------------------------------------------------------------
Standard_EXPORT GeomAbs_CurveType FUN_tool_typ(const TopoEdge& E);
Standard_EXPORT Standard_Boolean  FUN_tool_line(const TopoEdge& E);
Standard_EXPORT Standard_Boolean  FUN_tool_plane(const TopoShape& F);
Standard_EXPORT Standard_Boolean  FUN_tool_cylinder(const TopoShape& F);
Standard_EXPORT Standard_Boolean  FUN_tool_closedS(const TopoShape& F,
                                                   Standard_Boolean&   uclosed,
                                                   Standard_Real&      uperiod,
                                                   Standard_Boolean&   vclosed,
                                                   Standard_Real&      vperiod);
Standard_EXPORT Standard_Boolean  FUN_tool_closedS(const TopoShape& F);
Standard_EXPORT Standard_Boolean  FUN_tool_closedS(const TopoShape& F,
                                                   Standard_Boolean&   inU,
                                                   Standard_Real&      xmin,
                                                   Standard_Real&      xper);
Standard_EXPORT void              FUN_tool_mkBnd2d(const TopoShape& W,
                                                   const TopoShape& FF,
                                                   Bnd_Box2d&          B2d);

// ----------------------------------------------------------------------
//  closing topologies :
// ----------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FUN_tool_IsClosingE(const TopoEdge&  E,
                                                     const TopoShape& S,
                                                     const TopoFace&  F);
Standard_EXPORT Standard_Boolean FUN_tool_ClosingE(const TopoEdge& E,
                                                   const TopoWire& W,
                                                   const TopoFace& F);

// ----------------------------------------------------------------------
//  shared topologies :
// ----------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FUN_tool_inS(const TopoShape& subshape,
                                              const TopoShape& shape);
Standard_EXPORT Standard_Boolean FUN_tool_Eshared(const TopoShape& v,
                                                  const TopoShape& F1,
                                                  const TopoShape& F2,
                                                  TopoShape&       Eshared);

Standard_EXPORT Standard_Boolean FUN_tool_parVonE(const TopoVertex& v,
                                                  const TopoEdge&   E,
                                                  Standard_Real&       par);
Standard_EXPORT Standard_Boolean FUN_tool_parE(const TopoEdge&   E0,
                                               const Standard_Real& par0,
                                               const TopoEdge&   E,
                                               Standard_Real&       par,
                                               const Standard_Real  tol);
Standard_EXPORT Standard_Boolean FUN_tool_parE(const TopoEdge&   E0,
                                               const Standard_Real& par0,
                                               const TopoEdge&   E,
                                               Standard_Real&       par);
Standard_EXPORT Standard_Boolean FUN_tool_paronEF(const TopoEdge&   E,
                                                  const Standard_Real& par,
                                                  const TopoFace&   F,
                                                  gp_Pnt2d&            UV,
                                                  const Standard_Real  tol);
Standard_EXPORT Standard_Boolean FUN_tool_paronEF(const TopoEdge&   E,
                                                  const Standard_Real& par,
                                                  const TopoFace&   F,
                                                  gp_Pnt2d&            UV);
Standard_EXPORT Standard_Boolean FUN_tool_parF(const TopoEdge&   E,
                                               const Standard_Real& par,
                                               const TopoFace&   F,
                                               gp_Pnt2d&            UV,
                                               const Standard_Real  tol);
Standard_EXPORT Standard_Boolean FUN_tool_parF(const TopoEdge&   E,
                                               const Standard_Real& par,
                                               const TopoFace&   F,
                                               gp_Pnt2d&            UV);
Standard_EXPORT Dir3d FUN_tool_dirC(const Standard_Real par, const BRepAdaptor_Curve& BAC);
Standard_EXPORT Vector3d FUN_tool_tggeomE(const Standard_Real paronE, const TopoEdge& E);
Standard_EXPORT Standard_Boolean FUN_tool_line(const BRepAdaptor_Curve& BAC);
Standard_EXPORT Standard_Boolean FUN_tool_quad(const TopoEdge& E);
Standard_EXPORT Standard_Boolean FUN_tool_quad(const BRepAdaptor_Curve& BAC);
Standard_EXPORT Standard_Boolean FUN_tool_quad(const TopoFace& F);
Standard_EXPORT Standard_Boolean FUN_tool_findPinBAC(const BRepAdaptor_Curve& BAC,
                                                     Point3d&                  P,
                                                     Standard_Real&           par);
Standard_EXPORT Standard_Boolean FUN_tool_findparinBAC(const BRepAdaptor_Curve& BAC,
                                                       Standard_Real&           par);
Standard_EXPORT Standard_Boolean FUN_tool_findparinE(const TopoShape& E, Standard_Real& par);
Standard_EXPORT Standard_Boolean FUN_tool_findPinE(const TopoShape& E,
                                                   Point3d&             P,
                                                   Standard_Real&      par);
Standard_EXPORT Standard_Boolean FUN_tool_maxtol(const TopoShape&     S,
                                                 const TopAbs_ShapeEnum& typ,
                                                 Standard_Real&          tol);
Standard_EXPORT Standard_Real    FUN_tool_maxtol(const TopoShape& S);
Standard_EXPORT Standard_Integer FUN_tool_nbshapes(const TopoShape&     S,
                                                   const TopAbs_ShapeEnum& typ);
Standard_EXPORT void             FUN_tool_shapes(const TopoShape&     S,
                                                 const TopAbs_ShapeEnum& typ,
                                                 ShapeList&   ltyp);
Standard_EXPORT Standard_Integer FUN_tool_comparebndkole(const TopoShape& sh1,
                                                         const TopoShape& sh2);
Standard_EXPORT Standard_Boolean FUN_tool_SameOri(const TopoEdge& E1, const TopoEdge& E2);
Standard_EXPORT Standard_Boolean FUN_tool_haspc(const TopoEdge& E, const TopoFace& F);
Standard_EXPORT Standard_Boolean FUN_tool_pcurveonF(const TopoFace& F, TopoEdge& E);
Standard_EXPORT Standard_Boolean FUN_tool_pcurveonF(const TopoFace&          fF,
                                                    TopoEdge&                faultyE,
                                                    const Handle(GeomCurve2d)& C2d,
                                                    TopoFace&                newf);

// ----------------------------------------------------------------------
//  shared geometry :
// ----------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FUN_tool_curvesSO(const TopoEdge&  E1,
                                                   const Standard_Real p1,
                                                   const TopoEdge&  E2,
                                                   const Standard_Real p2,
                                                   Standard_Boolean&   so);
Standard_EXPORT Standard_Boolean FUN_tool_curvesSO(const TopoEdge&  E1,
                                                   const Standard_Real p1,
                                                   const TopoEdge&  E2,
                                                   Standard_Boolean&   so);
Standard_EXPORT Standard_Boolean FUN_tool_curvesSO(const TopoEdge& E1,
                                                   const TopoEdge& E2,
                                                   Standard_Boolean&  so);
Standard_EXPORT Standard_Boolean FUN_tool_findAncestor(const ShapeList& lF,
                                                       const TopoEdge&          E,
                                                       TopoFace&                Fanc);

// ----------------------------------------------------------------------
//  new topologies :
// ----------------------------------------------------------------------
Standard_EXPORT void             FUN_ds_CopyEdge(const TopoShape& Ein, TopoShape& Eou);
Standard_EXPORT void             FUN_ds_Parameter(const TopoShape& E,
                                                  const TopoShape& V,
                                                  const Standard_Real P);
Standard_EXPORT Standard_Boolean FUN_tool_MakeWire(const ShapeList& loE,
                                                   TopoWire&                newW);

#endif
