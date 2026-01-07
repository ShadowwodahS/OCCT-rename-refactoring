// Created on: 1995-08-09
// Created by: Arnaud BOUZY/Odile Olivier
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

#include <AIS_Point.hxx>

#include <AIS_InteractiveContext.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_Structure.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>
#include <Quantity_Color.hxx>
#include <Select3D_SensitivePoint.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Standard_Type.hxx>
#include <StdPrs_Point.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VisualPoint, VisualEntity)

//=================================================================================================

VisualPoint::VisualPoint(const Handle(Geom_Point)& aComponent)
    : myComponent(aComponent),
      myHasTOM(Standard_False),
      myTOM(Aspect_TOM_PLUS)
{
  myHilightDrawer = new StyleDrawer();
  myHilightDrawer->SetDisplayMode(-99);
  myHilightDrawer->SetPointAspect(new Prs3d_PointAspect(Aspect_TOM_PLUS, Quantity_NOC_GRAY80, 3.0));
  myHilightDrawer->SetColor(Quantity_NOC_GRAY80);
  myHilightDrawer->SetZLayer(Graphic3d_ZLayerId_UNKNOWN);
  myDynHilightDrawer = new StyleDrawer();
  myDynHilightDrawer->SetDisplayMode(-99);
  myDynHilightDrawer->SetPointAspect(
    new Prs3d_PointAspect(Aspect_TOM_PLUS, Quantity_NOC_CYAN1, 3.0));
  myDynHilightDrawer->SetColor(Quantity_NOC_CYAN1);
  myDynHilightDrawer->SetZLayer(Graphic3d_ZLayerId_Top);
}

//=================================================================================================

Handle(Geom_Point) VisualPoint::Component()
{
  return myComponent;
}

//=================================================================================================

void VisualPoint::SetComponent(const Handle(Geom_Point)& aComponent)
{
  myComponent = aComponent;
}

//=================================================================================================

void VisualPoint::Compute(const Handle(PrsMgr_PresentationManager)&,
                        const Handle(Prs3d_Presentation)& thePrs,
                        const Standard_Integer            theMode)
{
  thePrs->SetInfiniteState(myInfiniteState);
  if (theMode == 0)
  {
    StdPrs_Point::Add(thePrs, myComponent, myDrawer);
  }
  else if (theMode == -99)
  {
    Handle(Graphic3d_Group) aGroup = thePrs->CurrentGroup();
    aGroup->SetPrimitivesAspect(myHilightDrawer->PointAspect()->Aspect());
    Handle(Graphic3d_ArrayOfPoints) aPoint = new Graphic3d_ArrayOfPoints(1);
    aPoint->AddVertex(myComponent->X(), myComponent->Y(), myComponent->Z());
    aGroup->AddPrimitiveArray(aPoint);
  }
}

//=================================================================================================

void VisualPoint::ComputeSelection(const Handle(SelectionContainer)& aSelection,
                                 const Standard_Integer /*aMode*/)
{
  Handle(SelectMgr_EntityOwner)   eown = new SelectMgr_EntityOwner(this, 10);
  Handle(Select3D_SensitivePoint) sp   = new Select3D_SensitivePoint(eown, myComponent->Pnt());
  aSelection->Add(sp);
}

//=================================================================================================

void VisualPoint::SetColor(const Quantity_Color& theCol)
{
  hasOwnColor = Standard_True;
  myDrawer->SetColor(theCol);
  UpdatePointValues();
}

//=================================================================================================

void VisualPoint::UnsetColor()
{
  hasOwnColor = Standard_False;
  UpdatePointValues();
}

//=================================================================================================

TopoVertex VisualPoint::Vertex() const
{
  Point3d P = myComponent->Pnt();
  return BRepBuilderAPI_MakeVertex(P);
}

//=================================================================================================

void VisualPoint::SetMarker(const Aspect_TypeOfMarker aTOM)
{
  myTOM    = aTOM;
  myHasTOM = Standard_True;
  UpdatePointValues();
}

//=================================================================================================

void VisualPoint::UnsetMarker()
{
  myHasTOM = Standard_False;
  UpdatePointValues();
}

//=================================================================================================

Standard_Boolean VisualPoint::AcceptDisplayMode(const Standard_Integer theMode) const
{
  return theMode == 0 || theMode == -99;
}

//=================================================================================================

void VisualPoint::replaceWithNewPointAspect(const Handle(Prs3d_PointAspect)& theAspect)
{
  if (!myDrawer->HasLink())
  {
    myDrawer->SetPointAspect(theAspect);
    return;
  }

  const Handle(Graphic3d_AspectMarker3d) anAspectOld = myDrawer->PointAspect()->Aspect();
  const Handle(Graphic3d_AspectMarker3d) anAspectNew =
    !theAspect.IsNull() ? theAspect->Aspect() : myDrawer->Link1()->PointAspect()->Aspect();
  if (anAspectNew != anAspectOld)
  {
    myDrawer->SetPointAspect(theAspect);
    Graphic3d_MapOfAspectsToAspects aReplaceMap;
    aReplaceMap.Bind(anAspectOld, anAspectNew);
    replaceAspects(aReplaceMap);
  }
}

//=================================================================================================

void VisualPoint::UpdatePointValues()
{
  if (!hasOwnColor && myOwnWidth == 0.0f && !myHasTOM)
  {
    replaceWithNewPointAspect(Handle(Prs3d_PointAspect)());
    return;
  }

  Quantity_Color      aCol(Quantity_NOC_YELLOW);
  Aspect_TypeOfMarker aTOM   = Aspect_TOM_PLUS;
  Standard_Real       aScale = 1.0;
  if (myDrawer->HasLink())
  {
    aCol   = myDrawer->Link1()->PointAspect()->Aspect()->Color();
    aTOM   = myDrawer->Link1()->PointAspect()->Aspect()->Type();
    aScale = myDrawer->Link1()->PointAspect()->Aspect()->Scale();
  }

  if (hasOwnColor)
    aCol = myDrawer->Color();
  if (myOwnWidth != 0.0f)
    aScale = myOwnWidth;
  if (myHasTOM)
    aTOM = myTOM;

  if (myDrawer->HasOwnPointAspect())
  {
    Handle(Prs3d_PointAspect) PA = myDrawer->PointAspect();
    PA->SetColor(aCol);
    PA->SetTypeOfMarker(aTOM);
    PA->SetScale(aScale);
    SynchronizeAspects();
  }
  else
  {
    replaceWithNewPointAspect(new Prs3d_PointAspect(aTOM, aCol, aScale));
  }
}
