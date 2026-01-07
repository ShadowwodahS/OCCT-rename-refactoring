// Created on: 1992-08-27
// Created by: Christophe MARION
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef No_Exception
// #define No_Exception
#endif

#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <HLRBRep_Data.hxx>
#include <HLRBRep_ShapeToHLR.hxx>
#include <HLRTopoBRep_OutLiner.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

Handle(HLRBRep_Data) ShapeToHLRConverter::Load(const Handle(HLRTopoBRep_OutLiner)& S,
                                              const HLRAlgoProjector&            P,
                                              BRepTopAdaptor_MapOfShapeTool&      MST,
                                              const Standard_Integer              nbIso)
{
  S->Fill(P, MST, nbIso);

  Data1&                         TopDS = S->DataStructure();
  TopTools_IndexedMapOfShape                FM;
  TopTools_IndexedMapOfShape                EM;
  TopTools_IndexedDataMapOfShapeListOfShape VerticesToEdges;
  TopTools_IndexedDataMapOfShapeListOfShape EdgesToFaces;

  ShapeExplorer exshell, exface;

  for (exshell.Init(S->OutLinedShape(), TopAbs_SHELL); exshell.More(); exshell.Next())
  { // faces in a shell

    for (exface.Init(exshell.Current(), TopAbs_FACE); exface.More(); exface.Next())
    {
      if (!FM.Contains(exface.Current()))
        FM.Add(exface.Current());
    }
  }

  for (exface.Init(S->OutLinedShape(), TopAbs_FACE, TopAbs_SHELL); exface.More(); exface.Next())
  { // faces not in a shell
    if (!FM.Contains(exface.Current()))
      FM.Add(exface.Current());
  }

  TopExp1::MapShapes(S->OutLinedShape(), TopAbs_EDGE, EM);

  Standard_Integer i;
  Standard_Integer nbEdge = EM.Extent();

  for (i = 1; i <= nbEdge; i++) // vertices back to edges
    TopExp1::MapShapesAndAncestors(EM(i), TopAbs_VERTEX, TopAbs_EDGE, VerticesToEdges);

  Standard_Integer nbVert = VerticesToEdges.Extent();
  Standard_Integer nbFace = FM.Extent();

  TopoVertex                      VF, VL;
  TopTools_ListIteratorOfListOfShape itn;
  Standard_Integer                   i1, i2;
  Standard_Boolean                   o1, o2;
  Standard_Boolean                   c1, c2;
  Standard_Real                      pf, pl;
  Standard_ShortReal                 tf, tl;

  // Create the data structure
  Handle(HLRBRep_Data) DS = new HLRBRep_Data(nbVert, nbEdge, nbFace);
  HLRBRep_EdgeData*    ed = NULL;
  if (nbEdge != 0)
    ed = &(DS->EDataArray().ChangeValue(1));
  //  ed++;

  for (i = 1; i <= nbFace; i++)
  { // test of Double edges
    TopExp1::MapShapesAndAncestors(FM(i), TopAbs_EDGE, TopAbs_FACE, EdgesToFaces);
  }

  for (i = 1; i <= nbEdge; i++)
  { // load the Edges
    const TopoEdge& Edg = TopoDS::Edge(EM(i));
    TopExp1::Vertices(Edg, VF, VL);
    BRepInspector::Range(Edg, pf, pl);
    Standard_Boolean reg1 = Standard_False;
    Standard_Boolean regn = Standard_False;
    Standard_Integer inde = EdgesToFaces.FindIndex(Edg);
    if (inde > 0)
    {
      if (EdgesToFaces(inde).Extent() == 2)
      {
        itn                   = EdgesToFaces(inde);
        const TopoFace& F1 = TopoDS::Face(itn.Value());
        itn.Next();
        const TopoFace& F2 = TopoDS::Face(itn.Value());
        GeomAbs_Shape      rg = BRepInspector::Continuity(Edg, F1, F2);
        reg1                  = rg >= GeomAbs_G1;
        regn                  = rg >= GeomAbs_G2;
      }
    }

    if (VF.IsNull())
    {
      i1 = 0;
      o1 = Standard_False;
      c1 = Standard_False;
      pf = RealFirst();
      tf = (Standard_ShortReal)Epsilon(pf);
    }
    else
    {
      i1 = VerticesToEdges.FindIndex(VF);
      o1 = TopDS.IsOutV(VF);
      c1 = TopDS.IsIntV(VF);
      tf = (Standard_ShortReal)BRepInspector::Tolerance(VF);
    }

    if (VL.IsNull())
    {
      i2 = 0;
      o2 = Standard_False;
      c2 = Standard_False;
      pl = RealLast();
      tl = (Standard_ShortReal)Epsilon(pl);
    }
    else
    {
      i2 = VerticesToEdges.FindIndex(VL);
      o2 = TopDS.IsOutV(VL);
      c2 = TopDS.IsIntV(VL);
      tl = (Standard_ShortReal)BRepInspector::Tolerance(VL);
    }

    ed->Set(reg1, regn, Edg, i1, i2, o1, o2, c1, c2, pf, tf, pl, tl);
    DS->EdgeMap().Add(Edg);
    ed++;
  }

  ExploreShape(S, DS, FM, EM);
  return DS;
}

//=================================================================================================

void ShapeToHLRConverter::ExploreFace(const Handle(HLRTopoBRep_OutLiner)& S,
                                     const Handle(HLRBRep_Data)&         DS,
                                     const TopTools_IndexedMapOfShape&   FM,
                                     const TopTools_IndexedMapOfShape&   EM,
                                     Standard_Integer&                   i,
                                     const TopoFace&                  F,
                                     const Standard_Boolean              closed)
{
  i++;
  ShapeExplorer    Ex1, Ex2;
  Data1&  TopDS   = S->DataStructure();
  TopAbs_Orientation orient  = FM(i).Orientation();
  TopoFace        theFace = TopoDS::Face(FM(i));
  theFace.Orientation(TopAbs_FORWARD);
  HLRBRep_FaceData& fd = DS->FDataArray().ChangeValue(i);

  Standard_Integer nw = 0;

  for (Ex1.Init(theFace, TopAbs_WIRE); Ex1.More(); Ex1.Next())
    nw++;

  fd.Set(theFace, orient, closed, nw);
  nw = 0;

  for (Ex1.Init(theFace, TopAbs_WIRE); Ex1.More(); Ex1.Next())
  {
    nw++;
    Standard_Integer ne = 0;

    for (Ex2.Init(Ex1.Current(), TopAbs_EDGE); Ex2.More(); Ex2.Next())
    {
      const TopoEdge& anEdge = TopoDS::Edge(Ex2.Current());
      if (!BRepInspector::Degenerated(anEdge))
        ne++;
    }

    fd.SetWire(nw, ne);
    ne = 0;

    for (Ex2.Init(Ex1.Current(), TopAbs_EDGE); Ex2.More(); Ex2.Next())
    {
      const TopoEdge& E = TopoDS::Edge(Ex2.Current());
      if (BRepInspector::Degenerated(E))
        continue;
      ne++;
      Standard_Integer   ie        = EM.FindIndex(E);
      TopAbs_Orientation anOrientE = E.Orientation();
      Standard_Boolean   Int       = TopDS.IsIntLFaceEdge(F, E);
      Standard_Boolean   Iso       = TopDS.IsIsoLFaceEdge(F, E);
      Standard_Boolean   Out       = TopDS.IsOutLFaceEdge(F, E);
      Standard_Boolean   Dbl       = BRepTools1::IsReallyClosed(TopoDS::Edge(E), theFace);
      fd.SetWEdge(nw, ne, ie, anOrientE, Out, Int, Dbl, Iso);
    }
  }
  DS->FaceMap().Add(theFace);
}

//=================================================================================================

void ShapeToHLRConverter::ExploreShape(const Handle(HLRTopoBRep_OutLiner)& S,
                                      const Handle(HLRBRep_Data)&         DS,
                                      const TopTools_IndexedMapOfShape&   FM,
                                      const TopTools_IndexedMapOfShape&   EM)
{
  TopTools_MapOfShape ShapeMap;
  ShapeExplorer     exshell, exface, exedge;
  Standard_Integer    i = 0;

  for (exshell.Init(S->OriginalShape(), TopAbs_SHELL); exshell.More(); exshell.Next())
  { // faces in a shell (open or close)

    Standard_Boolean closed = exshell.Current().Closed();

    if (!closed)
    {
      Standard_Integer  ie;
      Standard_Integer  nbEdge = EM.Extent();
      Standard_Integer* flag   = new Standard_Integer[nbEdge + 1];

      for (ie = 1; ie <= nbEdge; ie++)
        flag[ie] = 0;

      for (exedge.Init(exshell.Current(), TopAbs_EDGE); exedge.More(); exedge.Next())
      {
        const TopoEdge& E      = TopoDS::Edge(exedge.Current());
        ie                        = EM.FindIndex(E);
        TopAbs_Orientation orient = E.Orientation();
        if (!BRepInspector::Degenerated(E))
        {
          if (orient == TopAbs_FORWARD)
            flag[ie] += 1;
          else if (orient == TopAbs_REVERSED)
            flag[ie] -= 1;
        }
      }
      closed = Standard_True;

      for (ie = 1; ie <= nbEdge && closed; ie++)
        closed = (flag[ie] == 0);
      delete[] flag;
      flag = NULL;
    }

    for (exface.Init(exshell.Current(), TopAbs_FACE); exface.More(); exface.Next())
    {
      if (ShapeMap.Add(exface.Current()))
      {
        ExploreFace(S, DS, FM, EM, i, TopoDS::Face(exface.Current()), closed);
      }
    }
  }

  for (exface.Init(S->OriginalShape(), TopAbs_FACE, TopAbs_SHELL); exface.More(); exface.Next())
  { // faces not in a shell
    if (ShapeMap.Add(exface.Current()))
    {
      ExploreFace(S, DS, FM, EM, i, TopoDS::Face(exface.Current()), Standard_False);
    }
  }
}
