// Created on: 1993-07-29
// Created by: Remi LEQUETTE
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

#include <BRep_Builder.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakePolygon.hxx>
#include <BRepTools.hxx>
#include <gp_Pnt.hxx>
#include <Precision.hxx>
#include <StdFail_NotDone.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

//=================================================================================================

BRepLib_MakePolygon::BRepLib_MakePolygon() {}

//=================================================================================================

BRepLib_MakePolygon::BRepLib_MakePolygon(const Point3d& P1, const Point3d& P2)
{
  Add(P1);
  Add(P2);
}

//=================================================================================================

BRepLib_MakePolygon::BRepLib_MakePolygon(const Point3d&          P1,
                                         const Point3d&          P2,
                                         const Point3d&          P3,
                                         const Standard_Boolean Cl)
{
  Add(P1);
  Add(P2);
  Add(P3);
  if (Cl)
    Close();
}

//=================================================================================================

BRepLib_MakePolygon::BRepLib_MakePolygon(const Point3d&          P1,
                                         const Point3d&          P2,
                                         const Point3d&          P3,
                                         const Point3d&          P4,
                                         const Standard_Boolean Cl)
{
  Add(P1);
  Add(P2);
  Add(P3);
  Add(P4);
  if (Cl)
    Close();
}

//=================================================================================================

BRepLib_MakePolygon::BRepLib_MakePolygon(const TopoVertex& V1, const TopoVertex& V2)
{
  Add(V1);
  Add(V2);
}

//=================================================================================================

BRepLib_MakePolygon::BRepLib_MakePolygon(const TopoVertex&   V1,
                                         const TopoVertex&   V2,
                                         const TopoVertex&   V3,
                                         const Standard_Boolean Cl)
{
  Add(V1);
  Add(V2);
  Add(V3);
  if (Cl)
    Close();
}

//=================================================================================================

BRepLib_MakePolygon::BRepLib_MakePolygon(const TopoVertex&   V1,
                                         const TopoVertex&   V2,
                                         const TopoVertex&   V3,
                                         const TopoVertex&   V4,
                                         const Standard_Boolean Cl)
{
  Add(V1);
  Add(V2);
  Add(V3);
  Add(V4);
  if (Cl)
    Close();
}

//=================================================================================================

void BRepLib_MakePolygon::Add(const Point3d& P)
{
  ShapeBuilder  B;
  TopoVertex V;
  B.MakeVertex(V, P, Precision::Confusion());
  Add(V);
}

//=================================================================================================

void BRepLib_MakePolygon::Add(const TopoVertex& V)
{
  if (myFirstVertex.IsNull())
  {
    myFirstVertex = V;
  }
  else
  {
    myEdge.Nullify();
    ShapeBuilder  B;
    TopoVertex last;

    Standard_Boolean second = myLastVertex.IsNull();
    if (second)
    {
      last         = myFirstVertex;
      myLastVertex = V;
      B.MakeWire(TopoDS::Wire(myShape));
      myShape.Closed(Standard_False);
      myShape.Orientable(Standard_True);
    }
    else
    {
      last = myLastVertex;
      if (BRepTools1::Compare(V, myFirstVertex))
      {
        myLastVertex = myFirstVertex;
        myShape.Closed(Standard_True);
      }
      else
        myLastVertex = V;
    }

    BRepLib_MakeEdge ME(last, myLastVertex);
    if (ME.IsDone())
    {
      myEdge = ME;
      B.Add(myShape, myEdge);
      Done();
    }
    else
    {
      // restore the previous last vertex
      if (second)
        myLastVertex.Nullify();
      else
        myLastVertex = last;
    }
  }
}

//=================================================================================================

Standard_Boolean BRepLib_MakePolygon::Added() const
{
  return !myEdge.IsNull();
}

//=================================================================================================

void BRepLib_MakePolygon::Close()
{
  if (myFirstVertex.IsNull() || myLastVertex.IsNull())
    return;

  // check not already closed
  if (myShape.Closed())
    return;

  // build the last edge
  ShapeBuilder B;
  myEdge.Nullify();
  BRepLib_MakeEdge ME(myLastVertex, myFirstVertex);
  if (ME.IsDone())
  {
    myEdge = ME;
    B.Add(myShape, myEdge);
    myShape.Closed(Standard_True);
  }
}

//=================================================================================================

const TopoVertex& BRepLib_MakePolygon::FirstVertex() const
{
  return myFirstVertex;
}

//=================================================================================================

const TopoVertex& BRepLib_MakePolygon::LastVertex() const
{
  return myLastVertex;
}

//=================================================================================================

const TopoEdge& BRepLib_MakePolygon::Edge() const
{
  return myEdge;
}

//=================================================================================================

const TopoWire& BRepLib_MakePolygon::Wire()
{
  return TopoDS::Wire(Shape());
}

//=================================================================================================

BRepLib_MakePolygon::operator TopoEdge() const
{
  return Edge();
}

//=================================================================================================

BRepLib_MakePolygon::operator TopoWire()
{
  return Wire();
}
