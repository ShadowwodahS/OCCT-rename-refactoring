// Created on: 1993-07-23
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

#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

//=================================================================================================

BRepBuilderAPI_MakeWire::BRepBuilderAPI_MakeWire() {}

//=================================================================================================

BRepBuilderAPI_MakeWire::BRepBuilderAPI_MakeWire(const TopoEdge& E)
    : myMakeWire(E)
{
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

BRepBuilderAPI_MakeWire::BRepBuilderAPI_MakeWire(const TopoEdge& E1, const TopoEdge& E2)
    : myMakeWire(E1, E2)
{
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

BRepBuilderAPI_MakeWire::BRepBuilderAPI_MakeWire(const TopoEdge& E1,
                                                 const TopoEdge& E2,
                                                 const TopoEdge& E3)
    : myMakeWire(E1, E2, E3)
{
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

BRepBuilderAPI_MakeWire::BRepBuilderAPI_MakeWire(const TopoEdge& E1,
                                                 const TopoEdge& E2,
                                                 const TopoEdge& E3,
                                                 const TopoEdge& E4)
    : myMakeWire(E1, E2, E3, E4)
{
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

BRepBuilderAPI_MakeWire::BRepBuilderAPI_MakeWire(const TopoWire& W)
    : myMakeWire(W)
{
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

BRepBuilderAPI_MakeWire::BRepBuilderAPI_MakeWire(const TopoWire& W, const TopoEdge& E)
    : myMakeWire(W, E)
{
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

void BRepBuilderAPI_MakeWire::Add(const TopoWire& W)
{
  myMakeWire.Add(W);
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

void BRepBuilderAPI_MakeWire::Add(const TopoEdge& E)
{
  myMakeWire.Add(E);
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

void BRepBuilderAPI_MakeWire::Add(const ShapeList& L)
{
  myMakeWire.Add(L);
  if (myMakeWire.IsDone())
  {
    Done();
    myShape = myMakeWire.Wire();
  }
}

//=================================================================================================

const TopoWire& BRepBuilderAPI_MakeWire::Wire()
{
  return myMakeWire.Wire();
}

//=================================================================================================

const TopoEdge& BRepBuilderAPI_MakeWire::Edge() const
{
  return myMakeWire.Edge();
}

//=================================================================================================

const TopoVertex& BRepBuilderAPI_MakeWire::Vertex() const
{
  return myMakeWire.Vertex();
}

//=================================================================================================

BRepBuilderAPI_MakeWire::operator TopoWire()
{
  return Wire();
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_MakeWire::IsDone() const
{
  return myMakeWire.IsDone();
}

//=================================================================================================

BRepBuilderAPI_WireError BRepBuilderAPI_MakeWire::Error() const
{
  switch (myMakeWire.Error())
  {

    case BRepLib_WireDone:
      return BRepBuilderAPI_WireDone;

    case BRepLib_EmptyWire:
      return BRepBuilderAPI_EmptyWire;

    case BRepLib_DisconnectedWire:
      return BRepBuilderAPI_DisconnectedWire;

    case BRepLib_NonManifoldWire:
      return BRepBuilderAPI_NonManifoldWire;
  }

  // portage WNT
  return BRepBuilderAPI_WireDone;
}
