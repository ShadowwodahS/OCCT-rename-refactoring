// Created on: 1995-11-10
// Created by: Yves FRICAUD
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
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRep_TVertex.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepAlgo_Loop.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomLib.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Ax2.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

#include <stdio.h>
// #define OCCT_DEBUG_ALGO
// #define DRAW
#ifdef DRAW
  #include <DBRep.hxx>
  #pragma comment(lib, "TKDraw")
#endif
#ifdef OCCT_DEBUG_ALGO
Standard_Boolean AffichLoop = Standard_True;
Standard_Integer NbLoops    = 0;
Standard_Integer NbWires    = 1;
static char*     name       = new char[100];
#endif

//=================================================================================================

BRepAlgo_Loop::BRepAlgo_Loop()
{
}

//=================================================================================================

void BRepAlgo_Loop::Init(const TopoDS_Face& F)
{
  myConstEdges.Clear();
  myEdges.Clear();
  myVerOnEdges.Clear();
  myNewWires.Clear();
  myNewFaces.Clear();
  myCutEdges.Clear();
  myFace = F;
}

//=======================================================================
// function : Bubble
// purpose  : Orders the sequence of vertices by increasing parameter.
//=======================================================================

static void Bubble(const TopoDS_Edge& E, TopTools_SequenceOfShape& Seq)
{
  // Remove duplicates
  for (Standard_Integer i = 1; i < Seq.Length(); i++)
    for (Standard_Integer j = i + 1; j <= Seq.Length(); j++)
      if (Seq(i) == Seq(j))
      {
        Seq.Remove(j);
        j--;
      }

  Standard_Boolean Invert   = true;
  Standard_Integer NbPoints = Seq.Length();

  while (Invert)
  {
    Invert = false;
    for (Standard_Integer i = 1; i < NbPoints; i++)
    {
      TopoDS_Shape aLocalV = Seq.Value(i).Oriented(TopAbs_INTERNAL);
      TopoDS_Vertex V1     = TopoDS::Vertex(aLocalV);
      aLocalV              = Seq.Value(i + 1).Oriented(TopAbs_INTERNAL);
      TopoDS_Vertex V2     = TopoDS::Vertex(aLocalV);

      Standard_Real U1 = BRep_Tool::Parameter(V1, E);
      Standard_Real U2 = BRep_Tool::Parameter(V2, E);
      if (U2 < U1)
      {
        Seq.Exchange(i, i + 1);
        Invert = true;
      }
    }
  }
}

//=================================================================================================

void BRepAlgo_Loop::AddEdge(TopoDS_Edge& E, const TopTools_ListOfShape& LV)
{
  myEdges.Append(E);
  myVerOnEdges.Bind(E, LV);
}

//=================================================================================================

void BRepAlgo_Loop::AddConstEdge(const TopoDS_Edge& E)
{
  myConstEdges.Append(E);
}

//=================================================================================================

void BRepAlgo_Loop::AddConstEdges(const TopTools_ListOfShape& LE)
{
  for (const auto& edge : LE)
  {
    myConstEdges.Append(edge);
  }
}

//=================================================================================================

void BRepAlgo_Loop::SetImageVV(const BRepAlgo_Image& theImageVV)
{
  myImageVV = theImageVV;
}

//=======================================================================
// function : UpdateClosedEdge
// purpose  : If the first or the last vertex of intersection
//           coincides with the closing vertex, it is removed from SV.
//           it will be added at the beginning and the end of SV by the caller.
//=======================================================================

static TopoDS_Vertex UpdateClosedEdge(const TopoDS_Edge& E, TopTools_SequenceOfShape& SV)
{
  TopoDS_Vertex    VB[2], V1, V2, VRes;
  gp_Pnt           P, PC;
  Standard_Boolean OnStart = false, OnEnd = false;
  //// modified by jgv, 13.04.04 for OCC5634 ////
  TopExp::Vertices(E, V1, V2);
  Standard_Real Tol = BRep_Tool::Tolerance(V1);
  ///////////////////////////////////////////////

  if (SV.IsEmpty())
    return VRes;

  VB[0] = TopoDS::Vertex(SV.First());
  VB[1] = TopoDS::Vertex(SV.Last());
  PC    = BRep_Tool::Pnt(V1);

  for (Standard_Integer i = 0; i < 2; i++)
  {
    P = BRep_Tool::Pnt(VB[i]);
    if (P.IsEqual(PC, Tol))
    {
      VRes = VB[i];
      if (i == 0)
        OnStart = true;
      else
        OnEnd = true;
    }
  }
  if (OnStart && OnEnd)
  {
    if (!VB[0].IsSame(VB[1]))
    {
#ifdef OCCT_DEBUG_ALGO
      if (AffichLoop)
        std::cout << "Two different vertices on the closing vertex" << std::endl;
#endif
    }
    else
    {
      SV.Remove(1);
      if (!SV.IsEmpty())
        SV.Remove(SV.Length());
    }
  }
  else if (OnStart)
    SV.Remove(1);
  else if (OnEnd)
    SV.Remove(SV.Length());

  return VRes;
}

//=================================================================================================

static void RemovePendingEdges(TopTools_IndexedDataMapOfShapeListOfShape& MVE)
{
  //--------------------------------
  // Remove hanging edges.
  //--------------------------------
  TopTools_ListOfShape        ToRemove;
  Standard_Boolean            YaSupress = true;

  while (YaSupress)
  {
    YaSupress = false;
    TopTools_ListOfShape VToRemove;
    TopTools_MapOfShape  EToRemove;

    for (Standard_Integer iV = 1; iV <= MVE.Extent(); iV++)
    {
      const TopoDS_Shape&         aVertex = MVE.FindKey(iV);
      const TopTools_ListOfShape& anEdges = MVE(iV);
      if (anEdges.IsEmpty())
      {
        VToRemove.Append(aVertex);
      }
      if (anEdges.Extent() == 1)
      {
        const TopoDS_Edge& E = TopoDS::Edge(anEdges.First());
        TopoDS_Vertex      V1, V2;
        TopExp::Vertices(E, V1, V2);
        if (!V1.IsSame(V2))
        {
          VToRemove.Append(aVertex);
          EToRemove.Add(anEdges.First());
        }
      }
    }

    if (!VToRemove.IsEmpty())
    {
      YaSupress = true;
      for (const auto& vertex : VToRemove)
      {
        MVE.RemoveKey(vertex);
      }
      if (!EToRemove.IsEmpty())
      {
        for (Standard_Integer iV = 1; iV <= MVE.Extent(); iV++)
        {
          TopTools_ListOfShape& LE = MVE.ChangeFromIndex(iV);
          TopTools_ListIteratorOfListOfShape itl(LE);
          while (itl.More())
          {
            if (EToRemove.Contains(itl.Value()))
            {
              LE.Remove(itl);
            }
            else
              itl.Next();
          }
        }
      }
    }
  }
}

//=================================================================================================

static Standard_Boolean SamePnt2d(const TopoDS_Vertex& V,
                                  TopoDS_Edge&         E1,
                                  TopoDS_Edge&         E2,
                                  TopoDS_Face&         F)
{
  Standard_Real f1, f2, l1, l2;
  gp_Pnt2d      P1, P2;
  TopoDS_Shape  aLocalF = F.Oriented(TopAbs_FORWARD);
  TopoDS_Face   FF      = TopoDS::Face(aLocalF);
  //  TopoDS_Face FF = TopoDS::Face(F.Oriented(TopAbs_FORWARD));
  Handle(Geom2d_Curve) C1 = BRep_Tool::CurveOnSurface(E1, FF, f1, l1);
  Handle(Geom2d_Curve) C2 = BRep_Tool::CurveOnSurface(E2, FF, f2, l2);
  if (E1.Orientation() == TopAbs_FORWARD)
    P1 = C1->Value(f1);
  else
    P1 = C1->Value(l1);

  if (E2.Orientation() == TopAbs_FORWARD)
    P2 = C2->Value(l2);
  else
    P2 = C2->Value(f2);
  Standard_Real Tol  = 100 * BRep_Tool::Tolerance(V);
  Standard_Real Dist = P1.Distance(P2);
  return Dist < Tol;
}

//=======================================================================
// function : SelectEdge
// purpose  : Find edge <NE> connected to <CE> by vertex <CV> in the
//           list <LE>. <NE> is removed from the list. If <CE> is
//           also in the list <LE> with the same orientation, it is
//           removed from the list.
//=======================================================================

static Standard_Boolean SelectEdge(const TopoDS_Face&    F,
                                   const TopoDS_Edge&    CE,
                                   const TopoDS_Vertex&  CV,
                                   TopoDS_Edge&          NE,
                                   TopTools_ListOfShape& LE)
{
  TopTools_ListIteratorOfListOfShape itl;
  NE.Nullify();
#ifdef OCCT_DEBUG_ALGO
  if (AffichLoop)
  {
    if (LE.Extent() > 2)
    {
      std::cout << "vertex on more than 2 edges in a face." << std::endl;
    }
  }
#endif
  for (itl.Initialize(LE); itl.More(); itl.Next())
  {
    if (itl.Value().IsEqual(CE))
    {
      LE.Remove(itl);
      break;
    }
  }
  if (LE.Extent() > 1)
  {
    //--------------------------------------------------------------
    // Several edges possible.
    // - Test edges different from CE , Selection of edge
    // for which CV has U,V closer to the face
    // than corresponding to CE.
    // - If several edges give representation less than the tolerance.
    // discrimination on tangents.
    //--------------------------------------------------------------
    Standard_Real   f, l;
    TopoDS_Face     FForward = F;
    FForward.Orientation(TopAbs_FORWARD);

    Handle(Geom2d_Curve) C = BRep_Tool::CurveOnSurface(CE, FForward, f, l);
    Standard_Integer     k = 1, kmin   = 0;
    Standard_Real        dist, distmin = 100 * BRep_Tool::Tolerance(CV);
    Standard_Real        u;
    if (CE.Orientation() == TopAbs_FORWARD)
      u = l;
    else
      u = f;

    gp_Pnt2d PV = C->Value(u);

    for (const auto& shape : LE)
    {
      const TopoDS_Edge& E = TopoDS::Edge(shape);
      if (!E.IsSame(CE))
      {
        C = BRep_Tool::CurveOnSurface(E, FForward, f, l);
        if (E.Orientation() == TopAbs_FORWARD)
          u = f;
        else
          u = l;
        gp_Pnt2d P2 = C->Value(u);
        dist        = PV.Distance(P2);
        if (dist <= distmin)
        {
          kmin    = k;
          distmin = dist;
        }
      }
      k++;
    }
    if (kmin == 0)
      return false;

    k = 1;
    itl.Initialize(LE);
    while (k < kmin)
    {
      k++;
      itl.Next();
    }
    NE = TopoDS::Edge(itl.Value());
    LE.Remove(itl);
  }
  else if (LE.Extent() == 1)
  {
    NE = TopoDS::Edge(LE.First());
    LE.RemoveFirst();
  }
  else
  {
    return false;
  }
#ifdef DRAW
  if (AffichLoop)
  {
    DBRep::Set("Selected", NE);
  }

#endif
  return true;
}

//=================================================================================================

static void PurgeNewEdges(TopTools_DataMapOfShapeListOfShape& NewEdges,
                          const TopTools_MapOfShape&          UsedEdges)
{
  for (auto& entry : NewEdges)
  {
    TopTools_ListOfShape&              LNE = entry;
    TopTools_ListIteratorOfListOfShape itL(LNE);
    while (itL.More())
    {
      const TopoDS_Shape& NE = itL.Value();
      if (!UsedEdges.Contains(NE))
      {
        LNE.Remove(itL);
      }
      else
      {
        itL.Next();
      }
    }
  }
}

//=================================================================================================

static void StoreInMVE(const TopoDS_Face&                         F,
                       TopoDS_Edge&                               E,
                       TopTools_IndexedDataMapOfShapeListOfShape& MVE,
                       Standard_Boolean&                          YaCouture,
                       TopTools_DataMapOfShapeShape&              VerticesForSubstitute,
                       const Standard_Real                        theTolConf)
{
  TopoDS_Vertex        V1, V2, V;
  TopTools_ListOfShape Empty;

  BRep_Builder BB;
  for (Standard_Integer iV = 1; iV <= MVE.Extent(); iV++)
  {
    V = TopoDS::Vertex(MVE.FindKey(iV));
    gp_Pnt P = BRep_Tool::Pnt(V);
    
    TopTools_ListOfShape VList;
    for (TopoDS_Iterator VerExp(E); VerExp.More(); VerExp.Next())
      VList.Append(VerExp.Value());

    for (const auto& shape : VList)
    {
      V1 = TopoDS::Vertex(shape);
      gp_Pnt P1 = BRep_Tool::Pnt(V1);
      if (P.IsEqual(P1, theTolConf) && !V.IsSame(V1))
      {
        V.Orientation(V1.Orientation());
        if (VerticesForSubstitute.IsBound(V1))
        {
          TopoDS_Shape OldNewV = VerticesForSubstitute(V1);
          if (!OldNewV.IsSame(V))
          {
            VerticesForSubstitute.Bind(OldNewV, V);
            VerticesForSubstitute(V1) = V;
          }
        }
        else
        {
          if (VerticesForSubstitute.IsBound(V))
          {
            TopoDS_Shape NewNewV = VerticesForSubstitute(V);
            if (!NewNewV.IsSame(V1))
              VerticesForSubstitute.Bind(V1, NewNewV);
          }
          else
          {
            VerticesForSubstitute.Bind(V1, V);
            for (auto& entry : VerticesForSubstitute)
              if (entry.IsSame(V1))
                entry = V;
          }
        }
        E.Free(true);
        BB.Remove(E, V1);
        BB.Add(E, V);
      }
    }
  }

  TopExp::Vertices(E, V1, V2);
  if (V1.IsNull() && V2.IsNull())
  {
    YaCouture = false;
    return;
  }
  if (!MVE.Contains(V1))
  {
    MVE.Add(V1, Empty);
  }
  MVE.ChangeFromKey(V1).Append(E);
  if (!V1.IsSame(V2))
  {
    if (!MVE.Contains(V2))
    {
      MVE.Add(V2, Empty);
    }
    MVE.ChangeFromKey(V2).Append(E);
  }
  TopLoc_Location      L;
  Handle(Geom_Surface) S = BRep_Tool::Surface(F, L);
  if (BRep_Tool::IsClosed(E, S, L))
  {
    MVE.ChangeFromKey(V2).Append(E.Reversed());
    if (!V1.IsSame(V2))
    {
      MVE.ChangeFromKey(V1).Append(E.Reversed());
    }
    YaCouture = true;
  }
}

//=================================================================================================

void BRepAlgo_Loop::Perform()
{
  TopoDS_Vertex    V1, V2;
  Standard_Boolean YaCouture = false;

#ifdef OCCT_DEBUG_ALGO
  if (AffichLoop)
  {
    std::cout << "NewLoop" << std::endl;
    NbLoops++;
  #ifdef DRAW
    sprintf(name, "FLoop_%d", NbLoops);
    DBRep::Set(name, myFace);
    Standard_Integer NbEdges = 1;
  #endif
    for (itl.Initialize(myEdges); itl.More(); itl.Next())
    {
      const TopoDS_Edge& E = TopoDS::Edge(itl.Value());
  #ifdef DRAW
      sprintf(name, "EEE_%d_%d", NbLoops, NbEdges++);
      DBRep::Set(name, E);
  #endif
    }
    for (itl.Initialize(myConstEdges); itl.More(); itl.Next())
    {
      const TopoDS_Edge& E = TopoDS::Edge(itl.Value());
  #ifdef DRAW
      sprintf(name, "EEE_%d_%d", NbLoops, NbEdges++);
      DBRep::Set(name, E);
  #endif
    }
  }
#endif
  //------------------------------------------------
  // Cut edges
  //------------------------------------------------
  for (const auto& shape : myEdges)
  {
    const TopoDS_Edge&          anEdge = TopoDS::Edge(shape);
    TopTools_ListOfShape        LCE;
    const TopTools_ListOfShape* pVertices = myVerOnEdges.Seek(anEdge);
    if (pVertices)
    {
      CutEdge(anEdge, *pVertices, LCE);
      myCutEdges.Bind(anEdge, LCE);
    }
  }
  //-----------------------------------
  // Construction map vertex => edges
  //-----------------------------------
  TopTools_IndexedDataMapOfShapeListOfShape MVE;

  // add cut edges.
  TopTools_MapOfShape Emap;
  for (const auto& shape : myEdges)
  {
    const TopTools_ListOfShape* pLCE = myCutEdges.Seek(shape);
    if (pLCE)
    {
      for (const auto& subShape : *pLCE)
      {
        TopoDS_Edge& E = TopoDS::Edge(const_cast<TopoDS_Shape&>(subShape));
        if (!Emap.Add(E))
          continue;
        StoreInMVE(myFace, E, MVE, YaCouture, myVerticesForSubstitute, myTolConf);
      }
    }
  }

  // add const edges
  // Sewn edges can be doubled or not in myConstEdges
  // => call only once StoreInMVE which should double them
  TopTools_MapOfShape DejaVu;
  for (auto& shape : myConstEdges)
  {
    TopoDS_Edge& E = TopoDS::Edge(shape);
    if (DejaVu.Add(E))
      StoreInMVE(myFace, E, MVE, YaCouture, myVerticesForSubstitute, myTolConf);
  }

#ifdef DRAW
  if (AffichLoop)
  {
    std::cout << "NewLoop" << std::endl;
    Standard_Integer    NbEdges = 1;
    TopTools_MapOfShape Done;
    for (Standard_Integer iV = 1; iV <= MVE.Extent(); iV++)
    {
      for (itl.Initialize(MVE(iV)); itl.More(); itl.Next())
      {
        TopoDS_Edge& E = TopoDS::Edge(itl.Value());
        if (Done.Add(E))
        {
          sprintf(name, "EEC_%d_%d", NbLoops, NbEdges++);
          DBRep::Set(name, E);
        }
      }
    }
  }
#endif

  //-----------------------------------------------
  // Construction of wires and new faces.
  //----------------------------------------------
  TopoDS_Vertex    VF, VL, CV;
  TopoDS_Edge      CE, NE, EF;
  BRep_Builder     B;
  TopoDS_Wire      NW;
  Standard_Boolean End;

  UpdateVEmap(MVE);

  TopTools_MapOfShape UsedEdges;

  while (MVE.Extent() > 0)
  {
    B.MakeWire(NW);
    //--------------------------------
    // Removal of hanging edges.
    //--------------------------------
    RemovePendingEdges(MVE);

    if (MVE.Extent() == 0)
      break;
    //--------------------------------
    // Start edge.
    //--------------------------------
    EF = CE = TopoDS::Edge(MVE(1).First());
    TopExp::Vertices(CE, V1, V2);
    //--------------------------------
    // VF vertex start of new wire
    //--------------------------------
    if (CE.Orientation() == TopAbs_FORWARD)
    {
      CV = VF = V1;
    }
    else
    {
      CV = VF = V2;
    }
    if (!MVE.Contains(CV))
      continue;
    if (!MVE.Contains(CV))
      continue;
    TopTools_ListOfShape& aListEdges = MVE.ChangeFromKey(CV);
    TopTools_ListIteratorOfListOfShape itl(aListEdges);
    for (; itl.More(); itl.Next())
    {
      if (itl.Value().IsEqual(CE))
      {
        aListEdges.Remove(itl);
        break;
      }
    }
    End = false;

    while (!End)
    {
      //-------------------------------
      // Construction of a wire.
      //-------------------------------
      TopExp::Vertices(CE, V1, V2);
      if (!CV.IsSame(V1))
        CV = V1;
      else
        CV = V2;

      B.Add(NW, CE);
      UsedEdges.Add(CE);

      if (!MVE.Contains(CV) || MVE.FindFromKey(CV).IsEmpty())
      {
        End = Standard_True;
      }
      else
      {
        End = !SelectEdge(myFace, CE, CV, NE, MVE.ChangeFromKey(CV));
        if (!End)
        {
          CE = NE;
          if (MVE.FindFromKey(CV).IsEmpty())
            MVE.RemoveKey(CV);
        }
      }
    }
    //--------------------------------------------------
    // Add new wire to the set of wires
    //------------------------------------------------

    if (VF.IsSame(CV))
    {
      if (SamePnt2d(VF, EF, CE, myFace))
      {
        NW.Closed(true);
        myNewWires.Append(NW);
      }
      else if (BRep_Tool::Tolerance(VF) < myTolConf)
      {
        BRep_Builder aBB;
        aBB.UpdateVertex(VF, myTolConf);
        if (SamePnt2d(VF, EF, CE, myFace))
        {
          NW.Closed(true);
          myNewWires.Append(NW);
        }
#ifdef OCCT_DEBUG_ALGO
        else
        {
          std::cout << "BRepAlgo_Loop: Open Wire" << std::endl;
          if (AffichLoop)
            std::cout << "OpenWire is : NW_" << NbLoops << "_" << NbWires << std::endl;
        }
#endif
      }
    }
#ifdef OCCT_DEBUG_ALGO
    else
    {
      std::cout << "BRepAlgo_Loop: Open Wire" << std::endl;
      if (AffichLoop)
        std::cout << "OpenWire is : NW_" << NbLoops << "_" << NbWires << std::endl;
    }
#endif

#ifdef DRAW
    if (AffichLoop)
    {
      sprintf(name, "NW_%d_%d", NbLoops, NbWires++);
      DBRep::Set(name, NW);
    }
#endif
  }

  PurgeNewEdges(myCutEdges, UsedEdges);
}

//=================================================================================================

void BRepAlgo_Loop::CutEdge(const TopoDS_Edge&          E,
                            const TopTools_ListOfShape& VOnE,
                            TopTools_ListOfShape&       NE) const
{
  TopoDS_Shape aLocalE = E.Oriented(TopAbs_FORWARD);
  TopoDS_Edge  WE      = TopoDS::Edge(aLocalE);

  Standard_Real            U1, U2;
  TopoDS_Vertex            V1, V2;
  TopTools_SequenceOfShape SV;
  BRep_Builder             B;

  for (const auto& shape : VOnE)
  {
    SV.Append(shape);
  }
  //--------------------------------
  // Parse vertices on the edge.
  //--------------------------------
  Bubble(WE, SV);

  Standard_Integer NbVer = SV.Length();
  //----------------------------------------------------------------
  // Construction of new edges.
  // Note :  vertices at the extremities of edges are not
  //         onligatorily in the list of vertices
  //----------------------------------------------------------------
  if (SV.IsEmpty())
  {
    NE.Append(E);
    return;
  }
  TopoDS_Vertex VF, VL;
  Standard_Real f, l;
  BRep_Tool::Range(WE, f, l);
  TopExp::Vertices(WE, VF, VL);

  if (NbVer == 2)
  {
    if (SV(1).IsEqual(VF) && SV(2).IsEqual(VL))
    {
      NE.Append(E);
#ifdef DRAW
      if (AffichLoop)
      {
        DBRep::Set("ECOpied", E);
      }
#endif
      return;
    }
  }
  //----------------------------------------------------
  // Processing of closed edges
  // If a vertex of intersection is on the common vertex
  // it should appear at the beginning and end of SV.
  //----------------------------------------------------
  TopoDS_Vertex VCEI;
  if (!VF.IsNull() && VF.IsSame(VL))
  {
    VCEI = UpdateClosedEdge(WE, SV);
    if (!VCEI.IsNull())
    {
      TopoDS_Shape aLocalV = VCEI.Oriented(TopAbs_FORWARD);
      VF                   = TopoDS::Vertex(aLocalV);
      aLocalV              = VCEI.Oriented(TopAbs_REVERSED);
      VL                   = TopoDS::Vertex(aLocalV);
    }
    SV.Prepend(VF);
    SV.Append(VL);
  }
  else
  {
    //-----------------------------------------
    // Eventually all extremities of the edge.
    //-----------------------------------------
    if (!VF.IsNull() && !VF.IsSame(SV.First()))
      SV.Prepend(VF);
    if (!VL.IsNull() && !VL.IsSame(SV.Last()))
      SV.Append(VL);
  }

  while (!SV.IsEmpty())
  {
    while (!SV.IsEmpty() && SV.First().Orientation() != TopAbs_FORWARD)
    {
      SV.Remove(1);
    }
    if (SV.IsEmpty())
      break;
    V1 = TopoDS::Vertex(SV.First());
    SV.Remove(1);
    if (SV.IsEmpty())
      break;
    if (SV.First().Orientation() == TopAbs_REVERSED)
    {
      V2 = TopoDS::Vertex(SV.First());
      SV.Remove(1);
      //-------------------------------------------
      // Copy the edge and restriction by V1 V2.
      //-------------------------------------------
      TopoDS_Shape NewEdge    = WE.EmptyCopied();
      TopoDS_Shape aLocalEdge = V1.Oriented(TopAbs_FORWARD);
      B.Add(NewEdge, aLocalEdge);
      aLocalEdge = V2.Oriented(TopAbs_REVERSED);
      B.Add(TopoDS::Edge(NewEdge), aLocalEdge);
      if (V1.IsSame(VF))
        U1 = f;
      else
      {
        TopoDS_Shape aLocalV = V1.Oriented(TopAbs_INTERNAL);
        U1                   = BRep_Tool::Parameter(TopoDS::Vertex(aLocalV), WE);
      }
      if (V2.IsSame(VL))
        U2 = l;
      else
      {
        TopoDS_Shape aLocalV = V2.Oriented(TopAbs_INTERNAL);
        U2                   = BRep_Tool::Parameter(TopoDS::Vertex(aLocalV), WE);
      }
      B.Range(TopoDS::Edge(NewEdge), U1, U2);
#ifdef DRAW
      if (AffichLoop)
      {
        DBRep::Set("Cut", NewEdge);
      }
#endif
      NE.Append(NewEdge.Oriented(E.Orientation()));
    }
  }

  // Remove edges with size <= tolerance
  Standard_Real Tol = 0.001; // 5.e-05; //5.e-07;
  TopTools_ListIteratorOfListOfShape it(NE);
  while (it.More())
  {
    // skl : I change "E" to "EE"
    TopoDS_Edge   EE = TopoDS::Edge(it.Value());
    Standard_Real fpar, lpar;
    BRep_Tool::Range(EE, fpar, lpar);
    if (lpar - fpar <= Precision::Confusion())
      NE.Remove(it);
    else
    {
      gp_Pnt2d pf, pl;
      BRep_Tool::UVPoints(EE, myFace, pf, pl);
      if (pf.Distance(pl) <= Tol && !BRep_Tool::IsClosed(EE))
        NE.Remove(it);
      else
        it.Next();
    }
  }
}

//=================================================================================================

const TopTools_ListOfShape& BRepAlgo_Loop::NewWires() const
{
  return myNewWires;
}

//=================================================================================================

const TopTools_ListOfShape& BRepAlgo_Loop::NewFaces() const
{
  return myNewFaces;
}

//=================================================================================================

void BRepAlgo_Loop::WiresToFaces()
{
  if (!myNewWires.IsEmpty())
  {
    BRepAlgo_FaceRestrictor FR;
    TopoDS_Shape            aLocalS = myFace.Oriented(TopAbs_FORWARD);
    FR.Init(TopoDS::Face(aLocalS), Standard_False);
    //    FR.Init (TopoDS::Face(myFace.Oriented(TopAbs_FORWARD)),
    //	     Standard_False);
    for (auto& wire : myNewWires)
    {
      FR.Add(TopoDS::Wire(wire));
    }

    FR.Perform();

    if (FR.IsDone())
    {
      TopAbs_Orientation OriF = myFace.Orientation();
      for (; FR.More(); FR.Next())
      {
        myNewFaces.Append(FR.Current().Oriented(OriF));
      }
    }
  }
}

//=================================================================================================

const TopTools_ListOfShape& BRepAlgo_Loop::NewEdges(const TopoDS_Edge& E) const
{
  return myCutEdges(E);
}

//=================================================================================================

void BRepAlgo_Loop::GetVerticesForSubstitute(TopTools_DataMapOfShapeShape& VerVerMap) const
{
  VerVerMap = myVerticesForSubstitute;
}

//=================================================================================================

void BRepAlgo_Loop::VerticesForSubstitute(TopTools_DataMapOfShapeShape& VerVerMap)
{
  myVerticesForSubstitute = VerVerMap;
}

//=================================================================================================

void BRepAlgo_Loop::UpdateVEmap(TopTools_IndexedDataMapOfShapeListOfShape& theVEmap)
{
  TopTools_IndexedDataMapOfShapeListOfShape VerLver;

  for (Standard_Integer ii = 1; ii <= theVEmap.Extent(); ii++)
  {
    const TopoDS_Vertex&        aVertex = TopoDS::Vertex(theVEmap.FindKey(ii));
    const TopTools_ListOfShape& aElist  = theVEmap(ii);
    if (aElist.Extent() == 1 && myImageVV.IsImage(aVertex))
    {
      const TopoDS_Vertex& aProVertex = TopoDS::Vertex(myImageVV.ImageFrom(aVertex));
      if (VerLver.Contains(aProVertex))
      {
        TopTools_ListOfShape& aVlist = VerLver.ChangeFromKey(aProVertex);
        aVlist.Append(aVertex.Oriented(TopAbs_FORWARD));
      }
      else
      {
        TopTools_ListOfShape aVlist;
        aVlist.Append(aVertex.Oriented(TopAbs_FORWARD));
        VerLver.Add(aProVertex, aVlist);
      }
    }
  }

  if (VerLver.IsEmpty())
    return;

  BRep_Builder aBB;
  for (Standard_Integer ii = 1; ii <= VerLver.Extent(); ii++)
  {
    const TopTools_ListOfShape& aVlist = VerLver(ii);
    if (aVlist.Extent() == 1)
      continue;

    Standard_Real      aMaxTol = 0.;
    TColgp_Array1OfPnt Points(1, aVlist.Extent());

    TopTools_ListIteratorOfListOfShape itl(aVlist);
    Standard_Integer                   jj = 0;
    for (; itl.More(); itl.Next())
    {
      const TopoDS_Vertex& aVertex = TopoDS::Vertex(itl.Value());
      Standard_Real        aTol    = BRep_Tool::Tolerance(aVertex);
      aMaxTol                      = Max(aMaxTol, aTol);
      gp_Pnt aPnt                  = BRep_Tool::Pnt(aVertex);
      Points(++jj)                 = aPnt;
    }

    gp_Ax2           anAxis;
    Standard_Boolean IsSingular;
    GeomLib::AxeOfInertia(Points, anAxis, IsSingular);
    gp_Pnt        aCentre  = anAxis.Location();
    Standard_Real aMaxDist = 0.;
    for (jj = 1; jj <= Points.Upper(); jj++)
    {
      Standard_Real aSqDist = aCentre.SquareDistance(Points(jj));
      aMaxDist              = Max(aMaxDist, aSqDist);
    }
    aMaxDist = Sqrt(aMaxDist);
    aMaxTol  = Max(aMaxTol, aMaxDist);

    // Find constant vertex
    TopoDS_Vertex aConstVertex;
    for (const auto& shape : aVlist)
    {
      const TopoDS_Vertex&               aVertex = TopoDS::Vertex(shape);
      const TopTools_ListOfShape&        aElist  = theVEmap.FindFromKey(aVertex);
      const TopoDS_Shape&                anEdge  = aElist.First();
      for (const auto& constEdge : myConstEdges)
        if (anEdge.IsSame(constEdge))
        {
          aConstVertex = aVertex;
          break;
        }
      if (!aConstVertex.IsNull())
        break;
    }
    if (aConstVertex.IsNull())
      aConstVertex = TopoDS::Vertex(aVlist.First());
    aBB.UpdateVertex(aConstVertex, aCentre, aMaxTol);

    for (const auto& shape : aVlist)
    {
      const TopoDS_Vertex& aVertex = TopoDS::Vertex(shape);
      if (aVertex.IsSame(aConstVertex))
        continue;

      const TopTools_ListOfShape& aElist = theVEmap.FindFromKey(aVertex);
      TopoDS_Edge                 anEdge = TopoDS::Edge(aElist.First());
      anEdge.Orientation(TopAbs_FORWARD);
      TopoDS_Vertex aV1, aV2;
      TopExp::Vertices(anEdge, aV1, aV2);
      TopoDS_Vertex aVertexToRemove = (aV1.IsSame(aVertex)) ? aV1 : aV2;
      anEdge.Free(true);
      aBB.Remove(anEdge, aVertexToRemove);
      aBB.Add(anEdge, aConstVertex.Oriented(aVertexToRemove.Orientation()));
    }
  }

  TopTools_IndexedMapOfShape Emap;
  for (Standard_Integer ii = 1; ii <= theVEmap.Extent(); ii++)
  {
    const TopTools_ListOfShape&        aElist = theVEmap(ii);
    for (const auto& shape : aElist)
      Emap.Add(shape);
  }

  theVEmap.Clear();
  for (Standard_Integer ii = 1; ii <= Emap.Extent(); ii++)
    TopExp::MapShapesAndAncestors(Emap(ii), TopAbs_VERTEX, TopAbs_EDGE, theVEmap);
}
