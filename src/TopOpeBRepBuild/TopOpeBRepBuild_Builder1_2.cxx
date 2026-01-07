// Created on: 2000-02-01
// Created by: Peter KURNEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepBuild_GTopo.hxx>
#include <TopOpeBRepBuild_HBuilder.hxx>
#include <TopOpeBRepBuild_PaveSet.hxx>
#include <TopOpeBRepBuild_WireEdgeSet.hxx>
#include <TopOpeBRepDS_CurveExplorer.hxx>
#include <TopOpeBRepDS_HDataStructure.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>

//=======================================================================
// function : TopOpeBRepBuild_Builder1::CorrectResult2d
// purpose  : Change the Result (after CorrectFace2d)
//=======================================================================
Standard_Integer TopOpeBRepBuild_Builder1::CorrectResult2d(TopoShape& aResult)

{
  Standard_Integer aNb = myMapOfCorrect2dEdges.Extent();
  if (!aNb)
    return 0;

  if (aResult.ShapeType() != TopAbs_SOLID)
    return 0;

  //
  // 1. Map Of sources' subshapes .
  //    The map (aSourceShapeMap) is to prevent unnecessary coping
  TopTools_IndexedMapOfShape aSourceShapeMap;
  TopExp1::MapShapes(myShape1, TopAbs_EDGE, aSourceShapeMap);
  TopExp1::MapShapes(myShape2, TopAbs_EDGE, aSourceShapeMap);

  TopTools_IndexedDataMapOfShapeShape EdMap;
  ShapeBuilder                        BB;
  TopoShape                        aLocalShape = aResult.EmptyCopied();
  TopoSolid                        aSolid      = TopoDS::Solid(aLocalShape);
  //  TopoSolid aSolid=TopoDS::Solid(aResult.EmptyCopied());

  ShapeExplorer anExpShells(aResult, TopAbs_SHELL);
  for (; anExpShells.More(); anExpShells.Next())
  {
    const TopoShell& S = TopoDS::Shell(anExpShells.Current());
    aLocalShape           = S.EmptyCopied();
    TopoShell aShell   = TopoDS::Shell(aLocalShape);
    //    TopoShell aShell=TopoDS::Shell(S.EmptyCopied());

    ShapeExplorer anExpFaces(S, TopAbs_FACE);
    for (; anExpFaces.More(); anExpFaces.Next())
    {
      TopoFace F = TopoDS::Face(anExpFaces.Current());
      // modified by NIZHNY-MZV  Mon Mar 27 09:51:59 2000
      TopAbs_Orientation Fori = F.Orientation();
      // we should explore FORWARD face
      //      F.Orientation(TopAbs_FORWARD);
      aLocalShape       = F.EmptyCopied();
      TopoFace aFace = TopoDS::Face(aLocalShape);
      //      TopoFace aFace=TopoDS::Face(F.EmptyCopied());

      ShapeExplorer anExpWires(F, TopAbs_WIRE);
      for (; anExpWires.More(); anExpWires.Next())
      {
        TopoWire W = TopoDS::Wire(anExpWires.Current());
        // modified by NIZHNY-MZV  Mon Mar 27 09:51:59 2000
        TopAbs_Orientation Wori = W.Orientation();

        // we should explore FORWARD wire
        //	W.Orientation(TopAbs_FORWARD);
        aLocalShape       = W.EmptyCopied();
        TopoWire aWire = TopoDS::Wire(aLocalShape);
        //	TopoWire aWire = TopoDS::Wire(W.EmptyCopied());

        ShapeExplorer anExpEdges(W, TopAbs_EDGE);
        for (; anExpEdges.More(); anExpEdges.Next())
        {
          TopoEdge E = TopoDS::Edge(anExpEdges.Current());

          if (EdMap.Contains(E))
          {
            TopoShape anEdge = EdMap.ChangeFromKey(E);

            anEdge.Orientation(E.Orientation());
            BB.Add(aWire, anEdge);
            continue;
          }

          if (myMapOfCorrect2dEdges.Contains(E))
          {
            TopoShape anEdge = myMapOfCorrect2dEdges.ChangeFromKey(E);

            anEdge.Orientation(E.Orientation());
            BB.Add(aWire, anEdge);
            EdMap.Add(E, anEdge);
            continue;
          }

          // add edges
          TopoEdge anEdge;
          // we copy edge in order to not change it in source shapes
          if (aSourceShapeMap.Contains(E))
          {
            TopoShape aLocalShape1 = E.EmptyCopied();
            anEdge                    = TopoDS::Edge(aLocalShape1);
            //	    anEdge = TopoDS::Edge(E.EmptyCopied());

            EdMap.Add(E, anEdge);

            ShapeExplorer  anExpVertices(E, TopAbs_VERTEX);
            Standard_Boolean free = anEdge.Free();
            anEdge.Free(Standard_True);
            for (; anExpVertices.More(); anExpVertices.Next())
              BB.Add(anEdge, anExpVertices.Current());

            anEdge.Free(free);
          }
          else
            anEdge = E;

          anEdge.Orientation(E.Orientation());
          BB.Add(aWire, anEdge);
        }
        // Add wires
        aWire.Orientation(Wori);
        aWire.Closed(BRepInspector::IsClosed(aWire));
        BB.Add(aFace, aWire);
      }

      aFace.Orientation(Fori);
      BB.Add(aShell, aFace);
    }

    aShell.Orientation(S.Orientation());
    aShell.Closed(BRepInspector::IsClosed(aShell));
    BB.Add(aSolid, aShell);
  }
  aResult = aSolid;

  // update section curves
  CurveExplorer cex(myDataStructure->DS());
  for (; cex.More(); cex.Next())
  {
    Standard_Integer                   ic  = cex.Index();
    ShapeList&              LSE = ChangeNewEdges(ic);
    ShapeList               corrLSE;
    TopTools_ListIteratorOfListOfShape it(LSE);
    for (; it.More(); it.Next())
    {
      const TopoShape& E = it.Value();
      if (EdMap.Contains(E))
      {
        const TopoShape& newE = EdMap.FindFromKey(E);
        corrLSE.Append(newE);
      }
      else
        corrLSE.Append(E);
    }
    LSE.Clear();
    LSE.Append(corrLSE);
  }

  // update section edges
  const TopOpeBRepDS_DataStructure& BDS = myDataStructure->DS();
  Standard_Integer                  i, nes = BDS.NbSectionEdges();

  for (i = 1; i <= nes; i++)
  {
    const TopoShape& es = BDS.SectionEdge(i);
    if (es.IsNull())
      continue;

    for (Standard_Integer j = 0; j <= 2; j++)
    {
      TopAbs_State                       staspl = TopAbs_State(j); // 0 - IN, 1 - OUT, 2 - ON
      ShapeList&              LSE    = ChangeSplit(es, staspl);
      ShapeList               corrLSE;
      TopTools_ListIteratorOfListOfShape it(LSE);
      for (; it.More(); it.Next())
      {
        const TopoShape& E = it.Value();
        if (EdMap.Contains(E))
        {
          const TopoShape& newE = EdMap.FindFromKey(E);
          corrLSE.Append(newE);
        }
        else
          corrLSE.Append(E);
      }
      LSE.Clear();
      LSE.Append(corrLSE);
    }
  }
  return 1;
}
