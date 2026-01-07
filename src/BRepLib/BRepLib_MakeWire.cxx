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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepLib_MakeWire.hxx>
#include <gp_Pnt.hxx>
#include <StdFail_NotDone.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

BRepLib_MakeWire::BRepLib_MakeWire()
    : myError(BRepLib_EmptyWire)
{
}

//=================================================================================================

BRepLib_MakeWire::BRepLib_MakeWire(const TopoEdge& E)
{
  Add(E);
}

//=================================================================================================

BRepLib_MakeWire::BRepLib_MakeWire(const TopoEdge& E1, const TopoEdge& E2)
{
  Add(E1);
  Add(E2);
}

//=================================================================================================

BRepLib_MakeWire::BRepLib_MakeWire(const TopoEdge& E1,
                                   const TopoEdge& E2,
                                   const TopoEdge& E3)
{
  Add(E1);
  Add(E2);
  Add(E3);
}

//=================================================================================================

BRepLib_MakeWire::BRepLib_MakeWire(const TopoEdge& E1,
                                   const TopoEdge& E2,
                                   const TopoEdge& E3,
                                   const TopoEdge& E4)
{
  Add(E1);
  Add(E2);
  Add(E3);
  Add(E4);
}

//=================================================================================================

BRepLib_MakeWire::BRepLib_MakeWire(const TopoWire& W)
{
  Add(W);
}

//=================================================================================================

BRepLib_MakeWire::BRepLib_MakeWire(const TopoWire& W, const TopoEdge& E)
{
  Add(W);
  Add(E);
}

//=================================================================================================

void BRepLib_MakeWire::Add(const TopoWire& W)
{
  for (TopoDS_Iterator it(W); it.More(); it.Next())
  {
    Add(TopoDS::Edge(it.Value()));
    if (myError != BRepLib_WireDone)
      break;
  }
}

//=================================================================================================

void BRepLib_MakeWire::Add(const TopoEdge& E)
{
  Add(E, Standard_True);
}

//=======================================================================
// function : Add
// purpose  :
// PMN  19/03/1998  For the Problem of performance TopExp1::Vertices are not used on wire
// PMN  10/09/1998  In case if the wire is previously closed (or degenerated)
//                  TopExp1::Vertices is used to reduce the ambiguity.
// IsCheckGeometryProximity flag : If true => check for the geometry proximity of vertices
//=======================================================================
void BRepLib_MakeWire::Add(const TopoEdge& E, Standard_Boolean IsCheckGeometryProximity)
{

  Standard_Boolean forward = Standard_False;
  // to tell if it has been decided to add forward
  Standard_Boolean reverse = Standard_False;
  // to tell if it has been decided to add reversed
  Standard_Boolean init = Standard_False;
  // To know if it is necessary to calculate VL, VF
  ShapeBuilder    B;
  TopoDS_Iterator it;

  if (myEdge.IsNull())
  {
    init = Standard_True;
    // first edge, create the wire
    B.MakeWire(TopoDS::Wire(myShape));

    // set the edge
    myEdge = E;

    // add the vertices
    for (it.Initialize(myEdge); it.More(); it.Next())
      myVertices.Add(it.Value());
  }

  else
  {
    init                     = myShape.Closed(); // If it is closed no control
    TopoShape aLocalShape = E.Oriented(TopAbs_FORWARD);
    TopoEdge  EE          = TopoDS::Edge(aLocalShape);
    //    TopoEdge EE = TopoDS::Edge(E.Oriented(TopAbs_FORWARD));

    // test the vertices of the edge

    Standard_Boolean connected = Standard_False;
    Standard_Boolean copyedge  = Standard_False;

    if (myError != BRepLib_NonManifoldWire)
    {
      if (VF.IsNull() || VL.IsNull())
        myError = BRepLib_NonManifoldWire;
    }

    for (it.Initialize(EE); it.More(); it.Next())
    {

      const TopoVertex& VE = TopoDS::Vertex(it.Value());

      // if the vertex is in the wire, ok for the connection
      if (myVertices.Contains(VE))
      {
        connected = Standard_True;
        myVertex  = VE;
        if (myError != BRepLib_NonManifoldWire)
        {
          // is it always so ?
          if (VF.IsSame(VL))
          {
            // Orientation indetermined (in 3d) : Preserve the initial
            if (!VF.IsSame(VE))
              myError = BRepLib_NonManifoldWire;
          }
          else
          {
            if (VF.IsSame(VE))
            {
              if (VE.Orientation() == TopAbs_FORWARD)
                reverse = Standard_True;
              else
                forward = Standard_True;
            }
            else if (VL.IsSame(VE))
            {
              if (VE.Orientation() == TopAbs_REVERSED)
                reverse = Standard_True;
              else
                forward = Standard_True;
            }
            else
              myError = BRepLib_NonManifoldWire;
          }
        }
      }
      else if (IsCheckGeometryProximity)
      {
        // search if there is a similar vertex in the edge
        Point3d PE = BRepInspector::Pnt(VE);

        for (Standard_Integer i = 1; i <= myVertices.Extent(); i++)
        {
          const TopoVertex& VW = TopoDS::Vertex(myVertices.FindKey(i));
          Point3d               PW = BRepInspector::Pnt(VW);
          Standard_Real        l  = PE.Distance(PW);

          if ((l < BRepInspector::Tolerance(VE)) || (l < BRepInspector::Tolerance(VW)))
          {
            copyedge = Standard_True;
            if (myError != BRepLib_NonManifoldWire)
            {
              // is it always so ?
              if (VF.IsSame(VL))
              {
                // Orientation indetermined (in 3d) : Preserve the initial
                if (!VF.IsSame(VW))
                  myError = BRepLib_NonManifoldWire;
              }
              else
              {
                if (VF.IsSame(VW))
                {
                  if (VE.Orientation() == TopAbs_FORWARD)
                    reverse = Standard_True;
                  else
                    forward = Standard_True;
                }
                else if (VL.IsSame(VW))
                {
                  if (VE.Orientation() == TopAbs_REVERSED)
                    reverse = Standard_True;
                  else
                    forward = Standard_True;
                }
                else
                  myError = BRepLib_NonManifoldWire;
              }
            }
            break;
          }
        }
        if (copyedge)
        {
          connected = Standard_True;
        }
      }
    }

    if (!connected)
    {
      myError = BRepLib_DisconnectedWire;
      NotDone();
      return;
    }
    else
    {
      if (!copyedge)
      {
        myEdge = EE;
        for (it.Initialize(EE); it.More(); it.Next())
          myVertices.Add(it.Value());
      }
      else
      {
        // copy the edge
        TopoShape Dummy = EE.EmptyCopied();
        myEdge             = TopoDS::Edge(Dummy);

        for (it.Initialize(EE); it.More(); it.Next())
        {

          const TopoVertex& VE = TopoDS::Vertex(it.Value());
          Point3d               PE = BRepInspector::Pnt(VE);

          Standard_Boolean newvertex = Standard_False;
          for (Standard_Integer i = 1; i <= myVertices.Extent(); i++)
          {
            const TopoVertex& VW = TopoDS::Vertex(myVertices.FindKey(i));
            Point3d               PW = BRepInspector::Pnt(VW);
            Standard_Real        l  = PE.Distance(PW), tolE, tolW;
            tolW                    = BRepInspector::Tolerance(VW);
            tolE                    = BRepInspector::Tolerance(VE);

            if ((l < tolE) || (l < tolW))
            {

              Standard_Real maxtol = .5 * (tolW + tolE + l), cW, cE;
              if (maxtol > tolW && maxtol > tolE)
              {
                cW = (maxtol - tolE) / l;
                cE = 1. - cW;
              }
              else if (maxtol > tolW)
              {
                maxtol = tolE;
                cW     = 0.;
                cE     = 1.;
              }
              else
              {
                maxtol = tolW;
                cW     = 1.;
                cE     = 0.;
              }

              Point3d PC(cW * PW.X() + cE * PE.X(),
                        cW * PW.Y() + cE * PE.Y(),
                        cW * PW.Z() + cE * PE.Z());

              B.UpdateVertex(VW, PC, maxtol);

              newvertex = Standard_True;
              myVertex  = VW;
              myVertex.Orientation(VE.Orientation());
              B.Add(myEdge, myVertex);
              B.Transfert(EE, myEdge, VE, myVertex);
              break;
            }
          }
          if (!newvertex)
          {
            myVertices.Add(VE);
            B.Add(myEdge, VE);
            B.Transfert(EE, myEdge, VE, VE);
          }
        }
      }
    }
    // Make a decision about the orientation of the edge
    // If there is an ambiguity (in 3d) preserve the orientation given at input
    // Case of ambiguity :
    // reverse and forward are false as nothing has been decided :
    //       closed wire, internal vertex ...
    // reverse and forward are true : closed or degenerated edge
    if (((forward == reverse) && (E.Orientation() == TopAbs_REVERSED)) || (reverse && !forward))
      myEdge.Reverse();
  }

  // add myEdge to myShape
  B.Add(myShape, myEdge);
  myShape.Closed(Standard_False);

  // Initialize VF, VL
  if (init)
    TopExp1::Vertices(TopoDS::Wire(myShape), VF, VL);
  else
  {
    if (myError == BRepLib_WireDone)
    { // Update only
      TopoVertex V1, V2, VRef;
      TopExp1::Vertices(myEdge, V1, V2);
      if (V1.IsSame(myVertex))
        VRef = V2;
      else if (V2.IsSame(myVertex))
        VRef = V1;
      else
      {
#ifdef OCCT_DEBUG
        std::cout << "MakeWire : There is a PROBLEM !!" << std::endl;
#endif
        myError = BRepLib_NonManifoldWire;
      }

      if (VF.IsSame(VL))
      {
        // Particular case: it is required to control the orientation
#ifdef OCCT_DEBUG
        if (!VF.IsSame(myVertex))
          std::cout << "MakeWire : There is a PROBLEM !!" << std::endl;
#endif
      }
      else
      { // General case
        if (VF.IsSame(myVertex))
          VF = VRef;
        else if (VL.IsSame(myVertex))
          VL = VRef;
        else
        {
#ifdef OCCT_DEBUG
          std::cout << "MakeWire : Y A UN PROBLEME !!" << std::endl;
#endif
          myError = BRepLib_NonManifoldWire;
        }
      }
    }
    if (myError == BRepLib_NonManifoldWire)
    {
      VF = VL = TopoVertex(); // nullify
    }
  }
  // Test myShape is closed
  if (!VF.IsNull() && !VL.IsNull() && VF.IsSame(VL))
    myShape.Closed(Standard_True);

  myError = BRepLib_WireDone;
  Done();
}

//=================================================================================================

const TopoWire& BRepLib_MakeWire::Wire()
{
  return TopoDS::Wire(Shape());
}

//=================================================================================================

const TopoEdge& BRepLib_MakeWire::Edge() const
{
  return myEdge;
}

//=================================================================================================

const TopoVertex& BRepLib_MakeWire::Vertex() const
{
  return myVertex;
}

//=================================================================================================

BRepLib_MakeWire::operator TopoWire()
{
  return Wire();
}

//=================================================================================================

BRepLib_WireError BRepLib_MakeWire::Error() const
{
  return myError;
}
