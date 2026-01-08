// Created on: 1996-12-05
// Created by: Arnaud BOUZY/Odile Olivier
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

#include <PrsDim_AngleDimension.hxx>

#include <PrsDim.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRep_Tool.hxx>
#include <ElCLib.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <gce_MakeLin2d.hxx>
#include <gce_MakeLin.hxx>
#include <gce_MakeCirc.hxx>
#include <gce_MakeCone.hxx>
#include <gce_MakePln.hxx>
#include <gce_MakeDir.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <IntAna2d_AnaIntersection.hxx>
#include <ProjLib.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <PrsMgr_PresentationManager.hxx>
#include <Select3D_SensitiveGroup.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <SelectMgr_Selection.hxx>
#include <Standard_ProgramError.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_AngleDimension, PrsDim_Dimension)

namespace
{
static const UtfString THE_EMPTY_LABEL_STRING;
static const Standard_Real              THE_EMPTY_LABEL_WIDTH = 0.0;
static const Standard_ExtCharacter      THE_DEGREE_SYMBOL(0x00B0);
static const Standard_Real              THE_3D_TEXT_MARGIN = 0.1;

//! Returns true if the given points lie on a same line.
static Standard_Boolean isSameLine(const Point3d& theFirstPoint,
                                   const Point3d& theCenterPoint,
                                   const Point3d& theSecondPoint)
{
  Vector3d aVec1(theFirstPoint, theCenterPoint);
  Vector3d aVec2(theCenterPoint, theSecondPoint);

  return aVec1.IsParallel(aVec2, Precision1::Angular());
}
} // namespace

//=================================================================================================

PrsDim_AngleDimension::PrsDim_AngleDimension(const TopoEdge& theFirstEdge,
                                             const TopoEdge& theSecondEdge)
    : PrsDim_Dimension(PrsDim_KOD_PLANEANGLE)
{
  Init();
  SetMeasuredGeometry(theFirstEdge, theSecondEdge);
}

//=================================================================================================

PrsDim_AngleDimension::PrsDim_AngleDimension(const Point3d& theFirstPoint,
                                             const Point3d& theSecondPoint,
                                             const Point3d& theThirdPoint)
    : PrsDim_Dimension(PrsDim_KOD_PLANEANGLE)
{
  Init();
  SetMeasuredGeometry(theFirstPoint, theSecondPoint, theThirdPoint);
}

//=================================================================================================

PrsDim_AngleDimension::PrsDim_AngleDimension(const TopoVertex& theFirstVertex,
                                             const TopoVertex& theSecondVertex,
                                             const TopoVertex& theThirdVertex)
    : PrsDim_Dimension(PrsDim_KOD_PLANEANGLE)
{
  Init();
  SetMeasuredGeometry(theFirstVertex, theSecondVertex, theThirdVertex);
}

//=================================================================================================

PrsDim_AngleDimension::PrsDim_AngleDimension(const TopoFace& theCone)
    : PrsDim_Dimension(PrsDim_KOD_PLANEANGLE)
{
  Init();
  SetMeasuredGeometry(theCone);
}

//=================================================================================================

PrsDim_AngleDimension::PrsDim_AngleDimension(const TopoFace& theFirstFace,
                                             const TopoFace& theSecondFace)
    : PrsDim_Dimension(PrsDim_KOD_PLANEANGLE)
{
  Init();
  SetMeasuredGeometry(theFirstFace, theSecondFace);
}

//=================================================================================================

PrsDim_AngleDimension::PrsDim_AngleDimension(const TopoFace& theFirstFace,
                                             const TopoFace& theSecondFace,
                                             const Point3d&      thePoint)
    : PrsDim_Dimension(PrsDim_KOD_PLANEANGLE)
{
  Init();
  SetMeasuredGeometry(theFirstFace, theSecondFace, thePoint);
}

//=================================================================================================

void PrsDim_AngleDimension::SetMeasuredGeometry(const TopoEdge& theFirstEdge,
                                                const TopoEdge& theSecondEdge)
{
  gp_Pln aComputedPlane;

  myFirstShape      = theFirstEdge;
  mySecondShape     = theSecondEdge;
  myThirdShape      = TopoShape();
  myGeometryType    = GeometryType_Edges;
  myIsGeometryValid = InitTwoEdgesAngle(aComputedPlane);

  if (myIsGeometryValid && !myIsPlaneCustom)
  {
    myPlane = aComputedPlane;
  }

  SetToUpdate();
}

//=================================================================================================

void PrsDim_AngleDimension::SetMeasuredGeometry(const Point3d& theFirstPoint,
                                                const Point3d& theSecondPoint,
                                                const Point3d& theThirdPoint)
{
  myFirstPoint      = theFirstPoint;
  myCenterPoint     = theSecondPoint;
  mySecondPoint     = theThirdPoint;
  myFirstShape      = BRepLib_MakeVertex(myFirstPoint);
  mySecondShape     = BRepLib_MakeVertex(myCenterPoint);
  myThirdShape      = BRepLib_MakeVertex(mySecondPoint);
  myGeometryType    = GeometryType_Points;
  myIsGeometryValid = IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);

  Standard_Boolean anIsSameLine = isSameLine(myFirstPoint, myCenterPoint, mySecondPoint);
  if (myIsGeometryValid && !myIsPlaneCustom && !anIsSameLine)
  {
    ComputePlane();
  }

  SetToUpdate();
}

//=================================================================================================

void PrsDim_AngleDimension::SetMeasuredGeometry(const TopoVertex& theFirstVertex,
                                                const TopoVertex& theSecondVertex,
                                                const TopoVertex& theThirdVertex)
{
  myFirstShape      = theFirstVertex;
  mySecondShape     = theSecondVertex;
  myThirdShape      = theThirdVertex;
  myFirstPoint      = BRepInspector::Pnt(theFirstVertex);
  myCenterPoint     = BRepInspector::Pnt(theSecondVertex);
  mySecondPoint     = BRepInspector::Pnt(theThirdVertex);
  myGeometryType    = GeometryType_Points;
  myIsGeometryValid = IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);

  Standard_Boolean anIsSameLine = isSameLine(myFirstPoint, myCenterPoint, mySecondPoint);
  if (myIsGeometryValid && !myIsPlaneCustom && !anIsSameLine)
  {
    ComputePlane();
  }

  SetToUpdate();
}

//=================================================================================================

void PrsDim_AngleDimension::SetMeasuredGeometry(const TopoFace& theCone)
{
  myFirstShape      = theCone;
  mySecondShape     = TopoShape();
  myThirdShape      = TopoShape();
  myGeometryType    = GeometryType_Face;
  myIsGeometryValid = InitConeAngle();

  if (myIsGeometryValid && !myIsPlaneCustom)
  {
    ComputePlane();
  }

  SetToUpdate();
}

//=================================================================================================

void PrsDim_AngleDimension::SetMeasuredGeometry(const TopoFace& theFirstFace,
                                                const TopoFace& theSecondFace)
{
  myFirstShape      = theFirstFace;
  mySecondShape     = theSecondFace;
  myThirdShape      = TopoShape();
  myGeometryType    = GeometryType_Faces;
  myIsGeometryValid = InitTwoFacesAngle();

  if (myIsGeometryValid && !myIsPlaneCustom)
  {
    ComputePlane();
  }

  SetToUpdate();
}

//=================================================================================================

void PrsDim_AngleDimension::SetMeasuredGeometry(const TopoFace& theFirstFace,
                                                const TopoFace& theSecondFace,
                                                const Point3d&      thePoint)
{
  myFirstShape      = theFirstFace;
  mySecondShape     = theSecondFace;
  myThirdShape      = TopoShape();
  myGeometryType    = GeometryType_Faces;
  myIsGeometryValid = InitTwoFacesAngle(thePoint);

  if (myIsGeometryValid && !myIsPlaneCustom)
  {
    ComputePlane();
  }

  SetToUpdate();
}

//=================================================================================================

void PrsDim_AngleDimension::Init()
{
  SetType(PrsDim_TypeOfAngle_Interior);
  SetArrowsVisibility(PrsDim_TypeOfAngleArrowVisibility_Both);
  SetSpecialSymbol(THE_DEGREE_SYMBOL);
  SetDisplaySpecialSymbol(PrsDim_DisplaySpecialSymbol_After);
  SetFlyout(15.0);
}

//=================================================================================================

Point3d PrsDim_AngleDimension::GetCenterOnArc(const Point3d& theFirstAttach,
                                             const Point3d& theSecondAttach,
                                             const Point3d& theCenter) const
{
  // construct plane where the circle and the arc are located
  gce_MakePln aConstructPlane(theFirstAttach, theSecondAttach, theCenter);
  if (!aConstructPlane.IsDone())
  {
    return gp1::Origin();
  }

  gp_Pln aPlane = aConstructPlane.Value();
  // to have an exterior angle presentation, a plane for further constructed circle should be
  // reversed
  if (myType == PrsDim_TypeOfAngle_Exterior)
  {
    Axis3d anAxis = aPlane.Axis();
    Dir3d aDir   = anAxis.Direction();
    aDir.Reverse();
    aPlane.SetAxis(Axis3d(anAxis.Location(), aDir));
  }

  Standard_Real aRadius = theFirstAttach.Distance(theCenter);

  // construct circle forming the arc
  gce_MakeCirc aConstructCircle(theCenter, aPlane, aRadius);
  if (!aConstructCircle.IsDone())
  {
    return gp1::Origin();
  }

  gp_Circ aCircle = aConstructCircle.Value();

  // compute angle parameters of arc end-points on circle
  Standard_Real aParamBeg = ElCLib1::Parameter(aCircle, theFirstAttach);
  Standard_Real aParamEnd = ElCLib1::Parameter(aCircle, theSecondAttach);
  ElCLib1::AdjustPeriodic(0.0, M_PI * 2, Precision1::PConfusion(), aParamBeg, aParamEnd);

  return ElCLib1::Value((aParamBeg + aParamEnd) * 0.5, aCircle);
}

//=================================================================================================

Dir3d PrsDim_AngleDimension::GetNormalForMinAngle() const
{
  const Dir3d& aNormal = myPlane.Axis().Direction();
  Dir3d        aFirst(Vector3d(myCenterPoint, myFirstPoint));
  Dir3d        aSecond(Vector3d(myCenterPoint, mySecondPoint));

  return aFirst.AngleWithRef(aSecond, aNormal) < 0.0 ? aNormal.Reversed() : aNormal;
}

//=======================================================================
// function : DrawArc
// purpose  : draws the arc between two attach points
//=======================================================================
void PrsDim_AngleDimension::DrawArc(const Handle(Prs3d_Presentation)& thePresentation,
                                    const Point3d&                     theFirstAttach,
                                    const Point3d&                     theSecondAttach,
                                    const Point3d&                     theCenter,
                                    const Standard_Real               theRadius,
                                    const Standard_Integer            theMode)
{
  gp_Pln aPlane(myCenterPoint, GetNormalForMinAngle());

  // to have an exterior angle presentation, a plane for further constructed circle should be
  // reversed
  if (myType == PrsDim_TypeOfAngle_Exterior)
  {
    Axis3d anAxis = aPlane.Axis();
    Dir3d aDir   = anAxis.Direction();
    aDir.Reverse();
    aPlane.SetAxis(Axis3d(anAxis.Location(), aDir));
  }

  // construct circle forming the arc
  gce_MakeCirc aConstructCircle(theCenter, aPlane, theRadius);
  if (!aConstructCircle.IsDone())
  {
    return;
  }

  gp_Circ aCircle = aConstructCircle.Value();

  // construct the arc
  GC_MakeArcOfCircle aConstructArc(aCircle, theFirstAttach, theSecondAttach, Standard_True);
  if (!aConstructArc.IsDone())
  {
    return;
  }

  // generate points with specified deflection
  const Handle(Geom_TrimmedCurve)& anArcCurve = aConstructArc.Value();

  GeomAdaptor_Curve anArcAdaptor(anArcCurve,
                                 anArcCurve->FirstParameter(),
                                 anArcCurve->LastParameter());

  // compute number of discretization elements in old-fanshioned way
  Vector3d        aCenterToFirstVec(theCenter, theFirstAttach);
  Vector3d        aCenterToSecondVec(theCenter, theSecondAttach);
  Standard_Real anAngle = aCenterToFirstVec.Angle(aCenterToSecondVec);
  if (myType == PrsDim_TypeOfAngle_Exterior)
    anAngle = 2.0 * M_PI - anAngle;
  // it sets 50 points on PI, and a part of points if angle is less
  const Standard_Integer aNbPoints = Max(4, Standard_Integer(50.0 * anAngle / M_PI));

  GCPnts_UniformAbscissa aMakePnts(anArcAdaptor, aNbPoints);
  if (!aMakePnts.IsDone())
  {
    return;
  }

  // init data arrays for graphical and selection primitives
  Handle(Graphic3d_ArrayOfPolylines) aPrimSegments = new Graphic3d_ArrayOfPolylines(aNbPoints);

  SelectionGeometry1::Curve& aSensitiveCurve = mySelectionGeom.NewCurve();

  // load data into arrays
  for (Standard_Integer aPntIt = 1; aPntIt <= aMakePnts.NbPoints(); ++aPntIt)
  {
    Point3d aPnt = anArcAdaptor.Value(aMakePnts.Parameter(aPntIt));

    aPrimSegments->AddVertex(aPnt);

    aSensitiveCurve.Append(aPnt);
  }

  // add display presentation
  if (!myDrawer->DimensionAspect()->IsText3d() && theMode == ComputeMode_All)
  {
    thePresentation->CurrentGroup()->SetStencilTestOptions(Standard_True);
  }
  Handle(Graphic3d_AspectLine3d) aDimensionLineStyle =
    myDrawer->DimensionAspect()->LineAspect()->Aspect();
  thePresentation->CurrentGroup()->SetPrimitivesAspect(aDimensionLineStyle);
  thePresentation->CurrentGroup()->AddPrimitiveArray(aPrimSegments);
  if (!myDrawer->DimensionAspect()->IsText3d() && theMode == ComputeMode_All)
  {
    thePresentation->CurrentGroup()->SetStencilTestOptions(Standard_False);
  }
}

//=================================================================================================

void PrsDim_AngleDimension::DrawArcWithText(const Handle(Prs3d_Presentation)& thePresentation,
                                            const Point3d&                     theFirstAttach,
                                            const Point3d&                     theSecondAttach,
                                            const Point3d&                     theCenter,
                                            const UtfString& theText,
                                            const Standard_Real               theTextWidth,
                                            const Standard_Integer            theMode,
                                            const Standard_Integer            theLabelPosition)
{
  gp_Pln aPlane(myCenterPoint, GetNormalForMinAngle());

  Standard_Real aRadius = theFirstAttach.Distance(myCenterPoint);

  // construct circle forming the arc
  gce_MakeCirc aConstructCircle(theCenter, aPlane, aRadius);
  if (!aConstructCircle.IsDone())
  {
    return;
  }

  gp_Circ aCircle = aConstructCircle.Value();

  // compute angle parameters of arc end-points on circle
  Standard_Real aParamBeg = ElCLib1::Parameter(aCircle, theFirstAttach);
  Standard_Real aParamEnd = ElCLib1::Parameter(aCircle, theSecondAttach);
  ElCLib1::AdjustPeriodic(0.0, M_PI * 2, Precision1::PConfusion(), aParamBeg, aParamEnd);

  // middle point of arc parameter on circle
  Standard_Real aParamMid = (aParamBeg + aParamEnd) * 0.5;

  // add text graphical primitives
  if (theMode == ComputeMode_All || theMode == ComputeMode_Text)
  {
    Point3d aTextPos = ElCLib1::Value(aParamMid, aCircle);
    Dir3d aTextDir = gce_MakeDir(theFirstAttach, theSecondAttach);

    // Drawing text
    drawText(thePresentation, aTextPos, aTextDir, theText, theLabelPosition);
  }

  if (theMode != ComputeMode_All && theMode != ComputeMode_Line)
  {
    return;
  }

  Handle(Prs3d_DimensionAspect) aDimensionAspect = myDrawer->DimensionAspect();

  Standard_Boolean isLineBreak =
    aDimensionAspect->TextVerticalPosition() == Prs3d_DTVP_Center && aDimensionAspect->IsText3d();

  if (isLineBreak)
  {
    // compute gap for label as parametric size of sector on circle segment
    Standard_Real aSectorOfText = theTextWidth / aRadius;
    Standard_Real aTextBegin    = aParamMid - aSectorOfText * 0.5;
    Standard_Real aTextEnd      = aParamMid + aSectorOfText * 0.5;
    Point3d        aTextPntBeg   = ElCLib1::Value(aTextBegin, aCircle);
    Point3d        aTextPntEnd   = ElCLib1::Value(aTextEnd, aCircle);

    // Drawing arcs
    if (aTextBegin > aParamBeg)
    {
      DrawArc(thePresentation, theFirstAttach, aTextPntBeg, theCenter, aRadius, theMode);
    }
    if (aTextEnd < aParamEnd)
    {
      DrawArc(thePresentation, aTextPntEnd, theSecondAttach, theCenter, aRadius, theMode);
    }
  }
  else
  {
    DrawArc(thePresentation, theFirstAttach, theSecondAttach, theCenter, aRadius, theMode);
  }
}

//=================================================================================================

Standard_Boolean PrsDim_AngleDimension::CheckPlane(const gp_Pln& thePlane) const
{
  if (!thePlane.Contains(myFirstPoint, Precision1::Confusion())
      && !thePlane.Contains(mySecondPoint, Precision1::Confusion())
      && !thePlane.Contains(myCenterPoint, Precision1::Confusion()))
  {
    return Standard_False;
  }

  return Standard_True;
}

//=================================================================================================

void PrsDim_AngleDimension::ComputePlane()
{
  if (!myIsGeometryValid)
  {
    return;
  }

  // Compute working plane so that Y axis is codirectional
  // with Y axis of text coordinate system (necessary for text alignment)
  Vector3d aFirstVec   = Vector3d(myCenterPoint, myFirstPoint);
  Vector3d aSecondVec  = Vector3d(myCenterPoint, mySecondPoint);
  Vector3d aDirectionN = aSecondVec ^ aFirstVec;
  Vector3d aDirectionY = aFirstVec + aSecondVec;
  Vector3d aDirectionX = aDirectionY ^ aDirectionN;

  myPlane = gp_Pln(Ax3(myCenterPoint, Dir3d(aDirectionN), Dir3d(aDirectionX)));
}

//=================================================================================================

const AsciiString1& PrsDim_AngleDimension::GetModelUnits() const
{
  return myDrawer->DimAngleModelUnits();
}

//=================================================================================================

const AsciiString1& PrsDim_AngleDimension::GetDisplayUnits() const
{
  return myDrawer->DimAngleDisplayUnits();
}

//=================================================================================================

void PrsDim_AngleDimension::SetModelUnits(const AsciiString1& theUnits)
{
  myDrawer->SetDimAngleModelUnits(theUnits);
}

//=================================================================================================

void PrsDim_AngleDimension::SetDisplayUnits(const AsciiString1& theUnits)
{
  myDrawer->SetDimAngleDisplayUnits(theUnits);
}

//=================================================================================================

Standard_Real PrsDim_AngleDimension::ComputeValue() const
{
  if (!IsValid())
  {
    return 0.0;
  }

  Vector3d aVec1(myCenterPoint, myFirstPoint);
  Vector3d aVec2(myCenterPoint, mySecondPoint);

  Standard_Real anAngle = aVec1.AngleWithRef(aVec2, GetNormalForMinAngle());

  return anAngle > 0.0 ? anAngle : (2.0 * M_PI + anAngle);
}

//=======================================================================
// function : Compute
// purpose  : Having three Point3d points compute presentation
//=======================================================================
void PrsDim_AngleDimension::Compute(const Handle(PrsMgr_PresentationManager)&,
                                    const Handle(Prs3d_Presentation)& thePresentation,
                                    const Standard_Integer            theMode)
{
  mySelectionGeom.Clear(theMode);

  if (!IsValid())
  {
    return;
  }

  // Parameters for presentation
  Handle(Prs3d_DimensionAspect) aDimensionAspect = myDrawer->DimensionAspect();

  thePresentation->CurrentGroup()->SetPrimitivesAspect(aDimensionAspect->LineAspect()->Aspect());

  Standard_Real anArrowLength = aDimensionAspect->ArrowAspect()->Length();

  // prepare label string and compute its geometrical width
  Standard_Real              aLabelWidth;
  UtfString aLabelString = GetValueString(aLabelWidth);

  // add margins to label width
  if (aDimensionAspect->IsText3d())
  {
    aLabelWidth += aDimensionAspect->TextAspect()->Height() * THE_3D_TEXT_MARGIN * 2.0;
  }

  // Get parameters from aspect or adjust it according with custom text position
  Standard_Real                         anExtensionSize = aDimensionAspect->ExtensionSize();
  Prs3d_DimensionTextHorizontalPosition aHorisontalTextPos =
    aDimensionAspect->TextHorizontalPosition();

  if (IsTextPositionCustom())
  {
    AdjustParameters(myFixedTextPosition, anExtensionSize, aHorisontalTextPos, myFlyout);
  }

  // Handle user-defined and automatic arrow placement
  Standard_Boolean isArrowsExternal = Standard_False;
  Standard_Integer aLabelPosition   = LabelPosition_None;

  FitTextAlignment(aHorisontalTextPos, aLabelPosition, isArrowsExternal);

  Point3d aFirstAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, myFirstPoint).Normalized() * GetFlyout());
  Point3d aSecondAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, mySecondPoint).Normalized() * GetFlyout());

  // Arrows positions and directions
  Vector3d aWorkingPlaneDir(GetNormalForMinAngle());

  Dir3d aFirstExtensionDir  = aWorkingPlaneDir.Reversed() ^ Vector3d(myCenterPoint, aFirstAttach);
  Dir3d aSecondExtensionDir = aWorkingPlaneDir ^ Vector3d(myCenterPoint, aSecondAttach);

  Vector3d aFirstArrowVec  = Vector3d(aFirstExtensionDir) * anArrowLength;
  Vector3d aSecondArrowVec = Vector3d(aSecondExtensionDir) * anArrowLength;

  if (isArrowsExternal)
  {
    aFirstArrowVec.Reverse();
    aSecondArrowVec.Reverse();
  }

  Point3d aFirstArrowBegin(0.0, 0.0, 0.0);
  Point3d aFirstArrowEnd(0.0, 0.0, 0.0);
  Point3d aSecondArrowBegin(0.0, 0.0, 0.0);
  Point3d aSecondArrowEnd(0.0, 0.0, 0.0);

  aFirstArrowBegin  = aFirstAttach;
  aSecondArrowBegin = aSecondAttach;
  aFirstArrowEnd    = aFirstAttach;
  aSecondArrowEnd   = aSecondAttach;

  if (aDimensionAspect->ArrowAspect()->IsZoomable())
  {
    aFirstArrowEnd.Translate(-aFirstArrowVec);
    aSecondArrowEnd.Translate(-aSecondArrowVec);
  }

  // Group1: stenciling text and the angle dimension arc
  thePresentation->NewGroup();

  Standard_Integer aHPosition = aLabelPosition & LabelPosition_HMask;

  // draw text label
  switch (aHPosition)
  {
    case LabelPosition_HCenter: {
      Standard_Boolean isLineBreak = aDimensionAspect->TextVerticalPosition() == Prs3d_DTVP_Center
                                     && aDimensionAspect->IsText3d();

      if (isLineBreak)
      {
        DrawArcWithText(thePresentation,
                        aFirstAttach,
                        aSecondAttach,
                        myCenterPoint,
                        aLabelString,
                        aLabelWidth,
                        theMode,
                        aLabelPosition);
        break;
      }

      // compute text primitives
      if (theMode == ComputeMode_All || theMode == ComputeMode_Text)
      {
        Vector3d aDimensionDir(aFirstAttach, aSecondAttach);
        Point3d aTextPos = IsTextPositionCustom()
                            ? myFixedTextPosition
                            : GetCenterOnArc(aFirstAttach, aSecondAttach, myCenterPoint);
        Dir3d aTextDir = aDimensionDir;

        drawText(thePresentation, aTextPos, aTextDir, aLabelString, aLabelPosition);
      }

      if (theMode == ComputeMode_All || theMode == ComputeMode_Line)
      {
        DrawArc(thePresentation,
                (isArrowsExternal || !isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_First))
                  ? aFirstAttach
                  : aFirstArrowEnd,
                (isArrowsExternal || !isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_Second))
                  ? aSecondAttach
                  : aSecondArrowEnd,
                myCenterPoint,
                Abs(GetFlyout()),
                theMode);
      }
    }
    break;

    case LabelPosition_Left: {
      DrawExtension(thePresentation,
                    anExtensionSize,
                    (isArrowsExternal && isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_First))
                      ? aFirstArrowEnd
                      : aFirstAttach,
                    aFirstExtensionDir,
                    aLabelString,
                    aLabelWidth,
                    theMode,
                    aLabelPosition);
    }
    break;

    case LabelPosition_Right: {
      DrawExtension(thePresentation,
                    anExtensionSize,
                    (isArrowsExternal && isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_Second))
                      ? aSecondArrowEnd
                      : aSecondAttach,
                    aSecondExtensionDir,
                    aLabelString,
                    aLabelWidth,
                    theMode,
                    aLabelPosition);
    }
    break;
  }

  // dimension arc without text
  if ((theMode == ComputeMode_All || theMode == ComputeMode_Line)
      && aHPosition != LabelPosition_HCenter)
  {
    thePresentation->NewGroup();

    DrawArc(thePresentation,
            (isArrowsExternal || !isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_First))
              ? aFirstAttach
              : aFirstArrowEnd,
            (isArrowsExternal || !isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_Second))
              ? aSecondAttach
              : aSecondArrowEnd,
            myCenterPoint,
            Abs(GetFlyout()),
            theMode);
  }

  // arrows and arrow extensions
  if (theMode == ComputeMode_All || theMode == ComputeMode_Line)
  {
    thePresentation->NewGroup();

    if (isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_First))
      DrawArrow(thePresentation, aFirstArrowBegin, Dir3d(aFirstArrowVec));
    if (isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_Second))
      DrawArrow(thePresentation, aSecondArrowBegin, Dir3d(aSecondArrowVec));
  }

  if ((theMode == ComputeMode_All || theMode == ComputeMode_Line) && isArrowsExternal)
  {
    thePresentation->NewGroup();

    if (aHPosition != LabelPosition_Left && isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_First))
    {
      DrawExtension(thePresentation,
                    aDimensionAspect->ArrowTailSize(),
                    aFirstArrowEnd,
                    aFirstExtensionDir,
                    THE_EMPTY_LABEL_STRING,
                    THE_EMPTY_LABEL_WIDTH,
                    theMode,
                    LabelPosition_None);
    }

    if (aHPosition != LabelPosition_Right
        && isArrowVisible(PrsDim_TypeOfAngleArrowVisibility_Second))
    {
      DrawExtension(thePresentation,
                    aDimensionAspect->ArrowTailSize(),
                    aSecondArrowEnd,
                    aSecondExtensionDir,
                    THE_EMPTY_LABEL_STRING,
                    THE_EMPTY_LABEL_WIDTH,
                    theMode,
                    LabelPosition_None);
    }
  }

  // flyouts
  if (theMode == ComputeMode_All)
  {
    thePresentation->NewGroup();

    Handle(Graphic3d_ArrayOfSegments) aPrimSegments = new Graphic3d_ArrayOfSegments(4);
    aPrimSegments->AddVertex(myCenterPoint);
    aPrimSegments->AddVertex(aFirstAttach);
    aPrimSegments->AddVertex(myCenterPoint);
    aPrimSegments->AddVertex(aSecondAttach);

    Handle(Graphic3d_AspectLine3d) aFlyoutStyle =
      myDrawer->DimensionAspect()->LineAspect()->Aspect();
    thePresentation->CurrentGroup()->SetPrimitivesAspect(aFlyoutStyle);
    thePresentation->CurrentGroup()->AddPrimitiveArray(aPrimSegments);
  }

  mySelectionGeom.IsComputed = Standard_True;
}

//=======================================================================
// function : ComputeFlyoutSelection
// purpose  : computes selection for flyouts
//=======================================================================
void PrsDim_AngleDimension::ComputeFlyoutSelection(const Handle(SelectionContainer)& theSelection,
                                                   const Handle(SelectMgr_EntityOwner)& theOwner)
{
  Point3d aFirstAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, myFirstPoint).Normalized() * GetFlyout());
  Point3d aSecondAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, mySecondPoint).Normalized() * GetFlyout());

  Handle(Select3D_SensitiveGroup) aSensitiveEntity = new Select3D_SensitiveGroup(theOwner);
  aSensitiveEntity->Add(new Select3D_SensitiveSegment(theOwner, myCenterPoint, aFirstAttach));
  aSensitiveEntity->Add(new Select3D_SensitiveSegment(theOwner, myCenterPoint, aSecondAttach));

  theSelection->Add(aSensitiveEntity);
}

//=================================================================================================

Standard_Boolean PrsDim_AngleDimension::InitTwoEdgesAngle(gp_Pln& theComputedPlane)
{
  TopoEdge aFirstEdge  = TopoDS::Edge(myFirstShape);
  TopoEdge aSecondEdge = TopoDS::Edge(mySecondShape);

  BRepAdaptor_Curve aMakeFirstLine(aFirstEdge);
  BRepAdaptor_Curve aMakeSecondLine(aSecondEdge);

  if (aMakeFirstLine.GetType() != GeomAbs_Line || aMakeSecondLine.GetType() != GeomAbs_Line)
  {
    return Standard_False;
  }

  Handle(GeomLine) aFirstLine  = new GeomLine(aMakeFirstLine.Line());
  Handle(GeomLine) aSecondLine = new GeomLine(aMakeSecondLine.Line());

  gp_Lin aFirstLin  = aFirstLine->Lin();
  gp_Lin aSecondLin = aSecondLine->Lin();

  Standard_Boolean isParallelLines =
    aFirstLin.Direction().IsParallel(aSecondLin.Direction(), Precision1::Angular());

  theComputedPlane = isParallelLines
                       ? gp_Pln(gp1::XOY())
                       : gp_Pln(aSecondLin.Location(),
                                Vector3d(aFirstLin.Direction()) ^ Vector3d(aSecondLin.Direction()));

  // Compute geometry for this plane and edges
  Standard_Boolean   isInfinite1, isInfinite2;
  Point3d             aFirstPoint1, aLastPoint1, aFirstPoint2, aLastPoint2;
  Handle(GeomCurve3d) aFirstCurve = aFirstLine, aSecondCurve = aSecondLine;
  if (!PrsDim1::ComputeGeometry(aFirstEdge,
                               aSecondEdge,
                               aFirstCurve,
                               aSecondCurve,
                               aFirstPoint1,
                               aLastPoint1,
                               aFirstPoint2,
                               aLastPoint2,
                               isInfinite1,
                               isInfinite2))
  {
    return Standard_False;
  }

  Standard_Boolean isSameLines =
    aFirstLin.Direction().IsEqual(aSecondLin.Direction(), Precision1::Angular())
    && aFirstLin.Location().IsEqual(aSecondLin.Location(), Precision1::Confusion());

  // It can be the same gp_Lin geometry but the different begin and end parameters
  Standard_Boolean isSameEdges = (aFirstPoint1.IsEqual(aFirstPoint2, Precision1::Confusion())
                                  && aLastPoint1.IsEqual(aLastPoint2, Precision1::Confusion()))
                                 || (aFirstPoint1.IsEqual(aLastPoint2, Precision1::Confusion())
                                     && aLastPoint1.IsEqual(aFirstPoint2, Precision1::Confusion()));

  if (isParallelLines)
  {
    // Zero angle, it could not handle this geometry
    if (isSameLines && isSameEdges)
    {
      return Standard_False;
    }

    // Handle the case of Pi angle
    const Standard_Real aParam11 = ElCLib1::Parameter(aFirstLin, aFirstPoint1);
    const Standard_Real aParam12 = ElCLib1::Parameter(aFirstLin, aLastPoint1);
    const Standard_Real aParam21 = ElCLib1::Parameter(aFirstLin, aFirstPoint2);
    const Standard_Real aParam22 = ElCLib1::Parameter(aFirstLin, aLastPoint2);
    myCenterPoint =
      ElCLib1::Value((Min(aParam11, aParam12) + Max(aParam21, aParam22)) * 0.5, aFirstLin);
    myFirstPoint  = myCenterPoint.Translated(Vector3d(aFirstLin.Direction()) * Abs(GetFlyout()));
    mySecondPoint = myCenterPoint.XYZ()
                    + (aFirstLin.Direction().IsEqual(aSecondLin.Direction(), Precision1::Angular())
                         ? aFirstLin.Direction().Reversed().XYZ() * Abs(GetFlyout())
                         : aSecondLin.Direction().XYZ() * Abs(GetFlyout()));
  }
  else
  {
    // Find intersection
    gp_Lin2d aFirstLin2d  = ProjLib1::Project(theComputedPlane, aFirstLin);
    gp_Lin2d aSecondLin2d = ProjLib1::Project(theComputedPlane, aSecondLin);

    AnalyticIntersection2d anInt2d(aFirstLin2d, aSecondLin2d);
    gp_Pnt2d                 anIntersectPoint;
    if (!anInt2d.IsDone() || anInt2d.IsEmpty())
    {
      return Standard_False;
    }

    anIntersectPoint = gp_Pnt2d(anInt2d.Point(1).Value());
    myCenterPoint    = ElCLib1::To3d(theComputedPlane.Position1().Ax2(), anIntersectPoint);

    if (isInfinite1 || isInfinite2)
    {
      myFirstPoint  = myCenterPoint.Translated(Vector3d(aFirstLin.Direction()) * Abs(GetFlyout()));
      mySecondPoint = myCenterPoint.Translated(Vector3d(aSecondLin.Direction()) * Abs(GetFlyout()));

      return IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);
    }

    // |
    // | <- dimension should be here
    // *----
    myFirstPoint = myCenterPoint.Distance(aFirstPoint1) > myCenterPoint.Distance(aLastPoint1)
                     ? aFirstPoint1
                     : aLastPoint1;

    mySecondPoint = myCenterPoint.Distance(aFirstPoint2) > myCenterPoint.Distance(aLastPoint2)
                      ? aFirstPoint2
                      : aLastPoint2;
  }

  return IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);
}

//=======================================================================
// function : InitTwoFacesAngle
// purpose  : initialization of angle dimension between two faces
//=======================================================================
Standard_Boolean PrsDim_AngleDimension::InitTwoFacesAngle()
{
  TopoFace aFirstFace  = TopoDS::Face(myFirstShape);
  TopoFace aSecondFace = TopoDS::Face(mySecondShape);

  Dir3d               aFirstDir, aSecondDir;
  gp_Pln               aFirstPln, aSecondPln;
  Handle(GeomSurface) aFirstBasisSurf, aSecondBasisSurf;
  PrsDim_KindOfSurface aFirstSurfType, aSecondSurfType;
  Standard_Real        aFirstOffset, aSecondOffset;

  PrsDim1::GetPlaneFromFace(aFirstFace, aFirstPln, aFirstBasisSurf, aFirstSurfType, aFirstOffset);

  PrsDim1::GetPlaneFromFace(aSecondFace,
                           aSecondPln,
                           aSecondBasisSurf,
                           aSecondSurfType,
                           aSecondOffset);

  if (aFirstSurfType == PrsDim_KOS_Plane && aSecondSurfType == PrsDim_KOS_Plane)
  {
    // Planar faces angle
    Handle(GeomPlane) aFirstPlane  = Handle(GeomPlane)::DownCast(aFirstBasisSurf);
    Handle(GeomPlane) aSecondPlane = Handle(GeomPlane)::DownCast(aSecondBasisSurf);
    return PrsDim1::InitAngleBetweenPlanarFaces(aFirstFace,
                                               aSecondFace,
                                               myCenterPoint,
                                               myFirstPoint,
                                               mySecondPoint)
           && IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);
  }
  else
  {
    // Curvilinear faces angle
    return PrsDim1::InitAngleBetweenCurvilinearFaces(aFirstFace,
                                                    aSecondFace,
                                                    aFirstSurfType,
                                                    aSecondSurfType,
                                                    myCenterPoint,
                                                    myFirstPoint,
                                                    mySecondPoint)
           && IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);
  }
}

//=======================================================================
// function : InitTwoFacesAngle
// purpose  : initialization of angle dimension between two faces
//=======================================================================
Standard_Boolean PrsDim_AngleDimension::InitTwoFacesAngle(const Point3d& thePointOnFirstFace)
{
  TopoFace aFirstFace  = TopoDS::Face(myFirstShape);
  TopoFace aSecondFace = TopoDS::Face(mySecondShape);

  Dir3d               aFirstDir, aSecondDir;
  gp_Pln               aFirstPln, aSecondPln;
  Handle(GeomSurface) aFirstBasisSurf, aSecondBasisSurf;
  PrsDim_KindOfSurface aFirstSurfType, aSecondSurfType;
  Standard_Real        aFirstOffset, aSecondOffset;

  PrsDim1::GetPlaneFromFace(aFirstFace, aFirstPln, aFirstBasisSurf, aFirstSurfType, aFirstOffset);

  PrsDim1::GetPlaneFromFace(aSecondFace,
                           aSecondPln,
                           aSecondBasisSurf,
                           aSecondSurfType,
                           aSecondOffset);

  myFirstPoint = thePointOnFirstFace;
  if (aFirstSurfType == PrsDim_KOS_Plane && aSecondSurfType == PrsDim_KOS_Plane)
  {
    // Planar faces angle
    Handle(GeomPlane) aFirstPlane  = Handle(GeomPlane)::DownCast(aFirstBasisSurf);
    Handle(GeomPlane) aSecondPlane = Handle(GeomPlane)::DownCast(aSecondBasisSurf);
    return PrsDim1::InitAngleBetweenPlanarFaces(aFirstFace,
                                               aSecondFace,
                                               myCenterPoint,
                                               myFirstPoint,
                                               mySecondPoint,
                                               Standard_True)
           && IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);
  }
  else
  {
    // Curvilinear faces angle
    return PrsDim1::InitAngleBetweenCurvilinearFaces(aFirstFace,
                                                    aSecondFace,
                                                    aFirstSurfType,
                                                    aSecondSurfType,
                                                    myCenterPoint,
                                                    myFirstPoint,
                                                    mySecondPoint,
                                                    Standard_True)
           && IsValidPoints(myFirstPoint, myCenterPoint, mySecondPoint);
  }
}

//=======================================================================
// function : InitConeAngle
// purpose  : initialization of the cone angle
//=======================================================================
Standard_Boolean PrsDim_AngleDimension::InitConeAngle()
{
  if (myFirstShape.IsNull())
  {
    return Standard_False;
  }

  TopoFace aConeShape = TopoDS::Face(myFirstShape);
  gp_Pln      aPln;
  Cone1     aCone;
  gp_Circ     aCircle;
  // A surface from the Face
  Handle(GeomSurface)             aSurf;
  Handle(Geom_OffsetSurface)       aOffsetSurf;
  Handle(Geom_ConicalSurface)      aConicalSurf;
  Handle(Geom_SurfaceOfRevolution) aRevSurf;
  Handle(GeomLine)                aLine;
  BRepAdaptor_Surface              aConeAdaptor(aConeShape);
  TopoFace                      aFace;
  PrsDim_KindOfSurface             aSurfType;
  Standard_Real                    anOffset = 0.;
  Handle(TypeInfo)            aType;

  const Standard_Real aMaxV = aConeAdaptor.FirstVParameter();
  const Standard_Real aMinV = aConeAdaptor.LastVParameter();
  PrsDim1::GetPlaneFromFace(aConeShape, aPln, aSurf, aSurfType, anOffset);
  if (aSurfType == PrsDim_KOS_Revolution)
  {
    // Surface of revolution
    aRevSurf = Handle(Geom_SurfaceOfRevolution)::DownCast(aSurf);
    gp_Lin             aLin(aRevSurf->Axis());
    Handle(GeomCurve3d) aBasisCurve = aRevSurf->BasisCurve();
    // Must be a part of line (basis curve should be linear)
    if (aBasisCurve->DynamicType() != STANDARD_TYPE(GeomLine))
      return Standard_False;

    Point3d aFirst1 = aConeAdaptor.Value(0., aMinV);
    Point3d aLast1  = aConeAdaptor.Value(0., aMaxV);
    Vector3d aVec1(aFirst1, aLast1);

    // Projection <aFirst> on <aLin>
    Point3d aFirst2 = ElCLib1::Value(ElCLib1::Parameter(aLin, aFirst1), aLin);
    // Projection <aLast> on <aLin>
    Point3d aLast2 = ElCLib1::Value(ElCLib1::Parameter(aLin, aLast1), aLin);

    Vector3d aVec2(aFirst2, aLast2);

    // Check if two parts of revolution are parallel (it's a cylinder) or normal (it's a circle).
    if (aVec1.IsParallel(aVec2, Precision1::Angular())
        || aVec1.IsNormal(aVec2, Precision1::Angular()))
      return Standard_False;

    gce_MakeCone aMkCone(aRevSurf->Axis(), aFirst1, aLast1);
    aCone         = aMkCone.Value();
    myCenterPoint = aCone.Apex();
  }
  else
  {
    aType = aSurf->DynamicType();
    if (aType == STANDARD_TYPE(Geom_OffsetSurface) || anOffset > 0.01)
    {
      // Offset surface
      aOffsetSurf = new Geom_OffsetSurface(aSurf, anOffset);
      aSurf       = aOffsetSurf->Surface();
      FaceMaker aMkFace(aSurf, Precision1::Confusion());
      aMkFace.Build();
      if (!aMkFace.IsDone())
        return Standard_False;
      aConeAdaptor.Initialize(aMkFace.Face());
    }
    aCone         = aConeAdaptor.Cone();
    aConicalSurf  = Handle(Geom_ConicalSurface)::DownCast(aSurf);
    myCenterPoint = aConicalSurf->Apex();
  }

  // A circle where the angle is drawn
  Handle(GeomCurve3d) aCurve;
  Standard_Real      aMidV = (aMinV + aMaxV) / 2.5;
  aCurve                   = aSurf->VIso(aMidV);
  aCircle                  = Handle(GeomCircle)::DownCast(aCurve)->Circ();

  aCurve            = aSurf->VIso(aMaxV);
  gp_Circ aCircVmax = Handle(GeomCircle)::DownCast(aCurve)->Circ();
  aCurve            = aSurf->VIso(aMinV);
  gp_Circ aCircVmin = Handle(GeomCircle)::DownCast(aCurve)->Circ();

  if (aCircVmax.Radius() < aCircVmin.Radius())
  {
    gp_Circ aTmpCirc = aCircVmax;
    aCircVmax        = aCircVmin;
    aCircVmin        = aTmpCirc;
  }

  myFirstPoint  = ElCLib1::Value(0, aCircle);
  mySecondPoint = ElCLib1::Value(M_PI, aCircle);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean PrsDim_AngleDimension::IsValidPoints(const Point3d& theFirstPoint,
                                                      const Point3d& theCenterPoint,
                                                      const Point3d& theSecondPoint) const
{
  return theFirstPoint.Distance(theCenterPoint) > Precision1::Confusion()
         && theSecondPoint.Distance(theCenterPoint) > Precision1::Confusion()
         && Vector3d(theCenterPoint, theFirstPoint).Angle(Vector3d(theCenterPoint, theSecondPoint))
              > Precision1::Angular();
}

//=======================================================================
// function : isArrowVisible
// purpose  : compares given and internal arrows types, returns true if the type should be shown
//=======================================================================
Standard_Boolean PrsDim_AngleDimension::isArrowVisible(
  const PrsDim_TypeOfAngleArrowVisibility theArrowType) const
{
  switch (theArrowType)
  {
    case PrsDim_TypeOfAngleArrowVisibility_Both:
      return myArrowsVisibility == PrsDim_TypeOfAngleArrowVisibility_Both;
    case PrsDim_TypeOfAngleArrowVisibility_First:
      return myArrowsVisibility == PrsDim_TypeOfAngleArrowVisibility_Both
             || myArrowsVisibility == PrsDim_TypeOfAngleArrowVisibility_First;
    case PrsDim_TypeOfAngleArrowVisibility_Second:
      return myArrowsVisibility == PrsDim_TypeOfAngleArrowVisibility_Both
             || myArrowsVisibility == PrsDim_TypeOfAngleArrowVisibility_Second;
    case PrsDim_TypeOfAngleArrowVisibility_None:
      return false;
  }
  return false;
}

//=================================================================================================

Point3d PrsDim_AngleDimension::GetTextPosition() const
{
  if (!IsValid())
  {
    return gp1::Origin();
  }

  if (IsTextPositionCustom())
  {
    return myFixedTextPosition;
  }

  // Counts text position according to the dimension parameters
  Point3d aTextPosition(gp1::Origin());

  Handle(Prs3d_DimensionAspect) aDimensionAspect = myDrawer->DimensionAspect();

  // Prepare label string and compute its geometrical width
  Standard_Real              aLabelWidth;
  UtfString aLabelString = GetValueString(aLabelWidth);

  Point3d aFirstAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, myFirstPoint).Normalized() * GetFlyout());
  Point3d aSecondAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, mySecondPoint).Normalized() * GetFlyout());

  // Handle user-defined and automatic arrow placement
  Standard_Boolean isArrowsExternal = Standard_False;
  Standard_Integer aLabelPosition   = LabelPosition_None;
  FitTextAlignment(aDimensionAspect->TextHorizontalPosition(), aLabelPosition, isArrowsExternal);

  // Get text position
  switch (aLabelPosition & LabelPosition_HMask)
  {
    case LabelPosition_HCenter: {
      aTextPosition = GetCenterOnArc(aFirstAttach, aSecondAttach, myCenterPoint);
    }
    break;
    case LabelPosition_Left: {
      Dir3d aPlaneNormal =
        Vector3d(aFirstAttach, aSecondAttach) ^ Vector3d(myCenterPoint, aFirstAttach);
      Dir3d        anExtensionDir  = aPlaneNormal ^ Vector3d(myCenterPoint, aFirstAttach);
      Standard_Real anExtensionSize = aDimensionAspect->ExtensionSize();
      Standard_Real anOffset        = isArrowsExternal
                                        ? anExtensionSize + aDimensionAspect->ArrowAspect()->Length()
                                        : anExtensionSize;
      Vector3d        anExtensionVec  = Vector3d(anExtensionDir) * -anOffset;
      aTextPosition                 = aFirstAttach.Translated(anExtensionVec);
    }
    break;
    case LabelPosition_Right: {
      Dir3d aPlaneNormal =
        Vector3d(aFirstAttach, aSecondAttach) ^ Vector3d(myCenterPoint, aFirstAttach);
      Dir3d        anExtensionDir  = aPlaneNormal ^ Vector3d(myCenterPoint, aSecondAttach);
      Standard_Real anExtensionSize = aDimensionAspect->ExtensionSize();
      Standard_Real anOffset        = isArrowsExternal
                                        ? anExtensionSize + aDimensionAspect->ArrowAspect()->Length()
                                        : anExtensionSize;
      Vector3d        anExtensionVec  = Vector3d(anExtensionDir) * anOffset;
      aTextPosition                 = aSecondAttach.Translated(anExtensionVec);
    }
    break;
  }

  return aTextPosition;
}

//=================================================================================================

void PrsDim_AngleDimension::SetTextPosition(const Point3d& theTextPos)
{
  if (!IsValid())
  {
    return;
  }

  // The text position point for angle dimension should belong to the working plane.
  if (!GetPlane().Contains(theTextPos, Precision1::Confusion()))
  {
    throw Standard_ProgramError(
      "The text position point for angle dimension doesn't belong to the working plane.");
  }

  myIsTextPositionFixed = Standard_True;
  myFixedTextPosition   = theTextPos;
}

//=================================================================================================

void PrsDim_AngleDimension::AdjustParameters(const Point3d&  theTextPos,
                                             Standard_Real& theExtensionSize,
                                             Prs3d_DimensionTextHorizontalPosition& theAlignment,
                                             Standard_Real&                         theFlyout) const
{
  Handle(Prs3d_DimensionAspect) aDimensionAspect = myDrawer->DimensionAspect();
  Standard_Real                 anArrowLength    = aDimensionAspect->ArrowAspect()->Length();

  // Build circle with radius that is equal to distance from text position to the center point.
  Standard_Real aRadius = Vector3d(myCenterPoint, theTextPos).Magnitude();

  // Set attach points in positive direction of the flyout.
  Point3d aFirstAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, myFirstPoint).Normalized() * aRadius);
  Point3d aSecondAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, mySecondPoint).Normalized() * aRadius);

  gce_MakeCirc aConstructCircle(myCenterPoint, GetPlane(), aRadius);
  if (!aConstructCircle.IsDone())
  {
    return;
  }
  gp_Circ aCircle = aConstructCircle.Value();

  // Default values
  theExtensionSize = aDimensionAspect->ArrowAspect()->Length();
  theAlignment     = Prs3d_DTHP_Center;

  Standard_Real aParamBeg = ElCLib1::Parameter(aCircle, aFirstAttach);
  Standard_Real aParamEnd = ElCLib1::Parameter(aCircle, aSecondAttach);
  if (aParamEnd < aParamBeg)
  {
    Standard_Real aParam = aParamEnd;
    aParamEnd            = aParamBeg;
    aParamBeg            = aParam;
  }

  ElCLib1::AdjustPeriodic(0.0, M_PI * 2, Precision1::PConfusion(), aParamBeg, aParamEnd);
  Standard_Real aTextPar = ElCLib1::Parameter(aCircle, theTextPos);

  // Horizontal center
  if (aTextPar > aParamBeg && aTextPar < aParamEnd)
  {
    theFlyout = aRadius;
    return;
  }

  aParamBeg += M_PI;
  aParamEnd += M_PI;
  ElCLib1::AdjustPeriodic(0.0, M_PI * 2, Precision1::PConfusion(), aParamBeg, aParamEnd);

  if (aTextPar > aParamBeg && aTextPar < aParamEnd)
  {
    theFlyout = -aRadius;
    return;
  }

  // Text on the extensions
  gp_Lin        aFirstLine      = gce_MakeLin(myCenterPoint, myFirstPoint);
  gp_Lin        aSecondLine     = gce_MakeLin(myCenterPoint, mySecondPoint);
  Point3d        aFirstTextProj  = PrsDim1::Nearest(aFirstLine, theTextPos);
  Point3d        aSecondTextProj = PrsDim1::Nearest(aSecondLine, theTextPos);
  Standard_Real aFirstDist      = aFirstTextProj.Distance(theTextPos);
  Standard_Real aSecondDist     = aSecondTextProj.Distance(theTextPos);

  if (aFirstDist <= aSecondDist)
  {
    aRadius                         = myCenterPoint.Distance(aFirstTextProj);
    Standard_Real aNewExtensionSize = aFirstDist - anArrowLength;
    theExtensionSize                = aNewExtensionSize < 0.0 ? 0.0 : aNewExtensionSize;

    theAlignment = Prs3d_DTHP_Left;

    Vector3d aPosFlyoutDir = Vector3d(myCenterPoint, myFirstPoint).Normalized().Scaled(aRadius);

    theFlyout =
      aFirstTextProj.Distance(myCenterPoint.Translated(aPosFlyoutDir)) > Precision1::Confusion()
        ? -aRadius
        : aRadius;
  }
  else
  {
    aRadius = myCenterPoint.Distance(aSecondTextProj);

    Standard_Real aNewExtensionSize = aSecondDist - anArrowLength;

    theExtensionSize = aNewExtensionSize < 0.0 ? 0.0 : aNewExtensionSize;

    theAlignment = Prs3d_DTHP_Right;

    Vector3d aPosFlyoutDir = Vector3d(myCenterPoint, mySecondPoint).Normalized().Scaled(aRadius);

    theFlyout =
      aSecondTextProj.Distance(myCenterPoint.Translated(aPosFlyoutDir)) > Precision1::Confusion()
        ? -aRadius
        : aRadius;
  }
}

//=================================================================================================

void PrsDim_AngleDimension::FitTextAlignment(
  const Prs3d_DimensionTextHorizontalPosition& theHorizontalTextPos,
  Standard_Integer&                            theLabelPosition,
  Standard_Boolean&                            theIsArrowsExternal) const
{
  Handle(Prs3d_DimensionAspect) aDimensionAspect = myDrawer->DimensionAspect();

  Standard_Real anArrowLength = aDimensionAspect->ArrowAspect()->Length();

  // Prepare label string and compute its geometrical width
  Standard_Real              aLabelWidth;
  UtfString aLabelString = GetValueString(aLabelWidth);

  // add margins to label width
  if (aDimensionAspect->IsText3d())
  {
    aLabelWidth += aDimensionAspect->TextAspect()->Height() * THE_3D_TEXT_MARGIN * 2.0;
  }

  Point3d aFirstAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, myFirstPoint).Normalized() * GetFlyout());
  Point3d aSecondAttach =
    myCenterPoint.Translated(Vector3d(myCenterPoint, mySecondPoint).Normalized() * GetFlyout());

  // Handle user-defined and automatic arrow placement
  switch (aDimensionAspect->ArrowOrientation())
  {
    case Prs3d_DAO_External:
      theIsArrowsExternal = true;
      break;
    case Prs3d_DAO_Internal:
      theIsArrowsExternal = false;
      break;
    case Prs3d_DAO_Fit: {
      Vector3d        anAttachVector(aFirstAttach, aSecondAttach);
      Standard_Real aDimensionWidth = anAttachVector.Magnitude();

      // Add margin to ensure a small tail between text and arrow
      Standard_Real anArrowMargin =
        aDimensionAspect->IsText3d() ? aDimensionAspect->TextAspect()->Height() * THE_3D_TEXT_MARGIN
                                     : 0.0;

      Standard_Real anArrowsWidth = (anArrowLength + anArrowMargin) * 2.0;

      theIsArrowsExternal = aDimensionWidth < aLabelWidth + anArrowsWidth;
      break;
    }
  }

  // Handle user-defined and automatic text placement
  switch (theHorizontalTextPos)
  {
    case Prs3d_DTHP_Left:
      theLabelPosition |= LabelPosition_Left;
      break;
    case Prs3d_DTHP_Right:
      theLabelPosition |= LabelPosition_Right;
      break;
    case Prs3d_DTHP_Center:
      theLabelPosition |= LabelPosition_HCenter;
      break;
    case Prs3d_DTHP_Fit: {
      Vector3d        anAttachVector(aFirstAttach, aSecondAttach);
      Standard_Real aDimensionWidth = anAttachVector.Magnitude();
      Standard_Real anArrowsWidth   = anArrowLength * 2.0;
      Standard_Real aContentWidth = theIsArrowsExternal ? aLabelWidth : aLabelWidth + anArrowsWidth;

      theLabelPosition |=
        aDimensionWidth < aContentWidth ? LabelPosition_Left : LabelPosition_HCenter;
      break;
    }
  }

  switch (aDimensionAspect->TextVerticalPosition())
  {
    case Prs3d_DTVP_Above:
      theLabelPosition |= LabelPosition_Above;
      break;
    case Prs3d_DTVP_Below:
      theLabelPosition |= LabelPosition_Below;
      break;
    case Prs3d_DTVP_Center:
      theLabelPosition |= LabelPosition_VCenter;
      break;
  }
}
