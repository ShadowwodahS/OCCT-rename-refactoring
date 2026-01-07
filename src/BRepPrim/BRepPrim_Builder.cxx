// Created on: 1991-07-25
// Created by: Christophe MARION
// Copyright (c) 1991-1999 Matra Datavision
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
#include <BRepPrim_Builder.hxx>
#include <BRepTools.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <gp_Circ.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <Precision.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

//=================================================================================================

BRepPrim_Builder::BRepPrim_Builder() {}

//=================================================================================================

BRepPrim_Builder::BRepPrim_Builder(const ShapeBuilder& B)
    : myBuilder(B)
{
}

//=================================================================================================

void BRepPrim_Builder::MakeShell(TopoShell& S) const
{
  myBuilder.MakeShell(S);
  S.Closed(Standard_True);
}

//=======================================================================
// function : MakeFace
// purpose  : Make a Planar Face
//=======================================================================

void BRepPrim_Builder::MakeFace(TopoFace& F, const gp_Pln& P) const
{
  myBuilder.MakeFace(F, new GeomPlane(P), Precision::Confusion());
}

//=======================================================================
// function : MakeWire
// purpose  : Make an empty Wire
//=======================================================================

void BRepPrim_Builder::MakeWire(TopoWire& W) const
{
  myBuilder.MakeWire(W);
}

//=================================================================================================

void BRepPrim_Builder::MakeDegeneratedEdge(TopoEdge& E) const
{
  myBuilder.MakeEdge(E);
  myBuilder.Degenerated(E, Standard_True);
}

//=======================================================================
// function : MakeEdge
// purpose  : Make a linear Edge
//=======================================================================

void BRepPrim_Builder::MakeEdge(TopoEdge& E, const gp_Lin& L) const
{
  myBuilder.MakeEdge(E, new GeomLine(L), Precision::Confusion());
}

//=======================================================================
// function : MakeEdge
// purpose  : Make a Circular Edge
//=======================================================================

void BRepPrim_Builder::MakeEdge(TopoEdge& E, const gp_Circ& C) const
{
  myBuilder.MakeEdge(E, new GeomCircle(C), Precision::Confusion());
}

//=================================================================================================

void BRepPrim_Builder::SetPCurve(TopoEdge& E, const TopoFace& F, const gp_Lin2d& L) const
{
  myBuilder.UpdateEdge(E, new Geom2d_Line(L), F, Precision::Confusion());
}

//=================================================================================================

void BRepPrim_Builder::SetPCurve(TopoEdge&       E,
                                 const TopoFace& F,
                                 const gp_Lin2d&    L1,
                                 const gp_Lin2d&    L2) const
{
  TopoShape aLocalShape = E.Oriented(TopAbs_FORWARD);
  myBuilder.UpdateEdge(TopoDS::Edge(aLocalShape),
                       new Geom2d_Line(L1),
                       new Geom2d_Line(L2),
                       F,
                       Precision::Confusion());
  //  myBuilder.UpdateEdge(TopoDS::Edge(E.Oriented(TopAbs_FORWARD)),
  //		       new Geom2d_Line(L1),
  //		       new Geom2d_Line(L2),
  //		       F,Precision::Confusion());
  myBuilder.Continuity(E, F, F, GeomAbs_CN);
}

//=================================================================================================

void BRepPrim_Builder::SetPCurve(TopoEdge& E, const TopoFace& F, const gp_Circ2d& C) const
{
  myBuilder.UpdateEdge(E, new Geom2d_Circle(C), F, Precision::Confusion());
}

//=======================================================================
// function : MakeVertex
// purpose  : Make a Vertex
//=======================================================================

void BRepPrim_Builder::MakeVertex(TopoVertex& V, const Point3d& P) const
{
  myBuilder.MakeVertex(V, P, Precision::Confusion());
}

//=======================================================================
// function : ReverseFace
// purpose  : Reverse a Face
//=======================================================================

void BRepPrim_Builder::ReverseFace(TopoFace& F) const
{
  F.Reverse();
}

//=======================================================================
// function : AddEdgeVertex
// purpose  : Add a Vertex to an Edge
//=======================================================================

void BRepPrim_Builder::AddEdgeVertex(TopoEdge&           E,
                                     const TopoVertex&   V,
                                     const Standard_Real    P,
                                     const Standard_Boolean direct) const
{
  TopoVertex VV = V;
  if (!direct)
    VV.Reverse();
  myBuilder.Add(E, VV);
  myBuilder.UpdateVertex(VV, P, E, Precision::Confusion());
}

//=======================================================================
// function : AddEdgeVertex
// purpose  : Add a Vertex to an Edge
//=======================================================================

void BRepPrim_Builder::AddEdgeVertex(TopoEdge&         E,
                                     const TopoVertex& V,
                                     const Standard_Real  P1,
                                     const Standard_Real  P2) const
{
  TopoVertex VV = V;
  VV.Orientation(TopAbs_FORWARD);
  myBuilder.Add(E, VV);
  VV.Orientation(TopAbs_REVERSED);
  myBuilder.Add(E, VV);
  myBuilder.Range(E, P1, P2);
}

//=================================================================================================

void BRepPrim_Builder::SetParameters(TopoEdge& E,
                                     const TopoVertex&,
                                     const Standard_Real P1,
                                     const Standard_Real P2) const
{
  myBuilder.Range(E, P1, P2);
}

//=======================================================================
// function : AddWireEdge
// purpose  : Add an Edge to a Wire
//=======================================================================

void BRepPrim_Builder::AddWireEdge(TopoWire&           W,
                                   const TopoEdge&     E,
                                   const Standard_Boolean direct) const
{
  TopoEdge EE = E;
  if (!direct)
    EE.Reverse();
  myBuilder.Add(W, EE);
}

//=======================================================================
// function : AddFaceWire
// purpose  : Add a Wire to a Face
//=======================================================================

void BRepPrim_Builder::AddFaceWire(TopoFace& F, const TopoWire& W) const
{
  myBuilder.Add(F, W);
}

//=======================================================================
// function : AddShellFace
// purpose  : Add a Face to a Shell
//=======================================================================

void BRepPrim_Builder::AddShellFace(TopoShell& S, const TopoFace& F) const
{
  myBuilder.Add(S, F);
}

//=================================================================================================

void BRepPrim_Builder::CompleteEdge(TopoEdge& E) const
{
  BRepTools1::Update(E);
}

//=================================================================================================

void BRepPrim_Builder::CompleteWire(TopoWire& W) const
{
  W.Closed(BRepInspector::IsClosed(W));
  BRepTools1::Update(W);
}

//=================================================================================================

void BRepPrim_Builder::CompleteFace(TopoFace& F) const
{
  BRepTools1::Update(F);
}

//=================================================================================================

void BRepPrim_Builder::CompleteShell(TopoShell& S) const
{
  S.Closed(BRepInspector::IsClosed(S));
  BRepTools1::Update(S);
}
