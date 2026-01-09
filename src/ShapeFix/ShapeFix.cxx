// Created on: 2000-01-21
// Created by: data exchange team
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

#include <ShapeFix.hxx>
//: k2 abv 16.12.98: eliminating code duplication
// pdn     18.12.98: checking deviation for SP edges
//:   abv 22.02.99: method FillFace() removed since PRO13123 is fixed
// szv#4 S4163
// szv#9:S4244:19Aug99: Added method FixWireGaps
// szv#10:S4244:23Aug99: Added method FixFaceGaps

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>

#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>

#include <Precision.hxx>

#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Face.hxx>
#include <Geom_Surface.hxx>

//: i2
#include <gp_Pnt.hxx>
#include <Geom_Plane.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <ShapeAnalysis_Surface.hxx>

#include <ShapeFix_Edge.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeFix_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <TopTools_MapOfShape.hxx>
#include <BRepLib.hxx>

#include <ShapeBuild_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopExp.hxx>

#include <Message_ProgressScope.hxx>
#include <Message_Msg.hxx>
#include <ShapeExtend_BasicMsgRegistrator.hxx>

//=================================================================================================

Standard_Boolean ShapeFix1::SameParameter(const TopoShape&                            shape,
                                         const Standard_Boolean                         enforce,
                                         const Standard_Real                            preci,
                                         const Message_ProgressRange&                   theProgress,
                                         const Handle(BasicMsgRegistrator)& theMsgReg)
{
  // Calculate number of edges
  Standard_Integer aNbEdges = 0;
  for (ShapeExplorer anEdgeExp(shape, TopAbs_EDGE); anEdgeExp.More(); anEdgeExp.Next())
    ++aNbEdges;

  // Calculate number of faces
  Standard_Integer aNbFaces = 0;
  for (ShapeExplorer anEdgeExp(shape, TopAbs_FACE); anEdgeExp.More(); anEdgeExp.Next())
    ++aNbFaces;

  TopTools_IndexedDataMapOfShapeListOfShape aMapEF;
  TopExp1::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, aMapEF);

  ShapeBuilder B;
#ifdef OCCT_DEBUG
  Standard_Integer nbfail = 0, numedge = 0;
#endif
  Standard_Boolean      status = Standard_True;
  Standard_Real         tol    = preci;
  Standard_Boolean      iatol  = (tol > 0);
  Handle(ShapeFix_Edge) sfe    = new ShapeFix_Edge;
  ShapeExplorer       ex(shape, TopAbs_EDGE);
  Message_Msg           doneMsg("FixEdge.SameParameter.MSG0");

  // Start progress scope (no need to check if progress exists -- it is safe)
  Message_ProgressScope aPSForSameParam(theProgress, "Fixing same parameter problem", 2);

  {
    // Start progress scope (no need to check if progress exists -- it is safe)
    Message_ProgressScope aPS(aPSForSameParam.Next(), "Fixing edge", aNbEdges);

    while (ex.More())
    {
      TopoEdge E;
      while (ex.More() && aPS.More())
      {
#ifdef OCCT_DEBUG
        numedge++;
#endif
        int             ierr = 0;
        TopLoc_Location loc;
        E = TopoDS::Edge(ex.Current());
        ex.Next();

        if (!iatol)
          tol = BRepInspector::Tolerance(E);
        if (enforce)
        {
          B.SameRange(E, Standard_False);
          B.SameParameter(E, Standard_False);
        }

        ShapeList aListOfFaces;
        aMapEF.FindFromKey(E, aListOfFaces);
        if (aListOfFaces.Extent() != 0)
        {
          ShapeList::Iterator aListOfFacesIt(aListOfFaces);
          for (; aListOfFacesIt.More(); aListOfFacesIt.Next())
          {
            TopoFace aF = TopoDS::Face(aListOfFacesIt.Value());
            sfe->FixSameParameter(E, aF);
          }
        }
        else
        {
          sfe->FixSameParameter(E); // K2-SEP97
        }

        if (!BRepInspector::SameParameter(E))
        {
          ierr = 1;
#ifdef OCCT_DEBUG
          nbfail++;
#endif
        }

        if (ierr)
        {
          status = Standard_False;
          B.SameRange(E, Standard_False);
          B.SameParameter(E, Standard_False);
        }
        else if (!theMsgReg.IsNull() && !sfe->Status(ShapeExtend_OK))
        {
          theMsgReg->Send(E, doneMsg, Message_Warning);
        }

        // Complete step in current progress scope
        aPS.Next();
      } // -- end while

      // Halt algorithm in case of user's abort
      if (!aPS.More())
        return Standard_False;
    }
  }

  {
    // Start progress scope (no need to check if progress exists -- it is safe)
    Message_ProgressScope aPS(aPSForSameParam.Next(), "Update tolerances", aNbFaces);

    //: i2 abv 21 Aug 98: ProSTEP TR8 Motor.rle face 710:
    // Update tolerance of edges on planes (no pcurves are stored)
    for (ShapeExplorer exp(shape, TopAbs_FACE); exp.More() && aPS.More(); exp.Next(), aPS.Next())
    {
      TopoFace          face = TopoDS::Face(exp.Current());
      Handle(GeomSurface) Surf = BRepInspector::Surface(face);

      Handle(GeomPlane) plane = Handle(GeomPlane)::DownCast(Surf);
      if (plane.IsNull())
      {
        Handle(Geom_RectangularTrimmedSurface) GRTS =
          Handle(Geom_RectangularTrimmedSurface)::DownCast(Surf);
        if (!GRTS.IsNull())
          plane = Handle(GeomPlane)::DownCast(GRTS->BasisSurface());
        if (plane.IsNull())
          continue;
      }

      Handle(GeomAdaptor_Surface) AS = new GeomAdaptor_Surface(plane);
      for (ShapeExplorer ed(face, TopAbs_EDGE); ed.More(); ed.Next())
      {
        TopoEdge        edge = TopoDS::Edge(ed.Current());
        Standard_Real      f, l;
        Handle(GeomCurve3d) crv = BRepInspector::Curve(edge, f, l);
        if (crv.IsNull())
          continue;

        Handle(GeomCurve2d) c2d = BRepInspector::CurveOnSurface(edge, face, f, l);
        if (c2d.IsNull())
          continue;
        Handle(Geom2dAdaptor_Curve) GHPC = new Geom2dAdaptor_Curve(c2d, f, l);
        Adaptor3d_CurveOnSurface    ACS(GHPC, AS);

        Standard_Real tol0               = BRepInspector::Tolerance(edge);
        tol                              = tol0;
        Standard_Real          tol2      = tol * tol;
        Standard_Boolean       isChanged = Standard_False;
        const Standard_Integer NCONTROL  = 23;
        for (Standard_Integer i = 0; i < NCONTROL; i++)
        {
          Standard_Real par  = (f * (NCONTROL - 1 - i) + l * i) / (NCONTROL - 1);
          Point3d        pnt  = crv->Value(par);
          Point3d        prj  = ACS.Value(par);
          Standard_Real dist = pnt.SquareDistance(prj);
          if (tol2 < dist)
          {
            tol2      = dist;
            isChanged = Standard_True;
          }
        }
        if (isChanged)
        {
          tol = 1.00005 * sqrt(tol2); // coeff: see trj3_pm1-ct-203.stp #19681, edge 10
          if (tol >= tol0)
          {
            B.UpdateEdge(edge, tol);
            for (TopoDS_Iterator itV(edge); itV.More(); itV.Next())
            {
              TopoShape S = itV.Value();
              B.UpdateVertex(TopoDS::Vertex(S), tol);
            }
          }
        }
      }
    }
    // Halt algorithm in case of user's abort
    if (!aPS.More())
      return Standard_False;
  }

  if (!status)
  {
#ifdef OCCT_DEBUG
    std::cout << "** SameParameter not complete. On " << numedge << " Edges:";
    if (nbfail > 0)
      std::cout << "  " << nbfail << " Failed";
    std::cout << std::endl;
#endif
  }
  return status;
}

//=================================================================================================

void ShapeFix1::EncodeRegularity(const TopoShape& shape, const Standard_Real tolang)
{
  BRepLib1::EncodeRegularity(shape, tolang);
}

//=================================================================================================

TopoShape ShapeFix1::RemoveSmallEdges(TopoShape&               Shape,
                                        const Standard_Real         Tolerance,
                                        Handle(ShapeBuild_ReShape)& context)
{
  Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape;
  sfs->Init(Shape);
  sfs->SetPrecision(Tolerance);
  sfs->FixFaceTool()->FixMissingSeamMode()   = Standard_False;
  sfs->FixFaceTool()->FixOrientationMode()   = Standard_False;
  sfs->FixFaceTool()->FixSmallAreaWireMode() = Standard_False;
  sfs->FixWireTool()->ModifyTopologyMode()   = Standard_True;
  // sfs.FixWireTool().FixReorderMode() = Standard_False;
  sfs->FixWireTool()->FixConnectedMode()        = Standard_False;
  sfs->FixWireTool()->FixEdgeCurvesMode()       = Standard_False;
  sfs->FixWireTool()->FixDegeneratedMode()      = Standard_False;
  sfs->FixWireTool()->FixSelfIntersectionMode() = Standard_False;
  sfs->FixWireTool()->FixLackingMode()          = Standard_False;
  sfs->FixWireTool()->FixSmallMode()            = Standard_True;
  sfs->Perform();
  TopoShape result = sfs->Shape();
  context             = sfs->Context();
  return result;
}

//=======================================================================
// function : ReplaceVertex
// purpose  : auxiliary for FixVertexPosition
//=======================================================================
static TopoEdge ReplaceVertex(const TopoEdge&     theEdge,
                                 const Point3d&          theP,
                                 const Standard_Boolean theFwd)
{
  TopoVertex aNewVertex;
  ShapeBuilder  aB;
  aB.MakeVertex(aNewVertex, theP, Precision1::Confusion());
  TopoVertex aV1, aV2;
  if (theFwd)
  {
    aV1 = aNewVertex;
    aV1.Orientation(TopAbs_FORWARD);
  }
  else
  {
    aV2 = aNewVertex;
    aV2.Orientation(TopAbs_REVERSED);
  }
  Edge2    aSbe;
  TopoEdge        e1  = theEdge;
  TopAbs_Orientation Ori = e1.Orientation();
  e1.Orientation(TopAbs_FORWARD);
  TopoEdge aNewEdge = aSbe.CopyReplaceVertices(e1, aV1, aV2);
  aNewEdge.Orientation(Ori);
  return aNewEdge;
}

//=======================================================================
// function : getNearPoint
// purpose  : auxiliary for FixVertexPosition
//=======================================================================
static Standard_Real getNearPoint(const TColgp_SequenceOfPnt& aSeq1,
                                  const TColgp_SequenceOfPnt& aSeq2,
                                  Coords3d&                     acent)
{
  Standard_Integer i    = 1;
  Standard_Integer ind1 = 0, ind2 = 0;
  Standard_Real    mindist = RealLast();
  for (; i <= aSeq1.Length(); i++)
  {
    Point3d           p1 = aSeq1.Value(i);
    Standard_Integer j  = 1;
    for (; j <= aSeq2.Length(); j++)
    {
      Point3d        p2 = aSeq2.Value(j);
      Standard_Real d  = p1.Distance(p2);
      if (fabs(d - mindist) <= Precision1::Confusion())
        continue;
      if (d < mindist)
      {
        mindist = d;
        ind1    = i;
        ind2    = j;
      }
    }
  }
  if (ind1 && ind2)
    acent = (aSeq1.Value(ind1).XYZ() + aSeq2.Value(ind2).XYZ()) / 2.0;
  return mindist;
}

//=======================================================================
// function : getNearestEdges
// purpose  : auxiliary for FixVertexPosition
//=======================================================================
static Standard_Boolean getNearestEdges(ShapeList&     theLEdges,
                                        const TopoVertex&      theVert,
                                        TopTools_SequenceOfShape& theSuitEdges,
                                        TopTools_SequenceOfShape& theRejectEdges,
                                        const Standard_Real       theTolerance,
                                        Coords3d&                   thecentersuit,
                                        Coords3d&                   thecenterreject)
{
  if (theLEdges.IsEmpty())
    return Standard_False;
  TopTools_MapOfShape aMapEdges;

  ShapeList atempList;
  atempList = theLEdges;
  TopTools_ListIteratorOfListOfShape alIter(atempList);

  TopoEdge   aEdge1 = TopoDS::Edge(alIter.Value());
  TopoVertex aVert11, aVert12;
  TopExp1::Vertices(aEdge1, aVert11, aVert12);
  aMapEdges.Add(aEdge1);
  Standard_Real      aFirst1, aLast1;
  Handle(GeomCurve3d) aCurve1 = BRepInspector::Curve(aEdge1, aFirst1, aLast1);
  Point3d             p11;
  Point3d             p12;
  Standard_Boolean   isFirst1 = theVert.IsSame(aVert11);
  Standard_Boolean   isSame1  = aVert11.IsSame(aVert12);
  if (!aCurve1.IsNull())
  {
    if (isFirst1)
      p11 = aCurve1->Value(aFirst1);
    else if (!isSame1)
      p11 = aCurve1->Value(aLast1);
    if (isSame1)
      p12 = aCurve1->Value(aLast1);
  }
  else
    return Standard_False;
  alIter.Next();
  TopTools_SequenceOfShape aseqreject;
  TopTools_SequenceOfShape aseqsuit;

  Standard_Integer anumLoop = 0;
  for (; alIter.More();)
  {
    TopoEdge aEdge = TopoDS::Edge(alIter.Value());
    if (aMapEdges.Contains(aEdge))
    {
      atempList.Remove(alIter);
      continue;
    }

    TopoVertex aVert1, aVert2;
    TopExp1::Vertices(aEdge, aVert1, aVert2);
    Standard_Real    isFirst = theVert.IsSame(aVert1);
    Standard_Boolean isSame  = aVert1.IsSame(aVert2);

    Standard_Boolean isLoop = ((aVert1.IsSame(aVert11) && aVert2.IsSame(aVert12))
                               || (aVert1.IsSame(aVert12) && aVert2.IsSame(aVert11)));
    if (isLoop /*&& !aseqsuit.Length()*/ && (atempList.Extent() > anumLoop))
    {
      atempList.Append(aEdge);
      atempList.Remove(alIter);
      anumLoop++;
      continue;
    }
    aMapEdges.Add(aEdge);
    Standard_Real      aFirst, aLast;
    Handle(GeomCurve3d) aCurve = BRepInspector::Curve(aEdge, aFirst, aLast);
    if (!aCurve.IsNull())
    {
      Point3d p1;
      Point3d p2;
      if (isFirst)
        p1 = aCurve->Value(aFirst);
      else
        p1 = aCurve->Value(aLast);
      if (isSame)
        p2 = aCurve->Value(aLast);
      Standard_Real aMinDist = RealLast();
      Coords3d        acent;
      if (!isSame && !isSame1)
      {
        aMinDist = p1.Distance(p11);
        acent    = (p1.XYZ() + p11.XYZ()) / 2.0;
      }
      else
      {
        TColgp_SequenceOfPnt aSeq1;
        TColgp_SequenceOfPnt aSeq2;
        aSeq1.Append(p11);
        if (isSame1)
          aSeq1.Append(p12);
        aSeq2.Append(p1);
        if (isSame)
          aSeq2.Append(p2);
        aMinDist = getNearPoint(aSeq1, aSeq2, acent);
      }

      if (aMinDist > theTolerance)
      {
        if (!aseqreject.Length())
          thecenterreject = acent;
        aseqreject.Append(aEdge);
      }
      else
      {
        if (!aseqsuit.Length())
        {
          thecentersuit = acent;
          aseqsuit.Append(aEdge);
        }
        else if (!isSame1)
          aseqsuit.Append(aEdge);
        else if ((thecentersuit - acent).Modulus() < theTolerance)
          aseqsuit.Append(aEdge);
        else
          aseqreject.Append(aEdge);
      }
    }
    atempList.Remove(alIter);
  }

  Standard_Boolean isDone = (!aseqsuit.IsEmpty() || !aseqreject.IsEmpty());
  if (isDone)
  {
    if (aseqsuit.IsEmpty())
    {
      theRejectEdges.Append(aEdge1);
      theLEdges.RemoveFirst();

      getNearestEdges(theLEdges,
                      theVert,
                      theSuitEdges,
                      theRejectEdges,
                      theTolerance,
                      thecentersuit,
                      thecenterreject);
    }
    else
    {
      theSuitEdges.Append(aEdge1);
      theSuitEdges.Append(aseqsuit);
      theRejectEdges.Append(aseqreject);
    }
  }
  else
    theRejectEdges.Append(aEdge1);

  return isDone;
}

//=================================================================================================

Standard_Boolean ShapeFix1::FixVertexPosition(TopoShape&                     theshape,
                                             const Standard_Real               theTolerance,
                                             const Handle(ShapeBuild_ReShape)& thecontext)
{
  TopTools_IndexedDataMapOfShapeListOfShape aMapVertEdges;
  ShapeExplorer                           aExp1(theshape, TopAbs_EDGE);
  for (; aExp1.More(); aExp1.Next())
  {
    TopoShape     aVert1;
    Standard_Integer nV = 1;
    TopoDS_Iterator  aExp3(aExp1.Current());
    for (; aExp3.More(); aExp3.Next(), nV++)
    {
      const TopoShape& aVert = aExp3.Value();
      if (nV == 1)
        aVert1 = aVert;
      else if (aVert1.IsSame(aVert))
        continue;
      if (aMapVertEdges.Contains(aVert))
        aMapVertEdges.ChangeFromKey(aVert).Append(aExp1.Current());
      else
      {
        ShapeList alEdges;
        alEdges.Append(aExp1.Current());
        aMapVertEdges.Add(aVert, alEdges);
      }
    }
  }
  Standard_Boolean isDone = Standard_False;
  Standard_Integer i      = 1;
  for (; i <= aMapVertEdges.Extent(); i++)
  {
    TopoVertex aVert    = TopoDS::Vertex(aMapVertEdges.FindKey(i));
    Standard_Real aTolVert = BRepInspector::Tolerance(aVert);
    if (aTolVert <= theTolerance)
      continue;

    ShapeBuilder aB1;
    aB1.UpdateVertex(aVert, theTolerance);
    Point3d aPvert = BRepInspector::Pnt(aVert);
    Coords3d acenter(aPvert.XYZ()), acenterreject(aPvert.XYZ());

    TopTools_SequenceOfShape aSuitEdges;
    TopTools_SequenceOfShape aRejectEdges;
    ShapeList     aledges;
    aledges = aMapVertEdges.FindFromIndex(i);
    if (aledges.Extent() == 1)
      continue;
    // if tolerance of vertex is more than specified tolerance
    //  check distance between curves and vertex

    if (!getNearestEdges(aledges,
                         aVert,
                         aSuitEdges,
                         aRejectEdges,
                         theTolerance,
                         acenter,
                         acenterreject))
      continue;

    // update vertex by nearest point
    Standard_Boolean isAdd = Standard_False;
    Standard_Integer k     = 1;
    for (; k <= aSuitEdges.Length(); k++)
    {

      TopoEdge   aEdgeOld = TopoDS::Edge(aSuitEdges.Value(k));
      TopoVertex aVert1, aVert2;
      TopExp1::Vertices(aEdgeOld, aVert1, aVert2);

      Standard_Boolean isFirst = (aVert1.IsSame(aVert));
      Standard_Boolean isLast  = (aVert2.IsSame(aVert));
      if (!isFirst && !isLast)
        continue;
      Standard_Real      aFirst, aLast;
      Handle(GeomCurve3d) aCurve;
      TopoEdge        aEdge = TopoDS::Edge(thecontext->Apply(aEdgeOld));

      TopoVertex aVert1n, aVert2n;
      TopExp1::Vertices(aEdge, aVert1n, aVert2n);
      aCurve = BRepInspector::Curve(aEdge, aFirst, aLast);
      if (!aCurve.IsNull())
      {
        Point3d p1 = aCurve->Value(aFirst);
        Point3d p2 = aCurve->Value(aLast);

        // if distance between ends of curve more than specified tolerance
        // but vertices are the same that one of the vertex will be replaced.

        Standard_Boolean isReplace = (aVert1n.IsSame(aVert2n) && p1.Distance(p2) > theTolerance);

        // Standard_Real dd1 = (acenter - p1.XYZ()).Modulus();
        // Standard_Real dd2 = (acenter - p2.XYZ()).Modulus();
        if (isFirst)
        {
          if (k > 2)
          {
            acenter += p1.XYZ();
            acenter /= 2.0;
          }
          if (isReplace)
          {
            TopoEdge enew;
            if (p1.Distance(acenter) < p2.Distance(acenter))
              enew = ReplaceVertex(aEdge, p2, Standard_False);
            else
              enew = ReplaceVertex(aEdge, p1, Standard_True);
            thecontext->Replace(aEdge, enew);
            isDone = Standard_True;
          }
        }
        else
        {
          if (k > 2)
          {
            acenter += p2.XYZ();
            acenter /= 2.0;
          }
          if (isReplace)
          {
            TopoEdge enew;
            if (p1.Distance(acenter) < p2.Distance(acenter))
              enew = ReplaceVertex(aEdge, p2, Standard_False);
            else
              enew = ReplaceVertex(aEdge, p1, Standard_True);
            thecontext->Replace(aEdge, enew);
            isDone = Standard_True;
          }
        }

        isAdd = Standard_True;
      }
    }

    if (isAdd && aPvert.Distance(acenter) > theTolerance)
    {

      ShapeBuilder aB;

      //  aB.UpdateVertex(aVert,Precision1::Confusion());
      // else {
      isDone = Standard_True;
      TopoVertex aNewVertex;
      aB.MakeVertex(aNewVertex, acenter, Precision1::Confusion());
      aNewVertex.Orientation(aVert.Orientation());
      thecontext->Replace(aVert, aNewVertex);
    }

    for (k = 1; k <= aRejectEdges.Length(); k++)
    {
      TopoEdge   aEdgeOld = TopoDS::Edge(aRejectEdges.Value(k));
      TopoVertex aVert1, aVert2;
      TopExp1::Vertices(aEdgeOld, aVert1, aVert2);

      Standard_Boolean isFirst = (aVert1.IsSame(aVert));
      Standard_Boolean isLast  = (aVert2.IsSame(aVert));
      if (!isFirst && !isLast)
        continue;
      Standard_Boolean   isSame = aVert1.IsSame(aVert2);
      Handle(GeomCurve3d) aCurve;
      TopoEdge        aEdge = TopoDS::Edge(thecontext->Apply(aEdgeOld));

      TopoVertex aVert1n, aVert2n;
      TopExp1::Vertices(aEdge, aVert1n, aVert2n);

      Standard_Real aFirst, aLast;
      aCurve = BRepInspector::Curve(aEdge, aFirst, aLast);
      if (!aCurve.IsNull())
      {
        Point3d      p1 = aCurve->Value(aFirst);
        Point3d      p2 = aCurve->Value(aLast);
        TopoEdge enew;
        if (isFirst)
        {
          enew = ReplaceVertex(aEdge, p1, Standard_True);
          if (isSame)
            enew = ReplaceVertex(enew, p2, Standard_False);
        }
        else
        {
          enew = ReplaceVertex(aEdge, p2, Standard_False);
          if (isSame)
            enew = ReplaceVertex(enew, p1, Standard_True);
        }

        thecontext->Replace(aEdge, enew);
        isDone = Standard_True;
      }
    }
  }
  if (isDone)
    theshape = thecontext->Apply(theshape);
  return isDone;
}

//=================================================================================================

Standard_Real ShapeFix1::LeastEdgeSize(TopoShape& theShape)
{
  Standard_Real aRes = RealLast();
  for (ShapeExplorer exp(theShape, TopAbs_EDGE); exp.More(); exp.Next())
  {
    TopoEdge        edge = TopoDS::Edge(exp.Current());
    Standard_Real      first, last;
    Handle(GeomCurve3d) c3d = BRepInspector::Curve(edge, first, last);
    if (!c3d.IsNull())
    {
      Box2 bb;
      bb.Add(c3d->Value(first));
      bb.Add(c3d->Value(last));
      bb.Add(c3d->Value((last + first) / 2.));
      Standard_Real x1, x2, y1, y2, z1, z2, size;
      bb.Get(x1, y1, z1, x2, y2, z2);
      size = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1);
      if (size < aRes)
        aRes = size;
    }
  }
  aRes = sqrt(aRes);
  return aRes;
}
