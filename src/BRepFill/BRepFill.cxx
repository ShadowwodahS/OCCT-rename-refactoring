// Created on: 1994-03-03
// Created by: Joelle CHAUVET
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

// Modified:	Mon Jan 12 10:50:10 1998
//              automatic management of origin and orientation
//              with method Organize
// Modified:	Mon Feb 23 09:28:46 1998
//              method Organize with option of projection for closed wires
//              new method SameNumber with option to report cuts
//              + utilities ComputeACR and InsertACR
//              + processing of the case of last point section
// Modified:	Thu Apr 30 15:24:17 1998
//              separation closed / open sections + debug
//              Organize becomes ComputeOrigin and SearchOrigin
// Modified:	Tue Jul 21 16:48:35 1998
//              limited case for Pnext of a twist (BUC60281)
// Modified:	Thu Jul 23 11:38:36 1998
//              calculate the angle of rotation in SearchOrigin
// Modified:	Fri Jul 31 15:14:19 1998
//              IntersectOnWire + MapVLV
// Modified:	Mon Oct 12 09:42:33 1998
//              number of edges in EdgesFromVertex (CTS21570)

#include <BRepFill.hxx>

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepExtrema_ExtPC.hxx>
#include <BRepLib.hxx>
#include <BRepLib_FindSurface.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepLib_MakeWire.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomFill_Generator.hxx>
#include <gp_Ax3.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Standard_NoSuchObject.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

static void MakeWire(const TopTools_Array1OfShape& Edges,
                     const Standard_Integer        rangdeb,
                     const Standard_Boolean        forward,
                     TopoWire&                  newwire)
{
  ShapeBuilder     BW;
  Standard_Integer rang, nbEdges = Edges.Length();
  BW.MakeWire(newwire);
  if (forward)
  {
    for (rang = rangdeb; rang <= nbEdges; rang++)
    {
      BW.Add(newwire, TopoDS::Edge(Edges(rang)));
    }
    for (rang = 1; rang < rangdeb; rang++)
    {
      BW.Add(newwire, TopoDS::Edge(Edges(rang)));
    }
  }

  else
  {
    TopoEdge E;
    for (rang = rangdeb; rang >= 1; rang--)
    {
      E = TopoDS::Edge(Edges(rang));
      BW.Add(newwire, E.Reversed());
    }
    for (rang = nbEdges; rang > rangdeb; rang--)
    {
      E = TopoDS::Edge(Edges(rang));
      BW.Add(newwire, E.Reversed());
    }
  }
  newwire.Orientation(TopAbs_FORWARD);
  newwire.Closed(Standard_True);
}

static void CutEdge(const TopoEdge&   CurrentEdge,
                    const Standard_Real& Param,
                    TopoEdge&         E1,
                    TopoEdge&         E2,
                    const TopoVertex& VRef)
{
  ShapeBuilder       B;
  Standard_Real      first, last;
  Handle(GeomCurve3d) C = BRepInspector::Curve(CurrentEdge, first, last);
  TopoVertex      Vf, Vl, Vi;
  B.MakeVertex(Vi, C->Value(Param), Precision::Confusion());
  TopExp1::Vertices(CurrentEdge, Vf, Vl);
  if (VRef.IsSame(Vf))
  {
    E1 = BRepLib_MakeEdge(C, Vf, Vi, first, Param);
    E2 = BRepLib_MakeEdge(C, Vi, Vl, Param, last);
  }
  else
  {
    E2 = BRepLib_MakeEdge(C, Vf, Vi, first, Param);
    E1 = BRepLib_MakeEdge(C, Vi, Vl, Param, last);
  }
}

static void TrimEdge(const TopoEdge&            CurrentEdge,
                     const TColStd_SequenceOfReal& CutValues,
                     const Standard_Real           t0,
                     const Standard_Real           t1,
                     const Standard_Boolean        SeqOrder,
                     TopTools_SequenceOfShape&     S)

{
  S.Clear();
  Standard_Integer   j, ndec = CutValues.Length();
  Standard_Real      first, last, m0, m1;
  Handle(GeomCurve3d) C = BRepInspector::Curve(CurrentEdge, first, last);

  TopoVertex      Vf, Vl, Vbid, V0, V1;
  TopAbs_Orientation CurrentOrient = CurrentEdge.Orientation();
  TopExp1::Vertices(CurrentEdge, Vf, Vl);
  Vbid.Nullify();

  if (SeqOrder)
  {
    // from first to last
    m0 = first;
    V0 = Vf;
    for (j = 1; j <= ndec; j++)
    {
      // piece of edge
      m1               = (CutValues.Value(j) - t0) * (last - first) / (t1 - t0) + first;
      TopoEdge CutE = BRepLib_MakeEdge(C, V0, Vbid, m0, m1);
      CutE.Orientation(CurrentOrient);
      S.Append(CutE);
      m0 = m1;
      V0 = TopExp1::LastVertex(CutE);
      if (j == ndec)
      {
        // last piece
        TopoEdge LastE = BRepLib_MakeEdge(C, V0, Vl, m0, last);
        LastE.Orientation(CurrentOrient);
        S.Append(LastE);
      }
    }
  }
  else
  {
    // from last to first
    m1 = last;
    V1 = Vl;
    for (j = ndec; j >= 1; j--)
    {
      // piece of edge
      m0               = (CutValues.Value(j) - t0) * (last - first) / (t1 - t0) + first;
      TopoEdge CutE = BRepLib_MakeEdge(C, Vbid, V1, m0, m1);
      CutE.Orientation(CurrentOrient);
      S.Append(CutE);
      m1 = m0;
      V1 = TopExp1::FirstVertex(CutE);
      if (j == 1)
      {
        // last piece
        TopoEdge LastE = BRepLib_MakeEdge(C, Vf, V1, first, m1);
        LastE.Orientation(CurrentOrient);
        S.Append(LastE);
      }
    }
  }
}

//=================================================================================================

TopoFace BRepFill1::Face(const TopoEdge& Edge1, const TopoEdge& Edge2)
{
  TopoFace Face;

  ShapeBuilder B;
  // Class BRepInspector without fields and without Constructor :
  //  BRepInspector BT;

  TopLoc_Location L, L1, L2;
  Standard_Real   f1, f2, l1, l2, Tol;

  //  Handle(GeomCurve3d) C1 = BT.Curve(Edge1,L1,f1,l1);
  Handle(GeomCurve3d) C1 = BRepInspector::Curve(Edge1, L1, f1, l1);
  //  Handle(GeomCurve3d) C2 = BT.Curve(Edge2,L2,f2,l2);
  Handle(GeomCurve3d) C2 = BRepInspector::Curve(Edge2, L2, f2, l2);

  // compute the location
  Standard_Boolean SameLoc = Standard_False;
  if (L1 == L2)
  {
    L  = L1;
    L1 = L2 = TopLoc_Location();
    SameLoc = Standard_True;
  }

  // transform and trim the curves

  TopoVertex V1f, V1l, V2f, V2l;

  // create a new Handle
  if (Abs(f1 - C1->FirstParameter()) > Precision::PConfusion()
      || Abs(l1 - C1->LastParameter()) > Precision::PConfusion())
  {
    C1 = new Geom_TrimmedCurve(C1, f1, l1);
  }
  else
  {
    C1 = Handle(GeomCurve3d)::DownCast(C1->Copy());
  }
  // eventually the curve is concerned
  if (!SameLoc)
  {
    C1->Transform(L1.Transformation());
  }
  // it is set in the proper direction and its vertices are taken
  if (Edge1.Orientation() == TopAbs_REVERSED)
  {
    TopExp1::Vertices(Edge1, V1l, V1f);
    C1->Reverse();
  }
  else
  {
    TopExp1::Vertices(Edge1, V1f, V1l);
  }

  // a new Handle is created
  if (Abs(f2 - C2->FirstParameter()) > Precision::PConfusion()
      || Abs(l2 - C2->LastParameter()) > Precision::PConfusion())
  {
    C2 = new Geom_TrimmedCurve(C2, f2, l2);
  }
  else
  {
    C2 = Handle(GeomCurve3d)::DownCast(C2->Copy());
  }
  // eventually the curve is concerned
  if (!SameLoc)
  {
    C2->Transform(L2.Transformation());
  }
  // it is set in the proper direction and its vertices are taken
  if (Edge2.Orientation() == TopAbs_REVERSED)
  {
    TopExp1::Vertices(Edge2, V2l, V2f);
    C2->Reverse();
  }
  else
  {
    TopExp1::Vertices(Edge2, V2f, V2l);
  }

  // Are they closed edges?
  Standard_Boolean Closed = V1f.IsSame(V1l) && V2f.IsSame(V2l);

  GeomFill_Generator Generator;
  Generator.AddCurve(C1);
  Generator.AddCurve(C2);
  Generator.Perform(Precision::PConfusion());

  Handle(GeomSurface) Surf = Generator.Surface();
  Handle(GeomCurve3d)   Iso;

  B.MakeFace(Face, Surf, Precision::Confusion());

  // make the missing edges
  Surf->Bounds(f1, l1, f2, l2);

  TopoEdge Edge3, Edge4;

  Iso = Surf->UIso(f1);
  Tol = Max(BRepInspector::Tolerance(V1f), BRepInspector::Tolerance(V2f));
  if (Iso->Value(f2).Distance(Iso->Value(l2)) > Tol)
  {
    B.MakeEdge(Edge3, Iso, Precision::Confusion());
  }
  else
  {
    B.MakeEdge(Edge3);
    B.Degenerated(Edge3, Standard_True);
  }
  V1f.Orientation(TopAbs_FORWARD);
  B.Add(Edge3, V1f);
  V2f.Orientation(TopAbs_REVERSED);
  B.Add(Edge3, V2f);
  B.Range(Edge3, f2, l2);

  if (Closed)
  {
    Edge4 = Edge3;
  }
  else
  {
    Iso = Surf->UIso(l1);
    Tol = Max(BRepInspector::Tolerance(V1l), BRepInspector::Tolerance(V2l));
    if (Iso->Value(l2).Distance(Iso->Value(f2)) > Tol)
    {
      B.MakeEdge(Edge4, Iso, Precision::Confusion());
    }
    else
    {
      B.MakeEdge(Edge4);
      B.Degenerated(Edge4, Standard_True);
    }
    V1l.Orientation(TopAbs_FORWARD);
    B.Add(Edge4, V1l);
    V2l.Orientation(TopAbs_REVERSED);
    B.Add(Edge4, V2l);
    B.Range(Edge4, f2, l2);
  }

  // make the wire

  TopoWire W;
  B.MakeWire(W);

  Edge3.Reverse();
  B.Add(W, Edge1);
  B.Add(W, Edge4);
  B.Add(W, Edge2.Reversed());
  B.Add(W, Edge3);
  W.Closed(Standard_True);

  B.Add(Face, W);

  // set the pcurves

  Standard_Real T = Precision::Confusion();

  if (Edge1.Orientation() == TopAbs_REVERSED)
  {
    B.UpdateEdge(Edge1, new Geom2d_Line(gp_Pnt2d(0, f2), gp_Dir2d(-1, 0)), Face, T);
    B.Range(Edge1, Face, -l1, -f1);
  }
  else
  {
    B.UpdateEdge(Edge1, new Geom2d_Line(gp_Pnt2d(0, f2), gp_Dir2d(1, 0)), Face, T);
    B.Range(Edge1, Face, f1, l1);
  }

  if (Edge2.Orientation() == TopAbs_REVERSED)
  {
    B.UpdateEdge(Edge2, new Geom2d_Line(gp_Pnt2d(0, l2), gp_Dir2d(-1, 0)), Face, T);
    B.Range(Edge2, Face, -l1, -f1);
  }
  else
  {
    B.UpdateEdge(Edge2, new Geom2d_Line(gp_Pnt2d(0, l2), gp_Dir2d(1, 0)), Face, T);
    B.Range(Edge2, Face, f1, l1);
  }

  if (Closed)
  {
    B.UpdateEdge(Edge3,
                 new Geom2d_Line(gp_Pnt2d(l1, 0), gp_Dir2d(0, 1)),
                 new Geom2d_Line(gp_Pnt2d(f1, 0), gp_Dir2d(0, 1)),
                 Face,
                 T);
  }
  else
  {
    B.UpdateEdge(Edge3, new Geom2d_Line(gp_Pnt2d(f1, 0), gp_Dir2d(0, 1)), Face, T);
    B.UpdateEdge(Edge4, new Geom2d_Line(gp_Pnt2d(l1, 0), gp_Dir2d(0, 1)), Face, T);
  }

  // Set the non parameter flag;
  B.SameParameter(Edge1, Standard_False);
  B.SameParameter(Edge2, Standard_False);
  B.SameParameter(Edge3, Standard_False);
  B.SameParameter(Edge4, Standard_False);
  B.SameRange(Edge1, Standard_False);
  B.SameRange(Edge2, Standard_False);
  B.SameRange(Edge3, Standard_False);
  B.SameRange(Edge4, Standard_False);

  BRepLib1::SameParameter(Face);

  if (SameLoc)
    Face.Move(L);
  return Face;
}

//=================================================================================================

TopoShell BRepFill1::Shell(const TopoWire& Wire1, const TopoWire& Wire2)
{
  TopoShell Shell;
  TopoFace  Face;
  TopoShape S1, S2;
  TopoEdge  Edge1, Edge2, Edge3, Edge4, Couture;

  ShapeBuilder B;
  // Class BRepInspector without fields and without Constructor :
  //  BRepInspector BT;
  B.MakeShell(Shell);

  ShapeExplorer ex1;
  ShapeExplorer ex2;

  Standard_Boolean Closed = Wire1.Closed() && Wire2.Closed();

  Standard_Boolean thefirst = Standard_True;

  ex1.Init(Wire1, TopAbs_EDGE);
  ex2.Init(Wire2, TopAbs_EDGE);

  while (ex1.More() && ex2.More())
  {

    Edge1 = TopoDS::Edge(ex1.Current());
    Edge2 = TopoDS::Edge(ex2.Current());

    Standard_Boolean Periodic = BRepInspector::IsClosed(Edge1) && BRepInspector::IsClosed(Edge2);

    ex1.Next();
    ex2.Next();

    TopLoc_Location L, L1, L2;
    Standard_Real   f1, l1, f2, l2, Tol;

    Handle(GeomCurve3d) C1 = BRepInspector::Curve(Edge1, L1, f1, l1);
    Handle(GeomCurve3d) C2 = BRepInspector::Curve(Edge2, L2, f2, l2);

    // compute the location
    Standard_Boolean SameLoc = Standard_False;
    if (L1 == L2)
    {
      L  = L1;
      L1 = L2 = TopLoc_Location();
      SameLoc = Standard_True;
    }

    // transform and trim the curves

    TopoVertex V1f, V1l, V2f, V2l;

    if (Abs(f1 - C1->FirstParameter()) > Precision::PConfusion()
        || Abs(l1 - C1->LastParameter()) > Precision::PConfusion())
    {
      C1 = new Geom_TrimmedCurve(C1, f1, l1);
    }
    else
    {
      C1 = Handle(GeomCurve3d)::DownCast(C1->Copy());
    }
    if (!SameLoc)
    {
      C1->Transform(L1.Transformation());
    }
    if (Edge1.Orientation() == TopAbs_REVERSED)
    {
      TopExp1::Vertices(Edge1, V1l, V1f);
      C1->Reverse();
    }
    else
      TopExp1::Vertices(Edge1, V1f, V1l);

    if (Abs(f2 - C2->FirstParameter()) > Precision::PConfusion()
        || Abs(l2 - C2->LastParameter()) > Precision::PConfusion())
    {
      C2 = new Geom_TrimmedCurve(C2, f2, l2);
    }
    else
    {
      C2 = Handle(GeomCurve3d)::DownCast(C2->Copy());
    }
    if (!SameLoc)
    {
      C2->Transform(L2.Transformation());
    }
    if (Edge2.Orientation() == TopAbs_REVERSED)
    {
      TopExp1::Vertices(Edge2, V2l, V2f);
      C2->Reverse();
    }
    else
      TopExp1::Vertices(Edge2, V2f, V2l);

    GeomFill_Generator Generator;
    Generator.AddCurve(C1);
    Generator.AddCurve(C2);
    Generator.Perform(Precision::PConfusion());

    Handle(GeomSurface) Surf = Generator.Surface();
    Handle(GeomCurve3d)   Iso;

    B.MakeFace(Face, Surf, Precision::Confusion());

    // make the missing edges
    Surf->Bounds(f1, l1, f2, l2);

    if (thefirst)
    {
      Iso = Surf->UIso(f1);
      //      Tol = Max(BT.Tolerance(V1f), BT.Tolerance(V2f));
      Tol = Max(BRepInspector::Tolerance(V1f), BRepInspector::Tolerance(V2f));
      if (Iso->Value(f2).Distance(Iso->Value(l2)) > Tol)
      {
        B.MakeEdge(Edge3, Iso, Precision::Confusion());
      }
      else
      {
        B.MakeEdge(Edge3);
        B.Degenerated(Edge3, Standard_True);
      }
      V1f.Orientation(TopAbs_FORWARD);
      B.Add(Edge3, V1f);
      V2f.Orientation(TopAbs_REVERSED);
      B.Add(Edge3, V2f);
      B.Range(Edge3, f2, l2);
      if (Closed)
      {
        Couture = Edge3;
      }
      Edge3.Reverse();
      thefirst = Standard_False;
    }
    else
    {
      Edge3 = Edge4;
      Edge3.Reverse();
    }

    if (Closed && !ex1.More() && !ex2.More())
    {
      Edge4 = Couture;
    }
    else
    {
      Iso = Surf->UIso(l1);
      //      Tol = Max(BT.Tolerance(V1l), BT.Tolerance(V2l));
      Tol = Max(BRepInspector::Tolerance(V1l), BRepInspector::Tolerance(V2l));
      if (Iso->Value(l2).Distance(Iso->Value(f2)) > Tol)
      {
        B.MakeEdge(Edge4, Iso, Precision::Confusion());
      }
      else
      {
        B.MakeEdge(Edge4);
        B.Degenerated(Edge4, Standard_True);
      }
      V1l.Orientation(TopAbs_FORWARD);
      B.Add(Edge4, V1l);
      V2l.Orientation(TopAbs_REVERSED);
      B.Add(Edge4, V2l);
      B.Range(Edge4, f2, l2);
    }

    // make the wire

    TopoWire W;
    B.MakeWire(W);

    B.Add(W, Edge1);
    B.Add(W, Edge4);
    B.Add(W, Edge2.Reversed());
    B.Add(W, Edge3);
    W.Closed(Standard_True);

    B.Add(Face, W);

    if (SameLoc)
      Face.Move(L);

    B.Add(Shell, Face);

    // set the pcurves

    Standard_Real T = Precision::Confusion();

    if (Edge1.Orientation() == TopAbs_REVERSED)
    {
      B.UpdateEdge(Edge1, new Geom2d_Line(gp_Pnt2d(0, f2), gp_Dir2d(-1, 0)), Face, T);
      B.Range(Edge1, Face, -l1, -f1);
    }
    else
    {
      B.UpdateEdge(Edge1, new Geom2d_Line(gp_Pnt2d(0, f2), gp_Dir2d(1, 0)), Face, T);
      B.Range(Edge1, Face, f1, l1);
    }

    if (Edge2.Orientation() == TopAbs_REVERSED)
    {
      B.UpdateEdge(Edge2, new Geom2d_Line(gp_Pnt2d(0, l2), gp_Dir2d(-1, 0)), Face, T);
      B.Range(Edge2, Face, -l1, -f1);
    }
    else
    {
      B.UpdateEdge(Edge2, new Geom2d_Line(gp_Pnt2d(0, l2), gp_Dir2d(1, 0)), Face, T);
      B.Range(Edge2, Face, f1, l1);
    }

    if (Periodic)
    {
      B.UpdateEdge(Edge3,
                   new Geom2d_Line(gp_Pnt2d(l1, 0), gp_Dir2d(0, 1)),
                   new Geom2d_Line(gp_Pnt2d(f1, 0), gp_Dir2d(0, 1)),
                   Face,
                   T);
    }
    else
    {
      B.UpdateEdge(Edge3, new Geom2d_Line(gp_Pnt2d(f1, 0), gp_Dir2d(0, 1)), Face, T);
      B.UpdateEdge(Edge4, new Geom2d_Line(gp_Pnt2d(l1, 0), gp_Dir2d(0, 1)), Face, T);
    }

    // Set the non parameter flag;
    B.SameParameter(Edge1, Standard_False);
    B.SameParameter(Edge2, Standard_False);
    B.SameParameter(Edge3, Standard_False);
    B.SameParameter(Edge4, Standard_False);
    B.SameRange(Edge1, Standard_False);
    B.SameRange(Edge2, Standard_False);
    B.SameRange(Edge3, Standard_False);
    B.SameRange(Edge4, Standard_False);
  }

  Shell.Closed(BRepInspector::IsClosed(Shell));
  BRepLib1::SameParameter(Shell);
  return Shell;
}

//=================================================================================================

void BRepFill1::Axe(const TopoShape& Spine,
                   const TopoWire&  Profile,
                   Ax3&             AxeProf,
                   Standard_Boolean&   ProfOnSpine,
                   const Standard_Real Tol)
{
  Point3d Loc, Loc1, Loc2;
  Vector3d Tang, Tang1, Tang2, Normal;

  Handle(GeomSurface) S;
  TopLoc_Location      L;

  TopoFace aFace;

  // normal to the Spine.
  if (Spine.ShapeType() == TopAbs_FACE)
  {
    aFace = TopoDS::Face(Spine);
    S     = BRepInspector::Surface(TopoDS::Face(Spine), L);
    if (!S->IsKind(STANDARD_TYPE(GeomPlane)))
    {
      BRepLib_FindSurface FS(TopoDS::Face(Spine), -1, Standard_True);
      if (FS.Found())
      {
        S = FS.Surface();
        L = FS.Location();
      }
      else
      {
        throw Standard_NoSuchObject("BRepFill_Evolved : The Face is not planar");
      }
    }
  }
  else if (Spine.ShapeType() == TopAbs_WIRE)
  {
    aFace = BRepLib_MakeFace(TopoDS::Wire(Spine), Standard_True);
    S     = BRepInspector::Surface(aFace, L);
  }

  if (S.IsNull())
    throw Standard_DomainError("BRepFill_Evolved::Axe");

  if (!L.IsIdentity())
    S = Handle(GeomSurface)::DownCast(S->Transformed(L.Transformation()));

  Normal = Handle(GeomPlane)::DownCast(S)->Pln().Axis().Direction();

  // Find vertex of the profile closest to the spine.
  Standard_Real DistMin = Precision::Infinite();
  Standard_Real Dist;
  //  Standard_Real     Tol2 = Tol*Tol;
  Standard_Real     Tol2 = 1.e-10;
  ShapeExplorer   PE, SE;
  BRepExtrema_ExtPC BE;
  Standard_Real     Par = 0., f, l;
  //  Standard_Real     D1,D2;
  Point3d P1, P2;

  // First check if there is contact Vertex Vertex.
  Standard_Boolean IsOnVertex = Standard_False;
  SE.Init(aFace.Oriented(TopAbs_FORWARD), TopAbs_VERTEX);
  //  modified by NIZHNY-EAP Wed Feb 23 12:31:52 2000 ___BEGIN___
  //  for (;SE.More() && !IsOnVertex ; SE.Next()) {
  for (; SE.More(); SE.Next())
  {
    P1 = BRepInspector::Pnt(TopoDS::Vertex(SE.Current()));

    PE.Init(Profile, TopAbs_VERTEX);
    for (; PE.More(); PE.Next())
    {
      P2                     = BRepInspector::Pnt(TopoDS::Vertex(PE.Current()));
      Standard_Real DistP1P2 = P1.SquareDistance(P2);
      IsOnVertex             = (DistP1P2 <= Tol2);
      if (IsOnVertex)
        break;
    }
    // otherwise SE.Next() is done and VonF is wrong
    if (IsOnVertex)
      break;
    //  modified by NIZHNY-EAP Wed Jan 26 09:08:36 2000 ___END___
  }

  if (IsOnVertex)
  {
    // try to find on which edge which shared this vertex,
    // the profile must be considered.
    // E1, E2 : those two edges.
    TopTools_IndexedDataMapOfShapeListOfShape Map;
    TopExp1::MapShapesAndAncestors(aFace.Oriented(TopAbs_FORWARD), TopAbs_VERTEX, TopAbs_EDGE, Map);

    const TopoVertex&        VonF = TopoDS::Vertex(SE.Current());
    const ShapeList& List = Map.FindFromKey(VonF);
    const TopoEdge&          E1   = TopoDS::Edge(List.First());
    const TopoEdge&          E2   = TopoDS::Edge(List.Last());

    Handle(GeomCurve3d) CE1  = BRepInspector::Curve(E1, L, f, l);
    Standard_Real      Par1 = BRepInspector::Parameter(VonF, E1, aFace);
    CE1->D1(Par1, Loc1, Tang1);
    if (!L.IsIdentity())
    {
      Tang1.Transform(L.Transformation());
      Loc1.Transform(L.Transformation());
    }
    if (E1.Orientation() == TopAbs_REVERSED)
      Tang1.Reverse();

    Handle(GeomCurve3d) CE2  = BRepInspector::Curve(E2, L, f, l);
    Standard_Real      Par2 = BRepInspector::Parameter(VonF, E2, aFace);
    CE2->D1(Par2, Loc2, Tang2);
    if (!L.IsIdentity())
    {
      Tang2.Transform(L.Transformation());
      Loc2.Transform(L.Transformation());
    }
    if (E2.Orientation() == TopAbs_REVERSED)
      Tang2.Reverse();

    //  modified by NIZHNY-EAP Wed Feb  2 15:38:41 2000 ___BEGIN___
    Tang1.Normalize();
    Tang2.Normalize();
    Standard_Real sca1 = 0., sca2 = 0.;
    TopoVertex V1, V2;
    TopoEdge   E;
    for (PE.Init(Profile, TopAbs_EDGE); PE.More(); PE.Next())
    {
      E = TopoDS::Edge(PE.Current());
      TopExp1::Vertices(E, V1, V2);
      P1 = BRepInspector::Pnt(V1);
      P2 = BRepInspector::Pnt(V2);
      Vector3d vec(P1, P2);
      sca1 += Abs(Tang1.Dot(vec));
      sca2 += Abs(Tang2.Dot(vec));
    }
    //  modified by NIZHNY-EAP Wed Feb  2 15:38:44 2000 ___END___

    if (Abs(sca1) < Abs(sca2))
    {
      Loc  = Loc1;
      Tang = Tang1;
    }
    else
    {
      Loc  = Loc2;
      Tang = Tang2;
    }
    DistMin = 0.;
  }
  else
  {
    SE.Init(aFace.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
    for (; SE.More(); SE.Next())
    {
      const TopoEdge& E = TopoDS::Edge(SE.Current());
      BE.Initialize(E);
      for (PE.Init(Profile, TopAbs_VERTEX); PE.More(); PE.Next())
      {
        Dist                   = Precision::Infinite();
        const TopoVertex& V = TopoDS::Vertex(PE.Current());
        BE.Perform(V);
        if (BE.IsDone())
        {
          // extrema.
          for (Standard_Integer i = 1; i <= BE.NbExt(); i++)
          {
            if (BE.IsMin(i))
            {
              Dist = sqrt(BE.SquareDistance(i));
              Par  = BE.Parameter(i);
              break;
            }
          }
        }
        // save minimum.
        if (Dist < DistMin)
        {
          DistMin = Dist;
          BRepAdaptor_Curve BAC(E);
          BAC.D1(Par, Loc, Tang);
          if (E.Orientation() == TopAbs_REVERSED)
            Tang.Reverse();
        }
      }
    }
  }

  ProfOnSpine = (DistMin < Tol);
  // Construction AxeProf;
  Ax3 A3(Loc, Normal, Tang);
  AxeProf = A3;
}

//=======================================================================
// function : SearchOrigin
// purpose  : Cut and orientate a closed wire.
//=======================================================================

void BRepFill1::SearchOrigin(TopoWire&        W,
                            const Point3d&       P,
                            const Vector3d&       Dir,
                            const Standard_Real Tol)
{
  if (!W.Closed())
    Standard_NoSuchObject::Raise("BRepFill1::SearchOrigin : the wire must be closed");

  Standard_Boolean NewVertex = Standard_False;
  Standard_Real    theparam  = 1.e101, angle;
  TopoVertex    V;
  TopoEdge      E, Eref;
  ShapeBuilder     B;
  // Class BRepInspector without fields and without Constructor :
  //  BRepInspector BT;

  W.Orientation(TopAbs_FORWARD); // to avoid composing the orientations

  // Calculate the distance
  B.MakeVertex(V, P, Tol);
  BRepExtrema_DistShapeShape DSS(V, W);
  if (DSS.IsDone())
  {
    Standard_Integer isol = 1;
    Standard_Real    dss  = P.Distance(DSS.PointOnShape2(isol));
    for (Standard_Integer iss = 2; iss <= DSS.NbSolution(); iss++)
      if (dss > P.Distance(DSS.PointOnShape2(iss)))
      {
        dss  = P.Distance(DSS.PointOnShape2(iss));
        isol = iss;
      }
    TopoShape supp = DSS.SupportOnShape2(isol);
    if (DSS.SupportTypeShape2(isol) == BRepExtrema_IsVertex)
    {
      V = TopoDS::Vertex(supp);
    }
    else
    {
      TopoVertex Vf, Vl;
      Standard_Real d, dist;
      E = TopoDS::Edge(supp);
      TopExp1::Vertices(E, Vf, Vl);
      //      dist = P.Distance(BT.Pnt(Vf));
      dist = P.Distance(BRepInspector::Pnt(Vf));
      if (dist < Tol)
      {
        V = Vl;
      }
      //      d = P.Distance(BT.Pnt(Vl));
      d = P.Distance(BRepInspector::Pnt(Vl));
      if ((d < Tol) && (d < dist))
      {
        V    = Vf;
        dist = d;
      }
      NewVertex = (dist > Tol);
      if (NewVertex)
      {
        DSS.ParOnEdgeS2(isol, theparam);
      }
    }
  }
#ifdef OCCT_DEBUG
  else
  {
    std::cout << "BRepFill1::SearchOrigine : Echec Distance" << std::endl;
  }
#endif

  Standard_Integer       ii, rangdeb = 0, NbEdges = 0;
  Standard_Boolean       forward;
  BRepTools_WireExplorer exp;

  // Calculate the number of edges
  for (exp.Init(W); exp.More(); exp.Next())
    NbEdges++;
  if (NewVertex)
  {
    NbEdges++;
    Eref = E;
  }

  // Construct the Table and calculate rangdeb
  TopTools_Array1OfShape Edges(1, NbEdges);
  for (exp.Init(W), ii = 1; exp.More(); exp.Next(), ii++)
  {
    E = exp.Current();
    if (NewVertex && E.IsSame(Eref))
    {
      TopoEdge E1, E2;
      CutEdge(E, theparam, E1, E2, exp.CurrentVertex());
      Edges(ii) = E1;
      ii++;
      Edges(ii) = E2;
      rangdeb   = ii;
    }
    else
    {
      Edges(ii) = E;
    }
    if (!NewVertex && V.IsSame(exp.CurrentVertex()))
    {
      rangdeb = ii;
    }
  }
  if (rangdeb == 0)
    rangdeb = NbEdges;

  // Calculate the direction of parsing
  E = TopoDS::Edge(Edges(rangdeb));
  if (!NewVertex)
  {
    //    theparam = BT.Parameter(V, E);
    theparam = BRepInspector::Parameter(V, E);
  }
  BRepAdaptor_Curve AC(E);
  Point3d            Pe;
  Vector3d            Ve;
  AC.D1(theparam, Pe, Ve);
  if (E.Orientation() == TopAbs_REVERSED)
  {
    Ve *= -1;
  }
  angle = Ve.Angle(Dir);
  if (angle > M_PI)
    angle = 2 * M_PI - angle;
  forward = (angle <= M_PI / 2);

  // Reconstruction
  MakeWire(Edges, rangdeb, forward, W);
  W.Closed(Standard_True);
}

//=================================================================================================

void BRepFill1::ComputeACR(const TopoWire& wire, TColStd_Array1OfReal& ACR)
{
  // calculate the reduced curvilinear abscisses and the length of the wire
  BRepTools_WireExplorer anExp;
  Standard_Integer       nbEdges = 0, i;

  // cumulated lengths
  ACR.Init(0);
  for (anExp.Init(wire); anExp.More(); anExp.Next())
  {
    nbEdges++;
    TopoEdge Ecur = TopoDS::Edge(anExp.Current());
    ACR(nbEdges)     = ACR(nbEdges - 1);
    if (!BRepInspector::Degenerated(Ecur))
    {
      BRepAdaptor_Curve anEcur(Ecur);
      ACR(nbEdges) += GCPnts_AbscissaPoint::Length(anEcur);
    }
  }

  // total length of the wire
  ACR(0) = ACR(nbEdges);

  // reduced curvilinear abscisses
  if (ACR(0) > Precision::Confusion())
  {
    for (i = 1; i <= nbEdges; i++)
    {
      ACR(i) /= ACR(0);
    }
  }
  else
  {
    // punctual wire
    ACR(nbEdges) = 1;
  }
}

//=================================================================================================

TopoWire BRepFill1::InsertACR(const TopoWire&          wire,
                                const TColStd_Array1OfReal& ACRcuts,
                                const Standard_Real         prec)
{
  // calculate ACR of the wire to be cut
  BRepTools_WireExplorer anExp;
  Standard_Integer       nbEdges = 0;
  for (anExp.Init(wire); anExp.More(); anExp.Next())
  {
    nbEdges++;
  }
  TColStd_Array1OfReal ACRwire(0, nbEdges);
  ComputeACR(wire, ACRwire);

  Standard_Integer     i, j, nmax = ACRcuts.Length();
  TColStd_Array1OfReal paradec(1, nmax);
  BRepLib_MakeWire     MW;

  Standard_Real t0, t1 = 0;
  nbEdges = 0;

  // processing edge by edge
  for (anExp.Init(wire); anExp.More(); anExp.Next())
  {
    nbEdges++;
    t0 = t1;
    t1 = ACRwire(nbEdges);

    // parameters of cut on this edge
    Standard_Integer ndec = 0;
    for (i = 1; i <= ACRcuts.Length(); i++)
    {
      if (t0 + prec < ACRcuts(i) && ACRcuts(i) < t1 - prec)
      {
        ndec++;
        paradec(ndec) = ACRcuts(i);
      }
    }

    const TopoEdge&   E = anExp.Current();
    const TopoVertex& V = anExp.CurrentVertex();

    if (ndec == 0 || BRepInspector::Degenerated(E))
    {
      // copy the edge
      MW.Add(E);
    }
    else
    {
      // it is necessary to cut the edge
      // following the direction of parsing of the wire
      Standard_Boolean         SO = (V.IsSame(TopExp1::FirstVertex(E)));
      TopTools_SequenceOfShape SE;
      SE.Clear();
      TColStd_SequenceOfReal SR;
      SR.Clear();
      // the wire is always FORWARD
      // it is necessary to modify the parameter of cut6 if the edge is REVERSED
      if (E.Orientation() == TopAbs_FORWARD)
      {
        for (j = 1; j <= ndec; j++)
          SR.Append(paradec(j));
      }
      else
      {
        for (j = 1; j <= ndec; j++)
          SR.Append(t0 + t1 - paradec(ndec + 1 - j));
      }
      TrimEdge(E, SR, t0, t1, SO, SE);
      for (j = 1; j <= SE.Length(); j++)
      {
        MW.Add(TopoDS::Edge(SE.Value(j)));
      }
    }
  }

  // result
  TopAbs_Orientation Orien       = wire.Orientation();
  TopoShape       aLocalShape = MW.Wire();
  aLocalShape.Orientation(Orien);
  TopoWire wres = TopoDS::Wire(aLocalShape);
  //  TopoWire wres = TopoDS::Wire(MW.Wire().Oriented(Orien));
  return wres;
}
