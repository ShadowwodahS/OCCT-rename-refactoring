// Created on: 1995-03-24
// Created by: Jing Cheng MEI
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

// dcl          CCI60011 : Correction of degeneratedSection
//              Improvement of SameParameter Edge to treat case of failure in BRepLib1::SameParameter
// dcl          Thu Aug 20 09:24:49 1998
//              Suppression of little faces.
// dcl          Fri Aug  7 15:27:46 1998
//                Refection of function SameParameter Edge.
//              Merge on the edge which has the less of poles.
//              Suppression of the Connected Edge function.
// dcl          Tue Jun  9 14:21:53 1998
//              Do not merge edge if they belong the same face
//              Tolerance management in VerticesAssembling
//              Tolerance management in Cutting
// dcl          Thu May 14 15:51:46 1998
//              optimization of cutting
// dcl          Thu May 7  15:51:46 1998
//              Add of cutting option
//              Add of SameParameter call

//-- lbr April 1 97
//-- dpf December 10 1997 Processing of pcurve collections

// rln 02.02.99 BUC60449 Making compilable on NT in DEB mode
// rln 02.02.99 BUC60449 Protection against exception on NT

#define TEST 1

#include <Bnd_Box2d.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <BndLib_Add3dCurve.hxx>
#include <BRep_Builder.hxx>
#include <BRep_ListOfPointRepresentation.hxx>
#include <BRep_PointOnCurve.hxx>
#include <BRep_Tool.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_TVertex.hxx>
#include <BRepBuilderAPI_BndBoxTreeSelector.hxx>
#include <BRepBuilderAPI_CellFilter.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_VertexInspector.hxx>
#include <BRepLib.hxx>
#include <BRepTools.hxx>
#include <BRepTools_Quilt.hxx>
#include <BRepTools_ReShape.hxx>
#include <Extrema_ExtPC.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomLib.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Message_ProgressScope.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <Precision.hxx>
#include <Standard_Failure.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_Type.hxx>
#include <TColgp_Array1OfVec.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_SequenceOfReal.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepBuilderAPI_Sewing, RefObject)

// #include <LocalAnalysis_SurfaceContinuity.hxx>
//=================================================================================================

Handle(GeomCurve2d) BRepBuilderAPI_Sewing::SameRange(const Handle(GeomCurve2d)& CurvePtr,
                                                      const Standard_Real         FirstOnCurve,
                                                      const Standard_Real         LastOnCurve,
                                                      const Standard_Real         RequestedFirst,
                                                      const Standard_Real RequestedLast) const
{
  Handle(GeomCurve2d) NewCurvePtr;
  try
  {

    GeomLib1::SameRange(Precision1::PConfusion(),
                       CurvePtr,
                       FirstOnCurve,
                       LastOnCurve,
                       RequestedFirst,
                       RequestedLast,
                       NewCurvePtr);
  }
  catch (ExceptionBase const& anException)
  {
#ifdef OCCT_DEBUG
    std::cout << "Exception in BRepBuilderAPI_Sewing::SameRange: ";
    anException.Print(std::cout);
    std::cout << std::endl;
#endif
    (void)anException;
  }
  return NewCurvePtr;
}

//=======================================================================
// function : WhichFace
// purpose  : Give the face whose edge is the border
//=======================================================================

TopoFace BRepBuilderAPI_Sewing::WhichFace(const TopoEdge&     theEdg,
                                             const Standard_Integer index) const
{
  TopoShape bound = theEdg;
  if (mySectionBound.IsBound(bound))
    bound = mySectionBound(bound);
  if (myBoundFaces.Contains(bound))
  {
    Standard_Integer                   i = 1;
    TopTools_ListIteratorOfListOfShape itf(myBoundFaces.FindFromKey(bound));
    for (; itf.More(); itf.Next(), i++)
      if (i == index)
        return TopoDS::Face(itf.Value());
  }
  return TopoFace();
}

//=================================================================================================

static Standard_Boolean IsClosedShape(const TopoShape& theshape,
                                      const TopoShape& v1,
                                      const TopoShape& v2)
{
  Standard_Real   TotLength = 0.0;
  ShapeExplorer aexp;
  for (aexp.Init(theshape, TopAbs_EDGE); aexp.More(); aexp.Next())
  {
    TopoEdge aedge = TopoDS::Edge(aexp.Current());
    if (aedge.IsNull())
      continue;
    TopoVertex ve1, ve2;
    TopExp1::Vertices(aedge, ve1, ve2);
    if (!ve1.IsSame(v1) && !ve1.IsSame(v2))
      continue;
    if (BRepInspector::Degenerated(aedge))
      continue;
    Standard_Real      first, last;
    Handle(GeomCurve3d) c3d = BRepInspector::Curve(TopoDS::Edge(aedge), first, last);
    if (!c3d.IsNull())
    {
      GeomAdaptor_Curve cAdapt(c3d);
      Standard_Real     length = GCPnts_AbscissaPoint::Length(cAdapt, first, last);
      TotLength += length;
      if (ve2.IsSame(v1) || ve2.IsSame(v2))
        break;
    }
  }
  if (TotLength > 0.0)
  {
    Point3d p1 = BRepInspector::Pnt(TopoDS::Vertex(v1));
    Point3d p2 = BRepInspector::Pnt(TopoDS::Vertex(v2));
    return (p1.Distance(p2) < TotLength / (1.2 * M_PI));
  }
  return Standard_False;
}

//=================================================================================================

static Standard_Boolean IsClosedByIsos(const Handle(GeomSurface)& thesurf,
                                       const Handle(GeomCurve2d)& acrv2d,
                                       const Standard_Real         f2d,
                                       const Standard_Real         l2d,
                                       const Standard_Boolean      isUIsos)
{
  Standard_Boolean isClosed = Standard_False;

  gp_Pnt2d psurf1 =
    (acrv2d->IsPeriodic() ? acrv2d->Value(f2d) : acrv2d->Value(Max(f2d, acrv2d->FirstParameter())));
  gp_Pnt2d psurf2 =
    (acrv2d->IsPeriodic() ? acrv2d->Value(l2d) : acrv2d->Value(Min(l2d, acrv2d->LastParameter())));
  Handle(GeomCurve3d) aCrv1;
  Handle(GeomCurve3d) aCrv2;
  if (isUIsos)
  {
    aCrv1 = thesurf->UIso(psurf1.X());
    aCrv2 = thesurf->UIso(psurf2.X());
  }
  else
  {
    aCrv1 = thesurf->VIso(psurf1.Y());
    aCrv2 = thesurf->VIso(psurf2.Y());
  }
  Point3d        p11, p1m, p12, p21, p2m, p22;
  Standard_Real af1 = aCrv1->FirstParameter();
  Standard_Real al1 = aCrv1->LastParameter();
  Standard_Real af2 = aCrv2->FirstParameter();
  Standard_Real al2 = aCrv2->LastParameter();
  aCrv1->D0(af1, p11);
  aCrv1->D0((af1 + al1) * 0.5, p1m);
  aCrv1->D0(al1, p12);
  aCrv2->D0(af2, p21);
  aCrv2->D0((af2 + al2) * 0.5, p2m);
  aCrv2->D0(al2, p22);
  isClosed = (((p11.XYZ() - p12.XYZ()).Modulus()
               < (p11.XYZ() - p1m.XYZ()).Modulus() - Precision1::Confusion())
              && ((p21.XYZ() - p22.XYZ()).Modulus()
                  < (p21.XYZ() - p2m.XYZ()).Modulus() - Precision1::Confusion()));
  return isClosed;
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::IsUClosedSurface(const Handle(GeomSurface)& surf,
                                                         const TopoShape&         theEdge,
                                                         const TopLoc_Location&      theloc) const
{
  Handle(GeomSurface) tmpsurf = surf;
  if (tmpsurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
    tmpsurf = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf)->BasisSurface();
  else if (tmpsurf->IsKind(STANDARD_TYPE(Geom_OffsetSurface)))
    tmpsurf = Handle(Geom_OffsetSurface)::DownCast(surf)->BasisSurface();
  else
  {
    Standard_Boolean isClosed = tmpsurf->IsUClosed();
    if (!isClosed)
    {
      Standard_Real        f2d, l2d;
      Handle(GeomCurve2d) acrv2d =
        BRepInspector::CurveOnSurface(TopoDS::Edge(theEdge), surf, theloc, f2d, l2d);
      if (!acrv2d.IsNull())
        isClosed = IsClosedByIsos(tmpsurf, acrv2d, f2d, l2d, Standard_False);
    }
    return isClosed;
  }
  return IsUClosedSurface(tmpsurf, theEdge, theloc);
  // return surf->IsUClosed();
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::IsVClosedSurface(const Handle(GeomSurface)& surf,
                                                         const TopoShape&         theEdge,
                                                         const TopLoc_Location&      theloc) const
{
  Handle(GeomSurface) tmpsurf = surf;
  if (tmpsurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
    tmpsurf = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf)->BasisSurface();
  else if (tmpsurf->IsKind(STANDARD_TYPE(Geom_OffsetSurface)))
    tmpsurf = Handle(Geom_OffsetSurface)::DownCast(surf)->BasisSurface();
  else
  {
    Standard_Boolean isClosed = tmpsurf->IsVClosed();
    if (!isClosed)
    {
      Standard_Real        f2d, l2d;
      Handle(GeomCurve2d) acrv2d =
        BRepInspector::CurveOnSurface(TopoDS::Edge(theEdge), surf, theloc, f2d, l2d);
      if (!acrv2d.IsNull())
        isClosed = IsClosedByIsos(tmpsurf, acrv2d, f2d, l2d, Standard_True);
    }
    return isClosed;
  }
  return IsVClosedSurface(tmpsurf, theEdge, theloc);
  // return surf->IsVClosed();
}

//=================================================================================================

void BRepBuilderAPI_Sewing::SameParameter(const TopoEdge& edge) const
{
  try
  {

    BRepLib1::SameParameter(edge);
  }
  catch (ExceptionBase const& anException)
  {
#ifdef OCCT_DEBUG
    std::cout << "Exception in BRepBuilderAPI_Sewing::SameParameter: ";
    anException.Print(std::cout);
    std::cout << std::endl;
#endif
    (void)anException;
  }
}

//=======================================================================
// function : SameParameterEdge
// purpose  : internal use
//           Merge the Sequence Of Section on one edge.
//           This function keep the curve3d,curve2d,range and parametrization
//           from the first section, and report and made sameparameter the
//           pcurves of the other function.
//           This function works when the are not more than two Pcurves
//           on a same face.
//=======================================================================

TopoEdge BRepBuilderAPI_Sewing::SameParameterEdge(const TopoShape&              edge,
                                                     const TopTools_SequenceOfShape&  seqEdges,
                                                     const TColStd_SequenceOfBoolean& seqForward,
                                                     TopTools_MapOfShape&             mapMerged,
                                                     const Handle(BRepTools_ReShape)& locReShape)
{
  // Retrieve reference section
  TopoShape aTmpShape = myReShape->Apply(edge); // for porting
  TopoEdge  Edge1     = TopoDS::Edge(aTmpShape);
  aTmpShape              = locReShape->Apply(Edge1);
  if (locReShape != myReShape)
    Edge1 = TopoDS::Edge(aTmpShape);
  Standard_Boolean isDone = Standard_False;

  // Create data structures for temporary merged edges
  ShapeList listFaces1;
  TopTools_MapOfShape  MergedFaces;

  if (mySewing)
  {

    // Fill MergedFaces with faces of Edge1
    TopoShape bnd1 = edge;
    if (mySectionBound.IsBound(bnd1))
      bnd1 = mySectionBound(bnd1);
    if (myBoundFaces.Contains(bnd1))
    {
      TopTools_ListIteratorOfListOfShape itf(myBoundFaces.FindFromKey(bnd1));
      for (; itf.More(); itf.Next())
        if (MergedFaces.Add(itf.Value()))
          listFaces1.Append(itf.Value());
    }
  }
  else
  {

    // Create presentation edge
    TopoVertex V1, V2;
    TopExp1::Vertices(Edge1, V1, V2);
    if (myVertexNode.Contains(V1))
      V1 = TopoDS::Vertex(myVertexNode.FindFromKey(V1));
    if (myVertexNode.Contains(V2))
      V2 = TopoDS::Vertex(myVertexNode.FindFromKey(V2));

    TopoEdge NewEdge = Edge1;
    NewEdge.EmptyCopy();

    // Add the vertices
    ShapeBuilder aBuilder;
    TopoShape anEdge = NewEdge.Oriented(TopAbs_FORWARD);
    aBuilder.Add(anEdge, V1.Oriented(TopAbs_FORWARD));
    aBuilder.Add(anEdge, V2.Oriented(TopAbs_REVERSED));

    Edge1 = NewEdge;
  }

  Standard_Boolean isForward = Standard_True;

  // Merge candidate sections
  for (Standard_Integer i = 1; i <= seqEdges.Length(); i++)
  {

    // Retrieve candidate section
    const TopoShape& oedge2 = seqEdges(i);

    if (mySewing)
    {

      aTmpShape         = myReShape->Apply(oedge2); // for porting
      TopoEdge Edge2 = TopoDS::Edge(aTmpShape);
      aTmpShape         = locReShape->Apply(Edge2);
      if (locReShape != myReShape)
        Edge2 = TopoDS::Edge(aTmpShape);

      // Calculate relative orientation
      Standard_Boolean Orientation = seqForward(i);
      if (!isForward)
        Orientation = !Orientation;

      // Retrieve faces information for the second edge
      TopoShape bnd2 = oedge2;
      if (mySectionBound.IsBound(bnd2))
        bnd2 = mySectionBound(bnd2);
      if (!myBoundFaces.Contains(bnd2))
        continue; // Skip floating edge
      const ShapeList& listFaces2 = myBoundFaces.FindFromKey(bnd2);

      Standard_Integer whichSec = 1; // Indicates on which edge the pCurve has been reported
      TopoEdge      NewEdge =
        SameParameterEdge(Edge1, Edge2, listFaces1, listFaces2, Orientation, whichSec);
      if (NewEdge.IsNull())
        continue;

      // Record faces information for the temporary merged edge
      TopTools_ListIteratorOfListOfShape itf(listFaces2);
      for (; itf.More(); itf.Next())
        if (MergedFaces.Add(itf.Value()))
          listFaces1.Append(itf.Value());

      // Record merged section orientation
      if (!Orientation && whichSec != 1)
        isForward = isForward ? Standard_False : Standard_True;
      Edge1 = NewEdge;
    }

    // Append actually merged edge
    mapMerged.Add(oedge2);
    isDone = Standard_True;

    if (!myNonmanifold)
      break;
  }

  if (isDone)
  {
    // Change result orientation
    Edge1.Orientation(isForward ? TopAbs_FORWARD : TopAbs_REVERSED);
  }
  else
    Edge1.Nullify();

  return Edge1;
}

//=================================================================================================

static Standard_Boolean findNMVertices(const TopoEdge&        theEdge,
                                       TopTools_SequenceOfShape& theSeqNMVert,
                                       TColStd_SequenceOfReal&   theSeqPars)
{
  TopoDS_Iterator aItV(theEdge, Standard_False);
  for (; aItV.More(); aItV.Next())
  {
    if (aItV.Value().Orientation() == TopAbs_INTERNAL
        || aItV.Value().Orientation() == TopAbs_EXTERNAL)
      theSeqNMVert.Append(aItV.Value());
  }
  Standard_Integer nbV = theSeqNMVert.Length();
  if (!nbV)
    return Standard_False;
  Standard_Real      first, last;
  Handle(GeomCurve3d) c3d = BRepInspector::Curve(theEdge, first, last);
  GeomAdaptor_Curve  GAC(c3d);
  Extrema_ExtPC      locProj;
  locProj.Initialize(GAC, first, last);
  Point3d pfirst = GAC.Value(first), plast = GAC.Value(last);

  for (Standard_Integer i = 1; i <= nbV; i++)
  {
    TopoVertex aV = TopoDS::Vertex(theSeqNMVert.Value(i));
    Point3d        pt = BRepInspector::Pnt(aV);

    Standard_Real distF2 = pfirst.SquareDistance(pt);
    Standard_Real distL2 = plast.SquareDistance(pt);
    Standard_Real apar   = (distF2 > distL2 ? last : first);
    // Project current point on curve
    locProj.Perform(pt);
    if (locProj.IsDone() && locProj.NbExt() > 0)
    {
      Standard_Real    dist2Min = Min(distF2, distL2);
      Standard_Integer ind, indMin = 0;
      for (ind = 1; ind <= locProj.NbExt(); ind++)
      {
        Standard_Real dProj2 = locProj.SquareDistance(ind);
        if (dProj2 < dist2Min)
        {
          indMin   = ind;
          dist2Min = dProj2;
        }
      }
      if (indMin)
        apar = locProj.Point(indMin).Parameter();

      theSeqPars.Append(apar);
    }
  }
  return Standard_True;
}

static void ComputeToleranceVertex(TopoVertex theV1, TopoVertex theV2, TopoVertex& theNewV)
{
  Standard_Integer m, n;
  Standard_Real    aR[2], dR, aD, aEps;
  TopoVertex    aV[2];
  Point3d           aP[2];
  ShapeBuilder     aBB;
  //
  aEps  = RealEpsilon();
  aV[0] = theV1;
  aV[1] = theV2;
  for (m = 0; m < 2; ++m)
  {
    aP[m] = BRepInspector::Pnt(aV[m]);
    aR[m] = BRepInspector::Tolerance(aV[m]);
  }
  //
  m = 0; // max R
  n = 1; // min R
  if (aR[0] < aR[1])
  {
    m = 1;
    n = 0;
  }
  //
  dR = aR[m] - aR[n]; // dR >= 0.
  Vector3d aVD(aP[m], aP[n]);
  aD = aVD.Magnitude();
  //
  if (aD <= dR || aD < aEps)
  {
    aBB.MakeVertex(theNewV, aP[m], aR[m]);
  }
  else
  {
    Standard_Real aRr;
    Coords3d        aXYZr;
    Point3d        aPr;
    //
    aRr   = 0.5 * (aR[m] + aR[n] + aD);
    aXYZr = 0.5 * (aP[m].XYZ() + aP[n].XYZ() - aVD.XYZ() * (dR / aD));
    aPr.SetXYZ(aXYZr);
    //
    aBB.MakeVertex(theNewV, aPr, aRr);
  }
  return;
}

static void ComputeToleranceVertex(TopoVertex  theV1,
                                   TopoVertex  theV2,
                                   TopoVertex  theV3,
                                   TopoVertex& theNewV)
{
  Standard_Real aDi, aDmax;
  Point3d        aCenter;
  Point3d        aP[3];
  Standard_Real aR[3];
  TopoVertex aV[3];
  Coords3d        aXYZ(0., 0., 0.);
  aV[0] = theV1;
  aV[1] = theV2;
  aV[2] = theV3;
  for (Standard_Integer i = 0; i < 3; ++i)
  {
    aP[i] = BRepInspector::Pnt(aV[i]);
    aR[i] = BRepInspector::Tolerance(aV[i]);
    aXYZ  = aXYZ + aP[i].XYZ();
  }
  //
  aXYZ.Divide(3.0);
  aCenter.SetXYZ(aXYZ);
  //
  aDmax = -1.;
  for (Standard_Integer i = 0; i < 3; ++i)
  {
    aDi = aCenter.Distance(aP[i]);
    aDi += aR[i];
    if (aDi > aDmax)
      aDmax = aDi;
  }

  ShapeBuilder aBB;
  aBB.MakeVertex(theNewV, aCenter, aDmax);
  return;
}

TopoEdge BRepBuilderAPI_Sewing::SameParameterEdge(const TopoEdge&          edgeFirst,
                                                     const TopoEdge&          edgeLast,
                                                     const ShapeList& listFacesFirst,
                                                     const ShapeList& listFacesLast,
                                                     const Standard_Boolean      secForward,
                                                     Standard_Integer&           whichSec,
                                                     const Standard_Boolean      firstCall)
{
  // Do not process floating edges
  if (!listFacesFirst.Extent() || !listFacesLast.Extent())
    return TopoEdge();

  // Sort input edges
  TopoEdge edge1, edge2;
  if (firstCall)
  {
    // Take the longest edge as first
    Standard_Real      f, l;
    Handle(GeomCurve3d) c3d1 = BRepInspector::Curve(TopoDS::Edge(edgeFirst), f, l);
    GeomAdaptor_Curve  cAdapt1(c3d1);
    Standard_Real      len1 = GCPnts_AbscissaPoint::Length(cAdapt1, f, l);
    Handle(GeomCurve3d) c3d2 = BRepInspector::Curve(TopoDS::Edge(edgeLast), f, l);
    GeomAdaptor_Curve  cAdapt2(c3d2);
    Standard_Real      len2 = GCPnts_AbscissaPoint::Length(cAdapt2, f, l);
    if (len1 < len2)
    {
      edge1    = edgeLast;
      edge2    = edgeFirst;
      whichSec = 2;
    }
    else
    {
      edge1    = edgeFirst;
      edge2    = edgeLast;
      whichSec = 1;
    }
  }
  else
  {
    if (whichSec == 1)
    {
      edge1    = edgeLast;
      edge2    = edgeFirst;
      whichSec = 2;
    }
    else
    {
      edge1    = edgeFirst;
      edge2    = edgeLast;
      whichSec = 1;
    }
  }

  Standard_Real first, last;
  BRepInspector::Range(edge1, first, last);
  ShapeBuilder aBuilder;

  // To keep NM vertices on edge
  TopTools_SequenceOfShape aSeqNMVert;
  TColStd_SequenceOfReal   aSeqNMPars;
  findNMVertices(edge1, aSeqNMVert, aSeqNMPars);
  findNMVertices(edge2, aSeqNMVert, aSeqNMPars);

  // Create new edge
  TopoEdge edge;
  aBuilder.MakeEdge(edge);
  edge.Orientation(edge1.Orientation());

  // Retrieve edge curve
  TopLoc_Location    loc3d;
  Standard_Real      first3d, last3d;
  Handle(GeomCurve3d) c3d = BRepInspector::Curve(edge1, loc3d, first3d, last3d);
  if (!loc3d.IsIdentity())
  {
    c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
    c3d->Transform(loc3d.Transformation());
  }
  aBuilder.UpdateEdge(edge, c3d, BRepInspector::Tolerance(edge1));
  aBuilder.Range(edge, first, last);
  aBuilder.SameRange(edge, Standard_False); // Standard_True
  aBuilder.SameParameter(edge, Standard_False);
  // Create and add new vertices
  {
    TopoVertex V1New, V2New;

    // Retrieve original vertices from edges
    TopoVertex V11, V12, V21, V22;
    TopExp1::Vertices(edge1, V11, V12);
    TopExp1::Vertices(edge2, V21, V22);

    // check that edges merged valid way (for edges having length less than specified
    // tolerance
    //  Check if edges are closed
    Standard_Boolean isClosed1 = V11.IsSame(V12);
    Standard_Boolean isClosed2 = V21.IsSame(V22);
    if (!isClosed1 && !isClosed2)
    {
      if (secForward)
      {
        if (V11.IsSame(V22) || V12.IsSame(V21))
          return TopoEdge();
      }
      else
      {
        if (V11.IsSame(V21) || V12.IsSame(V22))
          return TopoEdge();
      }
    }

    // szv: do not reshape here!!!
    // V11 = TopoDS::Vertex(myReShape->Apply(V11));
    // V12 = TopoDS::Vertex(myReShape->Apply(V12));
    // V21 = TopoDS::Vertex(myReShape->Apply(V21));
    // V22 = TopoDS::Vertex(myReShape->Apply(V22));

    // Standard_Boolean isRev = Standard_False;
    if (isClosed1 || isClosed2)
    {
      // at least one of the edges is closed
      if (isClosed1 && isClosed2)
      {
        // both edges are closed
        ComputeToleranceVertex(V11, V21, V1New);
      }
      else if (isClosed1)
      {
        // only first edge is closed
        ComputeToleranceVertex(V22, V21, V11, V1New);
      }
      else
      {
        // only second edge is closed
        ComputeToleranceVertex(V11, V12, V21, V1New);
      }
      V2New = V1New;
    }
    else
    {
      // both edges are open
      Standard_Boolean isOldFirst = (secForward ? V11.IsSame(V21) : V11.IsSame(V22));
      Standard_Boolean isOldLast  = (secForward ? V12.IsSame(V22) : V12.IsSame(V21));
      if (secForward)
      {
        // case if vertices already sewed
        if (!isOldFirst)
        {
          ComputeToleranceVertex(V11, V21, V1New);
        }
        if (!isOldLast)
        {
          ComputeToleranceVertex(V12, V22, V2New);
        }
      }
      else
      {
        if (!isOldFirst)
        {
          ComputeToleranceVertex(V11, V22, V1New);
        }
        if (!isOldLast)
        {
          ComputeToleranceVertex(V12, V21, V2New);
        }
      }
      if (isOldFirst)
        V1New = V11;

      if (isOldLast)
        V2New = V12;
    }
    // Add the vertices in the good sense
    TopoShape anEdge = edge.Oriented(TopAbs_FORWARD);
    // clang-format off
    TopoShape aLocalEdge = V1New.Oriented(TopAbs_FORWARD); //(listNode.First()).Oriented(TopAbs_FORWARD);
    // clang-format on
    aBuilder.Add(anEdge, aLocalEdge);
    aLocalEdge = V2New.Oriented(TopAbs_REVERSED); //(listNode.Last()).Oriented(TopAbs_REVERSED);
    aBuilder.Add(anEdge, aLocalEdge);

    Standard_Integer k = 1;
    for (; k <= aSeqNMVert.Length(); k++)
      aBuilder.Add(anEdge, aSeqNMVert.Value(k));
  }

  // Retrieve second PCurves
  TopLoc_Location      loc2;
  Handle(GeomSurface) surf2;

  // Handle(GeomCurve2d) c2d2, c2d21;
  //   Standard_Real firstOld, lastOld;

  TopTools_ListIteratorOfListOfShape itf2;
  if (whichSec == 1)
    itf2.Initialize(listFacesLast);
  else
    itf2.Initialize(listFacesFirst);
  Standard_Boolean isResEdge = Standard_False;
  TopoFace      fac2;
  for (; itf2.More(); itf2.Next())
  {
    Handle(GeomCurve2d) c2d2, c2d21;
    Standard_Real        firstOld, lastOld;
    fac2 = TopoDS::Face(itf2.Value());

    surf2 = BRepInspector::Surface(fac2, loc2);
    Standard_Boolean isSeam2 =
      ((IsUClosedSurface(surf2, edge2, loc2) || IsVClosedSurface(surf2, edge2, loc2))
       && BRepInspector::IsClosed(TopoDS::Edge(edge2), fac2));
    if (isSeam2)
    {
      if (!myNonmanifold)
        return TopoEdge();
      TopoShape aTmpShape = edge2.Reversed(); // for porting
      c2d21 = BRepInspector::CurveOnSurface(TopoDS::Edge(aTmpShape), fac2, firstOld, lastOld);
    }
    c2d2 = BRepInspector::CurveOnSurface(edge2, fac2, firstOld, lastOld);
    if (c2d2.IsNull() && c2d21.IsNull())
      continue;

    if (!c2d21.IsNull())
    {
      c2d21 = Handle(GeomCurve2d)::DownCast(c2d21->Copy());
      if (!secForward)
      {
        if (c2d21->IsKind(STANDARD_TYPE(Geom2d_Line)))
          c2d21 = new Geom2d_TrimmedCurve(c2d21, firstOld, lastOld);
        Standard_Real first2d = firstOld; // c2dTmp->FirstParameter(); BUG USA60321
        Standard_Real last2d  = lastOld;  // c2dTmp->LastParameter();
        firstOld              = c2d21->ReversedParameter(last2d);
        lastOld               = c2d21->ReversedParameter(first2d);
        c2d21->Reverse();
      }
      c2d21 = SameRange(c2d21, firstOld, lastOld, first, last);
    }

    // Make second PCurve sameRange with the 3d curve
    c2d2 = Handle(GeomCurve2d)::DownCast(c2d2->Copy());

    if (!secForward)
    {
      if (c2d2->IsKind(STANDARD_TYPE(Geom2d_Line)))
        c2d2 = new Geom2d_TrimmedCurve(c2d2, firstOld, lastOld);
      Standard_Real first2d = firstOld;
      Standard_Real last2d  = lastOld;
      firstOld              = c2d2->ReversedParameter(last2d);
      lastOld               = c2d2->ReversedParameter(first2d);
      c2d2->Reverse();
    }

    c2d2 = SameRange(c2d2, firstOld, lastOld, first, last);
    if (c2d2.IsNull())
      continue;

    // Add second PCurve
    Standard_Boolean   isSeam = Standard_False;
    TopAbs_Orientation Ori    = TopAbs_FORWARD;
    // Handle(GeomCurve2d) c2d1, c2d11;

    TopTools_ListIteratorOfListOfShape itf1;
    if (whichSec == 1)
      itf1.Initialize(listFacesFirst);
    else
      itf1.Initialize(listFacesLast);
    for (; itf1.More() && !isSeam; itf1.Next())
    {
      Handle(GeomCurve2d) c2d1, c2d11;
      const TopoFace&   fac1 = TopoDS::Face(itf1.Value());

      TopLoc_Location      loc1;
      Handle(GeomSurface) surf1 = BRepInspector::Surface(fac1, loc1);

      Standard_Real    first2d, last2d;
      Standard_Boolean isSeam1 =
        ((IsUClosedSurface(surf1, edge1, loc1) || IsVClosedSurface(surf1, edge1, loc1))
         && BRepInspector::IsClosed(TopoDS::Edge(edge1), fac1));
      c2d1 = BRepInspector::CurveOnSurface(edge1, fac1, first2d, last2d);
      Ori  = edge1.Orientation();
      if (fac1.Orientation() == TopAbs_REVERSED)
        Ori = TopAbs1::Reverse(Ori);

      if (isSeam1)
      {
        if (!myNonmanifold)
          return TopoEdge();
        TopoShape aTmpShape = edge1.Reversed(); // for porting
        c2d11 = BRepInspector::CurveOnSurface(TopoDS::Edge(aTmpShape), fac1, first2d, last2d);
        // if(fac1.Orientation() == TopAbs_REVERSED) //
        if (Ori == TopAbs_FORWARD)
          aBuilder.UpdateEdge(edge, c2d1, c2d11, fac1, 0);
        else
          aBuilder.UpdateEdge(edge, c2d11, c2d1, fac1, 0);
      }
      else
        aBuilder.UpdateEdge(edge, c2d1, fac1, 0);

      if (c2d1.IsNull() && c2d11.IsNull())
        continue;

      if (surf2 == surf1)
      {
        // Merge sections which are on the same face
        if (!loc2.IsDifferent(loc1))
        {
          Standard_Boolean uclosed = IsUClosedSurface(surf2, edge2, loc2);
          Standard_Boolean vclosed = IsVClosedSurface(surf2, edge2, loc2);
          if (uclosed || vclosed)
          {
            Standard_Real pf = c2d1->FirstParameter();
            //	    Standard_Real pl = c2d1->LastParameter();
            gp_Pnt2d p1n = c2d1->Value(Max(first, pf));
            //	    gp_Pnt2d p2n = c2d1->Value(Min(pl,last));
            gp_Pnt2d      p21n  = c2d2->Value(Max(first, c2d2->FirstParameter()));
            gp_Pnt2d      p22n  = c2d2->Value(Min(last, c2d2->LastParameter()));
            Standard_Real aDist = Min(p1n.Distance(p21n), p1n.Distance(p22n));
            Standard_Real U1, U2, V1, V2;
            surf2->Bounds(U1, U2, V1, V2);
            isSeam = ((uclosed && aDist > 0.75 * (fabs(U2 - U1)))
                      || (vclosed && aDist > 0.75 * (fabs(V2 - V1))));
            if (!isSeam && BRepInspector::IsClosed(TopoDS::Edge(edge), fac1))
              continue;
          }
        }
      }

      isResEdge = Standard_True;
      if (isSeam)
      {
        if (Ori == TopAbs_FORWARD)
          aBuilder.UpdateEdge(edge, c2d1, c2d2, surf2, loc2, Precision1::Confusion());
        else
          aBuilder.UpdateEdge(edge, c2d2, c2d1, surf2, loc2, Precision1::Confusion());
      }
      else if (isSeam2)
      {
        TopAbs_Orientation InitOri = edge2.Orientation();
        TopAbs_Orientation SecOri  = edge.Orientation();
        if (fac2.Orientation() == TopAbs_REVERSED)
        {

          InitOri = TopAbs1::Reverse(InitOri);
          SecOri  = TopAbs1::Reverse(SecOri);
        }
        if (!secForward)
          InitOri = TopAbs1::Reverse(InitOri);

        if (InitOri == TopAbs_FORWARD)
          aBuilder.UpdateEdge(edge, c2d2, c2d21, surf2, loc2, Precision1::Confusion());
        else
          aBuilder.UpdateEdge(edge, c2d21, c2d2, surf2, loc2, Precision1::Confusion());
      }
      else
      {
        aBuilder.UpdateEdge(edge, c2d2, surf2, loc2, Precision1::Confusion());
      }
    }
  }
  Standard_Real    tolReached = Precision1::Infinite();
  Standard_Boolean isSamePar  = Standard_False;
  try
  {
    if (isResEdge)
      SameParameter(edge);

    if (BRepInspector::SameParameter(edge))
    {
      isSamePar  = Standard_True;
      tolReached = BRepInspector::Tolerance(edge);
    }
  }

  catch (ExceptionBase const&)
  {
    isSamePar = Standard_False;
  }

  if (firstCall && (!isResEdge || !isSamePar || tolReached > myTolerance))
  {
    Standard_Integer whichSecn = whichSec;
    // Try to merge on the second section
    Standard_Boolean second_ok = Standard_False;
    TopoEdge      s_edge    = SameParameterEdge(edgeFirst,
                                           edgeLast,
                                           listFacesFirst,
                                           listFacesLast,
                                           secForward,
                                           whichSecn,
                                           Standard_False);
    if (!s_edge.IsNull())
    {
      Standard_Real tolReached_2 = BRepInspector::Tolerance(s_edge);
      second_ok                  = (BRepInspector::SameParameter(s_edge) && tolReached_2 < tolReached);
      if (second_ok)
      {
        edge       = s_edge;
        whichSec   = whichSecn;
        tolReached = tolReached_2;
      }
    }

    if (!second_ok && !edge.IsNull())
    {

      GeomAdaptor_Curve c3dAdapt(c3d);

      // Discretize edge curve
      Standard_Integer   i, j, nbp = 23;
      Standard_Real      deltaT = (last3d - first3d) / (nbp - 1);
      TColgp_Array1OfPnt c3dpnt(1, nbp);
      for (i = 1; i <= nbp; i++)
        c3dpnt(i) = c3dAdapt.Value(first3d + (i - 1) * deltaT);

      Standard_Real    dist = 0., maxTol = -1.0;
      Standard_Boolean more = Standard_True;

      for (j = 1; more; j++)
      {
        Handle(GeomCurve2d) c2d2;
        BRepInspector::CurveOnSurface(edge, c2d2, surf2, loc2, first, last, j);

        more = !c2d2.IsNull();
        if (more)
        {
          Handle(GeomSurface) aS = surf2;
          if (!loc2.IsIdentity())
            aS = Handle(GeomSurface)::DownCast(surf2->Transformed(loc2));

          Standard_Real dist2 = 0.;
          deltaT              = (last - first) / (nbp - 1);
          for (i = 1; i <= nbp; i++)
          {
            gp_Pnt2d aP2d = c2d2->Value(first + (i - 1) * deltaT);
            Point3d   aP2(0., 0., 0.);
            aS->D0(aP2d.X(), aP2d.Y(), aP2);
            Point3d aP1 = c3dpnt(i);
            dist       = aP2.SquareDistance(aP1);
            if (dist > dist2)
              dist2 = dist;
          }
          maxTol = Max(sqrt(dist2) * (1. + 1e-7), Precision1::Confusion());
        }
      }
      if (maxTol >= 0. && maxTol < tolReached)
      {
        if (tolReached > MaxTolerance())
        {
          // Set tolerance directly to overwrite too large tolerance
          static_cast<BRep_TEdge*>(edge.TShape().get())->Tolerance(maxTol);
        }
        else
        {
          // just update tolerance with computed distance
          aBuilder.UpdateEdge(edge, maxTol);
        }
      }
      aBuilder.SameParameter(edge, Standard_True);
    }
  }

  Standard_Real tolEdge1 = BRepInspector::Tolerance(edge);
  if (tolEdge1 > MaxTolerance())
    edge.Nullify();
  return edge;
}

//=================================================================================================

void BRepBuilderAPI_Sewing::EvaluateAngulars(TopTools_SequenceOfShape& sequenceSec,
                                             TColStd_Array1OfBoolean&  secForward,
                                             TColStd_Array1OfReal&     tabAng,
                                             const Standard_Integer    indRef) const
{
  tabAng.Init(-1.0);

  Standard_Integer i, j, npt = 4, lengSec = sequenceSec.Length();

  TopoEdge          edge;
  TopoFace          face;
  TopLoc_Location      loc;
  Standard_Real        first, last;
  Handle(GeomCurve3d)   c3d;
  Handle(GeomCurve2d) c2d;
  Handle(GeomSurface) surf;
  TColgp_Array1OfVec   normRef(1, npt);

  for (i = indRef; i <= lengSec; i++)
  {

    edge = TopoDS::Edge(sequenceSec(i));

    TopoShape bnd = edge;
    if (mySectionBound.IsBound(bnd))
      bnd = mySectionBound(bnd);
    if (myBoundFaces.Contains(bnd))
    {
      face = TopoDS::Face(myBoundFaces.FindFromKey(bnd).First());
      surf = BRepInspector::Surface(face, loc);
      if (!loc.IsIdentity())
      {
        surf = Handle(GeomSurface)::DownCast(surf->Copy());
        surf->Transform(loc.Transformation());
      }
      c2d = BRepInspector::CurveOnSurface(edge, face, first, last);
    }
    else if (i == indRef)
      return;
    else
      continue;

    c3d = BRepInspector::Curve(edge, loc, first, last);
    if (!loc.IsIdentity())
    {
      c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
      c3d->Transform(loc.Transformation());
    }

    GeomAdaptor_Curve      adapt(c3d);
    GCPnts_UniformAbscissa uniAbs(adapt, npt, first, last);

    Standard_Real    cumulateAngular = 0.0;
    Standard_Integer nbComputedAngle = 0;

    for (j = 1; j <= npt; j++)
    {
      gp_Pnt2d P;
      c2d->D0(uniAbs.Parameter((secForward(i) || i == indRef) ? j : npt - j + 1), P);
      Vector3d w1, w2;
      Point3d unused;
      surf->D1(P.X(), P.Y(), unused, w1, w2);
      Vector3d n = w1 ^ w2; // Compute the normal vector
      if (i == indRef)
        normRef(j) = n;
      else if ((n.Magnitude() > gp1::Resolution()) && (normRef(j).Magnitude() > gp1::Resolution()))
      {
        nbComputedAngle++;
        Standard_Real angular = n.Angle(normRef(j));
        if (angular > M_PI / 2.)
          angular = M_PI - angular;
        cumulateAngular += angular;
      }
    }

    if (nbComputedAngle)
      tabAng(i) = cumulateAngular / ((Standard_Real)nbComputedAngle);
  }
}

//=======================================================================
// function : EvaluateDistances
// purpose  : internal use
// Evaluate distance between edges with indice indRef and the following edges in the list
// Remarks (lengSec - indRef) must be >= 1
//=======================================================================
void BRepBuilderAPI_Sewing::EvaluateDistances(TopTools_SequenceOfShape& sequenceSec,
                                              TColStd_Array1OfBoolean&  secForward,
                                              TColStd_Array1OfReal&     tabDst,
                                              TColStd_Array1OfReal&     arrLen,
                                              TColStd_Array1OfReal&     tabMinDist,
                                              const Standard_Integer    indRef) const
{
  secForward.Init(Standard_True);
  tabDst.Init(-1.0);
  arrLen.Init(0.);
  tabMinDist.Init(Precision1::Infinite());
  const Standard_Integer npt = 8; // Number of points for curve discretization
  TColgp_Array1OfPnt     ptsRef(1, npt), ptsSec(1, npt);

  Standard_Integer     i, j, lengSec = sequenceSec.Length();
  TColgp_SequenceOfPnt seqSec;

  Handle(GeomCurve3d) c3dRef;
  Standard_Real      firstRef = 0., lastRef = 0.;

  for (i = indRef; i <= lengSec; i++)
  {

    // reading of the edge (attention for the first one: reference)
    const TopoEdge& sec = TopoDS::Edge(sequenceSec(i));

    TopLoc_Location    loc;
    Standard_Real      first, last;
    Handle(GeomCurve3d) c3d = BRepInspector::Curve(sec, loc, first, last);
    if (c3d.IsNull())
      continue;
    if (!loc.IsIdentity())
    {
      c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
      c3d->Transform(loc.Transformation());
    }

    if (i == indRef)
    {
      c3dRef   = c3d;
      firstRef = first;
      lastRef  = last;
    }

    Standard_Real dist = Precision1::Infinite(), distFor = -1.0, distRev = -1.0;
    Standard_Real aMinDist = Precision1::Infinite();

    Standard_Real T, deltaT = (last - first) / (npt - 1);
    Standard_Real aLenSec2 = 0.;

    Standard_Integer nbFound = 0;
    for (j = 1; j <= npt; j++)
    {

      // Uniform parameter on curve
      if (j == 1)
        T = first;
      else if (j == npt)
        T = last;
      else
        T = first + (j - 1) * deltaT;

      // Take point on curve
      Point3d pt = c3d->Value(T);

      if (i == indRef)
      {
        ptsRef(j) = pt;
        if (j > 1)
          aLenSec2 += pt.SquareDistance(ptsRef(j - 1));
      }
      else
      {
        ptsSec(j) = pt;
        // protection to avoid merging with small sections
        if (j > 1)
          aLenSec2 += pt.SquareDistance(ptsSec(j - 1));
        // To evaluate mutual orientation and distance
        dist = pt.Distance(ptsRef(j));
        if (aMinDist > dist)
          aMinDist = dist;
        if (distFor < dist)
          distFor = dist;
        dist = pt.Distance(ptsRef(npt - j + 1));

        if (aMinDist > dist)
          aMinDist = dist;
        if (distRev < dist)
          distRev = dist;

        // Check that point lays between vertices of reference curve
        const Point3d& p11 = ptsRef(1);
        const Point3d& p12 = ptsRef(npt);
        const Vector3d  aVec1(pt, p11);
        const Vector3d  aVec2(pt, p12);
        const Vector3d  aVecRef(p11, p12);
        if ((aVecRef * aVec1) * (aVecRef * aVec2) < 0.)
          nbFound++;
      }
    }

    Standard_Real aLenSec = sqrt(aLenSec2);

    // if(aLenSec < myMinTolerance )
    //  continue;
    arrLen.SetValue(i, aLenSec);
    // Record mutual orientation
    Standard_Boolean isForward = (distFor < distRev); // szv debug: <=
    secForward(i)              = isForward;

    dist = (isForward ? distFor : distRev);
    if (i == indRef || (dist < myTolerance && nbFound >= npt * 0.5))
    {
      tabDst(i)     = dist;
      tabMinDist(i) = aMinDist;
    }
    else
    {
      nbFound = 0, aMinDist = Precision1::Infinite(), dist = -1;
      TColgp_Array1OfPnt   arrProj(1, npt);
      TColStd_Array1OfReal arrDist(1, npt), arrPara(1, npt);
      if (arrLen(indRef) >= arrLen(i))
        ProjectPointsOnCurve(ptsSec,
                             c3dRef,
                             firstRef,
                             lastRef,
                             arrDist,
                             arrPara,
                             arrProj,
                             Standard_False);
      else
        ProjectPointsOnCurve(ptsRef, c3d, first, last, arrDist, arrPara, arrProj, Standard_False);
      for (j = 1; j <= npt; j++)
      {
        if (arrDist(j) < 0.)
          continue;
        if (dist < arrDist(j))
          dist = arrDist(j);
        if (aMinDist > arrDist(j))
          aMinDist = arrDist(j);
        nbFound++;
      }
      if (nbFound > 1)
      {
        tabDst(i)     = dist;
        tabMinDist(i) = aMinDist;
      }
    }
  }

  /*
  // Project distant points
  Standard_Integer nbFailed = seqSec.Length();
  if (!nbFailed) return;

  TColgp_Array1OfPnt arrPnt(1, nbFailed), arrProj(1, nbFailed);
  for (i = 1; i <= nbFailed; i++) arrPnt(i) = seqSec(i); seqSec.Clear();
  TColStd_Array1OfReal arrDist(1, nbFailed), arrPara(1, nbFailed);

  ProjectPointsOnCurve(arrPnt,c3dRef,firstRef,lastRef,arrDist,arrPara,arrProj,Standard_False);

  // Process distant sections
  Standard_Integer idx1 = 1;
  for (i = indRef + 1; i <= lengSec; i++) {

    // Skip section if already evaluated
    if (tabDst(i) >= 0.0) continue;

    Standard_Real dist, distMax = -1.0, aMinDist = Precision1::Infinite();

    Standard_Integer idx2 = (idx1 - 1)*npt;

    for (j = 1; j <= npt; j++) {

      dist = arrDist(idx2 + j);
      // If point is not projected - stop evaluation
      if (dist < 0.0) { distMax = -1.0; break; }
      if (distMax < dist) distMax = dist;
      if(aMinDist > dist) aMinDist = dist;
    }

    // If section is close - record distance
    if (distMax >= 0.0) {
      if (secForward(i)) {
        dist = arrPnt(idx2+1).Distance(ptsRef(1));
        if (distMax < dist) distMax = dist;
        if(aMinDist > dist) aMinDist = dist;
        dist = arrPnt(idx2+npt).Distance(ptsRef(npt));
        if (distMax < dist) distMax = dist;
        if(aMinDist > dist) aMinDist = dist;
      }
      else {
        dist = arrPnt(idx2+1).Distance(ptsRef(npt));
        if (distMax < dist) distMax = dist;
        if(aMinDist > dist) aMinDist = dist;
        dist = arrPnt(idx2+npt).Distance(ptsRef(1));
        if (distMax < dist) distMax = dist;
        if(aMinDist > dist) aMinDist = dist;
      }

      if (distMax < myTolerance)
      {
        tabDst(i) = distMax;
        tabMinDist(i) = aMinDist;
      }
    }

    idx1++; // To the next distant curve
  }*/
}

//=======================================================================
// function : IsMergedClosed
// purpose  :  internal use
//=======================================================================

Standard_Boolean BRepBuilderAPI_Sewing::IsMergedClosed(const TopoEdge& Edge1,
                                                       const TopoEdge& Edge2,
                                                       const TopoFace& face) const
{
  // Check for closed surface
  TopLoc_Location      loc;
  Handle(GeomSurface) surf      = BRepInspector::Surface(face, loc);
  Standard_Boolean     isUClosed = IsUClosedSurface(surf, Edge1, loc);
  Standard_Boolean     isVClosed = IsVClosedSurface(surf, Edge1, loc);
  if (!isUClosed && !isVClosed)
    return Standard_False;
  // Check condition on closed surface
  /*
  Standard_Real first1,last1,first2,last2;
  Handle(GeomCurve3d) C3d1 = BRepInspector::Curve(Edge1,first1,last1);
  Handle(GeomCurve3d) C3d2 = BRepInspector::Curve(Edge2,first2,last2);
  if (C3d1.IsNull() || C3d2.IsNull()) return Standard_False;
  */
  Standard_Real        first2d1, last2d1, first2d2, last2d2;
  Handle(GeomCurve2d) C2d1 = BRepInspector::CurveOnSurface(Edge1, face, first2d1, last2d1);
  Handle(GeomCurve2d) C2d2 = BRepInspector::CurveOnSurface(Edge2, face, first2d2, last2d2);
  if (C2d1.IsNull() || C2d2.IsNull())
    return Standard_False;
  /*
  Point3d p1 = C3d1->Value(0.5*(first1 + last1));
  Point3d p2 = C3d1->Value(0.5*(first2 + last2));
  Standard_Real dist = p1.Distance(p2);
  gp_Pnt2d p12d = C2d1->Value(0.5*(first2d1 + last2d1));
  gp_Pnt2d p22d = C2d1->Value(0.5*(first2d2 + last2d2));
  Standard_Real dist2d = p12d.Distance(p22d);
  GeomAdaptor_Surface Ads(BRepInspector::Surface(face));
  Standard_Real distSurf = Max(Ads.UResolution(dist), Ads.VResolution(dist));
  return (dist2d*0.2 >= distSurf);
  */
  Standard_Integer isULongC1, isULongC2, isVLongC1, isVLongC2;
  Standard_Real    SUmin, SUmax, SVmin, SVmax;
  Standard_Real    C1Umin, C1Vmin, C1Umax, C1Vmax;
  Standard_Real    C2Umin, C2Vmin, C2Umax, C2Vmax;
  { // szv: Use brackets to destroy local variables
    Bnd_Box2d           B1, B2;
    Geom2dAdaptor_Curve aC2d1(C2d1), aC2d2(C2d2);
    Add2dCurve::Add(aC2d1, first2d1, last2d1, Precision1::PConfusion(), B1);
    Add2dCurve::Add(aC2d2, first2d2, last2d2, Precision1::PConfusion(), B2);
    B1.Get(C1Umin, C1Vmin, C1Umax, C1Vmax);
    B2.Get(C2Umin, C2Vmin, C2Umax, C2Vmax);
    Standard_Real du, dv;
    du        = (C1Umax - C1Umin);
    dv        = (C1Vmax - C1Vmin);
    isULongC1 = (dv <= du);
    isVLongC1 = (du <= dv);
    du        = (C2Umax - C2Umin);
    dv        = (C2Vmax - C2Vmin);
    isULongC2 = (dv <= du);
    isVLongC2 = (du <= dv);
    surf->Bounds(SUmin, SUmax, SVmin, SVmax);
  }
  if (isUClosed && isVLongC1 && isVLongC2)
  {
    // Do not merge if not overlapped by V
    Standard_Real dist = Max((C2Vmin - C1Vmax), (C1Vmin - C2Vmax));
    if (dist < 0.0)
    {
      Standard_Real distInner = Max((C2Umin - C1Umax), (C1Umin - C2Umax));
      Standard_Real distOuter = (SUmax - SUmin) - Max((C2Umax - C1Umin), (C1Umax - C2Umin));
      if (distOuter <= distInner)
        return Standard_True;
    }
  }
  if (isVClosed && isULongC1 && isULongC2)
  {
    // Do not merge if not overlapped by U
    Standard_Real dist = Max((C2Umin - C1Umax), (C1Umin - C2Umax));
    if (dist < 0.0)
    {
      Standard_Real distInner = Max((C2Vmin - C1Vmax), (C1Vmin - C2Vmax));
      Standard_Real distOuter = (SVmax - SVmin) - Max((C2Vmax - C1Vmin), (C1Vmax - C2Vmin));
      if (distOuter <= distInner)
        return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

void BRepBuilderAPI_Sewing::AnalysisNearestEdges(const TopTools_SequenceOfShape& sequenceSec,
                                                 TColStd_SequenceOfInteger&      seqIndCandidate,
                                                 TColStd_SequenceOfBoolean&      seqOrientations,
                                                 const Standard_Boolean          evalDist)
{

  Standard_Integer     workIndex = seqIndCandidate.First();
  TopoShape         workedge  = sequenceSec.Value(workIndex);
  TopoShape         bnd       = workedge;
  ShapeList workfaces;
  if (mySectionBound.IsBound(bnd))
    bnd = mySectionBound(bnd);
  if (myBoundFaces.Contains(bnd))
    workfaces = myBoundFaces.FindFromKey(bnd);
  if (workfaces.IsEmpty())
    return;
  TopTools_MapOfShape                mapFaces;
  TopTools_ListIteratorOfListOfShape lIt;
  for (lIt.Initialize(workfaces); lIt.More(); lIt.Next())
    mapFaces.Add(lIt.Value());
  TColStd_SequenceOfInteger seqNotCandidate;
  TColStd_SequenceOfInteger seqNewForward;
  // Separates edges belonging the same face as work edge
  // for exception of edges belonging closed faces

  seqNotCandidate.Append(workIndex);
  for (Standard_Integer i = 1; i <= seqIndCandidate.Length();)
  {
    Standard_Integer index    = seqIndCandidate.Value(i);
    Standard_Boolean isRemove = Standard_False;
    if (index == workIndex)
    {
      seqIndCandidate.Remove(i);
      seqOrientations.Remove(i);
      isRemove = Standard_True;
    }
    if (!isRemove)
    {
      TopoShape bnd2 = sequenceSec.Value(index);
      if (mySectionBound.IsBound(bnd2))
        bnd2 = mySectionBound(bnd2);

      if (myBoundFaces.Contains(bnd2))
      {
        const ShapeList& listfaces = myBoundFaces.FindFromKey(bnd2);
        Standard_Boolean            isMerged  = Standard_True;
        for (lIt.Initialize(listfaces); lIt.More() && isMerged; lIt.Next())
        {
          if (mapFaces.Contains(lIt.Value()))
          {
            TopLoc_Location      loc;
            Handle(GeomSurface) surf = BRepInspector::Surface(TopoDS::Face(lIt.Value()), loc);
            isMerged = ((IsUClosedSurface(surf, bnd2, loc) || IsVClosedSurface(surf, bnd2, loc))
                        && IsMergedClosed(TopoDS::Edge(sequenceSec.Value(index)),
                                          TopoDS::Edge(workedge),
                                          TopoDS::Face(lIt.Value())));
          }
        }
        if (!isMerged)
        {
          seqNotCandidate.Append(index);
          seqIndCandidate.Remove(i);
          seqOrientations.Remove(i);
          isRemove = Standard_True;
        }
      }
      else
      {
        seqIndCandidate.Remove(i);
        seqOrientations.Remove(i);
        isRemove = Standard_True;
      }
    }
    if (!isRemove)
      i++;
  }
  if (seqIndCandidate.Length() == 0 || seqNotCandidate.Length() == 1)
    return;
  if (!evalDist)
    return;
  TColStd_Array2OfReal      TotTabDist(1, seqNotCandidate.Length(), 1, seqIndCandidate.Length());
  TColStd_MapOfInteger      MapIndex;
  TColStd_SequenceOfInteger seqForward;

  // Definition and removing edges which are not candidate for work edge
  // (they have other nearest edges belonging to the work face)
  for (Standard_Integer k = 1; k <= seqNotCandidate.Length(); k++)
  {
    Standard_Integer         index1 = seqNotCandidate.Value(k);
    const TopoShape&      edge   = sequenceSec.Value(index1);
    TopTools_SequenceOfShape tmpSeq;
    tmpSeq.Append(edge);
    for (Standard_Integer kk = 1; kk <= seqIndCandidate.Length(); kk++)
      tmpSeq.Append(sequenceSec.Value(seqIndCandidate.Value(kk)));

    Standard_Integer        lengSec = tmpSeq.Length();
    TColStd_Array1OfBoolean tabForward(1, lengSec);
    TColStd_Array1OfReal    tabDist(1, lengSec);
    TColStd_Array1OfReal    arrLen(1, lengSec);
    TColStd_Array1OfReal    tabMinDist(1, lengSec);
    for (Standard_Integer i1 = 1; i1 <= lengSec; i1++)
      tabDist(i1) = -1;

    EvaluateDistances(tmpSeq, tabForward, tabDist, arrLen, tabMinDist, 1);
    if (k == 1)
    {
      for (Standard_Integer n = 1; n < lengSec; n++)
      {
        if (tabDist(n + 1) == -1 || tabDist(n + 1) > myTolerance)
        {
          MapIndex.Add(n);
          continue;
        }
        TotTabDist(k, n) = tabDist(n + 1);
        seqForward.Append(tabForward(n + 1) ? 1 : 0);
      }
    }
    else
    {
      for (Standard_Integer n = 1; n < lengSec; n++)
      {
        if (tabDist(n) == -1 || tabDist(n) > myTolerance)
          continue;
        if (tabDist(n + 1) < TotTabDist(1, n))
        {
          MapIndex.Add(n);
        }
      }
    }
  }

  Standard_Integer i2 = seqIndCandidate.Length();
  for (; i2 >= 1; i2--)
  {
    if (MapIndex.Contains(i2))
    {
      seqIndCandidate.Remove(i2);
      seqOrientations.Remove(i2);
    }
  }
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::FindCandidates(TopTools_SequenceOfShape&    seqSections,
                                                       TColStd_IndexedMapOfInteger& mapReference,
                                                       TColStd_SequenceOfInteger&   seqCandidates,
                                                       TColStd_SequenceOfBoolean&   seqOrientations)
{
  Standard_Integer i, nbSections = seqSections.Length();
  if (nbSections <= 1)
    return Standard_False;
  // Retrieve last reference index
  Standard_Integer    indReference = mapReference(mapReference.Extent());
  Standard_Integer    nbCandidates = 0;
  TopTools_MapOfShape Faces1;
  // if (nbSections > 1) {

  TopoEdge Edge1 = TopoDS::Edge(seqSections(indReference));

  // Retrieve faces for reference section

  { // szv: Use brackets to destroy local variables
    TopoShape bnd = Edge1;
    if (mySectionBound.IsBound(bnd))
      bnd = mySectionBound(bnd);
    if (myBoundFaces.Contains(bnd))
    {
      TopTools_ListIteratorOfListOfShape itf1(myBoundFaces.FindFromKey(bnd));
      for (; itf1.More(); itf1.Next())
        Faces1.Add(itf1.Value());
    }
  }

  // Check merging conditions for candidates and remove unsatisfactory
  TopTools_SequenceOfShape  seqSectionsNew;
  TColStd_SequenceOfInteger seqCandidatesNew;
  for (i = 1; i <= nbSections; i++)
  {
    if (i == indReference)
    {
      seqSectionsNew.Prepend(Edge1);
      seqCandidatesNew.Prepend(i);
    }
    else
    {
      const TopoEdge& Edge2 = TopoDS::Edge(seqSections(i));
      seqSectionsNew.Append(Edge2);
      seqCandidatesNew.Append(i);
    }
  }

  Standard_Integer nbSectionsNew = seqSectionsNew.Length();
  if (nbSectionsNew > 1)
  {

    // Evaluate distances between reference and other sections
    TColStd_Array1OfBoolean arrForward(1, nbSectionsNew);
    TColStd_Array1OfReal    arrDistance(1, nbSectionsNew);
    TColStd_Array1OfReal    arrLen(1, nbSectionsNew);
    TColStd_Array1OfReal    arrMinDist(1, nbSectionsNew);
    EvaluateDistances(seqSectionsNew, arrForward, arrDistance, arrLen, arrMinDist, 1);

    // Fill sequence of candidate indices sorted by distance
    for (i = 2; i <= nbSectionsNew; i++)
    {
      Standard_Real aMaxDist = arrDistance(i);
      if (aMaxDist >= 0.0 && aMaxDist <= myTolerance && arrLen(i) > myMinTolerance)
      {

        // Reference1 section is connected to section #i
        Standard_Boolean isInserted = Standard_False;
        Standard_Boolean ori        = arrForward(i);
        for (Standard_Integer j = 1; (j <= seqCandidates.Length()) && !isInserted; j++)
        {
          Standard_Real aDelta = arrDistance(i) - arrDistance(seqCandidates.Value(j));

          if (aDelta < Precision1::Confusion())
          {

            if (fabs(aDelta) > RealSmall() || arrMinDist(i) < arrMinDist(seqCandidates.Value(j)))
            {
              seqCandidates.InsertBefore(j, i);
              seqOrientations.InsertBefore(j, ori);
              isInserted = Standard_True;
            }
          }
        }
        if (!isInserted)
        {
          seqCandidates.Append(i);
          seqOrientations.Append(ori);
        }
      }
    }

    nbCandidates = seqCandidates.Length();
    if (!nbCandidates)
      return Standard_False; // Section has no candidates to merge

    // Replace candidate indices

    for (i = 1; i <= nbCandidates; i++)
      seqCandidates(i) = seqCandidatesNew(seqCandidates(i));
  }

  if (!nbCandidates)
    return Standard_False; // Section has no candidates to merge

  if (myNonmanifold && nbCandidates > 1)
  {
    TColStd_SequenceOfInteger seqNewCandidates;
    TColStd_SequenceOfBoolean seqOrientationsNew;
    seqCandidates.Prepend(1);
    seqOrientations.Prepend(Standard_True);
    for (Standard_Integer k = 1; k <= seqSections.Length() && seqCandidates.Length() > 1; k++)
    {
      AnalysisNearestEdges(seqSections, seqCandidates, seqOrientations, (k == 1));
      if (k == 1 && !seqCandidates.Length())
        return Standard_False;
      if (seqCandidates.Length())
      {
        seqNewCandidates.Append(seqCandidates.First());
        seqOrientationsNew.Append(seqOrientations.First());
      }
    }
    seqCandidates.Prepend(seqNewCandidates);
    seqOrientations.Prepend(seqOrientationsNew);
    return Standard_True;
  }
  else
  {

    // For manifold case leave only one candidate from equidistant candidates
    /*Standard_Integer minIndex = seqCandidateIndex.First();
    Standard_Real minDistance = arrDistance(minIndex);

    // Find equidistant candidates
    TColStd_SequenceOfInteger seqEqDistantIndex; seqEqDistantIndex.Append(1);
    for (i = 2; i <= nbCandidates; i++) {
    Standard_Integer index = seqCandidateIndex(i);
    if (Abs(minDistance - arrDistance(index)) <= Precision1::Confusion())
    seqEqDistantIndex.Append(index);
    }

    Standard_Integer eqLen = seqEqDistantIndex.Length();
    if (eqLen > 2) {

    // Fill map of faces which equidistant sections belong to
    TopTools_MapOfShape mapFace;
    for (i = 1; i <= eqLen; i++) {
    Standard_Integer index = seqEqDistantIndex.Value(i);
    if (isCandidate(index)) {
    mapFace.Add(arrFace(index));
    }
    }

    // Non Manifold case
    // Edges are merged by pair among a face continuity C1 criterion
    if (mapFace.Extent() == eqLen) {

    tabDist.Init(-1);
    tabMinInd.Init(-1);
    min=10000000.;
    //indMin = -1;
    Standard_Integer indMin = -1;// To check if the edge can be merged.
    // Computation of distances between the edges.
    TopTools_SequenceOfShape seqSh;
    Standard_Integer nbInd = EqDistSeq.Length();
    TColStd_Array1OfBoolean tmptabForward(1,nbInd);
    seqSh.Append(sequenceSec.Value(1));
    for (j = 2; j <= EqDistSeq.Length(); j++) {
    Standard_Integer index = EqDistSeq.Value(j);
    tmptabForward(j) = tabForward(index);
    seqSh.Append(sequenceSec.Value(index));
    }

    EvaluateAngulars(seqSh, tmptabForward, tabDist,1);

    for(j=2; j <= seqSh.Length(); j++) {
    if (tabDist(j) > -1.) {  // if edge(j) is connected to edge(i)
    if (min > tabDist(j)) {
    min = tabDist(j);
    indMin = j;
    }
    }
    }

    //  Construct minDist, tabMinInd , tabMinForward(i) = tabForward(j);
    if (indMin > 0) {
    seqSh.Remove(indMin);
    for(j =2; j <= tmpSeq.Length(); ) {
    TopoShape sh = tmpSeq.Value(j);
    Standard_Boolean isRem = Standard_False;
    for(Standard_Integer k = 1; k<= seqSh.Length();k++) {
    if(seqSh.Value(k) == sh) {
    isRem = Standard_True;
    break;
    }
    }
    if(isRem) {
    tmpSeq.Remove(j);
    tabMinForward.Remove(j); // = -1;
    }
    else j++;
    }
    }
    }
    }*/

    // Find the best approved candidate
    while (nbCandidates)
    {
      // Retrieve first candidate
      Standard_Integer indCandidate = seqCandidates.First();
      // Candidate is approved if it is in the map
      if (mapReference.Contains(indCandidate))
        break;
      // Find candidates for candidate #indCandidate
      mapReference.Add(indCandidate); // Push candidate in the map
      TColStd_SequenceOfInteger seqCandidates1;
      TColStd_SequenceOfBoolean seqOrientations1;
      Standard_Boolean          isFound =
        FindCandidates(seqSections, mapReference, seqCandidates1, seqOrientations1);
      mapReference.RemoveLast(); // Pop candidate from the map
      if (isFound)
        isFound = (seqCandidates1.Length() > 0);
      if (isFound)
      {
        Standard_Integer indCandidate1 = seqCandidates1.First();
        // If indReference is the best candidate for indCandidate
        // then indCandidate is the best candidate for indReference
        if (indCandidate1 == indReference)
          break;
        // If some other reference in the map is the best candidate for indCandidate
        // then assume that reference is the best candidate for indReference
        if (mapReference.Contains(indCandidate1))
        {
          seqCandidates.Prepend(indCandidate1);
          nbCandidates++;
          break;
        }
        isFound = Standard_False;
      }
      if (!isFound)
      {
        // Remove candidate #1
        seqCandidates.Remove(1);
        seqOrientations.Remove(1);
        nbCandidates--;
      }
    }
  }
  // gka
  if (nbCandidates > 0)
  {
    Standard_Integer anInd = seqCandidates.Value(1);
    TopoEdge      Edge2 = TopoDS::Edge(seqSections(anInd));
    TopoShape     bnd   = Edge2;
    if (mySectionBound.IsBound(bnd))
      bnd = mySectionBound(bnd);
    // gka
    if (myBoundFaces.Contains(bnd))
    {
      Standard_Boolean                   isOK = Standard_True;
      TopTools_ListIteratorOfListOfShape itf2(myBoundFaces.FindFromKey(bnd));
      for (; itf2.More() && isOK; itf2.Next())
      {
        const TopoFace& Face2 = TopoDS::Face(itf2.Value());
        // Check whether condition is satisfied
        isOK = !Faces1.Contains(Face2);
        if (!isOK)
          isOK = IsMergedClosed(Edge1, Edge2, Face2);
      }
      if (!isOK)
        return Standard_False;
    }
  }
  return (nbCandidates > 0);
}

//=================================================================================================

BRepBuilderAPI_Sewing::BRepBuilderAPI_Sewing(const Standard_Real    tolerance,
                                             const Standard_Boolean optionSewing,
                                             const Standard_Boolean optionAnalysis,
                                             const Standard_Boolean optionCutting,
                                             const Standard_Boolean optionNonmanifold)
{
  myReShape = new BRepTools_ReShape;
  Init(tolerance, optionSewing, optionAnalysis, optionCutting, optionNonmanifold);
}

//=======================================================================
// function : Init
// purpose  : Initialise Talerance, and options sewing, faceAnalysis and cutting
//=======================================================================

void BRepBuilderAPI_Sewing::Init(const Standard_Real    tolerance,
                                 const Standard_Boolean optionSewing,
                                 const Standard_Boolean optionAnalysis,
                                 const Standard_Boolean optionCutting,
                                 const Standard_Boolean optionNonmanifold)
{
  // Set tolerance and Perform options
  myTolerance   = Max(tolerance, Precision1::Confusion());
  mySewing      = optionSewing;
  myAnalysis    = optionAnalysis;
  myCutting     = optionCutting;
  myNonmanifold = optionNonmanifold;
  // Set min and max tolerances
  myMinTolerance = myTolerance * 1e-4; // szv: proposal
  if (myMinTolerance < Precision1::Confusion())
    myMinTolerance = Precision1::Confusion();
  myMaxTolerance = Precision1::Infinite();
  // Set other modes
  myFaceMode          = Standard_True;
  myFloatingEdgesMode = Standard_False;
  // myCuttingFloatingEdgesMode = Standard_False; //gka
  mySameParameterMode  = Standard_True;
  myLocalToleranceMode = Standard_False;
  mySewedShape.Nullify();
  // Load empty shape
  Load(TopoShape());
}

//=======================================================================
// function : Load
// purpose  : Loads the context shape
//=======================================================================

void BRepBuilderAPI_Sewing::Load(const TopoShape& theShape)
{
  myReShape->Clear();
  if (theShape.IsNull())
    myShape.Nullify();
  else
    myShape = myReShape->Apply(theShape);
  mySewedShape.Nullify();
  // Nullify flags and counters
  myNbShapes = myNbEdges = myNbVertices = 0;
  // Clear all maps
  myOldShapes.Clear();
  // myOldFaces.Clear();
  myDegenerated.Clear();
  myFreeEdges.Clear();
  myMultipleEdges.Clear();
  myContigousEdges.Clear();
  myContigSecBound.Clear();
  myBoundFaces.Clear();
  myBoundSections.Clear();
  myVertexNode.Clear();
  myVertexNodeFree.Clear();
  myNodeSections.Clear();
  myCuttingNode.Clear();
  mySectionBound.Clear();
  myLittleFace.Clear();
}

//=================================================================================================

void BRepBuilderAPI_Sewing::Add(const TopoShape& aShape)
{
  if (aShape.IsNull())
    return;
  TopoShape oShape = myReShape->Apply(aShape);
  myOldShapes.Add(aShape, oShape);
  myNbShapes = myOldShapes.Extent();
}

//=================================================================================================

#ifdef OCCT_DEBUG
  #include <OSD_Timer.hxx>
#endif

void BRepBuilderAPI_Sewing::Perform(const Message_ProgressRange& theProgress)
{
  const Standard_Integer aNumberOfStages = myAnalysis + myCutting + mySewing + 2;
  Message_ProgressScope  aPS(theProgress, "Sewing", aNumberOfStages);
#ifdef OCCT_DEBUG
  Standard_Real   t_total = 0., t_analysis = 0., t_assembling = 0., t_cutting = 0., t_merging = 0.;
  Chronometer chr_total, chr_local;
  chr_total.Reset();
  chr_total.Start();
#endif

  // face analysis
  if (myAnalysis)
  {
#ifdef OCCT_DEBUG
    std::cout << "Begin face analysis..." << std::endl;
    chr_local.Reset();
    chr_local.Start();
#endif
    FaceAnalysis(aPS.Next());
    if (!aPS.More())
      return;
#ifdef OCCT_DEBUG
    chr_local.Stop();
    chr_local.Show(t_analysis);
    std::cout << "Face analysis finished after " << t_analysis << " s" << std::endl;
#endif
  }

  if (myNbShapes || !myShape.IsNull())
  {

    FindFreeBoundaries();

    if (myBoundFaces.Extent())
    {

#ifdef OCCT_DEBUG
      std::cout << "Begin vertices assembling..." << std::endl;
      chr_local.Reset();
      chr_local.Start();
#endif
      VerticesAssembling(aPS.Next());
      if (!aPS.More())
        return;
#ifdef OCCT_DEBUG
      chr_local.Stop();
      chr_local.Show(t_assembling);
      std::cout << "Vertices assembling finished after " << t_assembling << " s" << std::endl;
#endif
      if (myCutting)
      {
#ifdef OCCT_DEBUG
        std::cout << "Begin cutting..." << std::endl;
        chr_local.Reset();
        chr_local.Start();
#endif
        Cutting(aPS.Next());
        if (!aPS.More())
          return;
#ifdef OCCT_DEBUG
        chr_local.Stop();
        chr_local.Show(t_cutting);
        std::cout << "Cutting finished after " << t_cutting << " s" << std::endl;
#endif
      }
#ifdef OCCT_DEBUG
      std::cout << "Begin merging..." << std::endl;
      chr_local.Reset();
      chr_local.Start();
#endif
      Merging(Standard_True, aPS.Next());
      if (!aPS.More())
        return;
#ifdef OCCT_DEBUG
      chr_local.Stop();
      chr_local.Show(t_merging);
      std::cout << "Merging finished after " << t_merging << " s" << std::endl;
#endif
    }
    else
    {
      aPS.Next();
      if (myCutting)
        aPS.Next();
      aPS.Next();
      if (!aPS.More())
        return;
    }

    if (mySewing)
    {

#ifdef OCCT_DEBUG
      std::cout << "Creating sewed shape..." << std::endl;
#endif
      // examine the multiple edges if any and process sameparameter for edges if necessary
      EdgeProcessing(aPS.Next());
      if (!aPS.More())
        return;
      CreateSewedShape();
      if (!aPS.More())
      {
        mySewedShape.Nullify();
        return;
      }

      EdgeRegularity(aPS.Next());

      if (mySameParameterMode && myFaceMode)
        SameParameterShape();
      if (!aPS.More())
      {
        mySewedShape.Nullify();
        return;
      }
#ifdef OCCT_DEBUG
      std::cout << "Sewed shape created" << std::endl;
#endif
    }

    // create edge information for output
    CreateOutputInformations();
    if (!aPS.More())
    {
      mySewedShape.Nullify();
      return;
    }
  }
#ifdef OCCT_DEBUG
  chr_total.Stop();
  chr_total.Show(t_total);
  std::cout << "Sewing finished!" << std::endl;
  std::cout << " analysis time   : " << t_analysis << " s" << std::endl;
  std::cout << " assembling time : " << t_assembling << " s" << std::endl;
  std::cout << " cutting time    : " << t_cutting << " s" << std::endl;
  std::cout << " merging time    : " << t_merging << " s" << std::endl;
  std::cout << "Total time       : " << t_total << " s" << std::endl;
#endif
}

//=======================================================================
// function : SewedShape
// purpose  : give the sewed shape
//           if a null shape, reasons:
//             -- no useable input shapes : all input shapes are degenerated
//             -- has multiple edges
//=======================================================================

const TopoShape& BRepBuilderAPI_Sewing::SewedShape() const
{
  return mySewedShape;
}

//=================================================================================================

Standard_Integer BRepBuilderAPI_Sewing::NbFreeEdges() const
{
  return myFreeEdges.Extent();
}

//=================================================================================================

const TopoEdge& BRepBuilderAPI_Sewing::FreeEdge(const Standard_Integer index) const
{
  Standard_OutOfRange_Raise_if(index < 0 || index > NbFreeEdges(),
                               "BRepBuilderAPI_Sewing::FreeEdge");
  return TopoDS::Edge(myFreeEdges(index));
}

//=================================================================================================

Standard_Integer BRepBuilderAPI_Sewing::NbMultipleEdges() const
{
  return myMultipleEdges.Extent();
}

//=================================================================================================

const TopoEdge& BRepBuilderAPI_Sewing::MultipleEdge(const Standard_Integer index) const
{
  Standard_OutOfRange_Raise_if(index < 0 || index > NbMultipleEdges(),
                               "BRepBuilderAPI_Sewing::MultipleEdge");
  return TopoDS::Edge(myMultipleEdges(index));
}

//=================================================================================================

Standard_Integer BRepBuilderAPI_Sewing::NbContigousEdges() const
{
  return myContigousEdges.Extent();
}

//=================================================================================================

const TopoEdge& BRepBuilderAPI_Sewing::ContigousEdge(const Standard_Integer index) const
{
  Standard_OutOfRange_Raise_if(index < 0 || index > NbContigousEdges(),
                               "BRepBuilderAPI_Sewing::ContigousEdge");
  return TopoDS::Edge(myContigousEdges.FindKey(index));
}

//=================================================================================================

const ShapeList& BRepBuilderAPI_Sewing::ContigousEdgeCouple(
  const Standard_Integer index) const
{
  Standard_OutOfRange_Raise_if(index < 0 || index > NbContigousEdges(),
                               "BRepBuilderAPI_Sewing::ContigousEdgeCouple");
  return myContigousEdges(index);
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::IsSectionBound(const TopoEdge& section) const
{
  if (myContigSecBound.IsBound(section))
  {
    return Standard_True;
  }
  else
  {
    return Standard_False;
  }
}

//=================================================================================================

const TopoEdge& BRepBuilderAPI_Sewing::SectionToBoundary(const TopoEdge& section) const
{
  Standard_NoSuchObject_Raise_if(!IsSectionBound(section),
                                 "BRepBuilderAPI_Sewing::SectionToBoundary");
  return TopoDS::Edge(myContigSecBound(section));
}

//=================================================================================================

Standard_Integer BRepBuilderAPI_Sewing::NbDeletedFaces() const
{
  return myLittleFace.Extent();
}

//=================================================================================================

const TopoFace& BRepBuilderAPI_Sewing::DeletedFace(const Standard_Integer index) const
{
  Standard_OutOfRange_Raise_if(index < 0 || index > NbDeletedFaces(),
                               "BRepBuilderAPI_Sewing::DeletedFace");
  return TopoDS::Face(myLittleFace(index));
}

//=================================================================================================

Standard_Integer BRepBuilderAPI_Sewing::NbDegeneratedShapes() const
{
  return myDegenerated.Extent();
}

//=================================================================================================

const TopoShape& BRepBuilderAPI_Sewing::DegeneratedShape(const Standard_Integer index) const
{
  Standard_OutOfRange_Raise_if(index < 0 || index > NbDegeneratedShapes(),
                               "BRepBuilderAPI_Sewing::DegereratedShape");
  return myDegenerated(index);
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::IsDegenerated(const TopoShape& aShape) const
{
  TopoShape NewShape = myReShape->Apply(aShape);
  // Degenerated face
  if (aShape.ShapeType() == TopAbs_FACE)
    return NewShape.IsNull();
  if (NewShape.IsNull())
    return Standard_False;
  // Degenerated edge
  if (NewShape.ShapeType() == TopAbs_EDGE)
    return BRepInspector::Degenerated(TopoDS::Edge(NewShape));
  // Degenerated wire
  if (NewShape.ShapeType() == TopAbs_WIRE)
  {
    Standard_Boolean isDegenerated = Standard_True;
    for (TopoDS_Iterator aIt(NewShape); aIt.More() && isDegenerated; aIt.Next())
      isDegenerated = BRepInspector::Degenerated(TopoDS::Edge(aIt.Value()));
    return isDegenerated;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::IsModified(const TopoShape& aShape) const
{
  TopoShape NewShape = aShape;
  if (myOldShapes.Contains(aShape))
    NewShape = myOldShapes.FindFromKey(aShape);
  if (!NewShape.IsSame(aShape))
    return Standard_True;
  return Standard_False;
}

//=================================================================================================

const TopoShape& BRepBuilderAPI_Sewing::Modified(const TopoShape& aShape) const
{
  if (myOldShapes.Contains(aShape))
    return myOldShapes.FindFromKey(aShape);
  // if (myOldFaces.Contains(aShape)) return myOldFaces.FindFromKey(aShape);
  return aShape;
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::IsModifiedSubShape(const TopoShape& aShape) const
{
  TopoShape NewShape = myReShape->Apply(aShape);
  if (!NewShape.IsSame(aShape))
    return Standard_True;
  return Standard_False;
}

//=================================================================================================

TopoShape BRepBuilderAPI_Sewing::ModifiedSubShape(const TopoShape& aShape) const
{
  return myReShape->Apply(aShape);
}

//=================================================================================================

void BRepBuilderAPI_Sewing::Dump() const
{
  Standard_Integer    i, NbBounds = myBoundFaces.Extent(), NbSections = 0;
  TopTools_MapOfShape mapVertices, mapEdges;
  for (i = 1; i <= NbBounds; i++)
  {
    TopoShape bound = myBoundFaces.FindKey(i);
    if (myBoundSections.IsBound(bound))
      NbSections += myBoundSections(bound).Extent();
    else
      NbSections++;
    ShapeExplorer aExp(myReShape->Apply(bound), TopAbs_EDGE);
    for (; aExp.More(); aExp.Next())
    {
      TopoEdge E = TopoDS::Edge(aExp.Current());
      mapEdges.Add(E);
      TopoVertex V1, V2;
      TopExp1::Vertices(E, V1, V2);
      mapVertices.Add(V1);
      mapVertices.Add(V2);
    }
  }
  std::cout << " " << std::endl;
  std::cout << "                        Information                         " << std::endl;
  std::cout << " ===========================================================" << std::endl;
  std::cout << " " << std::endl;
  std::cout << " Number of input shapes      : " << myOldShapes.Extent() << std::endl;
  std::cout << " Number of actual shapes     : " << myNbShapes << std::endl;
  std::cout << " Number of Bounds            : " << NbBounds << std::endl;
  std::cout << " Number of Sections          : " << NbSections << std::endl;
  std::cout << " Number of Edges             : " << mapEdges.Extent() << std::endl;
  std::cout << " Number of Vertices          : " << myNbVertices << std::endl;
  std::cout << " Number of Nodes             : " << mapVertices.Extent() << std::endl;
  std::cout << " Number of Free Edges        : " << myFreeEdges.Extent() << std::endl;
  std::cout << " Number of Contiguous Edges  : " << myContigousEdges.Extent() << std::endl;
  std::cout << " Number of Multiple Edges    : " << myMultipleEdges.Extent() << std::endl;
  std::cout << " Number of Degenerated Edges : " << myDegenerated.Extent() << std::endl;
  std::cout << " ===========================================================" << std::endl;
  std::cout << " " << std::endl;
}

//=======================================================================
// function : FaceAnalysis
// purpose  : Remove
//	     Modifies:
//                      myNbShapes
//                      myOldShapes
//
//           Constructs:
//                      myDegenerated
//=======================================================================

void BRepBuilderAPI_Sewing::FaceAnalysis(const Message_ProgressRange& theProgress)
{
  if (!myShape.IsNull() && myOldShapes.IsEmpty())
  {
    Add(myShape);
    myShape.Nullify();
  }

  ShapeBuilder                              B;
  TopTools_MapOfShape                       SmallEdges;
  TopTools_IndexedDataMapOfShapeListOfShape GluedVertices;
  Standard_Integer                          i = 1;
  Message_ProgressScope aPS(theProgress, "Shape analysis", myOldShapes.Extent());
  for (i = 1; i <= myOldShapes.Extent() && aPS.More(); i++, aPS.Next())
  {
    for (ShapeExplorer fexp(myOldShapes(i), TopAbs_FACE); fexp.More(); fexp.Next())
    {

      // Retrieve current face
      TopoShape     aTmpShape = fexp.Current(); // for porting
      TopoFace      face      = TopoDS::Face(aTmpShape);
      Standard_Integer nbEdges = 0, nbSmall = 0;

      // Build replacing face
      aTmpShape                      = face.EmptyCopied().Oriented(TopAbs_FORWARD); // for porting
      TopoFace      nface         = TopoDS::Face(aTmpShape);
      Standard_Boolean isFaceChanged = Standard_False;

      TopoDS_Iterator witer(face.Oriented(TopAbs_FORWARD));
      for (; witer.More(); witer.Next())
      {

        // Retrieve current wire
        aTmpShape = witer.Value(); // for porting
        if (aTmpShape.ShapeType() != TopAbs_WIRE)
          continue;
        TopoWire wire = TopoDS::Wire(aTmpShape);

        // Build replacing wire
        aTmpShape                      = wire.EmptyCopied().Oriented(TopAbs_FORWARD);
        TopoWire      nwire         = TopoDS::Wire(aTmpShape);
        Standard_Boolean isWireChanged = Standard_False;

        TopoDS_Iterator eiter(wire.Oriented(TopAbs_FORWARD));
        for (; eiter.More(); eiter.Next())
        {

          // Retrieve current edge
          aTmpShape        = eiter.Value(); // for porting
          TopoEdge edge = TopoDS::Edge(aTmpShape);
          nbEdges++;

          // Process degenerated edge
          if (BRepInspector::Degenerated(edge))
          {
            B.Add(nwire, edge); // Old edge kept
            myDegenerated.Add(edge);
            nbSmall++;
            continue;
          }

          Standard_Boolean isSmall = SmallEdges.Contains(edge);
          if (!isSmall)
          {

            // Check for small edge
            Standard_Real      first, last;
            Handle(GeomCurve3d) c3d = BRepInspector::Curve(edge, first, last);
            if (c3d.IsNull())
            {
#ifdef OCCT_DEBUG
              std::cout << "Warning: Possibly small edge can be sewed: No 3D curve" << std::endl;
#endif
            }
            else
            {
              // Evaluate curve compactness
              const Standard_Integer npt = 5;
              Point3d                 cp((c3d->Value(first).XYZ() + c3d->Value(last).XYZ()) * 0.5);
              Standard_Real          dist, maxdist = 0.0;
              Standard_Real          delta = (last - first) / (npt - 1);
              for (Standard_Integer idx = 0; idx < npt; idx++)
              {
                dist = cp.Distance(c3d->Value(first + idx * delta));
                if (maxdist < dist)
                  maxdist = dist;
              }
              isSmall = (2. * maxdist <= MinTolerance());
              /*try {
            GeomAdaptor_Curve cAdapt(c3d);
            Standard_Real length = GCPnts_AbscissaPoint::Length(cAdapt,first,last);
            isSmall = (length <= MinTolerance());
              }
              catch (ExceptionBase) {
    #ifdef OCCT_DEBUG
            std::cout << "Warning: Possibly small edge can be sewed: ";
            ExceptionBase::Caught()->Print(std::cout); std::cout << std::endl;
    #endif
              }*/
            }

            if (isSmall)
            {

              // Store small edge in the map
              SmallEdges.Add(edge);

              TopoVertex v1, v2;
              TopExp1::Vertices(edge, v1, v2);
              TopoShape nv1 = myReShape->Apply(v1), nv2 = myReShape->Apply(v2);

              // Store glued vertices
              if (!nv1.IsSame(v1))
              {
                ShapeList& vlist1 = GluedVertices.ChangeFromKey(nv1);
                // First vertex was already glued
                if (!nv2.IsSame(v2))
                {
                  // Merge lists of glued vertices
                  if (!nv1.IsSame(nv2))
                  {
                    TopTools_ListIteratorOfListOfShape liter(GluedVertices.FindFromKey(nv2));
                    for (; liter.More(); liter.Next())
                    {
                      TopoShape v = liter.Value();
                      myReShape->Replace(v, nv1.Oriented(v.Orientation()));
                      vlist1.Append(v);
                    }
                    GluedVertices.RemoveKey(nv2);
                  }
                }
                else
                {
                  // Add second vertex to the existing list
                  vlist1.Append(v2);
                  myReShape->Replace(v2, nv1.Oriented(v2.Orientation()));
                }
              }
              else if (!nv2.IsSame(v2))
              {
                // Add first vertex to the existing list
                GluedVertices.ChangeFromKey(nv2).Append(v1);
                myReShape->Replace(v1, nv2.Oriented(v1.Orientation()));
              }
              else if (!v1.IsSame(v2))
              {
                // Record new glued vertices
                TopoVertex nv;
                B.MakeVertex(nv);
                ShapeList vlist;
                vlist.Append(v1);
                vlist.Append(v2);
                GluedVertices.Add(nv, vlist);
                myReShape->Replace(v1, nv.Oriented(v1.Orientation()));
                myReShape->Replace(v2, nv.Oriented(v2.Orientation()));
              }
            }
          }

          // Replace small edge
          if (isSmall)
          {
#ifdef OCCT_DEBUG
            std::cout << "Warning: Small edge made degenerated by FaceAnalysis" << std::endl;
#endif
            nbSmall++;
            // Create new degenerated edge
            aTmpShape                  = edge.Oriented(TopAbs_FORWARD);
            TopoEdge          fedge = TopoDS::Edge(aTmpShape);
            Standard_Real        pfirst, plast;
            Handle(GeomCurve2d) c2d = BRepInspector::CurveOnSurface(fedge, face, pfirst, plast);
            if (!c2d.IsNull())
            {
              TopoEdge nedge;
              B.MakeEdge(nedge);
              B.UpdateEdge(nedge, c2d, face, Precision1::Confusion());
              B.Range(nedge, pfirst, plast);
              B.Degenerated(nedge, Standard_True);
              TopoVertex v1, v2;
              TopExp1::Vertices(fedge, v1, v2);
              B.Add(nedge, myReShape->Apply(v1).Oriented(v1.Orientation()));
              B.Add(nedge, myReShape->Apply(v2).Oriented(v2.Orientation()));
              B.Add(nwire, nedge.Oriented(edge.Orientation()));
              myDegenerated.Add(nedge);
            }
            isWireChanged = Standard_True;
          }
          else
            B.Add(nwire, edge); // Old edge kept
        }

        // Record wire in the new face
        if (isWireChanged)
        {
          B.Add(nface, nwire.Oriented(wire.Orientation()));
          isFaceChanged = Standard_True;
        }
        else
          B.Add(nface, wire);
      }

      // Remove small face
      if (nbSmall == nbEdges)
      {
#ifdef OCCT_DEBUG
        std::cout << "Warning: Small face removed by FaceAnalysis" << std::endl;
#endif
        myLittleFace.Add(face);
        myReShape->Remove(face);
      }
      else if (isFaceChanged)
      {

        myReShape->Replace(face, nface.Oriented(face.Orientation()));
      }
    }
  }

  // Update glued vertices
  TopTools_IndexedDataMapOfShapeListOfShape::Iterator aMIter(GluedVertices);
  for (; aMIter.More(); aMIter.Next())
  {
    const TopoVertex&               vnew = TopoDS::Vertex(aMIter.Key1());
    Coords3d                             coord(0., 0., 0.);
    Standard_Integer                   nbPoints = 0;
    const ShapeList&        vlist    = aMIter.Value();
    TopTools_ListIteratorOfListOfShape liter1(vlist);
    for (; liter1.More(); liter1.Next())
    {
      coord += BRepInspector::Pnt(TopoDS::Vertex(liter1.Value())).XYZ();
      nbPoints++;
    }
    if (nbPoints)
    {
      Point3d                             vp(coord / nbPoints);
      Standard_Real                      tol = 0.0, mtol = 0.0;
      TopTools_ListIteratorOfListOfShape liter2(vlist);
      for (; liter2.More(); liter2.Next())
      {
        Standard_Real vtol = BRepInspector::Tolerance(TopoDS::Vertex(liter2.Value()));
        if (mtol < vtol)
          mtol = vtol;
        vtol = vp.Distance(BRepInspector::Pnt(TopoDS::Vertex(liter2.Value())));
        if (tol < vtol)
          tol = vtol;
      }
      B.UpdateVertex(vnew, vp, tol + mtol);
    }
  }

  // Update input shapes
  for (i = 1; i <= myOldShapes.Extent(); i++)
    myOldShapes(i) = myReShape->Apply(myOldShapes(i));
}

//=======================================================================
// function : FindFreeBoundaries
// purpose  : Constructs :
//                      myBoundFaces     (bound = list of faces) - REFERENCE
//                      myVertexNode     (vertex = node)
//                      myVertexNodeFree (floating vertex = node)
//
//=======================================================================

void BRepBuilderAPI_Sewing::FindFreeBoundaries()
{
  // Take into account the context shape if needed
  TopTools_IndexedMapOfShape NewShapes;
  if (!myShape.IsNull())
  {
    if (myOldShapes.IsEmpty())
    {
      Add(myShape);
      myShape.Nullify();
    }
    else
    {
      TopoShape newShape = myReShape->Apply(myShape);
      if (!newShape.IsNull())
        NewShapes.Add(newShape);
    }
  }
  // Create map Edge -> Faces
  TopTools_IndexedDataMapOfShapeListOfShape EdgeFaces;
  Standard_Integer                          i, nbShapes = myOldShapes.Extent();
  for (i = 1; i <= nbShapes; i++)
  {
    // Retrieve new shape
    const TopoShape& shape = myOldShapes(i);
    if (shape.IsNull())
      continue;
    NewShapes.Add(shape);
    // Explore shape to find all boundaries
    for (ShapeExplorer eExp(shape, TopAbs_EDGE); eExp.More(); eExp.Next())
    {
      const TopoShape& edge = eExp.Current();
      if (!EdgeFaces.Contains(edge))
      {
        ShapeList listFaces;
        EdgeFaces.Add(edge, listFaces);
      }
    }
  }
  // Fill map Edge -> Faces
  nbShapes = NewShapes.Extent();
  TopTools_MapOfShape mapFaces;
  for (i = 1; i <= nbShapes; i++)
  {
    // Explore shape to find all faces
    ShapeExplorer fExp(NewShapes.FindKey(i), TopAbs_FACE);
    for (; fExp.More(); fExp.Next())
    {
      const TopoShape& face = fExp.Current();
      if (mapFaces.Contains(face))
        continue;
      else
        mapFaces.Add(face);
      // Explore face to find all boundaries
      for (TopoDS_Iterator aIw(face); aIw.More(); aIw.Next())
      {
        if (aIw.Value().ShapeType() != TopAbs_WIRE)
          continue;
        for (TopoDS_Iterator aIIe(aIw.Value()); aIIe.More(); aIIe.Next())
        {

          const TopoShape& edge = aIIe.Value();

          if (EdgeFaces.Contains(edge))
          {
            EdgeFaces.ChangeFromKey(edge).Append(face);
            // ShapeList& listFaces = EdgeFaces.ChangeFromKey(edge);
            // Standard_Boolean isContained = Standard_False;
            // TopTools_ListIteratorOfListOfShape itf(listFaces);
            // for (; itf.More() && !isContained; itf.Next())
            //   isContained = face.IsSame(itf.Value());
            // if (!isContained) listFaces.Append(face);
          }
        }
      }
    }
  }
  // Find free boundaries
  TopTools_IndexedDataMapOfShapeListOfShape::Iterator anIterEF(EdgeFaces);
  for (; anIterEF.More(); anIterEF.Next())
  {
    ShapeList& listFaces = anIterEF.ChangeValue();
    Standard_Integer      nbFaces   = listFaces.Extent();
    TopoShape          edge      = anIterEF.Key1();
    if (edge.Orientation() == TopAbs_INTERNAL)
      continue;
    Standard_Boolean isSeam = Standard_False;
    if (nbFaces == 1)
    {
      TopoFace face = TopoDS::Face(listFaces.First());
      isSeam           = BRepInspector::IsClosed(TopoDS::Edge(edge), face);
      if (isSeam)
      {
        /// Handle(GeomSurface) surf = BRepInspector::Surface(face);
        // isSeam = (IsUClosedSurface(surf) || IsVClosedSurface(surf));
        // if(!isSeam) {
        ShapeBuilder    aB;
        TopoShape    anewEdge = edge.EmptyCopied();
        TopoDS_Iterator aItV(edge);
        for (; aItV.More(); aItV.Next())
          aB.Add(anewEdge, aItV.Value());

        Standard_Real        first2d, last2d;
        Handle(GeomCurve2d) c2dold = BRepInspector::CurveOnSurface(TopoDS::Edge(edge),
                                                                TopoDS::Face(listFaces.First()),
                                                                first2d,
                                                                last2d);

        Handle(GeomCurve2d) c2d;
        ShapeBuilder         B;
        B.UpdateEdge(TopoDS::Edge(anewEdge), c2d, c2d, TopoDS::Face(listFaces.First()), 0);
        B.UpdateEdge(TopoDS::Edge(anewEdge), c2dold, TopoDS::Face(listFaces.First()), 0);

        Standard_Real aFirst, aLast;
        BRepInspector::Range(TopoDS::Edge(edge), aFirst, aLast);
        aB.Range(TopoDS::Edge(anewEdge), aFirst, aLast);
        aB.Range(TopoDS::Edge(anewEdge), TopoDS::Face(listFaces.First()), first2d, last2d);
        myReShape->Replace(edge, anewEdge);
        edge = anewEdge;

        //}
        isSeam = Standard_False;
      }
    }
    Standard_Boolean isBoundFloat = (myFloatingEdgesMode && !nbFaces);
    Standard_Boolean isBound =
      (myFaceMode && ((myNonmanifold && nbFaces) || (nbFaces == 1 && !isSeam)));
    if (isBound || isBoundFloat)
    {
      // Ignore degenerated edge
      if (BRepInspector::Degenerated(TopoDS::Edge(edge)))
        continue;
      // Ignore edge with internal vertices
      // Standard_Integer nbVtx = 0;
      // for (ShapeExplorer vExp(edge,TopAbs_VERTEX); vExp.More(); vExp.Next()) nbVtx++;
      // if (nbVtx != 2) continue;
      // Add to BoundFaces
      ShapeList listFacesCopy;
      listFacesCopy.Append(listFaces);
      myBoundFaces.Add(edge, listFacesCopy);
      // Process edge vertices
      TopoVertex vFirst, vLast;
      TopExp1::Vertices(TopoDS::Edge(edge), vFirst, vLast);
      if (vFirst.IsNull() || vLast.IsNull())
        continue;
      if (vFirst.Orientation() == TopAbs_INTERNAL || vLast.Orientation() == TopAbs_INTERNAL)
        continue;
      if (isBound)
      {
        // Add to VertexNode
        if (!myVertexNode.Contains(vFirst))
          myVertexNode.Add(vFirst, vFirst);
        if (!myVertexNode.Contains(vLast))
          myVertexNode.Add(vLast, vLast);
      }
      else
      {
        // Add to VertexNodeFree
        if (!myVertexNodeFree.Contains(vFirst))
          myVertexNodeFree.Add(vFirst, vFirst);
        if (!myVertexNodeFree.Contains(vLast))
          myVertexNodeFree.Add(vLast, vLast);
      }
    }
  }
}

//=======================================================================
// function : VerticesAssembling
// purpose  : Modifies :
//                      myVertexNode     (nodes glued)
//                      myVertexNodeFree (nodes glued)
//                      myNodeSections   (lists of sections merged for glued nodes)
//
//=======================================================================

static Standard_Boolean CreateNewNodes(
  const TopTools_IndexedDataMapOfShapeShape&       NodeNearestNode,
  const TopTools_IndexedDataMapOfShapeListOfShape& NodeVertices,
  TopTools_IndexedDataMapOfShapeShape&             aVertexNode,
  TopTools_DataMapOfShapeListOfShape&              aNodeEdges)
{
  // Create new nodes
  ShapeBuilder                                  B;
  TopTools_DataMapOfShapeShape                  OldNodeNewNode;
  TopTools_IndexedDataMapOfShapeListOfShape     NewNodeOldNodes;
  TopTools_IndexedDataMapOfShapeShape::Iterator anIter(NodeNearestNode);
  for (; anIter.More(); anIter.Next())
  {
    // Retrieve a pair of nodes to merge
    const TopoShape& oldnode1 = anIter.Key1();
    const TopoShape& oldnode2 = anIter.Value();
    // Second node should also be in the map
    if (!NodeNearestNode.Contains(oldnode2))
      continue;
    // Get new node for old node #1
    if (OldNodeNewNode.IsBound(oldnode1))
    {
      const TopoShape& newnode1 = OldNodeNewNode(oldnode1);
      if (OldNodeNewNode.IsBound(oldnode2))
      {
        TopoShape newnode2 = OldNodeNewNode(oldnode2);
        if (!newnode1.IsSame(newnode2))
        {
          // Change data for new node #2
          ShapeList&              lnode1 = NewNodeOldNodes.ChangeFromKey(newnode1);
          TopTools_ListIteratorOfListOfShape itn(NewNodeOldNodes.FindFromKey(newnode2));
          for (; itn.More(); itn.Next())
          {
            const TopoShape& node2 = itn.Value();
            lnode1.Append(node2);
            OldNodeNewNode(node2) = newnode1;
          }
          NewNodeOldNodes.RemoveKey(newnode2);
        }
      }
      else
      {
        // Old node #2 is not bound - add to old node #1
        OldNodeNewNode.Bind(oldnode2, newnode1);
        NewNodeOldNodes.ChangeFromKey(newnode1).Append(oldnode2);
      }
    }
    else
    {
      if (OldNodeNewNode.IsBound(oldnode2))
      {
        // Old node #1 is not bound - add to old node #2
        const TopoShape& newnode2 = OldNodeNewNode(oldnode2);
        OldNodeNewNode.Bind(oldnode1, newnode2);
        NewNodeOldNodes.ChangeFromKey(newnode2).Append(oldnode1);
      }
      else
      {
        // Nodes are not bound - create new node
        TopoVertex newnode;
        B.MakeVertex(newnode);
        OldNodeNewNode.Bind(oldnode1, newnode);
        OldNodeNewNode.Bind(oldnode2, newnode);
        ShapeList lnodes;
        lnodes.Append(oldnode1);
        lnodes.Append(oldnode2);
        NewNodeOldNodes.Add(newnode, lnodes);
      }
    }
  }

  // Stop if no new nodes created
  if (!NewNodeOldNodes.Extent())
    return Standard_False;

  TopTools_IndexedDataMapOfShapeListOfShape::Iterator anIter1(NewNodeOldNodes);
  for (; anIter1.More(); anIter1.Next())
  {
    const TopoVertex& newnode = TopoDS::Vertex(anIter1.Key1());
    // Calculate new node center point
    Coords3d               theCoordinates(0., 0., 0.);
    ShapeList lvert; // Accumulate node vertices
    TopTools_MapOfShape  medge;
    ShapeList ledge; // Accumulate node edges
    // Iterate on old nodes
    TopTools_ListIteratorOfListOfShape itn(anIter1.Value());
    for (; itn.More(); itn.Next())
    {
      const TopoShape& oldnode = itn.Value();
      // Iterate on node vertices
      TopTools_ListIteratorOfListOfShape itv(NodeVertices.FindFromKey(oldnode));
      for (; itv.More(); itv.Next())
      {
        const TopoVertex& vertex = TopoDS::Vertex(itv.Value());
        // Change node for vertex
        aVertexNode.ChangeFromKey(vertex) = newnode;
        // Accumulate coordinates
        theCoordinates += BRepInspector::Pnt(vertex).XYZ();
        lvert.Append(vertex);
      }
      // Iterate on node edges
      const ShapeList&        edges = aNodeEdges(oldnode);
      TopTools_ListIteratorOfListOfShape ite(edges);
      for (; ite.More(); ite.Next())
      {
        const TopoShape& edge = ite.Value();
        if (!medge.Contains(edge))
        {
          medge.Add(edge);
          ledge.Append(edge);
        }
      }
      // Unbind old node edges
      aNodeEdges.UnBind(oldnode);
    }
    // Bind new node edges
    aNodeEdges.Bind(newnode, ledge);
    Point3d center(theCoordinates / lvert.Extent());
    // Calculate new node tolerance
    Standard_Real                      toler = 0.0;
    TopTools_ListIteratorOfListOfShape itv(lvert);
    for (; itv.More(); itv.Next())
    {
      const TopoVertex& vertex = TopoDS::Vertex(itv.Value());
      Standard_Real t = center.Distance(BRepInspector::Pnt(vertex)) + BRepInspector::Tolerance(vertex);
      if (toler < t)
        toler = t;
    }
    // Update new node parameters
    B.UpdateVertex(newnode, center, toler);
  }

  return Standard_True;
}

static Standard_Integer IsMergedVertices(const TopoShape& face1,
                                         const TopoShape& e1,
                                         const TopoShape& e2,
                                         const TopoShape& vtx1,
                                         const TopoShape& vtx2)
{
  // Case of floating edges
  if (face1.IsNull())
    return (!IsClosedShape(e1, vtx1, vtx2));

  // Find wires containing given edges
  TopoShape    wire1, wire2;
  ShapeExplorer itw(face1, TopAbs_WIRE);
  for (; itw.More() && (wire1.IsNull() || wire2.IsNull()); itw.Next())
  {
    TopoDS_Iterator ite(itw.Current(), Standard_False);
    for (; ite.More() && (wire1.IsNull() || wire2.IsNull()); ite.Next())
    {
      if (wire1.IsNull() && e1.IsSame(ite.Value()))
        wire1 = itw.Current();
      if (wire2.IsNull() && e2.IsSame(ite.Value()))
        wire2 = itw.Current();
    }
  }
  Standard_Integer Status = 0;
  if (!wire1.IsNull() && !wire2.IsNull())
  {
    if (wire1.IsSame(wire2))
    {
      for (TopoDS_Iterator aIte(wire1, Standard_False); aIte.More(); aIte.Next())
      {
        TopoVertex ve1, ve2;
        TopExp1::Vertices(TopoDS::Edge(aIte.Value()), ve1, ve2);
        if ((ve1.IsSame(vtx1) && ve2.IsSame(vtx2)) || (ve2.IsSame(vtx1) && ve1.IsSame(vtx2)))
          return (IsClosedShape(aIte.Value(), vtx1, vtx2) ? 0 : 1);
      }
      if (IsClosedShape(wire1, vtx1, vtx2))
      {
        TopoVertex V1, V2;
        TopExp1::Vertices(TopoDS::Wire(wire1), V1, V2);
        Standard_Boolean isEndVertex =
          ((V1.IsSame(vtx1) && V2.IsSame(vtx2)) || (V2.IsSame(vtx1) && V1.IsSame(vtx2)));
        if (!isEndVertex)
          Status = 1;
      }
      else
        Status = 1;
    }
    else
      Status = -1;
  }
  return Status;
}

static Standard_Boolean GlueVertices(TopTools_IndexedDataMapOfShapeShape&             aVertexNode,
                                     TopTools_DataMapOfShapeListOfShape&              aNodeEdges,
                                     const TopTools_IndexedDataMapOfShapeListOfShape& aBoundFaces,
                                     const Standard_Real                              Tolerance,
                                     const Message_ProgressRange&                     theProgress)
{
  // Create map of node -> vertices
  TopTools_IndexedDataMapOfShapeListOfShape     NodeVertices;
  BRepBuilderAPI_CellFilter                     aFilter(Tolerance);
  BRepBuilderAPI_VertexInspector                anInspector(Tolerance);
  TopTools_IndexedDataMapOfShapeShape::Iterator anIter1(aVertexNode);
  for (; anIter1.More(); anIter1.Next())
  {
    const TopoShape&  vertex = anIter1.Key1();
    const TopoVertex& node   = TopoDS::Vertex(anIter1.Value());
    if (NodeVertices.Contains(node))
    {
      NodeVertices.ChangeFromKey(node).Append(vertex);
    }
    else
    {
      ShapeList vlist;
      vlist.Append(vertex);
      NodeVertices.Add(node, vlist);
      Point3d aPnt = BRepInspector::Pnt(TopoDS::Vertex(node));
      aFilter.Add(NodeVertices.FindIndex(node), aPnt.XYZ());
      anInspector.Add(aPnt.XYZ());
    }
  }
  Standard_Integer nbNodes = NodeVertices.Extent();
#ifdef OCCT_DEBUG
  std::cout << "Glueing " << nbNodes << " nodes..." << std::endl;
#endif
  // Merge nearest nodes
  TopTools_IndexedDataMapOfShapeShape NodeNearestNode;
  Message_ProgressScope               aPS(theProgress, "Glueing nodes", nbNodes, Standard_True);
  for (Standard_Integer i = 1; i <= nbNodes && aPS.More(); i++, aPS.Next())
  {
    const TopoVertex& node1 = TopoDS::Vertex(NodeVertices.FindKey(i));
    // Find near nodes
    Point3d pt1 = BRepInspector::Pnt(node1);
    anInspector.SetCurrent(pt1.XYZ());
    Coords3d aPntMin = anInspector.Shift(pt1.XYZ(), -Tolerance);
    Coords3d aPntMax = anInspector.Shift(pt1.XYZ(), Tolerance);
    aFilter.Inspect(aPntMin, aPntMax, anInspector);
    if (anInspector.ResInd().IsEmpty())
      continue;
    // Retrieve list of edges for the first node
    const ShapeList& ledges1 = aNodeEdges(node1);
    // Explore list of near nodes and fill the sequence of glued nodes
    TopTools_SequenceOfShape SeqNodes;
    ShapeList     listNodesSameEdge;
    // Point3d pt1 = BRepInspector::Pnt(node1);
    TColStd_ListIteratorOfListOfInteger iter1(anInspector.ResInd());
    for (; iter1.More(); iter1.Next())
    {
      const TopoVertex& node2 = TopoDS::Vertex(NodeVertices.FindKey(iter1.Value()));
      if (node1 == node2)
        continue;
      // Retrieve list of edges for the second node
      const ShapeList& ledges2 = aNodeEdges(node2);
      // Check merging condition for the pair of nodes
      Standard_Integer Status = 0, isSameEdge = Standard_False;
      // Explore edges of the first node
      TopTools_ListIteratorOfListOfShape Ie1(ledges1);
      for (; Ie1.More() && !Status && !isSameEdge; Ie1.Next())
      {
        const TopoShape& e1 = Ie1.Value();
        // Obtain real vertex from edge
        TopoShape v1 = node1;
        { // szv: Use brackets to destroy local variables
          TopoVertex ov1, ov2;
          TopExp1::Vertices(TopoDS::Edge(e1), ov1, ov2);
          if (aVertexNode.Contains(ov1))
          {
            if (node1.IsSame(aVertexNode.FindFromKey(ov1)))
              v1 = ov1;
          }
          if (aVertexNode.Contains(ov2))
          {
            if (node1.IsSame(aVertexNode.FindFromKey(ov2)))
              v1 = ov2;
          }
        }
        // Create map of faces for e1
        TopTools_MapOfShape         Faces1;
        const ShapeList& lfac1 = aBoundFaces.FindFromKey(e1);
        if (lfac1.Extent())
        {
          TopTools_ListIteratorOfListOfShape itf(lfac1);
          for (; itf.More(); itf.Next())
            if (!itf.Value().IsNull())
              Faces1.Add(itf.Value());
        }
        // Explore edges of the second node
        TopTools_ListIteratorOfListOfShape Ie2(ledges2);
        for (; Ie2.More() && !Status && !isSameEdge; Ie2.Next())
        {
          const TopoShape& e2 = Ie2.Value();
          // Obtain real vertex from edge
          TopoShape v2 = node2;
          { // szv: Use brackets to destroy local variables
            TopoVertex ov1, ov2;
            TopExp1::Vertices(TopoDS::Edge(e2), ov1, ov2);
            if (aVertexNode.Contains(ov1))
            {
              if (node2.IsSame(aVertexNode.FindFromKey(ov1)))
                v2 = ov1;
            }
            if (aVertexNode.Contains(ov2))
            {
              if (node2.IsSame(aVertexNode.FindFromKey(ov2)))
                v2 = ov2;
            }
          }
          // Explore faces for e2
          const ShapeList& lfac2 = aBoundFaces.FindFromKey(e2);
          if (lfac2.Extent())
          {
            TopTools_ListIteratorOfListOfShape itf(lfac2);
            for (; itf.More() && !Status && !isSameEdge; itf.Next())
            {
              // Check merging conditions for the same face
              if (Faces1.Contains(itf.Value()))
              {
                Standard_Integer stat = IsMergedVertices(itf.Value(), e1, e2, v1, v2);
                if (stat == 1)
                  isSameEdge = Standard_True;
                else
                  Status = stat;
              }
            }
          }
          else if (Faces1.IsEmpty() && e1 == e2)
          {
            Standard_Integer stat = IsMergedVertices(TopoFace(), e1, e1, v1, v2);
            if (stat == 1)
              isSameEdge = Standard_True;
            else
              Status = stat;
            break;
          }
        }
      }
      if (Status)
        continue;
      if (isSameEdge)
        listNodesSameEdge.Append(node2);
      // Append near node to the sequence
      Point3d        pt2  = BRepInspector::Pnt(node2);
      Standard_Real dist = pt1.Distance(pt2);
      if (dist < Tolerance)
      {
        Standard_Boolean isIns = Standard_False;
        for (Standard_Integer kk = 1; kk <= SeqNodes.Length() && !isIns; kk++)
        {
          Point3d pt = BRepInspector::Pnt(TopoDS::Vertex(SeqNodes.Value(kk)));
          if (dist < pt1.Distance(pt))
          {
            SeqNodes.InsertBefore(kk, node2);
            isIns = Standard_True;
          }
        }
        if (!isIns)
          SeqNodes.Append(node2);
      }
    }
    if (SeqNodes.Length())
    {
      // Remove nodes near to some other from the same edge
      if (listNodesSameEdge.Extent())
      {
        TopTools_ListIteratorOfListOfShape lInt(listNodesSameEdge);
        for (; lInt.More(); lInt.Next())
        {
          const TopoVertex& n2 = TopoDS::Vertex(lInt.Value());
          Point3d               p2 = BRepInspector::Pnt(n2);
          for (Standard_Integer k = 1; k <= SeqNodes.Length();)
          {
            const TopoVertex& n1 = TopoDS::Vertex(SeqNodes.Value(k));
            if (n1 != n2)
            {
              Point3d p1 = BRepInspector::Pnt(n1);
              if (p2.Distance(p1) >= pt1.Distance(p1))
              {
                k++;
                continue;
              }
            }
            SeqNodes.Remove(k);
          }
        }
      }
      // Bind nearest node if at least one exists
      if (SeqNodes.Length())
        NodeNearestNode.Add(node1, SeqNodes.First());
    }
    anInspector.ClearResList();
  }

  // Create new nodes for chained nearest nodes
  if (NodeNearestNode.IsEmpty())
    return Standard_False;

  return CreateNewNodes(NodeNearestNode, NodeVertices, aVertexNode, aNodeEdges);
}

void BRepBuilderAPI_Sewing::VerticesAssembling(const Message_ProgressRange& theProgress)
{
  Standard_Integer      nbVert     = myVertexNode.Extent();
  Standard_Integer      nbVertFree = myVertexNodeFree.Extent();
  Message_ProgressScope aPS(theProgress, "Vertices assembling", 2);
  if (nbVert || nbVertFree)
  {
    // Fill map node -> sections
    Standard_Integer i;
    for (i = 1; i <= myBoundFaces.Extent(); i++)
    {
      TopoShape bound = myBoundFaces.FindKey(i);
      for (TopoDS_Iterator itv(bound, Standard_False); itv.More(); itv.Next())
      {
        const TopoShape& node = itv.Value();
        if (myNodeSections.IsBound(node))
          myNodeSections(node).Append(bound);
        else
        {
          ShapeList lbnd;
          lbnd.Append(bound);
          myNodeSections.Bind(node, lbnd);
        }
      }
    }
    // Glue vertices
    if (nbVert)
    {
#ifdef OCCT_DEBUG
      std::cout << "Assemble " << nbVert << " vertices on faces..." << std::endl;
#endif
      while (GlueVertices(myVertexNode, myNodeSections, myBoundFaces, myTolerance, aPS.Next()))
        ;
    }
    if (!aPS.More())
      return;
    if (nbVertFree)
    {
#ifdef OCCT_DEBUG
      std::cout << "Assemble " << nbVertFree << " vertices on floating edges..." << std::endl;
#endif
      while (GlueVertices(myVertexNodeFree, myNodeSections, myBoundFaces, myTolerance, aPS.Next()))
        ;
    }
  }
}

//=======================================================================
// function : replaceNMVertices
// purpose  : internal use (static)
//=======================================================================
static void replaceNMVertices(const TopoEdge&               theEdge,
                              const TopoVertex&             theV1,
                              const TopoVertex&             theV2,
                              const Handle(BRepTools_ReShape)& theReShape)
{
  // To keep NM vertices on edge
  TopTools_SequenceOfShape aSeqNMVert;
  TColStd_SequenceOfReal   aSeqNMPars;
  Standard_Boolean         hasNMVert = findNMVertices(theEdge, aSeqNMVert, aSeqNMPars);
  if (!hasNMVert)
    return;
  Standard_Real first, last;
  BRepInspector::Range(theEdge, first, last);
  TopLoc_Location    aLoc;
  Handle(GeomCurve3d) c3d = BRepInspector::Curve(theEdge, aLoc, first, last);
  if (c3d.IsNull())
    return;
  TopTools_SequenceOfShape aEdVert;
  TColStd_SequenceOfReal   aEdParams;
  Standard_Integer         i = 1, nb = aSeqNMPars.Length();

  for (; i <= nb; i++)
  {
    Standard_Real apar = aSeqNMPars.Value(i);
    if (fabs(apar - first) <= Precision1::PConfusion())
    {
      theReShape->Replace(aSeqNMVert.Value(i), theV1);
      continue;
    }
    if (fabs(apar - last) <= Precision1::PConfusion())
    {
      theReShape->Replace(aSeqNMVert.Value(i), theV2);
      continue;
    }
    const TopoShape& aV = aSeqNMVert.Value(i);
    Standard_Integer    j  = 1;
    for (; j <= aEdParams.Length(); j++)
    {
      Standard_Real apar2 = aEdParams.Value(j);
      if (fabs(apar - apar2) <= Precision1::PConfusion())
      {
        theReShape->Replace(aV, aEdVert.Value(j));
        break;
      }
      else if (apar < apar2)
      {
        TopoShape anewV = aV.EmptyCopied();
        aEdVert.InsertBefore(j, anewV);
        aEdParams.InsertBefore(j, apar);
        BRep_ListOfPointRepresentation& alistrep =
          (*((Handle(BRep_TVertex)*)&anewV.TShape()))->ChangePoints();
        Handle(BRep_PointOnCurve) aPRep = new BRep_PointOnCurve(apar, c3d, aLoc);
        alistrep.Append(aPRep);
        theReShape->Replace(aV, anewV);
        break;
      }
    }
    if (j > aEdParams.Length())
    {
      TopoShape anewV = aV.EmptyCopied();
      aEdVert.Append(anewV);
      aEdParams.Append(apar);
      BRep_ListOfPointRepresentation& alistrep =
        (*((Handle(BRep_TVertex)*)&anewV.TShape()))->ChangePoints();
      Handle(BRep_PointOnCurve) aPRep = new BRep_PointOnCurve(apar, c3d, aLoc);
      alistrep.Append(aPRep);
      theReShape->Replace(aV, anewV);
    }
  }

  Standard_Integer newnb = aEdParams.Length();
  if (newnb < nb)
  {

    TopoShape       anewEdge = theEdge.EmptyCopied();
    TopAbs_Orientation anOri    = theEdge.Orientation();
    anewEdge.Orientation(TopAbs_FORWARD);
    ShapeBuilder aB;
    aB.Add(anewEdge, theV1);
    aB.Add(anewEdge, theV2);

    for (i = 1; i <= aEdVert.Length(); i++)
      aB.Add(anewEdge, aEdVert.Value(i));
    anewEdge.Orientation(anOri);
    theReShape->Replace(theEdge, anewEdge);
  }
}

//=======================================================================
// function : ReplaceEdge
// purpose  : internal use (static)
//=======================================================================

static void ReplaceEdge(const TopoShape&              oldEdge,
                        const TopoShape&              theNewShape,
                        const Handle(BRepTools_ReShape)& aReShape)
{
  TopoShape oldShape = aReShape->Apply(oldEdge);
  TopoShape newShape = aReShape->Apply(theNewShape);
  if (oldShape.IsSame(newShape) || aReShape->IsRecorded(newShape))
    return;

  aReShape->Replace(oldShape, newShape);
  TopoVertex V1old, V2old, V1new, V2new;
  TopExp1::Vertices(TopoDS::Edge(oldShape), V1old, V2old);
  TopAbs_Orientation Orold = oldShape.Orientation();
  TopAbs_Orientation Ornew = Orold;
  if (newShape.ShapeType() == TopAbs_EDGE)
  {
    TopoEdge aEn = TopoDS::Edge(newShape);
    TopExp1::Vertices(aEn, V1new, V2new);
    Ornew = aEn.Orientation();
    replaceNMVertices(aEn, V1new, V2new, aReShape);
  }
  else if (newShape.ShapeType() == TopAbs_WIRE)
  {
    for (ShapeExplorer aex(newShape, TopAbs_EDGE); aex.More(); aex.Next())
    {
      TopoEdge ed = TopoDS::Edge(aex.Current());
      Ornew          = ed.Orientation();
      TopoVertex aV1, aV2;
      TopExp1::Vertices(ed, aV1, aV2);
      replaceNMVertices(ed, aV1, aV2, aReShape);
      if (V1new.IsNull())
        V1new = aV1;
      V2new = aV2;
    }
  }

  V1new.Orientation(V1old.Orientation());
  V2new.Orientation(V2old.Orientation());
  if (V1old.IsSame(V2old) && !V1old.IsSame(V1new) && !aReShape->IsRecorded(V1new))
  {
    aReShape->Replace(V1old, V1new);
    return;
  }
  if (Orold == Ornew)
  {
    V1new.Orientation(V1old.Orientation());
    V2new.Orientation(V2old.Orientation());
    if (!V1old.IsSame(V1new) && !V1old.IsSame(V2new) && !aReShape->IsRecorded(V1new))
      aReShape->Replace(V1old, V1new);
    if (!V2old.IsSame(V2new) && !V2old.IsSame(V1new) && !aReShape->IsRecorded(V2new))
      aReShape->Replace(V2old, V2new);
  }
  else
  {
    V1new.Orientation(V2old.Orientation());
    V2new.Orientation(V1old.Orientation());
    if (!V1old.IsSame(V2new) && !V1old.IsSame(V1new) && !aReShape->IsRecorded(V2new))
      aReShape->Replace(V1old, V2new);
    if (!V2old.IsSame(V2new) && !V2old.IsSame(V1new) && !aReShape->IsRecorded(V1new))
      aReShape->Replace(V2old, V1new);
  }
}

//=======================================================================
// function : Merging
// purpose  : Modifies :
//                   myHasFreeBound
//
//=======================================================================

void BRepBuilderAPI_Sewing::Merging(const Standard_Boolean /* firstTime */,
                                    const Message_ProgressRange& theProgress)
{
  ShapeBuilder B;
  //  TopTools_MapOfShape MergedEdges;
  Message_ProgressScope aPS(theProgress, "Merging bounds", myBoundFaces.Extent());
  TopTools_IndexedDataMapOfShapeListOfShape::Iterator anIterB(myBoundFaces);
  for (; anIterB.More() && aPS.More(); anIterB.Next(), aPS.Next())
  {

    const TopoShape& bound = anIterB.Key1();

    // If bound was already merged - continue
    if (myMergedEdges.Contains(bound))
      continue;

    if (!anIterB.Value().Extent())
    {
      // Merge free edge - only vertices
      TopoVertex no1, no2;
      TopExp1::Vertices(TopoDS::Edge(bound), no1, no2);
      TopoShape nno1 = no1, nno2 = no2;
      if (myVertexNodeFree.Contains(no1))
        nno1 = myVertexNodeFree.FindFromKey(no1);
      if (myVertexNodeFree.Contains(no2))
        nno2 = myVertexNodeFree.FindFromKey(no2);
      if (!no1.IsSame(nno1))
      {
        nno1.Orientation(no1.Orientation());
        myReShape->Replace(no1, nno1);
      }
      if (!no2.IsSame(nno2))
      {
        nno2.Orientation(no2.Orientation());
        myReShape->Replace(no2, nno2);
      }
      myMergedEdges.Add(bound);
      continue;
    }

    // Check for previous splitting, build replacing wire
    TopoWire      BoundWire;
    Standard_Boolean isPrevSplit        = Standard_False;
    Standard_Boolean hasCuttingSections = myBoundSections.IsBound(bound);
    if (hasCuttingSections)
    {
      B.MakeWire(BoundWire);
      BoundWire.Orientation(bound.Orientation());
      // Iterate on cutting sections
      TopTools_ListIteratorOfListOfShape its(myBoundSections(bound));
      for (; its.More(); its.Next())
      {
        TopoShape section = its.Value();
        B.Add(BoundWire, section);
        if (myMergedEdges.Contains(section))
          isPrevSplit = Standard_True;
      }
    }

    // Merge with bound
    TopTools_IndexedDataMapOfShapeShape MergedWithBound;
    if (!isPrevSplit)
    {
      // Obtain sequence of edges merged with bound
      TopTools_SequenceOfShape  seqMergedWithBound;
      TColStd_SequenceOfBoolean seqMergedWithBoundOri;
      if (MergedNearestEdges(bound, seqMergedWithBound, seqMergedWithBoundOri))
      {
        // Store bound in the map
        MergedWithBound.Add(bound, bound);
        // Iterate on edges merged with bound
        Standard_Integer ii = 1;
        while (ii <= seqMergedWithBound.Length())
        {
          TopoShape iedge = seqMergedWithBound.Value(ii);
          // Remove edge if recorded as merged
          Standard_Boolean isRejected =
            (myMergedEdges.Contains(iedge) || MergedWithBound.Contains(iedge));
          if (!isRejected)
          {
            if (myBoundSections.IsBound(iedge))
            {
              // Edge is split - check sections
              TopTools_ListIteratorOfListOfShape lit(myBoundSections(iedge));
              for (; lit.More() && !isRejected; lit.Next())
              {
                const TopoShape& sec = lit.Value();
                // Remove edge (bound) if at least one of its sections already merged
                isRejected = (myMergedEdges.Contains(sec) || MergedWithBound.Contains(sec));
              }
            }
            if (!isRejected)
            {
              if (mySectionBound.IsBound(iedge))
              {
                // Edge is a section - check bound
                const TopoShape& bnd = mySectionBound(iedge);
                // Remove edge (section) if its bound already merged
                isRejected = (myMergedEdges.Contains(bnd) || MergedWithBound.Contains(bnd));
              }
            }
          }
          // To the next merged edge
          if (isRejected)
          {
            // Remove rejected edge
            seqMergedWithBound.Remove(ii);
            seqMergedWithBoundOri.Remove(ii);
          }
          else
          {
            // Process accepted edge
            MergedWithBound.Add(iedge, iedge);
            ii++;
          }
        }
        Standard_Integer nbMerged = seqMergedWithBound.Length();
        if (nbMerged)
        {
          // Create same parameter edge
          TopTools_MapOfShape ActuallyMerged;
          TopoEdge         MergedEdge = SameParameterEdge(bound,
                                                     seqMergedWithBound,
                                                     seqMergedWithBoundOri,
                                                     ActuallyMerged,
                                                     myReShape);
          Standard_Boolean    isForward  = Standard_False;
          if (!MergedEdge.IsNull())
            isForward = (MergedEdge.Orientation() == TopAbs_FORWARD);
          // Process actually merged edges
          Standard_Integer nbActuallyMerged = 0;
          for (ii = 1; ii <= nbMerged; ii++)
          {
            const TopoShape& iedge = seqMergedWithBound(ii);
            if (ActuallyMerged.Contains(iedge))
            {
              nbActuallyMerged++;
              // Record merged edge in the map
              TopAbs_Orientation orient = iedge.Orientation();
              if (!isForward)
                orient = TopAbs1::Reverse(orient);
              if (!seqMergedWithBoundOri(ii))
                orient = TopAbs1::Reverse(orient);
              MergedWithBound.ChangeFromKey(iedge) = MergedEdge.Oriented(orient);
            }
            else
              MergedWithBound.RemoveKey(iedge);
          }
          if (nbActuallyMerged)
          {
            // Record merged bound in the map
            TopAbs_Orientation orient = bound.Orientation();
            if (!isForward)
              orient = TopAbs1::Reverse(orient);
            MergedWithBound.ChangeFromKey(bound) = MergedEdge.Oriented(orient);
          }
          nbMerged = nbActuallyMerged;
        }
        // Remove bound from the map if not finally merged
        if (!nbMerged)
          MergedWithBound.RemoveKey(bound);
      }
    }
    const Standard_Boolean isMerged = !MergedWithBound.IsEmpty();

    // Merge with cutting sections
    Handle(BRepTools_ReShape)           SectionsReShape = new BRepTools_ReShape;
    TopTools_IndexedDataMapOfShapeShape MergedWithSections;
    if (hasCuttingSections)
    {
      // Iterate on cutting sections
      TopTools_ListIteratorOfListOfShape its(myBoundSections(bound));
      for (; its.More(); its.Next())
      {
        // Retrieve cutting section
        TopoShape section = its.Value();
        // Skip section if already merged
        if (myMergedEdges.Contains(section))
          continue;
        // Merge cutting section
        TopTools_SequenceOfShape  seqMergedWithSection;
        TColStd_SequenceOfBoolean seqMergedWithSectionOri;
        if (MergedNearestEdges(section, seqMergedWithSection, seqMergedWithSectionOri))
        {
          // Store section in the map
          MergedWithSections.Add(section, section);
          // Iterate on edges merged with section
          Standard_Integer ii = 1;
          while (ii <= seqMergedWithSection.Length())
          {
            TopoShape iedge = seqMergedWithSection.Value(ii);
            // Remove edge if recorded as merged
            Standard_Boolean isRejected =
              (myMergedEdges.Contains(iedge) || MergedWithSections.Contains(iedge));
            if (!isRejected)
            {
              if (myBoundSections.IsBound(iedge))
              {
                // Edge is split - check sections
                TopTools_ListIteratorOfListOfShape lit(myBoundSections(iedge));
                for (; lit.More() && !isRejected; lit.Next())
                {
                  const TopoShape& sec = lit.Value();
                  // Remove edge (bound) if at least one of its sections already merged
                  isRejected = (myMergedEdges.Contains(sec) || MergedWithSections.Contains(sec));
                }
              }
              if (!isRejected)
              {
                if (mySectionBound.IsBound(iedge))
                {
                  // Edge is a section - check bound
                  const TopoShape& bnd = mySectionBound(iedge);
                  // Remove edge (section) if its bound already merged
                  isRejected = (myMergedEdges.Contains(bnd) || MergedWithSections.Contains(bnd));
                }
              }
            }
            // To the next merged edge
            if (isRejected)
            {
              // Remove rejected edge
              seqMergedWithSection.Remove(ii);
              seqMergedWithSectionOri.Remove(ii);
            }
            else
            {
              // Process accepted edge
              MergedWithSections.Add(iedge, iedge);
              ii++;
            }
          }
          Standard_Integer nbMerged = seqMergedWithSection.Length();
          if (nbMerged)
          {
            // Create same parameter edge
            TopTools_MapOfShape ActuallyMerged;
            TopoEdge         MergedEdge = SameParameterEdge(section,
                                                       seqMergedWithSection,
                                                       seqMergedWithSectionOri,
                                                       ActuallyMerged,
                                                       SectionsReShape);
            Standard_Boolean    isForward  = Standard_False;
            if (!MergedEdge.IsNull())
              isForward = (MergedEdge.Orientation() == TopAbs_FORWARD);
            // Process actually merged edges
            Standard_Integer nbActuallyMerged = 0;
            for (ii = 1; ii <= nbMerged; ii++)
            {
              const TopoShape& iedge = seqMergedWithSection(ii);
              if (ActuallyMerged.Contains(iedge))
              {
                nbActuallyMerged++;
                // Record merged edge in the map
                TopAbs_Orientation orient = iedge.Orientation();
                if (!isForward)
                  orient = TopAbs1::Reverse(orient);
                if (!seqMergedWithSectionOri(ii))
                  orient = TopAbs1::Reverse(orient);
                const TopoShape& oedge               = MergedEdge.Oriented(orient);
                MergedWithSections.ChangeFromKey(iedge) = oedge;
                ReplaceEdge(myReShape->Apply(iedge), oedge, SectionsReShape);
              }
              else
                MergedWithSections.RemoveKey(iedge);
            }
            if (nbActuallyMerged)
            {
              // Record merged section in the map
              TopAbs_Orientation orient = section.Orientation();
              if (!isForward)
                orient = TopAbs1::Reverse(orient);
              const TopoShape& oedge                 = MergedEdge.Oriented(orient);
              MergedWithSections.ChangeFromKey(section) = oedge;
              ReplaceEdge(myReShape->Apply(section), oedge, SectionsReShape);
            }
            nbMerged = nbActuallyMerged;
          }
          // Remove section from the map if not finally merged
          if (!nbMerged)
            MergedWithSections.RemoveKey(section);
        }
        else if (isMerged)
        {
          // Reject merging of sections
          MergedWithSections.Clear();
          break;
        }
      }
    }
    const Standard_Boolean isMergedSplit = !MergedWithSections.IsEmpty();

    if (!isMerged && !isMergedSplit)
    {
      // Nothing was merged in this iteration
      if (isPrevSplit)
      {
        // Replace previously split bound
        myReShape->Replace(myReShape->Apply(bound), myReShape->Apply(BoundWire));
      }
      //      else if (hasCuttingSections) {
      //	myBoundSections.UnBind(bound); //szv: are you sure ???
      //      }
      continue;
    }

    // Set splitting flag
    Standard_Boolean isSplitted = ((!isMerged && isMergedSplit) || isPrevSplit);

    // Choose between bound and sections merging
    if (isMerged && isMergedSplit && !isPrevSplit)
    {
      // Fill map of merged cutting sections
      TopTools_IndexedMapOfShape                    MapSplitEdges;
      TopTools_IndexedDataMapOfShapeShape::Iterator anItm(MergedWithSections);
      for (; anItm.More(); anItm.Next())
      {
        const TopoShape& edge = anItm.Key1();
        MapSplitEdges.Add(edge);
      }
      // Iterate on edges merged with bound
      TopTools_IndexedDataMapOfShapeShape::Iterator anItm1(MergedWithBound);
      for (; anItm1.More(); anItm1.Next())
      {
        // Retrieve edge merged with bound
        const TopoShape& edge = anItm1.Key1();
        // Remove edge from the map
        if (MapSplitEdges.Contains(edge))
          MapSplitEdges.RemoveKey(edge);
        if (myBoundSections.IsBound(edge))
        {
          // Edge has cutting sections
          TopTools_ListIteratorOfListOfShape its(myBoundSections(edge));
          for (; its.More(); its.Next())
          {
            const TopoShape& sec = its.Value();
            // Remove section from the map
            if (MapSplitEdges.Contains(sec))
              MapSplitEdges.RemoveKey(sec);
          }
        }
      }
      // Calculate section merging tolerance
      Standard_Real MinSplitTol = RealLast();
      for (Standard_Integer ii = 1; ii <= MapSplitEdges.Extent(); ii++)
      {
        const TopoEdge& edge =
          TopoDS::Edge(MergedWithSections.FindFromKey(MapSplitEdges.FindKey(ii)));
        MinSplitTol = Min(MinSplitTol, BRepInspector::Tolerance(edge));
      }
      // Calculate bound merging tolerance
      const TopoEdge& BoundEdge    = TopoDS::Edge(MergedWithBound.FindFromKey(bound));
      Standard_Real      BoundEdgeTol = BRepInspector::Tolerance(BoundEdge);
      isSplitted = ((MinSplitTol < BoundEdgeTol + MinTolerance()) || myNonmanifold);
      isSplitted = (!MapSplitEdges.IsEmpty() && isSplitted);
    }

    if (isSplitted)
    {
      // Merging of cutting sections
      // myMergedEdges.Add(bound);
      myReShape->Replace(myReShape->Apply(bound), myReShape->Apply(BoundWire));
      TopTools_IndexedDataMapOfShapeShape::Iterator anItm(MergedWithSections);
      for (; anItm.More(); anItm.Next())
      {
        const TopoShape& oldedge = anItm.Key1();
        TopoShape        newedge = SectionsReShape->Apply(anItm.Value());
        ReplaceEdge(myReShape->Apply(oldedge), newedge, myReShape);
        myMergedEdges.Add(oldedge);
        if (myBoundSections.IsBound(oldedge))
          myBoundSections.UnBind(oldedge);
      }
    }
    else
    {
      // Merging of initial bound
      // myMergedEdges.Add(bound);
      TopTools_IndexedDataMapOfShapeShape::Iterator anItm(MergedWithBound);
      for (; anItm.More(); anItm.Next())
      {
        const TopoShape& oldedge = anItm.Key1();
        const TopoShape& newedge = anItm.Value();
        ReplaceEdge(myReShape->Apply(oldedge), newedge, myReShape);
        myMergedEdges.Add(oldedge);
        if (myBoundSections.IsBound(oldedge))
          myBoundSections.UnBind(oldedge);
      }
      if (myBoundSections.IsBound(bound))
        myBoundSections.UnBind(bound);
      if (!myMergedEdges.Contains(bound))
        myMergedEdges.Add(bound);
    }
  }

  myNbVertices = myVertexNode.Extent() + myVertexNodeFree.Extent();
  myNodeSections.Clear();
  myVertexNode.Clear();
  myVertexNodeFree.Clear();
  myCuttingNode.Clear();
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_Sewing::MergedNearestEdges(const TopoShape&        edge,
                                                           TopTools_SequenceOfShape&  SeqMergedEdge,
                                                           TColStd_SequenceOfBoolean& SeqMergedOri)
{
  // Retrieve edge nodes
  TopoVertex no1, no2;
  TopExp1::Vertices(TopoDS::Edge(edge), no1, no2);
  TopoShape     nno1 = no1, nno2 = no2;
  Standard_Boolean isNode1 = myVertexNode.Contains(no1);
  Standard_Boolean isNode2 = myVertexNode.Contains(no2);
  if (isNode1)
    nno1 = myVertexNode.FindFromKey(no1);
  if (isNode2)
    nno2 = myVertexNode.FindFromKey(no2);

  // Fill map of nodes connected to the node #1
  TopTools_IndexedMapOfShape mapVert1;
  mapVert1.Add(nno1);
  if (myCuttingNode.IsBound(nno1))
  {
    TopTools_ListIteratorOfListOfShape ilv(myCuttingNode(nno1));
    for (; ilv.More(); ilv.Next())
    {
      TopoShape v1 = ilv.Value();
      mapVert1.Add(v1);
      if (!isNode1 && myCuttingNode.IsBound(v1))
      {
        TopTools_ListIteratorOfListOfShape ilvn(myCuttingNode(v1));
        for (; ilvn.More(); ilvn.Next())
        {
          TopoShape vn = ilvn.Value();
          mapVert1.Add(vn);
        }
      }
    }
  }

  // Fill map of nodes connected to the node #2
  TopTools_MapOfShape mapVert2;
  mapVert2.Add(nno2);
  if (myCuttingNode.IsBound(nno2))
  {
    TopTools_ListIteratorOfListOfShape ilv(myCuttingNode(nno2));
    for (; ilv.More(); ilv.Next())
    {
      TopoShape v1 = ilv.Value();
      mapVert2.Add(v1);
      if (!isNode2 && myCuttingNode.IsBound(v1))
      {
        TopTools_ListIteratorOfListOfShape ilvn(myCuttingNode(v1));
        for (; ilvn.More(); ilvn.Next())
        {
          TopoShape vn = ilvn.Value();
          mapVert2.Add(vn);
        }
      }
    }
  }

  // Find all possible contiguous edges
  TopTools_SequenceOfShape seqEdges;
  seqEdges.Append(edge);
  TopTools_MapOfShape mapEdges;
  mapEdges.Add(edge);
  for (Standard_Integer i = 1; i <= mapVert1.Extent(); i++)
  {
    const TopoShape& node1 = mapVert1.FindKey(i);
    if (!myNodeSections.IsBound(node1))
      continue;
    TopTools_ListIteratorOfListOfShape ilsec(myNodeSections(node1));
    for (; ilsec.More(); ilsec.Next())
    {
      TopoShape sec = ilsec.Value();
      if (sec.IsSame(edge))
        continue;
      // Retrieve section nodes
      TopoVertex vs1, vs2;
      TopExp1::Vertices(TopoDS::Edge(sec), vs1, vs2);
      TopoShape vs1n = vs1, vs2n = vs2;
      if (myVertexNode.Contains(vs1))
        vs1n = myVertexNode.FindFromKey(vs1);
      if (myVertexNode.Contains(vs2))
        vs2n = myVertexNode.FindFromKey(vs2);
      if ((mapVert1.Contains(vs1n) && mapVert2.Contains(vs2n))
          || (mapVert1.Contains(vs2n) && mapVert2.Contains(vs1n)))
        if (mapEdges.Add(sec))
        {
          // Check for rejected cutting
          Standard_Boolean isRejected = myMergedEdges.Contains(sec);
          if (!isRejected && myBoundSections.IsBound(sec))
          {
            TopTools_ListIteratorOfListOfShape its(myBoundSections(sec));
            for (; its.More() && !isRejected; its.Next())
            {
              TopoShape section = its.Value();

              if (myMergedEdges.Contains(section))
                isRejected = Standard_True;
            }
          }
          if (!isRejected && mySectionBound.IsBound(sec))
          {
            const TopoShape& bnd = mySectionBound(sec);
            isRejected = (!myBoundSections.IsBound(bnd) || myMergedEdges.Contains(bnd));
          }

          if (!isRejected)
            seqEdges.Append(sec);
        }
    }
  }

  mapEdges.Clear();

  Standard_Boolean success = Standard_False;

  Standard_Integer nbSection = seqEdges.Length();
  if (nbSection > 1)
  {
    // Find the longest edge CCI60011
    Standard_Integer i, indRef = 1;
    if (myNonmanifold)
    {
      Standard_Real lenRef = 0.;
      for (i = 1; i <= nbSection; i++)
      {
        Standard_Real      f, l;
        Handle(GeomCurve3d) c3d = BRepInspector::Curve(TopoDS::Edge(seqEdges(i)), f, l);
        GeomAdaptor_Curve  cAdapt(c3d);
        Standard_Real      len = GCPnts_AbscissaPoint::Length(cAdapt, f, l);
        if (len > lenRef)
        {
          indRef = i;
          lenRef = len;
        }
      }
      if (indRef != 1)
      {
        TopoShape longEdge = seqEdges(indRef);
        seqEdges(indRef)      = seqEdges(1);
        seqEdges(1)           = longEdge;
      }
    }

    // Find merging candidates
    TColStd_SequenceOfBoolean   seqForward;
    TColStd_SequenceOfInteger   seqCandidates;
    TColStd_IndexedMapOfInteger mapReference;
    mapReference.Add(indRef); // Add index of reference section
    if (FindCandidates(seqEdges, mapReference, seqCandidates, seqForward))
    {
      Standard_Integer nbCandidates = seqCandidates.Length();
      // Record candidate sections
      for (i = 1; i <= nbCandidates; i++)
      {
        // Retrieve merged edge
        TopoShape     iedge = seqEdges(seqCandidates(i));
        Standard_Boolean ori   = seqForward(i) != 0;
        SeqMergedEdge.Append(iedge);
        SeqMergedOri.Append(ori);
        if (!myNonmanifold)
          break;
      }
      success = (nbCandidates != 0);
    }
  }

  return success;
}

//=======================================================================
// function : Cutting
// purpose  : Modifies :
//                     myBoundSections
//                     myNodeSections
//                     myCuttingNode
//=======================================================================

void BRepBuilderAPI_Sewing::Cutting(const Message_ProgressRange& theProgress)
{
  Standard_Integer i, nbVertices = myVertexNode.Extent();
  if (!nbVertices)
    return;
  // Create a box tree with vertices
  Standard_Real                                       eps = myTolerance * 0.5;
  BRepBuilderAPI_BndBoxTree                           aTree;
  NCollection_UBTreeFiller<Standard_Integer, Box2> aTreeFiller(aTree);
  BRepBuilderAPI_BndBoxTreeSelector                   aSelector;
  for (i = 1; i <= nbVertices; i++)
  {
    Point3d  pt = BRepInspector::Pnt(TopoDS::Vertex(myVertexNode.FindKey(i)));
    Box2 aBox;
    aBox.Set(pt);
    aBox.Enlarge(eps);
    aTreeFiller.Add(i, aBox);
  }
  aTreeFiller.Fill();

  Handle(GeomCurve3d) c3d;
  TopLoc_Location    loc;
  Standard_Real      first, last;
  // Iterate on all boundaries
  Standard_Integer                                    nbBounds = myBoundFaces.Extent();
  Message_ProgressScope                               aPS(theProgress, "Cutting bounds", nbBounds);
  TopTools_IndexedDataMapOfShapeListOfShape::Iterator anIterB(myBoundFaces);
  for (; anIterB.More() && aPS.More(); anIterB.Next(), aPS.Next())
  {
    const TopoEdge& bound = TopoDS::Edge(anIterB.Key1());
    // Do not cut floating edges
    if (!anIterB.Value().Extent())
      continue;
    // Obtain bound curve
    c3d = BRepInspector::Curve(bound, loc, first, last);
    if (c3d.IsNull())
      continue;
    if (!loc.IsIdentity())
    {
      c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
      c3d->Transform(loc.Transformation());
    }
    // Create cutting sections
    ShapeList listSections;
    { // szv: Use brackets to destroy local variables
      // Obtain candidate vertices
      TopoVertex              V1, V2;
      TopTools_IndexedMapOfShape CandidateVertices;
      { // szv: Use brackets to destroy local variables
        // Create bounding box around curve
        Box2           aGlobalBox;
        GeomAdaptor_Curve adptC(c3d, first, last);
        Add3dCurve::Add(adptC, myTolerance, aGlobalBox);
        // Sort vertices to find candidates
        aSelector.SetCurrent(aGlobalBox);
        aTree.Select(aSelector);
        // Skip bound if no node is in the boundind box
        if (!aSelector.ResInd().Extent())
          continue;
        // Retrieve bound nodes
        TopExp1::Vertices(bound, V1, V2);
        const TopoShape& Node1 = myVertexNode.FindFromKey(V1);
        const TopoShape& Node2 = myVertexNode.FindFromKey(V2);
        // Fill map of candidate vertices
        TColStd_ListIteratorOfListOfInteger itl(aSelector.ResInd());
        for (; itl.More(); itl.Next())
        {
          const Standard_Integer index = itl.Value();
          const TopoShape&    Node  = myVertexNode.FindFromIndex(index);
          if (!Node.IsSame(Node1) && !Node.IsSame(Node2))
          {
            TopoShape vertex = myVertexNode.FindKey(index);
            CandidateVertices.Add(vertex);
          }
        }
        aSelector.ClearResList();
      }
      Standard_Integer nbCandidates = CandidateVertices.Extent();
      if (!nbCandidates)
        continue;
      // Project vertices on curve
      TColStd_Array1OfReal arrPara(1, nbCandidates), arrDist(1, nbCandidates);
      TColgp_Array1OfPnt   arrPnt(1, nbCandidates), arrProj(1, nbCandidates);
      for (Standard_Integer j = 1; j <= nbCandidates; j++)
        arrPnt(j) = BRepInspector::Pnt(TopoDS::Vertex(CandidateVertices(j)));
      ProjectPointsOnCurve(arrPnt, c3d, first, last, arrDist, arrPara, arrProj, Standard_True);
      // Create cutting nodes
      TopTools_SequenceOfShape seqNode;
      TColStd_SequenceOfReal   seqPara;
      CreateCuttingNodes(CandidateVertices,
                         bound,
                         V1,
                         V2,
                         arrDist,
                         arrPara,
                         arrProj,
                         seqNode,
                         seqPara);
      if (!seqPara.Length())
        continue;
      // Create cutting sections
      CreateSections(bound, seqNode, seqPara, listSections);
    }
    if (listSections.Extent() > 1)
    {
      // modification of maps:
      //                     myBoundSections
      TopTools_ListIteratorOfListOfShape its(listSections);
      for (; its.More(); its.Next())
      {
        TopoShape section = its.Value();
        // Iterate on section vertices
        for (TopoDS_Iterator itv(section); itv.More(); itv.Next())
        {
          TopoShape vertex = itv.Value();
          // Convert vertex to node
          if (myVertexNode.Contains(vertex))
            vertex = TopoDS::Vertex(myVertexNode.FindFromKey(vertex));
          // Update node sections
          if (myNodeSections.IsBound(vertex))
            myNodeSections.ChangeFind(vertex).Append(section);
          else
          {
            ShapeList lsec;
            lsec.Append(section);
            myNodeSections.Bind(vertex, lsec);
          }
        }
        // Store bound for section
        mySectionBound.Bind(section, bound);
      }
      // Store split bound
      myBoundSections.Bind(bound, listSections);
    }
  }
#ifdef OCCT_DEBUG
  std::cout << "From " << nbBounds << " bounds " << myBoundSections.Extent() << " were cut into "
            << mySectionBound.Extent() << " sections" << std::endl;
#endif
}

//=================================================================================================

static void GetSeqEdges(const TopoShape&                 edge,
                        TopTools_SequenceOfShape&           seqEdges,
                        TopTools_DataMapOfShapeListOfShape& VertEdge)
{
  Standard_Integer numV = 0;
  for (TopoDS_Iterator Iv(edge, Standard_False); Iv.More(); Iv.Next())
  {
    const TopoVertex& V1 = TopoDS::Vertex(Iv.Value());
    numV++;
    if (VertEdge.IsBound(V1))
    {
      const ShapeList& listEdges = VertEdge.Find(V1);
      for (TopTools_ListIteratorOfListOfShape lIt(listEdges); lIt.More(); lIt.Next())
      {
        const TopoShape& edge1 = lIt.Value();
        if (edge1.IsSame(edge))
          continue;
        Standard_Boolean isContained = Standard_False;
        Standard_Integer i, index = 1;
        for (i = 1; i <= seqEdges.Length() && !isContained; i++)
        {
          isContained = seqEdges.Value(i).IsSame(edge1);
          if (!isContained && seqEdges.Value(i).IsSame(edge))
            index = i;
        }
        if (!isContained)
        {
          if (numV == 1)
            seqEdges.InsertBefore(index, edge1);
          else
            seqEdges.InsertAfter(index, edge1);
          GetSeqEdges(edge1, seqEdges, VertEdge);
        }
      }
    }
  }
}

//=================================================================================================

void BRepBuilderAPI_Sewing::GetFreeWires(TopTools_IndexedMapOfShape& MapFreeEdges,
                                         TopTools_SequenceOfShape&   seqWires)
{
  TopTools_DataMapOfShapeListOfShape VertEdge;
  TopTools_SequenceOfShape           seqFreeEdges;
  for (Standard_Integer i = 1; i <= MapFreeEdges.Extent(); i++)
  {
    const TopoShape& edge = MapFreeEdges.FindKey(i);
    seqFreeEdges.Append(edge);
    for (TopoDS_Iterator Iv(edge, Standard_False); Iv.More(); Iv.Next())
    {
      const TopoVertex& V1 = TopoDS::Vertex(Iv.Value());
      if (VertEdge.IsBound(V1))
        VertEdge.ChangeFind(V1).Append(edge);
      else
      {
        ShapeList ls;
        ls.Append(edge);
        VertEdge.Bind(V1, ls);
      }
    }
  }
  ShapeBuilder     B;
  Standard_Integer i, j;
  for (i = 1; i <= seqFreeEdges.Length(); i++)
  {
    TopTools_SequenceOfShape seqEdges;
    const TopoShape&      edge = seqFreeEdges.Value(i);
    if (!MapFreeEdges.Contains(edge))
      continue;
    seqEdges.Append(edge);
    GetSeqEdges(edge, seqEdges, VertEdge);
    TopoWire wire;
    B.MakeWire(wire);
    for (j = 1; j <= seqEdges.Length(); j++)
    {
      B.Add(wire, seqEdges.Value(j));
      MapFreeEdges.RemoveKey(seqEdges.Value(j));
    }
    seqWires.Append(wire);
    if (MapFreeEdges.IsEmpty())
      break;
  }
}

//=======================================================================
// function :  IsDegeneratedWire
// purpose  :  internal use
//=======================================================================

static Standard_Boolean IsDegeneratedWire(const TopoShape& wire)
{
  if (wire.ShapeType() != TopAbs_WIRE)
    return Standard_False;
  // Get maximal vertices tolerance
  TopoVertex V1, V2;
  // TopExp1::Vertices(TopoDS::Wire(wire),V1,V2);
  // Standard_Real tol = Max(BRepInspector::Tolerance(V1),BRepInspector::Tolerance(V2));
  Standard_Real    wireLength = 0.0;
  TopLoc_Location  loc;
  Standard_Real    first, last;
  Standard_Integer nume    = 0;
  Standard_Integer isSmall = 0;
  for (TopoDS_Iterator aIt(wire, Standard_False); aIt.More(); aIt.Next())
  {
    nume++;
    TopoShape  edge = aIt.Value();
    TopoVertex Ve1, Ve2;
    TopExp1::Vertices(TopoDS::Edge(edge), Ve1, Ve2);
    if (nume == 1)
    {
      V1 = Ve1;
      V2 = Ve2;
    }
    else
    {
      if (Ve1.IsSame(V1))
        V1 = Ve2;
      else if (Ve1.IsSame(V2))
        V2 = Ve2;
      if (Ve2.IsSame(V1))
        V1 = Ve1;
      else if (Ve2.IsSame(V2))
        V2 = Ve1;
    }
    Handle(GeomCurve3d) c3d = BRepInspector::Curve(TopoDS::Edge(aIt.Value()), loc, first, last);
    if (!c3d.IsNull())
    {
      c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
      if (!loc.IsIdentity())
      {
        // c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
        c3d->Transform(loc.Transformation());
      }
      Point3d        pfirst = c3d->Value(first);
      Point3d        plast  = c3d->Value(last);
      Point3d        pmid   = c3d->Value((first + last) * 0.5);
      Standard_Real length = 0;
      if (pfirst.Distance(plast) > pfirst.Distance(pmid))
      {
        length = pfirst.Distance(plast);
      }
      else
      {
        GeomAdaptor_Curve cAdapt(c3d);
        length = GCPnts_AbscissaPoint::Length(cAdapt, first, last);
      }
      Standard_Real tole = BRepInspector::Tolerance(Ve1) + BRepInspector::Tolerance(Ve2);
      if (length <= tole)
        isSmall++;
      wireLength += length;
    }
  }
  if (isSmall == nume)
    return Standard_True;
  // clang-format off
  Standard_Real tol = BRepInspector::Tolerance(V1)+BRepInspector::Tolerance(V2);//Max(BRepInspector::Tolerance(V1),BRepInspector::Tolerance(V2));
  // clang-format on
  if (wireLength > tol)
    return Standard_False;
  return Standard_True;
}

//=======================================================================
// function :  DegeneratedSection
// purpose  :  internal use
//            create a new degenerated edge if the section is degenerated
//=======================================================================

static TopoEdge DegeneratedSection(const TopoShape& section, const TopoShape& face)
{
  // Return if section is already degenerated
  if (BRepInspector::Degenerated(TopoDS::Edge(section)))
    return TopoDS::Edge(section);

  // Retrieve edge curve
  TopLoc_Location    loc;
  Standard_Real      first, last;
  Handle(GeomCurve3d) c3d = BRepInspector::Curve(TopoDS::Edge(section), loc, first, last);
  if (c3d.IsNull())
  { // gka
    ShapeBuilder aB;
    TopoEdge  edge1 = TopoDS::Edge(section);
    aB.Degenerated(edge1, Standard_True);
    return edge1;
  }
  if (!loc.IsIdentity())
  {
    c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
    c3d->Transform(loc.Transformation());
  }

  // Test if the new edge is degenerated
  TopoVertex v1, v2;
  TopExp1::Vertices(TopoDS::Edge(section), v1, v2);
  // Standard_Real tol = Max(BRepInspector::Tolerance(v1),BRepInspector::Tolerance(v2));
  // tol = Max(tolerance,tol);

  Point3d p1, p2, p3;
  p1 = BRepInspector::Pnt(v1);
  p3 = BRepInspector::Pnt(v2);
  c3d->D0(0.5 * (first + last), p2);

  // Standard_Boolean isDegenerated = Standard_False;
  // if (p1.Distance(p3) < tol) {
  // GeomAdaptor_Curve cAdapt(c3d);
  // Standard_Real length = GCPnts_AbscissaPoint::Length(cAdapt, first, last);
  // isDegenerated =  Standard_True; //(length < tol);
  // }

  TopoEdge edge;
  // if (!isDegenerated) return edge;

  // processing
  ShapeBuilder aBuilder;
  edge = TopoDS::Edge(section);
  edge.EmptyCopy();
  if (v1.IsSame(v2))
  {
    TopoShape anEdge = edge.Oriented(TopAbs_FORWARD);
    aBuilder.Add(anEdge, v1.Oriented(TopAbs_FORWARD));
    aBuilder.Add(anEdge, v2.Oriented(TopAbs_REVERSED));
  }
  else
  {
    TopoVertex newVertex;
    if (p1.Distance(p3) < BRepInspector::Tolerance(v1))
      newVertex = v1;
    else if (p1.Distance(p3) < BRepInspector::Tolerance(v2))
      newVertex = v2;
    else
    {
      Standard_Real d1           = BRepInspector::Tolerance(v1) + p2.Distance(p1);
      Standard_Real d2           = BRepInspector::Tolerance(v2) + p2.Distance(p3);
      Standard_Real newTolerance = Max(d1, d2);
      aBuilder.MakeVertex(newVertex, p2, newTolerance);
    }
    TopoShape anEdge = edge.Oriented(TopAbs_FORWARD);
    aBuilder.Add(anEdge, newVertex.Oriented(TopAbs_FORWARD));
    aBuilder.Add(anEdge, newVertex.Oriented(TopAbs_REVERSED));
  }

  BRepInspector::Range(TopoDS::Edge(section), first, last);
  TopoShape anEdge = edge.Oriented(TopAbs_FORWARD);
  aBuilder.Range(TopoDS::Edge(anEdge), first, last);
  aBuilder.Degenerated(edge, Standard_True);
  Handle(GeomCurve3d) aC3dNew;
  if (!face.IsNull())
  {
    Standard_Real        af, al;
    Handle(GeomCurve2d) aC2dt =
      BRepInspector::CurveOnSurface(TopoDS::Edge(section), TopoDS::Face(face), af, al);
    aBuilder.UpdateEdge(edge, aC3dNew, 0);
    Handle(GeomCurve2d) aC2dn = BRepInspector::CurveOnSurface(edge, TopoDS::Face(face), af, al);
    if (aC2dn.IsNull())
      aBuilder.UpdateEdge(edge, aC2dt, TopoDS::Face(face), 0);
  }

  return edge;
}

//=======================================================================
// function : EdgeProcessing
// purpose  : modifies :
//                       myNbEdges
//                       myHasMultipleEdge
//                       myHasFreeBound
//           . if multiple edge
//              - length < 100.*myTolerance -> several free edge
//           . if no multiple edge
//              - make the contiguous edges sameparameter
//=======================================================================

void BRepBuilderAPI_Sewing::EdgeProcessing(const Message_ProgressRange& theProgress)
{
  // constructs sectionEdge
  TopTools_IndexedMapOfShape   MapFreeEdges;
  TopTools_DataMapOfShapeShape EdgeFace;
  Message_ProgressScope        aPS(theProgress, "Edge processing", myBoundFaces.Extent());
  TopTools_IndexedDataMapOfShapeListOfShape::Iterator anIterB(myBoundFaces);
  for (; anIterB.More() && aPS.More(); anIterB.Next(), aPS.Next())
  {
    const TopoShape&         bound     = anIterB.Key1();
    const ShapeList& listFaces = anIterB.Value();
    if (listFaces.Extent() == 1)
    {
      if (myBoundSections.IsBound(bound))
      {
        TopTools_ListIteratorOfListOfShape liter(myBoundSections(bound));
        for (; liter.More(); liter.Next())
        {
          if (!myMergedEdges.Contains(liter.Value()))
          { // myReShape->IsRecorded(liter.Value())) {
            const TopoShape& edge = myReShape->Apply(liter.Value());
            if (!MapFreeEdges.Contains(edge))
            {
              const TopoShape& face = listFaces.First();
              EdgeFace.Bind(edge, face);
              MapFreeEdges.Add(edge);
            }
          }
        }
      }
      else
      {
        if (!myMergedEdges.Contains(bound))
        {
          const TopoShape& edge = myReShape->Apply(bound);
          if (!MapFreeEdges.Contains(edge))
          {
            const TopoShape& face = listFaces.First();
            EdgeFace.Bind(edge, face);
            MapFreeEdges.Add(edge);
          }
        }
      }
    }
  }

  if (!MapFreeEdges.IsEmpty())
  {
    TopTools_SequenceOfShape seqWires;
    GetFreeWires(MapFreeEdges, seqWires);
    for (Standard_Integer j = 1; j <= seqWires.Length(); j++)
    {
      const TopoWire& wire = TopoDS::Wire(seqWires.Value(j));
      if (!IsDegeneratedWire(wire))
        continue;
      for (TopoDS_Iterator Ie(wire, Standard_False); Ie.More(); Ie.Next())
      {
        TopoShape       aTmpShape = myReShape->Apply(Ie.Value()); // for porting
        const TopoEdge& edge      = TopoDS::Edge(aTmpShape);
        TopoShape       face;
        if (EdgeFace.IsBound(edge))
          face = EdgeFace.Find(edge);
        TopoShape degedge = DegeneratedSection(edge, face);
        if (degedge.IsNull())
          continue;
        if (!degedge.IsSame(edge))
          ReplaceEdge(edge, degedge, myReShape);
        if (BRepInspector::Degenerated(TopoDS::Edge(degedge)))
          myDegenerated.Add(degedge);
      }
    }
  }
}

//=======================================================================
// function : EdgeRegularity
// purpose  : update Continuity flag on newly created edges
//=======================================================================

void BRepBuilderAPI_Sewing::EdgeRegularity(const Message_ProgressRange& theProgress)
{
  TopTools_IndexedDataMapOfShapeListOfShape aMapEF;
  TopExp1::MapShapesAndAncestors(mySewedShape, TopAbs_EDGE, TopAbs_FACE, aMapEF);

  Message_ProgressScope aPS(theProgress, "Encode edge regularity", myMergedEdges.Extent());
  for (TopTools_MapIteratorOfMapOfShape aMEIt(myMergedEdges); aMEIt.More() && aPS.More();
       aMEIt.Next(), aPS.Next())
  {
    TopoEdge                 anEdge = TopoDS::Edge(myReShape->Apply(aMEIt.Value()));
    const ShapeList* aFaces = aMapEF.Seek(anEdge);
    // encode regularity if and only if edges is shared by two faces
    if (aFaces && aFaces->Extent() == 2)
      BRepLib1::EncodeRegularity(anEdge,
                                TopoDS::Face(aFaces->First()),
                                TopoDS::Face(aFaces->Last()));
  }

  myMergedEdges.Clear();
}

//=================================================================================================

void BRepBuilderAPI_Sewing::CreateSewedShape()
{
  // ---------------------
  // create the new shapes
  // ---------------------
  ShapeQuilt  aQuilt;
  Standard_Boolean isLocal = !myShape.IsNull();
  if (isLocal)
  {
    // Local sewing
    TopoShape ns = myReShape->Apply(myShape);
    aQuilt.Add(ns);
  }
  TopTools_IndexedDataMapOfShapeShape::Iterator anIter(myOldShapes);
  for (; anIter.More(); anIter.Next())
  {
    TopoShape sh = anIter.Value();
    if (!sh.IsNull())
    {
      sh                   = myReShape->Apply(sh);
      anIter.ChangeValue() = sh;
      if (!isLocal)
        aQuilt.Add(sh);
    }
  }
  TopoShape     aNewShape = aQuilt.Shells();
  Standard_Integer numsh     = 0;

  TopTools_IndexedMapOfShape OldShells;

  ShapeBuilder    aB;
  TopoCompound aComp;
  aB.MakeCompound(aComp);
  for (TopoDS_Iterator aExpSh(aNewShape, Standard_False); aExpSh.More(); aExpSh.Next())
  {
    TopoShape     sh       = aExpSh.Value();
    Standard_Boolean hasEdges = Standard_False;
    if (sh.ShapeType() == TopAbs_SHELL)
    {
      if (myNonmanifold)
        hasEdges = !OldShells.Contains(sh);
      else
      {
        TopoShape     face;
        Standard_Integer numf = 0;
        for (ShapeExplorer aExpF(sh, TopAbs_FACE); aExpF.More() && (numf < 2); aExpF.Next())
        {
          face = aExpF.Current();
          numf++;
        }
        if (numf == 1)
          aB.Add(aComp, face);
        else if (numf > 1)
          aB.Add(aComp, sh);
        if (numf)
          numsh++;
      }
    }
    else if (sh.ShapeType() == TopAbs_FACE)
    {
      if (myNonmanifold)
      {
        TopoShell ss;
        aB.MakeShell(ss);
        aB.Add(ss, sh);
        sh       = ss;
        hasEdges = Standard_True;
      }
      else
      {
        aB.Add(aComp, sh);
        numsh++;
      }
    }
    else
    {
      aB.Add(aComp, sh);
      numsh++;
    }
    if (hasEdges)
      OldShells.Add(sh);
  }
  // Process collected shells
  if (myNonmanifold)
  {
    Standard_Integer nbOldShells = OldShells.Extent();
    if (nbOldShells == 1)
    {
      // Single shell - check for single face
      const TopoShape& sh = OldShells.FindKey(1);
      TopoShape        face;
      Standard_Integer    numf = 0;
      for (ShapeExplorer aExpF(sh, TopAbs_FACE); aExpF.More() && (numf < 2); aExpF.Next())
      {
        face = aExpF.Current();
        numf++;
      }
      if (numf == 1)
        aB.Add(aComp, face);
      else if (numf > 1)
        aB.Add(aComp, sh);
      if (numf)
        numsh++;
    }
    else if (nbOldShells)
    {
      // Several shells should be merged
      TColStd_MapOfInteger IndexMerged;
      while (IndexMerged.Extent() < nbOldShells)
      {
        TopoShell        NewShell;
        TopTools_MapOfShape NewEdges;
        for (Standard_Integer i = 1; i <= nbOldShells; i++)
        {
          if (IndexMerged.Contains(i))
            continue;
          const TopoShell& shell = TopoDS::Shell(OldShells.FindKey(i));
          if (NewShell.IsNull())
          {
            aB.MakeShell(NewShell);
            TopoDS_Iterator aItSS(shell);
            for (; aItSS.More(); aItSS.Next())
              aB.Add(NewShell, aItSS.Value());
            // Fill map of edges
            for (ShapeExplorer eexp(shell, TopAbs_EDGE); eexp.More(); eexp.Next())
            {
              const TopoShape& edge = eexp.Current();
              NewEdges.Add(edge);
            }
            IndexMerged.Add(i);
          }
          else
          {
            Standard_Boolean hasSharedEdge = Standard_False;
            ShapeExplorer  eexp(shell, TopAbs_EDGE);
            for (; eexp.More() && !hasSharedEdge; eexp.Next())
              hasSharedEdge = NewEdges.Contains(eexp.Current());
            if (hasSharedEdge)
            {
              // Add edges to the map
              for (ShapeExplorer eexp1(shell, TopAbs_EDGE); eexp1.More(); eexp1.Next())
              {
                const TopoShape& edge = eexp1.Current();
                NewEdges.Add(edge);
              }
              // Add faces to the shell
              for (ShapeExplorer fexp(shell, TopAbs_FACE); fexp.More(); fexp.Next())
              {
                const TopoShape& face = fexp.Current();
                aB.Add(NewShell, face);
              }
              IndexMerged.Add(i);
            }
          }
        }
        // Process new shell
        TopoShape     face;
        Standard_Integer numf = 0;
        ShapeExplorer  aExpF(NewShell, TopAbs_FACE);
        for (; aExpF.More() && (numf < 2); aExpF.Next())
        {
          face = aExpF.Current();
          numf++;
        }
        if (numf == 1)
          aB.Add(aComp, face);
        else if (numf > 1)
          aB.Add(aComp, NewShell);
        if (numf)
          numsh++;
      }
    }
  }
  if (numsh == 1)
  {
    // Extract single component
    TopoDS_Iterator aIt(aComp, Standard_False);
    mySewedShape = aIt.Value();
  }
  else
    mySewedShape = aComp;
}

//=======================================================================
// function : CreateOutputInformations
// purpose  : constructs :
//                       myEdgeSections
//                       mySectionBound
//                       myNbFreeEdges
//                       myNbContigousEdges
//                       myNbMultipleEdges
//                       myNbDegenerated
//=======================================================================

void BRepBuilderAPI_Sewing::CreateOutputInformations()
{
  // Construct edgeSections
  Standard_Integer i;
  // TopTools_DataMapOfShapeListOfShape edgeSections;
  // clang-format off
  TopTools_IndexedDataMapOfShapeListOfShape edgeSections; //use index map for regulating free edges
  // clang-format on
  for (i = 1; i <= myBoundFaces.Extent(); i++)
  {
    const TopoShape&  bound = myBoundFaces.FindKey(i);
    ShapeList lsect;
    if (myBoundSections.IsBound(bound))
      lsect = myBoundSections(bound);
    ShapeExplorer aExp(myReShape->Apply(bound), TopAbs_EDGE);
    for (; aExp.More(); aExp.Next())
    {
      TopoShape                       sec  = bound;
      const TopoShape&                edge = aExp.Current();
      TopTools_ListIteratorOfListOfShape aI(lsect);
      for (; aI.More(); aI.Next())
      {
        const TopoShape& section = aI.Value();
        if (edge.IsSame(myReShape->Apply(section)))
        {
          sec = section;
          break;
        }
      }
      if (edgeSections.Contains(edge))
        edgeSections.ChangeFromKey(edge).Append(sec);
      else
      {
        ShapeList listSec;
        listSec.Append(sec);
        edgeSections.Add(edge, listSec);
      }
    }
  }

  // Fill maps of Free, Contiguous and Multiple edges
  TopTools_IndexedDataMapOfShapeListOfShape::Iterator anIter(edgeSections);
  for (; anIter.More(); anIter.Next())
  {
    const TopoShape&         edge        = anIter.Key1();
    const ShapeList& listSection = anIter.Value();
    if (listSection.Extent() == 1)
    {
      if (BRepInspector::Degenerated(TopoDS::Edge(edge)))
        myDegenerated.Add(edge);
      else
        myFreeEdges.Add(edge);
    }
    else if (listSection.Extent() == 2)
    {
      myContigousEdges.Add(edge, listSection);
    }
    else
    {
      myMultipleEdges.Add(edge);
    }
  }

  // constructs myContigSectBound
  TopTools_DataMapOfShapeListOfShape aEdgeMap; // gka
  for (i = 1; i <= myBoundFaces.Extent(); i++)
  {
    const TopoShape& bound = myBoundFaces.FindKey(i);
    if (myBoundSections.IsBound(bound))
    {
      TopTools_ListIteratorOfListOfShape iter(myBoundSections(bound));
      for (; iter.More(); iter.Next())
      {
        const TopoShape& section = iter.Value();
        if (!myMergedEdges.Contains(section))
          continue;
        // if (!myReShape->IsRecorded(section)) continue; // section is free
        TopoShape nedge = myReShape->Apply(section);
        if (nedge.IsNull())
          continue; // szv debug
        if (!bound.IsSame(section))
          if (myContigousEdges.Contains(nedge))
            myContigSecBound.Bind(section, bound);
      }
    }
  }
}

//=================================================================================================

void BRepBuilderAPI_Sewing::ProjectPointsOnCurve(const TColgp_Array1OfPnt& arrPnt,
                                                 const Handle(GeomCurve3d)& c3d,
                                                 const Standard_Real       first,
                                                 const Standard_Real       last,
                                                 TColStd_Array1OfReal&     arrDist,
                                                 TColStd_Array1OfReal&     arrPara,
                                                 TColgp_Array1OfPnt&       arrProj,
                                                 const Standard_Boolean    isConsiderEnds) const
{
  arrDist.Init(-1.0);

  GeomAdaptor_Curve GAC(c3d);
  Extrema_ExtPC     locProj;
  locProj.Initialize(GAC, first, last);
  Point3d           pfirst = GAC.Value(first), plast = GAC.Value(last);
  Standard_Integer find = 1;            //(isConsiderEnds ? 1 : 2);
                                        // clang-format off
  Standard_Integer lind = arrPnt.Length();//(isConsiderEnds ? arrPnt.Length() : arrPnt.Length() -1);
                                        // clang-format on

  for (Standard_Integer i1 = find; i1 <= lind; i1++)
  {
    Point3d           pt          = arrPnt(i1);
    Standard_Real    worktol     = myTolerance;
    Standard_Real    distF2      = pfirst.SquareDistance(pt);
    Standard_Real    distL2      = plast.SquareDistance(pt);
    Standard_Boolean isProjected = Standard_False;
    try
    {

      // Project current point on curve
      locProj.Perform(pt);
      if (locProj.IsDone() && locProj.NbExt() > 0)
      {
        Standard_Real dist2Min =
          (isConsiderEnds || i1 == find || i1 == lind ? Min(distF2, distL2)
                                                      : Precision1::Infinite());
        Standard_Integer ind, indMin = 0;
        for (ind = 1; ind <= locProj.NbExt(); ind++)
        {
          Standard_Real dProj2 = locProj.SquareDistance(ind);
          if (dProj2 < dist2Min)
          {
            indMin   = ind;
            dist2Min = dProj2;
          }
        }
        if (indMin)
        {
          isProjected               = Standard_True;
          PointOnCurve1 pOnC      = locProj.Point(indMin);
          Standard_Real   paramProj = pOnC.Parameter();
          Point3d          ptProj    = GAC.Value(paramProj);
          Standard_Real   distProj2 = ptProj.SquareDistance(pt);
          if (!locProj.IsMin(indMin))
          {
            if (Min(distF2, distL2) < dist2Min)
            {
              if (distF2 < distL2)
              {
                paramProj = first;
                distProj2 = distF2;
                ptProj    = pfirst;
              }
              else
              {
                paramProj = last;
                distProj2 = distL2;
                ptProj    = plast;
              }
            }
          }
          if (distProj2 < worktol * worktol || !isConsiderEnds)
          {
            arrDist(i1) = sqrt(distProj2);
            arrPara(i1) = paramProj;
            arrProj(i1) = ptProj;
          }
        }
      }
    }
    catch (ExceptionBase const& anException)
    {
#ifdef OCCT_DEBUG
      std::cout << "Exception in BRepBuilderAPI_Sewing::ProjectPointsOnCurve: ";
      anException.Print(std::cout);
      std::cout << std::endl;
#endif
      (void)anException;
      worktol = MinTolerance();
    }
    if (!isProjected && isConsiderEnds)
    {
      if (Min(distF2, distL2) < worktol * worktol)
      {
        if (distF2 < distL2)
        {
          arrDist(i1) = sqrt(distF2);
          arrPara(i1) = first;
          arrProj(i1) = pfirst;
        }
        else
        {
          arrDist(i1) = sqrt(distL2);
          arrPara(i1) = last;
          arrProj(i1) = plast;
        }
      }
    }
  }
}

//=================================================================================================

void BRepBuilderAPI_Sewing::CreateCuttingNodes(const TopTools_IndexedMapOfShape& MapVert,
                                               const TopoShape&               bound,
                                               const TopoShape&               vfirst,
                                               const TopoShape&               vlast,
                                               const TColStd_Array1OfReal&       arrDist,
                                               const TColStd_Array1OfReal&       arrPara,
                                               const TColgp_Array1OfPnt&         arrPnt,
                                               TopTools_SequenceOfShape&         seqVert,
                                               TColStd_SequenceOfReal&           seqPara)
{
  Standard_Integer i, j, nbProj = MapVert.Extent();

  // Reorder projections by distance
  TColStd_SequenceOfInteger seqOrderedIndex;
  { // szv: Use brackets to destroy local variables
    TColStd_SequenceOfReal seqOrderedDistance;
    for (i = 1; i <= nbProj; i++)
    {
      Standard_Real distProj = arrDist(i);
      if (distProj < 0.0)
        continue; // Skip vertex if not projected
      Standard_Boolean isInserted = Standard_False;
      for (j = 1; j <= seqOrderedIndex.Length() && !isInserted; j++)
      {
        isInserted = (distProj < seqOrderedDistance(j));
        if (isInserted)
        {
          seqOrderedIndex.InsertBefore(j, i);
          seqOrderedDistance.InsertBefore(j, distProj);
        }
      }
      if (!isInserted)
      {
        seqOrderedIndex.Append(i);
        seqOrderedDistance.Append(distProj);
      }
    }
  }
  nbProj = seqOrderedIndex.Length();
  if (!nbProj)
    return;

  ShapeBuilder aBuilder;

  // Insert two initial vertices (to be removed later)
  TColStd_SequenceOfReal seqDist;
  TColgp_SequenceOfPnt   seqPnt;
  { // szv: Use brackets to destroy local variables
    // Retrieve bound curve
    TopLoc_Location    loc;
    Standard_Real      first, last;
    Handle(GeomCurve3d) c3d = BRepInspector::Curve(TopoDS::Edge(bound), loc, first, last);
    if (!loc.IsIdentity())
    {
      c3d = Handle(GeomCurve3d)::DownCast(c3d->Copy());
      c3d->Transform(loc.Transformation());
    }
    GeomAdaptor_Curve GAC(c3d);
    seqVert.Prepend(vfirst);
    seqVert.Append(vlast);
    seqPara.Prepend(first);
    seqPara.Append(last);
    seqDist.Prepend(-1.0);
    seqDist.Append(-1.0);
    seqPnt.Prepend(GAC.Value(first));
    seqPnt.Append(GAC.Value(last));
  }

  TopTools_IndexedDataMapOfShapeShape NodeCuttingVertex;
  for (i = 1; i <= nbProj; i++)
  {

    const Standard_Integer index   = seqOrderedIndex(i);
    Standard_Real          disProj = arrDist(index);
    Point3d                 pntProj = arrPnt(index);

    // Skip node if already bound to cutting vertex
    TopoShape node = myVertexNode.FindFromKey(MapVert(index));
    if (NodeCuttingVertex.Contains(node))
      continue;

    // Find the closest vertex
    Standard_Integer indexMin = 1;
    Standard_Real    dist, distMin = pntProj.Distance(seqPnt(1));
    for (j = 2; j <= seqPnt.Length(); j++)
    {
      dist = pntProj.Distance(seqPnt(j));
      if (dist < distMin)
      {
        distMin  = dist;
        indexMin = j;
      }
    }

    // Check if current point is close to one of the existent
    if (distMin <= Max(disProj * 0.1, MinTolerance()))
    {
      // Check distance if close
      Standard_Real jdist = seqDist.Value(indexMin);
      if (jdist < 0.0)
      {
        // Bind new cutting node (end vertex only)
        seqDist.SetValue(indexMin, disProj);
        const TopoShape& cvertex = seqVert.Value(indexMin);
        NodeCuttingVertex.Add(node, cvertex);
      }
      else
      {
        // Bind secondary cutting nodes
        NodeCuttingVertex.Add(node, TopoVertex());
      }
    }
    else
    {
      // Build new cutting vertex
      TopoVertex cvertex;
      aBuilder.MakeVertex(cvertex, pntProj, Precision1::Confusion());
      // Bind new cutting vertex
      NodeCuttingVertex.Add(node, cvertex);
      // Insert cutting vertex in the sequences
      Standard_Real parProj = arrPara(index);
      for (j = 2; j <= seqPara.Length(); j++)
      {
        if (parProj <= seqPara.Value(j))
        {
          seqVert.InsertBefore(j, cvertex);
          seqPara.InsertBefore(j, parProj);
          seqDist.InsertBefore(j, disProj);
          seqPnt.InsertBefore(j, pntProj);
          break;
        }
      }
    }
  }

  // filling map for cutting nodes
  TopTools_IndexedDataMapOfShapeShape::Iterator aMIt(NodeCuttingVertex);
  for (; aMIt.More(); aMIt.Next())
  {
    TopoShape cnode = aMIt.Value();
    // Skip secondary nodes
    if (cnode.IsNull())
      continue;
    // Obtain vertex node
    const TopoShape& node = aMIt.Key1();
    if (myVertexNode.Contains(cnode))
    {
      // This is an end vertex
      cnode = myVertexNode.FindFromKey(cnode);
    }
    else
    {
      // Create link: cutting vertex -> node
      ShapeList ls;
      ls.Append(node);
      myCuttingNode.Bind(cnode, ls);
    }
    // Create link: node -> cutting vertex
    if (myCuttingNode.IsBound(node))
    {
      myCuttingNode.ChangeFind(node).Append(cnode);
    }
    else
    {
      ShapeList ls;
      ls.Append(cnode);
      myCuttingNode.Bind(node, ls);
    }
  }

  // Remove two initial vertices
  seqVert.Remove(1);
  seqVert.Remove(seqVert.Length());
  seqPara.Remove(1);
  seqPara.Remove(seqPara.Length());
}

//=================================================================================================

void BRepBuilderAPI_Sewing::CreateSections(const TopoShape&             section,
                                           const TopTools_SequenceOfShape& seqNode,
                                           const TColStd_SequenceOfReal&   seqPara,
                                           ShapeList&           listEdge)
{
  const TopoEdge& sec = TopoDS::Edge(section);
  //  TopAbs_Orientation aInitOr = sec.Orientation();

  // To keep NM vertices on edge
  TopTools_SequenceOfShape aSeqNMVert;
  TColStd_SequenceOfReal   aSeqNMPars;
  findNMVertices(sec, aSeqNMVert, aSeqNMPars);

  ShapeBuilder aBuilder;

  Standard_Real first, last;
  BRepInspector::Range(sec, first, last);

  // Create cutting sections
  Standard_Real    par1, par2;
  TopoShape     V1, V2;
  Standard_Integer i, len = seqPara.Length() + 1;
  for (i = 1; i <= len; i++)
  {

    TopoEdge edge = sec;
    edge.EmptyCopy();

    if (i == 1)
    {
      par1 = first;
      par2 = seqPara(i);
      V1   = TopExp1::FirstVertex(sec);
      V2   = seqNode(i);
    }
    else if (i == len)
    {
      par1 = seqPara(i - 1);
      par2 = last;
      V1   = seqNode(i - 1);
      V2   = TopExp1::LastVertex(sec);
    }
    else
    {
      par1 = seqPara(i - 1);
      par2 = seqPara(i);
      V1   = seqNode(i - 1);
      V2   = seqNode(i);
    }

    TopoShape aTmpShape = edge.Oriented(TopAbs_FORWARD);
    TopoEdge  aTmpEdge  = TopoDS::Edge(aTmpShape); // for porting
    aTmpShape              = V1.Oriented(TopAbs_FORWARD);
    aBuilder.Add(aTmpEdge, aTmpShape);
    aTmpShape = V2.Oriented(TopAbs_REVERSED);
    aBuilder.Add(aTmpEdge, aTmpShape);
    aBuilder.Range(aTmpEdge, par1, par2);
    //    if(aInitOr == TopAbs_REVERSED)
    //      listEdge.Prepend(edge);
    //    else

    Standard_Integer k = 1;
    for (; k <= aSeqNMPars.Length(); k++)
    {
      Standard_Real apar = aSeqNMPars.Value(k);
      if (apar >= par1 && apar <= par2)
      {
        aBuilder.Add(aTmpEdge, aSeqNMVert.Value(k));
        aSeqNMVert.Remove(k);
        aSeqNMPars.Remove(k);
        k--;
      }
    }
    listEdge.Append(edge);
  }

  const ShapeList& listFaces = myBoundFaces.FindFromKey(sec);
  if (!listFaces.Extent())
    return;

  Standard_Real tolEdge = BRepInspector::Tolerance(sec);

  // Add cutting pcurves
  TopTools_ListIteratorOfListOfShape itf(listFaces);
  for (; itf.More(); itf.Next())
  {

    const TopoFace& fac = TopoDS::Face(itf.Value());

    // Retrieve curve on surface
    Standard_Real        first2d = 0., last2d = 0., first2d1 = 0, last2d1 = 0.;
    Handle(GeomCurve2d) c2d = BRepInspector::CurveOnSurface(sec, fac, first2d, last2d);
    if (c2d.IsNull())
      continue;
    Handle(GeomCurve2d) c2d1;
    Standard_Boolean     isSeam = BRepInspector::IsClosed(sec, fac);

    // gka  - Convert to BSpline was commented because
    // it is not necessary to create BSpline instead of Lines or cIrcles.
    // Besides after conversion circles to BSpline
    // it is necessary to recompute parameters of cutting because parameterization of created
    // BSpline curve differs from parametrization of circle.

    // Convert pcurve to BSpline
    /*Handle(Geom2d_BSplineCurve) c2dBSP,c2dBSP1;
    if (c2d->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve))) {
    c2dBSP = Handle(Geom2d_BSplineCurve)::DownCast(c2d);
    }
    else {
    if (first > (c2d->FirstParameter() + Precision1::PConfusion()) ||
    last < (c2d->LastParameter() - Precision1::PConfusion())) {
    Handle(Geom2d_TrimmedCurve) TC = new Geom2d_TrimmedCurve(c2d, first, last);
    c2dBSP = Geom2dConvert1::CurveToBSplineCurve(TC);
    }
    else c2dBSP = Geom2dConvert1::CurveToBSplineCurve(c2d);
    }
    if (c2dBSP.IsNull()) continue;*/
    // gka fix for bug OCC12203 21.04.06 addition second curve for seam edges

    if (isSeam)
    {
      TopoEdge secRev = TopoDS::Edge(sec.Reversed());

      c2d1 = BRepInspector::CurveOnSurface(secRev, fac, first2d1, last2d1);
      if (c2d1.IsNull())
        continue;

      /*if (c2d1->IsKind(STANDARD_TYPE(Geom2d_BSplineCurve))) {
      c2dBSP1 = Handle(Geom2d_BSplineCurve)::DownCast(c2d1);
      }
      else {
      if (first > (c2d1->FirstParameter() + Precision1::PConfusion()) ||
      last < (c2d1->LastParameter() - Precision1::PConfusion())) {
      Handle(Geom2d_TrimmedCurve) TC = new Geom2d_TrimmedCurve(c2d1, first, last);
      //c2dBSP1 = Geom2dConvert1::CurveToBSplineCurve(TC);
      }
      //else c2dBSP1 = Geom2dConvert1::CurveToBSplineCurve(c2d);

      }*/
    }

    /*first2d = c2dBSP->FirstParameter();
    last2d = c2dBSP->LastParameter();

    if(!c2dBSP1.IsNull()) {
    first2d1 = c2dBSP1->FirstParameter();
    last2d1 = c2dBSP1->LastParameter();
    }*/

    // Update cutting sections
    Handle(GeomCurve2d)               c2dNew, c2d1New;
    TopTools_ListIteratorOfListOfShape ite(listEdge);
    for (; ite.More(); ite.Next())
    {

      // Retrieve cutting section
      const TopoEdge& edge = TopoDS::Edge(ite.Value());
      BRepInspector::Range(edge, par1, par2);

      // Cut BSpline pcurve
      // try {
      c2dNew = Handle(GeomCurve2d)::DownCast(c2d->Copy());
      // c2dNew = Handle(GeomCurve2d)::DownCast(c2dBSP->Copy());
      // Handle(Geom2d_BSplineCurve)::DownCast(c2dNew)->Segment1(Max(first2d,par1),Min(par2,last2d));
      if (!c2d1.IsNull())
      { // if(!c2dBSP1.IsNull()) {
        c2d1New = Handle(GeomCurve2d)::DownCast(c2d1->Copy());
        // c2d1New = Handle(GeomCurve2d)::DownCast(c2dBSP1->Copy());
        // Handle(Geom2d_BSplineCurve)::DownCast(c2d1New)->Segment1(Max(first2d1,par1),Min(par2,last2d1));
      }
      //}
      /*catch (ExceptionBase) {
      #ifdef OCCT_DEBUG
      std::cout << "Exception in CreateSections: segment [" << par1 << "," << par2 << "]: ";
      ExceptionBase::Caught()->Print(std::cout); std::cout << std::endl;
      #endif
      Handle(Geom2d_TrimmedCurve) c2dT = new
      Geom2d_TrimmedCurve(c2dNew,Max(first2d,par1),Min(par2,last2d)); c2dNew = c2dT;
      }*/

      if (!isSeam && c2d1New.IsNull())
        aBuilder.UpdateEdge(edge, c2dNew, fac, tolEdge);
      else
      {
        TopAbs_Orientation Ori = edge.Orientation();
        if (fac.Orientation() == TopAbs_REVERSED)
          Ori = TopAbs1::Reverse(Ori);

        if (Ori == TopAbs_FORWARD)
          aBuilder.UpdateEdge(edge, c2dNew, c2d1New, fac, tolEdge);
        else
          aBuilder.UpdateEdge(edge, c2d1New, c2dNew, fac, tolEdge);
      }
    }
  }
}

//=================================================================================================

void BRepBuilderAPI_Sewing::SameParameterShape()
{
  if (!mySameParameterMode)
    return;
  ShapeExplorer exp(mySewedShape, TopAbs_EDGE);
  // Le flag sameparameter est a false pour chaque edge cousue
  for (; exp.More(); exp.Next())
  {
    const TopoEdge& sec = TopoDS::Edge(exp.Current());
    try
    {

      BRepLib1::SameParameter(sec, BRepInspector::Tolerance(sec));
    }
    catch (ExceptionBase const&)
    {
#ifdef OCCT_DEBUG
      std::cout
        << "Fail: BRepBuilderAPI_Sewing::SameParameterShape exception in BRepLib1::SameParameter"
        << std::endl;
#endif
      continue;
    }
  }
}

//=======================================================================
// function : Inspect
// purpose  : Used for selection and storage of coinciding points
//=======================================================================

NCollection_CellFilter_Action BRepBuilderAPI_VertexInspector::Inspect(
  const Standard_Integer theTarget)
{
  /*Point3d aPnt = Point3d (myPoints.Value (theTarget - 1));
  if (aPnt.SquareDistance (Point3d (myCurrent)) <= myTol)
    myResInd.Append (theTarget);*/

  const Coords3d& aPnt = myPoints.Value(theTarget - 1);
  Standard_Real aDx, aDy, aDz;
  aDx = myCurrent.X() - aPnt.X();
  aDy = myCurrent.Y() - aPnt.Y();
  aDz = myCurrent.Z() - aPnt.Z();

  if ((aDx * aDx <= myTol) && (aDy * aDy <= myTol) && (aDz * aDz <= myTol))
    myResInd.Append(theTarget);
  return CellFilter_Keep;
}

//=================================================================================================

const Handle(BRepTools_ReShape)& BRepBuilderAPI_Sewing::GetContext() const
{
  return myReShape;
}

//=================================================================================================

void BRepBuilderAPI_Sewing::SetContext(const Handle(BRepTools_ReShape)& theContext)
{
  myReShape = theContext;
}
