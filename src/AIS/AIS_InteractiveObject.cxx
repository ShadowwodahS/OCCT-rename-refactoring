// Created on: 1996-12-18
// Created by: Robert COUBLANC
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

#include <AIS_InteractiveObject.hxx>

#include <AIS_InteractiveContext.hxx>
#include <Graphic3d_CStructure.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_Structure.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_TextAspect.hxx>
#include <PrsMgr_PresentationManager.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VisualEntity, SelectMgr_SelectableObject)

//=================================================================================================

VisualEntity::VisualEntity(
  const PrsMgr_TypeOfPresentation3d aTypeOfPresentation3d)
    : SelectMgr_SelectableObject(aTypeOfPresentation3d),
      myCTXPtr(NULL)
{
  //
}

//=================================================================================================

void VisualEntity::Redisplay(const Standard_Boolean AllModes)
{
  if (myCTXPtr == NULL)
    return;

  myCTXPtr->Redisplay(this, Standard_False, AllModes);
}

//=================================================================================================

Standard_Boolean VisualEntity::ProcessDragging(const Handle(VisualContext)&,
                                                        const Handle(ViewWindow)&,
                                                        const Handle(SelectMgr_EntityOwner)&,
                                                        const Graphic3d_Vec2i&,
                                                        const Graphic3d_Vec2i&,
                                                        const AIS_DragAction)
{
  return Standard_False;
}

//=================================================================================================

Handle(VisualContext) VisualEntity::GetContext() const
{
  return myCTXPtr;
}

//=================================================================================================

void VisualEntity::SetContext(const Handle(VisualContext)& theCtx)
{
  if (myCTXPtr == theCtx.get())
  {
    return;
  }

  myCTXPtr = theCtx.get();
  if (!theCtx.IsNull())
  {
    myDrawer->Link1(theCtx->DefaultDrawer());
  }
}

//=================================================================================================

void VisualEntity::SetDisplayStatus(PrsMgr_DisplayStatus theStatus)
{
  myDisplayStatus = theStatus;
}

//=================================================================================================

Standard_Boolean VisualEntity::HasPresentation() const
{
  return HasInteractiveContext()
         && myCTXPtr->MainPrsMgr()->HasPresentation(this, myDrawer->DisplayMode());
}

//=================================================================================================

Handle(Prs3d_Presentation) VisualEntity::Presentation() const
{
  if (!HasInteractiveContext())
  {
    return Handle(Prs3d_Presentation)();
  }

  Handle(PrsMgr_Presentation) aPrs =
    myCTXPtr->MainPrsMgr()->Presentation(this, myDrawer->DisplayMode(), false);
  return aPrs;
}

//=================================================================================================

void VisualEntity::SetAspect(const Handle(Prs3d_BasicAspect)& theAspect)
{

  if (!HasPresentation())
  {
    return;
  }

  Handle(Prs3d_Presentation) aPrs = Presentation();
  if (aPrs->Groups().IsEmpty())
  {
    return;
  }
  const Handle(Graphic3d_Group)& aGroup = aPrs->Groups().Last();
  if (Handle(Prs3d_ShadingAspect) aShadingAspect = Handle(Prs3d_ShadingAspect)::DownCast(theAspect))
  {
    aGroup->SetGroupPrimitivesAspect(aShadingAspect->Aspect());
  }
  else if (Handle(Prs3d_LineAspect) aLineAspect = Handle(Prs3d_LineAspect)::DownCast(theAspect))
  {
    aGroup->SetGroupPrimitivesAspect(aLineAspect->Aspect());
  }
  else if (Handle(Prs3d_PointAspect) aPointAspect = Handle(Prs3d_PointAspect)::DownCast(theAspect))
  {
    aGroup->SetGroupPrimitivesAspect(aPointAspect->Aspect());
  }
  else if (Handle(Prs3d_TextAspect) aTextAspect = Handle(Prs3d_TextAspect)::DownCast(theAspect))
  {
    aGroup->SetGroupPrimitivesAspect(aTextAspect->Aspect());
  }
}

//=================================================================================================

void VisualEntity::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, SelectMgr_SelectableObject)
  OCCT_DUMP_FIELD_VALUE_POINTER(theOStream, myCTXPtr)
  OCCT_DUMP_FIELD_VALUE_POINTER(theOStream, myOwner)
}
