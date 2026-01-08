// Created on: 1993-02-15
// Created by: Laurent BOURESCHE
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

#include <BRepSweep_Rotation.hxx>

#include <GeomAdaptor_SurfaceOfRevolution.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <BRepTools_Quilt.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>
#include <gp_Ax22d.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Cone.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Lin.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_Trsf.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Sweep_NumShape.hxx>
#include <TopExp.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>

#include <TopExp_Explorer.hxx>

static Standard_Real ComputeTolerance(TopoEdge&                E,
                                      const TopoFace&          F,
                                      const Handle(GeomCurve2d)& C)

{
  if (BRepInspector::Degenerated(E))
    return BRepInspector::Tolerance(E);

  Standard_Real first, last;

  Handle(GeomSurface) surf = BRepInspector::Surface(F);
  Handle(GeomCurve3d)   c3d  = BRepInspector::Curve(E, first, last);

  Standard_Real    d2      = 0.;
  Standard_Integer nn      = 23;
  Standard_Real    unsurnn = 1. / nn;
  for (Standard_Integer i = 0; i <= nn; i++)
  {
    Standard_Real t     = unsurnn * i;
    Standard_Real u     = first * (1. - t) + last * t;
    Point3d        Pc3d  = c3d->Value(u);
    gp_Pnt2d      UV    = C->Value(u);
    Point3d        Pcons = surf->Value(UV.X(), UV.Y());
    if (Precision::IsInfinite(Pcons.X()) || Precision::IsInfinite(Pcons.Y())
        || Precision::IsInfinite(Pcons.Z()))
    {
      d2 = Precision::Infinite();
      break;
    }
    Standard_Real temp = Pc3d.SquareDistance(Pcons);
    if (temp > d2)
      d2 = temp;
  }
  d2 = 1.5 * sqrt(d2);
  if (d2 < 1.e-7)
    d2 = 1.e-7;
  return d2;
}

static void SetThePCurve(const ShapeBuilder&         B,
                         TopoEdge&                E,
                         const TopoFace&          F,
                         const TopAbs_Orientation    O,
                         const Handle(GeomCurve2d)& C)
{
  // check if there is already a pcurve
  Standard_Real        f, l;
  Handle(GeomCurve2d) OC;
  TopLoc_Location      SL;
  Handle(GeomPlane)   GP = Handle(GeomPlane)::DownCast(BRepInspector::Surface(F, SL));
  if (GP.IsNull())
    OC = BRepInspector::CurveOnSurface(E, F, f, l);
  if (OC.IsNull())
    B.UpdateEdge(E, C, F, ComputeTolerance(E, F, C));
  else
  {
    if (O == TopAbs_REVERSED)
      B.UpdateEdge(E, OC, C, F, ComputeTolerance(E, F, C));
    else
      B.UpdateEdge(E, C, OC, F, ComputeTolerance(E, F, C));
  }
}

//=================================================================================================

BRepSweep_Rotation::BRepSweep_Rotation(const TopoShape&    S,
                                       const Sweep_NumShape&  N,
                                       const TopLoc_Location& L,
                                       const Axis3d&          A,
                                       const Standard_Real    D,
                                       const Standard_Boolean C)
    : BRepSweep_Trsf(ShapeBuilder(), S, N, L, C),
      myAng(D),
      myAxe(A)

{
  Standard_ConstructionError_Raise_if(D < Precision::Angular(), "BRepSweep_Rotation::Constructor");
  Init();
}

//=================================================================================================

TopoShape BRepSweep_Rotation::MakeEmptyVertex(const TopoShape&   aGenV,
                                                 const Sweep_NumShape& aDirV)
{
  // call only in construction mode with copy.
  Standard_ConstructionError_Raise_if(!myCopy, "BRepSweep_Rotation::MakeEmptyVertex");
  Point3d        P = BRepInspector::Pnt(TopoDS::Vertex(aGenV));
  TopoVertex V;
  if (aDirV.Index() == 2)
    P.Transform(myLocation.Transformation());
  ////// modified by jgv, 1.10.01, for buc61005 //////
  // myBuilder.Builder().MakeVertex(V,P,Precision::Confusion());
  myBuilder.Builder().MakeVertex(V, P, BRepInspector::Tolerance(TopoDS::Vertex(aGenV)));
  ////////////////////////////////////////////////////
  if (aDirV.Index() == 1 && IsInvariant(aGenV) && myDirShapeTool.NbShapes1() == 3)
  {
    myBuiltShapes(myGenShapeTool.Index(aGenV), 3) = Standard_True;
    myShapes(myGenShapeTool.Index(aGenV), 3)      = V;
  }
  return V;
}

//=================================================================================================

TopoShape BRepSweep_Rotation::MakeEmptyDirectingEdge(const TopoShape& aGenV,
                                                        const Sweep_NumShape&)
{
  TopoEdge E;
  Point3d      P = BRepInspector::Pnt(TopoDS::Vertex(aGenV));
  Dir3d      Dirz(myAxe.Direction());
  Vector3d      V(Dirz);
  Point3d      O(myAxe.Location());
  O.Translate(V.Dot(Vector3d(O, P)) * V);
  if (O.IsEqual(P, Precision::Confusion()))
  {
    // make a degenerated edge
    // temporary make 3D curve null so that
    // parameters should be registered.
    // myBuilder.Builder().MakeEdge(E);
    Frame3d              Axis(O, Dirz);
    Handle(GeomCircle) GC = new GeomCircle(Axis, 0.);
    myBuilder.Builder().MakeEdge(E, GC, BRepInspector::Tolerance(TopoDS::Vertex(aGenV)));
    myBuilder.Builder().Degenerated(E, Standard_True);
  }
  else
  {
    Frame3d              Axis(O, Dirz, Dir3d(Vector3d(O, P)));
    Handle(GeomCircle) GC  = new GeomCircle(Axis, O.Distance(P));
    Standard_Real       tol = BRepInspector::Tolerance(TopoDS::Vertex(aGenV));
    myBuilder.Builder().MakeEdge(E, GC, tol);
  }
  return E;
}

//=================================================================================================

TopoShape BRepSweep_Rotation::MakeEmptyGeneratingEdge(const TopoShape&   aGenE,
                                                         const Sweep_NumShape& aDirV)
{
  // call in case of construction with copy, or only when meridian touches myaxe.
  TopoEdge E;
  if (BRepInspector::Degenerated(TopoDS::Edge(aGenE)))
  {
    myBuilder.Builder().MakeEdge(E);
    myBuilder.Builder().UpdateEdge(E, BRepInspector::Tolerance(TopoDS::Edge(aGenE)));
    myBuilder.Builder().Degenerated(E, Standard_True);
  }
  else
  {
    Standard_Real      First, Last;
    TopLoc_Location    Loc;
    Handle(GeomCurve3d) C =
      Handle(GeomCurve3d)::DownCast(BRepInspector::Curve(TopoDS::Edge(aGenE), Loc, First, Last)->Copy());
    if (!C.IsNull())
    {
      C->Transform(Loc.Transformation());
      if (aDirV.Index() == 2)
        C->Transform(myLocation.Transformation());
    }
    myBuilder.Builder().MakeEdge(E, C, BRepInspector::Tolerance(TopoDS::Edge(aGenE)));
  }
  if (aDirV.Index() == 1 && IsInvariant(aGenE) && myDirShapeTool.NbShapes1() == 3)
  {
    myBuiltShapes(myGenShapeTool.Index(aGenE), 3) = Standard_True;
    myShapes(myGenShapeTool.Index(aGenE), 3)      = E;
  }
  return E;
}

//=================================================================================================

void BRepSweep_Rotation::SetParameters(const TopoShape& aNewFace,
                                       TopoShape&       aNewVertex,
                                       const TopoShape& aGenF,
                                       const TopoShape& aGenV,
                                       const Sweep_NumShape&)
{
  // Glue the parameter of vertices directly included in cap faces.
  gp_Pnt2d pnt2d = BRepInspector::Parameters(TopoDS::Vertex(aGenV), TopoDS::Face(aGenF));
  myBuilder.Builder().UpdateVertex(TopoDS::Vertex(aNewVertex),
                                   pnt2d.X(),
                                   pnt2d.Y(),
                                   TopoDS::Face(aNewFace),
                                   Precision::PConfusion());
}

//=================================================================================================

void BRepSweep_Rotation::SetDirectingParameter(const TopoShape& aNewEdge,
                                               TopoShape&       aNewVertex,
                                               const TopoShape&,
                                               const Sweep_NumShape&,
                                               const Sweep_NumShape& aDirV)
{
  Standard_Real      param = 0;
  TopAbs_Orientation ori   = TopAbs_FORWARD;
  if (aDirV.Index() == 2)
  {
    param = myAng;
    ori   = TopAbs_REVERSED;
  }
  TopoVertex V_wnt = TopoDS::Vertex(aNewVertex);
  V_wnt.Orientation(ori);
  myBuilder.Builder().UpdateVertex(V_wnt, param, TopoDS::Edge(aNewEdge), Precision::PConfusion());
}

//=================================================================================================

void BRepSweep_Rotation::SetGeneratingParameter(const TopoShape& aNewEdge,
                                                TopoShape&       aNewVertex,
                                                const TopoShape& aGenE,
                                                const TopoShape& aGenV,
                                                const Sweep_NumShape&)
{
  TopoVertex vbid = TopoDS::Vertex(aNewVertex);
  vbid.Orientation(aGenV.Orientation());
  myBuilder.Builder().UpdateVertex(vbid,
                                   BRepInspector::Parameter(TopoDS::Vertex(aGenV), TopoDS::Edge(aGenE)),
                                   TopoDS::Edge(aNewEdge),
                                   Precision::PConfusion());
}

//=================================================================================================

TopoShape BRepSweep_Rotation::MakeEmptyFace(const TopoShape&   aGenS,
                                               const Sweep_NumShape& aDirS)
{
  Standard_Real        toler;
  TopoFace          F;
  Handle(GeomSurface) S;
  if (aGenS.ShapeType() == TopAbs_EDGE)
  {
    TopLoc_Location    L;
    Standard_Real      First, Last;
    Handle(GeomCurve3d) C = BRepInspector::Curve(TopoDS::Edge(aGenS), L, First, Last);
    toler                = BRepInspector::Tolerance(TopoDS::Edge(aGenS));
    Transform3d Tr           = L.Transformation();
    C                    = Handle(GeomCurve3d)::DownCast(C->Copy());
    //// modified by jgv, 9.12.03 ////
    C = new Geom_TrimmedCurve(C, First, Last);
    //////////////////////////////////
    C->Transform(Tr);

    Handle(GeomAdaptor_Curve) HC = new GeomAdaptor_Curve();
    HC->Load(C, First, Last);
    GeomAdaptor_SurfaceOfRevolution AS(HC, myAxe);
    switch (AS.GetType())
    {
      case GeomAbs_Plane: {
        Handle(GeomPlane) Pl = new GeomPlane(AS.Plane());
        S                     = Pl;
      }
      break;
      case GeomAbs_Cylinder: {
        Handle(Geom_CylindricalSurface) Cy = new Geom_CylindricalSurface(AS.Cylinder());
        S                                  = Cy;
      }
      break;
      case GeomAbs_Sphere: {
        Handle(Geom_SphericalSurface) Sp = new Geom_SphericalSurface(AS.Sphere());
        S                                = Sp;
      }
      break;
      case GeomAbs_Cone: {
        Handle(Geom_ConicalSurface) Co = new Geom_ConicalSurface(AS.Cone());
        S                              = Co;
      }
      break;
      case GeomAbs_Torus: {
        Handle(Geom_ToroidalSurface) To = new Geom_ToroidalSurface(AS.Torus());
        S                               = To;
      }
      break;
      default: {
        Handle(Geom_SurfaceOfRevolution) Se = new Geom_SurfaceOfRevolution(C, myAxe);
        S                                   = Se;
      }
      break;
    }
  }
  else
  {
    TopLoc_Location L;
    S          = BRepInspector::Surface(TopoDS::Face(aGenS), L);
    toler      = BRepInspector::Tolerance(TopoDS::Face(aGenS));
    Transform3d Tr = L.Transformation();
    S          = Handle(GeomSurface)::DownCast(S->Copy());
    S->Transform(Tr);
    if (aDirS.Index() == 2)
      S->Transform(myLocation.Transformation());
  }
  myBuilder.Builder().MakeFace(F, S, toler);
  return F;
}

//=================================================================================================

void BRepSweep_Rotation::SetPCurve(const TopoShape& aNewFace,
                                   TopoShape&       aNewEdge,
                                   const TopoShape& aGenF,
                                   const TopoShape& aGenE,
                                   const Sweep_NumShape&,
                                   const TopAbs_Orientation orien)
{
  // Set on edges of cap faces the same pcurves as
  // on edges of the generator face.
  Standard_Real First, Last;
  SetThePCurve(myBuilder.Builder(),
               TopoDS::Edge(aNewEdge),
               TopoDS::Face(aNewFace),
               orien,
               BRepInspector::CurveOnSurface(TopoDS::Edge(aGenE), TopoDS::Face(aGenF), First, Last));
}

//=================================================================================================

void BRepSweep_Rotation::SetGeneratingPCurve(const TopoShape& aNewFace,
                                             TopoShape&       aNewEdge,
                                             const TopoShape&,
                                             const Sweep_NumShape&,
                                             const Sweep_NumShape&    aDirV,
                                             const TopAbs_Orientation orien)
{
  TopLoc_Location     Loc;
  GeomAdaptor_Surface AS(BRepInspector::Surface(TopoDS::Face(aNewFace), Loc));
  Standard_Real       First, Last;
  Standard_Real       u, v;
  Point3d              point;
  gp_Pnt2d            pnt2d;
  gp_Dir2d            dir2d;
  gp_Lin2d            L;
  if (AS.GetType() == GeomAbs_Plane)
  {
    gp_Pln             pln = AS.Plane();
    Ax3             ax3 = pln.Position();
    Handle(GeomCurve3d) aC  = BRepInspector::Curve(TopoDS::Edge(aNewEdge), Loc, First, Last);
    Handle(GeomLine)  GL  = Handle(GeomLine)::DownCast(aC);
    if (GL.IsNull())
    {
      Handle(Geom_TrimmedCurve) aTrimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(aC);
      if (!aTrimmedCurve.IsNull())
      {
        GL = Handle(GeomLine)::DownCast(aTrimmedCurve->BasisCurve());
        if (GL.IsNull())
        {
          throw Standard_ConstructionError("BRepSweep_Rotation::SetGeneratingPCurve");
        }
      }
    }
    gp_Lin gl = GL->Lin();
    gl.Transform(Loc.Transformation());
    point      = gl.Location();
    Dir3d dir = gl.Direction();
    ElSLib1::PlaneParameters(ax3, point, u, v);
    pnt2d.SetCoord(u, v);
    dir2d.SetCoord(dir.Dot(ax3.XDirection()), dir.Dot(ax3.YDirection()));
    L.SetLocation(pnt2d);
    L.SetDirection(dir2d);
  }
  else if (AS.GetType() == GeomAbs_Torus)
  {
    gp_Torus          tor = AS.Torus();
    BRepAdaptor_Curve BC(TopoDS::Edge(aNewEdge));
    Standard_Real     U = BC.FirstParameter();
    point               = BC.Value(U);
    if (point.Distance(tor.Location()) < Precision::Confusion())
    {
      v = M_PI;
      //  modified by NIZHNY-EAP Wed Mar  1 17:49:29 2000 ___BEGIN___
      u = 0.;
    }
    else
    {
      ElSLib1::TorusParameters(tor.Position(), tor.MajorRadius(), tor.MinorRadius(), point, u, v);
    }
    //    u = 0.;
    v = ElCLib1::InPeriod(v, 0., 2 * M_PI);
    if ((2 * M_PI - v) <= Precision::PConfusion())
      v -= 2 * M_PI;
    if (aDirV.Index() == 2)
    {
      Standard_Real uLeft = u - myAng;
      ElCLib1::AdjustPeriodic(-M_PI, M_PI, Precision::PConfusion(), uLeft, u);
    }
    else
    {
      Standard_Real uRight = u + myAng;
      ElCLib1::AdjustPeriodic(-M_PI, M_PI, Precision::PConfusion(), u, uRight);
    }
    //  modified by NIZHNY-EAP Wed Mar  1 17:49:32 2000 ___END___
    pnt2d.SetCoord(u, v - U);
    L.SetLocation(pnt2d);
    L.SetDirection(gp1::DY2d());
  }
  else if (AS.GetType() == GeomAbs_Sphere)
  {
    Sphere3         sph = AS.Sphere();
    BRepAdaptor_Curve BC(TopoDS::Edge(aNewEdge));
    Standard_Real     U = BC.FirstParameter();
    point               = BC.Value(U);
    ElSLib1::SphereParameters(sph.Position(), sph.Radius(), point, u, v);
    u = 0.;
    if (aDirV.Index() == 2)
      u = myAng;
    pnt2d.SetCoord(u, v - U);
    L.SetLocation(pnt2d);
    L.SetDirection(gp1::DY2d());
  }
  else
  {
    Standard_Real anAngleTemp = 0;
    if (aDirV.Index() == 2)
      anAngleTemp = myAng;
    L.SetLocation(gp_Pnt2d(anAngleTemp, 0));
    L.SetDirection(gp1::DY2d());
  }
  Handle(Geom2d_Line) GL = new Geom2d_Line(L);
  SetThePCurve(myBuilder.Builder(), TopoDS::Edge(aNewEdge), TopoDS::Face(aNewFace), orien, GL);
}

//=================================================================================================

void BRepSweep_Rotation::SetDirectingPCurve(const TopoShape& aNewFace,
                                            TopoShape&       aNewEdge,
                                            const TopoShape& aGenE,
                                            const TopoShape& aGenV,
                                            const Sweep_NumShape&,
                                            const TopAbs_Orientation orien)
{
  TopLoc_Location      Loc;
  GeomAdaptor_Surface  AS(BRepInspector::Surface(TopoDS::Face(aNewFace), Loc));
  Standard_Real        par = BRepInspector::Parameter(TopoDS::Vertex(aGenV), TopoDS::Edge(aGenE));
  Point3d               p2  = BRepInspector::Pnt(TopoDS::Vertex(aGenV));
  gp_Pnt2d             p22d;
  Standard_Real        u, v;
  Handle(GeomCurve2d) thePCurve;

  switch (AS.GetType())
  {

    case GeomAbs_Plane: {
      gp_Pln        pln = AS.Plane();
      Ax3        ax3 = pln.Position();
      Point3d        p1  = pln.Location();
      Standard_Real R   = p1.Distance(p2);
      ElSLib1::PlaneParameters(ax3, p2, u, v);
      gp_Dir2d              dx2d(u, v);
      Ax22d              axe(gp1::Origin2d(), dx2d, gp1::DY2d());
      gp_Circ2d             C(axe, R);
      Handle(Geom2d_Circle) GC = new Geom2d_Circle(C);
      thePCurve                = GC;
    }
    break;

    case GeomAbs_Cone: {
      Cone1 cone = AS.Cone();
      ElSLib1::ConeParameters(cone.Position(), cone.RefRadius(), cone.SemiAngle(), p2, u, v);
      p22d.SetCoord(0., v);
      gp_Lin2d            L(p22d, gp1::DX2d());
      Handle(Geom2d_Line) GL = new Geom2d_Line(L);
      thePCurve              = GL;
    }
    break;

    case GeomAbs_Sphere: {
      Sphere3 sph = AS.Sphere();
      ElSLib1::SphereParameters(sph.Position(), sph.Radius(), p2, u, v);
      p22d.SetCoord(0., v);
      gp_Lin2d            L(p22d, gp1::DX2d());
      Handle(Geom2d_Line) GL = new Geom2d_Line(L);
      thePCurve              = GL;
    }
    break;

    case GeomAbs_Torus: {
      Point3d            p1;
      Standard_Real     u1, u2, v1, v2;
      gp_Torus          tor = AS.Torus();
      BRepAdaptor_Curve BC(TopoDS::Edge(aGenE));
      p1 = BC.Value(BC.FirstParameter());
      if (p1.Distance(tor.Location()) < Precision::Confusion())
      {
        v1 = M_PI;
        //  modified by NIZHNY-EAP Thu Mar  2 09:43:26 2000 ___BEGIN___
        u1 = 0.;
        //  modified by NIZHNY-EAP Thu Mar  2 15:28:59 2000 ___END___
      }
      else
      {
        ElSLib1::TorusParameters(tor.Position(), tor.MajorRadius(), tor.MinorRadius(), p1, u1, v1);
      }
      p2 = BC.Value(BC.LastParameter());
      if (p2.Distance(tor.Location()) < Precision::Confusion())
      {
        v2 = M_PI;
      }
      else
      {
        ElSLib1::TorusParameters(tor.Position(), tor.MajorRadius(), tor.MinorRadius(), p2, u2, v2);
      }
      ElCLib1::AdjustPeriodic(0., 2 * M_PI, Precision::PConfusion(), v1, v2);
      //  modified by NIZHNY-EAP Thu Mar  2 15:29:04 2000 ___BEGIN___
      u2 = u1 + myAng;
      ElCLib1::AdjustPeriodic(-M_PI, M_PI, Precision::PConfusion(), u1, u2);
      if (aGenV.Orientation() == TopAbs_FORWARD)
      {
        p22d.SetCoord(u1, v1);
      }
      else
      {
        p22d.SetCoord(u1, v2);
        //  modified by NIZHNY-EAP Thu Mar  2 09:43:32 2000 ___END___
      }
      gp_Lin2d            L(p22d, gp1::DX2d());
      Handle(Geom2d_Line) GL = new Geom2d_Line(L);
      thePCurve              = GL;
    }
    break;

    default: {
      p22d.SetCoord(0., par);
      gp_Lin2d            L(p22d, gp1::DX2d());
      Handle(Geom2d_Line) GL = new Geom2d_Line(L);
      thePCurve              = GL;
    }
    break;
  }
  SetThePCurve(myBuilder.Builder(),
               TopoDS::Edge(aNewEdge),
               TopoDS::Face(aNewFace),
               orien,
               thePCurve);
}

// modified by NIZNHY-PKV Tue Jun 14 08:33:55 2011f
//=================================================================================================

TopAbs_Orientation BRepSweep_Rotation::DirectSolid(const TopoShape& aGenS, const Sweep_NumShape&)
{ // compare the face normal and the direction
  Standard_Real       aU1, aU2, aV1, aV2, aUx, aVx, aX, aMV2, aTol2, aTx;
  TopAbs_Orientation  aOr;
  Point3d              aP;
  Vector3d              du, dv;
  BRepAdaptor_Surface surf(TopoDS::Face(aGenS));
  //
  aTol2 = Precision::Confusion();
  aTol2 = aTol2 * aTol2;
  //
  const Point3d& aPAxeLoc = myAxe.Location();
  const Dir3d& aPAxeDir = myAxe.Direction();
  //
  aU1 = surf.FirstUParameter();
  aU2 = surf.LastUParameter();
  aV1 = surf.FirstVParameter();
  aV2 = surf.LastVParameter();
  //
  aTx = 0.5;
  aUx = aTx * (aU1 + aU2);
  aVx = aTx * (aV1 + aV2);
  surf.D1(aUx, aVx, aP, du, dv);
  //
  Vector3d aV(aPAxeLoc, aP);
  aV.Cross(aPAxeDir);
  aMV2 = aV.SquareMagnitude();
  if (aMV2 < aTol2)
  {
    aTx = 0.43213918;
    aUx = aU1 * (1. - aTx) + aU2 * aTx;
    aVx = aV1 * (1. - aTx) + aV2 * aTx;
    surf.D1(aUx, aVx, aP, du, dv);
    aV.SetXYZ(aP.XYZ() - aPAxeLoc.XYZ());
    aV.Cross(aPAxeDir);
  }
  //
  aX  = aV.DotCross(du, dv);
  aOr = (aX > 0.) ? TopAbs_FORWARD : TopAbs_REVERSED;
  return aOr;
}

/*
//=================================================================================================

TopAbs_Orientation
  BRepSweep_Rotation::DirectSolid (const TopoShape& aGenS,
                   const Sweep_NumShape&)
{
  // compare the face normal and the direction
  BRepAdaptor_Surface surf(TopoDS::Face(aGenS));
  Point3d P;
  Vector3d du,dv;
  surf.D1((surf.FirstUParameter() + surf.LastUParameter()) / 2.,
      (surf.FirstVParameter() + surf.LastVParameter()) / 2.,
      P,du,dv);

  Vector3d V(myAxe.Location(),P);
  V.Cross(myAxe.Direction());
  Standard_Real x = V.DotCross(du,dv);
  TopAbs_Orientation orient = (x > 0) ? TopAbs_FORWARD : TopAbs_REVERSED;
  return orient;
}
*/
// modified by NIZNHY-PKV Tue Jun 14 08:33:59 2011t

//=================================================================================================

Standard_Boolean BRepSweep_Rotation::GGDShapeIsToAdd(const TopoShape&   aNewShape,
                                                     const TopoShape&   aNewSubShape,
                                                     const TopoShape&   aGenS,
                                                     const TopoShape&   aSubGenS,
                                                     const Sweep_NumShape& aDirS) const
{
  Standard_Boolean aRes = Standard_True;
  if (aNewShape.ShapeType() == TopAbs_FACE && aNewSubShape.ShapeType() == TopAbs_EDGE
      && aGenS.ShapeType() == TopAbs_EDGE && aSubGenS.ShapeType() == TopAbs_VERTEX
      && aDirS.Type() == TopAbs_EDGE)
  {
    TopLoc_Location     Loc;
    GeomAdaptor_Surface AS(BRepInspector::Surface(TopoDS::Face(aNewShape), Loc));
    if (AS.GetType() == GeomAbs_Plane)
    {
      return (!IsInvariant(aSubGenS));
    }
    else
    {
      return aRes;
    }
  }
  else
  {
    return aRes;
  }
}

//=================================================================================================

Standard_Boolean BRepSweep_Rotation::GDDShapeIsToAdd(const TopoShape&   aNewShape,
                                                     const TopoShape&   aNewSubShape,
                                                     const TopoShape&   aGenS,
                                                     const Sweep_NumShape& aDirS,
                                                     const Sweep_NumShape& aSubDirS) const
{
  if (aNewShape.ShapeType() == TopAbs_SOLID && aNewSubShape.ShapeType() == TopAbs_FACE
      && aGenS.ShapeType() == TopAbs_FACE && aDirS.Type() == TopAbs_EDGE
      && aSubDirS.Type() == TopAbs_VERTEX)
  {
    return (Abs(myAng - 2 * M_PI) > Precision::Angular());
  }
  else if (aNewShape.ShapeType() == TopAbs_FACE && aNewSubShape.ShapeType() == TopAbs_EDGE
           && aGenS.ShapeType() == TopAbs_EDGE && aDirS.Type() == TopAbs_EDGE
           && aSubDirS.Type() == TopAbs_VERTEX)
  {
    TopLoc_Location     Loc;
    GeomAdaptor_Surface AS(BRepInspector::Surface(TopoDS::Face(aNewShape), Loc));
    if (AS.GetType() == GeomAbs_Plane)
    {
      return (Abs(myAng - 2 * M_PI) > Precision::Angular());
    }
    else
    {
      return Standard_True;
    }
  }
  else
  {
    return Standard_True;
  }
}

//=================================================================================================

Standard_Boolean BRepSweep_Rotation::SeparatedWires(const TopoShape&   aNewShape,
                                                    const TopoShape&   aNewSubShape,
                                                    const TopoShape&   aGenS,
                                                    const TopoShape&   aSubGenS,
                                                    const Sweep_NumShape& aDirS) const
{
  if (aNewShape.ShapeType() == TopAbs_FACE && aNewSubShape.ShapeType() == TopAbs_EDGE
      && aGenS.ShapeType() == TopAbs_EDGE && aSubGenS.ShapeType() == TopAbs_VERTEX
      && aDirS.Type() == TopAbs_EDGE)
  {
    TopLoc_Location     Loc;
    GeomAdaptor_Surface AS(BRepInspector::Surface(TopoDS::Face(aNewShape), Loc));
    if (AS.GetType() == GeomAbs_Plane)
    {
      return (Abs(myAng - 2 * M_PI) <= Precision::Angular());
    }
    else
    {
      return Standard_False;
    }
  }
  else
  {
    return Standard_False;
  }
}

//=================================================================================================

TopoShape BRepSweep_Rotation::SplitShell(const TopoShape& aNewShape) const
{
  ShapeQuilt Q;
  Q.Add(aNewShape);
  return Q.Shells();
}

//=================================================================================================

Standard_Boolean BRepSweep_Rotation::HasShape(const TopoShape&   aGenS,
                                              const Sweep_NumShape& aDirS) const
{
  if (aDirS.Type() == TopAbs_EDGE && aGenS.ShapeType() == TopAbs_EDGE)
  {
    // Verify that the edge has entrails
    const TopoEdge& anEdge = TopoDS::Edge(aGenS);
    //
    if (BRepInspector::Degenerated(anEdge))
      return Standard_False;

    Standard_Real      aPFirst, aPLast;
    TopLoc_Location    aLoc;
    Handle(GeomCurve3d) aCurve = BRepInspector::Curve(anEdge, aLoc, aPFirst, aPLast);
    if (aCurve.IsNull())
      return Standard_False;

    if (IsInvariant(aGenS))
      return Standard_False;

    // Check seem edge
    ShapeExplorer FaceExp(myGenShape, TopAbs_FACE);
    for (; FaceExp.More(); FaceExp.Next())
    {
      TopoFace F = TopoDS::Face(FaceExp.Current());
      if (BRepTools1::IsReallyClosed(anEdge, F))
        return Standard_False;
    }

    return Standard_True;
  }
  else
  {
    return Standard_True;
  }
}

//=================================================================================================

Standard_Boolean BRepSweep_Rotation::IsInvariant(const TopoShape& aGenS) const
{
  if (aGenS.ShapeType() == TopAbs_EDGE)
  {
    BRepAdaptor_Curve aC(TopoDS::Edge(aGenS));
    if (aC.GetType() == GeomAbs_Line || aC.GetType() == GeomAbs_BSplineCurve
        || aC.GetType() == GeomAbs_BezierCurve)
    {
      TopoVertex V1, V2;
      TopExp1::Vertices(TopoDS::Edge(aGenS), V1, V2);
      if (IsInvariant(V1) && IsInvariant(V2))
      {
        if (aC.GetType() == GeomAbs_Line)
          return Standard_True;

        Standard_Real             aTol = Max(BRepInspector::Tolerance(V1), BRepInspector::Tolerance(V2));
        gp_Lin                    Lin(myAxe.Location(), myAxe.Direction());
        const TColgp_Array1OfPnt& aPoles =
          (aC.GetType() == GeomAbs_BSplineCurve ? aC.BSpline()->Poles() : aC.Bezier()->Poles());

        for (Standard_Integer i = aPoles.Lower(); i <= aPoles.Upper(); i++)
        {
          if (Lin.Distance(aPoles(i)) > aTol)
            return Standard_False;
        }
        return Standard_True;
      }
    }
  }
  else if (aGenS.ShapeType() == TopAbs_VERTEX)
  {
    Point3d P = BRepInspector::Pnt(TopoDS::Vertex(aGenS));
    gp_Lin Lin(myAxe.Location(), myAxe.Direction());
    return (Lin.Distance(P) <= BRepInspector::Tolerance(TopoDS::Vertex(aGenS)));
  }
  return Standard_False;
}

//=================================================================================================

Standard_Real BRepSweep_Rotation::Angle() const
{
  return myAng;
}

//=================================================================================================

Axis3d BRepSweep_Rotation::Axe() const
{
  return myAxe;
}
