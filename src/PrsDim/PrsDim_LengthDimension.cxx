// Created on: 1996-12-05
// Created by: Arnaud BOUZY/Odile Olivier
// Copyright (c) 1996-1999 Matra Datavision
// Copyright (c) 1999-2013 OPEN CASCADE SAS
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

#include <PrsDim_LengthDimension.hxx>

#include <PrsDim.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRepTools.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <gce_MakeDir.hxx>
#include <gce_MakePln.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ExtremaSurfaceSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_LengthDimension, PrsDim_Dimension)

//=================================================================================================

PrsDim_LengthDimension::PrsDim_LengthDimension()
    : PrsDim_Dimension(PrsDim_KOD_LENGTH),
      myHasCustomDirection(Standard_False)
{
  SetFlyout(15.0);
}

//=======================================================================
// function : Constructor
// purpose  : Dimension between two faces
//=======================================================================
PrsDim_LengthDimension::PrsDim_LengthDimension(const TopoFace& theFirstFace,
                                               const TopoFace& theSecondFace)
    : PrsDim_Dimension(PrsDim_KOD_LENGTH),
      myHasCustomDirection(Standard_False)
{
  SetMeasuredGeometry(theFirstFace, theSecondFace);
  SetFlyout(15.0);
}

//=======================================================================
// function : Constructor
// purpose  : Dimension between two shape
//=======================================================================
PrsDim_LengthDimension::PrsDim_LengthDimension(const TopoFace& theFace,
                                               const TopoEdge& theEdge)
    : PrsDim_Dimension(PrsDim_KOD_LENGTH),
      myHasCustomDirection(Standard_False)
{
  SetMeasuredGeometry(theFace, theEdge);
  SetFlyout(15.0);
}

//=======================================================================
// function : Constructor
// purpose  : Dimension between two points
//=======================================================================
PrsDim_LengthDimension::PrsDim_LengthDimension(const Point3d& theFirstPoint,
                                               const Point3d& theSecondPoint,
                                               const gp_Pln& thePlane)
    : PrsDim_Dimension(PrsDim_KOD_LENGTH),
      myHasCustomDirection(Standard_False)
{
  SetMeasuredGeometry(theFirstPoint, theSecondPoint, thePlane);
  SetFlyout(15.0);
}

//=======================================================================
// function : Constructor
// purpose  : Dimension between two shape
//=======================================================================
PrsDim_LengthDimension::PrsDim_LengthDimension(const TopoShape& theFirstShape,
                                               const TopoShape& theSecondShape,
                                               const gp_Pln&       thePlane)
    : PrsDim_Dimension(PrsDim_KOD_LENGTH),
      myHasCustomDirection(Standard_False)
{
  SetCustomPlane(thePlane);
  SetMeasuredShapes(theFirstShape, theSecondShape);
  SetFlyout(15.0);
}

//=======================================================================
// function : Constructor
// purpose  : Dimension of one edge
//=======================================================================
PrsDim_LengthDimension::PrsDim_LengthDimension(const TopoEdge& theEdge, const gp_Pln& thePlane)
    : PrsDim_Dimension(PrsDim_KOD_LENGTH),
      myHasCustomDirection(Standard_False)
{
  SetMeasuredGeometry(theEdge, thePlane);
  SetFlyout(15.0);
}

//=================================================================================================

void PrsDim_LengthDimension::SetMeasuredGeometry(const Point3d& theFirstPoint,
                                                 const Point3d& theSecondPoint,
                                                 const gp_Pln& thePlane)
{
  myFirstPoint   = theFirstPoint;
  mySecondPoint  = theSecondPoint;
  myFirstShape   = BRepLib_MakeVertex(myFirstPoint);
  mySecondShape  = BRepLib_MakeVertex(mySecondPoint);
  myGeometryType = GeometryType_Points;
  SetCustomPlane(thePlane);
  myIsGeometryValid = IsValidPoints(theFirstPoint, theSecondPoint);

  SetToUpdate();
}

//=================================================================================================

void PrsDim_LengthDimension::SetMeasuredGeometry(const TopoEdge& theEdge, const gp_Pln& thePlane)
{
  myFirstShape   = theEdge;
  mySecondShape  = TopoShape();
  myGeometryType = GeometryType_Edge;
  SetCustomPlane(thePlane);
  myIsGeometryValid = InitOneShapePoints(myFirstShape);

  SetToUpdate();
}

//=================================================================================================

void PrsDim_LengthDimension::SetMeasuredGeometry(const TopoFace& theFirstFace,
                                                 const TopoFace& theSecondFace)
{
  SetMeasuredShapes(theFirstFace, theSecondFace);
}

//=================================================================================================

void PrsDim_LengthDimension::SetMeasuredGeometry(const TopoFace& theFace,
                                                 const TopoEdge& theEdge)
{
  SetMeasuredShapes(theFace, theEdge);
}

//=================================================================================================

void PrsDim_LengthDimension::SetMeasuredShapes(const TopoShape& theFirstShape,
                                               const TopoShape& theSecondShape)
{
  gp_Pln           aComputedPlane;
  Standard_Boolean isPlaneReturned = Standard_False;

  myFirstShape  = theFirstShape;
  mySecondShape = theSecondShape;
  myIsGeometryValid =
    InitTwoShapesPoints(myFirstShape, mySecondShape, aComputedPlane, isPlaneReturned);

  if (myIsGeometryValid && !myIsPlaneCustom)
  {
    if (isPlaneReturned)
    {
      myPlane = aComputedPlane;
    }
    else
    {
      myIsGeometryValid = Standard_False;
    }
  }

  SetToUpdate();
}

//=================================================================================================

Standard_Boolean PrsDim_LengthDimension::CheckPlane(const gp_Pln& thePlane) const
{
  Standard_Boolean anIsFaultyNormal =
    thePlane.Axis().Direction().IsParallel(gce_MakeDir(myFirstPoint, mySecondPoint),
                                           Precision::Angular());

  if ((!thePlane.Contains(myFirstPoint, Precision::Confusion())
       && !thePlane.Contains(mySecondPoint, Precision::Confusion()))
      || anIsFaultyNormal)
  {
    return Standard_False;
  }

  return Standard_True;
}

//=================================================================================================

gp_Pln PrsDim_LengthDimension::ComputePlane(const Dir3d& theAttachDir) const
{
  if (!IsValidPoints(myFirstPoint, mySecondPoint))
  {
    return gp_Pln();
  }

  Point3d      aThirdPoint(myFirstPoint.Translated(Vector3d(theAttachDir)));
  gce_MakePln aPlaneConstrustor(myFirstPoint, mySecondPoint, aThirdPoint);
  return aPlaneConstrustor.Value();
}

//=================================================================================================

const AsciiString1& PrsDim_LengthDimension::GetModelUnits() const
{
  return myDrawer->DimLengthModelUnits();
}

//=================================================================================================

const AsciiString1& PrsDim_LengthDimension::GetDisplayUnits() const
{
  return myDrawer->DimLengthDisplayUnits();
}

//=================================================================================================

void PrsDim_LengthDimension::SetModelUnits(const AsciiString1& theUnits)
{
  myDrawer->SetDimLengthModelUnits(theUnits);
}

//=================================================================================================

void PrsDim_LengthDimension::SetDisplayUnits(const AsciiString1& theUnits)
{
  myDrawer->SetDimLengthDisplayUnits(theUnits);
}

//=================================================================================================

Standard_Real PrsDim_LengthDimension::ComputeValue() const
{
  if (!IsValid())
    return 0.0;

  if (!myHasCustomDirection)
    return myFirstPoint.Distance(mySecondPoint);

  return fabs(Vector3d(myFirstPoint, mySecondPoint).Dot(myDirection));
}

//=================================================================================================

void PrsDim_LengthDimension::Compute(const Handle(PrsMgr_PresentationManager)&,
                                     const Handle(Prs3d_Presentation)& thePresentation,
                                     const Standard_Integer            theMode)
{
  mySelectionGeom.Clear(theMode);

  if (!IsValid())
  {
    return;
  }

  DrawLinearDimension(thePresentation, theMode, myFirstPoint, mySecondPoint);
}

//=================================================================================================

void PrsDim_LengthDimension::ComputeFlyoutLinePoints(const Point3d& theFirstPoint,
                                                     const Point3d& theSecondPoint,
                                                     Point3d&       theLineBegPoint,
                                                     Point3d&       theLineEndPoint)
{
  if (!myHasCustomDirection)
  {
    PrsDim_Dimension::ComputeFlyoutLinePoints(theFirstPoint,
                                              theSecondPoint,
                                              theLineBegPoint,
                                              theLineEndPoint);
    return;
  }

  // find scalar of projection target vector (from start to second point) to flyout vector
  Axis3d aPlaneNormal = GetPlane().Axis();
  Vector3d aFlyoutNormalizedDir(aPlaneNormal.Direction() ^ myDirection);
  aFlyoutNormalizedDir.Normalize();
  Standard_Real aTargetProjectedToFlyout =
    Vector3d(theFirstPoint, theSecondPoint).Dot(aFlyoutNormalizedDir);

  Dir3d aFlyoutVector = aFlyoutNormalizedDir;
  // create lines for layouts
  gp_Lin aLine1(theFirstPoint, aFlyoutVector);
  gp_Lin aLine2(theSecondPoint, aFlyoutVector);

  // Get flyout end points
  theLineBegPoint =
    ElCLib1::Value(ElCLib1::Parameter(aLine1, theFirstPoint) + GetFlyout() + aTargetProjectedToFlyout,
                  aLine1);
  theLineEndPoint = ElCLib1::Value(ElCLib1::Parameter(aLine2, theSecondPoint) + GetFlyout(), aLine2);
}

//=================================================================================================

void PrsDim_LengthDimension::ComputeFlyoutSelection(
  const Handle(SelectionContainer)&   theSelection,
  const Handle(SelectMgr_EntityOwner)& theEntityOwner)
{
  if (!IsValid())
  {
    return;
  }

  ComputeLinearFlyouts(theSelection, theEntityOwner, myFirstPoint, mySecondPoint);
}

//=================================================================================================

Standard_Boolean PrsDim_LengthDimension::IsValidPoints(const Point3d& theFirstPoint,
                                                       const Point3d& theSecondPoint) const
{
  return theFirstPoint.Distance(theSecondPoint) > Precision::Confusion();
}

//=======================================================================
// function : InitTwoEdgesLength
// purpose  : Initialization of dimension between two linear edges
//=======================================================================
Standard_Boolean PrsDim_LengthDimension::InitTwoEdgesLength(const TopoEdge& theFirstEdge,
                                                            const TopoEdge& theSecondEdge,
                                                            Dir3d&            theDirAttach)
{
  Handle(GeomCurve3d) aFirstCurve, aSecondCurve;
  Point3d             aPoint11, aPoint12, aPoint21, aPoint22;
  Standard_Boolean   isFirstInfinite  = Standard_False;
  Standard_Boolean   isSecondInfinite = Standard_False;
  if (!PrsDim::ComputeGeometry(theFirstEdge,
                               theSecondEdge,
                               aFirstCurve,
                               aSecondCurve,
                               aPoint11,
                               aPoint12,
                               aPoint21,
                               aPoint22,
                               isFirstInfinite,
                               isSecondInfinite))
  {
    return Standard_False;
  }

  const Handle(GeomLine) aFirstLine  = Handle(GeomLine)::DownCast(aFirstCurve);
  const Handle(GeomLine) aSecondLine = Handle(GeomLine)::DownCast(aSecondCurve);
  if (!aFirstLine.IsNull() && !aSecondLine.IsNull())
  {
    if (!aFirstLine->Lin().Direction().IsParallel(aSecondLine->Lin().Direction(),
                                                  Precision::Angular()))
    {
      return Standard_False;
    }

    theDirAttach = aFirstLine->Lin().Direction();

    Point3d aPoint;
    if (!isFirstInfinite)
    {
      if (PrsDim::Nearest(aSecondCurve, aPoint11, aPoint21, aPoint22, aPoint))
      {
        myFirstPoint  = aPoint11;
        mySecondPoint = aPoint;
        return IsValidPoints(myFirstPoint, mySecondPoint);
      }
      else if (PrsDim::Nearest(aSecondCurve, aPoint12, aPoint21, aPoint22, aPoint))
      {
        myFirstPoint  = aPoint12;
        mySecondPoint = aPoint;
        return IsValidPoints(myFirstPoint, mySecondPoint);
      }
    }

    if (!isSecondInfinite)
    {
      if (PrsDim::Nearest(aFirstCurve, aPoint21, aPoint11, aPoint12, aPoint))
      {
        myFirstPoint  = aPoint;
        mySecondPoint = aPoint21;
        return IsValidPoints(myFirstPoint, mySecondPoint);
      }
      if (PrsDim::Nearest(aFirstCurve, aPoint22, aPoint11, aPoint12, aPoint))
      {
        myFirstPoint  = aPoint;
        mySecondPoint = aPoint22;
        return IsValidPoints(myFirstPoint, mySecondPoint);
      }
    }
  }

  GeomAPI_ExtremaCurveCurve anExtrema(aFirstCurve, aSecondCurve);
  if (anExtrema.NbExtrema() == 0)
  {
    return false;
  }

  anExtrema.NearestPoints(myFirstPoint, mySecondPoint);
  if (!IsValidPoints(myFirstPoint, mySecondPoint))
  {
    return false;
  }

  if (aFirstLine.IsNull() || aSecondLine.IsNull())
  {
    Standard_Real aParam1 = 0.0, aParam2 = 0.0;
    anExtrema.LowerDistanceParameters(aParam1, aParam2);
    BRepAdaptor_Curve aCurveAdaptor(theFirstEdge);
    Point3d            aPoint;
    Vector3d            aDir;
    aCurveAdaptor.D1(aParam1, aPoint, aDir);
    if (aDir.SquareMagnitude() <= gp1::Resolution())
    {
      return false;
    }

    theDirAttach = aDir;
  }
  return true;
}

//=======================================================================
// function : InitEdgeVertexLength
// purpose  : for first edge and second vertex shapes
//=======================================================================
Standard_Boolean PrsDim_LengthDimension::InitEdgeVertexLength(const TopoEdge&   theEdge,
                                                              const TopoVertex& theVertex,
                                                              Dir3d&              theEdgeDir,
                                                              Standard_Boolean     isInfinite)
{
  Point3d             anEdgePoint1, anEdgePoint2;
  Handle(GeomCurve3d) aCurve;
  if (!PrsDim::ComputeGeometry(theEdge, aCurve, anEdgePoint1, anEdgePoint2, isInfinite))
  {
    return Standard_False;
  }

  myFirstPoint = BRepInspector::Pnt(theVertex);

  if (Handle(GeomLine) aGeomLine = Handle(GeomLine)::DownCast(aCurve))
  {
    const gp_Lin aLin = aGeomLine->Lin();

    // Get direction of edge to build plane automatically.
    theEdgeDir = aLin.Direction();

    mySecondPoint = PrsDim::Nearest(aLin, myFirstPoint);
    return IsValidPoints(myFirstPoint, mySecondPoint);
  }

  GeomAPI_ProjectPointOnCurve anExtrema(myFirstPoint, aCurve);
  if (anExtrema.NbPoints() == 0)
  {
    return false;
  }

  mySecondPoint = anExtrema.NearestPoint();
  if (!IsValidPoints(myFirstPoint, mySecondPoint))
  {
    return false;
  }

  BRepAdaptor_Curve aCurveAdaptor(theEdge);
  Point3d            aPoint;
  Vector3d            aDir;
  aCurveAdaptor.D1(anExtrema.LowerDistanceParameter(), aPoint, aDir);
  if (aDir.SquareMagnitude() <= gp1::Resolution())
  {
    return false;
  }
  theEdgeDir = aDir;
  return true;
}

//=================================================================================================

Standard_Boolean PrsDim_LengthDimension::InitEdgeFaceLength(const TopoEdge& theEdge,
                                                            const TopoFace& theFace,
                                                            Dir3d&            theEdgeDir)
{
  theEdgeDir = gp1::DX();

  // Find attachment points (closest distance between the edge and the face)
  BRepExtrema_DistShapeShape aDistAdaptor(theEdge, theFace, Extrema_ExtFlag_MIN);
  if (!aDistAdaptor.IsDone() || aDistAdaptor.NbSolution() < 1)
  {
    return Standard_False;
  }
  myFirstPoint  = aDistAdaptor.PointOnShape1(1);
  mySecondPoint = aDistAdaptor.PointOnShape2(1);

  // Take direction for dimension line (will be orthogonalized later) parallel to edge
  BRepAdaptor_Curve aCurveAdaptor(theEdge);
  Standard_Real     aParam;
  if (aDistAdaptor.SupportOnShape1(1).ShapeType() == TopAbs_EDGE)
  {
    aDistAdaptor.ParOnEdgeS1(1, aParam);
  }
  else
  {
    Standard_Real aD1 =
      aCurveAdaptor.Value(aCurveAdaptor.FirstParameter()).SquareDistance(myFirstPoint);
    Standard_Real aD2 =
      aCurveAdaptor.Value(aCurveAdaptor.LastParameter()).SquareDistance(myFirstPoint);
    aParam = (aD1 < aD2 ? aCurveAdaptor.FirstParameter() : aCurveAdaptor.LastParameter());
  }
  Point3d aP;
  Vector3d aV;
  aCurveAdaptor.D1(aParam, aP, aV);
  if (aV.SquareMagnitude() > gp1::Resolution())
  {
    theEdgeDir = aV;
  }

  // reverse direction if parameter is close to the end of the curve,
  // to reduce chances to have overlapping between dimension line and edge
  if (Abs(aParam - aCurveAdaptor.FirstParameter()) < Abs(aParam - aCurveAdaptor.LastParameter()))
  {
    theEdgeDir.Reverse();
  }

  return IsValidPoints(myFirstPoint, mySecondPoint);
}

//=======================================================================
// function : InitTwoShapesPoints
// purpose  : Initialization of two points where dimension layouts
//           will be attached
//=======================================================================
Standard_Boolean PrsDim_LengthDimension::InitTwoShapesPoints(const TopoShape& theFirstShape,
                                                             const TopoShape& theSecondShape,
                                                             gp_Pln&             theComputedPlane,
                                                             Standard_Boolean&   theIsPlaneComputed)
{
  theIsPlaneComputed = Standard_False;
  Dir3d           aDirAttach;
  Standard_Boolean isInfinite = Standard_False;
  Standard_Boolean isSuccess  = Standard_False;
  switch (theFirstShape.ShapeType())
  {
    case TopAbs_FACE: {
      // Initialization for face
      gp_Pln               aFirstPlane;
      Handle(GeomSurface) aFirstSurface;
      PrsDim_KindOfSurface aFirstSurfKind;
      Standard_Real        aFirstOffset;

      TopoFace aFirstFace = TopoDS::Face(theFirstShape);

      PrsDim::InitFaceLength(TopoDS::Face(theFirstShape),
                             aFirstPlane,
                             aFirstSurface,
                             aFirstSurfKind,
                             aFirstOffset);

      if (theSecondShape.ShapeType() == TopAbs_FACE)
      {
        // Initialization for face
        myGeometryType = GeometryType_Faces;
        gp_Pln               aSecondPlane;
        Handle(GeomSurface) aSecondSurface;
        PrsDim_KindOfSurface aSecondSurfKind;
        Standard_Real        aSecondOffset;

        TopoFace aSecondFace = TopoDS::Face(theSecondShape);

        PrsDim::InitFaceLength(aSecondFace,
                               aSecondPlane,
                               aSecondSurface,
                               aSecondSurfKind,
                               aSecondOffset);

        if (aFirstSurfKind == PrsDim_KOS_Plane)
        {
          if (!aFirstPlane.Axis().Direction().IsParallel(aSecondPlane.Axis().Direction(),
                                                         Precision::Angular()))
          {
            return Standard_False;
          }

          ShapeExplorer anExplorer(theFirstShape, TopAbs_VERTEX);

          // In case of infinite planes
          if (!anExplorer.More())
          {
            myFirstPoint = aFirstPlane.Location();
          }
          else
          {
            myFirstPoint = BRepInspector::Pnt(TopoDS::Vertex(anExplorer.Current()));
          }

          mySecondPoint = PrsDim::ProjectPointOnPlane(myFirstPoint, aSecondPlane);

          Standard_Real anU, aV;
          ElSLib1::Parameters(aSecondPlane, mySecondPoint, anU, aV);

          BRepTopAdaptor_FClass2d aClassifier(aSecondFace, Precision::Confusion());
          TopAbs_State            aState = aClassifier.Perform(gp_Pnt2d(anU, aV), Standard_False);

          if (aState == TopAbs_OUT || aState == TopAbs_UNKNOWN)
          {
            mySecondPoint = PrsDim::Nearest(aSecondFace, myFirstPoint);
          }

          isSuccess = IsValidPoints(myFirstPoint, mySecondPoint);
          if (isSuccess)
          {
            theComputedPlane   = ComputePlane(aFirstPlane.Position().XDirection());
            theIsPlaneComputed = Standard_True;
          }
        }
        else // curvilinear faces
        {
          Standard_Real aU1Min, aV1Min, aU1Max, aV1Max;
          Standard_Real aU2Min, aV2Min, aU2Max, aV2Max;
          BRepTools1::UVBounds(aFirstFace, aU1Min, aU1Max, aV1Min, aV1Max);
          BRepTools1::UVBounds(aSecondFace, aU2Min, aU2Max, aV2Min, aV2Max);

          GeomAPI_ExtremaSurfaceSurface anExtrema(aFirstSurface,
                                                  aSecondSurface,
                                                  aU1Min,
                                                  aU1Max,
                                                  aV1Min,
                                                  aV1Max,
                                                  aU2Min,
                                                  aU2Max,
                                                  aV2Min,
                                                  aV2Max);

          Standard_Real aU1, aV1, aU2, aV2;
          anExtrema.LowerDistanceParameters(aU1, aV1, aU2, aV2);
          myFirstPoint  = BRepInspector::Surface(aFirstFace)->Value(aU1, aV1);
          mySecondPoint = BRepInspector::Surface(aSecondFace)->Value(aU2, aV2);

          // Adjust automatic plane
          Frame3d aLocalAxes(myFirstPoint, gce_MakeDir(myFirstPoint, mySecondPoint));
          aDirAttach = gce_MakeDir(aLocalAxes.XDirection());

          // Check points
          isSuccess = IsValidPoints(myFirstPoint, mySecondPoint);
          if (isSuccess)
          {
            theComputedPlane   = ComputePlane(aDirAttach);
            theIsPlaneComputed = Standard_True;
          }
        }

        return isSuccess && IsValidPoints(myFirstPoint, mySecondPoint);
      }
      else if (theSecondShape.ShapeType() == TopAbs_EDGE)
      {
        myGeometryType = GeometryType_EdgeFace;
        isSuccess =
          InitEdgeFaceLength(TopoDS::Edge(theSecondShape), TopoDS::Face(theFirstShape), aDirAttach);

        if (isSuccess)
        {
          theComputedPlane   = ComputePlane(aDirAttach);
          theIsPlaneComputed = Standard_True;
        }

        return isSuccess;
      }
    }
    break;

    case TopAbs_EDGE: {
      if (theSecondShape.ShapeType() == TopAbs_VERTEX)
      {
        myGeometryType = GeometryType_EdgeVertex;
        isSuccess      = InitEdgeVertexLength(TopoDS::Edge(theFirstShape),
                                         TopoDS::Vertex(theSecondShape),
                                         aDirAttach,
                                         isInfinite);

        if (isSuccess)
        {
          theComputedPlane   = ComputePlane(aDirAttach);
          theIsPlaneComputed = Standard_True;
        }

        return isSuccess;
      }
      else if (theSecondShape.ShapeType() == TopAbs_EDGE)
      {
        myGeometryType = GeometryType_Edges;
        isSuccess =
          InitTwoEdgesLength(TopoDS::Edge(theFirstShape), TopoDS::Edge(theSecondShape), aDirAttach);

        if (isSuccess)
        {
          theComputedPlane   = ComputePlane(aDirAttach);
          theIsPlaneComputed = Standard_True;
        }

        return isSuccess;
      }
      else if (theSecondShape.ShapeType() == TopAbs_FACE)
      {
        myGeometryType = GeometryType_EdgeFace;
        isSuccess =
          InitEdgeFaceLength(TopoDS::Edge(theFirstShape), TopoDS::Face(theSecondShape), aDirAttach);

        if (isSuccess)
        {
          theComputedPlane   = ComputePlane(aDirAttach);
          theIsPlaneComputed = Standard_True;
        }

        return isSuccess;
      }
    }
    break;

    case TopAbs_VERTEX: {
      if (theSecondShape.ShapeType() == TopAbs_VERTEX)
      {
        myGeometryType = GeometryType_Points;
        myFirstPoint   = BRepInspector::Pnt(TopoDS::Vertex(theFirstShape));
        mySecondPoint  = BRepInspector::Pnt(TopoDS::Vertex(theSecondShape));

        return IsValidPoints(myFirstPoint, mySecondPoint);
      }
      else if (theSecondShape.ShapeType() == TopAbs_EDGE)
      {
        myGeometryType = GeometryType_EdgeVertex;
        isSuccess      = InitEdgeVertexLength(TopoDS::Edge(theSecondShape),
                                         TopoDS::Vertex(theFirstShape),
                                         aDirAttach,
                                         isInfinite);
        if (isSuccess)
        {
          theComputedPlane   = ComputePlane(aDirAttach);
          theIsPlaneComputed = Standard_True;
        }

        return isSuccess;
      }
    }
    break;

    case TopAbs_COMPOUND:
    case TopAbs_COMPSOLID:
    case TopAbs_SOLID:
    case TopAbs_SHELL:
    case TopAbs_WIRE:
    case TopAbs_SHAPE:
      break;
  }

  return Standard_False;
}

//=======================================================================
// function : InitOneShapePoints
// purpose  : Initialization of two points where dimension layouts
//           will be attached
// Attention: 1) <theShape> can be only the edge in current implementation
//            2) No length for infinite edge
//=======================================================================
Standard_Boolean PrsDim_LengthDimension::InitOneShapePoints(const TopoShape& theShape)
{
  if (theShape.ShapeType() != TopAbs_EDGE)
  {
    return Standard_False;
  }

  TopoEdge anEdge = TopoDS::Edge(theShape);

  BRepAdaptor_Curve aBrepCurve(anEdge);
  Standard_Real     aFirst = aBrepCurve.FirstParameter();
  Standard_Real     aLast  = aBrepCurve.LastParameter();

  if (aBrepCurve.GetType() != GeomAbs_Line)
  {
    return Standard_False;
  }

  Standard_Boolean isInfinite = (Precision::IsInfinite(aFirst) || Precision::IsInfinite(aLast));
  if (isInfinite)
  {
    return Standard_False;
  }

  myFirstPoint  = aBrepCurve.Value(aBrepCurve.FirstParameter());
  mySecondPoint = aBrepCurve.Value(aBrepCurve.LastParameter());

  return IsValidPoints(myFirstPoint, mySecondPoint);
}

//=================================================================================================

Point3d PrsDim_LengthDimension::GetTextPosition() const
{
  if (IsTextPositionCustom())
  {
    return myFixedTextPosition;
  }

  // Counts text position according to the dimension parameters
  return GetTextPositionForLinear(myFirstPoint, mySecondPoint);
}

//=================================================================================================

void PrsDim_LengthDimension::SetTextPosition(const Point3d& theTextPos)
{
  if (!IsValid())
  {
    return;
  }

  myIsTextPositionFixed = Standard_True;
  myFixedTextPosition   = theTextPos;

  SetToUpdate();
}

//=================================================================================================

void PrsDim_LengthDimension::SetDirection(const Dir3d&          theDirection,
                                          const Standard_Boolean theUseDirection)
{
  myHasCustomDirection = theUseDirection;
  if (myHasCustomDirection)
    myDirection = theDirection;
}
