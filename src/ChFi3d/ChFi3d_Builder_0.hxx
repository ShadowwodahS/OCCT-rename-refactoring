// Created on: 1994-03-24
// Created by: Isabelle GRIGNON
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

#ifndef ChFi3d_Builder_0_HeaderFile
#define ChFi3d_Builder_0_HeaderFile

#include <TopOpeBRepDS_SurfaceCurveInterference.hxx>
#include <TopOpeBRepDS_CurvePointInterference.hxx>
#include <TopOpeBRepDS_DataStructure.hxx>
#include <BRepBlend_Extremity.hxx>
#include <ChFiDS_Stripe.hxx>
#include <ChFiDS_SurfData.hxx>
#include <ChFiDS_Spine.hxx>
#include <ChFiDS_ElSpine.hxx>
#include <ChFiDS_CommonPoint.hxx>
#include <ChFiDS_Regularities.hxx>
#include <ChFiDS_FaceInterference.hxx>
#include <ChFiDS_Map.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopTools_ListOfShape.hxx>
#include <IntSurf_TypeTrans.hxx>
#include <GeomFill_BoundWithSurf.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Circle.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Adaptor3d_Curve.hxx>
#include <Adaptor3d_Surface.hxx>
#include <Bnd_Box.hxx>
#include <GeomAbs_Shape.hxx>
#include <gp_Pnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopTools_Array1OfShape.hxx>
#ifdef OCCT_DEBUG
  #include <OSD_Chronometer.hxx>
extern OSD_Chronometer simul, elspine, chemine;
#endif

Standard_Real ChFi3d_InPeriod(const Standard_Real U,
                              const Standard_Real UFirst,
                              const Standard_Real ULast,
                              const Standard_Real Eps);

void ChFi3d_Boite(const gp_Pnt2d& p1,
                  const gp_Pnt2d& p2,
                  Standard_Real&  mu,
                  Standard_Real&  Mu,
                  Standard_Real&  mv,
                  Standard_Real&  Mv);

void ChFi3d_Boite(const gp_Pnt2d& p1,
                  const gp_Pnt2d& p2,
                  const gp_Pnt2d& p3,
                  const gp_Pnt2d& p4,
                  Standard_Real&  Du,
                  Standard_Real&  Dv,
                  Standard_Real&  mu,
                  Standard_Real&  Mu,
                  Standard_Real&  mv,
                  Standard_Real&  Mv);

void ChFi3d_SetPointTolerance(TopOpeBRepDS_DataStructure& DStr,
                              const Bnd_Box&              box,
                              const Standard_Integer      IP);

void ChFi3d_EnlargeBox(const Handle(GeomCurve3d)& C,
                       const Standard_Real       wd,
                       const Standard_Real       wf,
                       Bnd_Box&                  box1,
                       Bnd_Box&                  box2);

void ChFi3d_EnlargeBox(const Handle(Adaptor3d_Surface)& S,
                       const Handle(GeomCurve2d)&      PC,
                       const Standard_Real              wd,
                       const Standard_Real              wf,
                       Bnd_Box&                         box1,
                       Bnd_Box&                         box2);

void ChFi3d_EnlargeBox(const TopoEdge&          E,
                       const ShapeList& LF,
                       const Standard_Real         w,
                       Bnd_Box&                    box);

void ChFi3d_EnlargeBox(TopOpeBRepDS_DataStructure&    DStr,
                       const Handle(ChFiDS_Stripe)&   st,
                       const Handle(ChFiDS_SurfData)& sd,
                       Bnd_Box&                       b1,
                       Bnd_Box&                       b2,
                       const Standard_Boolean         isfirst);

GeomAbs_Shape ChFi3d_evalconti(const TopoEdge& E, const TopoFace& F1, const TopoFace& F2);

void ChFi3d_conexfaces(const TopoEdge& E,
                       TopoFace&       F1,
                       TopoFace&       F2,
                       const ChFiDS_Map&  EFMap);

ChFiDS_State ChFi3d_EdgeState(TopoEdge* E, const ChFiDS_Map& EFMap);

Standard_Boolean ChFi3d_KParticular(const Handle(ChFiDS_Spine)& Spine,
                                    const Standard_Integer      IE,
                                    const BRepAdaptor_Surface&  S1,
                                    const BRepAdaptor_Surface&  S2);

void ChFi3d_BoundFac(BRepAdaptor_Surface&   S,
                     const Standard_Real    umin,
                     const Standard_Real    umax,
                     const Standard_Real    vmin,
                     const Standard_Real    vmax,
                     const Standard_Boolean checknaturalbounds = Standard_True);

void ChFi3d_BoundSrf(GeomAdaptor_Surface&   S,
                     const Standard_Real    umin,
                     const Standard_Real    umax,
                     const Standard_Real    vmin,
                     const Standard_Real    vmax,
                     const Standard_Boolean checknaturalbounds = Standard_True);

Standard_Boolean ChFi3d_InterPlaneEdge(const Handle(Adaptor3d_Surface)& Plan,
                                       const Handle(Adaptor3d_Curve)&   C,
                                       Standard_Real&                   W,
                                       const Standard_Boolean           Sens,
                                       const Standard_Real              tolc);

void ChFi3d_ExtrSpineCarac(const TopOpeBRepDS_DataStructure& DStr,
                           const Handle(ChFiDS_Stripe)&      cd,
                           const Standard_Integer            i,
                           const Standard_Real               p,
                           const Standard_Integer            jf,
                           const Standard_Integer            sens,
                           Point3d&                           P,
                           Vector3d&                           V,
                           Standard_Real&                    R);

Handle(GeomCircle) ChFi3d_CircularSpine(Standard_Real&      WFirst,
                                         Standard_Real&      WLast,
                                         const Point3d&       Pdeb,
                                         const Vector3d&       Vdeb,
                                         const Point3d&       Pfin,
                                         const Vector3d&       Vfin,
                                         const Standard_Real rad);

Handle(BezierCurve3d) ChFi3d_Spine(const Point3d&       pd,
                                      Vector3d&             vd,
                                      const Point3d&       pf,
                                      Vector3d&             vf,
                                      const Standard_Real R);

Handle(GeomFill_Boundary) ChFi3d_mkbound(const Handle(Adaptor3d_Surface)& Fac,
                                         Handle(GeomCurve2d)&            curv,
                                         const Standard_Integer           sens1,
                                         const gp_Pnt2d&                  pfac1,
                                         const gp_Vec2d&                  vfac1,
                                         const Standard_Integer           sens2,
                                         const gp_Pnt2d&                  pfac2,
                                         const gp_Vec2d&                  vfac2,
                                         const Standard_Real              t3d,
                                         const Standard_Real              ta);

Handle(GeomFill_Boundary) ChFi3d_mkbound(const Handle(Adaptor3d_Surface)& Surf,
                                         Handle(GeomCurve2d)&            curv,
                                         const Standard_Integer           sens1,
                                         const gp_Pnt2d&                  p1,
                                         Vector3d&                          v1,
                                         const Standard_Integer           sens2,
                                         const gp_Pnt2d&                  p2,
                                         Vector3d&                          v2,
                                         const Standard_Real              t3d,
                                         const Standard_Real              ta);

Handle(GeomFill_Boundary) ChFi3d_mkbound(const Handle(GeomSurface)& s,
                                         const gp_Pnt2d&             p1,
                                         const gp_Pnt2d&             p2,
                                         const Standard_Real         t3d,
                                         const Standard_Real         ta,
                                         const Standard_Boolean isfreeboundary = Standard_False);

Handle(GeomFill_Boundary) ChFi3d_mkbound(const Handle(Adaptor3d_Surface)& HS,
                                         const gp_Pnt2d&                  p1,
                                         const gp_Pnt2d&                  p2,
                                         const Standard_Real              t3d,
                                         const Standard_Real              ta,
                                         const Standard_Boolean isfreeboundary = Standard_False);

Handle(GeomFill_Boundary) ChFi3d_mkbound(const Handle(Adaptor3d_Surface)& HS,
                                         const Handle(GeomCurve2d)&      curv,
                                         const Standard_Real              t3d,
                                         const Standard_Real              ta,
                                         const Standard_Boolean isfreeboundary = Standard_False);

Handle(GeomFill_Boundary) ChFi3d_mkbound(const Handle(Adaptor3d_Surface)& Fac,
                                         Handle(GeomCurve2d)&            curv,
                                         const gp_Pnt2d&                  p1,
                                         const gp_Pnt2d&                  p2,
                                         const Standard_Real              t3d,
                                         const Standard_Real              ta,
                                         const Standard_Boolean isfreeboundary = Standard_False);

void ChFi3d_Coefficient(const Vector3d&  V3d,
                        const Vector3d&  D1u,
                        const Vector3d&  D1v,
                        Standard_Real& DU,
                        Standard_Real& DV);

Handle(GeomCurve2d) ChFi3d_BuildPCurve(const gp_Pnt2d&        p1,
                                        gp_Dir2d&              d1,
                                        const gp_Pnt2d&        p2,
                                        gp_Dir2d&              d2,
                                        const Standard_Boolean redresse = Standard_True);

Handle(GeomCurve2d) ChFi3d_BuildPCurve(const Handle(Adaptor3d_Surface)& Surf,
                                        const gp_Pnt2d&                  p1,
                                        const Vector3d&                    v1,
                                        const gp_Pnt2d&                  p2,
                                        const Vector3d&                    v2,
                                        const Standard_Boolean           redresse = Standard_False);

Handle(GeomCurve2d) ChFi3d_BuildPCurve(const Handle(Adaptor3d_Surface)& Surf,
                                        const gp_Pnt2d&                  p1,
                                        const gp_Vec2d&                  v1,
                                        const gp_Pnt2d&                  p2,
                                        const gp_Vec2d&                  v2,
                                        const Standard_Boolean           redresse = Standard_False);

Standard_Boolean ChFi3d_CheckSameParameter(const Handle(Adaptor3d_Curve)&   C3d,
                                           Handle(GeomCurve2d)&            Pcurv,
                                           const Handle(Adaptor3d_Surface)& S,
                                           const Standard_Real              tol3d,
                                           Standard_Real&                   tolreached);

Standard_Boolean ChFi3d_SameParameter(const Handle(Adaptor3d_Curve)&   C3d,
                                      Handle(GeomCurve2d)&            Pcurv,
                                      const Handle(Adaptor3d_Surface)& S,
                                      const Standard_Real              tol3d,
                                      Standard_Real&                   tolreached);

Standard_Boolean ChFi3d_SameParameter(const Handle(GeomCurve3d)&   C3d,
                                      Handle(GeomCurve2d)&       Pcurv,
                                      const Handle(GeomSurface)& S,
                                      const Standard_Real         Pardeb,
                                      const Standard_Real         Parfin,
                                      const Standard_Real         tol3d,
                                      Standard_Real&              tolreached);

void ChFi3d_ComputePCurv(const Handle(GeomCurve3d)&   C3d,
                         const gp_Pnt2d&             UV1,
                         const gp_Pnt2d&             UV2,
                         Handle(GeomCurve2d)&       Pcurv,
                         const Handle(GeomSurface)& S,
                         const Standard_Real         Pardeb,
                         const Standard_Real         Parfin,
                         const Standard_Real         tol3d,
                         Standard_Real&              tolreached,
                         const Standard_Boolean      reverse = Standard_False);

void ChFi3d_ComputePCurv(const Handle(Adaptor3d_Curve)&   C3d,
                         const gp_Pnt2d&                  UV1,
                         const gp_Pnt2d&                  UV2,
                         Handle(GeomCurve2d)&            Pcurv,
                         const Handle(Adaptor3d_Surface)& S,
                         const Standard_Real              Pardeb,
                         const Standard_Real              Parfin,
                         const Standard_Real              tol3d,
                         Standard_Real&                   tolreached,
                         const Standard_Boolean           reverse = Standard_False);

void ChFi3d_ComputePCurv(const gp_Pnt2d&        UV1,
                         const gp_Pnt2d&        UV2,
                         Handle(GeomCurve2d)&  Pcurv,
                         const Standard_Real    Pardeb,
                         const Standard_Real    Parfin,
                         const Standard_Boolean reverse = Standard_False);

Standard_Boolean ChFi3d_IntTraces(const Handle(ChFiDS_SurfData)& fd1,
                                  const Standard_Real            pref1,
                                  Standard_Real&                 p1,
                                  const Standard_Integer         jf1,
                                  const Standard_Integer         sens1,
                                  const Handle(ChFiDS_SurfData)& fd2,
                                  const Standard_Real            pref2,
                                  Standard_Real&                 p2,
                                  const Standard_Integer         jf2,
                                  const Standard_Integer         sens2,
                                  const gp_Pnt2d&                RefP2d,
                                  const Standard_Boolean         Check2dDistance = Standard_False,
                                  const Standard_Boolean         enlarge         = Standard_False);

Standard_Boolean ChFi3d_IsInFront(TopOpeBRepDS_DataStructure&  DStr,
                                  const Handle(ChFiDS_Stripe)& cd1,
                                  const Handle(ChFiDS_Stripe)& cd2,
                                  const Standard_Integer       i1,
                                  const Standard_Integer       i2,
                                  const Standard_Integer       sens1,
                                  const Standard_Integer       sens2,
                                  Standard_Real&               p1,
                                  Standard_Real&               p2,
                                  TopoFace&                 face,
                                  Standard_Boolean&            sameside,
                                  Standard_Integer&            jf1,
                                  Standard_Integer&            jf2,
                                  Standard_Boolean&            visavis,
                                  const TopoVertex&         Vtx,
                                  const Standard_Boolean       Check2dDistance = Standard_False,
                                  const Standard_Boolean       enlarge         = Standard_False);

void ChFi3d_ProjectPCurv(const Handle(Adaptor3d_Curve)&   HCg,
                         const Handle(Adaptor3d_Surface)& HSg,
                         Handle(GeomCurve2d)&            Pcurv,
                         const Standard_Real              tol3d,
                         Standard_Real&                   tolreached);

void ChFi3d_ReparamPcurv(const Standard_Real   Uf,
                         const Standard_Real   Ul,
                         Handle(GeomCurve2d)& Pcurv);

void ChFi3d_ComputeArete(const ChFiDS_CommonPoint&   P1,
                         const gp_Pnt2d&             UV1,
                         const ChFiDS_CommonPoint&   P2,
                         const gp_Pnt2d&             UV2,
                         const Handle(GeomSurface)& Surf,
                         Handle(GeomCurve3d)&         C3d,
                         Handle(GeomCurve2d)&       Pcurv,
                         Standard_Real&              Pardeb,
                         Standard_Real&              Parfin,
                         const Standard_Real         tol3d,
                         const Standard_Real         tol2d,
                         Standard_Real&              tolreached,
                         const Standard_Integer      IFlag);

Handle(TopOpeBRepDS_SurfaceCurveInterference) ChFi3d_FilCurveInDS(const Standard_Integer      Icurv,
                                                                  const Standard_Integer      Isurf,
                                                                  const Handle(GeomCurve2d)& Pcurv,
                                                                  const TopAbs_Orientation    Et);

TopAbs_Orientation ChFi3d_TrsfTrans(const IntSurf_TypeTrans T1);

Standard_EXPORT void ChFi3d_FilCommonPoint(const BRepBlend_Extremity& SP,
                                           const IntSurf_TypeTrans    TransLine,
                                           const Standard_Boolean     Start,
                                           ChFiDS_CommonPoint&        CP,
                                           const Standard_Real        Tol);

Standard_Integer ChFi3d_SolidIndex(const Handle(ChFiDS_Spine)& sp,
                                   TopOpeBRepDS_DataStructure& DStr,
                                   ChFiDS_Map&                 MapESo,
                                   ChFiDS_Map&                 MapESh);

Standard_Integer ChFi3d_IndexPointInDS(const ChFiDS_CommonPoint&   P1,
                                       TopOpeBRepDS_DataStructure& DStr);

Handle(TopOpeBRepDS_CurvePointInterference) ChFi3d_FilPointInDS(
  const TopAbs_Orientation Et,
  const Standard_Integer   Ic,
  const Standard_Integer   Ip,
  const Standard_Real      Par,
  const Standard_Boolean   IsVertex = Standard_False);

Handle(TopOpeBRepDS_CurvePointInterference) ChFi3d_FilVertexInDS(const TopAbs_Orientation Et,
                                                                 const Standard_Integer   Ic,
                                                                 const Standard_Integer   Ip,
                                                                 const Standard_Real      Par);

void ChFi3d_FilDS(const Standard_Integer       SolidIndex,
                  const Handle(ChFiDS_Stripe)& CorDat,
                  TopOpeBRepDS_DataStructure&  DStr,
                  ChFiDS_Regularities&         reglist,
                  const Standard_Real          tol3d,
                  const Standard_Real          tol2d);

void ChFi3d_StripeEdgeInter(const Handle(ChFiDS_Stripe)& theStripe1,
                            const Handle(ChFiDS_Stripe)& theStripe2,
                            TopOpeBRepDS_DataStructure&  DStr,
                            const Standard_Real          tol2d);

Standard_Integer ChFi3d_IndexOfSurfData(const TopoVertex&         V1,
                                        const Handle(ChFiDS_Stripe)& CD,
                                        Standard_Integer&            sens);

TopoEdge ChFi3d_EdgeFromV1(const TopoVertex&         V1,
                              const Handle(ChFiDS_Stripe)& CD,
                              Standard_Integer&            sens);

Standard_Real ChFi3d_ConvTol2dToTol3d(const Handle(Adaptor3d_Surface)& S,
                                      const Standard_Real              tol2d);

Standard_Boolean ChFi3d_ComputeCurves(const Handle(Adaptor3d_Surface)& S1,
                                      const Handle(Adaptor3d_Surface)& S2,
                                      const TColStd_Array1OfReal&      Pardeb,
                                      const TColStd_Array1OfReal&      Parfin,
                                      Handle(GeomCurve3d)&              C3d,
                                      Handle(GeomCurve2d)&            Pc1,
                                      Handle(GeomCurve2d)&            Pc2,
                                      const Standard_Real              tol3d,
                                      const Standard_Real              tol2d,
                                      Standard_Real&                   tolreached,
                                      const Standard_Boolean           wholeCurv = Standard_True);

Standard_Boolean ChFi3d_IntCS(const Handle(Adaptor3d_Surface)& S,
                              const Handle(Adaptor3d_Curve)&   C,
                              gp_Pnt2d&                        p2dS,
                              Standard_Real&                   wc);

void ChFi3d_ComputesIntPC(const ChFiDS_FaceInterference&     Fi1,
                          const ChFiDS_FaceInterference&     Fi2,
                          const Handle(GeomAdaptor_Surface)& HS1,
                          const Handle(GeomAdaptor_Surface)& HS2,
                          Standard_Real&                     UInt1,
                          Standard_Real&                     UInt2);

void ChFi3d_ComputesIntPC(const ChFiDS_FaceInterference&     Fi1,
                          const ChFiDS_FaceInterference&     Fi2,
                          const Handle(GeomAdaptor_Surface)& HS1,
                          const Handle(GeomAdaptor_Surface)& HS2,
                          Standard_Real&                     UInt1,
                          Standard_Real&                     UInt2,
                          Point3d&                            P);

Handle(GeomAdaptor_Surface) ChFi3d_BoundSurf(TopOpeBRepDS_DataStructure&    DStr,
                                             const Handle(ChFiDS_SurfData)& Fd1,
                                             const Standard_Integer&        IFaCo1,
                                             const Standard_Integer&        IFaArc1);

Standard_Integer ChFi3d_SearchPivot(Standard_Integer*   s,
                                    Standard_Real       u[3][3],
                                    const Standard_Real t);

Standard_Boolean ChFi3d_SearchFD(TopOpeBRepDS_DataStructure&  DStr,
                                 const Handle(ChFiDS_Stripe)& cd1,
                                 const Handle(ChFiDS_Stripe)& cd2,
                                 const Standard_Integer       sens1,
                                 const Standard_Integer       sens2,
                                 Standard_Integer&            i1,
                                 Standard_Integer&            i2,
                                 Standard_Real&               p1,
                                 Standard_Real&               p2,
                                 const Standard_Integer       ind1,
                                 const Standard_Integer       ind2,
                                 TopoFace&                 face,
                                 Standard_Boolean&            sameside,
                                 Standard_Integer&            jf1,
                                 Standard_Integer&            jf2);

void ChFi3d_Parameters(const Handle(GeomSurface)& S,
                       const Point3d&               p3d,
                       Standard_Real&              u,
                       Standard_Real&              v);

void ChFi3d_TrimCurve(const Handle(GeomCurve3d)&  gc,
                      const Point3d&              FirstP,
                      const Point3d&              LastP,
                      Handle(Geom_TrimmedCurve)& gtc);

Standard_EXPORT void ChFi3d_PerformElSpine(Handle(ChFiDS_ElSpine)& HES,
                                           Handle(ChFiDS_Spine)&   Spine,
                                           const GeomAbs_Shape     continuity,
                                           const Standard_Real     tol,
                                           const Standard_Boolean  IsOffset = Standard_False);

TopoFace ChFi3d_EnlargeFace(const Handle(ChFiDS_Spine)&        Spine,
                               const Handle(BRepAdaptor_Surface)& HS,
                               const Standard_Real                Tol);

void ChFi3d_cherche_face1(const ShapeList& map, const TopoFace& F1, TopoFace& F);

void ChFi3d_cherche_element(const TopoVertex& V,
                            const TopoEdge&   E1,
                            const TopoFace&   F1,
                            TopoEdge&         E,
                            TopoVertex&       Vtx);

Standard_Real ChFi3d_EvalTolReached(const Handle(Adaptor3d_Surface)& S1,
                                    const Handle(GeomCurve2d)&      pc1,
                                    const Handle(Adaptor3d_Surface)& S2,
                                    const Handle(GeomCurve2d)&      pc2,
                                    const Handle(GeomCurve3d)&        C);

void ChFi3d_cherche_edge(const TopoVertex&          V,
                         const TopTools_Array1OfShape& E1,
                         const TopoFace&            F1,
                         TopoEdge&                  E,
                         TopoVertex&                Vtx);

Standard_Integer ChFi3d_nbface(const ShapeList& mapVF);

void ChFi3d_edge_common_faces(const ShapeList& mapEF, TopoFace& F1, TopoFace& F2);

Standard_Real ChFi3d_AngleEdge(const TopoVertex& Vtx,
                               const TopoEdge&   E1,
                               const TopoEdge&   E2);

void ChFi3d_ChercheBordsLibres(const ChFiDS_Map&    myVEMap,
                               const TopoVertex& V1,
                               Standard_Boolean&    bordlibre,
                               TopoEdge&         edgelibre1,
                               TopoEdge&         edgelibre2);

Standard_Integer ChFi3d_NbNotDegeneratedEdges(const TopoVertex& Vtx, const ChFiDS_Map& VEMap);
Standard_Integer ChFi3d_NumberOfEdges(const TopoVertex& Vtx, const ChFiDS_Map& VEMap);

Standard_Integer ChFi3d_NumberOfSharpEdges(const TopoVertex& Vtx,
                                           const ChFiDS_Map&    VEMap,
                                           const ChFiDS_Map&    EFmap);

void ChFi3d_cherche_vertex(const TopoEdge& E1,
                           const TopoEdge& E2,
                           TopoVertex&     vertex,
                           Standard_Boolean&  trouve);

void ChFi3d_Couture(const TopoFace& F, Standard_Boolean& couture, TopoEdge& edgecouture);

void ChFi3d_CoutureOnVertex(const TopoFace&   F,
                            const TopoVertex& V,
                            Standard_Boolean&    couture,
                            TopoEdge&         edgecouture);

Standard_Boolean ChFi3d_IsPseudoSeam(const TopoEdge& E, const TopoFace& F);

Handle(BSplineCurve3d) ChFi3d_ApproxByC2(const Handle(GeomCurve3d)& C);

Standard_Boolean ChFi3d_IsSmooth(const Handle(GeomCurve3d)& C);

#endif
