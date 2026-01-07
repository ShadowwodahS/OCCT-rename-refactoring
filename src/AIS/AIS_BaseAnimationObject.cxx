// Copyright (c) 2023 OPEN CASCADE SAS
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

#include <AIS_BaseAnimationObject.hxx>

#include <V3d_View.hxx>

IMPLEMENT_STANDARD_RTTIEXT(AIS_BaseAnimationObject, AIS_Animation)

//=================================================================================================

AIS_BaseAnimationObject::AIS_BaseAnimationObject(const AsciiString1& theAnimationName,
                                                 const Handle(VisualContext)& theContext,
                                                 const Handle(VisualEntity)&  theObject)
    : AIS_Animation(theAnimationName),
      myContext(theContext),
      myObject(theObject)
{
  //
}

//=================================================================================================

void AIS_BaseAnimationObject::updateTrsf(const Transform3d& theTrsf)
{
  if (!myContext.IsNull())
  {
    myContext->SetLocation(myObject, theTrsf);
    invalidateViewer();
  }
  else
  {
    myObject->SetLocalTransformation(theTrsf);
  }
}

//=================================================================================================

void AIS_BaseAnimationObject::invalidateViewer()
{
  if (myContext.IsNull())
  {
    return;
  }

  const Standard_Boolean isImmediate =
    myContext->CurrentViewer()->ZLayerSettings(myObject->ZLayer()).IsImmediate();
  if (!isImmediate)
  {
    myContext->CurrentViewer()->Invalidate();
    return;
  }

  // Invalidate immediate view only if it is going out of z-fit range.
  // This might be sub-optimal performing this for each animated objects in case of many animated
  // objects.
  for (V3d_ListOfView::Iterator aDefViewIter = myContext->CurrentViewer()->DefinedViewIterator();
       aDefViewIter.More();
       aDefViewIter.Next())
  {
    const Handle(ViewWindow)& aView       = aDefViewIter.Value();
    const Box2           aMinMaxBox  = aView->View()->MinMaxValues(Standard_False);
    const Box2           aGraphicBox = aView->View()->MinMaxValues(Standard_True);
    Standard_Real           aZNear      = 0.0;
    Standard_Real           aZFar       = 0.0;
    if (aView->Camera()->ZFitAll(aDefViewIter.Value()->AutoZFitScaleFactor(),
                                 aMinMaxBox,
                                 aGraphicBox,
                                 aZNear,
                                 aZFar))
    {
      if (aZNear < aView->Camera()->ZNear() || aZFar > aView->Camera()->ZFar())
      {
        aDefViewIter.Value()->Invalidate();
      }
    }
  }
}
