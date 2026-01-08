// Created by: CKY / Contract Toubro-Larsen
// Copyright (c) 1993-1999 Matra Datavision
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

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#include <gp_GTrsf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <gp_XY.hxx>
#include <gp_XYZ.hxx>
#include <IGESData_TransfEntity.hxx>
#include <IGESDraw_PerspectiveView.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESDraw_PerspectiveView, IGESData_ViewKindEntity)

IGESDraw_PerspectiveView::IGESDraw_PerspectiveView() {}

// This class inherits from IGESData_ViewKindEntity

void IGESDraw_PerspectiveView::Init(const Standard_Integer aViewNumber,
                                    const Standard_Real    aScaleFactor,
                                    const Coords3d&          aViewNormalVector,
                                    const Coords3d&          aViewReferencePoint,
                                    const Coords3d&          aCenterOfProjection,
                                    const Coords3d&          aViewUpVector,
                                    const Standard_Real    aViewPlaneDistance,
                                    const Coords2d&           aTopLeft,
                                    const Coords2d&           aBottomRight,
                                    const Standard_Integer aDepthClip,
                                    const Standard_Real    aBackPlaneDistance,
                                    const Standard_Real    aFrontPlaneDistance)
{
  theViewNumber         = aViewNumber;
  theScaleFactor        = aScaleFactor;
  theViewNormalVector   = aViewNormalVector;
  theViewReferencePoint = aViewReferencePoint;
  theCenterOfProjection = aCenterOfProjection;
  theViewUpVector       = aViewUpVector;
  theViewPlaneDistance  = aViewPlaneDistance;
  theTopLeft            = aTopLeft;
  theBottomRight        = aBottomRight;
  theDepthClip          = aDepthClip;
  theBackPlaneDistance  = aBackPlaneDistance;
  theFrontPlaneDistance = aFrontPlaneDistance;
  InitTypeAndForm(410, 1);
}

Standard_Boolean IGESDraw_PerspectiveView::IsSingle() const
{
  return Standard_True;
}

Standard_Integer IGESDraw_PerspectiveView::NbViews() const
{
  return 1;
}

Handle(IGESData_ViewKindEntity) IGESDraw_PerspectiveView::ViewItem(const Standard_Integer) const
{
  return Handle(IGESData_ViewKindEntity)::DownCast(This());
}

Standard_Integer IGESDraw_PerspectiveView::ViewNumber() const
{
  return theViewNumber;
}

Standard_Real IGESDraw_PerspectiveView::ScaleFactor() const
{
  return theScaleFactor;
}

Vector3d IGESDraw_PerspectiveView::ViewNormalVector() const
{
  Vector3d tempRes(theViewNormalVector);
  return tempRes;
}

Point3d IGESDraw_PerspectiveView::ViewReferencePoint() const
{
  Point3d tempRes(theViewReferencePoint);
  return tempRes;
}

Point3d IGESDraw_PerspectiveView::CenterOfProjection() const
{
  Point3d tempRes(theCenterOfProjection);
  return tempRes;
}

Vector3d IGESDraw_PerspectiveView::ViewUpVector() const
{
  Vector3d tempRes(theViewUpVector);
  return tempRes;
}

Standard_Real IGESDraw_PerspectiveView::ViewPlaneDistance() const
{
  return theViewPlaneDistance;
}

gp_Pnt2d IGESDraw_PerspectiveView::TopLeft() const
{
  gp_Pnt2d tempRes(theTopLeft);
  return tempRes;
}

gp_Pnt2d IGESDraw_PerspectiveView::BottomRight() const
{
  gp_Pnt2d tempRes(theBottomRight);
  return tempRes;
}

Standard_Integer IGESDraw_PerspectiveView::DepthClip() const
{
  return theDepthClip;
}

Standard_Real IGESDraw_PerspectiveView::BackPlaneDistance() const
{
  return theBackPlaneDistance;
}

Standard_Real IGESDraw_PerspectiveView::FrontPlaneDistance() const
{
  return theFrontPlaneDistance;
}

Handle(IGESData_TransfEntity) IGESDraw_PerspectiveView::ViewMatrix() const
{
  return (Transf());
}

Coords3d IGESDraw_PerspectiveView::ModelToView(const Coords3d& coords) const
{
  Coords3d tempCoords = coords;
  Location().Transforms(tempCoords);
  return (tempCoords);
}
