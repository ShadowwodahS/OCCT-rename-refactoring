// Created on: 1996-09-03
// Created by: Yves FRICAUD
// Copyright (c) 1996-1999 Matra Datavision
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

//  Modified by skv - Fri Dec 26 12:20:14 2003 OCC4455

#include <Bnd_Tools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgo_AsDes.hxx>
#include <BRepAlgo_Image.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepOffset_Analyse.hxx>
#include <BRepOffset_Inter3d.hxx>
#include <BRepOffset_Interval.hxx>
#include <BRepOffset_ListOfInterval.hxx>
#include <BRepOffset_Offset.hxx>
#include <BRepOffset_Tool.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
//
#include <BRepBndLib.hxx>
#include <BOPTools_BoxTree.hxx>
//
#include <BOPTools_AlgoTools.hxx>

//=================================================================================================

BRepOffset_Inter3d::BRepOffset_Inter3d(const Handle(BRepAlgo_AsDes)& AsDes,
                                       const TopAbs_State            Side,
                                       const Standard_Real           Tol)
    : myAsDes(AsDes),
      mySide(Side),
      myTol(Tol)
{
}

//=================================================================================================

static void ExtentEdge(const TopoFace& /*F*/, const TopoEdge& E, TopoEdge& NE)
{
  TopoShape aLocalShape = E.EmptyCopied();
  NE                       = TopoDS::Edge(aLocalShape);
  //  NE = TopoDS::Edge(E.EmptyCopied());

  // Enough for analytic edges, in general case reconstruct the
  // geometry of the edge recalculating the intersection of surfaces.

  NE.Orientation(TopAbs_FORWARD);
  Standard_Real f, l;
  BRepInspector::Range(E, f, l);
  Standard_Real length = l - f;
  f -= 100 * length;
  l += 100 * length;

  ShapeBuilder B;
  B.Range(NE, f, l);
  BRepAdaptor_Curve CE(E);
  TopoVertex     V1 = BRepLib_MakeVertex(CE.Value(f));
  TopoVertex     V2 = BRepLib_MakeVertex(CE.Value(l));
  B.Add(NE, V1.Oriented(TopAbs_FORWARD));
  B.Add(NE, V2.Oriented(TopAbs_REVERSED));
  NE.Orientation(E.Orientation());
}

//=================================================================================================

void BRepOffset_Inter3d::CompletInt(const ShapeList&  SetOfFaces,
                                    const ShapeImage&        InitOffsetFace,
                                    const Message_ProgressRange& theRange)
{
  //---------------------------------------------------------------
  // Calculate the intersections of offset faces
  // Distinction of intersection between faces // tangents.
  //---------------------------------------------------------------

  // Prepare tools for sorting the bounding boxes
  BOPTools_BoxTree aBBTree;
  aBBTree.SetSize(SetOfFaces.Extent());
  //
  NCollection_IndexedDataMap<TopoShape, Box2, ShapeHasher> aMFaces;
  // Construct bounding boxes for faces and add them to the tree
  TopTools_ListIteratorOfListOfShape aItL(SetOfFaces);
  for (; aItL.More(); aItL.Next())
  {
    const TopoFace& aF = TopoDS::Face(aItL.Value());
    //
    // compute bounding box
    Box2 aBoxF;
    BRepBndLib1::Add(aF, aBoxF);
    //
    Standard_Integer i = aMFaces.Add(aF, aBoxF);
    //
    aBBTree.Add(i, Tools5::Bnd2BVH(aBoxF));
  }

  // Build BVH
  aBBTree.Build();

  // Perform selection of the pairs
  BOPTools_BoxPairSelector aSelector;
  aSelector.SetBVHSets(&aBBTree, &aBBTree);
  aSelector.SetSame(Standard_True);
  aSelector.Select();
  aSelector.Sort();

  // Treat the selected pairs
  const std::vector<BOPTools_BoxPairSelector::PairIDs>& aPairs = aSelector.Pairs();
  const Standard_Integer aNbPairs = static_cast<Standard_Integer>(aPairs.size());
  Message_ProgressScope  aPS(theRange, "Complete intersection", aNbPairs);
  for (Standard_Integer iPair = 0; iPair < aNbPairs; ++iPair, aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    const BOPTools_BoxPairSelector::PairIDs& aPair = aPairs[iPair];

    const TopoFace& aF1 = TopoDS::Face(aMFaces.FindKey(Min(aPair.ID1, aPair.ID2)));
    const TopoFace& aF2 = TopoDS::Face(aMFaces.FindKey(Max(aPair.ID1, aPair.ID2)));

    // intersect faces
    FaceInter(aF1, aF2, InitOffsetFace);
  }
}

//=======================================================================
// function : FaceInter
// purpose  : Performs intersection of the given faces
//=======================================================================

void BRepOffset_Inter3d::FaceInter(const TopoFace&    F1,
                                   const TopoFace&    F2,
                                   const ShapeImage& InitOffsetFace)
{
  ShapeList LInt1, LInt2;
  TopoEdge          NullEdge;
  TopoFace          NullFace;

  if (F1.IsSame(F2))
    return;
  if (IsDone(F1, F2))
    return;

  const TopoShape& InitF1 = InitOffsetFace.ImageFrom(F1);
  const TopoShape& InitF2 = InitOffsetFace.ImageFrom(F2);
  if (InitF1.IsSame(InitF2))
    return;

  Standard_Boolean InterPipes =
    (InitF2.ShapeType() == TopAbs_EDGE && InitF1.ShapeType() == TopAbs_EDGE);
  Standard_Boolean InterFaces =
    (InitF1.ShapeType() == TopAbs_FACE && InitF2.ShapeType() == TopAbs_FACE);
  ShapeList LE, LV;
  LInt1.Clear();
  LInt2.Clear();
  if (Tool5::FindCommonShapes(F1, F2, LE, LV) || myAsDes->HasCommonDescendant(F1, F2, LE))
  {
    //-------------------------------------------------
    // F1 and F2 share shapes.
    //-------------------------------------------------
    if (LE.IsEmpty() && !LV.IsEmpty())
    {
      if (InterPipes)
      {
        //----------------------
        // tubes share a vertex.
        //----------------------
        const TopoEdge& EE1 = TopoDS::Edge(InitF1);
        const TopoEdge& EE2 = TopoDS::Edge(InitF2);
        TopoVertex      VE1[2], VE2[2];
        TopExp1::Vertices(EE1, VE1[0], VE1[1]);
        TopExp1::Vertices(EE2, VE2[0], VE2[1]);
        TopoVertex V;
        for (Standard_Integer i = 0; i < 2; i++)
        {
          for (Standard_Integer j = 0; j < 2; j++)
          {
            if (VE1[i].IsSame(VE2[j]))
            {
              V = VE1[i];
            }
          }
        }
        if (!InitOffsetFace.HasImage(V))
        { // no sphere
          Tool5::PipeInter(F1, F2, LInt1, LInt2, mySide);
        }
      }
      else
      {
        //--------------------------------------------------------
        // Intersection having only common vertices
        // and supports having common edges.
        // UNSUFFICIENT, but a larger criterion shakes too
        // many sections.
        //--------------------------------------------------------
        if (InterFaces)
        {
          if (Tool5::FindCommonShapes(TopoDS::Face(InitF1), TopoDS::Face(InitF2), LE, LV))
          {
            if (!LE.IsEmpty())
            {
              Tool5::Inter3D(F1, F2, LInt1, LInt2, mySide, NullEdge, NullFace, NullFace);
            }
          }
          else
          {
            Tool5::Inter3D(F1, F2, LInt1, LInt2, mySide, NullEdge, NullFace, NullFace);
          }
        }
      }
    }
  }
  else
  {
    if (InterPipes)
    {
      Tool5::PipeInter(F1, F2, LInt1, LInt2, mySide);
    }
    else
    {
      Tool5::Inter3D(F1, F2, LInt1, LInt2, mySide, NullEdge, NullFace, NullFace);
    }
  }
  Store(F1, F2, LInt1, LInt2);
}

//=================================================================================================

void BRepOffset_Inter3d::ConnexIntByArc(const ShapeList& /*SetOfFaces*/,
                                        const TopoShape&          ShapeInit,
                                        const BRepOffset_Analyse&    Analyse,
                                        const ShapeImage&        InitOffsetFace,
                                        const Message_ProgressRange& theRange)
{
  ChFiDS_TypeOfConcavity OT = ChFiDS_Concave;
  if (mySide == TopAbs_OUT)
    OT = ChFiDS_Convex;
  ShapeExplorer       Exp(ShapeInit, TopAbs_EDGE);
  ShapeList  LInt1, LInt2;
  TopoFace           F1, F2;
  TopoEdge           NullEdge;
  TopoFace           NullFace;
  Message_ProgressScope aPSOuter(theRange, NULL, 2);
  Message_ProgressScope aPSIntF(aPSOuter.Next(), "Intersecting offset faces", 1, Standard_True);
  //---------------------------------------------------------------------
  // etape 1 : Intersection of faces // corresponding to the initial faces
  //           separated by a concave edge if offset > 0, otherwise convex.
  //---------------------------------------------------------------------
  for (; Exp.More(); Exp.Next(), aPSIntF.Next())
  {
    if (!aPSIntF.More())
    {
      return;
    }
    const TopoEdge&               E = TopoDS::Edge(Exp.Current());
    const BRepOffset_ListOfInterval& L = Analyse.Type(E);
    if (!L.IsEmpty() && L.First().Type() == OT)
    {
      //-----------------------------------------------------------
      // edge is of the proper type , return adjacent faces.
      //-----------------------------------------------------------
      const ShapeList& Anc = Analyse.Ancestors(E);
      if (Anc.Extent() == 2)
      {

        const TopoFace& InitF1 = TopoDS::Face(Anc.First());
        const TopoFace& InitF2 = TopoDS::Face(Anc.Last());
        F1                        = TopoDS::Face(InitOffsetFace.Image(InitF1).First());
        F2                        = TopoDS::Face(InitOffsetFace.Image(InitF2).First());
        if (!IsDone(F1, F2))
        {
          Tool5::Inter3D(F1, F2, LInt1, LInt2, mySide, E, InitF1, InitF2);
          Store(F1, F2, LInt1, LInt2);
        }
      }
    }
  }
  //---------------------------------------------------------------------
  // etape 2 : Intersections of tubes sharing a vertex without sphere with:
  //           - tubes on each other edge sharing the vertex
  //           - faces containing an edge connected to vertex that has no tubes.
  //---------------------------------------------------------------------
  TopoVertex                      V[2];
  TopTools_ListIteratorOfListOfShape it;
  Message_ProgressScope aPSIntT(aPSOuter.Next(), "Intersecting tubes", 1, Standard_True);
  for (Exp.Init(ShapeInit, TopAbs_EDGE); Exp.More(); Exp.Next(), aPSIntT.Next())
  {
    if (!aPSIntT.More())
    {
      return;
    }
    const TopoEdge& E1 = TopoDS::Edge(Exp.Current());
    if (InitOffsetFace.HasImage(E1))
    {
      //---------------------------
      // E1 generated a tube.
      //---------------------------
      F1 = TopoDS::Face(InitOffsetFace.Image(E1).First());
      TopExp1::Vertices(E1, V[0], V[1]);
      const ShapeList& AncE1 = Analyse.Ancestors(E1);

      for (Standard_Integer i = 0; i < 2; i++)
      {
        if (!InitOffsetFace.HasImage(V[i]))
        {
          //-----------------------------
          // the vertex has no sphere.
          //-----------------------------
          const ShapeList& Anc = Analyse.Ancestors(V[i]);
          ShapeList        TangOnV;
          Analyse.TangentEdges(E1, V[i], TangOnV);
          TopTools_MapOfShape MTEV;
          for (it.Initialize(TangOnV); it.More(); it.Next())
          {
            MTEV.Add(it.Value());
          }
          for (it.Initialize(Anc); it.More(); it.Next())
          {
            const TopoEdge& E2 = TopoDS::Edge(it.Value());
            //  Modified by skv - Fri Jan 16 16:27:54 2004 OCC4455 Begin
            //            if (E1.IsSame(E2) || MTEV.Contains(E2)) continue;
            Standard_Boolean isToSkip = Standard_False;

            if (!E1.IsSame(E2))
            {
              const BRepOffset_ListOfInterval& aL = Analyse.Type(E2);

              isToSkip =
                (MTEV.Contains(E2) && (aL.IsEmpty() || (!aL.IsEmpty() && aL.First().Type() != OT)));
            }

            if (E1.IsSame(E2) || isToSkip)
              continue;
            //  Modified by skv - Fri Jan 16 16:27:54 2004 OCC4455 End
            if (InitOffsetFace.HasImage(E2))
            {
              //-----------------------------
              // E2 generated a tube.
              //-----------------------------
              F2 = TopoDS::Face(InitOffsetFace.Image(E2).First());
              if (!IsDone(F1, F2))
              {
                //---------------------------------------------------------------------
                // Intersection tube/tube if the edges are not tangent (AFINIR).
                //----------------------------------------------------------------------
                Tool5::PipeInter(F1, F2, LInt1, LInt2, mySide);
                Store(F1, F2, LInt1, LInt2);
              }
            }
            else
            {
              //-------------------------------------------------------
              // Intersection of the tube of E1 with faces //
              // to face containing E2 if they are not tangent
              // to the tube or if E2 is not a tangent edge.
              //-------------------------------------------------------
              const BRepOffset_ListOfInterval& L = Analyse.Type(E2);
              if (!L.IsEmpty() && L.First().Type() == ChFiDS_Tangential)
              {
                continue;
              }
              const ShapeList& AncE2        = Analyse.Ancestors(E2);
              Standard_Boolean            TangentFaces = Standard_False;
              if (AncE2.Extent() == 2)
              {
                TopoFace InitF2 = TopoDS::Face(AncE2.First());
                TangentFaces       = (InitF2.IsSame(AncE1.First()) || InitF2.IsSame(AncE1.Last()));
                if (!TangentFaces)
                {
                  F2 = TopoDS::Face(InitOffsetFace.Image(InitF2).First());
                  if (!IsDone(F1, F2))
                  {
                    Tool5::Inter3D(F1,
                                             F2,
                                             LInt1,
                                             LInt2,
                                             mySide,
                                             NullEdge,
                                             NullFace,
                                             NullFace);
                    Store(F1, F2, LInt1, LInt2);
                  }
                }
                InitF2       = TopoDS::Face(AncE2.Last());
                TangentFaces = (InitF2.IsSame(AncE1.First()) || InitF2.IsSame(AncE1.Last()));
                if (!TangentFaces)
                {
                  F2 = TopoDS::Face(InitOffsetFace.Image(InitF2).First());
                  if (!IsDone(F1, F2))
                  {
                    Tool5::Inter3D(F1,
                                             F2,
                                             LInt1,
                                             LInt2,
                                             mySide,
                                             NullEdge,
                                             NullFace,
                                             NullFace);
                    Store(F1, F2, LInt1, LInt2);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

//=================================================================================================

void BRepOffset_Inter3d::ConnexIntByInt(const TopoShape&                    SI,
                                        const BRepOffset_DataMapOfShapeOffset& MapSF,
                                        const BRepOffset_Analyse&              Analyse,
                                        TopTools_DataMapOfShapeShape&          MES,
                                        TopTools_DataMapOfShapeShape&          Build,
                                        ShapeList&                  Failed,
                                        const Message_ProgressRange&           theRange,
                                        const Standard_Boolean                 bIsPlanar)
{
  TopTools_IndexedMapOfShape         VEmap;
  TopoFace                        F1, F2, OF1, OF2, NF1, NF2;
  TopAbs_State                       CurSide = mySide;
  ShapeBuilder                       B;
  Standard_Boolean                   bEdge;
  Standard_Integer                   i, aNb = 0;
  TopTools_ListIteratorOfListOfShape it, it1, itF1, itF2;
  //
  TopExp1::MapShapes(SI, TopAbs_EDGE, VEmap);
  // Take the vertices for treatment
  Message_ProgressScope aPSOuter(theRange, NULL, 10);
  if (bIsPlanar)
  {
    aNb = VEmap.Extent();
    for (i = 1; i <= aNb; ++i)
    {
      const TopoEdge& aE    = TopoDS::Edge(VEmap(i));
      TopoShape       aFGen = Analyse.Generated(aE);
      if (!aFGen.IsNull())
        TopExp1::MapShapes(aFGen, TopAbs_EDGE, VEmap);
    }

    // Add vertices for treatment
    TopExp1::MapShapes(SI, TopAbs_VERTEX, VEmap);

    for (ShapeList::Iterator itNF(Analyse.NewFaces()); itNF.More(); itNF.Next())
      TopExp1::MapShapes(itNF.Value(), TopAbs_VERTEX, VEmap);
  }
  //
  TopTools_DataMapOfShapeListOfShape        aDMVLF1, aDMVLF2, aDMIntFF;
  TopTools_IndexedDataMapOfShapeListOfShape aDMIntE;
  //
  if (bIsPlanar)
  {
    // Find internal edges in the faces to skip them while preparing faces
    // for intersection through vertices
    NCollection_DataMap<TopoShape, TopTools_MapOfShape, ShapeHasher> aDMFEI;
    {
      for (ShapeExplorer expF(SI, TopAbs_FACE); expF.More(); expF.Next())
      {
        const TopoShape& aFx = expF.Current();

        TopTools_MapOfShape aMEI;
        for (ShapeExplorer expE(aFx, TopAbs_EDGE); expE.More(); expE.Next())
        {
          const TopoShape& aEx = expE.Current();
          if (aEx.Orientation() != TopAbs_FORWARD && aEx.Orientation() != TopAbs_REVERSED)
            aMEI.Add(aEx);
        }
        if (!aMEI.IsEmpty())
          aDMFEI.Bind(aFx, aMEI);
      }
    }

    // Analyze faces connected through vertices
    for (i = aNb + 1, aNb = VEmap.Extent(); i <= aNb; ++i)
    {
      if (!aPSOuter.More())
      {
        return;
      }
      const TopoShape& aS = VEmap(i);
      if (aS.ShapeType() != TopAbs_VERTEX)
        continue;

      // Find faces connected to the vertex
      ShapeList aLF;
      {
        const ShapeList& aLE = Analyse.Ancestors(aS);
        for (ShapeList::Iterator itLE(aLE); itLE.More(); itLE.Next())
        {
          const ShapeList& aLEA = Analyse.Ancestors(itLE.Value());
          for (ShapeList::Iterator itLEA(aLEA); itLEA.More(); itLEA.Next())
          {
            if (!aLF.Contains(itLEA.Value()))
              aLF.Append(itLEA.Value());
          }
        }
      }

      if (aLF.Extent() < 2)
        continue;

      // build lists of faces connected to the same vertex by looking for
      // the pairs in which the vertex is alone (not connected to shared edges)
      ShapeList aLF1, aLF2;

      it.Initialize(aLF);
      for (; it.More(); it.Next())
      {
        const TopoShape& aFV1 = it.Value();

        // get edges of first face connected to current vertex
        TopTools_MapOfShape         aME;
        const TopTools_MapOfShape*  pF1Internal = aDMFEI.Seek(aFV1);
        const ShapeList* pLE1        = Analyse.Descendants(aFV1);
        if (!pLE1)
          continue;
        ShapeList::Iterator itLE1(*pLE1);
        for (; itLE1.More(); itLE1.Next())
        {
          const TopoShape& aE = itLE1.Value();
          if (pF1Internal && pF1Internal->Contains(aE))
            break;

          for (TopoDS_Iterator aItV(aE); aItV.More(); aItV.Next())
          {
            if (aS.IsSame(aItV.Value()))
            {
              aME.Add(aE);
              break;
            }
          }
        }
        if (itLE1.More())
          continue;

        // get to the next face in the list
        it1 = it;
        for (it1.Next(); it1.More(); it1.Next())
        {
          const TopoFace& aFV2 = TopoDS::Face(it1.Value());

          const TopTools_MapOfShape* pF2Internal = aDMFEI.Seek(aFV2);

          const ShapeList* pLE2 = Analyse.Descendants(aFV2);
          if (!pLE2)
            continue;
          ShapeList::Iterator itLE2(*pLE2);
          for (; itLE2.More(); itLE2.Next())
          {
            const TopoShape& aEV2 = itLE2.Value();
            if (!aME.Contains(aEV2))
              continue;

            if (pF2Internal && pF2Internal->Contains(aEV2))
              // Avoid intersection of faces connected by internal edge
              break;

            if (Analyse.HasAncestor(aEV2) && Analyse.Ancestors(aEV2).Extent() == 2)
              // Faces will be intersected through the edge
              break;
          }

          if (!itLE2.More())
          {
            aLF1.Append(aFV1);
            aLF2.Append(aFV2);
          }
        }
      }
      //
      if (aLF1.Extent())
      {
        aDMVLF1.Bind(aS, aLF1);
        aDMVLF2.Bind(aS, aLF2);
      }
    }
  }
  //
  aNb = VEmap.Extent();
  Message_ProgressScope aPSInter(aPSOuter.Next(8), "Intersecting offset faces", aNb);
  for (i = 1; i <= aNb; ++i, aPSInter.Next())
  {
    if (!aPSInter.More())
    {
      return;
    }
    const TopoShape& aS = VEmap(i);
    //
    TopoEdge          E;
    ShapeList aLF1, aLF2;
    //
    bEdge = (aS.ShapeType() == TopAbs_EDGE);
    if (bEdge)
    {
      // faces connected by the edge
      E = *(TopoEdge*)&aS;
      //
      const BRepOffset_ListOfInterval& L = Analyse.Type(E);
      if (L.IsEmpty())
      {
        continue;
      }
      //
      ChFiDS_TypeOfConcavity OT = L.First().Type();
      if (OT != ChFiDS_Convex && OT != ChFiDS_Concave)
      {
        continue;
      }
      //
      if (OT == ChFiDS_Concave)
        CurSide = TopAbs_IN;
      else
        CurSide = TopAbs_OUT;
      //-----------------------------------------------------------
      // edge is of the proper type, return adjacent faces.
      //-----------------------------------------------------------
      const ShapeList& Anc = Analyse.Ancestors(E);
      if (Anc.Extent() != 2)
      {
        continue;
      }
      //
      F1 = TopoDS::Face(Anc.First());
      F2 = TopoDS::Face(Anc.Last());
      //
      aLF1.Append(F1);
      aLF2.Append(F2);
    }
    else
    {
      if (!aDMVLF1.IsBound(aS))
      {
        continue;
      }
      //
      aLF1 = aDMVLF1.Find(aS);
      aLF2 = aDMVLF2.Find(aS);
      //
      CurSide = mySide;
    }
    //
    itF1.Initialize(aLF1);
    itF2.Initialize(aLF2);
    for (; itF1.More() && itF2.More(); itF1.Next(), itF2.Next())
    {
      F1 = TopoDS::Face(itF1.Value());
      F2 = TopoDS::Face(itF2.Value());
      //
      OF1 = TopoDS::Face(MapSF(F1).Face());
      OF2 = TopoDS::Face(MapSF(F2).Face());
      if (!MES.IsBound(OF1))
      {
        Standard_Boolean enlargeU      = Standard_True;
        Standard_Boolean enlargeVfirst = Standard_True, enlargeVlast = Standard_True;
        Tool5::CheckBounds(F1, Analyse, enlargeU, enlargeVfirst, enlargeVlast);
        Tool5::EnLargeFace(OF1,
                                     NF1,
                                     Standard_True,
                                     Standard_True,
                                     enlargeU,
                                     enlargeVfirst,
                                     enlargeVlast);
        MES.Bind(OF1, NF1);
      }
      else
      {
        NF1 = TopoDS::Face(MES(OF1));
      }
      //
      if (!MES.IsBound(OF2))
      {
        Standard_Boolean enlargeU      = Standard_True;
        Standard_Boolean enlargeVfirst = Standard_True, enlargeVlast = Standard_True;
        Tool5::CheckBounds(F2, Analyse, enlargeU, enlargeVfirst, enlargeVlast);
        Tool5::EnLargeFace(OF2,
                                     NF2,
                                     Standard_True,
                                     Standard_True,
                                     enlargeU,
                                     enlargeVfirst,
                                     enlargeVlast);
        MES.Bind(OF2, NF2);
      }
      else
      {
        NF2 = TopoDS::Face(MES(OF2));
      }
      //
      if (!IsDone(NF1, NF2))
      {
        ShapeList LInt1, LInt2;
        Tool5::Inter3D(NF1, NF2, LInt1, LInt2, CurSide, E, F1, F2);
        SetDone(NF1, NF2);
        if (!LInt1.IsEmpty())
        {
          Store(NF1, NF2, LInt1, LInt2);
          //
          TopoCompound C;
          B.MakeCompound(C);
          //
          if (Build.IsBound(aS))
          {
            const TopoShape& aSE = Build(aS);
            ShapeExplorer     aExp(aSE, TopAbs_EDGE);
            for (; aExp.More(); aExp.Next())
            {
              const TopoShape& aNE = aExp.Current();
              B.Add(C, aNE);
            }
          }
          //
          it.Initialize(LInt1);
          for (; it.More(); it.Next())
          {
            const TopoShape& aNE = it.Value();
            B.Add(C, aNE);
            //
            // keep connection from new edge to shape from which it was created
            ShapeList* pLS = &aDMIntE(aDMIntE.Add(aNE, ShapeList()));
            pLS->Append(aS);
            // keep connection to faces created the edge as well
            ShapeList* pLFF = aDMIntFF.Bound(aNE, ShapeList());
            pLFF->Append(F1);
            pLFF->Append(F2);
          }
          //
          Build.Bind(aS, C);
        }
        else
        {
          Failed.Append(aS);
        }
      }
      else
      { // IsDone(NF1,NF2)
        //  Modified by skv - Fri Dec 26 12:20:13 2003 OCC4455 Begin
        const ShapeList& aLInt1 = myAsDes->Descendant(NF1);
        const ShapeList& aLInt2 = myAsDes->Descendant(NF2);

        if (!aLInt1.IsEmpty())
        {
          TopoCompound C;
          B.MakeCompound(C);
          //
          if (Build.IsBound(aS))
          {
            const TopoShape& aSE = Build(aS);
            ShapeExplorer     aExp(aSE, TopAbs_EDGE);
            for (; aExp.More(); aExp.Next())
            {
              const TopoShape& aNE = aExp.Current();
              B.Add(C, aNE);
            }
          }
          //
          for (it.Initialize(aLInt1); it.More(); it.Next())
          {
            const TopoShape& anE1 = it.Value();
            //
            for (it1.Initialize(aLInt2); it1.More(); it1.Next())
            {
              const TopoShape& anE2 = it1.Value();
              if (anE1.IsSame(anE2))
              {
                B.Add(C, anE1);
                //
                ShapeList* pLS = aDMIntE.ChangeSeek(anE1);
                if (pLS)
                {
                  pLS->Append(aS);
                }
              }
            }
          }
          Build.Bind(aS, C);
        }
        else
        {
          Failed.Append(aS);
        }
      }
    }
    //  Modified by skv - Fri Dec 26 12:20:14 2003 OCC4455 End
  }
  //
  // create unique intersection for each localized shared part
  aNb = aDMIntE.Extent();
  Message_ProgressScope aPSPostTreat(aPSOuter.Next(2), "Creating unique intersection", aNb);
  for (i = 1; i <= aNb; ++i, aPSPostTreat.Next())
  {
    if (!aPSPostTreat.More())
    {
      return;
    }
    const ShapeList& aLS = aDMIntE(i);
    if (aLS.Extent() < 2)
    {
      continue;
    }
    //
    // intersection edge
    const TopoEdge& aE = TopoDS::Edge(aDMIntE.FindKey(i));
    // faces created the edge
    const ShapeList& aLFF = aDMIntFF.Find(aE);
    const TopoShape&         aF1  = aLFF.First();
    const TopoShape&         aF2  = aLFF.Last();

    // Build really localized blocks from the original shapes in <aLS>:
    // 1. Find edges from original faces connected to two or more shapes in <aLS>;
    // 2. Make connexity blocks from edges in <aLS> and found connection edges;
    // 3. Check if the vertices from <aLS> are not connected by these connection edges:
    //    a. If so - add these vertices to Connexity Block1 containing the corresponding
    //       connexity edge;
    //    b. If not - add this vertex to list of connexity blocks
    // 4. Create unique intersection edge for each connexity block

    // list of vertices
    ShapeList aLV;
    // compound of edges to build connexity blocks
    TopoCompound aCE;
    B.MakeCompound(aCE);
    TopTools_MapOfShape                aMS;
    TopTools_ListIteratorOfListOfShape aItLS(aLS);
    for (; aItLS.More(); aItLS.Next())
    {
      const TopoShape& aS = aItLS.Value();
      aMS.Add(aS);
      if (aS.ShapeType() == TopAbs_EDGE)
      {
        B.Add(aCE, aS);
      }
      else
      {
        aLV.Append(aS);
      }
    }
    //
    // look for additional edges to connect the shared parts
    TopTools_MapOfShape aMEConnection;
    for (Standard_Integer j = 0; j < 2; ++j)
    {
      const TopoShape& aF = !j ? aF1 : aF2;
      //
      ShapeExplorer aExp(aF, TopAbs_EDGE);
      for (; aExp.More(); aExp.Next())
      {
        const TopoShape& aEF = aExp.Current();
        if (aMS.Contains(aEF) || aMEConnection.Contains(aEF))
        {
          continue;
        }
        //
        TopoVertex aV1, aV2;
        TopExp1::Vertices(TopoDS::Edge(aEF), aV1, aV2);
        //
        // find parts to which the edge is connected
        Standard_Integer iCounter = 0;
        aItLS.Initialize(aLS);
        for (; aItLS.More(); aItLS.Next())
        {
          const TopoShape& aS = aItLS.Value();
          // iterator is not suitable here, because aS may be a vertex
          ShapeExplorer aExpV(aS, TopAbs_VERTEX);
          for (; aExpV.More(); aExpV.Next())
          {
            const TopoShape& aV = aExpV.Current();
            if (aV.IsSame(aV1) || aV.IsSame(aV2))
            {
              ++iCounter;
              break;
            }
          }
        }
        //
        if (iCounter >= 2)
        {
          B.Add(aCE, aEF);
          aMEConnection.Add(aEF);
        }
      }
    }
    //
    ShapeList aLCBE;
    AlgoTools::MakeConnexityBlocks(aCE, TopAbs_VERTEX, TopAbs_EDGE, aLCBE);
    //
    // create connexity blocks for alone vertices
    ShapeList               aLCBV;
    TopTools_ListIteratorOfListOfShape aItLV(aLV);
    for (; aItLV.More(); aItLV.Next())
    {
      const TopoShape& aV = aItLV.Value();
      // check if this vertex is contained in some connexity block of edges
      TopTools_ListIteratorOfListOfShape aItLCB(aLCBE);
      for (; aItLCB.More(); aItLCB.Next())
      {
        TopoShape&   aCB = aItLCB.ChangeValue();
        ShapeExplorer aExpV(aCB, TopAbs_VERTEX);
        for (; aExpV.More(); aExpV.Next())
        {
          if (aV.IsSame(aExpV.Current()))
          {
            B.Add(aCB, aV);
            break;
          }
        }
        if (aExpV.More())
        {
          break;
        }
      }
      //
      if (!aItLCB.More())
      {
        TopoCompound aCV;
        B.MakeCompound(aCV);
        B.Add(aCV, aV);
        aLCBV.Append(aCV);
      }
    }
    //
    aLCBE.Append(aLCBV);
    //
    if (aLCBE.Extent() == 1)
    {
      continue;
    }
    //
    const TopoShape& aNF1 = MES(MapSF(aF1).Face());
    const TopoShape& aNF2 = MES(MapSF(aF2).Face());
    //
    TopTools_ListIteratorOfListOfShape aItLCB(aLCBE);
    for (aItLCB.Next(); aItLCB.More(); aItLCB.Next())
    {
      // make new edge with different tedge instance
      TopoEdge   aNewEdge;
      TopoVertex aV1, aV2;
      Standard_Real aT1, aT2;
      //
      TopExp1::Vertices(aE, aV1, aV2);
      BRepInspector::Range(aE, aT1, aT2);
      //
      AlgoTools::MakeSplitEdge(aE, aV1, aT1, aV2, aT2, aNewEdge);
      //
      myAsDes->Add(aNF1, aNewEdge);
      myAsDes->Add(aNF2, aNewEdge);
      //
      const TopoShape& aCB = aItLCB.Value();
      TopoDS_Iterator     aItCB(aCB);
      for (; aItCB.More(); aItCB.Next())
      {
        const TopoShape& aS = aItCB.Value();
        if (aMEConnection.Contains(aS))
        {
          continue;
        }
        TopoShape& aCI = Build.ChangeFind(aS);
        //
        TopoCompound aNewCI;
        B.MakeCompound(aNewCI);
        ShapeExplorer aExp(aCI, TopAbs_EDGE);
        for (; aExp.More(); aExp.Next())
        {
          const TopoShape& aSx = aExp.Current();
          if (!aSx.IsSame(aE))
          {
            B.Add(aNewCI, aSx);
          }
        }
        B.Add(aNewCI, aNewEdge);
        aCI = aNewCI;
      }
    }
  }
}

//=================================================================================================

void BRepOffset_Inter3d::ContextIntByInt(const TopTools_IndexedMapOfShape&      ContextFaces,
                                         const Standard_Boolean                 ExtentContext,
                                         const BRepOffset_DataMapOfShapeOffset& MapSF,
                                         const BRepOffset_Analyse&              Analyse,
                                         TopTools_DataMapOfShapeShape&          MES,
                                         TopTools_DataMapOfShapeShape&          Build,
                                         ShapeList&                  Failed,
                                         const Message_ProgressRange&           theRange,
                                         const Standard_Boolean                 bIsPlanar)
{
  TopTools_MapOfShape                MV;
  ShapeExplorer                    exp;
  TopoFace                        OF, NF, WCF;
  TopoEdge                        OE;
  TopoCompound                    C;
  ShapeBuilder                       B;
  TopTools_ListIteratorOfListOfShape it, itF;
  Standard_Integer                   i, j, aNb, aNbVE;
  Standard_Boolean                   bEdge;

  aNb = ContextFaces.Extent();
  for (i = 1; i <= aNb; i++)
  {
    const TopoFace& CF = TopoDS::Face(ContextFaces(i));
    myTouched.Add(CF);
    if (ExtentContext)
    {
      Tool5::EnLargeFace(CF, NF, 0, 0);
      MES.Bind(CF, NF);
    }
  }
  TopAbs_State Side = TopAbs_OUT;

  Message_ProgressScope aPS(theRange, "Intersecting with deepening faces", aNb);
  for (i = 1; i <= aNb; i++, aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    const TopoFace& CF = TopoDS::Face(ContextFaces(i));
    if (ExtentContext)
      WCF = TopoDS::Face(MES(CF));
    else
      WCF = CF;

    TopTools_IndexedMapOfShape VEmap;
    TopExp1::MapShapes(CF.Oriented(TopAbs_FORWARD), TopAbs_EDGE, VEmap);
    //
    if (bIsPlanar)
    {
      TopExp1::MapShapes(CF.Oriented(TopAbs_FORWARD), TopAbs_VERTEX, VEmap);
    }
    //
    aNbVE = VEmap.Extent();
    for (j = 1; j <= aNbVE; ++j)
    {
      const TopoShape& aS = VEmap(j);
      //
      bEdge = (aS.ShapeType() == TopAbs_EDGE);
      //
      TopoEdge          E;
      ShapeList Anc;
      //
      if (bEdge)
      {
        // faces connected by the edge
        //
        E = *(TopoEdge*)&aS;
        if (!Analyse.HasAncestor(E))
        {
          //----------------------------------------------------------------
          // the edges of faces of context that are not in the initial shape
          // can appear in the result.
          //----------------------------------------------------------------
          if (!ExtentContext)
          {
            myAsDes->Add(CF, E);
            myNewEdges.Add(E);
          }
          else
          {
            if (!MES.IsBound(E))
            {
              TopoEdge   NE;
              Standard_Real f, l, Tol;
              BRepInspector::Range(E, f, l);
              Tol = BRepInspector::Tolerance(E);
              ExtentEdge(CF, E, NE);
              TopoVertex V1, V2;
              TopExp1::Vertices(E, V1, V2);
              NE.Orientation(TopAbs_FORWARD);
              myAsDes->Add(NE, V1.Oriented(TopAbs_REVERSED));
              myAsDes->Add(NE, V2.Oriented(TopAbs_FORWARD));
              TopoShape aLocalShape = V1.Oriented(TopAbs_INTERNAL);
              B.UpdateVertex(TopoDS::Vertex(aLocalShape), f, NE, Tol);
              aLocalShape = V2.Oriented(TopAbs_INTERNAL);
              B.UpdateVertex(TopoDS::Vertex(aLocalShape), l, NE, Tol);
              //            B.UpdateVertex(TopoDS::Vertex(V1.Oriented(TopAbs_INTERNAL)),f,NE,Tol);
              //            B.UpdateVertex(TopoDS::Vertex(V2.Oriented(TopAbs_INTERNAL)),l,NE,Tol);
              NE.Orientation(E.Orientation());
              myAsDes->Add(CF, NE);
              myNewEdges.Add(NE);
              MES.Bind(E, NE);
            }
            else
            {
              TopoShape NE          = MES(E);
              TopoShape aLocalShape = NE.Oriented(E.Orientation());
              myAsDes->Add(CF, aLocalShape);
              //            myAsDes->Add(CF,NE.Oriented(E.Orientation()));
            }
          }
          continue;
        }
        Anc = Analyse.Ancestors(E);
      }
      else
      {
        // faces connected by the vertex
        //
        if (!Analyse.HasAncestor(aS))
        {
          continue;
        }
        //
        const ShapeList& aLE = Analyse.Ancestors(aS);
        it.Initialize(aLE);
        for (; it.More(); it.Next())
        {
          const TopoEdge& aE = *(TopoEdge*)&it.Value();
          //
          if (BRepInspector::Degenerated(aE))
          {
            continue;
          }
          //
          if (VEmap.Contains(aE))
          {
            continue;
          }
          //
          const ShapeList& aLF = Analyse.Ancestors(aE);
          itF.Initialize(aLF);
          for (; itF.More(); itF.Next())
          {
            const TopoShape& aF   = itF.Value();
            Standard_Boolean    bAdd = Standard_True;
            exp.Init(aF, TopAbs_EDGE);
            for (; exp.More() && bAdd; exp.Next())
            {
              const TopoShape& aEF = exp.Current();
              bAdd                    = !VEmap.Contains(aEF);
            }
            if (bAdd)
            {
              Anc.Append(aF);
            }
          }
        }
      }
      //
      itF.Initialize(Anc);
      for (; itF.More(); itF.Next())
      {
        const TopoFace& F     = TopoDS::Face(itF.Value());
        OF                       = TopoDS::Face(MapSF(F).Face());
        TopoShape aLocalShape = MapSF(F).Generated(E);
        OE                       = TopoDS::Edge(aLocalShape);
        //      OE = TopoDS::Edge(MapSF(F).Generated(E));
        if (!MES.IsBound(OF))
        {
          Tool5::EnLargeFace(OF, NF, 1, 1);
          MES.Bind(OF, NF);
        }
        else
        {
          NF = TopoDS::Face(MES(OF));
        }
        if (!IsDone(NF, CF))
        {
          ShapeList LInt1, LInt2;
          ShapeList LOE;
          LOE.Append(OE);
          Tool5::Inter3D(WCF, NF, LInt1, LInt2, Side, E, CF, F);
          SetDone(NF, CF);
          if (!LInt1.IsEmpty())
          {
            Store(CF, NF, LInt1, LInt2);
            if ((LInt1.Extent() == 1) && !Build.IsBound(aS))
            {
              Build.Bind(aS, LInt1.First());
            }
            else
            {
              B.MakeCompound(C);
              if (Build.IsBound(aS))
              {
                const TopoShape& aSE = Build(aS);
                exp.Init(aSE, TopAbs_EDGE);
                for (; exp.More(); exp.Next())
                {
                  const TopoShape& aNE = exp.Current();
                  B.Add(C, aNE);
                }
              }
              //
              for (it.Initialize(LInt1); it.More(); it.Next())
              {
                B.Add(C, it.Value());
              }
              Build.Bind(aS, C);
            }
          }
          else
          {
            Failed.Append(aS);
          }
        }
      }
    }
  }
}

//=================================================================================================

void BRepOffset_Inter3d::ContextIntByArc(const TopTools_IndexedMapOfShape& ContextFaces,
                                         const Standard_Boolean            InSide,
                                         const BRepOffset_Analyse&         Analyse,
                                         const ShapeImage&             InitOffsetFace,
                                         ShapeImage&                   InitOffsetEdge,
                                         const Message_ProgressRange&      theRange)
{
  ShapeList LInt1, LInt2;
  TopTools_MapOfShape  MV;
  ShapeExplorer      exp;
  TopoFace          OF1, OF2;
  TopoEdge          OE;
  ShapeBuilder         B;
  TopoEdge          NullEdge;
  TopoFace          NullFace;
  Standard_Integer     j;

  for (j = 1; j <= ContextFaces.Extent(); j++)
  {
    const TopoFace& CF = TopoDS::Face(ContextFaces(j));
    myTouched.Add(CF);
  }

  Message_ProgressScope aPS(theRange, "Intersecting with deepening faces", ContextFaces.Extent());
  for (j = 1; j <= ContextFaces.Extent(); j++, aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    const TopoFace& CF = TopoDS::Face(ContextFaces(j));
    for (exp.Init(CF.Oriented(TopAbs_FORWARD), TopAbs_EDGE); exp.More(); exp.Next())
    {
      const TopoEdge& E = TopoDS::Edge(exp.Current());
      if (!Analyse.HasAncestor(E))
      {
        if (InSide)
          myAsDes->Add(CF, E);
        else
        {
          TopoEdge NE;
          if (!InitOffsetEdge.HasImage(E))
          {
            Standard_Real f, l, Tol;
            BRepInspector::Range(E, f, l);
            Tol = BRepInspector::Tolerance(E);
            ExtentEdge(CF, E, NE);
            TopoVertex V1, V2;
            TopExp1::Vertices(E, V1, V2);
            NE.Orientation(TopAbs_FORWARD);
            myAsDes->Add(NE, V1.Oriented(TopAbs_REVERSED));
            myAsDes->Add(NE, V2.Oriented(TopAbs_FORWARD));
            TopoShape aLocalShape = V1.Oriented(TopAbs_INTERNAL);
            B.UpdateVertex(TopoDS::Vertex(aLocalShape), f, NE, Tol);
            aLocalShape = V2.Oriented(TopAbs_INTERNAL);
            B.UpdateVertex(TopoDS::Vertex(aLocalShape), l, NE, Tol);
            //            B.UpdateVertex(TopoDS::Vertex(V1.Oriented(TopAbs_INTERNAL)),f,NE,Tol);
            //            B.UpdateVertex(TopoDS::Vertex(V2.Oriented(TopAbs_INTERNAL)),l,NE,Tol);
            NE.Orientation(E.Orientation());
            myAsDes->Add(CF, NE);
            InitOffsetEdge.Bind(E, NE);
          }
          else
          {
            NE = TopoDS::Edge(InitOffsetEdge.Image(E).First());
            myAsDes->Add(CF, NE.Oriented(E.Orientation()));
          }
        }
        continue;
      }
      OE.Nullify();
      //---------------------------------------------------
      // OF1 parallel facee generated by the ancestor of E.
      //---------------------------------------------------
      const TopoShape SI = Analyse.Ancestors(E).First();
      OF1                   = TopoDS::Face(InitOffsetFace.Image(SI).First());
      OE                    = TopoDS::Edge(InitOffsetEdge.Image(E).First());

      {
        // Check if OE has pcurve in CF

        Standard_Real f, l;

        Handle(GeomCurve2d) C1 = BRepInspector::CurveOnSurface(OE, CF, f, l);
        Handle(GeomCurve2d) C2 = BRepInspector::CurveOnSurface(OE, OF1, f, l);

        if (C1.IsNull() || C2.IsNull())
        {
          continue;
        }
      }

      //--------------------------------------------------
      // MAJ of OE on cap CF.
      //--------------------------------------------------
      //      ShapeList LOE; LOE.Append(OE);
      //      Tool5::TryProject(CF,OF1,LOE,LInt1,LInt2,mySide);
      //      LInt2.Clear();
      //      StoreInter3d(CF,OF1,myTouched,NewEdges,InterDone,myAsDes,
      //                   LInt1,LInt2);
      LInt1.Clear();
      LInt1.Append(OE);
      LInt2.Clear();
      TopAbs_Orientation anOri1, anOri2;
      Tool5::OrientSection(OE, CF, OF1, anOri1, anOri2);
      //    if (mySide == TopAbs_OUT);
      anOri1 = TopAbs1::Reverse(anOri1);
      LInt1.First().Orientation(anOri1);
      Store(CF, OF1, LInt1, LInt2);

      //------------------------------------------------------
      // Processing of offsets on the ancestors of vertices.
      //------------------------------------------------------
      TopoVertex V[2];
      TopExp1::Vertices(E, V[0], V[1]);
      for (Standard_Integer i = 0; i < 2; i++)
      {
        if (!MV.Add(V[i]))
          continue;
        OF1.Nullify();
        const ShapeList&        LE = Analyse.Ancestors(V[i]);
        TopTools_ListIteratorOfListOfShape itLE(LE);
        for (; itLE.More(); itLE.Next())
        {
          const TopoEdge& EV = TopoDS::Edge(itLE.Value());
          if (InitOffsetFace.HasImage(EV))
          {
            //-------------------------------------------------
            // OF1 parallel face generated by an ancestor edge of V[i].
            //-------------------------------------------------
            OF1 = TopoDS::Face(InitOffsetFace.Image(EV).First());
            OE  = TopoDS::Edge(InitOffsetEdge.Image(V[i]).First());

            {
              // Check if OE has pcurve in CF and OF1

              Standard_Real f, l;

              Handle(GeomCurve2d) C1 = BRepInspector::CurveOnSurface(OE, CF, f, l);
              Handle(GeomCurve2d) C2 = BRepInspector::CurveOnSurface(OE, OF1, f, l);

              if (C1.IsNull() || C2.IsNull())
              {
                continue;
              }
            }

            //--------------------------------------------------
            // MAj of OE on cap CF.
            //--------------------------------------------------
            //              LOE.Clear(); LOE.Append(OE);
            //              Tool5::TryProject(CF,OF1,LOE,LInt1,LInt2,mySide);
            //              LInt2.Clear();
            //              StoreInter3d(CF,OF1,myTouched,NewEdges,InterDone,myAsDes,
            //                           LInt1,LInt2);
            LInt1.Clear();
            LInt1.Append(OE);
            LInt2.Clear();
            TopAbs_Orientation O1, O2;
            Tool5::OrientSection(OE, CF, OF1, O1, O2);
            //            if (mySide == TopAbs_OUT);
            O1 = TopAbs1::Reverse(O1);
            LInt1.First().Orientation(O1);
            Store(CF, OF1, LInt1, LInt2);
          }
        }
      }
    }

    for (exp.Init(CF.Oriented(TopAbs_FORWARD), TopAbs_VERTEX); exp.More(); exp.Next())
    {
      const TopoVertex& V = TopoDS::Vertex(exp.Current());
      if (!Analyse.HasAncestor(V))
      {
        continue;
      }
      const ShapeList&        LE = Analyse.Ancestors(V);
      TopTools_ListIteratorOfListOfShape itLE(LE);
      for (; itLE.More(); itLE.Next())
      {
        const TopoEdge&                 EV = TopoDS::Edge(itLE.Value());
        const ShapeList&        LF = Analyse.Ancestors(EV);
        TopTools_ListIteratorOfListOfShape itLF(LF);
        for (; itLF.More(); itLF.Next())
        {
          const TopoFace& FEV = TopoDS::Face(itLF.Value());
          //-------------------------------------------------
          // OF1 parallel face generated by uneFace ancestor of V[i].
          //-------------------------------------------------
          OF1 = TopoDS::Face(InitOffsetFace.Image(FEV).First());
          if (!IsDone(OF1, CF))
          {
            //-------------------------------------------------------
            // Find if one of edges of OF1 has no trace in CF.
            //-------------------------------------------------------
            ShapeList LOE;
            ShapeExplorer      exp2(OF1.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
            for (; exp2.More(); exp2.Next())
            {
              LOE.Append(exp2.Current());
            }
            //-------------------------------------------------------
            // If no trace try intersection.
            //-------------------------------------------------------
            if (!Tool5::TryProject(CF, OF1, LOE, LInt1, LInt2, mySide, myTol)
                || LInt1.IsEmpty())
            {
              Tool5::Inter3D(CF, OF1, LInt1, LInt2, mySide, NullEdge, NullFace, NullFace);
            }
            Store(CF, OF1, LInt1, LInt2);
          }
        }
      }
    }
  }
}

//=================================================================================================

void BRepOffset_Inter3d::SetDone(const TopoFace& F1, const TopoFace& F2)
{
  if (!myDone.IsBound(F1))
  {
    ShapeList empty;
    myDone.Bind(F1, empty);
  }
  myDone(F1).Append(F2);
  if (!myDone.IsBound(F2))
  {
    ShapeList empty;
    myDone.Bind(F2, empty);
  }
  myDone(F2).Append(F1);
}

//=================================================================================================

Standard_Boolean BRepOffset_Inter3d::IsDone(const TopoFace& F1, const TopoFace& F2) const
{
  if (myDone.IsBound(F1))
  {
    TopTools_ListIteratorOfListOfShape it(myDone(F1));
    for (; it.More(); it.Next())
    {
      if (it.Value().IsSame(F2))
        return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

void BRepOffset_Inter3d::Store(const TopoFace&          F1,
                               const TopoFace&          F2,
                               const ShapeList& LInt1,
                               const ShapeList& LInt2)
{
  if (!LInt1.IsEmpty())
  {
    myTouched.Add(F1);
    myTouched.Add(F2);
    myAsDes->Add(F1, LInt1);
    myAsDes->Add(F2, LInt2);
    TopTools_ListIteratorOfListOfShape it(LInt1);
    for (; it.More(); it.Next())
    {
      myNewEdges.Add(it.Value());
    }
  }
  SetDone(F1, F2);
}
