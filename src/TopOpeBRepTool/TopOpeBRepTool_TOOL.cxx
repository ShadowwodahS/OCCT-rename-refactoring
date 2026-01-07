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

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepLProp_CLProps.hxx>
#include <ElCLib.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
#include <GeomLProp_SLProps.hxx>
#include <gp_Circ.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Lin.hxx>
#include <gp_Parab.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_Vec.hxx>
#include <gp_Vec2d.hxx>
#include <NCollection_Array1.hxx>
#include <Precision.hxx>
#include <TColStd_IndexedMapOfReal.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopOpeBRepTool.hxx>
#include <TopOpeBRepTool_2d.hxx>
#include <TopOpeBRepTool_C2DF.hxx>
#include <TopOpeBRepTool_define.hxx>
#include <TopOpeBRepTool_EXPORT.hxx>
#include <TopOpeBRepTool_ShapeTool.hxx>
#include <TopOpeBRepTool_TOOL.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>

#include <algorithm>
#define M_FORWARD(sta) (sta == TopAbs_FORWARD)
#define M_REVERSED(sta) (sta == TopAbs_REVERSED)
#define M_INTERNAL(sta) (sta == TopAbs_INTERNAL)
#define M_EXTERNAL(sta) (sta == TopAbs_EXTERNAL)

#define FORWARD (1)
#define REVERSED (2)
#define INTERNAL (3)
#define EXTERNAL (4)
#define CLOSING (5)

static Standard_Boolean FUN_nullprodv(const Standard_Real prodv)
{
  //  Standard_Real tola = Precision::Angular()*1.e+1; // NYI
  Standard_Real tola = 1.e-6; // NYI NYI NYI : for case cto 012 I2
  return (Abs(prodv) < tola);
}

// modified by NIZNHY-PKV Fri Aug  4 11:22:57 2000 from

//=================================================================================================

static Standard_Boolean CheckEdgeLength(const TopoEdge& E)
{
  BRepAdaptor_Curve BC(E);

  TopTools_IndexedMapOfShape aM;
  TopExp1::MapShapes(E, TopAbs_VERTEX, aM);
  Standard_Integer i, anExtent, aN = 10;
  Standard_Real    ln = 0., d, t, f, l, dt;
  anExtent            = aM.Extent();

  if (anExtent != 1)
    return Standard_True;

  Point3d p1, p2;
  f  = BC.FirstParameter();
  l  = BC.LastParameter();
  dt = (l - f) / aN;

  BC.D0(f, p1);
  for (i = 1; i <= aN; i++)
  {
    t = f + i * dt;

    if (i == aN)
      BC.D0(l, p2);
    else
      BC.D0(t, p2);

    d = p1.Distance(p2);
    ln += d;
    p1 = p2;
  }

  return (ln > Precision::Confusion());
}

// modified by NIZNHY-PKV Fri Aug  4 11:23:07 2000 to

//=================================================================================================

Standard_Integer TOOL1::OriinSor(const TopoShape&    sub,
                                               const TopoShape&    S,
                                               const Standard_Boolean checkclo)
{
  if (checkclo)
  {
    Standard_Boolean Sclosed = Standard_False;
    if (S.ShapeType() == TopAbs_EDGE)
    {
      if (sub.ShapeType() != TopAbs_VERTEX)
        return 0;

      TopoVertex vclo;
      Sclosed = TOOL1::ClosedE(TopoDS::Edge(S), vclo);
      if (Sclosed)
        if (sub.IsSame(vclo))
          return CLOSING;
    }
    else if (S.ShapeType() == TopAbs_FACE)
    {
      if (sub.ShapeType() != TopAbs_EDGE)
        return 0;

      Sclosed = ClosedS(TopoDS::Face(S));
      if (Sclosed)
        if (IsClosingE(TopoDS::Edge(sub), TopoDS::Face(S)))
          return CLOSING;
    }
  }

  ShapeExplorer ex(S, sub.ShapeType());
  for (; ex.More(); ex.Next())
  {
    const TopoShape& ssub = ex.Current();
    Standard_Boolean    same = ssub.IsSame(sub);
    if (!same)
      continue;
    TopAbs_Orientation osub = ssub.Orientation();
    if (M_FORWARD(osub))
      return FORWARD;
    else if (M_REVERSED(osub))
      return REVERSED;
    else if (M_INTERNAL(osub))
      return INTERNAL;
    else if (M_EXTERNAL(osub))
      return EXTERNAL;
  }
  return 0;
}

//=================================================================================================

Standard_Integer TOOL1::OriinSorclosed(const TopoShape& sub, const TopoShape& S)
{
  if (S.ShapeType() == TopAbs_EDGE)
  {
    if (sub.ShapeType() != TopAbs_VERTEX)
      return 0;
  }
  else if (S.ShapeType() == TopAbs_FACE)
  {
    if (sub.ShapeType() != TopAbs_EDGE)
      return 0;
  }
  TopoDS_Iterator it(S);
  for (; it.More(); it.Next())
  {
    const TopoShape& ssub  = it.Value();
    Standard_Boolean    equal = ssub.IsEqual(sub);
    if (!equal)
      continue;
    TopAbs_Orientation osub = ssub.Orientation();
    if (M_FORWARD(osub))
      return FORWARD;
    else if (M_REVERSED(osub))
      return REVERSED;
  }
  return 0;
}

//=================================================================================================

Standard_Boolean TOOL1::ClosedE(const TopoEdge& E, TopoVertex& vclo)
{
  // returns true if <E> has a closing vertex <vclosing>
  //  return E.IsClosed();
  Standard_Boolean isdgE = BRepInspector::Degenerated(E);
  if (isdgE)
    return Standard_False;

  TopoShape vv;
  vclo.Nullify();
  ShapeExplorer ex(E, TopAbs_VERTEX);
  for (; ex.More(); ex.Next())
  {
    const TopoShape& v = ex.Current();
    if (M_INTERNAL(v.Orientation()))
      continue;
    if (vv.IsNull())
      vv = v;
    else if (v.IsSame(vv))
    {
      vclo = TopoDS::Vertex(vv);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TOOL1::ClosedS(const TopoFace& F)
{
  Handle(GeomSurface) S = ShapeTool::BASISSURFACE(TopoDS::Face(F));
  if (S.IsNull())
    return Standard_False;
  Standard_Boolean uclosed = S->IsUClosed();
  if (uclosed)
    uclosed = S->IsUPeriodic();
  Standard_Boolean vclosed = S->IsVClosed();
  if (vclosed)
    vclosed = S->IsVPeriodic();
  return (uclosed || vclosed);
}

//=================================================================================================

Standard_Boolean TOOL1::IsClosingE(const TopoEdge& E, const TopoFace& F)
{
  Standard_Integer nbocc = 0;
  ShapeExplorer  exp(F, TopAbs_EDGE);
  for (; exp.More(); exp.Next())
    if (exp.Current().IsSame(E))
      nbocc++;
  if (nbocc != 2)
    return Standard_False;
  return BRepInspector::IsClosed(E, F);
}

//=================================================================================================

Standard_Boolean TOOL1::IsClosingE(const TopoEdge&  E,
                                                 const TopoShape& W,
                                                 const TopoFace&  F)
{
  Standard_Integer nbocc = 0;
  ShapeExplorer  exp(W, TopAbs_EDGE);
  for (; exp.More(); exp.Next())
    if (exp.Current().IsSame(E))
      nbocc++;
  if (nbocc != 2)
    return Standard_False;
  return BRepInspector::IsClosed(E, F);
}

//=================================================================================================

void TOOL1::Vertices(const TopoEdge& E, TopTools_Array1OfShape& Vces)
{
  // Returns vertices (F,R) if E is FORWARD
  //                  (R,V) if E is REVERSED
  TopAbs_Orientation oriE = E.Orientation();
  TopoVertex      v1, v2;
  TopExp1::Vertices(E, v1, v2);

  if (M_INTERNAL(oriE) || M_EXTERNAL(oriE))
  {
    Vces.ChangeValue(1) = v1;
    Vces.ChangeValue(2) = v2;
  }

  Standard_Real par1 = BRepInspector::Parameter(v1, E);
  Standard_Real par2 = BRepInspector::Parameter(v2, E);
#ifdef OCCT_DEBUG
//  if (par1>par2) std::cout<<"TOOL1::Vertices ERROR"<<std::endl;
#endif
  Standard_Integer ivparSMA = (par1 < par2) ? FORWARD : REVERSED;
  Standard_Integer ivparSUP = (par1 < par2) ? REVERSED : FORWARD;
  if (M_REVERSED(oriE))
  {
    ivparSMA = (ivparSMA == FORWARD) ? REVERSED : FORWARD;
    ivparSUP = (ivparSUP == REVERSED) ? FORWARD : REVERSED;
  }
  Vces.ChangeValue(ivparSMA) = v1;
  Vces.ChangeValue(ivparSUP) = v2;
}

//=================================================================================================

TopoVertex TOOL1::Vertex(const Standard_Integer Iv, const TopoEdge& E)
{
  TopTools_Array1OfShape Vces(1, 2);
  Vertices(E, Vces);
  TopoVertex V = TopoDS::Vertex(Vces(Iv));
  return V;
}

//=================================================================================================

Standard_Real TOOL1::ParE(const Standard_Integer Iv, const TopoEdge& E)
{
  const TopoVertex& v = Vertex(Iv, E);
  return (BRepInspector::Parameter(v, E));
}

//=================================================================================================

Standard_Integer TOOL1::OnBoundary(const Standard_Real par, const TopoEdge& e)
{
  BRepAdaptor_Curve bc(e);
  Standard_Boolean  closed = bc.IsClosed();
  Standard_Real     first  = bc.FirstParameter();
  Standard_Real     last   = bc.LastParameter();
  Standard_Real     tole   = bc.Tolerance();
  Standard_Real     tolp   = bc.Resolution(tole);

  Standard_Boolean onf  = Abs(par - first) < tolp;
  Standard_Boolean onl  = Abs(par - last) < tolp;
  Standard_Boolean onfl = (onf || onl);
  if (onfl && closed)
    return CLOSING;
  if (onf)
    return FORWARD;
  if (onl)
    return REVERSED;
  if ((first < par) && (par < last))
    return INTERNAL;
  return EXTERNAL;
}

static void FUN_tool_sortVonE(ShapeList& lov, const TopoEdge& E)
{
  TopTools_DataMapOfIntegerShape mapiv;  // mapiv.Find(iV) = V
  TColStd_IndexedMapOfReal       mappar; // mappar.FindIndex(parV) = iV

  for (TopTools_ListIteratorOfListOfShape itlove(lov); itlove.More(); itlove.Next())
  {
    const TopoVertex& v   = TopoDS::Vertex(itlove.Value());
    Standard_Real        par = BRepInspector::Parameter(v, E);
    Standard_Integer     iv  = mappar.Add(par);
    mapiv.Bind(iv, v);
  }
  Standard_Integer                  nv = mapiv.Extent();
  NCollection_Array1<Standard_Real> tabpar(1, nv);
  //  for (Standard_Integer i = 1; i <= nv; i++) {
  Standard_Integer i;
  for (i = 1; i <= nv; i++)
  {
    Standard_Real p = mappar.FindKey(i);
    tabpar.SetValue(i, p);
  }

  ShapeList newlov;
  std::sort(tabpar.begin(), tabpar.end());
  for (i = 1; i <= nv; i++)
  {
    Standard_Real       par = tabpar.Value(i);
    Standard_Integer    iv  = mappar.FindIndex(par);
    const TopoShape& v   = mapiv.Find(iv);
    newlov.Append(v);
  }
  lov.Clear();
  lov.Append(newlov);
}

//=================================================================================================

Standard_Boolean TOOL1::SplitE(const TopoEdge& Eanc, ShapeList& Splits)
{
  // prequesitory : <Eanc> is a valid edge.
  TopAbs_Orientation oEanc       = Eanc.Orientation();
  TopoShape       aLocalShape = Eanc.Oriented(TopAbs_FORWARD);
  TopoEdge        EFOR        = TopoDS::Edge(aLocalShape);
  //  TopoEdge EFOR = TopoDS::Edge(Eanc.Oriented(TopAbs_FORWARD));
  ShapeList lov;
  ShapeExplorer      exv(EFOR, TopAbs_VERTEX);
  for (; exv.More(); exv.Next())
  {
    const TopoShape& v = exv.Current();
    lov.Append(v);
  }
  Standard_Integer nv = lov.Extent();
  if (nv <= 2)
    return Standard_False;

  ::FUN_tool_sortVonE(lov, EFOR);

  TopoVertex                      v0;
  TopTools_ListIteratorOfListOfShape itlov(lov);
  if (itlov.More())
  {
    v0 = TopoDS::Vertex(itlov.Value());
    itlov.Next();
  }
  else
    return Standard_False;

  for (; itlov.More(); itlov.Next())
  {
    TopoVertex v = TopoDS::Vertex(itlov.Value());

    // prequesitory: par0 < par
    Standard_Real par0 = BRepInspector::Parameter(v0, EFOR);
    Standard_Real par  = BRepInspector::Parameter(v, EFOR);

    // here, ed has the same geometries than Ein, but with no subshapes.
    TopoEdge ed;
    FUN_ds_CopyEdge(EFOR, ed);
    ShapeBuilder BB;
    v0.Orientation(TopAbs_FORWARD);
    BB.Add(ed, v0);
    FUN_ds_Parameter(ed, v0, par0);
    v.Orientation(TopAbs_REVERSED);
    BB.Add(ed, v);
    FUN_ds_Parameter(ed, v, par);
    Splits.Append(ed.Oriented(oEanc));
    v0 = v;
  }
  return Standard_True;
}

//=================================================================================================

gp_Pnt2d TOOL1::UVF(const Standard_Real par, const TopOpeBRepTool_C2DF& C2DF)
{
  Standard_Real               f, l, tol;
  const Handle(GeomCurve2d)& PC = C2DF.PC(f, l, tol);
  gp_Pnt2d                    UV;
  PC->D0(par, UV);
  return UV;
}

//=================================================================================================

Standard_Boolean TOOL1::ParISO(const gp_Pnt2d&    uv,
                                             const TopoEdge& E,
                                             const TopoFace& F,
                                             Standard_Real&     par)
{
  par = 1.e7;
  Standard_Boolean isou, isov;
  gp_Dir2d         d2d;
  gp_Pnt2d         o2d;
  Standard_Boolean uviso = TOOL1::UVISO(E, F, isou, isov, d2d, o2d);
  if (!uviso)
    return Standard_False;
  if (isou)
  {
    par = (uv.Y() - o2d.Y());
    if (d2d.Y() < 0)
      par = -par;
  }
  if (isov)
  {
    par = (uv.X() - o2d.X());
    if (d2d.X() < 0)
      par = -par;
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::ParE2d(const gp_Pnt2d&    p2d,
                                             const TopoEdge& E,
                                             const TopoFace& F,
                                             Standard_Real&     par,
                                             Standard_Real&     dist)
{
  // Avoid projections if possible :
  BRepAdaptor_Curve2d         BC2d(E, F);
  GeomAbs_CurveType           CT  = BC2d.GetType();
  const Handle(GeomCurve2d)& C2d = BC2d.Curve();
  if (CT == GeomAbs_Line)
  {
    Standard_Boolean isoU, isoV;
    gp_Pnt2d         Loc;
    gp_Dir2d         dir2d;
    TOOL1::UVISO(C2d, isoU, isoV, dir2d, Loc);
    if (isoU)
    {
      par  = p2d.Y() - Loc.Y();
      dist = Abs(p2d.X() - Loc.X());
    }
    if (isoV)
    {
      par  = p2d.X() - Loc.X();
      dist = Abs(p2d.Y() - Loc.Y());
    }
    if (isoU || isoV)
      return Standard_True;
  }

  Geom2dAPI_ProjectPointOnCurve proj(p2d, C2d);
  dist = p2d.Distance(proj.NearestPoint());
  par  = proj.LowerDistanceParameter();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::TgINSIDE(const TopoVertex& v,
                                               const TopoEdge&   E,
                                               Vector3d&              Tg,
                                               Standard_Integer&    OvinE)
{
  TopoShape aLocalShape = E.Oriented(TopAbs_FORWARD);
  TopoEdge  EFOR        = TopoDS::Edge(aLocalShape);
  //  TopoEdge EFOR = TopoDS::Edge(E.Oriented(TopAbs_FORWARD));
  Standard_Integer ovE = TOOL1::OriinSor(v, EFOR, Standard_True);
  if (ovE == 0)
    return Standard_False;
  OvinE               = ovE;
  Standard_Integer iv = 0;
  if (ovE == CLOSING)
    iv = FORWARD;
  else if ((ovE == FORWARD) || (ovE == REVERSED))
    iv = ovE;
  Standard_Real parE;
  if (iv == 0)
    parE = BRepInspector::Parameter(v, E);
  else
    parE = TOOL1::ParE(iv, EFOR);
  Standard_Boolean ok = TOOL1::TggeomE(parE, EFOR, Tg);
  if (!ok)
    return Standard_False;
  if (ovE == REVERSED)
    Tg.Reverse();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::TggeomE(const Standard_Real      par,
                                              const BRepAdaptor_Curve& BC,
                                              Vector3d&                  Tg)
{
  // #ifdef OCCT_DEBUG
  //   GeomAbs_CurveType ct =
  // #endif
  //                          BC.GetType();
  // #ifdef OCCT_DEBUG
  //   Standard_Boolean apoles = (ct == GeomAbs_BezierCurve)||(ct == GeomAbs_BSplineCurve);
  // #endif

  Standard_Real f = BC.FirstParameter(), l = BC.LastParameter();
  Standard_Real tolE = BC.Tolerance();
  Standard_Real tolp = BC.Resolution(tolE);

  Standard_Boolean onf      = Abs(f - par) < tolp;
  Standard_Boolean onl      = Abs(l - par) < tolp;
  Standard_Boolean inbounds = (f < par) && (par < l);

  if ((!inbounds) && (!onf) && (!onl))
    return Standard_False;
  Standard_Real thepar = par;

  Point3d thepnt;
  BC.D1(thepar, thepnt, Tg);
  Tg.Normalize();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::TggeomE(const Standard_Real par,
                                              const TopoEdge&  E,
                                              Vector3d&             Tg)
{
  Standard_Boolean isdgE = BRepInspector::Degenerated(E);
  if (isdgE)
    return Standard_False;

  BRepAdaptor_Curve BC(E);
  // modified by NIZNHY-PKV Fri Aug  4 09:49:31 2000 f
  if (!CheckEdgeLength(E))
  {
    return Standard_False;
  }
  // modified by NIZNHY-PKV Fri Aug  4 09:49:36 2000 t

  return (TOOL1::TggeomE(par, BC, Tg));
}

//=================================================================================================

gp_Vec2d TOOL1::Tg2d(const Standard_Integer     iv,
                                   const TopoEdge&         E,
                                   const TopOpeBRepTool_C2DF& C2DF)
{
  Standard_Real               f, l, tol;
  const Handle(GeomCurve2d)& PC  = C2DF.PC(f, l, tol);
  Standard_Real               par = TOOL1::ParE(iv, E);
  gp_Pnt2d                    UV;
  gp_Vec2d                    tg2d;
  PC->D1(par, UV, tg2d);
  gp_Dir2d d2d(tg2d);
  return d2d;
}

//=================================================================================================

gp_Vec2d TOOL1::Tg2dApp(const Standard_Integer     iv,
                                      const TopoEdge&         E,
                                      const TopOpeBRepTool_C2DF& C2DF,
                                      const Standard_Real        factor)
{
  Standard_Real               f, l, tol;
  const Handle(GeomCurve2d)& PC = C2DF.PC(f, l, tol);

  Standard_Integer iOOv  = (iv == 1) ? 2 : 1;
  Standard_Real    par   = TOOL1::ParE(iv, E);
  Standard_Real    OOpar = TOOL1::ParE(iOOv, E);
  Standard_Real    parE  = (1 - factor) * par + factor * OOpar;

  gp_Pnt2d UV;
  gp_Vec2d tg2d;
  PC->D1(parE, UV, tg2d);
  gp_Dir2d d2d(tg2d);

  // modified by NIZHNY-MZV  Wed May 24 12:52:18 2000
  //   TopAbs_Orientation oE = E.Orientation();
  //   if (M_REVERSED(oE)) d2d.Reverse();
  // we remove this line because we want to know original tangent
  return d2d;
}

//=================================================================================================

gp_Vec2d TOOL1::tryTg2dApp(const Standard_Integer     iv,
                                         const TopoEdge&         E,
                                         const TopOpeBRepTool_C2DF& C2DF,
                                         const Standard_Real        factor)
{
  Standard_Real               f, l, tol;
  const Handle(GeomCurve2d)& PC     = C2DF.PC(f, l, tol);
  Standard_Boolean            isquad = FUN_tool_quad(PC);
  Standard_Boolean            line   = FUN_tool_line(PC);
  if (!isquad || line)
    return TOOL1::Tg2d(iv, E, C2DF);
  return TOOL1::Tg2dApp(iv, E, C2DF, factor);
}

//=================================================================================================

Standard_Integer TOOL1::tryOriEinF(const Standard_Real par,
                                                 const TopoEdge&  e,
                                                 const TopoFace&  f)
{
  // ------------------------------------------------------------
  // 1) <e> is a subshape of <f>
  // 2) else, compute oriEinF, using <e>'s 2d rep on <f>
  //    PREQUESITORY : <e> must have a pcurve on <f>.
  // ------------------------------------------------------------
  Standard_Boolean checkclo = Standard_True;
  Standard_Integer oeinf    = TOOL1::OriinSor(e, f, checkclo);
  if (oeinf != 0)
    return oeinf;

  Handle(GeomCurve2d) pc;
  Standard_Real        pf, pl, tol;
  Standard_Boolean     hasold = FC2D_HasOldCurveOnSurface(e, f, pc);
  if (!hasold)
    return 0;
  pc = FC2D_EditableCurveOnSurface(e, f, pf, pl, tol);

  // n2d is such that (p2d,oop2d) is oriented INSIDE F
  gp_Pnt2d uv;
  gp_Vec2d tg2d;
  pc->D1(par, uv, tg2d);
  gp_Vec2d n2d(gp_Dir2d(-tg2d.Y(), tg2d.X()));

  Standard_Real delta = TOOL1::minDUV(f);
  delta *= 1.e-1;
  gp_Pnt2d         ouv         = uv.Translated(delta * n2d);
  Standard_Boolean outuvbounds = TOOL1::outUVbounds(ouv, f);
  oeinf                        = (outuvbounds) ? 2 : 1;
  return oeinf;
}

//=================================================================================================

Standard_Boolean TOOL1::NgApp(const Standard_Real par,
                                            const TopoEdge&  e,
                                            const TopoFace&  f,
                                            const Standard_Real tola,
                                            Dir3d&             ngApp)
{
  // Give us an edge <e>, a face <f>, <e> has its geometry on <f>.
  //
  // P is the point of <par> on <e>
  // purpose : the compute of <neinsidef>, at a point P' on <F>, near P
  //           direction pp' is normal to <e>.
  // return false if the compute fails, or <neinsidef> is closed to <newneinsidef>
  //
  // PREQUESITORY : <e> must have a pcurve on <f>.
  // --------------

  Handle(GeomSurface) S = ShapeTool::BASISSURFACE(f);
  if (S.IsNull())
    return Standard_False;

  Standard_Boolean fplane = FUN_tool_plane(f);
  if (fplane)
    return Standard_False;

  // NYI : for bspline surfaces, use a evolutive parameter
  //       on curve to find out "significant" tangents
  Standard_Boolean fquad = FUN_tool_quad(f);
  if (!fquad)
    return Standard_False;
  // <pc> :
  Handle(GeomCurve2d) pc;
  Standard_Real        pf, pl, tol;
  Standard_Boolean     hasold = FC2D_HasOldCurveOnSurface(e, f, pc);
  if (!hasold)
    return Standard_False;
  pc = FC2D_EditableCurveOnSurface(e, f, pf, pl, tol);
  // <orieinf> :
  TopoShape     aLocalShape = f.Oriented(TopAbs_FORWARD);
  Standard_Integer orieinf     = TOOL1::tryOriEinF(par, e, TopoDS::Face(aLocalShape));
  //  Standard_Integer orieinf =
  //  TOOL1::tryOriEinF(par,e,TopoDS::Face(f.Oriented(TopAbs_FORWARD)));
  if (orieinf == 0)
    return Standard_False;
  // <uv> :
  gp_Pnt2d         uv;
  Standard_Boolean ok = FUN_tool_paronEF(e, par, f, uv);
  if (!ok)
    return Standard_False;
  // <ng> :
  Dir3d ng = FUN_tool_ngS(uv, S);
  if (!ok)
    return Standard_False;

  // <n2dinsideS> :
  gp_Vec2d tg2d;
  pc->D1(par, uv, tg2d);
  gp_Dir2d n2dinsideS = FUN_tool_nC2dINSIDES(gp_Dir2d(tg2d));
  if (orieinf == 2)
    n2dinsideS.Reverse();
  //<duv> : '
  Standard_Real eps = 0.45678;
  gp_Vec2d      duv = gp_Vec2d(n2dinsideS).Multiplied(eps);

  // cto009S4 : we need an iterative process to get other normal vector
  Standard_Integer nmax  = 5;
  Standard_Boolean same  = Standard_False;
  Standard_Real    delta = 0.45678;
  for (Standard_Integer i = 1; i <= nmax; i++)
  {

    gp_Pnt2d newuv       = uv.Translated(duv);
    Vector3d   newng       = FUN_tool_ngS(newuv, S);
    same                 = ng.IsEqual(newng, tola);
    Standard_Boolean okk = (newng.Magnitude() > tola);
    if (!same && okk)
    {
      ngApp = Dir3d(newng);
      break;
    }
    delta *= 1.25; //  NYI
    duv = gp_Vec2d(n2dinsideS).Multiplied(delta);

  } // i=1..nmax
  return !same;
}

//=================================================================================================

Standard_Boolean TOOL1::tryNgApp(const Standard_Real par,
                                               const TopoEdge&  e,
                                               const TopoFace&  f,
                                               const Standard_Real tola,
                                               Dir3d&             Ng)
{
  gp_Pnt2d         uv;
  Standard_Boolean ok = FUN_tool_paronEF(e, par, f, uv);
  if (!ok)
    return Standard_False;
  Dir3d ng(FUN_tool_nggeomF(uv, f));
#ifdef OCCT_DEBUG
  Dir3d ngApp;
#endif
  ok = TOOL1::NgApp(par, e, f, tola, Ng);
  if (!ok)
    Ng = ng;
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::IsQuad(const TopoEdge& E)
{
  BRepAdaptor_Curve bc(E);
  return (FUN_quadCT(bc.GetType()));
}

//=================================================================================================

Standard_Boolean TOOL1::IsQuad(const TopoFace& F)
{
  Handle(GeomSurface) S = ShapeTool::BASISSURFACE(F);
  return (FUN_tool_quad(S));
}

//=================================================================================================

Standard_Boolean TOOL1::CurvE(const TopoEdge&  E,
                                            const Standard_Real par,
                                            const Dir3d&       tg0,
                                            Standard_Real&      curv)
{
  curv = 0.;
  BRepAdaptor_Curve BAC(E);
  GeomAbs_CurveType CT   = BAC.GetType();
  Standard_Boolean  line = (CT == GeomAbs_Line);
  Standard_Real     tola = Precision::Angular() * 1.e3; // NYITOLXPU
  if (line)
  {
    Dir3d        dir = BAC.Line().Direction();
    Standard_Real dot = dir.Dot(tg0);
    if (Abs(1 - dot) < tola)
      return Standard_False;
    return Standard_True;
  }

  BRepLProp_CLProps clprops(BAC, par, 2, Precision::Confusion());
  Standard_Boolean  tgdef = clprops.IsTangentDefined();
  if (!tgdef)
    return Standard_False;
  curv = Abs(clprops.Curvature());

  Standard_Real    tol      = Precision::Confusion() * 1.e+2; // NYITOLXPU
  Standard_Boolean nullcurv = (curv < tol);
  if (nullcurv)
  {
    curv = 0.;
    return Standard_True;
  }

  Dir3d N;
  clprops.Normal(N);
  Dir3d T;
  clprops.Tangent(T);
  Dir3d        axis       = N ^ T;
  Standard_Real dot        = Abs(axis.Dot(tg0));
  nullcurv                 = dot < tola;
  Standard_Boolean maxcurv = Abs(1 - dot) < tola;
  if (nullcurv)
  {
    curv = 0.;
    return Standard_True;
  }
  if (maxcurv)
  {
    return Standard_True;
  }
  return Standard_False; // nyi general case
}

// ================================================================================
//   In 3d space, give us a curve <C> and a surface <S>,
//   <C> is tangent to <S> at point P0 = <uv0> on <S> ,
//   <tgC> = C's tangent at P0,
//   <ngS> = <S>'s normal at P0.

//   These define a plane thePlane = (O = P0, XY = (<ngS>,<tgC>)),
//   the projection of <S> in thePlane describes an apparent contour theContour.

//   In thePlane :
//   P0 -> p2d0
//   <ngS> -> 2d axis x
//   <tgC> -> 2d axis y

//   <C> -> C2d (same curvature)
//   <S>'s contour -> theContour
//   - the half3dspace described by (<S>,<ngS>) -> the half2dspace described by (theContour,x)

//   if (<tgC>.<ngS> = 0.) : (X,Y) are normal vectors
//                           (x,y) are normal vectors
// ================================================================================
static Standard_Boolean FUN_analyticcS(const gp_Pnt2d&             uv0,
                                       const Handle(GeomSurface)& S,
                                       const Dir3d&               ngS,
                                       const Dir3d&               tg0,
                                       Standard_Real&              curv,
                                       Standard_Boolean& direct) // dummy if !analyticcontour
{
  curv   = 0.;
  direct = Standard_True;
  // purpose : Returns true if theContour is analytic, and
  //           then computes its curvature <curv>.
  Handle(GeomSurface) su = ShapeTool::BASISSURFACE(S);
  if (S.IsNull())
    return Standard_True;
  GeomAdaptor_Surface GS(su);
  GeomAbs_SurfaceType ST    = GS.GetType();
  Standard_Boolean    plane = (ST == GeomAbs_Plane);
  Standard_Boolean    cyl   = (ST == GeomAbs_Cylinder);
  Standard_Boolean    cone  = (ST == GeomAbs_Cone);
  Standard_Boolean    sphe  = (ST == GeomAbs_Sphere);
  Standard_Boolean    torus = (ST == GeomAbs_Torus);

  Standard_Boolean curvdone = Standard_False;
  if (plane)
  {
    curv     = 0.;
    curvdone = Standard_True;
  }
  if (cyl || cone || torus)
  {
    Dir3d axis;
    if (cyl)
    {
      const gp_Cylinder& cycy = GS.Cylinder();
      axis                    = cycy.Axis().Direction();
      direct                  = cycy.Direct();
    }
    if (cone)
    {
      const gp_Cone& coco = GS.Cone();
      axis                = coco.Axis().Direction();
      direct              = coco.Direct();
    }
    if (torus)
    {
      const gp_Torus& toto = GS.Torus();
      axis                 = toto.Axis().Direction();
      direct               = toto.Direct();
    }
    Standard_Real    prod       = axis.Dot(tg0);
    Standard_Boolean isMaxAcurv = FUN_nullprodv(1 - Abs(prod));
    Standard_Boolean nullcurv   = FUN_nullprodv(prod);

    Standard_Real prod2 = ngS.Dot(tg0);
    if (cyl || cone)
      nullcurv = nullcurv || FUN_nullprodv(1 - Abs(prod2));

    if (nullcurv)
    {
      curv     = 0.;
      curvdone = Standard_True;
    }
    if (isMaxAcurv)
    {
      GeomLProp_SLProps slprops(S, uv0.X(), uv0.Y(), 2, Precision::Confusion());
      Standard_Boolean  curdef = slprops.IsCurvatureDefined();
      if (curdef)
      {
        Standard_Real    minAcurv = Abs(slprops.MinCurvature());
        Standard_Real    maxAcurv = Abs(slprops.MaxCurvature());
        Standard_Boolean isAmax   = (maxAcurv > minAcurv);
        curv                      = isAmax ? maxAcurv : minAcurv;
      }
      curvdone = Standard_True;
    }
  }
  if (sphe)
  {
    const gp_Sphere& spsp = GS.Sphere();
    curv                  = 1. / spsp.Radius();
    curvdone              = Standard_True;
    direct                = spsp.Direct();
  }

  return curvdone;
}

//=================================================================================================

Standard_Boolean TOOL1::CurvF(const TopoFace& F,
                                            const gp_Pnt2d&    uv,
                                            const Dir3d&      tg0,
                                            Standard_Real&     curv,
                                            Standard_Boolean&  direct)
{
  curv                     = 0.;
  Dir3d               ngS = FUN_tool_nggeomF(uv, F);
  Handle(GeomSurface) S   = ShapeTool::BASISSURFACE(F);
  if (S.IsNull())
    return Standard_False;
  // purpose : Computes theContour's curvature,
  //          returns false if the compute fails.

  Standard_Real tola = 1.e-6; // NYITOLXPU

  Standard_Boolean analyticcontour = FUN_analyticcS(uv, S, ngS, tg0, curv, direct);
  if (analyticcontour)
    return Standard_True;

  GeomLProp_SLProps slprops(S, uv.X(), uv.Y(), 2, Precision::Confusion());
  Standard_Boolean  curdef = slprops.IsCurvatureDefined();
  if (curdef)
  {
    Dir3d npl = tg0;

    Dir3d MaxD, MinD;
    slprops.CurvatureDirections(MaxD, MinD);
    Standard_Real mincurv = slprops.MinCurvature();
    Standard_Real maxcurv = slprops.MaxCurvature();

    Vector3d           Dmax = ngS ^ MaxD, Dmin = ngS ^ MinD; // xpu180898 : cto015G2
    Standard_Real    dotmax   = Dmax.Dot(npl);             // MaxD.Dot(npl); -xpu180898
    Standard_Boolean iscurmax = Abs(1 - dotmax) < tola;
    if (iscurmax)
    {
      direct = (maxcurv < 0.);
      curv   = Abs(maxcurv);
    }
    Standard_Real    dotmin   = Dmin.Dot(npl); // MinD.Dot(npl); -xpu180898
    Standard_Boolean iscurmin = Abs(1 - dotmin) < tola;
    if (iscurmin)
    {
      direct = (mincurv < 0.);
      curv   = Abs(mincurv);
    }
    curdef = iscurmax || iscurmin;
    // -------------
    // NYI : !curdef
    // -------------
  }
  return curdef;
}

//=================================================================================================

Standard_Boolean TOOL1::UVISO(const Handle(GeomCurve2d)& PC,
                                            Standard_Boolean&           isoU,
                                            Standard_Boolean&           isoV,
                                            gp_Dir2d&                   d2d,
                                            gp_Pnt2d&                   o2d)
{
  isoU = isoV = Standard_False;
  if (PC.IsNull())
    return Standard_False;
  Handle(GeomCurve2d)  LLL      = BASISCURVE2D(PC);
  Handle(TypeInfo) T2       = LLL->DynamicType();
  Standard_Boolean      isline2d = (T2 == STANDARD_TYPE(Geom2d_Line));
  if (!isline2d)
    return Standard_False;

  Handle(Geom2d_Line) L  = Handle(Geom2d_Line)::DownCast(LLL);
  d2d                    = L->Direction();
  isoU                   = (Abs(d2d.X()) < Precision::Parametric(Precision::Confusion()));
  isoV                   = (Abs(d2d.Y()) < Precision::Parametric(Precision::Confusion()));
  Standard_Boolean isoUV = isoU || isoV;
  if (!isoUV)
    return Standard_False;

  o2d = L->Location();
  return Standard_True;
}

Standard_Boolean TOOL1::UVISO(const TopoEdge& E,
                                            const TopoFace& F,
                                            Standard_Boolean&  isoU,
                                            Standard_Boolean&  isoV,
                                            gp_Dir2d&          d2d,
                                            gp_Pnt2d&          o2d)
{
  //  Standard_Real f,l,tol; Handle(GeomCurve2d) PC = FC2D_CurveOnSurface(E,F,f,l,tol);
  Handle(GeomCurve2d) PC;
  Standard_Real        f, l, tol;
  Standard_Boolean     hasold = FC2D_HasOldCurveOnSurface(E, F, PC);
  PC                          = FC2D_EditableCurveOnSurface(E, F, f, l, tol);
  if (!hasold)
    FC2D_AddNewCurveOnSurface(PC, E, F, f, l, tol);

  Standard_Boolean iso = UVISO(PC, isoU, isoV, d2d, o2d);
  return iso;
}

Standard_Boolean TOOL1::UVISO(const TopOpeBRepTool_C2DF& C2DF,
                                            Standard_Boolean&          isoU,
                                            Standard_Boolean&          isoV,
                                            gp_Dir2d&                  d2d,
                                            gp_Pnt2d&                  o2d)
{
  Standard_Real               f, l, tol;
  const Handle(GeomCurve2d)& PC = C2DF.PC(f, l, tol);
  // #ifdef OCCT_DEBUG
  //   const iso = UVISO(PC,isoU,isoV,d2d,o2d);
  // #else
  const Standard_Boolean iso = UVISO(PC, isoU, isoV, d2d, o2d);
  // #endif
  return iso;
}

//=================================================================================================

Standard_Boolean TOOL1::IsonCLO(const Handle(GeomCurve2d)& PC,
                                              const Standard_Boolean      onU,
                                              const Standard_Real         xfirst,
                                              const Standard_Real         xperiod,
                                              const Standard_Real         xtol)
{
  Standard_Boolean isou, isov;
  gp_Pnt2d         o2d;
  gp_Dir2d         d2d;
  Standard_Boolean isouv = UVISO(PC, isou, isov, d2d, o2d);
  if (!isouv)
    return Standard_False;
  Standard_Boolean onX = (onU && isou) || ((!onU) && isov);
  if (!onX)
    return Standard_False;
  Standard_Real dxx = 0;
  if (onU)
    dxx = Abs(o2d.X() - xfirst);
  else
    dxx = Abs(o2d.Y() - xfirst);

  Standard_Boolean onclo = (dxx < xtol);
  onclo                  = onclo || (Abs(xperiod - dxx) < xtol);
  return onclo;
}

Standard_Boolean TOOL1::IsonCLO(const TopOpeBRepTool_C2DF& C2DF,
                                              const Standard_Boolean     onU,
                                              const Standard_Real        xfirst,
                                              const Standard_Real        xperiod,
                                              const Standard_Real        xtol)
{
  Standard_Real               f, l, tol;
  const Handle(GeomCurve2d)& PC    = C2DF.PC(f, l, tol);
  Standard_Boolean            onclo = IsonCLO(PC, onU, xfirst, xperiod, xtol);
  return onclo;
}

//=================================================================================================

void TOOL1::TrslUV(const gp_Vec2d& t2d, TopOpeBRepTool_C2DF& C2DF)
{
  Standard_Real        f, l, tol;
  Handle(GeomCurve2d) PC = C2DF.PC(f, l, tol);
  PC->Translate(t2d);
  C2DF.SetPC(PC, f, l, tol);
}

Standard_Boolean TOOL1::TrslUVModifE(const gp_Vec2d&    t2d,
                                                   const TopoFace& F,
                                                   TopoEdge&       E)
{
  Standard_Real        f, l, tol;
  Handle(GeomCurve2d) PC = FC2D_CurveOnSurface(E, F, f, l, tol);
  //  Handle(GeomCurve2d) PC; Standard_Real f,l,tol;

  if (PC.IsNull())
    return Standard_False;
  PC->Translate(t2d);
  //  Handle(GeomCurve2d) toclear; BB.UpdateEdge(E,toclear,F,tole);
  ShapeBuilder BB;
  BB.UpdateEdge(E, PC, F, tol);
  return Standard_True;
}

//=================================================================================================

Standard_Real TOOL1::Matter(const Vector3d& d1, const Vector3d& dR2, const Vector3d& Ref)
{
  Vector3d d2 = dR2.Reversed();

  Standard_Real    tola  = Precision::Angular();
  Standard_Real    ang   = d1.Angle(d2);
  Standard_Boolean equal = (ang < tola);
  if (equal)
    return 0.;
  Standard_Boolean oppo = ((M_PI - ang) < tola);
  if (oppo)
    return M_PI;

  ang = d1.AngleWithRef(d2, Ref);
  if (ang < 0)
    ang = 2. * M_PI + ang;
  return ang;
}

//=================================================================================================

Standard_Real TOOL1::Matter(const gp_Vec2d& d1, const gp_Vec2d& dR2)
{
  Vector3d v1  = Vector3d(d1.X(), d1.Y(), 0.);
  Vector3d vR2 = Vector3d(dR2.X(), dR2.Y(), 0.);
  Vector3d Ref(0., 0., 1.);

  Standard_Real ang = TOOL1::Matter(v1, vR2, Ref);
  return ang;
}

//=================================================================================================

Standard_Boolean TOOL1::Matter(const Dir3d&       xx1,
                                             const Dir3d&       nt1,
                                             const Dir3d&       xx2,
                                             const Dir3d&       nt2,
                                             const Standard_Real tola,
                                             Standard_Real&      ang)
// purpose : the compute of MatterAng(f1,f2)
{
  // --------------------------------------------------
  // Give us a face f1 and one edge e of f1, pone=pnt(e,pare)
  // We project the problem in a plane normal to e, at point pone
  // ie we see the problem in space (x,y), with RONd (x,y,z), z tangent to e at pone.
  // RONd (x,y,z) = (xx1,nt1,x^y)
  //
  // Make the analogy :
  // f <-> Ef, e <-> Ve,
  // In view (x,y), f1 is seen as an edge Ef, e is seen as a vertex Ve,
  // the matter delimited by f can be seen as the one delimited by Ef.
  // --------------------------------------------------

  // Sign( (v1^nt1).z ) describes Ve's orientation in Ef1
  // (v1^nt1).z > 0. => Ve is oriented REVERSED in Ef1.
  // - ori(Ve,Ef1) == REVERSED : the matter delimited by <f1>
  //                              is (y<=0) in (x,y) 2d space -

  Dir3d           z1   = xx1 ^ nt1;
  Dir3d           z2   = xx2 ^ nt2;
  Standard_Real    dot  = z2.Dot(z1);
  Standard_Boolean oppo = (dot < 0.);
  if (!oppo)
    return Standard_False;

  // -nti points towards 3dmatter(fi)
  // => zi = xxi^nti gives the opposite sense for the compute of the matter angle
  z1.Reverse();
  ang = xx1.AngleWithRef(xx2, z1);
  if (Abs(ang) < tola)
  {
    ang = 0.;
    return Standard_True;
  }
  if (ang < 0)
    ang = 2. * M_PI + ang;

  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::Getduv(const TopoFace&  f,
                                             const gp_Pnt2d&     uv,
                                             const Vector3d&       dir,
                                             const Standard_Real factor,
                                             gp_Dir2d&           duv)
{
  Standard_Boolean quad = TOOL1::IsQuad(f);
  if (!quad)
    return Standard_False;
  Box2 bndf;
  BRepBndLib::AddClose(f, bndf);
  Standard_Real f1, f2, f3, l1, l2, l3;
  bndf.Get(f1, f2, f3, l1, l2, l3);
  Vector3d d123(f1 - l1, f2 - l2, f3 - l3);

  Point3d p;
  FUN_tool_value(uv, f, p);
  p.Translate(dir.Multiplied(factor));
  Standard_Real d;
  gp_Pnt2d      uvtr;
  FUN_tool_projPonF(p, f, uvtr, d);
  Standard_Real tolf = BRepInspector::Tolerance(f);
  tolf *= 1.e2; // NYIXPUTOL
  if (d > tolf)
    return Standard_False;

  gp_Vec2d             DUV(uv, uvtr);
  Handle(GeomSurface) S = ShapeTool::BASISSURFACE(f);
  if ((S->IsUPeriodic()) && (Abs(DUV.X()) > S->UPeriod() / 2.))
  {
    Standard_Real U1 = uv.X(), U2 = uvtr.X(), period = S->UPeriod();
    ElCLib1::AdjustPeriodic(0., period, Precision::PConfusion(), U1, U2);
    Standard_Real dx = U2 - U1;
    if (dx > period / 2.)
      dx -= period;
    DUV.SetX(dx);
  }
  if ((S->IsVPeriodic()) && (Abs(DUV.Y()) > S->VPeriod() / 2.))
  {
    Standard_Real V1 = uv.Y(), V2 = uvtr.Y(), period = S->VPeriod();
    ElCLib1::AdjustPeriodic(0., period, Precision::PConfusion(), V1, V2);
    Standard_Real dy = V2 - V1;
    if (dy > period / 2.)
      dy -= period;
    DUV.SetY(dy);
  }
  duv = gp_Dir2d(DUV);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::uvApp(const TopoFace&  f,
                                            const TopoEdge&  e,
                                            const Standard_Real pare,
                                            const Standard_Real eps,
                                            gp_Pnt2d&           uvapp)
{
  // uv :
  Standard_Boolean ok = FUN_tool_paronEF(e, pare, f, uvapp);
  if (!ok)
    return Standard_False;
  gp_Vec2d dxx;
  ok = FUN_tool_getdxx(f, e, pare, dxx);
  if (!ok)
    return Standard_False;
  uvapp.Translate(dxx.Multiplied(eps));
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::XX(const gp_Pnt2d&     uv,
                                         const TopoFace&  f,
                                         const Standard_Real par,
                                         const TopoEdge&  e,
                                         Dir3d&             XX)
{
  // ng(uv):
  Vector3d ng     = FUN_tool_nggeomF(uv, f);
  Vector3d geomxx = FUN_tool_getgeomxx(f, e, par, ng);

  Standard_Real    tol    = Precision::Confusion() * 1.e2; // NYITOL
  Standard_Boolean nullng = (geomxx.Magnitude() < tol);
  if (nullng)
    return Standard_False;

  TopAbs_Orientation oef;
  Standard_Boolean   ok = FUN_tool_orientEinFFORWARD(e, f, oef);
  if (!ok)
    return Standard_False;
  XX = Dir3d(geomxx);
  if (M_REVERSED(oef))
    XX.Reverse();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::Nt(const gp_Pnt2d& uv, const TopoFace& f, Dir3d& normt)
{
  Vector3d           nggeom;
  Standard_Boolean ok = TOOL1::NggeomF(uv, f, nggeom);
  if (!ok)
    return Standard_False;
  normt = Dir3d(nggeom);
  if (M_REVERSED(f.Orientation()))
    normt.Reverse();
  return Standard_True;
}

//=================================================================================================

static Standard_Boolean FUN_ngF(const gp_Pnt2d& uv, const TopoFace& F, Vector3d& ngF)
{
  BRepAdaptor_Surface bs(F);
  Standard_Real       tol3d = bs.Tolerance();
  Standard_Real       tolu  = bs.UResolution(tol3d);
  Standard_Real       tolv  = bs.VResolution(tol3d);

  // ###############################
  // nyi : all geometries are direct
  // ###############################
  Point3d p;
  Vector3d d1u, d1v;
  bs.D1(uv.X(), uv.Y(), p, d1u, d1v);

  Standard_Real delta = TOOL1::minDUV(F);
  delta *= 1.e-1;

  Standard_Real    du    = d1u.Magnitude();
  Standard_Real    dv    = d1v.Magnitude();
  Standard_Boolean kpart = (du < tolu) || (dv < tolv);
  if (kpart)
  {
    GeomAbs_SurfaceType ST = bs.GetType();
    if (ST == GeomAbs_Cone)
    {
      Standard_Boolean nullx = (Abs(uv.X()) < tolu);
      Standard_Boolean apex  = nullx && (Abs(uv.Y()) < tolv);
      if (apex)
      {
        const Dir3d axis = bs.Cone().Axis().Direction();
        Vector3d       ng(axis);
        ng.Reverse();
        ngF = ng;
        return Standard_True;
      }
      else if (du < tolu)
      {
        Standard_Real x = uv.X();

        Standard_Real y  = uv.Y();
        Standard_Real vf = bs.FirstVParameter();

        if (Abs(vf - y) < tolu)
          vf += delta;
        else
          vf -= delta;

        // modified by NIZHNY-MZV  Fri Nov 26 12:38:55 1999
        y = vf;
        bs.D1(x, y, p, d1u, d1v);
        Vector3d ng = d1u ^ d1v;

        ngF = ng;
        return Standard_True;
      }
    }
    if (ST == GeomAbs_Sphere)
    {
      Standard_Real    pisur2 = M_PI * .5;
      Standard_Real    u = uv.X(), v = uv.Y();
      Standard_Boolean vpisur2      = (Abs(v - pisur2) < tolv);
      Standard_Boolean vmoinspisur2 = (Abs(v + pisur2) < tolv);
      Standard_Boolean apex         = vpisur2 || vmoinspisur2;
      if (apex)
      {
        Point3d center = bs.Sphere().Location();
        Point3d value  = bs.Value(u, v);
        Vector3d ng(center, value);
        ngF = ng;
        return Standard_True;
      }
    }
#ifdef OCCT_DEBUG
    std::cout << "FUN_tool_nggeomF NYI" << std::endl;
#endif
    return Standard_False;
  } // kpart

  Dir3d udir(d1u);
  Dir3d vdir(d1v);
  ngF = Vector3d(Dir3d(udir ^ vdir));
  return Standard_True;
}

Standard_Boolean TOOL1::NggeomF(const gp_Pnt2d& uv, const TopoFace& f, Vector3d& ng)
{
  return FUN_ngF(uv, f, ng);
}

//=================================================================================================

Standard_Boolean TOOL1::Matter(const TopoFace&  f1,
                                             const TopoFace&  f2,
                                             const TopoEdge&  e,
                                             const Standard_Real par,
                                             const Standard_Real tola,
                                             Standard_Real&      ang)
{
  Dir3d xx1, xx2;
  Dir3d nt1, nt2;

  Standard_Real    tolf1 = BRepInspector::Tolerance(f1) * 1.e2; // nyitolxpu
  gp_Pnt2d         uv1;
  Standard_Boolean ok1 = FUN_tool_paronEF(e, par, f1, uv1, tolf1);
  if (!ok1)
    return Standard_False;
  ok1 = TOOL1::Nt(uv1, f1, nt1);
  if (!ok1)
    return Standard_False;
  ok1 = TOOL1::XX(uv1, f1, par, e, xx1);
  if (!ok1)
    return Standard_False;

  Standard_Real    tolf2 = BRepInspector::Tolerance(f2) * 2.e2; // nyitolxpu
  gp_Pnt2d         uv2;
  Standard_Boolean ok2 = FUN_tool_paronEF(e, par, f2, uv2, tolf2);
  if (!ok2)
    return Standard_False;
  ok2 = TOOL1::Nt(uv2, f2, nt2);
  if (!ok2)
    return Standard_False;
  ok2 = TOOL1::XX(uv2, f2, par, e, xx2);
  if (!ok2)
    return Standard_False;

  return (TOOL1::Matter(xx1, nt1, xx2, nt2, tola, ang));
}

//=================================================================================================

Standard_Boolean TOOL1::MatterKPtg(const TopoFace& f1,
                                                 const TopoFace& f2,
                                                 const TopoEdge& e,
                                                 Standard_Real&     ang)
{
  Standard_Real f, l;
  FUN_tool_bounds(e, f, l);
  Standard_Real x    = 0.45678;
  Standard_Real pare = (1 - x) * f + x * l;

  Standard_Real eps = 0.123; // NYIXPU190199

  // Standard_Real tola = Precision::Angular()*1.e3;

  gp_Pnt2d uv1;
  FUN_tool_paronEF(e, pare, f1, uv1);
  Dir3d           nt1;
  Standard_Boolean ok1 = TOOL1::Nt(uv1, f1, nt1);
  if (!ok1)
    return Standard_False;
  gp_Pnt2d uvapp1;
  ok1 = TOOL1::uvApp(f1, e, pare, eps, uvapp1);
  if (!ok1)
    return Standard_False;
  Point3d pf1;
  FUN_tool_value(uvapp1, f1, pf1);

  gp_Pnt2d         uv2;
  Standard_Real    d;
  Standard_Boolean ok2 = FUN_tool_projPonF(pf1, f2, uv2, d);
  Point3d           pf2;
  FUN_tool_value(uv2, f2, pf2);
  if (!ok2)
    return Standard_False;

  Dir3d        v12(Vector3d(pf1, pf2));
  Standard_Real dot = v12.Dot(nt1);
  ang               = (dot < 0.) ? 0. : 2. * M_PI;

  //  Dir3d nt1; ok1 = TOOL1::Nt(uv1,f1,nt1);
  //  if (!ok1) return Standard_False;
  //  Dir3d xx1; ok1 = TOOL1::XX(uv1,f1,pare,e,xx1);
  //  if (!ok1) return Standard_False;
  //  gp_Pnt2d uv2; Standard_Boolean ok2 = TOOL1::uvApp(f2,e,pare,eps,uv2);
  //  if (!ok2) return Standard_False;
  //  Dir3d nt2; ok2 = TOOL1::Nt(uv2,f2,nt2);
  //  if (!ok2) return Standard_False;
  //  Dir3d xx2; ok2 = TOOL1::XX(uv2,f2,pare,e,xx2);
  //  if (!ok2) return Standard_False;
  //  Standard_Real angapp; Standard_Boolean ok = TOOL1::Matter(xx1,nt1,
  //  xx2,nt2,tola,angapp); if (!ok) return Standard_False; Standard_Boolean is0 = (Abs(angapp) <
  //  Abs(2.*M_PI-angapp)); ang = is0 ? 0. : 2.*M_PI;
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::Getstp3dF(const Point3d&      p,
                                                const TopoFace& f,
                                                gp_Pnt2d&          uv,
                                                TopAbs_State&      st)
// classification solide de <P> / <F>
{
  st                  = TopAbs_UNKNOWN;
  Standard_Real tol3d = BRepInspector::Tolerance(f);
  // EXPENSIVE : calls an extrema
  Standard_Real    d;
  Standard_Boolean ok = FUN_tool_projPonF(p, f, uv, d);
  if (!ok)
    return Standard_False;
  if (d < tol3d)
  {
    st = TopAbs_ON;
    return Standard_True;
  }

  Point3d ppr;
  ok = FUN_tool_value(uv, f, ppr);
  if (!ok)
    return Standard_False;

  Dir3d ntf;
  ok = TOOL1::Nt(uv, f, ntf);
  if (!ok)
    return Standard_False;

  Dir3d           dppr(Vector3d(p, ppr));
  Standard_Real    dot   = dppr.Dot(ntf);
  Standard_Boolean isOUT = (dot < 0.);
  st                     = (isOUT ? TopAbs_OUT : TopAbs_IN);
  return Standard_True;
}

//=================================================================================================

void TOOL1::MkShell(const ShapeList& lF, TopoShape& She)
{
  ShapeBuilder BB;
  BB.MakeShell(TopoDS::Shell(She));
  for (TopTools_ListIteratorOfListOfShape li(lF); li.More(); li.Next())
    BB.Add(She, li.Value());
}

//=================================================================================================

Standard_Boolean TOOL1::Remove(ShapeList& loS,
                                             const TopoShape&   toremove)
{
  TopTools_ListIteratorOfListOfShape it(loS);
  Standard_Boolean                   found = Standard_False;
  while (it.More())
  {
    if (it.Value().IsEqual(toremove))
    {
      loS.Remove(it);
      found = Standard_True;
    }
    else
      it.Next();
  }
  return found;
}

//=================================================================================================

Standard_Real TOOL1::minDUV(const TopoFace& F)
{
  BRepAdaptor_Surface BS(F);
  Standard_Real       delta = BS.LastUParameter() - BS.FirstUParameter();
  Standard_Real       tmp   = BS.LastVParameter() - BS.FirstVParameter();
  delta                     = (tmp < delta) ? tmp : delta;
  return delta;
}

//=================================================================================================

#define INFFIRST (-1)
#define SUPLAST (-2)
#define ONFIRST (1)
#define ONLAST (2)

void TOOL1::stuvF(const gp_Pnt2d&    uv,
                                const TopoFace& f,
                                Standard_Integer&  onU,
                                Standard_Integer&  onV)
{
  BRepAdaptor_Surface bs(f);
  onU = onV          = 0;
  Standard_Real tolf = bs.Tolerance();
  Standard_Real tolu = bs.UResolution(tolf), tolv = bs.VResolution(tolf);
  Standard_Real u = uv.X(), v = uv.Y();
  Standard_Real uf = bs.FirstUParameter(), ul = bs.LastUParameter(), vf = bs.FirstVParameter(),
                vl      = bs.LastVParameter();
  Standard_Boolean onuf = (Abs(uf - u) < tolu), onul = (Abs(ul - u) < tolu);
  Standard_Boolean onvf = (Abs(vf - v) < tolv), onvl = (Abs(vl - v) < tolv);
  if (onuf)
    onU = ONFIRST;
  if (onul)
    onU = ONLAST;
  if (onvf)
    onV = ONFIRST;
  if (onvl)
    onV = ONLAST;
  if (u < (uf - tolu))
    onU = INFFIRST;
  if (u > (ul + tolu))
    onU = SUPLAST;
  if (v < (vf - tolv))
    onV = INFFIRST;
  if (v > (vl + tolv))
    onV = SUPLAST;
}

//=================================================================================================

Standard_Boolean TOOL1::outUVbounds(const gp_Pnt2d& uv, const TopoFace& F)
{
  BRepAdaptor_Surface BS(F);
  Standard_Boolean outofboundU = (uv.X() > BS.LastUParameter()) || (uv.X() < BS.FirstUParameter());
  Standard_Boolean outofboundV = (uv.Y() > BS.LastVParameter()) || (uv.Y() < BS.FirstVParameter());
  return outofboundU || outofboundV;
}

//=================================================================================================

Standard_Real TOOL1::TolUV(const TopoFace& F, const Standard_Real tol3d)
{
  BRepAdaptor_Surface bs(F);
  Standard_Real       tol2d = bs.UResolution(tol3d);
  tol2d                     = Max(tol2d, bs.VResolution(tol3d));
  return tol2d;
}

//=================================================================================================

Standard_Real TOOL1::TolP(const TopoEdge& E, const TopoFace& F)
{
  BRepAdaptor_Curve2d BC2d(E, F);
  return (BC2d.Resolution(BRepInspector::Tolerance(E)));
}

//=================================================================================================

Standard_Boolean TOOL1::WireToFace(const TopoFace&                        Fref,
                                                 const TopTools_DataMapOfShapeListOfShape& mapWlow,
                                                 ShapeList&                     lFs)
{
  ShapeBuilder BB;
  TopoShape aLocalShape = Fref.Oriented(TopAbs_FORWARD);
  TopoFace  F           = TopoDS::Face(aLocalShape);
  //  TopoFace F = TopoDS::Face(Fref.Oriented(TopAbs_FORWARD));
  Standard_Boolean                                    toreverse = M_REVERSED(Fref.Orientation());
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itm(mapWlow);
  for (; itm.More(); itm.Next())
  {
    TopoShape       FF = F.EmptyCopied();
    const TopoWire& wi = TopoDS::Wire(itm.Key());
    BB.Add(FF, wi);
    TopTools_ListIteratorOfListOfShape itw(itm.Value());
    for (; itw.More(); itw.Next())
    {
      const TopoWire& wwi = TopoDS::Wire(itw.Value());
      BB.Add(FF, wwi);
    }
    if (toreverse)
      FF.Orientation(TopAbs_REVERSED);
    lFs.Append(FF);
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean TOOL1::EdgeONFace(const Standard_Real par,
                                                 const TopoEdge&  ed,
                                                 const gp_Pnt2d&     uv,
                                                 const TopoFace&  fa,
                                                 Standard_Boolean&   isonfa)
{
  isonfa = Standard_False;
  // prequesitory : pnt(par,ed) = pnt(uv,f)
  Standard_Boolean dge = BRepInspector::Degenerated(ed);
  if (dge)
  {
    isonfa = Standard_True;
    return Standard_True;
  }

  Standard_Real    tola = Precision::Angular() * 1.e2; // NYITOLXPU
  Vector3d           tge;
  Standard_Boolean ok = TOOL1::TggeomE(par, ed, tge);
  if (!ok)
    return Standard_False;
  Vector3d           ngf      = FUN_tool_nggeomF(uv, fa);
  Standard_Real    aProdDot = tge.Dot(ngf);
  Standard_Boolean etgf     = Abs(aProdDot) < tola;
  if (!etgf)
    return Standard_True;

  BRepAdaptor_Surface bs(fa);
  GeomAbs_SurfaceType st       = bs.GetType();
  Standard_Boolean    plane    = (st == GeomAbs_Plane);
  Standard_Boolean    cylinder = (st == GeomAbs_Cylinder);

  BRepAdaptor_Curve bc(ed);
  GeomAbs_CurveType ct     = bc.GetType();
  Standard_Boolean  line   = (ct == GeomAbs_Line);
  Standard_Boolean  circle = (ct == GeomAbs_Circle);

  Standard_Real tole   = bc.Tolerance();
  Standard_Real tol1de = bc.Resolution(tole);
  Standard_Real tolf   = bs.Tolerance();
  Standard_Real tol3d  = Max(tole, tolf) * 1.e2; // NYITOLXPU

  // NYIxpu100299 : for other analytic geometries
  if (plane && line)
  {
    isonfa = Standard_True;
    return Standard_True;
  }
  if (plane)
  {
    Dir3d           ne;
    Standard_Boolean det = Standard_True;
    if (circle)
      ne = bc.Circle().Axis().Direction();
    else if (ct == GeomAbs_Ellipse)
      ne = bc.Ellipse().Axis().Direction();
    else if (ct == GeomAbs_Hyperbola)
      ne = bc.Hyperbola().Axis().Direction();
    else if (ct == GeomAbs_Parabola)
      ne = bc.Parabola().Axis().Direction();
    else
      det = Standard_False;
    if (det)
    {
      Standard_Real prod = ne.Dot(ngf);
      isonfa             = (Abs(1 - Abs(prod)) < tola);
      return Standard_True;
    }
  } // plane
  else if (cylinder)
  {
    Dir3d           ne;
    Standard_Boolean det = Standard_True;
    if (line)
      ne = tge;
    else if (circle)
      ne = bc.Circle().Axis().Direction();
    else
      det = Standard_False;
    Dir3d axicy = bs.Cylinder().Axis().Direction();

    if (det)
    {
      Standard_Real prod = ne.Dot(axicy);
      isonfa             = (Abs(1 - Abs(prod)) < tola);
      if (isonfa && circle)
      {
        Standard_Real radci = bc.Circle().Radius();
        Standard_Real radcy = bs.Cylinder().Radius();
        isonfa              = (Abs(radci - radcy) < tol3d);
      }
      return Standard_True;
    }
  } // cylinder

  // !!!!!!!!!!!!!!!! NOT STILL OK !!!!!!!!!!!!!!
  // projecting point of <ed> on <fa>
  Standard_Real x = 0.12345;
  Standard_Real f, l;
  FUN_tool_bounds(ed, f, l);
  Standard_Boolean onf  = (Abs(par - f) < tol1de);
  Standard_Real    opar = onf ? ((1 - x) * f + x * l) : ((1 - x) * f + x * par);
  Point3d           opc  = bc.Value(opar);

  gp_Pnt2d ouv;
  ok = FUN_tool_parF(ed, opar, fa, ouv, tolf);
  if (!ok)
    return Standard_False;
  Point3d ops = bs.Value(ouv.X(), ouv.Y());

  Standard_Real dd = opc.Distance(ops);
  isonfa           = (dd < tol3d);
  return Standard_True;
}
