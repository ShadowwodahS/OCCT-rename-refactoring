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

//  Modified by skv - Wed Dec 24 18:08:39 2003 OCC4455

#include <BRepOffset_Inter2d.hxx>

#include <BOPTools_AlgoTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Curve2d.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgo_AsDes.hxx>
#include <BRepAlgo_Image.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepOffset_Analyse.hxx>
#include <BRepOffset_Offset.hxx>
#include <BRepOffset_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2dConvert_CompCurveToBSplineCurve.hxx>
#include <Geom2dInt_GInter.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomLib.hxx>
#include <GeomProjLib.hxx>
#include <gp_Pnt.hxx>
#include <IntRes2d_IntersectionPoint.hxx>
#include <IntRes2d_IntersectionSegment.hxx>
#include <IntTools_Tools.hxx>
#include <Precision.hxx>
#include <TColGeom2d_SequenceOfCurve.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

#include <stdio.h>
#ifdef DRAW
  #include <DBRep.hxx>
  #include <Geom2d_BoundedCurve.hxx>
  #include <Geom_BoundedSurface.hxx>
  #include <Geom_BoundedCurve.hxx>
  #include <BRep_CurveOnSurface.hxx>
  #include <Geom_Surface.hxx>
Standard_Boolean        Inter2dAffichInt2d;
static Standard_Integer NbF2d         = 0;
static Standard_Integer NbE2d         = 0;
static Standard_Integer NbNewVertices = 0;
#endif

//=================================================================================================

static TopoVertex CommonVertex(TopoEdge& E1, TopoEdge& E2)
{
  TopoVertex V1[2], V2[2], V;
  //
  TopExp1::Vertices(E1, V1[0], V1[1], Standard_True);
  TopExp1::Vertices(E2, V2[0], V2[1], Standard_True);
  // The first edge is the current one, the second edge is the next one.
  // We check last vertex of the first edge first.
  if (V1[1].IsSame(V2[0]) || V1[1].IsSame(V2[1]))
    return V1[1];
  if (V1[0].IsSame(V2[0]) || V1[0].IsSame(V2[1]))
    return V1[0];
  //
  return V;
}

static Standard_Integer DefineClosedness(const TopoFace& theFace)
{
  ShapeExplorer anExplo(theFace, TopAbs_EDGE);
  for (; anExplo.More(); anExplo.Next())
  {
    const TopoEdge& anEdge = TopoDS::Edge(anExplo.Current());
    if (BRepTools1::IsReallyClosed(anEdge, theFace))
    {
      Standard_Real        fpar, lpar;
      Handle(GeomCurve2d) aPCurve     = BRepInspector::CurveOnSurface(anEdge, theFace, fpar, lpar);
      gp_Vec2d             aTangent    = aPCurve->DN(fpar, 1);
      Standard_Real        aCrossProd1 = aTangent ^ gp1::DX2d();
      Standard_Real        aCrossProd2 = aTangent ^ gp1::DY2d();
      if (Abs(aCrossProd2) < Abs(aCrossProd1)) // pcurve is parallel to OY
        return 1;
      else
        return 2;
    }
  }

  return 0;
}

static void GetEdgesOrientedInFace(const TopoShape&           theShape,
                                   const TopoFace&            theFace,
                                   const Handle(BRepAlgo_AsDes)& theAsDes,
                                   TopTools_SequenceOfShape&     theSeqEdges)
{
  const ShapeList& aEdges = theAsDes->Descendant(theFace);

  ShapeExplorer anExplo(theShape, TopAbs_EDGE);
  for (; anExplo.More(); anExplo.Next())
  {
    const TopoShape&                anEdge = anExplo.Current();
    TopTools_ListIteratorOfListOfShape itl(aEdges);
    for (; itl.More(); itl.Next())
    {
      const TopoShape& anEdgeInFace = itl.Value();
      if (anEdgeInFace.IsSame(anEdge))
      {
        theSeqEdges.Append(anEdgeInFace);
        break;
      }
    }
  }

  if (theSeqEdges.Length() == 1)
    return;

  TopTools_IndexedDataMapOfShapeListOfShape aVEmap;
  for (Standard_Integer ii = 1; ii <= theSeqEdges.Length(); ii++)
    TopExp1::MapShapesAndAncestors(theSeqEdges(ii), TopAbs_VERTEX, TopAbs_EDGE, aVEmap);

  TopoVertex aFirstVertex;
  TopoEdge   aFirstEdge;
  for (Standard_Integer ii = 1; ii <= aVEmap.Extent(); ii++)
  {
    const TopoVertex&        aVertex = TopoDS::Vertex(aVEmap.FindKey(ii));
    const ShapeList& aElist  = aVEmap(ii);
    if (aElist.Extent() == 1)
    {
      const TopoEdge& anEdge = TopoDS::Edge(aElist.First());
      TopoVertex      aV1, aV2;
      TopExp1::Vertices(anEdge, aV1, aV2, Standard_True); // with orientation
      if (aV1.IsSame(aVertex))
      {
        aFirstVertex = aVertex;
        aFirstEdge   = anEdge;
        break;
      }
    }
  }

  if (aFirstEdge.IsNull()) // closed set of edges
  {
    // Standard_Real aPeriod = 0.;
    Standard_Integer IndCoord = DefineClosedness(theFace);
    /*
    BRepAdaptor_Surface aBAsurf (theFace, Standard_False);
    if (IndCoord == 1)
      aPeriod = aBAsurf.LastUParameter() - aBAsurf.FirstUParameter();
    else if (IndCoord == 2)
      aPeriod = aBAsurf.LastVParameter() - aBAsurf.FirstVParameter();
    */

    if (IndCoord != 0)
    {
      Standard_Real aMaxDelta = 0.;
      for (Standard_Integer ii = 1; ii <= aVEmap.Extent(); ii++)
      {
        const TopoVertex&        aVertex = TopoDS::Vertex(aVEmap.FindKey(ii));
        const ShapeList& aElist  = aVEmap(ii);
        const TopoEdge&          anEdge1 = TopoDS::Edge(aElist.First());
        const TopoEdge&          anEdge2 = TopoDS::Edge(aElist.Last());
        Standard_Real               aParam1 = BRepInspector::Parameter(aVertex, anEdge1);
        Standard_Real               aParam2 = BRepInspector::Parameter(aVertex, anEdge2);
        BRepAdaptor_Curve2d         aBAcurve1(anEdge1, theFace);
        BRepAdaptor_Curve2d         aBAcurve2(anEdge2, theFace);
        gp_Pnt2d                    aPnt1  = aBAcurve1.Value(aParam1);
        gp_Pnt2d                    aPnt2  = aBAcurve2.Value(aParam2);
        Standard_Real               aDelta = Abs(aPnt1.Coord(IndCoord) - aPnt2.Coord(IndCoord));
        if (aDelta > aMaxDelta)
        {
          aMaxDelta    = aDelta;
          aFirstVertex = aVertex;
        }
      }
      const ShapeList&        aElist = aVEmap.FindFromKey(aFirstVertex);
      TopTools_ListIteratorOfListOfShape itl(aElist);
      for (; itl.More(); itl.Next())
      {
        const TopoEdge& anEdge = TopoDS::Edge(itl.Value());
        TopoVertex      aV1, aV2;
        TopExp1::Vertices(anEdge, aV1, aV2, Standard_True); // with orientation
        if (aV1.IsSame(aFirstVertex))
        {
          aFirstEdge = anEdge;
          break;
        }
      }
    }
  }

  Standard_Integer aNbEdges = theSeqEdges.Length();
  theSeqEdges.Clear();
  theSeqEdges.Append(aFirstEdge);
  TopoEdge anEdge = aFirstEdge;
  for (;;)
  {
    TopoVertex aLastVertex = TopExp1::LastVertex(anEdge, Standard_True); // with orientation
    if (aLastVertex.IsSame(aFirstVertex))
      break;

    const ShapeList& aElist = aVEmap.FindFromKey(aLastVertex);
    if (aElist.Extent() == 1)
      break;

    if (aElist.First().IsSame(anEdge))
      anEdge = TopoDS::Edge(aElist.Last());
    else
      anEdge = TopoDS::Edge(aElist.First());

    theSeqEdges.Append(anEdge);
    if (theSeqEdges.Length() == aNbEdges)
      break;
  }
}

//=======================================================================
// function : Store
// purpose  : Store the vertices <theLV> into AsDes for the edge <theEdge>.
//           The vertices are added despite of the coincidence with
//           already added vertices. When all vertices for all edges
//           are added the coinciding chains of vertices should be fused
//           using FuseVertices() method.
//=======================================================================
static void Store(const TopoEdge&                         theEdge,
                  const ShapeList&                theLV,
                  const Standard_Real                        theTol,
                  const Standard_Boolean                     IsToUpdate,
                  Handle(BRepAlgo_AsDes)                     theAsDes2d,
                  TopTools_IndexedDataMapOfShapeListOfShape& theDMVV)
{
  // Update vertices
  TopTools_ListIteratorOfListOfShape aIt(theLV);
  for (; aIt.More(); aIt.Next())
  {
    const TopoVertex& aV = TopoDS::Vertex(aIt.Value());
    ShapeBuilder().UpdateVertex(aV, theTol);
  }

  // Get vertices already added to the edge and check the distances to the new ones
  const ShapeList& aLVEx = theAsDes2d->Descendant(theEdge);
  if (!IsToUpdate && aLVEx.IsEmpty())
  {
    if (theLV.Extent())
      theAsDes2d->Add(theEdge, theLV);
    return;
  }
  //
  GeomAPI_ProjectPointOnCurve aProjPC;
  Standard_Real               aTolE = 0.0;
  if (IsToUpdate)
  {
    Standard_Real             aT1, aT2;
    const Handle(GeomCurve3d)& aC = BRepInspector::Curve(theEdge, aT1, aT2);
    aProjPC.Init(aC, aT1, aT2);
    aTolE = BRepInspector::Tolerance(theEdge);
  }
  //
  TopTools_MapOfShape aMV;
  for (aIt.Init(theLV); aIt.More(); aIt.Next())
  {
    const TopoVertex& aV = TopoDS::Vertex(aIt.Value());
    if (!aMV.Add(aV))
    {
      continue;
    }
    //
    const Point3d&       aP   = BRepInspector::Pnt(aV);
    const Standard_Real aTol = BRepInspector::Tolerance(aV);
    //
    ShapeList               aLVC;
    TopTools_ListIteratorOfListOfShape aItEx(aLVEx);
    for (; aItEx.More(); aItEx.Next())
    {
      const TopoVertex& aVEx = TopoDS::Vertex(aItEx.Value());
      if (aV.IsSame(aVEx))
      {
        break;
      }
      const Point3d&       aPEx    = BRepInspector::Pnt(aVEx);
      const Standard_Real aTolVEx = BRepInspector::Tolerance(aVEx);
      if (aP.IsEqual(aPEx, aTol + aTolVEx))
      {
        aLVC.Append(aVEx);
      }
    }
    //
    if (aItEx.More())
    {
      continue;
    }
    //
    if (IsToUpdate)
    {
      // get parameter of the vertex on the edge
      aProjPC.Perform(aP);
      if (!aProjPC.NbPoints())
      {
        continue;
      }
      //
      if (aProjPC.LowerDistance() > aTol + aTolE)
      {
        continue;
      }
      //
      Standard_Real aT          = aProjPC.LowerDistanceParameter();
      TopoShape  aLocalShape = aV.Oriented(TopAbs_INTERNAL);
      ShapeBuilder().UpdateVertex(TopoDS::Vertex(aLocalShape), aT, theEdge, aTol);
    }
    //
    if (aLVC.Extent())
    {
      TopTools_ListIteratorOfListOfShape aItLV(aLVC);
      for (; aItLV.More(); aItLV.Next())
      {
        const TopoShape&   aVC = aItLV.Value();
        ShapeList* pLV = theDMVV.ChangeSeek(aVC);
        if (!pLV)
        {
          pLV = &theDMVV(theDMVV.Add(aVC, ShapeList()));
        }
        pLV->Append(aV);
      }
      //
      ShapeList* pLV = theDMVV.ChangeSeek(aV);
      if (!pLV)
      {
        pLV = &theDMVV(theDMVV.Add(aV, ShapeList()));
      }
      pLV->Append(aLVC);
    }
    theAsDes2d->Add(theEdge, aV);
  }
}

//=======================================================================
// function : Store
// purpose  : Store the intersection vertices between two edges into AsDes
//=======================================================================
static void Store(const TopoEdge&                         theE1,
                  const TopoEdge&                         theE2,
                  const ShapeList&                theLV1,
                  const ShapeList&                theLV2,
                  const Standard_Real                        theTol,
                  Handle(BRepAlgo_AsDes)                     theAsDes2d,
                  TopTools_IndexedDataMapOfShapeListOfShape& theDMVV)
{
  for (Standard_Integer i = 0; i < 2; ++i)
  {
    const TopoEdge&          aE  = !i ? theE1 : theE2;
    const ShapeList& aLV = !i ? theLV1 : theLV2;
    Store(aE, aLV, theTol, Standard_False, theAsDes2d, theDMVV);
  }
}

//=================================================================================================

static void EdgeInter(const TopoFace&                         F,
                      const BRepAdaptor_Surface&                 BAsurf,
                      const TopoEdge&                         E1,
                      const TopoEdge&                         E2,
                      const Handle(BRepAlgo_AsDes)&              AsDes,
                      Standard_Real                              Tol,
                      Standard_Boolean                           WithOri,
                      TopTools_IndexedDataMapOfShapeListOfShape& aDMVV)
{
#ifdef DRAW
  if (Inter2dAffichInt2d)
  {
    char name[256];
    sprintf(name, "E2d_%d_%d", NbF2d, NbE2d++);
    DBRep1::Set(name, E1);
    sprintf(name, "E2d_%d_%d", NbF2d, NbE2d++);
    DBRep1::Set(name, E2);
  }
#endif

  if (E1.IsSame(E2))
    return;

  Standard_Real    f[3], l[3];
  Standard_Real    TolDub = 1.e-7;
  Standard_Integer i;

  BRepInspector::Range(E1, f[1], l[1]);
  BRepInspector::Range(E2, f[2], l[2]);

  BRepAdaptor_Curve CE1(E1, F);
  BRepAdaptor_Curve CE2(E2, F);

  TopoEdge EI[3];
  EI[1] = E1;
  EI[2] = E2;
  ShapeList LV1;
  ShapeList LV2;
  ShapeBuilder         B;

  TopoVertex CV;
  if (!TopExp1::CommonVertex(E1, E2, CV))
  {
    BRepLib1::BuildCurve3d(E1);
    BRepLib1::BuildCurve3d(E2);

    Standard_Real TolSum = BRepInspector::Tolerance(E1) + BRepInspector::Tolerance(E2);
    TolSum               = Max(TolSum, 1.e-5);

    TColgp_SequenceOfPnt   ResPoints;
    TColStd_SequenceOfReal ResParamsOnE1, ResParamsOnE2;
    Point3d                 DegPoint;
    Standard_Boolean       WithDegen = BRepInspector::Degenerated(E1) || BRepInspector::Degenerated(E2);

    if (WithDegen)
    {
      Standard_Integer ideg = (BRepInspector::Degenerated(E1)) ? 1 : 2;
      TopoDS_Iterator  iter(EI[ideg]);
      if (iter.More())
      {
        const TopoVertex& vdeg = TopoDS::Vertex(iter.Value());
        DegPoint                  = BRepInspector::Pnt(vdeg);
      }
      else
      {
        BRepAdaptor_Curve CEdeg(EI[ideg], F);
        DegPoint = CEdeg.Value(CEdeg.FirstParameter());
      }
    }
    //
    Handle(GeomCurve2d) pcurve1 = BRepInspector::CurveOnSurface(E1, F, f[1], l[1]);
    Handle(GeomCurve2d) pcurve2 = BRepInspector::CurveOnSurface(E2, F, f[2], l[2]);
    Geom2dAdaptor_Curve  GAC1(pcurve1, f[1], l[1]);
    Geom2dAdaptor_Curve  GAC2(pcurve2, f[2], l[2]);
    Geom2dInt_GInter     Inter2d(GAC1, GAC2, TolDub, TolDub);
    for (i = 1; i <= Inter2d.NbPoints(); i++)
    {
      Point3d P3d;
      if (WithDegen)
        P3d = DegPoint;
      else
      {
        gp_Pnt2d P2d = Inter2d.Point(i).Value();
        P3d          = BAsurf.Value(P2d.X(), P2d.Y());
      }
      ResPoints.Append(P3d);
      ResParamsOnE1.Append(Inter2d.Point(i).ParamOnFirst());
      ResParamsOnE2.Append(Inter2d.Point(i).ParamOnSecond());
    }

    for (i = 1; i <= ResPoints.Length(); i++)
    {
      Standard_Real aT1 = ResParamsOnE1(i); // ponc1.Parameter();
      Standard_Real aT2 = ResParamsOnE2(i); // ponc2.Parameter();
      if (Precision1::IsInfinite(aT1) || Precision1::IsInfinite(aT2))
      {
#ifdef OCCT_DEBUG
        std::cout << "Inter2d : Solution rejected due to infinite parameter" << std::endl;
#endif
        continue;
      }

      Point3d        P          = ResPoints(i); // ponc1.Value();
      TopoVertex aNewVertex = BRepLib_MakeVertex(P);
      aNewVertex.Orientation(TopAbs_INTERNAL);
      B.UpdateVertex(aNewVertex, aT1, E1, Tol);
      B.UpdateVertex(aNewVertex, aT2, E2, Tol);
      Point3d        P1 = CE1.Value(aT1);
      Point3d        P2 = CE2.Value(aT2);
      Standard_Real dist1, dist2, dist3;
      dist1 = P1.Distance(P);
      dist2 = P2.Distance(P);
      dist3 = P1.Distance(P2);
      dist1 = Max(dist1, dist2);
      dist1 = Max(dist1, dist3);
      B.UpdateVertex(aNewVertex, dist1);

#ifdef OCCT_DEBUG
      if (aT1 < f[1] - Tol || aT1 > l[1] + Tol)
      {
        std::cout << "out of limit" << std::endl;
        std::cout << "aT1 = " << aT1 << ", f[1] = " << f[1] << ", l[1] = " << l[1] << std::endl;
      }
      if (aT2 < f[2] - Tol || aT2 > l[2] + Tol)
      {
        std::cout << "out of limit" << std::endl;
        std::cout << "aT2 = " << aT2 << ", f[2] = " << f[2] << ", l[2] = " << l[2] << std::endl;
      }
      Standard_Real MilTol2 = 1000 * Tol * Tol;
      if (P1.SquareDistance(P) > MilTol2 || P2.SquareDistance(P) > MilTol2
          || P1.Distance(P2) > 2. * Tol)
      {
        std::cout << "Inter2d : Solution rejected " << std::endl;
        std::cout << "P  = " << P.X() << " " << P.Y() << " " << P.Z() << std::endl;
        std::cout << "P1 = " << P1.X() << " " << P1.Y() << " " << P1.Z() << std::endl;
        std::cout << "P2 = " << P2.X() << " " << P2.Y() << " " << P2.Z() << std::endl;
        std::cout << "MaxDist = " << dist1 << std::endl;
      }
#endif
      // define the orientation of a new vertex
      TopAbs_Orientation OO1 = TopAbs_REVERSED;
      TopAbs_Orientation OO2 = TopAbs_REVERSED;
      if (WithOri)
      {
        BRepAdaptor_Curve2d PCE1(E1, F);
        BRepAdaptor_Curve2d PCE2(E2, F);
        gp_Pnt2d            P2d1, P2d2;
        gp_Vec2d            V1, V2, V1or, V2or;
        PCE1.D1(aT1, P2d1, V1);
        PCE2.D1(aT2, P2d2, V2);
        V1or = V1;
        V2or = V2;
        if (E1.Orientation() == TopAbs_REVERSED)
          V1or.Reverse();
        if (E2.Orientation() == TopAbs_REVERSED)
          V2or.Reverse();
        Standard_Real CrossProd = V2or ^ V1;
#ifdef OCCT_DEBUG
        if (Abs(CrossProd) <= gp1::Resolution())
          std::cout << std::endl << "CrossProd = " << CrossProd << std::endl;
#endif
        if (CrossProd > 0.)
          OO1 = TopAbs_FORWARD;
        CrossProd = V1or ^ V2;
        if (CrossProd > 0.)
          OO2 = TopAbs_FORWARD;
      }
      LV1.Append(aNewVertex.Oriented(OO1));
      LV2.Append(aNewVertex.Oriented(OO2));
    }
  }

  //----------------------------------
  // Test at end.
  //---------------------------------
  Standard_Real U1, U2;
  Standard_Real TolConf = Tol;
  TopoVertex V1[2], V2[2];
  TopExp1::Vertices(E1, V1[0], V1[1]);
  TopExp1::Vertices(E2, V2[0], V2[1]);

  Standard_Integer j;
  for (j = 0; j < 2; j++)
  {
    if (V1[j].IsNull())
      continue;
    for (Standard_Integer k = 0; k < 2; k++)
    {
      if (V2[k].IsNull())
        continue;
      if (V1[j].IsSame(V2[k]))
      {
        if (AsDes->HasAscendant(V1[j]))
        {
          continue;
        }
      }
      //
      Point3d        P1   = BRepInspector::Pnt(V1[j]);
      Point3d        P2   = BRepInspector::Pnt(V2[k]);
      Standard_Real Dist = P1.Distance(P2);
      if (Dist < TolConf)
      {
        Standard_Real aTol = Max(BRepInspector::Tolerance(V1[j]), BRepInspector::Tolerance(V2[k]));
        TopoVertex V    = BRepLib_MakeVertex(P1);
        U1                 = (j == 0) ? f[1] : l[1];
        U2                 = (k == 0) ? f[2] : l[2];
        //
        TopoShape aLocalShape = V.Oriented(TopAbs_INTERNAL);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape), U1, E1, aTol);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape), U2, E2, aTol);
        //
        LV1.Prepend(V.Oriented(V1[j].Orientation()));
        LV2.Prepend(V.Oriented(V2[k].Orientation()));
      }
    }
  }

  Standard_Boolean AffichPurge = Standard_False;

  if (!LV1.IsEmpty())
  {
    //----------------------------------
    // Remove all vertices.
    // There can be doubles
    //----------------------------------
    TopTools_ListIteratorOfListOfShape it1LV1, it1LV2, it2LV1;
    Point3d                             P1, P2;
    Standard_Boolean                   Purge = Standard_True;

    while (Purge)
    {
      i     = 1;
      Purge = Standard_False;
      for (it1LV1.Initialize(LV1), it1LV2.Initialize(LV2); it1LV1.More();
           it1LV1.Next(), it1LV2.Next())
      {
        j = 1;
        it2LV1.Initialize(LV1);
        while (j < i)
        {
          P1 = BRepInspector::Pnt(TopoDS::Vertex(it1LV1.Value()));
          P2 = BRepInspector::Pnt(TopoDS::Vertex(it2LV1.Value()));
          //  Modified by skv - Thu Jan 22 18:19:04 2004 OCC4455 Begin
          //           if (P1.IsEqual(P2,10*Tol)) {
          Standard_Real aTol;

          aTol = Max(BRepInspector::Tolerance(TopoDS::Vertex(it1LV1.Value())),
                     BRepInspector::Tolerance(TopoDS::Vertex(it2LV1.Value())));
          if (P1.IsEqual(P2, aTol))
          {
            //  Modified by skv - Thu Jan 22 18:19:05 2004 OCC4455 End
            LV1.Remove(it1LV1);
            LV2.Remove(it1LV2);
            if (AffichPurge)
              std::cout << "Doubles removed in EdgeInter." << std::endl;
            Purge = Standard_True;
            break;
          }
          j++;
          it2LV1.Next();
        }
        if (Purge)
          break;
        i++;
      }
    }
    //---------------------------------
    // Vertex storage in DS.
    //---------------------------------
    Standard_Real TolStore = BRepInspector::Tolerance(E1) + BRepInspector::Tolerance(E2);
    TolStore               = Max(TolStore, Tol);
    Store(E1, E2, LV1, LV2, TolStore, AsDes, aDMVV);
  }
}

//=================================================================================================

static void RefEdgeInter(const TopoFace&                         F,
                         const BRepAdaptor_Surface&                 BAsurf,
                         const TopoEdge&                         E1,
                         const TopoEdge&                         E2,
                         const TopAbs_Orientation                   theOr1,
                         const TopAbs_Orientation                   theOr2,
                         const Handle(BRepAlgo_AsDes)&              AsDes,
                         Standard_Real                              Tol,
                         Standard_Boolean                           WithOri,
                         const TopoVertex&                       theVref,
                         ShapeImage&                            theImageVV,
                         TopTools_IndexedDataMapOfShapeListOfShape& aDMVV,
                         Standard_Boolean&                          theCoincide)
{
#ifdef DRAW
  if (Inter2dAffichInt2d)
  {
    char name[256];
    sprintf(name, "E2d_%d_%d", NbF2d, NbE2d++);
    DBRep1::Set(name, E1);
    sprintf(name, "E2d_%d_%d", NbF2d, NbE2d++);
    DBRep1::Set(name, E2);
  }
#endif
  //
  theCoincide = Standard_False;
  //
  if (E1.IsSame(E2))
    return;

  Standard_Real    f[3], l[3];
  Standard_Real    TolDub = 1.e-7, TolLL = 0.0;
  Standard_Integer i;

  // BRepInspector::Range(E1, f[1], l[1]);
  // BRepInspector::Range(E2, f[2], l[2]);

  BRepAdaptor_Curve CE1(E1, F);
  BRepAdaptor_Curve CE2(E2, F);

  TopoEdge EI[3];
  EI[1] = E1;
  EI[2] = E2;
  ShapeList LV1;
  ShapeList LV2;
  ShapeBuilder         B;

  BRepLib1::BuildCurve3d(E1);
  BRepLib1::BuildCurve3d(E2);

  TColgp_SequenceOfPnt   ResPoints;
  TColStd_SequenceOfReal ResParamsOnE1, ResParamsOnE2;
  Point3d                 DegPoint;
  Standard_Boolean       WithDegen = BRepInspector::Degenerated(E1) || BRepInspector::Degenerated(E2);

  if (WithDegen)
  {
    Standard_Integer ideg = (BRepInspector::Degenerated(E1)) ? 1 : 2;
    TopoDS_Iterator  iter(EI[ideg]);
    if (iter.More())
    {
      const TopoVertex& vdeg = TopoDS::Vertex(iter.Value());
      DegPoint                  = BRepInspector::Pnt(vdeg);
    }
    else
    {
      BRepAdaptor_Curve CEdeg(EI[ideg], F);
      DegPoint = CEdeg.Value(CEdeg.FirstParameter());
    }
  }
  //
  Handle(GeomCurve2d) pcurve1 = BRepInspector::CurveOnSurface(E1, F, f[1], l[1]);
  Handle(GeomCurve2d) pcurve2 = BRepInspector::CurveOnSurface(E2, F, f[2], l[2]);
  Geom2dAdaptor_Curve  GAC1(pcurve1, f[1], l[1]);
  Geom2dAdaptor_Curve  GAC2(pcurve2, f[2], l[2]);
  if ((GAC1.GetType() == GeomAbs_Line) && (GAC2.GetType() == GeomAbs_Line))
  {
    // Just quickly check if lines coincide
    Standard_Real anAngle = Abs(GAC1.Line().Direction().Angle(GAC2.Line().Direction()));
    if (anAngle <= 1.e-8 || M_PI - anAngle <= 1.e-8)
    {
      theCoincide = Standard_True;
      return;
    }
    else
    {
      // Take into account the intersection range of line-line intersection
      // (the smaller angle between curves, the bigger range)
      TolLL = Tools2::ComputeIntRange(TolDub, TolDub, anAngle);
      TolLL = Min(TolLL, 1.e-5);
    }
  }

  Geom2dInt_GInter Inter2d(GAC1, GAC2, TolDub, TolDub);
  //
  if (!Inter2d.IsDone() || !Inter2d.NbPoints())
  {
    theCoincide = (Inter2d.NbSegments() && (GAC1.GetType() == GeomAbs_Line)
                   && (GAC2.GetType() == GeomAbs_Line));
    return;
  }
  //
  for (i = 1; i <= Inter2d.NbPoints(); i++)
  {
    Point3d P3d;
    if (WithDegen)
      P3d = DegPoint;
    else
    {
      gp_Pnt2d P2d = Inter2d.Point(i).Value();
      P3d          = BAsurf.Value(P2d.X(), P2d.Y());
    }
    ResPoints.Append(P3d);
    ResParamsOnE1.Append(Inter2d.Point(i).ParamOnFirst());
    ResParamsOnE2.Append(Inter2d.Point(i).ParamOnSecond());
  }

  for (i = 1; i <= ResPoints.Length(); i++)
  {
    Standard_Real aT1 = ResParamsOnE1(i); // ponc1.Parameter();
    Standard_Real aT2 = ResParamsOnE2(i); // ponc2.Parameter();
    if (Precision1::IsInfinite(aT1) || Precision1::IsInfinite(aT2))
    {
#ifdef OCCT_DEBUG
      std::cout << "Inter2d : Solution rejected due to infinite parameter" << std::endl;
#endif
      continue;
    }

    Point3d        P          = ResPoints(i); // ponc1.Value();
    TopoVertex aNewVertex = BRepLib_MakeVertex(P);
    aNewVertex.Orientation(TopAbs_INTERNAL);
    B.UpdateVertex(aNewVertex, aT1, E1, Tol);
    B.UpdateVertex(aNewVertex, aT2, E2, Tol);
    Point3d        P1 = CE1.Value(aT1);
    Point3d        P2 = CE2.Value(aT2);
    Standard_Real dist1, dist2, dist3;
    dist1 = P1.Distance(P);
    dist2 = P2.Distance(P);
    dist3 = P1.Distance(P2);
    dist1 = Max(dist1, dist2);
    dist1 = Max(dist1, dist3);
    B.UpdateVertex(aNewVertex, dist1);

#ifdef OCCT_DEBUG
    if (aT1 < f[1] - Tol || aT1 > l[1] + Tol)
    {
      std::cout << "out of limit" << std::endl;
      std::cout << "aT1 = " << aT1 << ", f[1] = " << f[1] << ", l[1] = " << l[1] << std::endl;
    }
    if (aT2 < f[2] - Tol || aT2 > l[2] + Tol)
    {
      std::cout << "out of limit" << std::endl;
      std::cout << "aT2 = " << aT2 << ", f[2] = " << f[2] << ", l[2] = " << l[2] << std::endl;
    }
    Standard_Real MilTol2 = 1000 * Tol * Tol;
    if (P1.SquareDistance(P) > MilTol2 || P2.SquareDistance(P) > MilTol2
        || P1.Distance(P2) > 2. * Tol)
    {
      std::cout << "Inter2d : Solution rejected" << std::endl;
      std::cout << "P  = " << P.X() << " " << P.Y() << " " << P.Z() << std::endl;
      std::cout << "P1 = " << P1.X() << " " << P1.Y() << " " << P1.Z() << std::endl;
      std::cout << "P2 = " << P2.X() << " " << P2.Y() << " " << P2.Z() << std::endl;
      std::cout << "MaxDist = " << dist1 << std::endl;
    }
#endif
    // define the orientation of a new vertex
    TopAbs_Orientation OO1 = TopAbs_REVERSED;
    TopAbs_Orientation OO2 = TopAbs_REVERSED;
    if (WithOri)
    {
      BRepAdaptor_Curve2d PCE1(E1, F);
      BRepAdaptor_Curve2d PCE2(E2, F);
      gp_Pnt2d            P2d1, P2d2;
      gp_Vec2d            V1, V2, V1or, V2or;
      PCE1.D1(aT1, P2d1, V1);
      PCE2.D1(aT2, P2d2, V2);
      V1or = V1;
      V2or = V2;
      if (E1.Orientation() == TopAbs_REVERSED)
        V1or.Reverse();
      if (E2.Orientation() == TopAbs_REVERSED)
        V2or.Reverse();
      Standard_Real CrossProd = V2or ^ V1;
#ifdef OCCT_DEBUG
      if (Abs(CrossProd) <= gp1::Resolution())
        std::cout << std::endl << "CrossProd = " << CrossProd << std::endl;
#endif
      if (CrossProd > 0.)
        OO1 = TopAbs_FORWARD;
      CrossProd = V1or ^ V2;
      if (CrossProd > 0.)
        OO2 = TopAbs_FORWARD;
    }

    if (theOr1 != TopAbs_EXTERNAL)
      OO1 = theOr1;
    if (theOr2 != TopAbs_EXTERNAL)
      OO2 = theOr2;

    LV1.Append(aNewVertex.Oriented(OO1));
    LV2.Append(aNewVertex.Oriented(OO2));
  }

  //----------------------------------
  // Test at end.
  //---------------------------------
  Standard_Real U1, U2;
  Standard_Real TolConf = Tol;
  TopoVertex V1[2], V2[2];
  TopExp1::Vertices(E1, V1[0], V1[1]);
  TopExp1::Vertices(E2, V2[0], V2[1]);

  Standard_Integer j;
  for (j = 0; j < 2; j++)
  {
    if (V1[j].IsNull())
      continue;
    for (Standard_Integer k = 0; k < 2; k++)
    {
      if (V2[k].IsNull())
        continue;
      if (V1[j].IsSame(V2[k]))
      {
        if (AsDes->HasAscendant(V1[j]))
        {
          continue;
        }
      }
      //
      Point3d        P1   = BRepInspector::Pnt(V1[j]);
      Point3d        P2   = BRepInspector::Pnt(V2[k]);
      Standard_Real Dist = P1.Distance(P2);
      if (Dist < TolConf)
      {
        TopoVertex V          = BRepLib_MakeVertex(P1);
        U1                       = (j == 0) ? f[1] : l[1];
        U2                       = (k == 0) ? f[2] : l[2];
        TopoShape aLocalShape = V.Oriented(TopAbs_INTERNAL);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape), U1, E1, Tol);
        B.UpdateVertex(TopoDS::Vertex(aLocalShape), U2, E2, Tol);
        LV1.Prepend(V.Oriented(V1[j].Orientation()));
        LV2.Prepend(V.Oriented(V2[k].Orientation()));
      }
    }
  }

  Standard_Boolean AffichPurge = Standard_False;

  if (!LV1.IsEmpty())
  {
    //----------------------------------
    // Remove all vertices.
    // there can be doubles
    //----------------------------------
    TopTools_ListIteratorOfListOfShape it1LV1, it1LV2, it2LV1;
    Point3d                             P1, P2;
    Standard_Boolean                   Purge = Standard_True;

    while (Purge)
    {
      i     = 1;
      Purge = Standard_False;
      for (it1LV1.Initialize(LV1), it1LV2.Initialize(LV2); it1LV1.More();
           it1LV1.Next(), it1LV2.Next())
      {
        j = 1;
        it2LV1.Initialize(LV1);
        while (j < i)
        {
          P1 = BRepInspector::Pnt(TopoDS::Vertex(it1LV1.Value()));
          P2 = BRepInspector::Pnt(TopoDS::Vertex(it2LV1.Value()));
          if (P1.IsEqual(P2, Tol))
          {
            LV1.Remove(it1LV1);
            LV2.Remove(it1LV2);
            if (AffichPurge)
              std::cout << "Doubles removed in EdgeInter." << std::endl;
            Purge = Standard_True;
            break;
          }
          j++;
          it2LV1.Next();
        }
        if (Purge)
          break;
        i++;
      }
    }
    //---------------------------------
    // Vertex storage in SD.
    //---------------------------------
    ////-----------------------------------------------------
    if (LV1.Extent() > 1)
    {
      // std::cout << "IFV - RefEdgeInter: remove vertex" << std::endl;
      Point3d        Pref = BRepInspector::Pnt(theVref);
      Standard_Real dmin = RealLast();
      TopoVertex Vmin;
      for (it1LV1.Initialize(LV1); it1LV1.More(); it1LV1.Next())
      {
        Point3d        P = BRepInspector::Pnt(TopoDS::Vertex(it1LV1.Value()));
        Standard_Real d = P.SquareDistance(Pref);
        if (d < dmin)
        {
          dmin = d;
          Vmin = TopoDS::Vertex(it1LV1.Value());
        }
      }
      for (it1LV1.Initialize(LV1), it1LV2.Initialize(LV2); it1LV1.More();
           it1LV1.Next(), it1LV2.Next())
      {
        if (!Vmin.IsSame(it1LV1.Value()))
        {
          LV1.Remove(it1LV1);
          LV2.Remove(it1LV2);
          if (!it1LV1.More())
            break;
        }
      }
    }

    TopTools_ListIteratorOfListOfShape itl(LV1);
    for (; itl.More(); itl.Next())
    {
      TopoShape aNewVertex = itl.Value();
      aNewVertex.Orientation(TopAbs_FORWARD);
      if (theImageVV.HasImage(theVref))
        theImageVV.Add(theVref.Oriented(TopAbs_FORWARD), aNewVertex);
      else
        theImageVV.Bind(theVref.Oriented(TopAbs_FORWARD), aNewVertex);
    }

    ////-----------------------------------------------------
    Standard_Real TolStore = BRepInspector::Tolerance(E1) + BRepInspector::Tolerance(E2);
    TolStore               = Max(TolStore, Tol);
    // Compare to Line-Line tolerance
    TolStore = Max(TolStore, TolLL);
    Store(E1, E2, LV1, LV2, TolStore, AsDes, aDMVV);
  }
}

//======================================================================
// function : EvaluateMaxSegment
// purpose  : return MaxSegment to pass in approximation
//======================================================================

static Standard_Integer evaluateMaxSegment(const Adaptor3d_CurveOnSurface& aCurveOnSurface)
{
  const Handle(SurfaceAdaptor)& aSurf   = aCurveOnSurface.GetSurface();
  const Handle(Adaptor2d_Curve2d)& aCurv2d = aCurveOnSurface.GetCurve();

  Standard_Real aNbSKnots = 0, aNbC2dKnots = 0;

  if (aSurf->GetType() == GeomAbs_BSplineSurface)
  {
    Handle(Geom_BSplineSurface) aBSpline = aSurf->BSpline();
    aNbSKnots                            = Max(aBSpline->NbUKnots(), aBSpline->NbVKnots());
  }
  if (aCurv2d->GetType() == GeomAbs_BSplineCurve)
  {
    aNbC2dKnots = aCurv2d->NbKnots();
  }
  Standard_Integer aReturn = (Standard_Integer)(30 + Max(aNbSKnots, aNbC2dKnots));
  return aReturn;
}

//=================================================================================================

static Standard_Boolean ExtendPCurve(const Handle(GeomCurve2d)& aPCurve,
                                     const Standard_Real         anEf,
                                     const Standard_Real         anEl,
                                     const Standard_Real         a2Offset,
                                     Handle(GeomCurve2d)&       NewPCurve)
{
  NewPCurve = aPCurve;
  if (NewPCurve->IsInstance(STANDARD_TYPE(Geom2d_TrimmedCurve)))
    NewPCurve = Handle(Geom2d_TrimmedCurve)::DownCast(NewPCurve)->BasisCurve();

  Standard_Real FirstPar = NewPCurve->FirstParameter();
  Standard_Real LastPar  = NewPCurve->LastParameter();

  if (NewPCurve->IsKind(STANDARD_TYPE(Geom2d_BoundedCurve))
      && (FirstPar > anEf - a2Offset || LastPar < anEl + a2Offset))
  {
    if (NewPCurve->IsInstance(STANDARD_TYPE(Geom2d_BezierCurve)))
    {
      Handle(Geom2d_BezierCurve) aBezier = Handle(Geom2d_BezierCurve)::DownCast(NewPCurve);
      if (aBezier->NbPoles() == 2)
      {
        TColgp_Array1OfPnt2d thePoles(1, 2);
        aBezier->Poles(thePoles);
        gp_Vec2d aVec(thePoles(1), thePoles(2));
        NewPCurve = new Geom2d_Line(thePoles(1), aVec);
        return Standard_True;
      }
    }
    else if (NewPCurve->IsInstance(STANDARD_TYPE(Geom2d_BSplineCurve)))
    {
      Handle(Geom2d_BSplineCurve) aBSpline = Handle(Geom2d_BSplineCurve)::DownCast(NewPCurve);
      if (aBSpline->NbKnots() == 2 && aBSpline->NbPoles() == 2)
      {
        TColgp_Array1OfPnt2d thePoles(1, 2);
        aBSpline->Poles(thePoles);
        gp_Vec2d aVec(thePoles(1), thePoles(2));
        NewPCurve = new Geom2d_Line(thePoles(1), aVec);
        return Standard_True;
      }
    }
  }

  FirstPar                             = aPCurve->FirstParameter();
  LastPar                              = aPCurve->LastParameter();
  Handle(Geom2d_TrimmedCurve) aTrCurve = new Geom2d_TrimmedCurve(aPCurve, FirstPar, LastPar);

  // The curve is not prolonged on begin or end.
  // Trying to prolong it adding a segment to its bound.
  gp_Pnt2d                              aPBnd;
  gp_Vec2d                              aVBnd;
  gp_Pnt2d                              aPBeg;
  gp_Dir2d                              aDBnd;
  Handle(Geom2d_Line)                   aLin;
  Handle(Geom2d_TrimmedCurve)           aSegment;
  Geom2dConvert_CompCurveToBSplineCurve aCompCurve(aTrCurve, Convert_RationalC1);
  constexpr Standard_Real               aTol   = Precision1::Confusion();
  Standard_Real                         aDelta = Max(a2Offset, 1.);

  if (FirstPar > anEf - a2Offset)
  {
    aPCurve->D1(FirstPar, aPBnd, aVBnd);
    aDBnd.SetXY(aVBnd.XY());
    aPBeg    = aPBnd.Translated(gp_Vec2d(-aDelta * aDBnd.XY()));
    aLin     = new Geom2d_Line(aPBeg, aDBnd);
    aSegment = new Geom2d_TrimmedCurve(aLin, 0, aDelta);

    if (!aCompCurve.Add(aSegment, aTol))
      return Standard_False;
  }

  if (LastPar < anEl + a2Offset)
  {
    aPCurve->D1(LastPar, aPBeg, aVBnd);
    aDBnd.SetXY(aVBnd.XY());
    aLin     = new Geom2d_Line(aPBeg, aDBnd);
    aSegment = new Geom2d_TrimmedCurve(aLin, 0, aDelta);

    if (!aCompCurve.Add(aSegment, aTol))
      return Standard_False;
  }

  NewPCurve = aCompCurve.BSplineCurve();
  return Standard_True;
}

//=================================================================================================

//  Modified by skv - Fri Dec 26 17:00:55 2003 OCC4455 Begin
// static void ExtentEdge(const TopoEdge& E,TopoEdge& NE)
Standard_Boolean Inter2d::ExtentEdge(const TopoEdge&  E,
                                                TopoEdge&        NE,
                                                const Standard_Real theOffset)
{
  // BRepLib1::BuildCurve3d(E);

  TopoShape     aLocalShape = E.EmptyCopied();
  Standard_Real    anEf;
  Standard_Real    anEl;
  Standard_Real    a2Offset = 2. * Abs(theOffset);
  ShapeBuilder     BB;
  Standard_Integer i, j;

  BRepInspector::Range(E, anEf, anEl);
  NE = TopoDS::Edge(aLocalShape);
  //  NE = TopoDS::Edge(E.EmptyCopied());
  // Enough for analytic edges, for general case reconstruct the
  // geometry of the edge recalculating the intersection of surfaces.

  // BRepLib1::BuildCurve3d(E);

  Standard_Integer     NbPCurves    = 0;
  Standard_Real        FirstParOnPC = RealFirst(), LastParOnPC = RealLast();
  Handle(GeomCurve2d) MinPC;
  Handle(GeomSurface) MinSurf;
  TopLoc_Location      MinLoc;

  BRep_ListIteratorOfListOfCurveRepresentation itr(
    (Handle(BRep_TEdge)::DownCast(NE.TShape()))->ChangeCurves());
  for (; itr.More(); itr.Next())
  {
    Handle(BRep_CurveRepresentation) CurveRep = itr.Value();
    Standard_Real                    FirstPar, LastPar;
    if (CurveRep->IsCurveOnSurface())
    {
      NbPCurves++;
      Handle(GeomCurve2d) theCurve = CurveRep->PCurve();
      FirstPar                      = theCurve->FirstParameter();
      LastPar                       = theCurve->LastParameter();

      if (theCurve->IsKind(STANDARD_TYPE(Geom2d_BoundedCurve))
          && (FirstPar > anEf - a2Offset || LastPar < anEl + a2Offset))
      {
        Handle(GeomCurve2d) NewPCurve;
        if (ExtendPCurve(theCurve, anEf, anEl, a2Offset, NewPCurve))
        {
          CurveRep->PCurve(NewPCurve);
          FirstPar = NewPCurve->FirstParameter();
          LastPar  = NewPCurve->LastParameter();
          if (CurveRep->IsCurveOnClosedSurface())
          {
            Handle(GeomCurve2d) PCurve2 = CurveRep->PCurve2();
            if (ExtendPCurve(PCurve2, anEf, anEl, a2Offset, NewPCurve))
              CurveRep->PCurve2(NewPCurve);
          }
        }
      }
      else if (theCurve->IsPeriodic())
      {
        Standard_Real delta = (theCurve->Period() - (anEl - anEf)) * 0.5;
        delta *= 0.95;
        FirstPar = anEf - delta;
        LastPar  = anEl + delta;
      }
      else if (theCurve->IsClosed())
        LastPar -= 0.05 * (LastPar - FirstPar);

      // check FirstPar and LastPar: the pcurve should be in its surface
      theCurve                     = CurveRep->PCurve();
      Handle(GeomSurface) theSurf = CurveRep->Surface();
      Standard_Real        Umin, Umax, Vmin, Vmax;
      theSurf->Bounds(Umin, Umax, Vmin, Vmax);
      TColGeom2d_SequenceOfCurve BoundLines;
      if (!Precision1::IsInfinite(Vmin))
      {
        Handle(Geom2d_Line) aLine = new Geom2d_Line(gp_Pnt2d(0., Vmin), gp_Dir2d(1., 0.));
        BoundLines.Append(aLine);
      }
      if (!Precision1::IsInfinite(Umin))
      {
        Handle(Geom2d_Line) aLine = new Geom2d_Line(gp_Pnt2d(Umin, 0.), gp_Dir2d(0., 1.));
        BoundLines.Append(aLine);
      }
      if (!Precision1::IsInfinite(Vmax))
      {
        Handle(Geom2d_Line) aLine = new Geom2d_Line(gp_Pnt2d(0., Vmax), gp_Dir2d(1., 0.));
        BoundLines.Append(aLine);
      }
      if (!Precision1::IsInfinite(Umax))
      {
        Handle(Geom2d_Line) aLine = new Geom2d_Line(gp_Pnt2d(Umax, 0.), gp_Dir2d(0., 1.));
        BoundLines.Append(aLine);
      }

      TColStd_SequenceOfReal params;
      Geom2dInt_GInter       IntCC;
      Geom2dAdaptor_Curve    GAcurve(theCurve);
      for (i = 1; i <= BoundLines.Length(); i++)
      {
        Geom2dAdaptor_Curve GAline(BoundLines(i));
        IntCC.Perform(GAcurve, GAline, Precision1::PConfusion(), Precision1::PConfusion());
        if (IntCC.IsDone())
        {
          for (j = 1; j <= IntCC.NbPoints(); j++)
          {
            const IntersectionPoint3& ip     = IntCC.Point(j);
            gp_Pnt2d                          aPoint = ip.Value();
            if (aPoint.X() >= Umin && aPoint.X() <= Umax && aPoint.Y() >= Vmin
                && aPoint.Y() <= Vmax)
              params.Append(ip.ParamOnFirst());
          }
          for (j = 1; j <= IntCC.NbSegments(); j++)
          {
            const IntRes2d_IntersectionSegment& is = IntCC.Segment1(j);
            if (is.HasFirstPoint())
            {
              const IntersectionPoint3& ip     = is.FirstPoint();
              gp_Pnt2d                          aPoint = ip.Value();
              if (aPoint.X() >= Umin && aPoint.X() <= Umax && aPoint.Y() >= Vmin
                  && aPoint.Y() <= Vmax)
                params.Append(ip.ParamOnFirst());
            }
            if (is.HasLastPoint())
            {
              const IntersectionPoint3& ip     = is.LastPoint();
              gp_Pnt2d                          aPoint = ip.Value();
              if (aPoint.X() >= Umin && aPoint.X() <= Umax && aPoint.Y() >= Vmin
                  && aPoint.Y() <= Vmax)
                params.Append(ip.ParamOnFirst());
            }
          }
        }
      }
      if (!params.IsEmpty())
      {
        if (params.Length() == 1)
        {
          gp_Pnt2d PntFirst = theCurve->Value(FirstPar);
          if (PntFirst.X() >= Umin && PntFirst.X() <= Umax && PntFirst.Y() >= Vmin
              && PntFirst.Y() <= Vmax)
          {
            if (LastPar > params(1))
              LastPar = params(1);
          }
          else if (FirstPar < params(1))
            FirstPar = params(1);
        }
        else
        {
          Standard_Real fpar = RealLast(), lpar = RealFirst();
          for (i = 1; i <= params.Length(); i++)
          {
            if (params(i) < fpar)
              fpar = params(i);
            if (params(i) > lpar)
              lpar = params(i);
          }
          if (FirstPar < fpar)
            FirstPar = fpar;
          if (LastPar > lpar)
            LastPar = lpar;
        }
      }
      //// end of check ////
      (Handle(BRep_GCurve)::DownCast(CurveRep))->SetRange(FirstPar, LastPar);
      // gp_Pnt2d Pfirst = theCurve->Value(FirstPar);
      // gp_Pnt2d Plast  = theCurve->Value(LastPar);
      //(Handle(BRep_CurveOnSurface)::DownCast(CurveRep))->SetUVPoints( Pfirst, Plast );

      // update FirstParOnPC and LastParOnPC
      if (FirstPar > FirstParOnPC)
      {
        FirstParOnPC = FirstPar;
        MinPC        = theCurve;
        MinSurf      = theSurf;
        MinLoc       = CurveRep->Location();
      }
      if (LastPar < LastParOnPC)
      {
        LastParOnPC = LastPar;
        MinPC       = theCurve;
        MinSurf     = theSurf;
        MinLoc      = CurveRep->Location();
      }
    }
  }

  Standard_Real      f, l;
  Handle(GeomCurve3d) C3d = BRepInspector::Curve(NE, f, l);
  if (NbPCurves)
  {
    MinLoc = E.Location() * MinLoc;
    if (!C3d.IsNull())
    {
      if (MinPC->IsClosed())
      {
        f = FirstParOnPC;
        l = LastParOnPC;
      }
      else if (C3d->IsPeriodic())
      {
        Standard_Real delta = (C3d->Period() - (l - f)) * 0.5;
        delta *= 0.95;
        f -= delta;
        l += delta;
      }
      else if (C3d->IsClosed())
        l -= 0.05 * (l - f);
      else
      {
        f = FirstParOnPC;
        l = LastParOnPC;
        GeomAPI_ProjectPointOnCurve Projector;
        if (!Precision1::IsInfinite(FirstParOnPC))
        {
          gp_Pnt2d P2d1 = MinPC->Value(FirstParOnPC);
          Point3d   P1   = MinSurf->Value(P2d1.X(), P2d1.Y());
          P1.Transform(MinLoc.Transformation());
          Projector.Init(P1, C3d);
          if (Projector.NbPoints() > 0)
            f = Projector.LowerDistanceParameter();
#ifdef OCCT_DEBUG
          else
            std::cout << "ProjectPointOnCurve not done" << std::endl;
#endif
        }
        if (!Precision1::IsInfinite(LastParOnPC))
        {
          gp_Pnt2d P2d2 = MinPC->Value(LastParOnPC);
          Point3d   P2   = MinSurf->Value(P2d2.X(), P2d2.Y());
          P2.Transform(MinLoc.Transformation());
          Projector.Init(P2, C3d);
          if (Projector.NbPoints() > 0)
            l = Projector.LowerDistanceParameter();
#ifdef OCCT_DEBUG
          else
            std::cout << "ProjectPointOnCurve not done" << std::endl;
#endif
        }
      }
      BB.Range(NE, f, l);
      if (!Precision1::IsInfinite(f) && !Precision1::IsInfinite(l))
        BRepLib1::SameParameter(NE, Precision1::Confusion(), Standard_True);
    }
    else if (!BRepInspector::Degenerated(E)) // no 3d curve
    {
      MinSurf = Handle(GeomSurface)::DownCast(MinSurf->Transformed(MinLoc.Transformation()));
      Standard_Real max_deviation = 0.;
      if (Precision1::IsInfinite(FirstParOnPC) || Precision1::IsInfinite(LastParOnPC))
      {
        if (MinPC->IsInstance(STANDARD_TYPE(Geom2d_Line)))
        {
          Standard_Boolean IsLine = Standard_False;
          if (MinSurf->IsInstance(STANDARD_TYPE(GeomPlane)))
            IsLine = Standard_True;
          else if (MinSurf->IsInstance(STANDARD_TYPE(Geom_CylindricalSurface))
                   || MinSurf->IsInstance(STANDARD_TYPE(Geom_ConicalSurface)))
          {
            Handle(Geom2d_Line) theLine = Handle(Geom2d_Line)::DownCast(MinPC);
            gp_Dir2d            LineDir = theLine->Direction();
            if (LineDir.IsParallel(gp1::DY2d(), Precision1::Angular()))
              IsLine = Standard_True;
          }
          if (IsLine)
          {
            gp_Pnt2d P2d1 = MinPC->Value(0.), P2d2 = MinPC->Value(1.);
            Point3d   P1 = MinSurf->Value(P2d1.X(), P2d1.Y());
            Point3d   P2 = MinSurf->Value(P2d2.X(), P2d2.Y());
            Vector3d   aVec(P1, P2);
            C3d = new GeomLine(P1, aVec);
          }
        }
      }
      else
      {
        Geom2dAdaptor_Curve              AC2d(MinPC, FirstParOnPC, LastParOnPC);
        GeomAdaptor_Surface              GAsurf(MinSurf);
        Handle(Geom2dAdaptor_Curve)      HC2d  = new Geom2dAdaptor_Curve(AC2d);
        Handle(GeomAdaptor_Surface)      HSurf = new GeomAdaptor_Surface(GAsurf);
        Adaptor3d_CurveOnSurface         ConS(HC2d, HSurf);
        Standard_Real /*max_deviation,*/ average_deviation;
        GeomAbs_Shape                    Continuity = GeomAbs_C1;
        Standard_Integer                 MaxDegree  = 14;
        Standard_Integer                 MaxSegment = evaluateMaxSegment(ConS);
        GeomLib1::BuildCurve3d(Precision1::Confusion(),
                              ConS,
                              FirstParOnPC,
                              LastParOnPC,
                              C3d,
                              max_deviation,
                              average_deviation,
                              Continuity,
                              MaxDegree,
                              MaxSegment);
      }
      BB.UpdateEdge(NE, C3d, max_deviation);
      // BB.Range( NE, FirstParOnPC, LastParOnPC );
      Standard_Boolean ProjectionSuccess = Standard_True;
      if (NbPCurves > 1)
        // BRepLib1::SameParameter( NE, Precision1::Confusion(), Standard_True );
        for (itr.Initialize((Handle(BRep_TEdge)::DownCast(NE.TShape()))->ChangeCurves());
             itr.More();
             itr.Next())
        {
          Handle(BRep_CurveRepresentation) CurveRep = itr.Value();
          Standard_Real                    FirstPar, LastPar;
          if (CurveRep->IsCurveOnSurface())
          {
            Handle(GeomCurve2d) theCurve = CurveRep->PCurve();
            Handle(GeomSurface) theSurf  = CurveRep->Surface();
            TopLoc_Location      theLoc   = CurveRep->Location();
            if (theCurve == MinPC && theSurf == MinSurf && theLoc == MinLoc)
              continue;
            FirstPar = (Handle(BRep_GCurve)::DownCast(CurveRep))->First();
            LastPar  = (Handle(BRep_GCurve)::DownCast(CurveRep))->Last();
            if (Abs(FirstPar - FirstParOnPC) > Precision1::PConfusion()
                || Abs(LastPar - LastParOnPC) > Precision1::PConfusion())
            {
              theLoc = E.Location() * theLoc;
              theSurf =
                Handle(GeomSurface)::DownCast(theSurf->Transformed(theLoc.Transformation()));

              if (theCurve->IsInstance(STANDARD_TYPE(Geom2d_Line))
                  && theSurf->IsKind(STANDARD_TYPE(Geom_BoundedSurface)))
              {
                gp_Dir2d theDir = Handle(Geom2d_Line)::DownCast(theCurve)->Direction();
                if (theDir.IsParallel(gp1::DX2d(), Precision1::Angular())
                    || theDir.IsParallel(gp1::DY2d(), Precision1::Angular()))
                {
                  Standard_Real U1, U2, V1, V2;
                  theSurf->Bounds(U1, U2, V1, V2);
                  gp_Pnt2d Origin = Handle(Geom2d_Line)::DownCast(theCurve)->Location();
                  if (Abs(Origin.X() - U1) <= Precision1::Confusion()
                      || Abs(Origin.X() - U2) <= Precision1::Confusion()
                      || Abs(Origin.Y() - V1) <= Precision1::Confusion()
                      || Abs(Origin.Y() - V2) <= Precision1::Confusion())
                  {
                    BRepLib1::SameParameter(NE, Precision1::Confusion(), Standard_True);
                    break;
                  }
                }
              }
              if (!C3d.IsNull() && FirstParOnPC < LastParOnPC)
              {
                Handle(GeomCurve2d) ProjPCurve =
                  GeomProjLib1::Curve2d(C3d, FirstParOnPC, LastParOnPC, theSurf);
                if (ProjPCurve.IsNull())
                  ProjectionSuccess = Standard_False;
                else
                  CurveRep->PCurve(ProjPCurve);
              }
              else
              {
                return Standard_False;
              }
            }
          }
        }
      if (ProjectionSuccess)
        BB.Range(NE, FirstParOnPC, LastParOnPC);
      else
      {
        BB.Range(NE, FirstParOnPC, LastParOnPC, Standard_True);
        BRepLib1::SameParameter(NE, Precision1::Confusion(), Standard_True);
      }
    }
  }
  else // no pcurves
  {
    Standard_Real FirstPar = C3d->FirstParameter();
    Standard_Real LastPar  = C3d->LastParameter();

    if (C3d->IsKind(STANDARD_TYPE(Geom_BoundedCurve))
        && (FirstPar > anEf - a2Offset || LastPar < anEl + a2Offset))
    {
      Handle(Geom_TrimmedCurve) aTrCurve = new Geom_TrimmedCurve(C3d, FirstPar, LastPar);

      // The curve is not prolonged on begin or end.
      // Trying to prolong it adding a segment to its bound.
      Point3d                              aPBnd;
      Vector3d                              aVBnd;
      Point3d                              aPBeg;
      Dir3d                              aDBnd;
      Handle(GeomLine)                   aLin;
      Handle(Geom_TrimmedCurve)           aSegment;
      GeomConvert_CompCurveToBSplineCurve aCompCurve(aTrCurve, Convert_RationalC1);
      constexpr Standard_Real             aTol   = Precision1::Confusion();
      Standard_Real                       aDelta = Max(a2Offset, 1.);

      if (FirstPar > anEf - a2Offset)
      {
        C3d->D1(FirstPar, aPBnd, aVBnd);
        aDBnd.SetXYZ(aVBnd.XYZ());
        aPBeg    = aPBnd.Translated(Vector3d(-aDelta * aDBnd.XYZ()));
        aLin     = new GeomLine(aPBeg, aDBnd);
        aSegment = new Geom_TrimmedCurve(aLin, 0, aDelta);

        if (!aCompCurve.Add(aSegment, aTol))
          return Standard_True;
      }

      if (LastPar < anEl + a2Offset)
      {
        C3d->D1(LastPar, aPBeg, aVBnd);
        aDBnd.SetXYZ(aVBnd.XYZ());
        aLin     = new GeomLine(aPBeg, aDBnd);
        aSegment = new Geom_TrimmedCurve(aLin, 0, aDelta);

        if (!aCompCurve.Add(aSegment, aTol))
          return Standard_True;
      }

      C3d      = aCompCurve.BSplineCurve();
      FirstPar = C3d->FirstParameter();
      LastPar  = C3d->LastParameter();
      BB.UpdateEdge(NE, C3d, Precision1::Confusion());
    }
    else if (C3d->IsPeriodic())
    {
      Standard_Real delta = (C3d->Period() - (anEl - anEf)) * 0.5;
      delta *= 0.95;
      FirstPar = anEf - delta;
      LastPar  = anEl + delta;
    }
    else if (C3d->IsClosed())
      LastPar -= 0.05 * (LastPar - FirstPar);

    BB.Range(NE, FirstPar, LastPar);
  }
  return Standard_True;
}

//  Modified by skv - Fri Dec 26 17:00:57 2003 OCC4455 End

//=================================================================================================

static Standard_Boolean UpdateVertex(const TopoVertex& V,
                                     TopoEdge&         OE,
                                     TopoEdge&         NE,
                                     Standard_Real        TolConf)
{
  BRepAdaptor_Curve       OC(OE);
  BRepAdaptor_Curve       NC(NE);
  Standard_Real           Of     = OC.FirstParameter();
  Standard_Real           Ol     = OC.LastParameter();
  Standard_Real           Nf     = NC.FirstParameter();
  Standard_Real           Nl     = NC.LastParameter();
  Standard_Real           U      = 0.;
  constexpr Standard_Real ParTol = Precision1::PConfusion();
  Point3d                  P      = BRepInspector::Pnt(V);
  Standard_Boolean        OK     = Standard_False;

  if (P.Distance(OC.Value(Of)) < TolConf)
  {
    if (Of >= Nf + ParTol && Of <= Nl + ParTol && P.Distance(NC.Value(Of)) < TolConf)
    {
      OK = Standard_True;
      U  = Of;
    }
  }
  if (P.Distance(OC.Value(Ol)) < TolConf)
  {
    if (Ol >= Nf + ParTol && Ol <= Nl + ParTol && P.Distance(NC.Value(Ol)) < TolConf)
    {
      OK = Standard_True;
      U  = Ol;
    }
  }
  if (OK)
  {
    ShapeBuilder B;
    TopoShape aLocalShape = NE.Oriented(TopAbs_FORWARD);
    TopoEdge  EE          = TopoDS::Edge(aLocalShape);
    //    TopoEdge EE = TopoDS::Edge(NE.Oriented(TopAbs_FORWARD));
    aLocalShape = V.Oriented(TopAbs_INTERNAL);
    B.UpdateVertex(TopoDS::Vertex(aLocalShape), U, NE, BRepInspector::Tolerance(NE));
    //    B.UpdateVertex(TopoDS::Vertex(V.Oriented(TopAbs_INTERNAL)),
    //                   U,NE,BRepInspector::Tolerance(NE));
  }
  return OK;
}

//=================================================================================================

void Inter2d::Compute(const Handle(BRepAlgo_AsDes)&              AsDes,
                                 const TopoFace&                         F,
                                 const TopTools_IndexedMapOfShape&          NewEdges,
                                 const Standard_Real                        Tol,
                                 const TopTools_DataMapOfShapeListOfShape&  theEdgeIntEdges,
                                 TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
                                 const Message_ProgressRange&               theRange)
{
#ifdef DRAW
  NbF2d++;
  NbE2d = 0;
#endif

  // Do not intersect the edges of face
  TopTools_MapOfShape EdgesOfFace;
  ShapeExplorer     Explo(F, TopAbs_EDGE);
  for (; Explo.More(); Explo.Next())
    EdgesOfFace.Add(Explo.Current());

  //-----------------------------------------------------------
  // calculate intersections2d on faces touched by
  // intersection3d
  //---------------------------------------------------------
  TopTools_ListIteratorOfListOfShape it1LE;
  TopTools_ListIteratorOfListOfShape it2LE;

  //-----------------------------------------------
  // Intersection of edges 2*2.
  //-----------------------------------------------
  const ShapeList& LE = AsDes->Descendant(F);
  TopoVertex               V1, V2;
  Standard_Integer            j, i = 1;
  BRepAdaptor_Surface         BAsurf(F);
  //
  Message_ProgressScope aPS(theRange, "Intersecting edges on faces", LE.Size());
  for (it1LE.Initialize(LE); it1LE.More(); it1LE.Next(), aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    const TopoEdge& E1 = TopoDS::Edge(it1LE.Value());
    j                     = 1;
    it2LE.Initialize(LE);

    while (j < i && it2LE.More())
    {
      const TopoEdge& E2 = TopoDS::Edge(it2LE.Value());

      Standard_Boolean ToIntersect = Standard_True;
      if (theEdgeIntEdges.IsBound(E1))
      {
        const ShapeList&        aElist = theEdgeIntEdges(E1);
        TopTools_ListIteratorOfListOfShape itedges(aElist);
        for (; itedges.More(); itedges.Next())
          if (E2.IsSame(itedges.Value()))
            ToIntersect = Standard_False;

        if (ToIntersect)
        {
          for (itedges.Initialize(aElist); itedges.More(); itedges.Next())
          {
            const TopoShape& anEdge = itedges.Value();
            if (theEdgeIntEdges.IsBound(anEdge))
            {
              const ShapeList&        aElist2 = theEdgeIntEdges(anEdge);
              TopTools_ListIteratorOfListOfShape itedges2(aElist2);
              for (; itedges2.More(); itedges2.Next())
                if (E2.IsSame(itedges2.Value()))
                  ToIntersect = Standard_False;
            }
          }
        }
      }

      //--------------------------------------------------------------
      // Intersections of New edges obtained by intersection
      // between them and with edges of restrictions
      //------------------------------------------------------
      if (ToIntersect && (!EdgesOfFace.Contains(E1) || !EdgesOfFace.Contains(E2))
          && (NewEdges.Contains(E1) || NewEdges.Contains(E2)))
      {

        TopoShape aLocalShape = F.Oriented(TopAbs_FORWARD);
        EdgeInter(TopoDS::Face(aLocalShape), BAsurf, E1, E2, AsDes, Tol, Standard_True, theDMVV);
        //          EdgeInter(TopoDS::Face(F.Oriented(TopAbs_FORWARD)),E1,E2,AsDes,Tol,Standard_True);
      }
      it2LE.Next();
      j++;
    }
    i++;
  }
}

//=================================================================================================

Standard_Boolean Inter2d::ConnexIntByInt(
  const TopoFace&                         FI,
  BRepOffset_Offset&                         OFI,
  TopTools_DataMapOfShapeShape&              MES,
  const TopTools_DataMapOfShapeShape&        Build,
  const Handle(BRepAlgo_AsDes)&              theAsDes,
  const Handle(BRepAlgo_AsDes)&              AsDes2d,
  const Standard_Real                        Offset,
  const Standard_Real                        Tol,
  const BRepOffset_Analyse&                  Analyse,
  TopTools_IndexedMapOfShape&                FacesWithVerts,
  ShapeImage&                            theImageVV,
  TopTools_DataMapOfShapeListOfShape&        theEdgeIntEdges,
  TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
  const Message_ProgressRange&               theRange)
{

  TopTools_DataMapOfShapeListOfShape MVE;
  Tool5::MapVertexEdges(FI, MVE);
  Message_ProgressScope aPS(theRange,
                            "Intersecting edges obtained as intersection of faces",
                            1,
                            Standard_True);
  //---------------------
  // Extension of edges.
  //---------------------
  TopoEdge                                         NE;
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape it(MVE);
  for (; it.More(); it.Next())
  {
    if (!aPS.More())
    {
      return Standard_False;
    }
    const ShapeList&        L       = it.Value();
    Standard_Boolean                   YaBuild = 0;
    TopTools_ListIteratorOfListOfShape itL(L);
    for (; itL.More(); itL.Next())
    {
      YaBuild = Build.IsBound(itL.Value());
      if (YaBuild)
        break;
    }
    if (YaBuild)
    {
      for (itL.Initialize(L); itL.More(); itL.Next())
      {
        const TopoEdge& EI = TopoDS::Edge(itL.Value());
        if (EI.Orientation() != TopAbs_FORWARD && EI.Orientation() != TopAbs_REVERSED)
          continue;
        TopoShape       aLocalShape = OFI.Generated(EI);
        const TopoEdge& OE          = TopoDS::Edge(aLocalShape);
        if (!MES.IsBound(OE) && !Build.IsBound(EI))
        {
          if (!ExtentEdge(OE, NE, Offset))
          {
            return Standard_False;
          }
          MES.Bind(OE, NE);
        }
      }
    }
  }

  TopoFace FIO = TopoDS::Face(OFI.Face());
  if (MES.IsBound(FIO))
    FIO = TopoDS::Face(MES(FIO));
  //
  BRepAdaptor_Surface BAsurf(FIO);

  ShapeExplorer exp(FI.Oriented(TopAbs_FORWARD), TopAbs_WIRE);
  for (; exp.More(); exp.Next(), aPS.Next())
  {
    if (!aPS.More())
    {
      return Standard_False;
    }
    const TopoWire&     W = TopoDS::Wire(exp.Current());
    BRepTools_WireExplorer wexp;
    Standard_Boolean       end = Standard_False;
    TopoEdge            FirstE, CurE, NextE;

    TopoShape aLocalWire = W.Oriented(TopAbs_FORWARD);
    TopoShape aLocalFace = FI.Oriented(TopAbs_FORWARD);
    wexp.Init(TopoDS::Wire(aLocalWire), TopoDS::Face(aLocalFace));
    if (!wexp.More())
      continue; // Protection from case when explorer does not contain edges.
    CurE = FirstE = wexp.Current();
    TopTools_IndexedMapOfShape Edges;

    while (!end)
    {
      wexp.Next();
      if (wexp.More())
      {
        NextE = wexp.Current();
      }
      else
      {
        NextE = FirstE;
        end   = Standard_True;
      }
      if (CurE.IsSame(NextE))
        continue;

      TopoVertex Vref = CommonVertex(CurE, NextE);

      CurE  = Analyse.EdgeReplacement(FI, CurE);
      NextE = Analyse.EdgeReplacement(FI, NextE);

      TopoShape aLocalShape = OFI.Generated(CurE);
      TopoEdge  CEO         = TopoDS::Edge(aLocalShape);
      aLocalShape              = OFI.Generated(NextE);
      TopoEdge NEO          = TopoDS::Edge(aLocalShape);
      //------------------------------------------
      // Inter processing of images of CurE NextE.
      //------------------------------------------
      ShapeList     LV1, LV2;
      Standard_Boolean         DoInter = 1;
      TopoShape             NE1, NE2;
      TopTools_SequenceOfShape NE1seq, NE2seq;
      TopAbs_Orientation       anOr1 = TopAbs_EXTERNAL, anOr2 = TopAbs_EXTERNAL;

      Standard_Integer aChoice = 0;
      if (Build.IsBound(CurE) && Build.IsBound(NextE))
      {
        aChoice = 1;
        NE1     = Build(CurE);
        NE2     = Build(NextE);
        GetEdgesOrientedInFace(NE1, FIO, theAsDes, NE1seq);
        GetEdgesOrientedInFace(NE2, FIO, theAsDes, NE2seq);
        anOr1 = TopAbs_REVERSED;
        anOr2 = TopAbs_FORWARD;
      }
      else if (Build.IsBound(CurE) && MES.IsBound(NEO))
      {
        aChoice = 2;
        NE1     = Build(CurE);
        NE2     = MES(NEO);
        NE2.Orientation(NextE.Orientation());
        GetEdgesOrientedInFace(NE1, FIO, theAsDes, NE1seq);
        NE2seq.Append(NE2);
        anOr1 = TopAbs_REVERSED;
        anOr2 = TopAbs_FORWARD;
      }
      else if (Build.IsBound(NextE) && MES.IsBound(CEO))
      {
        aChoice = 3;
        NE1     = Build(NextE);
        NE2     = MES(CEO);
        NE2.Orientation(CurE.Orientation());
        GetEdgesOrientedInFace(NE1, FIO, theAsDes, NE1seq);
        NE2seq.Append(NE2);
        anOr1 = TopAbs_FORWARD;
        anOr2 = TopAbs_REVERSED;
      }
      else
      {
        DoInter = 0;
      }
      if (DoInter)
      {
        //------------------------------------
        // NE1,NE2 can be a compound of Edges.
        //------------------------------------
        Standard_Boolean bCoincide;
        TopoEdge      aE1, aE2;
        if (aChoice == 1 || aChoice == 2)
        {
          aE1 = TopoDS::Edge(NE1seq.Last());
          aE2 = TopoDS::Edge(NE2seq.First());
        }
        else // aChoice == 3
        {
          aE1 = TopoDS::Edge(NE1seq.First());
          aE2 = TopoDS::Edge(NE2seq.Last());
        }

        if (aE1.Orientation() == TopAbs_REVERSED)
          anOr1 = TopAbs1::Reverse(anOr1);
        if (aE2.Orientation() == TopAbs_REVERSED)
          anOr2 = TopAbs1::Reverse(anOr2);

        RefEdgeInter(FIO,
                     BAsurf,
                     aE1,
                     aE2,
                     anOr1,
                     anOr2,
                     AsDes2d,
                     Tol,
                     Standard_True,
                     Vref,
                     theImageVV,
                     theDMVV,
                     bCoincide);

        if (theEdgeIntEdges.IsBound(aE1))
          theEdgeIntEdges(aE1).Append(aE2);
        else
        {
          ShapeList aElist;
          aElist.Append(aE2);
          theEdgeIntEdges.Bind(aE1, aElist);
        }
        if (theEdgeIntEdges.IsBound(aE2))
          theEdgeIntEdges(aE2).Append(aE1);
        else
        {
          ShapeList aElist;
          aElist.Append(aE1);
          theEdgeIntEdges.Bind(aE2, aElist);
        }

        //
        // check if some of the offset edges have been
        // generated out of the common vertex
        if (Build.IsBound(Vref))
        {
          FacesWithVerts.Add(FI);
        }
      }
      else
      {
        TopoVertex V = CommonVertex(CEO, NEO);
        if (!V.IsNull())
        {
          if (MES.IsBound(CEO))
          {
            UpdateVertex(V, CEO, TopoDS::Edge(MES(CEO)), Tol);
            AsDes2d->Add(MES(CEO), V);
          }
          if (MES.IsBound(NEO))
          {
            UpdateVertex(V, NEO, TopoDS::Edge(MES(NEO)), Tol);
            AsDes2d->Add(MES(NEO), V);
          }
        }
      }
      CurE = wexp.Current();
    }
  }
  return Standard_True;
}

//=======================================================================
// function : ConnexIntByIntInVert
// purpose  : Intersection of the edges generated out of vertices
//=======================================================================
void Inter2d::ConnexIntByIntInVert(const TopoFace&                         FI,
                                              BRepOffset_Offset&                         OFI,
                                              TopTools_DataMapOfShapeShape&              MES,
                                              const TopTools_DataMapOfShapeShape&        Build,
                                              const Handle(BRepAlgo_AsDes)&              AsDes,
                                              const Handle(BRepAlgo_AsDes)&              AsDes2d,
                                              const Standard_Real                        Tol,
                                              const BRepOffset_Analyse&                  Analyse,
                                              TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
                                              const Message_ProgressRange&               theRange)
{
  TopoFace FIO = TopoDS::Face(OFI.Face());
  if (MES.IsBound(FIO))
    FIO = TopoDS::Face(MES(FIO));
  //
  TopTools_MapOfShape                aME;
  const ShapeList&        aLE = AsDes->Descendant(FIO);
  TopTools_ListIteratorOfListOfShape aItLE(aLE);
  for (; aItLE.More(); aItLE.Next())
  {
    const TopoShape& aE = aItLE.Value();
    aME.Add(aE);
  }
  //
  BRepAdaptor_Surface BAsurf(FIO);
  //
  Message_ProgressScope aPS(theRange, "Intersecting edges created from vertices", 1, Standard_True);
  ShapeExplorer       exp(FI.Oriented(TopAbs_FORWARD), TopAbs_WIRE);
  for (; exp.More(); exp.Next(), aPS.Next())
  {
    if (!aPS.More())
    {
      return;
    }
    const TopoWire& W = TopoDS::Wire(exp.Current());
    //
    BRepTools_WireExplorer wexp;
    Standard_Boolean       end = Standard_False;
    TopoEdge            FirstE, CurE, NextE;
    //
    TopoShape aLocalWire = W.Oriented(TopAbs_FORWARD);
    TopoShape aLocalFace = FI.Oriented(TopAbs_FORWARD);
    wexp.Init(TopoDS::Wire(aLocalWire), TopoDS::Face(aLocalFace));
    if (!wexp.More())
      continue; // Protection from case when explorer does not contain edges.
    //
    CurE = FirstE = wexp.Current();
    while (!end)
    {
      wexp.Next();
      if (wexp.More())
      {
        NextE = wexp.Current();
      }
      else
      {
        NextE = FirstE;
        end   = Standard_True;
      }
      if (CurE.IsSame(NextE))
        continue;
      //
      TopoVertex Vref = CommonVertex(CurE, NextE);
      if (!Build.IsBound(Vref))
      {
        CurE = NextE;
        continue;
      }

      CurE  = Analyse.EdgeReplacement(FI, CurE);
      NextE = Analyse.EdgeReplacement(FI, NextE);

      TopoShape aLocalShape = OFI.Generated(CurE);
      TopoEdge  CEO         = TopoDS::Edge(aLocalShape);
      aLocalShape              = OFI.Generated(NextE);
      TopoEdge NEO          = TopoDS::Edge(aLocalShape);
      //
      TopoShape       NE1, NE2;
      TopAbs_Orientation anOr1 = TopAbs_EXTERNAL, anOr2 = TopAbs_EXTERNAL;

      if (Build.IsBound(CurE) && Build.IsBound(NextE))
      {
        NE1 = Build(CurE);
        NE2 = Build(NextE);
      }
      else if (Build.IsBound(CurE) && MES.IsBound(NEO))
      {
        NE1 = Build(CurE);
        NE2 = MES(NEO);
      }
      else if (Build.IsBound(NextE) && MES.IsBound(CEO))
      {
        NE1 = Build(NextE);
        NE2 = MES(CEO);
      }
      else
      {
        CurE = wexp.Current();
        continue;
      }
      //
      ShapeExplorer  Exp1, Exp2;
      Standard_Boolean bCoincide;
      // intersect edges generated from vertex with the edges of the face
      const TopoShape& NE3 = Build(Vref);
      //
      for (Exp2.Init(NE3, TopAbs_EDGE); Exp2.More(); Exp2.Next())
      {
        const TopoEdge& aE3 = *(TopoEdge*)&Exp2.Current();
        if (!aME.Contains(aE3))
        {
          continue;
        }
        //
        // intersection with first edge
        for (Exp1.Init(NE1, TopAbs_EDGE); Exp1.More(); Exp1.Next())
        {
          const TopoEdge& aE1 = TopoDS::Edge(Exp1.Current());
          ShapeImage     anEmptyImage;
          RefEdgeInter(FIO,
                       BAsurf,
                       aE1,
                       aE3,
                       anOr1,
                       anOr2,
                       AsDes2d,
                       Tol,
                       Standard_True,
                       Vref,
                       anEmptyImage,
                       theDMVV,
                       bCoincide);
          if (bCoincide)
          {
            // in case of coincidence trim the edge E3 the same way as E1
            Store(aE3, AsDes2d->Descendant(aE1), Tol, Standard_True, AsDes2d, theDMVV);
          }
        }
        //
        // intersection with second edge
        for (Exp1.Init(NE2, TopAbs_EDGE); Exp1.More(); Exp1.Next())
        {
          const TopoEdge& aE2 = TopoDS::Edge(Exp1.Current());
          ShapeImage     anEmptyImage;
          RefEdgeInter(FIO,
                       BAsurf,
                       aE2,
                       aE3,
                       anOr1,
                       anOr2,
                       AsDes2d,
                       Tol,
                       Standard_True,
                       Vref,
                       anEmptyImage,
                       theDMVV,
                       bCoincide);
          if (bCoincide)
          {
            // in case of coincidence trim the edge E3 the same way as E2
            Store(aE3, AsDes2d->Descendant(aE2), Tol, Standard_True, AsDes2d, theDMVV);
          }
        }
        //
        // intersection of the edges generated from vertex
        // among themselves
        for (Exp1.Init(NE3, TopAbs_EDGE); Exp1.More(); Exp1.Next())
        {
          if (aE3.IsSame(Exp1.Current()))
          {
            break;
          }
        }
        //
        for (Exp1.Next(); Exp1.More(); Exp1.Next())
        {
          const TopoEdge& aE3Next = TopoDS::Edge(Exp1.Current());
          if (aME.Contains(aE3Next))
          {
            ShapeImage anEmptyImage;
            RefEdgeInter(FIO,
                         BAsurf,
                         aE3Next,
                         aE3,
                         anOr1,
                         anOr2,
                         AsDes2d,
                         Tol,
                         Standard_True,
                         Vref,
                         anEmptyImage,
                         theDMVV,
                         bCoincide);
          }
        }
      }
      CurE = wexp.Current();
    }
  }
}

//=================================================================================================

static void MakeChain(const TopoShape&                              theV,
                      const TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
                      TopTools_MapOfShape&                             theMDone,
                      ShapeList&                            theChain)
{
  if (theMDone.Add(theV))
  {
    theChain.Append(theV);
    const ShapeList* pLV = theDMVV.Seek(theV);
    if (pLV)
    {
      TopTools_ListIteratorOfListOfShape aIt(*pLV);
      for (; aIt.More(); aIt.Next())
      {
        MakeChain(aIt.Value(), theDMVV, theMDone, theChain);
      }
    }
  }
}

//=================================================================================================

Standard_Boolean Inter2d::FuseVertices(
  const TopTools_IndexedDataMapOfShapeListOfShape& theDMVV,
  const Handle(BRepAlgo_AsDes)&                    theAsDes,
  ShapeImage&                                  theImageVV)
{
  ShapeBuilder        aBB;
  TopTools_MapOfShape aMVDone;
  Standard_Integer    i, aNb = theDMVV.Extent();
  for (i = 1; i <= aNb; ++i)
  {
    const TopoVertex& aV = TopoDS::Vertex(theDMVV.FindKey(i));
    //
    // find chain of vertices
    ShapeList aLVChain;
    MakeChain(aV, theDMVV, aMVDone, aLVChain);
    //
    if (aLVChain.Extent() < 2)
    {
      continue;
    }
    //
    // make new vertex
    TopoVertex aVNew;
    AlgoTools::MakeVertex(aLVChain, aVNew);
    //
    TopoVertex aVNewInt = TopoDS::Vertex(aVNew.Oriented(TopAbs_INTERNAL));
    //
    TopTools_ListIteratorOfListOfShape aIt(aLVChain);
    for (; aIt.More(); aIt.Next())
    {
      const TopoShape& aVOld = aIt.Value();
      // update the parameters on edges
      TopoVertex               aVOldInt = TopoDS::Vertex(aVOld.Oriented(TopAbs_INTERNAL));
      const ShapeList& aLE      = theAsDes->Ascendant(aVOld);
      //
      TopTools_ListIteratorOfListOfShape aItLE(aLE);
      for (; aItLE.More(); aItLE.Next())
      {
        const TopoEdge& aE    = TopoDS::Edge(aItLE.Value());
        Standard_Real      aTolE = BRepInspector::Tolerance(aE);
        Standard_Real      aT;
        if (!BRepInspector::Parameter(aVOldInt, aE, aT))
        {
          return Standard_False;
        }
        aBB.UpdateVertex(aVNewInt, aT, aE, aTolE);
      }
      // and replace the vertex
      theAsDes->Replace(aVOld, aVNew);
      if (theImageVV.IsImage(aVOld))
      {
        const TopoVertex& aProVertex = TopoDS::Vertex(theImageVV.ImageFrom(aVOld));
        theImageVV.Add(aProVertex, aVNew.Oriented(TopAbs_FORWARD));
      }
    }
  }
  return Standard_True;
}
