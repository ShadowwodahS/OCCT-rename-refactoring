// Created on: 1995-01-04
// Created by: Bruno DUMORTIER
// Copyright (c) 1995-1999 Matra Datavision
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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeEdge2d.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <Extrema_ExtPC2d.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_Hyperbola.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_Parabola.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom_Plane.hxx>
#include <gp.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Hypr2d.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Parab2d.hxx>
#include <gp_Pnt2d.hxx>
#include <Precision.hxx>
#include <StdFail_NotDone.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

//=======================================================================
// function : Point
// purpose  : make a 3d point on the current plane
//=======================================================================
static Point3d Point(const gp_Pnt2d& P)
{
  return BRepLib::Plane1()->Value(P.X(), P.Y());
}

//=======================================================================
// function : Project
// purpose  : project a vertex on the current plane
//=======================================================================

static gp_Pnt2d Project(const TopoVertex& Ve)
{
  Point3d        P = BRepInspector::Pnt(Ve);
  Standard_Real U, V;
  ElSLib1::Parameters(BRepLib::Plane1()->Pln(), P, U, V);
  return gp_Pnt2d(U, V);
}

//=======================================================================
// function : Project
// purpose  : project a vertex on a curve
//=======================================================================

static Standard_Boolean Project(const Handle(GeomCurve2d)& C,
                                const TopoVertex&        V,
                                Standard_Real&              p)
{
  gp_Pnt2d            P = Project(V);
  Geom2dAdaptor_Curve AC(C);
  if (AC.GetType() == GeomAbs_Line)
  {
    p = ElCLib1::LineParameter(AC.Line().Position(), P);
  }
  else if (AC.GetType() == GeomAbs_Circle)
  {
    p = ElCLib1::CircleParameter(AC.Circle().Position(), P);
  }
  else
  {
    Extrema_ExtPC2d extrema(P, AC);
    if (extrema.IsDone())
    {
      Standard_Integer i, n = extrema.NbExt();

      Standard_Real d2 = RealLast();
      for (i = 1; i <= n; i++)
      {
        // OCC16852:if (extrema.IsMin(i)) {
        const Standard_Real dd2 = extrema.SquareDistance(i);
        if (dd2 < d2)
        {
          d2 = dd2;
          p  = extrema.Point(i).Parameter();
        }
        // OCC16852:}
      }
    }
    else
      return Standard_False;
  }
  return Standard_True;
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const TopoVertex& V1, const TopoVertex& V2)
{
  gp_Pnt2d      P1 = Project(V1);
  gp_Pnt2d      P2 = Project(V2);
  Standard_Real l  = P1.Distance(P2);
  if (l <= gp1::Resolution())
  {
    myError = BRepLib_LineThroughIdenticPoints;
    return;
  }
  gp_Lin2d            L(P1, gp_Vec2d(P1, P2));
  Handle(Geom2d_Line) GL = new Geom2d_Line(L);
  Init(GL, V1, V2, 0, l);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{
  Standard_Real l = P1.Distance(P2);
  if (l <= gp1::Resolution())
  {
    myError = BRepLib_LineThroughIdenticPoints;
    return;
  }
  gp_Lin2d            L(P1, gp_Vec2d(P1, P2));
  Handle(Geom2d_Line) GL = new Geom2d_Line(L);
  Init(GL, P1, P2, 0, l);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Lin2d& L)
{
  Handle(Geom2d_Line) GL = new Geom2d_Line(L);
  Init(GL);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Lin2d&     L,
                                       const Standard_Real p1,
                                       const Standard_Real p2)
{
  Handle(Geom2d_Line) GL = new Geom2d_Line(L);
  Init(GL, p1, p2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Lin2d& L, const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{
  Handle(Geom2d_Line) GL = new Geom2d_Line(L);
  Init(GL, P1, P2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Lin2d&      L,
                                       const TopoVertex& V1,
                                       const TopoVertex& V2)
{
  Handle(Geom2d_Line) GL = new Geom2d_Line(L);
  Init(GL, V1, V2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Circ2d& C)
{
  Handle(Geom2d_Circle) GC = new Geom2d_Circle(C);
  Init(GC);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Circ2d&    C,
                                       const Standard_Real p1,
                                       const Standard_Real p2)
{
  Handle(Geom2d_Circle) GC = new Geom2d_Circle(C);
  Init(GC, p1, p2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Circ2d& C, const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{
  Handle(Geom2d_Circle) GC = new Geom2d_Circle(C);
  Init(GC, P1, P2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Circ2d&     C,
                                       const TopoVertex& V1,
                                       const TopoVertex& V2)
{
  Handle(Geom2d_Circle) GC = new Geom2d_Circle(C);
  Init(GC, V1, V2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Elips2d& E)
{
  Handle(Geom2d_Ellipse) GE = new Geom2d_Ellipse(E);
  Init(GE);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Elips2d&   E,
                                       const Standard_Real p1,
                                       const Standard_Real p2)
{
  Handle(Geom2d_Ellipse) GE = new Geom2d_Ellipse(E);
  Init(GE, p1, p2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Elips2d& E, const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{
  Handle(Geom2d_Ellipse) GE = new Geom2d_Ellipse(E);
  Init(GE, P1, P2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Elips2d&    E,
                                       const TopoVertex& V1,
                                       const TopoVertex& V2)
{
  Handle(Geom2d_Ellipse) GE = new Geom2d_Ellipse(E);
  Init(GE, V1, V2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Hypr2d& H)
{
  Handle(Geom2d_Hyperbola) GH = new Geom2d_Hyperbola(H);
  Init(GH);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Hypr2d&    H,
                                       const Standard_Real p1,
                                       const Standard_Real p2)
{
  Handle(Geom2d_Hyperbola) GH = new Geom2d_Hyperbola(H);
  Init(GH, p1, p2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Hypr2d& H, const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{
  Handle(Geom2d_Hyperbola) GH = new Geom2d_Hyperbola(H);
  Init(GH, P1, P2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Hypr2d&     H,
                                       const TopoVertex& V1,
                                       const TopoVertex& V2)
{
  Handle(Geom2d_Hyperbola) GH = new Geom2d_Hyperbola(H);
  Init(GH, V1, V2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Parab2d& P)
{
  Handle(Geom2d_Parabola) GP = new Geom2d_Parabola(P);
  Init(GP);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Parab2d&   P,
                                       const Standard_Real p1,
                                       const Standard_Real p2)
{
  Handle(Geom2d_Parabola) GP = new Geom2d_Parabola(P);
  Init(GP, p1, p2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Parab2d& P, const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{
  Handle(Geom2d_Parabola) GP = new Geom2d_Parabola(P);
  Init(GP, P1, P2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const gp_Parab2d&    P,
                                       const TopoVertex& V1,
                                       const TopoVertex& V2)
{
  Handle(Geom2d_Parabola) GP = new Geom2d_Parabola(P);
  Init(GP, V1, V2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const Handle(GeomCurve2d)& L)
{
  Init(L);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const Handle(GeomCurve2d)& L,
                                       const Standard_Real         p1,
                                       const Standard_Real         p2)
{
  Init(L, p1, p2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const Handle(GeomCurve2d)& L,
                                       const gp_Pnt2d&             P1,
                                       const gp_Pnt2d&             P2)
{
  Init(L, P1, P2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const Handle(GeomCurve2d)& L,
                                       const TopoVertex&        V1,
                                       const TopoVertex&        V2)
{
  Init(L, V1, V2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const Handle(GeomCurve2d)& L,
                                       const gp_Pnt2d&             P1,
                                       const gp_Pnt2d&             P2,
                                       const Standard_Real         p1,
                                       const Standard_Real         p2)
{
  Init(L, P1, P2, p1, p2);
}

//=================================================================================================

BRepLib_MakeEdge2d::BRepLib_MakeEdge2d(const Handle(GeomCurve2d)& L,
                                       const TopoVertex&        V1,
                                       const TopoVertex&        V2,
                                       const Standard_Real         p1,
                                       const Standard_Real         p2)
{
  Init(L, V1, V2, p1, p2);
}

//=================================================================================================

void BRepLib_MakeEdge2d::Init(const Handle(GeomCurve2d)& C)
{
  Init(C, C->FirstParameter(), C->LastParameter());
}

//=================================================================================================

void BRepLib_MakeEdge2d::Init(const Handle(GeomCurve2d)& C,
                              const Standard_Real         p1,
                              const Standard_Real         p2)
{
  //  ShapeBuilder B;

  TopoVertex V1, V2;
  Init(C, V1, V2, p1, p2);
}

//=================================================================================================

void BRepLib_MakeEdge2d::Init(const Handle(GeomCurve2d)& C, const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{
  ShapeBuilder  B;
  TopoVertex V1, V2;
  B.MakeVertex(V1, Point(P1), Precision::Confusion());
  if (P1.Distance(P2) < Precision::Confusion())
    V2 = V1;
  else
    B.MakeVertex(V2, Point(P2), Precision::Confusion());
  Init(C, V1, V2);
}

//=================================================================================================

void BRepLib_MakeEdge2d::Init(const Handle(GeomCurve2d)& C,
                              const TopoVertex&        V1,
                              const TopoVertex&        V2)
{
  // try projecting the vertices on the curve

  Standard_Real p1, p2;

  if (V1.IsNull())
    p1 = C->FirstParameter();
  else if (!Project(C, V1, p1))
  {
    myError = BRepLib_PointProjectionFailed;
    return;
  }
  if (V2.IsNull())
    p2 = C->LastParameter();
  else if (!Project(C, V2, p2))
  {
    myError = BRepLib_PointProjectionFailed;
    return;
  }

  Init(C, V1, V2, p1, p2);
}

//=================================================================================================

void BRepLib_MakeEdge2d::Init(const Handle(GeomCurve2d)& C,
                              const gp_Pnt2d&             P1,
                              const gp_Pnt2d&             P2,
                              const Standard_Real         p1,
                              const Standard_Real         p2)
{
  ShapeBuilder B;

  TopoVertex V1, V2;
  B.MakeVertex(V1, Point(P1), Precision::Confusion());
  if (P1.Distance(P2) < Precision::Confusion())
    V2 = V1;
  else
    B.MakeVertex(V2, Point(P2), Precision::Confusion());

  Init(C, V1, V2, p1, p2);
}

//=======================================================================
// function : Init
// purpose  : this one really makes the job ...
//=======================================================================

void BRepLib_MakeEdge2d::Init(const Handle(GeomCurve2d)& CC,
                              const TopoVertex&        VV1,
                              const TopoVertex&        VV2,
                              const Standard_Real         pp1,
                              const Standard_Real         pp2)
{
  // kill trimmed curves
  Handle(GeomCurve2d)        C  = CC;
  Handle(Geom2d_TrimmedCurve) CT = Handle(Geom2d_TrimmedCurve)::DownCast(C);
  while (!CT.IsNull())
  {
    C  = CT->BasisCurve();
    CT = Handle(Geom2d_TrimmedCurve)::DownCast(C);
  }

  // check parameters
  Standard_Real           p1       = pp1;
  Standard_Real           p2       = pp2;
  Standard_Real           cf       = C->FirstParameter();
  Standard_Real           cl       = C->LastParameter();
  constexpr Standard_Real epsilon  = Precision::Confusion();
  Standard_Boolean        periodic = C->IsPeriodic();

  TopoVertex V1, V2;
  if (periodic)
  {
    // adjust in period
    ElCLib1::AdjustPeriodic(cf, cl, epsilon, p1, p2);
    V1 = VV1;
    V2 = VV2;
  }
  else
  {
    // reordonate
    if (p1 < p2)
    {
      V1 = VV1;
      V2 = VV2;
    }
    else
    {
      V2              = VV1;
      V1              = VV2;
      Standard_Real x = p1;
      p1              = p2;
      p2              = x;
    }

    // check range
    if ((cf - p1 > epsilon) || (p2 - cl > epsilon))
    {
      myError = BRepLib_ParameterOutOfRange;
      return;
    }
  }

  // compute points on the curve
  Standard_Boolean p1inf = Precision::IsNegativeInfinite(p1);
  Standard_Boolean p2inf = Precision::IsPositiveInfinite(p2);
  gp_Pnt2d         P1, P2;
  if (!p1inf)
    P1 = C->Value(p1);
  if (!p2inf)
    P2 = C->Value(p2);

  constexpr Standard_Real preci = Precision::Confusion();
  ShapeBuilder            B;

  // check for closed curve
  Standard_Boolean closed = Standard_False;
  if (!p1inf && !p2inf)
    closed = (P1.Distance(P2) <= preci);

  // check if the vertices are on the curve
  if (closed)
  {
    if (V1.IsNull() && V2.IsNull())
    {
      B.MakeVertex(V1, Point(P1), preci);
      V2 = V1;
    }
    else if (V1.IsNull())
      V1 = V2;
    else if (V2.IsNull())
      V2 = V1;
    else
    {
      if (!V1.IsSame(V2))
      {
        myError = BRepLib_DifferentPointsOnClosedCurve;
        return;
      }
      else if (Point(P1).Distance(BRepInspector::Pnt(V1)) > preci)
      {
        myError = BRepLib_DifferentPointsOnClosedCurve;
        return;
      }
    }
  }

  else
  { // not closed

    if (p1inf)
    {
      if (!V1.IsNull())
      {
        myError = BRepLib_PointWithInfiniteParameter;
        return;
      }
    }
    else
    {
      Point3d P = Point(P1);
      if (V1.IsNull())
      {
        B.MakeVertex(V1, P, preci);
      }
#if 0
      // desctivate control (RLE) for speed in sketcher
        else if (P.Distance(BRepInspector::Pnt(V1)) > preci) {
	myError = BRepLib_DifferentsPointAndParameter;
	return;
      }
#endif
    }

    if (p2inf)
    {
      if (!V2.IsNull())
      {
        myError = BRepLib_PointWithInfiniteParameter;
        return;
      }
    }
    else
    {
      Point3d P = Point(P2);
      if (V2.IsNull())
      {
        B.MakeVertex(V2, P, preci);
      }
#if 0
      // desctivate control (RLE) for speed in sketcher
        else if (P.Distance(BRepInspector::Pnt(V2)) > preci){
	myError = BRepLib_DifferentsPointAndParameter;
	return;
      }
#endif
    }
  }

  V1.Orientation(TopAbs_FORWARD);
  V2.Orientation(TopAbs_REVERSED);
  myVertex1 = V1;
  myVertex2 = V2;

  TopoEdge& E = TopoDS::Edge(myShape);
  B.MakeEdge(E);
  B.UpdateEdge(E, C, BRepLib::Plane1(), TopLoc_Location(), preci);
  if (!V1.IsNull())
  {
    B.Add(E, V1);
  }
  if (!V2.IsNull())
  {
    B.Add(E, V2);
  }
  B.Range(E, p1, p2);
  Done();
}

//=================================================================================================

BRepLib_EdgeError BRepLib_MakeEdge2d::Error() const
{
  return myError;
}

//=================================================================================================

const TopoEdge& BRepLib_MakeEdge2d::Edge()
{
  return TopoDS::Edge(Shape());
}

//=================================================================================================

const TopoVertex& BRepLib_MakeEdge2d::Vertex1() const
{
  Check();
  return myVertex1;
}

//=================================================================================================

const TopoVertex& BRepLib_MakeEdge2d::Vertex2() const
{
  Check();
  return myVertex2;
}

//=================================================================================================

BRepLib_MakeEdge2d::operator TopoEdge()
{
  return Edge();
}
