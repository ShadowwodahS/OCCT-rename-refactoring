// Created on: 2015-04-24
// Created by: NIKOLAI BUKHALOV
// Copyright (c) 2015 OPEN CASCADE SAS
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

#include <BRepBuilderAPI_FastSewing.hxx>

#include <BRepTools_Quilt.hxx>
#include <Bnd_Box.hxx>

#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>

#include <Precision.hxx>

#include <Standard_NullObject.hxx>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_MapOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepBuilderAPI_FastSewing, RefObject)

//=======================================================================
// function : IntersetctionOfSets
// purpose  : Returns minimal value of intersection result
//=======================================================================
static Standard_Integer IntersectionOfSets(const NCollection_List<Standard_Integer>& theSet1,
                                           const NCollection_List<Standard_Integer>& theSet2)
{
  constexpr Standard_Integer anIntMax = IntegerLast();
  Standard_Integer           aRetVal  = anIntMax;
  for (NCollection_List<Standard_Integer>::Iterator anIt1 = theSet1.begin().Iterator();
       anIt1.More();
       anIt1.Next())
  {
    const Standard_Integer aVal1 = anIt1.Value();
    for (NCollection_List<Standard_Integer>::Iterator anIt2 = theSet2.begin().Iterator();
         anIt2.More();
         anIt2.Next())
    {
      const Standard_Integer aVal2 = anIt2.Value();
      if (aVal1 == aVal2)
      {
        // theIntersectionResult.Append(aVal1);
        if (aVal1 < aRetVal)
          aRetVal = aVal1;
      }
    }
  }

  if (aRetVal == anIntMax)
    return -1;

  return aRetVal;
}

//=================================================================================================

static Handle(GeomCurve2d) Get2DCurve(const Standard_Integer theIndex,
                                       const Standard_Real    theUfirst,
                                       const Standard_Real    theUlast,
                                       const Standard_Real    theVfirst,
                                       const Standard_Real    theVlast,
                                       const Standard_Boolean theIsReverse = Standard_False)
{
  if ((theIndex < 0) || (theIndex > 3))
    throw Standard_OutOfRange("BRepBuilderAPI_FastSewing.cxx, Get2DCurve(): OUT of Range");

  Handle(GeomCurve2d) a2dCurv;

  if (!theIsReverse)
  {
    switch (theIndex)
    {
      case 0:
        a2dCurv =
          new Geom2d_TrimmedCurve(new Geom2d_Line(gp_Pnt2d(0.0, theVfirst), gp_Dir2d(1.0, 0.0)),
                                  theUfirst,
                                  theUlast);
        break;
      case 1:
        a2dCurv =
          new Geom2d_TrimmedCurve(new Geom2d_Line(gp_Pnt2d(theUlast, 0.0), gp_Dir2d(0.0, 1.0)),
                                  theVfirst,
                                  theVlast);
        break;
      case 2:
        a2dCurv =
          new Geom2d_TrimmedCurve(new Geom2d_Line(gp_Pnt2d(0.0, theVlast), gp_Dir2d(1.0, 0.0)),
                                  theUfirst,
                                  theUlast);
        break;
      case 3:
        a2dCurv =
          new Geom2d_TrimmedCurve(new Geom2d_Line(gp_Pnt2d(theUfirst, 0.0), gp_Dir2d(0.0, 1.0)),
                                  theVfirst,
                                  theVlast);
        break;
      default:
        break;
    }
  }
  else
  {
    switch (theIndex)
    {
      case 0:
        a2dCurv = new Geom2d_TrimmedCurve(
          new Geom2d_Line(gp_Pnt2d(theUfirst + theUlast, theVfirst), gp_Dir2d(-1.0, 0.0)),
          theUfirst,
          theUlast);
        break;
      case 1:
        a2dCurv = new Geom2d_TrimmedCurve(
          new Geom2d_Line(gp_Pnt2d(theUlast, theVfirst + theVlast), gp_Dir2d(0.0, -1.0)),
          theVfirst,
          theVlast);
        break;
      case 2:
        a2dCurv = new Geom2d_TrimmedCurve(
          new Geom2d_Line(gp_Pnt2d(theUfirst + theUlast, theVlast), gp_Dir2d(-1.0, 0.0)),
          theUfirst,
          theUlast);
        break;
      case 3:
        a2dCurv = new Geom2d_TrimmedCurve(
          new Geom2d_Line(gp_Pnt2d(theUfirst, theVfirst + theVlast), gp_Dir2d(0.0, -1.0)),
          theVfirst,
          theVlast);
        break;
      default:
        break;
    }
  }

  return a2dCurv;
}

//=================================================================================================

BRepBuilderAPI_FastSewing::BRepBuilderAPI_FastSewing(const Standard_Real theTol)
    : myTolerance(theTol),
      myStatusList(0)
{
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_FastSewing::Add(const TopoShape& theShape)
{
  Standard_Boolean aResult = Standard_False;
  if (theShape.IsNull())
  {
    SetStatus(FS_EmptyInput);
    return aResult;
  }

  TopTools_MapOfShape aMS;
  // aMS.Add(theShape);
  ShapeExplorer aFExp(theShape, TopAbs_FACE);
  for (; aFExp.More(); aFExp.Next())
  {
    const TopoFace& aFace = TopoDS::Face(aFExp.Current());
    if (aMS.Add(aFace))
    {
      Handle(GeomSurface) aSurf = BRepInspector::Surface(aFace);
      if (aSurf.IsNull())
      {
        SetStatus(FS_FaceWithNullSurface);
        continue;
      }

      if (aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
      {
        SetStatus(FS_NotNaturalBoundsFace);
        continue;
      }

      Standard_Real aUf = 0.0, aUl = 0.0, aVf = 0.0, aVl = 0.0;
      aSurf->Bounds(aUf, aUl, aVf, aVl);

      if (Precision::IsInfinite(aUf) || Precision::IsInfinite(aUl) || Precision::IsInfinite(aVf)
          || Precision::IsInfinite(aVl))
      {
        SetStatus(FS_InfiniteSurface);
        continue;
      }

      FS_Face aFFace;
      aFFace.mySrcFace = aFace;
      aFFace.myID      = myFaceVec.Length(); // because start index is 0
      myFaceVec.Append(aFFace);
      aResult = Standard_True;
    }
  }

  return aResult;
}

//=================================================================================================

Standard_Boolean BRepBuilderAPI_FastSewing::Add(const Handle(GeomSurface)& theSurface)
{
  if (theSurface.IsNull())
  {
    SetStatus(FS_FaceWithNullSurface);
    return Standard_False;
  }

  if (theSurface->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
  {
    SetStatus(FS_NotNaturalBoundsFace);
    return Standard_False;
  }

  Standard_Real aUf = 0.0, aUl = 0.0, aVf = 0.0, aVl = 0.0;
  theSurface->Bounds(aUf, aUl, aVf, aVl);

  if (Precision::IsInfinite(aUf) || Precision::IsInfinite(aUl) || Precision::IsInfinite(aVf)
      || Precision::IsInfinite(aVl))
  {
    SetStatus(FS_InfiniteSurface);
    return Standard_False;
  }

  FS_Face aFace;

  ShapeBuilder aBuilder;
  aBuilder.MakeFace(aFace.mySrcFace);
  aBuilder.MakeFace(aFace.mySrcFace, theSurface, myTolerance);
  aBuilder.NaturalRestriction(aFace.mySrcFace, Standard_True);

  aFace.myID = myFaceVec.Length(); // because start index is 0
  myFaceVec.Append(aFace);

  return Standard_True;
}

//=================================================================================================

void BRepBuilderAPI_FastSewing::Perform(void)
{
  if (myFaceVec.IsEmpty())
  {
    SetStatus(FS_EmptyInput);
    return;
  }

  try
  {
    {
      // create vertices having unique coordinates
      Standard_Real                         aRange  = Compute3DRange();
      Handle(NCollection_IncAllocator)      anAlloc = new NCollection_IncAllocator;
      NCollection_CellFilter<NodeInspector> aCells(Max(myTolerance, aRange / IntegerLast()),
                                                   anAlloc);

      for (Standard_Integer i = myFaceVec.Lower(); i <= myFaceVec.Upper(); i++)
      {
        FindVertexes(i, aCells);
      }
    }

    for (Standard_Integer i = myFaceVec.Lower(); i <= myFaceVec.Upper(); i++)
    {
      FindEdges(i);
    }

    // Create topological structures

    for (Standard_Integer i = myVertexVec.Lower(); i <= myVertexVec.Upper(); i++)
    {
      myVertexVec.ChangeValue(i).CreateTopologicalVertex(myTolerance);
    }

    // Edges
    for (Standard_Integer i = myEdgeVec.Lower(); i <= myEdgeVec.Upper(); i++)
    {
      myEdgeVec.ChangeValue(i).CreateTopologicalEdge(myVertexVec, myFaceVec, myTolerance);
    }

    // Shell
    ShapeQuilt aQuilt;

    // Faces
    for (Standard_Integer i = myFaceVec.Lower(); i <= myFaceVec.Upper(); i++)
    {
      FS_Face& aFace = myFaceVec.ChangeValue(i);
      aFace.CreateTopologicalWire(myEdgeVec, myTolerance);
      aFace.CreateTopologicalFace();
      aQuilt.Add(aFace.myRetFace);
    }

    myResShape = aQuilt.Shells();
  }
  catch (ExceptionBase const&)
  {
    SetStatus(FS_Exception);
#ifdef OCCT_DEBUG
    // ExceptionBase::Caught()->Print(std::cout);
#endif
    return;
  }
}

//=================================================================================================

void BRepBuilderAPI_FastSewing::UpdateEdgeInfo(const Standard_Integer theIDPrevVertex,
                                               const Standard_Integer theIDCurrVertex,
                                               const Standard_Integer theFaceID,
                                               const Standard_Integer theIDCurvOnFace)
{
  // Indeed, two vertices combine into one edge only.
  const Standard_Integer anEdgeID = IntersectionOfSets(myVertexVec.Value(theIDPrevVertex).myEdges,
                                                       myVertexVec.Value(theIDCurrVertex).myEdges);

  // For DEBUG mode only
  Standard_ProgramError_Raise_if(
    anEdgeID < 0,
    "BRepBuilderAPI_FastSewing::UpdateEdgeInfo: Update not existing edge.");

  FS_Edge& anEdge = myEdgeVec.ChangeValue(anEdgeID);
  anEdge.myFaces.Append(theFaceID);
  FS_Face& aFace = myFaceVec.ChangeValue(theFaceID);
  aFace.SetEdge(theIDCurvOnFace, anEdge.myID);
}

//=======================================================================
// function : CreateNewEdge
// purpose  : Creates FS_Edge
//=======================================================================
void BRepBuilderAPI_FastSewing::CreateNewEdge(const Standard_Integer theIDPrevVertex,
                                              const Standard_Integer theIDCurrVertex,
                                              const Standard_Integer theFaceID,
                                              const Standard_Integer theIDCurvOnFace)
{
  FS_Edge anEdge(theIDPrevVertex, theIDCurrVertex);
  anEdge.myID = myEdgeVec.Length(); // because start index is 0

  anEdge.myFaces.Append(theFaceID);
  FS_Face& aFace = myFaceVec.ChangeValue(theFaceID);
  aFace.SetEdge(theIDCurvOnFace, anEdge.myID);

  myVertexVec.ChangeValue(theIDPrevVertex).myEdges.Append(anEdge.myID);

  if (theIDPrevVertex == theIDCurrVertex)
  { // the Edge is degenerated
    SetStatus(FS_Degenerated);
  }
  else
  {
    myVertexVec.ChangeValue(theIDCurrVertex).myEdges.Append(anEdge.myID);
  }

  myEdgeVec.Append(anEdge);
}

//=================================================================================================

void BRepBuilderAPI_FastSewing::FindVertexes(const Standard_Integer                 theSurfID,
                                             NCollection_CellFilter<NodeInspector>& theCells)
{
  const Standard_Integer     aNbPoints = 4;
  FS_Face&                   aFace     = myFaceVec.ChangeValue(theSurfID);
  const Handle(GeomSurface) aSurf     = BRepInspector::Surface(aFace.mySrcFace);
  Standard_Real              aUf = 0.0, aUl = 0.0, aVf = 0.0, aVl = 0.0;
  aSurf->Bounds(aUf, aUl, aVf, aVl);

  const Point3d aPnts[aNbPoints] = {aSurf->Value(aUf, aVf),
                                   aSurf->Value(aUl, aVf),
                                   aSurf->Value(aUl, aVl),
                                   aSurf->Value(aUf, aVl)};

  for (Standard_Integer i = 0; i < aNbPoints; i++)
  {
    FS_Vertex aVert;

    NodeInspector anInspector(myVertexVec, aPnts[i], myTolerance);
    Bnd_Box       aBox;
    aBox.Add(aPnts[i]);
    aBox.Enlarge(myTolerance);

    theCells.Inspect(aBox.CornerMin().XYZ(), aBox.CornerMax().XYZ(), anInspector);
    NodeInspector::Target aResID = anInspector.GetResult();

    if (aResID < 0)
    {                                     // Add new Vertex
      aVert.myID  = myVertexVec.Length(); // because start index is 0
      aVert.myPnt = aPnts[i];
      aVert.myFaces.Append(theSurfID);
      myVertexVec.Append(aVert);
      aFace.SetVertex(i, aVert.myID);

      theCells.Add(aVert.myID, aBox.CornerMin().XYZ(), aBox.CornerMax().XYZ());
    }
    else
    { // Change existing vertex
      aFace.SetVertex(i, aResID);
      myVertexVec.ChangeValue(aResID).myFaces.Append(theSurfID);
    }
  }
}

//=================================================================================================

void BRepBuilderAPI_FastSewing::FindEdges(const Standard_Integer theSurfID)
{
  const Standard_Integer aNbPoints = 4;
  FS_Face&               aFace     = myFaceVec.ChangeValue(theSurfID);

  const Standard_Integer aFirstInd[aNbPoints] = {0, 1, 3, 0};
  const Standard_Integer aLastInd[aNbPoints]  = {1, 2, 2, 3};

  for (Standard_Integer i = 0; i < aNbPoints; i++)
  {
    const Standard_Integer aFirstVertIndex = aFirstInd[i], aLastVertIndex = aLastInd[i];
    const Standard_Integer aFirstVertID = aFace.myVertices[aFirstVertIndex],
                           aLastVertID  = aFace.myVertices[aLastVertIndex];

    if (aFirstVertID == aLastVertID)
    { // Edge is degenerated.
      CreateNewEdge(aFirstVertID, aLastVertID, theSurfID, i);
      continue;
    }

    // Must be minimal element from list
    const Standard_Integer anIntRes = IntersectionOfSets(myVertexVec.Value(aFirstVertID).myFaces,
                                                         myVertexVec.Value(aLastVertID).myFaces);

    if ((anIntRes < 0) || (anIntRes >= theSurfID))
    {
      CreateNewEdge(aFirstVertID, aLastVertID, theSurfID, i);
    }
    else
    { // if(theSurfID > anIntRes) => The edge has been processed earlier
      UpdateEdgeInfo(aFirstVertID, aLastVertID, theSurfID, i);
    }
  }
}

//=================================================================================================

BRepBuilderAPI_FastSewing::FS_VARStatuses BRepBuilderAPI_FastSewing::GetStatuses(
  Standard_OStream* const theOS)
{
  if (!theOS)
    return myStatusList;

  if (!myStatusList)
  {
    *theOS << "Fast Sewing OK!\n";
    return myStatusList;
  }

  // Number of bits
  const Standard_Integer aNumMax = 8 * sizeof(myStatusList);
  FS_Statuses            anIDS   = static_cast<FS_Statuses>(0x0001);
  for (Standard_Integer i = 1; i <= aNumMax; i++, anIDS = static_cast<FS_Statuses>(anIDS << 1))
  {
    if ((anIDS & myStatusList) == 0)
      continue;

    switch (anIDS)
    {
      case FS_Degenerated:
        *theOS << "Degenerated case. Try to reduce tolerance.\n";
        break;
      case FS_FindVertexError:
        *theOS << "Error while creating list of vertices.\n";
        break;
      case FS_FindEdgeError:
        *theOS << "Error while creating list of edges.\n";
        break;
      case FS_Exception:
        *theOS << "Exception during the operation.\n";
        break;
      case FS_FaceWithNullSurface:
        *theOS << "Source face has null surface.\n";
        break;
      case FS_NotNaturalBoundsFace:
        *theOS << "Source face has trimmed surface.\n";
        break;
      case FS_InfiniteSurface:
        *theOS << "Source face has the surface with infinite boundaries.\n";
        break;
      case FS_EmptyInput:
        *theOS << "Empty source data.\n";
        break;

      default:
        return myStatusList;
    }
  }

  return myStatusList;
}

//=================================================================================================

Standard_Real BRepBuilderAPI_FastSewing::Compute3DRange()
{
  Bnd_Box aBox;

  for (Standard_Integer i = myFaceVec.Lower(); i <= myFaceVec.Upper(); i++)
  {
    FS_Face&                   aFace = myFaceVec.ChangeValue(i);
    const Handle(GeomSurface) aSurf = BRepInspector::Surface(aFace.mySrcFace);
    if (aSurf.IsNull())
      continue;
    Standard_Real aUf = 0.0, aUl = 0.0, aVf = 0.0, aVl = 0.0;
    aSurf->Bounds(aUf, aUl, aVf, aVl);

    aBox.Add(aSurf->Value(aUf, aVf));
    aBox.Add(aSurf->Value(aUl, aVf));
    aBox.Add(aSurf->Value(aUl, aVl));
    aBox.Add(aSurf->Value(aUf, aVl));
  }

  Standard_Real aXm = 0.0, aYm = 0.0, aZm = 0.0, aXM = 0.0, aYM = 0.0, aZM = 0.0;
  aBox.Get(aXm, aYm, aZm, aXM, aYM, aZM);
  Standard_Real aDelta = aXM - aXm;
  aDelta               = Max(aDelta, aYM - aYm);
  aDelta               = Max(aDelta, aZM - aZm);

  return aDelta;
}

//=================================================================================================

BRepBuilderAPI_FastSewing::NodeInspector::NodeInspector(const NCollection_Vector<FS_Vertex>& theVec,
                                                        const Point3d&                        thePnt,
                                                        const Standard_Real                  theTol)
    : myVecOfVertexes(theVec),
      myPoint(thePnt),
      myResID(-1),
      myIsFindingEnable(Standard_False)
{
  mySQToler = theTol * theTol;
}

//=================================================================================================

NCollection_CellFilter_Action BRepBuilderAPI_FastSewing::NodeInspector::Inspect(const Target theID)
{
  const Point3d&       aPt     = myVecOfVertexes.Value(theID).myPnt;
  const Standard_Real aSQDist = aPt.SquareDistance(myPoint);
  if (aSQDist < mySQToler)
  {
    mySQToler = aSQDist;
    myResID   = theID;
  }

  return CellFilter_Keep;
}

//=================================================================================================

void BRepBuilderAPI_FastSewing::FS_Edge::CreateTopologicalEdge(
  const NCollection_Vector<FS_Vertex>& theVertexVec,
  const NCollection_Vector<FS_Face>&   theFaceVec,
  const Standard_Real                  theTol)
{
  ShapeBuilder aBuilder;

  TopoVertex aV1 = theVertexVec(myVertices[0]).myTopoVert;
  TopoVertex aV2 = theVertexVec(myVertices[1]).myTopoVert;

  aV1.Orientation(TopAbs_FORWARD);
  aV2.Orientation(TopAbs_REVERSED);

  Handle(GeomCurve3d) a3dCurv;
  TopLoc_Location    aLocation;

  const FS_Face& aFace = theFaceVec.Value(myFaces.Value(myFaces.Lower()));

  // 3D-curves in 1st and 2nd faces are considered to be in same-range
  const Handle(GeomSurface)& aSurf = BRepInspector::Surface(aFace.mySrcFace, aLocation);

  Standard_Real aUf = 0.0, aUl = 0.0, aVf = 0.0, aVl = 0.0;
  aSurf->Bounds(aUf, aUl, aVf, aVl);

  Standard_Integer anEdgeID = -1;
  for (Standard_Integer anInd = 0; anInd < 4; anInd++)
  {
    if (myID == aFace.myEdges[anInd])
    {
      anEdgeID = anInd;
      break;
    }
  }

  // For DEBUG mode only
  Standard_ProgramError_Raise_if(
    anEdgeID < 0,
    "BRepBuilderAPI_FastSewing::FS_Edge::CreateTopologicalEdge: Single edge.");

  if (IsDegenerated())
  {
    Handle(GeomCurve2d) a2dCurv = Get2DCurve(anEdgeID, aUf, aUl, aVf, aVl);
    const Standard_Real  aFPar = a2dCurv->FirstParameter(), aLPar = a2dCurv->LastParameter();

    aBuilder.MakeEdge(myTopoEdge);
    aBuilder.UpdateEdge(myTopoEdge, a2dCurv, aSurf, aLocation, theTol);
    aBuilder.Add(myTopoEdge, aV1);
    aBuilder.Add(myTopoEdge, aV2);
    aBuilder.Range(myTopoEdge, aFPar, aLPar);
    aBuilder.Degenerated(myTopoEdge, Standard_True);
    return;
  }

  switch (anEdgeID)
  {
    case 0:
      a3dCurv = aSurf->VIso(aVf);
      break;
    case 1:
      a3dCurv = aSurf->UIso(aUl);
      break;
    case 2:
      a3dCurv = aSurf->VIso(aVl);
      break;
    case 3:
      a3dCurv = aSurf->UIso(aUf);
      break;
    default:
      throw Standard_OutOfRange("FS_Edge::CreateTopologicalEdge()");
      break;
  }

  aBuilder.MakeEdge(myTopoEdge, a3dCurv, theTol);
  aBuilder.Add(myTopoEdge, aV1);
  aBuilder.Add(myTopoEdge, aV2);
  aBuilder.Range(myTopoEdge, a3dCurv->FirstParameter(), a3dCurv->LastParameter());
}

//=================================================================================================

void BRepBuilderAPI_FastSewing::FS_Face::CreateTopologicalWire(
  const NCollection_Vector<FS_Edge>& theEdgeVec,
  const Standard_Real                theToler)
{
  TopLoc_Location aLocation;
  // 3D-curves in 1st and 2nd faces are considered to be in same-range
  const Handle(GeomSurface)& aSurf = BRepInspector::Surface(mySrcFace, aLocation);
  Standard_Real               aUf = 0.0, aUl = 0.0, aVf = 0.0, aVl = 0.0;
  aSurf->Bounds(aUf, aUl, aVf, aVl);

  ShapeBuilder aB;
  aB.MakeWire(myWire);
  for (Standard_Integer anEdge = 0; anEdge < 4; anEdge++)
  {
    Standard_ProgramError_Raise_if(
      myEdges[anEdge] < 0,
      "BRepBuilderAPI_FastSewing::FS_Face::CreateTopologicalWire: Wire is not closed.");

    const BRepBuilderAPI_FastSewing::FS_Edge& aFSEdge = theEdgeVec.Value(myEdges[anEdge]);
    TopAbs_Orientation                        anOri = anEdge < 2 ? TopAbs_FORWARD : TopAbs_REVERSED;
    TopoEdge                               anTopE = aFSEdge.myTopoEdge;

    if (aFSEdge.IsDegenerated())
    {
      anTopE.Orientation(anOri);
      aB.Add(myWire, anTopE);
      continue;
    }

    // Check if 3D and 2D-curve have same-orientation.
    // If it is not, 2d-curve will be reversed.
    {
      Standard_Real aFirstPar = 0.0, aLastPar = 0.0;

      const Handle(GeomCurve3d) a3dCurv = BRepInspector::Curve(anTopE, aFirstPar, aLastPar);
      Handle(GeomCurve2d)     a2dCurv = Get2DCurve(anEdge, aUf, aUl, aVf, aVl);
      const Point3d             aPref(a3dCurv->Value(aFirstPar));
      const gp_Pnt2d           aP2df(a2dCurv->Value(aFirstPar)), aP2dl(a2dCurv->Value(aLastPar));
      Point3d                   aP3df(aSurf->Value(aP2df.X(), aP2df.Y()));
      Point3d                   aP3dl(aSurf->Value(aP2dl.X(), aP2dl.Y()));
      aP3df.Transform(aLocation);
      aP3dl.Transform(aLocation);
      const Standard_Real aSqD1 = aP3df.SquareDistance(aPref);
      const Standard_Real aSqD2 = aP3dl.SquareDistance(aPref);

      if (aSqD2 < aSqD1)
      {
        a2dCurv = Get2DCurve(anEdge, aUf, aUl, aVf, aVl, Standard_True);
        anOri   = TopAbs1::Reverse(anOri);
      }

      aB.UpdateEdge(anTopE, a2dCurv, aSurf, aLocation, theToler);
    }

    anTopE.Orientation(anOri);

    aB.Add(myWire, anTopE);
  }

  myWire.Closed(Standard_True);
}

//=================================================================================================

void BRepBuilderAPI_FastSewing::FS_Face::CreateTopologicalFace()
{
  Standard_ProgramError_Raise_if(
    myWire.IsNull(),
    "BRepBuilderAPI_FastSewing::FS_Face::CreateTopologicalFace: Cannot create wire.");

  ShapeBuilder aBuilder;
  myRetFace = TopoDS::Face(mySrcFace.EmptyCopied());
  aBuilder.Add(myRetFace, myWire);
  aBuilder.NaturalRestriction(myRetFace, Standard_True);
}
