// Created on: 2012-08-06
// Created by: jgv@ROLEX
// Copyright (c) 2012-2014 OPEN CASCADE SAS
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
#include <BRepAdaptor_Curve.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepLib_MakeWire.hxx>
#include <BRepOffsetAPI_MiddlePath.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <GC_MakeCircle.hxx>
#include <GCE2d_MakeLine.hxx>
#include <gce_MakeLin.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomLib.hxx>
#include <gp_Circ.hxx>
#include <gp_Lin.hxx>
#include <GProp_GProps.hxx>
#include <Precision.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TColStd_HArray1OfBoolean.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

static Standard_Boolean IsLinear(const TopoEdge& anEdge, gp_Lin& aLine)
{
  Standard_Real      fpar, lpar;
  Handle(GeomCurve3d) aCurve = BRepInspector::Curve(anEdge, fpar, lpar);
  if (aCurve->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
    aCurve = Handle(Geom_TrimmedCurve)::DownCast(aCurve)->BasisCurve();

  Point3d Pnt1, Pnt2;
  if (aCurve->IsKind(STANDARD_TYPE(GeomLine)))
  {
    aLine = Handle(GeomLine)::DownCast(aCurve)->Lin();
    return Standard_True;
  }
  else if (aCurve->IsKind(STANDARD_TYPE(BezierCurve3d)))
  {
    Handle(BezierCurve3d) theBezier = Handle(BezierCurve3d)::DownCast(aCurve);
    if (theBezier->NbPoles() == 2)
    {
      Pnt1  = theBezier->Pole(1);
      Pnt2  = theBezier->Pole(2);
      aLine = gce_MakeLin(Pnt1, Pnt2);
      return Standard_True;
    }
  }
  else if (aCurve->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    Handle(BSplineCurve3d) theBSpline = Handle(BSplineCurve3d)::DownCast(aCurve);
    if (theBSpline->NbPoles() == 2)
    {
      Pnt1  = theBSpline->Pole(1);
      Pnt2  = theBSpline->Pole(2);
      aLine = gce_MakeLin(Pnt1, Pnt2);
      return Standard_True;
    }
  }

  return Standard_False;
}

static GeomAbs_CurveType TypeOfEdge(const TopoEdge& anEdge)
{
  gp_Lin aLin;
  if (IsLinear(anEdge, aLin))
    return GeomAbs_Line;

  BRepAdaptor_Curve BAcurve(anEdge);
  return BAcurve.GetType();
}

static Vector3d TangentOfEdge(const TopoShape& aShape, const Standard_Boolean OnFirst)
{
  TopoEdge        anEdge = TopoDS::Edge(aShape);
  TopAbs_Orientation anOr   = anEdge.Orientation();

  Standard_Real      fpar, lpar;
  Handle(GeomCurve3d) aCurve = BRepInspector::Curve(anEdge, fpar, lpar);
  Standard_Real      thePar;
  if (OnFirst)
    thePar = (anOr == TopAbs_FORWARD) ? fpar : lpar;
  else
    thePar = (anOr == TopAbs_FORWARD) ? lpar : fpar;

  Point3d thePoint;
  Vector3d theTangent;
  aCurve->D1(thePar, thePoint, theTangent);
  if (anOr == TopAbs_REVERSED)
    theTangent.Reverse();

  return theTangent;
}

static Standard_Boolean IsValidEdge(const TopoEdge& theEdge, const TopoFace& theFace)
{
  TopoVertex V1, V2;
  TopExp1::Vertices(theEdge, V1, V2);

  constexpr Standard_Real Tol = Precision::Confusion();
  Standard_Integer        i;

  ShapeExplorer Explo(theFace, TopAbs_EDGE);
  for (; Explo.More(); Explo.Next())
  {
    const TopoShape&        anEdge = Explo.Current();
    BRepExtrema_DistShapeShape DistMini(theEdge, anEdge);
    if (DistMini.Value() <= Tol)
    {
      for (i = 1; i <= DistMini.NbSolution(); i++)
      {
        BRepExtrema_SupportType theType = DistMini.SupportTypeShape2(i);
        if (theType == BRepExtrema_IsOnEdge)
          return Standard_False;
        // theType is "IsVertex"
        TopoShape aVertex = DistMini.SupportOnShape2(i);
        if (!(aVertex.IsSame(V1) || aVertex.IsSame(V2)))
          return Standard_False;
      }
    }
  }

  return Standard_True;
}

/*
//=================================================================================================

BRepOffsetAPI_MiddlePath::BRepOffsetAPI_MiddlePath(const TopoShape& aShape,
                                                   const TopoWire&  StartWire)
{
  myInitialShape = aShape;
  myStartWire    = StartWire;
  myClosedSection = myStartWire.Closed();
}

//=================================================================================================

BRepOffsetAPI_MiddlePath::BRepOffsetAPI_MiddlePath(const TopoShape& aShape,
                                                   const TopoEdge&  StartEdge)
{
  myInitialShape = aShape;

  BRepLib_MakeWire MW(StartEdge);

  //BB.Add(myStartWire, StartEdge);

  TopTools_IndexedDataMapOfShapeListOfShape EFmap;
  TopTools_IndexedDataMapOfShapeListOfShape VEmap;
  TopExp1::MapShapesAndAncestors(myInitialShape, TopAbs_EDGE,   TopAbs_FACE, EFmap);
  TopExp1::MapShapesAndAncestors(myInitialShape, TopAbs_VERTEX, TopAbs_EDGE, VEmap);

  //Standard_Boolean Start = Standard_True;
  //if (Start)
  //{
  //TopExp1::Vertices(CurEdge, V1, V2);
  //  StartVertex = V1;
  //  CurVertex   = V2;
  //  if (VEmap(CurVertex).Extent() == 2) //end: two free edges
  //  {
  //    StartVertex = V2;
  //    CurVertex   = V1;
  //    if (VEmap(CurVertex).Extent() == 2) //end: two free edges
  //      break;
  //  }
  //  Start = Standard_False;
  //  continue;
  //}

  TopoVertex StartVertex, CurVertex, V1, V2;
  TopExp1::Vertices(StartEdge, StartVertex, CurVertex);
  TopoEdge CurEdge = StartEdge;
  Standard_Integer i;
  for (i = 1; i <= 2; i++)
  {
    for (;;)
    {
      const ShapeList& LE = VEmap.FindFromKey(CurVertex);
      if (LE.Extent() == 2) //end: two free edges or one closed free edge
        break;
      TopTools_ListIteratorOfListOfShape itl(LE);
      TopoEdge anEdge;
      for (; itl.More(); itl.Next())
      {
        anEdge = TopoDS::Edge(itl.Value());
        if (anEdge.IsSame(CurEdge))
          continue;
        if (EFmap.FindFromKey(anEdge).Extent() == 1) //another free edge found
          break;
      }
      //BB.Add(myStartWire, anEdge);
      MW.Add(anEdge);
      TopExp1::Vertices(anEdge, V1, V2);
      CurVertex = (V1.IsSame(CurVertex))? V2 : V1;
      CurEdge = anEdge;
      if (CurVertex.IsSame(StartVertex))
        break;
    }
    if (CurVertex.IsSame(StartVertex))
      break;
    CurVertex = StartVertex;
    CurEdge = StartEdge;
  }

  myStartWire = MW.Wire();
  myClosedSection = myStartWire.Closed();
}
*/

//=================================================================================================

static TopoWire GetUnifiedWire(const TopoWire&            theWire,
                                  ShapeUpgrade_UnifySameDomain& theUnifier)
{
  BRepLib_MakeWire       aWMaker;
  BRepTools_WireExplorer wexp(theWire);
  TopTools_MapOfShape    aGeneratedEdges;
  for (; wexp.More(); wexp.Next())
  {
    TopoShape                anEdge = wexp.Current();
    const ShapeList& aLS    = theUnifier.History()->Modified(anEdge);
    if (!aLS.IsEmpty())
    {
      TopTools_ListIteratorOfListOfShape anIt(aLS);
      for (; anIt.More(); anIt.Next())
      {
        const TopoShape& aShape = anIt.Value();
        // wire shouldn't contain duplicated generated edges
        if (aGeneratedEdges.Add(aShape))
          aWMaker.Add(TopoDS::Edge(aShape));
      }
    }
    else
    {
      // no change, put original edge
      aWMaker.Add(TopoDS::Edge(anEdge));
    }
  }
  return aWMaker.Wire();
}

//=================================================================================================

BRepOffsetAPI_MiddlePath::BRepOffsetAPI_MiddlePath(const TopoShape& aShape,
                                                   const TopoShape& StartShape,
                                                   const TopoShape& EndShape)
{
  ShapeUpgrade_UnifySameDomain Unifier(aShape);
  Unifier.Build();
  myInitialShape = Unifier.Shape();

  TopoWire aStartWire, anEndWire;
  if (StartShape.ShapeType() == TopAbs_FACE)
  {
    const TopoFace& StartFace = TopoDS::Face(StartShape);
    aStartWire                   = BRepTools1::OuterWire(StartFace);
  }
  else
    aStartWire = TopoDS::Wire(StartShape);

  if (EndShape.ShapeType() == TopAbs_FACE)
  {
    const TopoFace& EndFace = TopoDS::Face(EndShape);
    anEndWire                  = BRepTools1::OuterWire(EndFace);
  }
  else
    anEndWire = TopoDS::Wire(EndShape);

  myStartWire = GetUnifiedWire(aStartWire, Unifier);
  myEndWire   = GetUnifiedWire(anEndWire, Unifier);

  myClosedSection = myStartWire.Closed();
  myClosedRing    = myStartWire.IsSame(myEndWire);
}

//=================================================================================================

void BRepOffsetAPI_MiddlePath::Build(const Message_ProgressRange& /*theRange*/)
{
  TopTools_ListIteratorOfListOfShape itl;

  TopTools_SequenceOfShape                StartVertices;
  TopTools_MapOfShape                     EndVertices;
  TopTools_MapOfShape                     EndEdges;
  BRepOffsetAPI_SequenceOfSequenceOfShape SectionsEdges;

  BRepTools_WireExplorer   wexp(myStartWire);
  TopTools_SequenceOfShape EdgeSeq;
  for (; wexp.More(); wexp.Next())
  {
    StartVertices.Append(wexp.CurrentVertex());
    EdgeSeq.Append(wexp.Current());
  }
  if (!myClosedSection)
    StartVertices.Append(wexp.CurrentVertex());
  SectionsEdges.Append(EdgeSeq);

  for (wexp.Init(myEndWire); wexp.More(); wexp.Next())
  {
    EndVertices.Add(wexp.CurrentVertex());
    EndEdges.Add(wexp.Current());
  }
  if (!myClosedSection)
    EndVertices.Add(wexp.CurrentVertex());

  TopoDS_Iterator itw(myStartWire);
  for (; itw.More(); itw.Next())
    myStartWireEdges.Add(itw.Value());
  for (itw.Initialize(myEndWire); itw.More(); itw.Next())
    myEndWireEdges.Add(itw.Value());

  TopTools_IndexedDataMapOfShapeListOfShape VEmap;
  TopTools_IndexedDataMapOfShapeListOfShape EFmap;
  TopExp1::MapShapesAndAncestors(myInitialShape, TopAbs_VERTEX, TopAbs_EDGE, VEmap);
  TopExp1::MapShapesAndAncestors(myInitialShape, TopAbs_EDGE, TopAbs_FACE, EFmap);

  TopTools_MapOfShape CurVertices;

  Standard_Integer i, j, k;
  TopoEdge      anEdge;
  TopoVertex    V1, V2, NextVertex;
  // Initialization of <myPaths>
  for (i = 1; i <= StartVertices.Length(); i++)
  {
    TopTools_SequenceOfShape    Edges;
    const ShapeList& LE = VEmap.FindFromKey(StartVertices(i));
    for (itl.Initialize(LE); itl.More(); itl.Next())
    {
      anEdge = TopoDS::Edge(itl.Value());
      if (!myStartWireEdges.Contains(anEdge))
      {
        TopExp1::Vertices(anEdge, V1, V2, Standard_True);
        if (V1.IsSame(StartVertices(i)))
          CurVertices.Add(V2);
        else
        {
          anEdge.Reverse();
          CurVertices.Add(V1);
        }
        Edges.Append(anEdge);
        break;
      }
    }
    if (!Edges.IsEmpty())
      myPaths.Append(Edges);
    else
      return;
  }

  // Filling of "myPaths"
  ShapeList NextVertices;
  for (;;)
  {
    for (i = 1; i <= myPaths.Length(); i++)
    {
      const TopoShape& theShape = myPaths(i).Last();
      TopoEdge         theEdge;
      TopoVertex       theVertex;
      if (theShape.ShapeType() == TopAbs_EDGE)
      {
        theEdge   = TopoDS::Edge(theShape);
        theVertex = TopExp1::LastVertex(theEdge, Standard_True);
      }
      else // last segment of path was punctual
      {
        theEdge   = TopoDS::Edge(myPaths(i)(myPaths(i).Length() - 1));
        theVertex = TopoDS::Vertex(theShape);
      }

      if (EndVertices.Contains(theVertex))
        continue;
      const ShapeList& LE = VEmap.FindFromKey(theVertex);
      TopTools_MapOfShape         NextEdgeCandidates;
      for (itl.Initialize(LE); itl.More(); itl.Next())
      {
        anEdge = TopoDS::Edge(itl.Value());
        if (anEdge.IsSame(theEdge))
          continue;
        TopExp1::Vertices(anEdge, V1, V2, Standard_True);
        if (V1.IsSame(theVertex))
          NextVertex = V2;
        else
        {
          anEdge.Reverse();
          NextVertex = V1;
        }
        if (!CurVertices.Contains(NextVertex))
          NextEdgeCandidates.Add(anEdge);
      }
      if (!NextEdgeCandidates.IsEmpty())
      {
        if (NextEdgeCandidates.Extent() > 1)
          myPaths(i).Append(theVertex); // punctual segment of path
        else
        {
          TopTools_MapIteratorOfMapOfShape mapit(NextEdgeCandidates);
          anEdge = TopoDS::Edge(mapit.Key());
          myPaths(i).Append(anEdge);
          NextVertex = TopExp1::LastVertex(anEdge, Standard_True);
          NextVertices.Append(NextVertex);
        }
      }
    }
    if (NextVertices.IsEmpty())
      break;
    for (itl.Initialize(NextVertices); itl.More(); itl.Next())
      CurVertices.Add(itl.Value());
    NextVertices.Clear();
  }

  // Temporary
  /*
  TopoCompound aCompound, aCmp1;
  ShapeBuilder BB;
  BB.MakeCompound(aCompound);
  BB.MakeCompound(aCmp1);
  for (i = 1; i <= myPaths.Length(); i++)
  {
    TopoCompound aCmp;
    BB.MakeCompound(aCmp);
    for (j = 1; j <= myPaths(i).Length(); j++)
      BB.Add(aCmp, myPaths(i)(j));
    BB.Add(aCmp1, aCmp);
  }
  BB.Add(aCompound, aCmp1);

  myShape = aCompound;

  Done();
  return;
  */
  ////////////

  // Building of set of sections
  Standard_Integer NbE     = EdgeSeq.Length();
  Standard_Integer NbPaths = myPaths.Length();
  Standard_Integer NbVer   = myPaths.Length();
  if (myClosedSection)
    NbVer++;
  i = 1;
  for (;;)
  {
    for (j = 1; j <= EdgeSeq.Length(); j++)
      EdgeSeq(j).Nullify();

    Standard_Boolean ToInsertVertex = Standard_False;

    for (j = 2; j <= NbVer; j++)
    {
      if (!EdgeSeq(j - 1).IsNull())
        continue;

      // for the end of initial shape
      if (myPaths(j - 1).Length() < i)
      {
        TopoEdge  aE1     = TopoDS::Edge(myPaths(j - 1)(i - 1));
        TopoShape LastVer = TopExp1::LastVertex(aE1, Standard_True);
        myPaths(j - 1).Append(LastVer);
      }
      if (myPaths((j <= NbPaths) ? j : 1).Length() < i)
      {
        TopoEdge  aE2     = TopoDS::Edge(myPaths((j <= NbPaths) ? j : 1)(i - 1));
        TopoShape LastVer = TopExp1::LastVertex(aE2, Standard_True);
        myPaths((j <= NbPaths) ? j : 1).Append(LastVer);
      }
      //////////////////////////////

      if (ToInsertVertex)
      {
        if (myPaths(j - 1)(i).ShapeType() == TopAbs_EDGE)
        {
          TopoEdge  aE1  = TopoDS::Edge(myPaths(j - 1)(i));
          TopoShape fver = TopExp1::FirstVertex(aE1, Standard_True);
          myPaths(j - 1).InsertBefore(i, fver);
        }
        if (myPaths((j <= NbPaths) ? j : 1)(i).ShapeType() == TopAbs_EDGE)
        {
          TopoEdge  aE2  = TopoDS::Edge(myPaths((j <= NbPaths) ? j : 1)(i));
          TopoShape fver = TopExp1::FirstVertex(aE2, Standard_True);
          myPaths((j <= NbPaths) ? j : 1).InsertBefore(i, fver);
        }
        ToInsertVertex = Standard_False;
      }

      TopoEdge E1, E2;
      if (myPaths(j - 1)(i).ShapeType() == TopAbs_EDGE)
        E1 = TopoDS::Edge(myPaths(j - 1)(i));
      if (myPaths((j <= NbPaths) ? j : 1)(i).ShapeType() == TopAbs_EDGE)
        E2 = TopoDS::Edge(myPaths((j <= NbPaths) ? j : 1)(i));
      TopoEdge E12 = TopoDS::Edge(SectionsEdges(i)(j - 1));
      // Find the face on which (E1 or E2) and E12 lie
      TopoShape E1orE2 = (E1.IsNull()) ? E2 : E1;
      if (E1orE2.IsNull()) // both E1 and E2 are vertices =>
      {
        EdgeSeq(j - 1) = E12; // => proper edge is the edge of previous section between them
        continue;
      }
      const ShapeList& LF = EFmap.FindFromKey(E1orE2);
      TopoFace                 theFace;
      for (itl.Initialize(LF); itl.More(); itl.Next())
      {
        const TopoShape&                aFace = itl.Value();
        const ShapeList&        LF2   = EFmap.FindFromKey(E12);
        TopTools_ListIteratorOfListOfShape itl2(LF2);
        for (; itl2.More(); itl2.Next())
        {
          const TopoShape& aFace2 = itl2.Value();
          if (aFace.IsSame(aFace2))
          {
            theFace = TopoDS::Face(aFace);
            break;
          }
        }
        if (!theFace.IsNull())
          break;
      }

      TopoVertex PrevVertex =
        (E1.IsNull()) ? TopoDS::Vertex(myPaths(j - 1)(i)) : TopExp1::LastVertex(E1, Standard_True);
      TopoVertex CurVertex = (E2.IsNull()) ? TopoDS::Vertex(myPaths((j <= NbPaths) ? j : 1)(i))
                                              : TopExp1::LastVertex(E2, Standard_True);

      TopoEdge                 ProperEdge;
      const ShapeList& LE = VEmap.FindFromKey(PrevVertex);
      // Temporary
      // Standard_Integer LenList = LE.Extent();
      ///////////
      TopTools_IndexedMapOfShape EdgesOfTheFace;
      TopExp1::MapShapes(theFace, TopAbs_EDGE, EdgesOfTheFace);
      for (itl.Initialize(LE); itl.More(); itl.Next())
      {
        anEdge = TopoDS::Edge(itl.Value());
        TopExp1::Vertices(anEdge, V1, V2);
        if (((V1.IsSame(PrevVertex) && V2.IsSame(CurVertex))
             || (V1.IsSame(CurVertex) && V2.IsSame(PrevVertex)))
            && EdgesOfTheFace.Contains(anEdge) && // this condition is for a section of two edges
            !anEdge.IsSame(E1))                   // the last condition is for torus-like shape
        {
          ProperEdge = anEdge;
          break;
        }
      }

      if ((myPaths(j - 1)(i)).ShapeType() == TopAbs_VERTEX
          && (myPaths((j <= NbPaths) ? j : 1)(i)).ShapeType() == TopAbs_VERTEX)
      {
        EdgeSeq(j - 1) = ProperEdge;
        continue;
      }

      TopoVertex PrevPrevVer =
        (E1.IsNull()) ? PrevVertex : TopExp1::FirstVertex(E1, Standard_True);
      TopoVertex PrevCurVer = (E2.IsNull()) ? CurVertex : TopExp1::FirstVertex(E2, Standard_True);
      if (ProperEdge.IsNull()) // no connection between these two vertices
      {
        // Find the face on which E1, E2 and E12 lie
        // ToInsertVertex = Standard_False;
        ShapeList ListOneFace;
        ListOneFace.Append(theFace);

        if (E1.IsNull() || E2.IsNull())
        {
          if (E1.IsNull())
            E1 = TopoDS::Edge(myPaths(j - 1)(i - 1));
          if (E2.IsNull())
            E2 = TopoDS::Edge(myPaths((j <= NbPaths) ? j : 1)(i - 1));
          Standard_Real        fpar1, lpar1, fpar2, lpar2;
          Standard_Real        LastPar1, LastPar2;
          Handle(GeomCurve2d) PCurve1 = BRepInspector::CurveOnSurface(E1, theFace, fpar1, lpar1);
          Handle(GeomCurve2d) PCurve2 = BRepInspector::CurveOnSurface(E2, theFace, fpar2, lpar2);
          if (E1.Orientation() == TopAbs_FORWARD)
          {
            LastPar1 = lpar1;
          }
          else
          {
            LastPar1 = fpar1;
          }
          if (E2.Orientation() == TopAbs_FORWARD)
          {
            LastPar2 = lpar2;
          }
          else
          {
            LastPar2 = fpar2;
          }
          gp_Pnt2d             FirstPnt2d = PCurve1->Value(LastPar1);
          gp_Pnt2d             LastPnt2d  = PCurve2->Value(LastPar2);
          Handle(GeomSurface) theSurf    = BRepInspector::Surface(theFace);
          Handle(Geom2d_Line)  theLine    = GCE2d_MakeLine(FirstPnt2d, LastPnt2d);
          Standard_Real        len_ne     = FirstPnt2d.Distance(LastPnt2d);
          TopoEdge          NewEdge =
            BRepLib_MakeEdge(theLine, theSurf, PrevVertex, CurVertex, 0., len_ne);
          BRepLib::BuildCurve3d(NewEdge);
          EdgeSeq(j - 1) = NewEdge;
          EFmap.Add(NewEdge, ListOneFace);
        }
        else // E1 is edge
        {
          // Extract points 2d
          Standard_Real        fpar1, lpar1, fpar2, lpar2;
          Standard_Real        FirstPar1, LastPar1, FirstPar2, LastPar2;
          Handle(GeomCurve2d) PCurve1 = BRepInspector::CurveOnSurface(E1, theFace, fpar1, lpar1);
          Handle(GeomCurve2d) PCurve2 = BRepInspector::CurveOnSurface(E2, theFace, fpar2, lpar2);
          if (E1.Orientation() == TopAbs_FORWARD)
          {
            FirstPar1 = fpar1;
            LastPar1  = lpar1;
          }
          else
          {
            FirstPar1 = lpar1;
            LastPar1  = fpar1;
          }
          if (E2.Orientation() == TopAbs_FORWARD)
          {
            FirstPar2 = fpar2;
            LastPar2  = lpar2;
          }
          else
          {
            FirstPar2 = lpar2;
            LastPar2  = fpar2;
          }
          gp_Pnt2d             FirstPnt2d = PCurve1->Value(LastPar1);
          gp_Pnt2d             LastPnt2d  = PCurve2->Value(LastPar2);
          Handle(GeomSurface) theSurf    = BRepInspector::Surface(theFace);
          Handle(Geom2d_Line)  theLine    = GCE2d_MakeLine(FirstPnt2d, LastPnt2d);
          Standard_Real        len_ne     = FirstPnt2d.Distance(LastPnt2d);
          TopoEdge          NewEdge =
            BRepLib_MakeEdge(theLine, theSurf, PrevVertex, CurVertex, 0., len_ne);
          BRepLib::BuildCurve3d(NewEdge);
          gp_Pnt2d            PrevFirstPnt2d = PCurve1->Value(FirstPar1);
          gp_Pnt2d            PrevLastPnt2d  = PCurve2->Value(FirstPar2);
          Handle(Geom2d_Line) Line1          = GCE2d_MakeLine(PrevFirstPnt2d, LastPnt2d);
          Handle(Geom2d_Line) Line2          = GCE2d_MakeLine(FirstPnt2d, PrevLastPnt2d);
          Standard_Real       len_ne1        = PrevFirstPnt2d.Distance(LastPnt2d);
          TopoEdge         NewEdge1 =
            BRepLib_MakeEdge(Line1, theSurf, PrevPrevVer, CurVertex, 0., len_ne1);
          BRepLib::BuildCurve3d(NewEdge1);
          Standard_Real len_ne2 = FirstPnt2d.Distance(PrevLastPnt2d);
          TopoEdge   NewEdge2 =
            BRepLib_MakeEdge(Line2, theSurf, PrevVertex, PrevCurVer, 0., len_ne2);
          BRepLib::BuildCurve3d(NewEdge2);
          Standard_Boolean good_ne  = IsValidEdge(NewEdge, theFace);
          Standard_Boolean good_ne1 = IsValidEdge(NewEdge1, theFace);

          GeomAbs_CurveType type_E1 = TypeOfEdge(E1);
          GeomAbs_CurveType type_E2 = TypeOfEdge(E2);

          Standard_Integer ChooseEdge = 0;

          if (!good_ne || type_E1 != type_E2)
          {
            if (type_E1 == type_E2) //! good_ne
            {
              if (good_ne1)
                ChooseEdge = 1;
              else
                ChooseEdge = 2;
            }
            else // types are different
            {
              if (type_E1 == GeomAbs_Line)
                ChooseEdge = 1;
              else if (type_E2 == GeomAbs_Line)
                ChooseEdge = 2;
              else // to be developed later...
              {
              }
            }
          }

          if (ChooseEdge == 0)
          {
            EdgeSeq(j - 1) = NewEdge;
            EFmap.Add(NewEdge, ListOneFace);
          }
          else if (ChooseEdge == 1)
          {
            EdgeSeq(j - 1) = NewEdge1;
            EFmap.Add(NewEdge1, ListOneFace);
            for (k = 1; k < j - 1; k++)
              EdgeSeq(k).Nullify();
            for (k = 1; k <= j - 1; k++)
            {
              TopoEdge  aLastEdge    = TopoDS::Edge(myPaths(k)(i));
              TopoShape VertexAsEdge = TopExp1::FirstVertex(aLastEdge, Standard_True);
              myPaths(k).InsertBefore(i, VertexAsEdge);
            }
            j = 1; // start from beginning
          }
          else if (ChooseEdge == 2)
          {
            EdgeSeq(j - 1) = NewEdge2;
            EFmap.Add(NewEdge2, ListOneFace);
            ToInsertVertex = Standard_True;
          }
        } // else //E1 is edge
      } // if (ProperEdge.IsNull())
      else // connecting edge exists
      {
        /*
        if (ToInsertVertex)
        {
          myPaths(j-1).InsertBefore(i, PrevPrevVer);
          myPaths((j<=NbPaths)? j : 1).InsertBefore(i, PrevCurVer);
          EdgeSeq(j-1) = E12;
        }
        else
        */
        EdgeSeq(j - 1) = ProperEdge;
      }
    } // for (j = 2; j <= NbVer; j++)
    SectionsEdges.Append(EdgeSeq);

    // check for exit from for(;;)
    Standard_Integer NbEndEdges = 0;
    for (j = 1; j <= EdgeSeq.Length(); j++)
      if (EndEdges.Contains(EdgeSeq(j)))
        NbEndEdges++;
    if (NbEndEdges == NbE)
      break;

    i++;
  } // for (;;)

  // final phase: building of middle path
  Standard_Integer       NbSecFaces = SectionsEdges.Length();
  TopTools_Array1OfShape SecFaces(1, NbSecFaces);
  for (i = 1; i <= NbSecFaces; i++)
  {
    BRepLib_MakeWire MW;
    for (j = 1; j <= NbE; j++)
    {
      anEdge = TopoDS::Edge(SectionsEdges(i)(j));
      MW.Add(anEdge);
    }
    if (!myClosedSection)
    {
      TopExp1::Vertices(MW.Wire(), V1, V2);
      anEdge = BRepLib_MakeEdge(V2, V1);
      MW.Add(anEdge);
    }
    TopoWire      aWire = MW.Wire();
    BRepLib_MakeFace MF(aWire, Standard_True); // Only plane
    if (MF.IsDone())
      SecFaces(i) = MF.Face();
    else
      SecFaces(i) = aWire;
  }

  TColgp_Array1OfPnt Centers(1, NbSecFaces);
  for (i = 1; i <= NbSecFaces; i++)
  {
    GeometricProperties Properties;
    if (SecFaces(i).ShapeType() == TopAbs_FACE)
      BRepGProp::SurfaceProperties(SecFaces(i), Properties);
    else // wire
      BRepGProp::LinearProperties(SecFaces(i), Properties);

    Centers(i) = Properties.CentreOfMass();
  }

  TopTools_Array1OfShape MidEdges(1, NbSecFaces - 1);
  Standard_Real          LinTol = 1.e-5;
  Standard_Real          AngTol = 1.e-7;
  Point3d                 Pnt1, Pnt2;
  for (i = 1; i < NbSecFaces; i++)
  {
    GeomAbs_CurveType TypeOfMidEdge = GeomAbs_OtherCurve;
    for (j = 1; j <= myPaths.Length(); j++)
    {
      const TopoShape& aShape = myPaths(j)(i);
      if (aShape.ShapeType() == TopAbs_VERTEX)
      {
        TypeOfMidEdge = GeomAbs_OtherCurve;
        break;
      }
      anEdge                  = TopoDS::Edge(aShape);
      GeomAbs_CurveType aType = TypeOfEdge(anEdge);
      if (j == 1)
        TypeOfMidEdge = aType;
      else
      {
        if (aType != TypeOfMidEdge)
        {
          TypeOfMidEdge = GeomAbs_OtherCurve;
          break;
        }
      }
    }
    if (TypeOfMidEdge == GeomAbs_Line)
      MidEdges(i) = BRepLib_MakeEdge(Centers(i), Centers(i + 1));
    else if (TypeOfMidEdge == GeomAbs_Circle)
    {
      Axis3d           theAxis;
      Dir3d           theDir1, theDir2;
      Standard_Real    theAngle = 0.;
      Vector3d           theTangent;
      Standard_Boolean SimilarArcs = Standard_True;
      for (j = 1; j <= myPaths.Length(); j++)
      {
        anEdge = TopoDS::Edge(myPaths(j)(i));
        Standard_Real      fpar, lpar;
        Handle(GeomCurve3d) aCurve = BRepInspector::Curve(anEdge, fpar, lpar);
        if (aCurve->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
          aCurve = Handle(Geom_TrimmedCurve)::DownCast(aCurve)->BasisCurve();
        Pnt1                        = aCurve->Value(fpar);
        Pnt2                        = aCurve->Value(lpar);
        Handle(GeomCircle) aCircle = Handle(GeomCircle)::DownCast(aCurve);
        gp_Circ             aCirc   = aCircle->Circ();
        if (j == 1)
        {
          theAxis                = aCirc.Axis();
          theDir1                = Vector3d(aCirc.Location(), Pnt1);
          theDir2                = Vector3d(aCirc.Location(), Pnt2);
          theAngle               = lpar - fpar;
          Standard_Real theParam = (anEdge.Orientation() == TopAbs_FORWARD) ? fpar : lpar;
          aCurve->D1(theParam, Pnt1, theTangent);
          if (anEdge.Orientation() == TopAbs_REVERSED)
            theTangent.Reverse();
        }
        else
        {
          Axis3d anAxis = aCirc.Axis();
          gp_Lin aLin(anAxis);
          if (!aLin.Contains(theAxis.Location(), LinTol) || !anAxis.IsParallel(theAxis, AngTol))
          {
            SimilarArcs = Standard_False;
            break;
          }
          Dir3d aDir1 = Vector3d(aCirc.Location(), Pnt1);
          Dir3d aDir2 = Vector3d(aCirc.Location(), Pnt2);
          if (!((aDir1.IsEqual(theDir1, AngTol) && aDir2.IsEqual(theDir2, AngTol))
                || (aDir1.IsEqual(theDir2, AngTol) && aDir2.IsEqual(theDir1, AngTol))))
          {
            SimilarArcs = Standard_False;
            break;
          }
        }
      }
      if (SimilarArcs)
      {
        gp_XYZ        AxisLoc   = theAxis.Location().XYZ();
        gp_XYZ        AxisDir   = theAxis.Direction().XYZ();
        Standard_Real Parameter = (Centers(i).XYZ() - AxisLoc) * AxisDir;
        Point3d        theCenterOfCirc(AxisLoc + Parameter * AxisDir);

        Vector3d Vec1(theCenterOfCirc, Centers(i));
        Vector3d Vec2(theCenterOfCirc, Centers(i + 1));
        /*
        Dir3d VecProd = Vec1 ^ Vec2;
        if (theAxis.Direction() * VecProd < 0.)
          theAxis.Reverse();
        */

        Standard_Real anAngle = Vec1.AngleWithRef(Vec2, theAxis.Direction());
        if (anAngle < 0.)
          anAngle += 2. * M_PI;
        if (Abs(anAngle - theAngle) > AngTol)
          theAxis.Reverse();
        Frame3d              theAx2(theCenterOfCirc, theAxis.Direction(), Vec1);
        Handle(GeomCircle) theCircle = GC_MakeCircle(theAx2, Vec1.Magnitude());
        Vector3d              aTangent;
        theCircle->D1(0., Pnt1, aTangent);
        if (aTangent * theTangent < 0.)
        {
          theAxis.Reverse();
          theAx2    = Frame3d(theCenterOfCirc, theAxis.Direction(), Vec1);
          theCircle = GC_MakeCircle(theAx2, Vec1.Magnitude());
        }
        BRepLib_MakeEdge aME(theCircle, 0., theAngle);
        aME.Build();

        MidEdges(i) = aME.IsDone() ? aME.Shape() : TopoEdge();
      }
    }
  }

  // Build missed edges
  for (i = 1; i < NbSecFaces; i++)
  {
    if (MidEdges(i).IsNull())
    {
      for (j = i + 1; j < NbSecFaces; j++)
      {
        if (!MidEdges(j).IsNull())
          break;
      }
      // from i to j-1 all edges are null
      Handle(TColgp_HArray1OfPnt)      thePoints = new TColgp_HArray1OfPnt(1, j - i + 1);
      TColgp_Array1OfVec               theTangents(1, j - i + 1);
      Handle(TColStd_HArray1OfBoolean) theFlags = new TColStd_HArray1OfBoolean(1, j - i + 1);
      for (k = i; k <= j; k++)
        thePoints->SetValue(k - i + 1, Centers(k));
      for (k = i; k <= j; k++)
      {
        TColgp_SequenceOfPnt PntSeq;
        for (Standard_Integer indp = 1; indp <= myPaths.Length(); indp++)
        {
          Vector3d aTangent;
          if (k == i)
          {
            if (myPaths(indp)(k).ShapeType() == TopAbs_VERTEX)
              continue;
            aTangent = TangentOfEdge(myPaths(indp)(k), Standard_True); // at begin
          }
          else if (k == j)
          {
            if (myPaths(indp)(k - 1).ShapeType() == TopAbs_VERTEX)
              continue;
            aTangent = TangentOfEdge(myPaths(indp)(k - 1), Standard_False); // at end
          }
          else
          {
            if (myPaths(indp)(k - 1).ShapeType() == TopAbs_VERTEX
                || myPaths(indp)(k).ShapeType() == TopAbs_VERTEX)
              continue;
            Vector3d Tangent1 = TangentOfEdge(myPaths(indp)(k - 1), Standard_False); // at end
            Vector3d Tangent2 = TangentOfEdge(myPaths(indp)(k), Standard_True);      // at begin
            aTangent        = Tangent1 + Tangent2;
          }
          aTangent.Normalize();
          Point3d aPnt(aTangent.XYZ());
          PntSeq.Append(aPnt);
        }
        TColgp_Array1OfPnt PntArray(1, PntSeq.Length());
        for (Standard_Integer ip = 1; ip <= PntSeq.Length(); ip++)
          PntArray(ip) = PntSeq(ip);
        Point3d        theBary;
        Dir3d        xdir, ydir;
        Standard_Real xgap, ygap, zgap;
        GeomLib::Inertia(PntArray, theBary, xdir, ydir, xgap, ygap, zgap);
        Vector3d theTangent(theBary.XYZ());
        theTangents(k - i + 1) = theTangent;
      }
      theFlags->Init(Standard_True);

      GeomAPI_Interpolate Interpol(thePoints, Standard_False, LinTol);
      Interpol.Load(theTangents, theFlags);
      Interpol.Perform();
      if (!Interpol.IsDone())
      {
        std::cout << std::endl << "Interpolation failed" << std::endl;
      }
      Handle(GeomCurve3d) InterCurve(Interpol.Curve());
      MidEdges(i) = BRepLib_MakeEdge(InterCurve);
      i           = j;
    }
  }

  BRepLib_MakeWire MakeFinalWire;
  for (i = 1; i < NbSecFaces; i++)
    if (!MidEdges(i).IsNull())
      MakeFinalWire.Add(TopoDS::Edge(MidEdges(i)));

  TopoWire FinalWire = MakeFinalWire.Wire();
  myShape               = MakeFinalWire.Wire();

  // Temporary
  /*
  TopoCompound aCompound, aCmp1, aCmp2;
  ShapeBuilder BB;
  BB.MakeCompound(aCompound);
  BB.MakeCompound(aCmp1);
  BB.MakeCompound(aCmp2);
  for (i = 1; i <= myPaths.Length(); i++)
  {
    TopoCompound aCmp;
    BB.MakeCompound(aCmp);
    for (j = 1; j <= myPaths(i).Length(); j++)
      BB.Add(aCmp, myPaths(i)(j));
    BB.Add(aCmp1, aCmp);
  }
  for (i = 1; i <= SectionsEdges.Length(); i++)
  {
    TopoWire aWire;
    BB.MakeWire(aWire);
    for (j = 1; j <= SectionsEdges(i).Length(); j++)
      BB.Add(aWire, SectionsEdges(i)(j));
    BB.Add(aCmp2, aWire);
  }
  BB.Add(aCompound, aCmp1);
  BB.Add(aCompound, aCmp2);
  BB.Add(aCompound, FinalWire);

  myShape = aCompound;
  */
  ////////////

  Done();
}
