// Created on: 1998-04-27
// Created by: Roman LYGIN
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

// Modified:	Thu Sep 17 12:27:58 1998
// 25.12.98 pdn transmission from BRepTools_Sewing to BRepBuilderAPI_Sewing
// szv#4 S4163
// 11.01.00 svv #1: porting on DEC

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <gp_Pnt.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_BoxBndTree.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeAnalysis_Shell.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeBuild_Edge.hxx>
#include <ShapeBuild_Vertex.hxx>
#include <ShapeExtend_Explorer.hxx>
#include <ShapeExtend_WireData.hxx>
#include <TColStd_Array1OfBoolean.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_SequenceOfInteger.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_HArray1OfShape.hxx>
#include <TopTools_MapOfShape.hxx>

// ied_modif_for_compil_Nov-19-1998
//=======================================================================
// function : ShapeAnalysis_FreeBounds
// purpose  : Empty constructor
//=======================================================================
ShapeAnalysis_FreeBounds::ShapeAnalysis_FreeBounds() {}

//=================================================================================================

ShapeAnalysis_FreeBounds::ShapeAnalysis_FreeBounds(const TopoShape&    shape,
                                                   const Standard_Real    toler,
                                                   const Standard_Boolean splitclosed,
                                                   const Standard_Boolean splitopen)
    : myTolerance(toler),
      myShared(Standard_False),
      mySplitClosed(splitclosed),
      mySplitOpen(splitopen)
{
  BRepBuilderAPI_Sewing Sew(toler, Standard_False, Standard_False);
  for (TopoDS_Iterator S(shape); S.More(); S.Next())
    Sew.Add(S.Value());
  Sew.Perform();
  //
  // Extract free edges.
  //
  Standard_Integer                  nbedge = Sew.NbFreeEdges();
  Handle(TopTools_HSequenceOfShape) edges  = new TopTools_HSequenceOfShape;
  Handle(TopTools_HSequenceOfShape) wires;
  TopoEdge                       anEdge;
  for (Standard_Integer iedge = 1; iedge <= nbedge; iedge++)
  {
    anEdge = TopoDS::Edge(Sew.FreeEdge(iedge));
    if (!BRepInspector::Degenerated(anEdge))
      edges->Append(anEdge);
  }
  //
  // Chainage.
  //
  ConnectEdgesToWires(edges, toler, Standard_False, wires);
  DispatchWires(wires, myWires, myEdges);
  SplitWires();

  return;
}

//=================================================================================================

ShapeAnalysis_FreeBounds::ShapeAnalysis_FreeBounds(const TopoShape&    shape,
                                                   const Standard_Boolean splitclosed,
                                                   const Standard_Boolean splitopen,
                                                   const Standard_Boolean checkinternaledges)
    : myTolerance(0.),
      myShared(Standard_True),
      mySplitClosed(splitclosed),
      mySplitOpen(splitopen)
{
  TopoShell aTmpShell;
  ShapeBuilder aB;
  aB.MakeShell(aTmpShell);
  for (ShapeExplorer aExpFace(shape, TopAbs_FACE); aExpFace.More(); aExpFace.Next())
    aB.Add(aTmpShell, aExpFace.Current());

  ShapeAnalysis_Shell sas;
  sas.CheckOrientedShells(aTmpShell, Standard_True, checkinternaledges);

  if (sas.HasFreeEdges())
  {
    ShapeExtend_Explorer              see;
    Handle(TopTools_HSequenceOfShape) edges = see.SeqFromCompound(sas.FreeEdges(), Standard_False);

    Handle(TopTools_HSequenceOfShape) wires;
    ConnectEdgesToWires(edges, Precision::Confusion(), Standard_True, wires);
    DispatchWires(wires, myWires, myEdges);
    SplitWires();
  }
}

//=================================================================================================

void ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Handle(TopTools_HSequenceOfShape)& edges,
                                                   const Standard_Real                toler,
                                                   const Standard_Boolean             shared,
                                                   Handle(TopTools_HSequenceOfShape)& wires)
{
  Handle(TopTools_HSequenceOfShape) iwires = new TopTools_HSequenceOfShape;
  ShapeBuilder                      B;

  Standard_Integer i; // svv #1
  for (i = 1; i <= edges->Length(); i++)
  {
    TopoWire wire;
    B.MakeWire(wire);
    B.Add(wire, edges->Value(i));
    iwires->Append(wire);
  }

  ConnectWiresToWires(iwires, toler, shared, wires);

  for (i = 1; i <= edges->Length(); i++)
    if (iwires->Value(i).Orientation() == TopAbs_REVERSED)
      edges->ChangeValue(i).Reverse();
}

//=================================================================================================

void ShapeAnalysis_FreeBounds::ConnectWiresToWires(Handle(TopTools_HSequenceOfShape)& iwires,
                                                   const Standard_Real                toler,
                                                   const Standard_Boolean             shared,
                                                   Handle(TopTools_HSequenceOfShape)& owires)
{
  TopTools_DataMapOfShapeShape map;
  ConnectWiresToWires(iwires, toler, shared, owires, map);
}

//=================================================================================================

void ShapeAnalysis_FreeBounds::ConnectWiresToWires(Handle(TopTools_HSequenceOfShape)& iwires,
                                                   const Standard_Real                toler,
                                                   const Standard_Boolean             shared,
                                                   Handle(TopTools_HSequenceOfShape)& owires,
                                                   TopTools_DataMapOfShapeShape&      vertices)
{
  if (iwires.IsNull() || !iwires->Length())
    return;
  Handle(TopTools_HArray1OfShape) arrwires = new TopTools_HArray1OfShape(1, iwires->Length());
  // amv
  Standard_Integer i;
  for (i = 1; i <= arrwires->Length(); i++)
    arrwires->SetValue(i, iwires->Value(i));
  owires                  = new TopTools_HSequenceOfShape;
  Standard_Real tolerance = Max(toler, Precision::Confusion());

  Handle(ShapeExtend_WireData) sewd = new ShapeExtend_WireData(TopoDS::Wire(arrwires->Value(1)));

  Standard_Boolean isUsedManifoldMode = Standard_True;

  if ((sewd->NbEdges() < 1) && (sewd->NbNonManifoldEdges() > 0))
  {
    isUsedManifoldMode = Standard_False;
    sewd =
      new ShapeExtend_WireData(TopoDS::Wire(arrwires->Value(1)), Standard_True, isUsedManifoldMode);
  }

  Handle(ShapeAnalysis_Wire) saw = new ShapeAnalysis_Wire;
  saw->Load(sewd);
  saw->SetPrecision(tolerance);

  ShapeAnalysis_BoxBndTree                            aBBTree;
  NCollection_UBTreeFiller<Standard_Integer, Box2> aTreeFiller(aBBTree);
  ShapeAnalysis_BoxBndTreeSelector                    aSel(arrwires, shared);
  aSel.LoadList(1);

  for (Standard_Integer inbW = 2; inbW <= arrwires->Length(); inbW++)
  {
    TopoWire   trW = TopoDS::Wire(arrwires->Value(inbW));
    Box2       aBox;
    TopoVertex trV1, trV2;
    ShapeAnalysis::FindBounds(trW, trV1, trV2);
    Point3d trP1 = BRepInspector::Pnt(trV1);
    Point3d trP2 = BRepInspector::Pnt(trV2);
    aBox.Set(trP1);
    aBox.Add(trP2);
    aBox.SetGap(tolerance);
    aTreeFiller.Add(inbW, aBox);
  }

  aTreeFiller.Fill();
  Standard_Integer nsel;

  ShapeAnalysis_Edge sae; // szv#4:S4163:12Mar99 moved
  Standard_Boolean   done = Standard_False;

  while (!done)
  {
    Standard_Boolean found = Standard_False, tail = Standard_False, direct = Standard_False;
    Standard_Integer lwire = 0;
    aSel.SetStop();
    Box2       FVBox, LVBox;
    TopoVertex Vf, Vl;
    Vf = sae.FirstVertex(sewd->Edge(1));
    Vl = sae.LastVertex(sewd->Edge(sewd->NbEdges()));

    Point3d pf, pl;
    pf = BRepInspector::Pnt(Vf);
    pl = BRepInspector::Pnt(Vl);
    FVBox.Set(pf);
    FVBox.SetGap(tolerance);
    LVBox.Set(pl);
    LVBox.SetGap(tolerance);

    aSel.DefineBoxes(FVBox, LVBox);

    if (shared)
      aSel.DefineVertexes(Vf, Vl);
    else
    {
      aSel.DefinePnt(pf, pl);
      aSel.SetTolerance(tolerance);
    }

    nsel = aBBTree.Select(aSel);

    if (nsel != 0 && !aSel.LastCheckStatus(ShapeExtend_FAIL))
    {
      found  = Standard_True;
      lwire  = aSel.GetNb();
      tail   = aSel.LastCheckStatus(ShapeExtend_DONE1) || aSel.LastCheckStatus(ShapeExtend_DONE2);
      direct = aSel.LastCheckStatus(ShapeExtend_DONE1) || aSel.LastCheckStatus(ShapeExtend_DONE3);
      aSel.LoadList(lwire);
    }

    if (found)
    {
      if (!direct)
        arrwires->ChangeValue(lwire).Reverse();

      TopoWire                  aCurW = TopoDS::Wire(arrwires->Value(lwire));
      Handle(ShapeExtend_WireData) acurwd =
        new ShapeExtend_WireData(TopoDS::Wire(arrwires->Value(lwire)),
                                 Standard_True,
                                 isUsedManifoldMode);
      if (!acurwd->NbEdges())
        continue;
      sewd->Add(acurwd, (tail ? 0 : 1));
    }
    else
    {
      // making wire
      // 1.providing connection (see WireHealer::FixConnected())
      // Standard_Integer i; // svv #1
      for (/*Standard_Integer*/ i = 1; i <= saw->NbEdges(); i++)
      {
        if (saw->CheckConnected(i))
        {
          Standard_Integer n2 = i;
          Standard_Integer n1 = (n2 > 1 ? n2 - 1 : saw->NbEdges());
          TopoEdge      E1 = sewd->Edge(n1);
          TopoEdge      E2 = sewd->Edge(n2);

          TopoVertex Vprev, Vfol, V; // connection vertex
          Vprev = sae.LastVertex(E1);
          Vfol  = sae.FirstVertex(E2);

          if (saw->LastCheckStatus(ShapeExtend_DONE1)) // absolutely confused
            V = Vprev;
          else
          {
            ShapeBuild_Vertex sbv;
            V = sbv.CombineVertex(Vprev, Vfol);
          }
          vertices.Bind(Vprev, V);
          vertices.Bind(Vfol, V);

          // replace vertices to a new one
          ShapeBuild_Edge sbe;
          if (saw->NbEdges() < 2)
            sewd->Set(sbe.CopyReplaceVertices(E2, V, V), n2);
          else
          {
            sewd->Set(sbe.CopyReplaceVertices(E2, V, TopoVertex()), n2);
            if (!saw->LastCheckStatus(ShapeExtend_DONE1))
              sewd->Set(sbe.CopyReplaceVertices(E1, TopoVertex(), V), n1);
          }
        }
      }

      // 2.making wire
      TopoWire wire = sewd->Wire();
      if (isUsedManifoldMode)
      {
        if (!saw->CheckConnected(1) && saw->LastCheckStatus(ShapeExtend_OK))
          wire.Closed(Standard_True);
      }
      else
      {
        // Try to check connection by number of free vertices
        TopTools_MapOfShape vmap;
        TopoDS_Iterator     it(wire);

        for (; it.More(); it.Next())
        {
          const TopoShape& E = it.Value();
          TopoDS_Iterator     ite(E, Standard_False, Standard_True);
          for (; ite.More(); ite.Next())
          {
            const TopoShape& V = ite.Value();
            if (V.Orientation() == TopAbs_FORWARD || V.Orientation() == TopAbs_REVERSED)
            {
              // add or remove in the vertex map
              if (!vmap.Add(V))
                vmap.Remove(V);
            }
          }
        }
        if (vmap.IsEmpty())
        {
          wire.Closed(Standard_True);
        }
      }

      owires->Append(wire);
      sewd->Clear();
      sewd->ManifoldMode() = isUsedManifoldMode;

      // Recherche de la premier edge non traitee pour un autre wire.
      // Searching for first edge for next wire
      lwire = -1;
      for (/*Standard_Integer*/ i = 1; i <= arrwires->Length(); i++)
      {
        if (!aSel.ContWire(i))
        {
          lwire = i; // szv#4:S4163:12Mar99 optimized
          sewd->Add(TopoDS::Wire(arrwires->Value(lwire)));
          aSel.LoadList(lwire);

          if (sewd->NbEdges() > 0)
            break;
          sewd->Clear();
        }
      }

      if (lwire == -1)
        done = 1;
    }
  }

  for (/*Standard_Integer*/ i = 1; i <= iwires->Length(); i++)
  {
    iwires->SetValue(i, arrwires->Value(i));
  }
}

static void SplitWire(const TopoWire&                 wire,
                      const Standard_Real                toler,
                      const Standard_Boolean             shared,
                      Handle(TopTools_HSequenceOfShape)& closed,
                      Handle(TopTools_HSequenceOfShape)& open)
{
  closed                  = new TopTools_HSequenceOfShape;
  open                    = new TopTools_HSequenceOfShape;
  Standard_Real tolerance = Max(toler, Precision::Confusion());

  ShapeBuilder       B;
  ShapeAnalysis_Edge sae;

  Handle(ShapeExtend_WireData) sewd    = new ShapeExtend_WireData(wire);
  Standard_Integer             nbedges = sewd->NbEdges();

  // ConnectedEdgeSequence - list of indices of connected edges to build a wire
  TColStd_SequenceOfInteger ces;
  // statuses - array of flags describing the edge:
  // 0-free, 1-in CES, 2-already in wire,
  // 3-no closed wire can be produced starting at this edge
  TColStd_Array1OfInteger statuses(1, nbedges);
  statuses.Init(0);

  // building closed wires
  Standard_Integer i; // svv #1
  for (i = 1; i <= nbedges; i++)
    if (statuses.Value(i) == 0)
    {
      ces.Append(i);
      statuses.SetValue(i, 1); // putting into CES
      Standard_Boolean SearchBackward = Standard_True;

      for (;;)
      {
        Standard_Boolean found;
        TopoEdge      edge;
        TopoVertex    lvertex;
        Point3d           lpoint;

        // searching for connection in ces
        if (SearchBackward)
        {
          SearchBackward = Standard_False;
          found          = Standard_False;
          edge           = sewd->Edge(ces.Last());
          lvertex        = sae.LastVertex(edge);
          lpoint         = BRepInspector::Pnt(lvertex);
          Standard_Integer j; // svv #1
          for (j = ces.Length(); (j >= 1) && !found; j--)
          {
            TopoVertex fv = sae.FirstVertex(sewd->Edge(ces.Value(j)));
            Point3d        fp = BRepInspector::Pnt(fv);
            if ((shared && lvertex.IsSame(fv)) || (!shared && lpoint.IsEqual(fp, tolerance)))
              found = Standard_True;
          }

          if (found)
          {
            j++; // because of decreasing last iteration
            // making closed wire
            TopoWire wire1;
            B.MakeWire(wire1);
            for (Standard_Integer cesindex = j; cesindex <= ces.Length(); cesindex++)
            {
              B.Add(wire1, sewd->Edge(ces.Value(cesindex)));
              statuses.SetValue(ces.Value(cesindex), 2);
            }
            wire1.Closed(Standard_True);
            closed->Append(wire1);
            ces.Remove(j, ces.Length());
            if (ces.IsEmpty())
              break;
          }
        }

        // searching for connection among free edges
        found   = Standard_False;
        edge    = sewd->Edge(ces.Last());
        lvertex = sae.LastVertex(edge);
        lpoint  = BRepInspector::Pnt(lvertex);
        Standard_Integer j; // svv #1
        for (j = 1; (j <= nbedges) && !found; j++)
          if (statuses.Value(j) == 0)
          {
            TopoVertex fv = sae.FirstVertex(sewd->Edge(j));
            Point3d        fp = BRepInspector::Pnt(fv);
            if ((shared && lvertex.IsSame(fv)) || (!shared && lpoint.IsEqual(fp, tolerance)))
              found = Standard_True;
          }

        if (found)
        {
          j--; // because of last iteration
          ces.Append(j);
          statuses.SetValue(j, 1); // putting into CES
          SearchBackward = Standard_True;
          continue;
        }

        // no edges found - mark the branch as open (use status 3)
        statuses.SetValue(ces.Last(), 3);
        ces.Remove(ces.Length());
        if (ces.IsEmpty())
          break;
      }
    }

  // building open wires
  Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape;
  for (i = 1; i <= nbedges; i++)
    if (statuses.Value(i) != 2)
      edges->Append(sewd->Edge(i));

  ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, toler, shared, open);
}

void ShapeAnalysis_FreeBounds::SplitWires(const Handle(TopTools_HSequenceOfShape)& wires,
                                          const Standard_Real                      toler,
                                          const Standard_Boolean                   shared,
                                          Handle(TopTools_HSequenceOfShape)&       closed,
                                          Handle(TopTools_HSequenceOfShape)&       open)
{
  closed = new TopTools_HSequenceOfShape;
  open   = new TopTools_HSequenceOfShape;

  for (Standard_Integer i = 1; i <= wires->Length(); i++)
  {
    Handle(TopTools_HSequenceOfShape) tmpclosed, tmpopen;
    SplitWire(TopoDS::Wire(wires->Value(i)), toler, shared, tmpclosed, tmpopen);
    closed->Append(tmpclosed);
    open->Append(tmpopen);
  }
}

//=================================================================================================

void ShapeAnalysis_FreeBounds::DispatchWires(const Handle(TopTools_HSequenceOfShape)& wires,
                                             TopoCompound&                         closed,
                                             TopoCompound&                         open)
{
  ShapeBuilder B;
  if (closed.IsNull())
    B.MakeCompound(closed);
  if (open.IsNull())
    B.MakeCompound(open);
  if (wires.IsNull())
    return;

  for (Standard_Integer iw = 1; iw <= wires->Length(); iw++)
    if (wires->Value(iw).Closed())
      B.Add(closed, wires->Value(iw));
    else
      B.Add(open, wires->Value(iw));
}

//=======================================================================
// function : SplitWires
// purpose  : Splits compounds of closed (myWires) and open (myEdges) wires
//           into small closed wires according to fields mySplitClosed and
//           mySplitOpen and rebuilds compounds
//=======================================================================

void ShapeAnalysis_FreeBounds::SplitWires()
{
  if (!mySplitClosed && !mySplitOpen)
    return; // nothing to do

  ShapeExtend_Explorer              see;
  Handle(TopTools_HSequenceOfShape) closedwires, cw1, cw2, openwires, ow1, ow2;
  closedwires = see.SeqFromCompound(myWires, Standard_False);
  openwires   = see.SeqFromCompound(myEdges, Standard_False);

  if (mySplitClosed)
    SplitWires(closedwires, myTolerance, myShared, cw1, ow1);
  else
  {
    cw1 = closedwires;
    ow1 = new TopTools_HSequenceOfShape;
  }

  if (mySplitOpen)
    SplitWires(openwires, myTolerance, myShared, cw2, ow2);
  else
  {
    cw2 = new TopTools_HSequenceOfShape;
    ow2 = openwires;
  }

  closedwires = cw1;
  closedwires->Append(cw2);
  openwires = ow1;
  openwires->Append(ow2);

  // szv#4:S4163:12Mar99 SGI warns
  TopoShape compWires = see.CompoundFromSeq(closedwires);
  TopoShape compEdges = see.CompoundFromSeq(openwires);
  myWires                = TopoDS::Compound(compWires);
  myEdges                = TopoDS::Compound(compEdges);
}
