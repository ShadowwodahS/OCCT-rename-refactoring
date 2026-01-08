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

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_CLProps.hxx>
#include <BRepTools.hxx>
#include <ElCLib.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pnt.hxx>
#include <Precision.hxx>
#include <Standard_ProgramError.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepTool_2d.hxx>
#include <TopOpeBRepTool_EXPORT.hxx>
#include <TopOpeBRepTool_ShapeTool.hxx>

//=================================================================================================

Standard_Real ShapeTool::Tolerance(const TopoShape& S)
{
  if (S.IsNull())
    return 0.;
  Standard_Real tol = 0;
  switch (S.ShapeType())
  {
    case TopAbs_FACE:
      tol = BRepInspector::Tolerance(TopoDS::Face(S));
      break;
    case TopAbs_EDGE:
      tol = BRepInspector::Tolerance(TopoDS::Edge(S));
      break;
    case TopAbs_VERTEX:
      tol = BRepInspector::Tolerance(TopoDS::Vertex(S));
      break;
    default:
      throw Standard_ProgramError("ShapeTool : Shape has no tolerance");
      break;
  }
  return tol;
}

//=================================================================================================

Point3d ShapeTool::Pnt(const TopoShape& S)
{
  if (S.ShapeType() != TopAbs_VERTEX)
  {
    throw Standard_ProgramError("ShapeTool::Pnt");
  }
  return BRepInspector::Pnt(TopoDS::Vertex(S));
}

#include <Geom_OffsetCurve.hxx>
#include <Geom_TrimmedCurve.hxx>

//=================================================================================================

Handle(GeomCurve3d) ShapeTool::BASISCURVE(const Handle(GeomCurve3d)& C)
{
  Handle(TypeInfo) T = C->DynamicType();
  if (T == STANDARD_TYPE(Geom_OffsetCurve))
    return BASISCURVE(Handle(Geom_OffsetCurve)::DownCast(C)->BasisCurve());
  else if (T == STANDARD_TYPE(Geom_TrimmedCurve))
    return BASISCURVE(Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve());
  else
    return C;
}

Handle(GeomCurve3d) ShapeTool::BASISCURVE(const TopoEdge& E)
{
  Standard_Real      f, l;
  Handle(GeomCurve3d) C = BRepInspector::Curve(E, f, l);
  if (C.IsNull())
    return C;
  return BASISCURVE(C);
}

#include <Geom_OffsetSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>

//=================================================================================================

Handle(GeomSurface) ShapeTool::BASISSURFACE(const Handle(GeomSurface)& S)
{
  Handle(TypeInfo) T = S->DynamicType();
  if (T == STANDARD_TYPE(Geom_OffsetSurface))
    return BASISSURFACE(Handle(Geom_OffsetSurface)::DownCast(S)->BasisSurface());
  else if (T == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
    return BASISSURFACE(Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface());
  else
    return S;
}

Handle(GeomSurface) ShapeTool::BASISSURFACE(const TopoFace& F)
{
  TopLoc_Location      L;
  Handle(GeomSurface) S = BRepInspector::Surface(F, L);
  return BASISSURFACE(S);
}

//=================================================================================================

void ShapeTool::UVBOUNDS(const Handle(GeomSurface)& S,
                                        Standard_Boolean&           UPeriodic,
                                        Standard_Boolean&           VPeriodic,
                                        Standard_Real&              Umin,
                                        Standard_Real&              Umax,
                                        Standard_Real&              Vmin,
                                        Standard_Real&              Vmax)
{
  const Handle(GeomSurface) BS = BASISSURFACE(S);
  Handle(TypeInfo)      T  = BS->DynamicType();

  if (T == STANDARD_TYPE(Geom_SurfaceOfRevolution))
  {
    Handle(Geom_SurfaceOfRevolution) SR = Handle(Geom_SurfaceOfRevolution)::DownCast(BS);
    Handle(GeomCurve3d)               C  = BASISCURVE(SR->BasisCurve());
    if (C->IsPeriodic())
    {
      UPeriodic = Standard_False;
      VPeriodic = Standard_True;
      Vmin      = C->FirstParameter();
      Vmax      = C->LastParameter();
    }
  }
  else if (T == STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion))
  {
    Handle(Geom_SurfaceOfLinearExtrusion) SE = Handle(Geom_SurfaceOfLinearExtrusion)::DownCast(BS);
    Handle(GeomCurve3d)                    C  = BASISCURVE(SE->BasisCurve());
    if (C->IsPeriodic())
    {
      UPeriodic = Standard_True;
      Umin      = C->FirstParameter();
      Umax      = C->LastParameter();
      VPeriodic = Standard_False;
    }
  }
  else
  {
    UPeriodic = BS->IsUPeriodic();
    VPeriodic = BS->IsVPeriodic();
    BS->Bounds(Umin, Umax, Vmin, Vmax);
  }
}

void ShapeTool::UVBOUNDS(const TopoFace& F,
                                        Standard_Boolean&  UPeriodic,
                                        Standard_Boolean&  VPeriodic,
                                        Standard_Real&     Umin,
                                        Standard_Real&     Umax,
                                        Standard_Real&     Vmin,
                                        Standard_Real&     Vmax)
{
  TopLoc_Location      L;
  Handle(GeomSurface) S = BRepInspector::Surface(F, L);
  UVBOUNDS(S, UPeriodic, VPeriodic, Umin, Umax, Vmin, Vmax);
}

//=================================================================================================

void ShapeTool::AdjustOnPeriodic(const TopoShape& F,
                                                Standard_Real&      u,
                                                Standard_Real&      v)
{
  TopoFace                FF = TopoDS::Face(F);
  TopLoc_Location            Loc;
  const Handle(GeomSurface) Surf = BRepInspector::Surface(FF, Loc);

  //  Standard_Real Ufirst,Ulast,Vfirst,Vlast;
  Standard_Boolean isUperio, isVperio;
  isUperio = Surf->IsUPeriodic();
  isVperio = Surf->IsVPeriodic();

  // exit if surface supporting F is not periodic on U or V
  if (!isUperio && !isVperio)
    return;

  Standard_Real UFfirst, UFlast, VFfirst, VFlast;
  BRepTools1::UVBounds(FF, UFfirst, UFlast, VFfirst, VFlast);

  Standard_Real tol = Precision1::PConfusion();

  if (isUperio)
  {
    Standard_Real Uperiod = Surf->UPeriod();

    //    Standard_Real ubid = UFfirst;

    //    ElCLib1::AdjustPeriodic(UFfirst,UFfirst + Uperiod,tol,ubid,u);
    if (Abs(u - UFfirst - Uperiod) > tol)
      u = ElCLib1::InPeriod(u, UFfirst, UFfirst + Uperiod);
  }
  if (isVperio)
  {
    Standard_Real Vperiod = Surf->VPeriod();

    //    Standard_Real vbid = VFfirst;

    //    ElCLib1::AdjustPeriodic(VFfirst,VFfirst + Vperiod,tol,vbid,v);
    if (Abs(v - VFfirst - Vperiod) > tol)
      v = ElCLib1::InPeriod(v, VFfirst, VFfirst + Vperiod);
  }
}

//=================================================================================================

Standard_Boolean ShapeTool::Closed(const TopoShape& S1, const TopoShape& S2)
{
  const TopoEdge& E          = TopoDS::Edge(S1);
  const TopoFace& F          = TopoDS::Face(S2);
  Standard_Boolean   brepclosed = BRepInspector::IsClosed(E, F);
  if (brepclosed)
  {
    Standard_Integer n = 0;
    for (ShapeExplorer x(F, TopAbs_EDGE); x.More(); x.Next())
      if (x.Current().IsSame(E))
        n++;
    if (n < 2)
      return Standard_False;
    else
      return Standard_True;
  }
  return Standard_False;
}

#ifdef OCCT_DEBUG
extern Standard_Boolean TopOpeBRepTool_GettraceVC();
extern Standard_Boolean TopOpeBRepTool_GettraceNYI();
#endif

inline Standard_Boolean PARINBOUNDS(const Standard_Real par,
                                    const Standard_Real first,
                                    const Standard_Real last,
                                    const Standard_Real tol)
{
  Standard_Boolean b = (((first + tol) <= par) && (par <= (last - tol)));
  return b;
}

inline Standard_Boolean PARONBOUND(const Standard_Real par,
                                   const Standard_Real bound,
                                   const Standard_Real tol)
{
  Standard_Boolean b = (((bound - tol) <= par) && (par <= (bound + tol)));
  return b;
}

Standard_Real ADJUST(const Standard_Real par,
                     const Standard_Real first,
                     const Standard_Real last,
                     const Standard_Real tol)
{
  Standard_Real period = last - first, periopar = par;

  if (PARINBOUNDS(par, first, last, tol))
  {
    periopar = par + period;
  }
  else if (PARONBOUND(par, first, tol))
  {
    periopar = par + period;
  }
  else if (PARONBOUND(par, last, tol))
  {
    periopar = par - period;
  }
  return periopar;
}

//=================================================================================================

Standard_Real ShapeTool::PeriodizeParameter(const Standard_Real par,
                                                           const TopoShape& EE,
                                                           const TopoShape& FF)
{
  Standard_Real periopar = par;
  if (!ShapeTool::Closed(EE, FF))
    return periopar;

  TopoEdge E = TopoDS::Edge(EE);
  TopoFace F = TopoDS::Face(FF);

  TopLoc_Location            Loc;
  const Handle(GeomSurface) Surf     = BRepInspector::Surface(F, Loc);
  Standard_Boolean           isUperio = Surf->IsUPeriodic();
  Standard_Boolean           isVperio = Surf->IsVPeriodic();
  if (!isUperio && !isVperio)
    return periopar;

  Standard_Real Ufirst, Ulast, Vfirst, Vlast;
  Surf->Bounds(Ufirst, Ulast, Vfirst, Vlast);

  Standard_Real              first, last, tolpc;
  const Handle(GeomCurve2d) PC = FC2D_CurveOnSurface(E, F, first, last, tolpc);
  if (PC.IsNull())
    throw Standard_ProgramError("ShapeTool::PeriodizeParameter : no 2d curve");

  Handle(TypeInfo) TheType = PC->DynamicType();
  if (TheType == STANDARD_TYPE(Geom2d_Line))
  {

    Handle(Geom2d_Line) HL(Handle(Geom2d_Line)::DownCast(PC));
    const gp_Dir2d&     D = HL->Direction();

    Standard_Real    tol  = Precision1::Angular();
    Standard_Boolean isoU = Standard_False, isoV = Standard_False;
    if (D.IsParallel(gp_Dir2d(0., 1.), tol))
      isoU = Standard_True;
    else if (D.IsParallel(gp_Dir2d(1., 0.), tol))
      isoV = Standard_True;
    if (isoU)
    {
      periopar = ADJUST(par, Ufirst, Ulast, tol);
    }
    else if (isoV)
    {
      periopar = ADJUST(par, Vfirst, Vlast, tol);
    }

#ifdef OCCT_DEBUG
    if (TopOpeBRepTool_GettraceVC())
    {
      std::cout << "ShapeTool PC on edge is ";
      if (isoU)
        std::cout << "isoU f,l " << Ufirst << " " << Ulast << std::endl;
      else if (isoV)
        std::cout << "isoV f,l " << Vfirst << " " << Vlast << std::endl;
      else
        std::cout << "not isoU, not isoV" << std::endl;
      std::cout << "par = " << par << " --> " << periopar << std::endl;
    }
#endif
  }
  // NYI : BSpline ...

  return periopar;
}

//=================================================================================================

Standard_Boolean ShapeTool::ShapesSameOriented(const TopoShape& S1,
                                                              const TopoShape& S2)
{
  Standard_Boolean so = Standard_True;

  Standard_Boolean sam = S1.IsSame(S2);
  if (sam)
  {
    const TopAbs_Orientation o1 = S1.Orientation();
    const TopAbs_Orientation o2 = S2.Orientation();
    if ((o1 == TopAbs_FORWARD || o1 == TopAbs_REVERSED)
        && (o2 == TopAbs_FORWARD || o2 == TopAbs_REVERSED))
    {
      so = (o1 == o2);
      return so;
    }
  }

  TopAbs_ShapeEnum t1 = S1.ShapeType(), t2 = S2.ShapeType();
  if ((t1 == TopAbs_SOLID) && (t2 == TopAbs_SOLID))
  {
    so = Standard_True;
  }
  else if ((t1 == TopAbs_FACE) && (t2 == TopAbs_FACE))
  {
    so = FacesSameOriented(S1, S2);
  }
  else if ((t1 == TopAbs_EDGE) && (t2 == TopAbs_EDGE))
  {
    so = EdgesSameOriented(S1, S2);
  }
  else if ((t1 == TopAbs_VERTEX) && (t2 == TopAbs_VERTEX))
  {
    TopAbs_Orientation o1 = S1.Orientation();
    TopAbs_Orientation o2 = S2.Orientation();
    if (o1 == TopAbs_EXTERNAL || o1 == TopAbs_INTERNAL || o2 == TopAbs_EXTERNAL
        || o2 == TopAbs_INTERNAL)
      so = Standard_True;
    else
      so = (o1 == o2);
  }

  return so;
}

//=================================================================================================

Standard_Boolean ShapeTool::SurfacesSameOriented(const BRepAdaptor_Surface& S1,
                                                                const BRepAdaptor_Surface& Sref)
{
  const BRepAdaptor_Surface& S2  = Sref;
  GeomAbs_SurfaceType        ST1 = S1.GetType();
  GeomAbs_SurfaceType        ST2 = S2.GetType();

  Standard_Boolean so = Standard_True;

  if (ST1 == GeomAbs_Plane && ST2 == GeomAbs_Plane)
  {

    Standard_Real u1 = S1.FirstUParameter();
    Standard_Real v1 = S1.FirstVParameter();
    Point3d        p1;
    Vector3d        d1u, d1v;
    S1.D1(u1, v1, p1, d1u, d1v);
    Vector3d n1 = d1u.Crossed(d1v);

    Standard_Real u2 = S2.FirstUParameter();
    Standard_Real v2 = S2.FirstVParameter();
    Point3d        p2;
    Vector3d        d2u, d2v;
    S2.D1(u2, v2, p2, d2u, d2v);
    Vector3d n2 = d2u.Crossed(d2v);

    Standard_Real d = n1.Dot(n2);
    so              = (d > 0.);
  }
  else if (ST1 == GeomAbs_Cylinder && ST2 == GeomAbs_Cylinder)
  {

    // On peut projeter n'importe quel point.
    // prenons donc l'origine
    Standard_Real u1 = 0.;
    Standard_Real v1 = 0.;
    Point3d        p1;
    Vector3d        d1u, d1v;
    S1.D1(u1, v1, p1, d1u, d1v);
    Vector3d n1 = d1u.Crossed(d1v);

    Handle(GeomSurface) HS2 = S2.Surface().Surface();
    HS2                      = Handle(GeomSurface)::DownCast(HS2->Transformed(S2.Trsf()));
    gp_Pnt2d         p22d;
    Standard_Real    dp2;
    Standard_Boolean ok = FUN_tool_projPonS(p1, HS2, p22d, dp2);
    if (!ok)
      return so; // NYI : raise

    Standard_Real u2 = p22d.X();
    Standard_Real v2 = p22d.Y();
    Point3d        p2;
    Vector3d        d2u, d2v;
    S2.D1(u2, v2, p2, d2u, d2v);
    Vector3d n2 = d2u.Crossed(d2v);

    Standard_Real d = n1.Dot(n2);
    so              = (d > 0.);
  }
  else
  {
    // prendre u1,v1 et projeter sur 2 pour calcul des normales
    // au meme point 3d.
#ifdef OCCT_DEBUG
    if (TopOpeBRepTool_GettraceNYI())
    {
      std::cout << "ShapeTool::SurfacesSameOriented surfaces non traitees : NYI";
      std::cout << std::endl;
    }
#endif
  }

  return so;
}

//=================================================================================================

Standard_Boolean ShapeTool::FacesSameOriented(const TopoShape& S1,
                                                             const TopoShape& Sref)
{
  const TopoShape& S2 = Sref;
  const TopoFace&  F1 = TopoDS::Face(S1);
  const TopoFace&  F2 = TopoDS::Face(S2);
  TopAbs_Orientation  o1 = F1.Orientation();
  TopAbs_Orientation  o2 = F2.Orientation();
  if (o1 == TopAbs_EXTERNAL || o1 == TopAbs_INTERNAL || o2 == TopAbs_EXTERNAL
      || o2 == TopAbs_INTERNAL)
  {
    return Standard_True;
  }

  Standard_Boolean    computerestriction = Standard_False;
  BRepAdaptor_Surface BAS1(F1, computerestriction);
  BRepAdaptor_Surface BAS2(F2, computerestriction);
  Standard_Boolean    so = F1.IsSame(F2) || SurfacesSameOriented(BAS1, BAS2);
  Standard_Boolean    b  = so;
  if (o1 != o2)
    b = !so;
  return b;
}

//=================================================================================================

Standard_Boolean ShapeTool::CurvesSameOriented(const BRepAdaptor_Curve& C1,
                                                              const BRepAdaptor_Curve& Cref)
{
  const BRepAdaptor_Curve& C2  = Cref;
  GeomAbs_CurveType        CT1 = C1.GetType();
  GeomAbs_CurveType        CT2 = C2.GetType();
  Standard_Boolean         so  = Standard_True;

  if (CT1 == GeomAbs_Line && CT2 == GeomAbs_Line)
  {
    Standard_Real p1 = C1.FirstParameter();
    Dir3d        t1, n1;
    Standard_Real c1;
    EdgeData(C1, p1, t1, n1, c1);
    Standard_Real p2 = C2.FirstParameter();
    Dir3d        t2, n2;
    Standard_Real c2;
    EdgeData(C2, p2, t2, n2, c2);
    Standard_Real d = t1.Dot(t2);
    so              = (d > 0.);
  }
  else
  {
    // prendre p1 et projeter sur 2 pour calcul des normales
    // au meme point 3d.
#ifdef OCCT_DEBUG
    if (TopOpeBRepTool_GettraceNYI())
    {
      std::cout << "ShapeTool::CurvesSameOriented non lineaires : NYI";
      std::cout << std::endl;
    }
#endif
  }

  return so;
}

//=================================================================================================

Standard_Boolean ShapeTool::EdgesSameOriented(const TopoShape& S1,
                                                             const TopoShape& Sref)
{
  const TopoShape& S2 = Sref;
  const TopoEdge&  E1 = TopoDS::Edge(S1);
  const TopoEdge&  E2 = TopoDS::Edge(S2);
  TopAbs_Orientation  o1 = E1.Orientation();
  TopAbs_Orientation  o2 = E2.Orientation();
  if (o1 == TopAbs_EXTERNAL || o1 == TopAbs_INTERNAL || o2 == TopAbs_EXTERNAL
      || o2 == TopAbs_INTERNAL)
  {
    return Standard_True;
  }
  BRepAdaptor_Curve BAC1(E1);
  BRepAdaptor_Curve BAC2(E2);
  Standard_Boolean  so = CurvesSameOriented(BAC1, BAC2);
  Standard_Boolean  b  = so;
  if (o1 != o2)
    b = !so;
  return b;
}

//=================================================================================================

Standard_Real ShapeTool::EdgeData(const BRepAdaptor_Curve& BAC,
                                                 const Standard_Real      P,
                                                 Dir3d&                  T,
                                                 Dir3d&                  N,
                                                 Standard_Real&           C)

{
  Standard_Real tol = Precision1::Angular();

  BRepLProp_CLProps BL(BAC, P, 2, tol);
  BL.Tangent(T);
  C = BL.Curvature();

  // xpu150399 cto900R4
  const Standard_Real     tol1 = Epsilon(0.);
  constexpr Standard_Real tol2 = RealLast();
  Standard_Real           tolm = Max(tol, Max(tol1, tol2));

  if (Abs(C) > tolm)
    BL.Normal(N);
  return tol;
}

//=================================================================================================

Standard_Real ShapeTool::EdgeData(const TopoShape& E,
                                                 const Standard_Real P,
                                                 Dir3d&             T,
                                                 Dir3d&             N,
                                                 Standard_Real&      C)
{
  BRepAdaptor_Curve BAC(TopoDS::Edge(E));
  Standard_Real     d = EdgeData(BAC, P, T, N, C);
  return d;
}

//=================================================================================================

Standard_Real ShapeTool::Resolution3dU(const Handle(GeomSurface)& SU,
                                                      const Standard_Real         Tol2d)
{
  GeomAdaptor_Surface GAS(SU);
  Standard_Real       r3dunit = 0.00001; // petite valeur (1.0 -> RangeError sur un tore)
  Standard_Real       ru      = GAS.UResolution(r3dunit);
  Standard_Real       r3du    = r3dunit * (Tol2d / ru);
  return r3du;
}

//=================================================================================================

Standard_Real ShapeTool::Resolution3dV(const Handle(GeomSurface)& SU,
                                                      const Standard_Real         Tol2d)
{
  GeomAdaptor_Surface GAS(SU);
  Standard_Real       r3dunit = 0.00001; // petite valeur (1.0 -> RangeError sur un tore)
  Standard_Real       rv      = GAS.VResolution(r3dunit);
  Standard_Real       r3dv    = r3dunit * (Tol2d / rv);
  return r3dv;
}

//=================================================================================================

Standard_Real ShapeTool::Resolution3d(const Handle(GeomSurface)& SU,
                                                     const Standard_Real         Tol2d)
{
  Standard_Real ru = Resolution3dU(SU, Tol2d);
  Standard_Real rv = Resolution3dV(SU, Tol2d);
  Standard_Real r  = Max(ru, rv);
  return r;
}

//=================================================================================================

Standard_Real ShapeTool::Resolution3d(const TopoFace&  F,
                                                     const Standard_Real Tol2d)
{
  TopLoc_Location             L;
  const Handle(GeomSurface)& SU = BRepInspector::Surface(F, L);
  Standard_Real               r  = Resolution3d(SU, Tol2d);
  return r;
}
