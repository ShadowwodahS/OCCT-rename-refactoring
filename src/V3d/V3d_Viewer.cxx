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

#include <V3d_Viewer.hxx>

#include <Aspect_IdentDefinitionError.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_AspectMarker3d.hxx>
#include <Graphic3d_AspectText3d.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_Structure.hxx>
#include <Graphic3d_Text.hxx>
#include <Standard_ErrorHandler.hxx>
#include <V3d.hxx>
#include <V3d_BadValue.hxx>
#include <V3d_CircularGrid.hxx>
#include <V3d_AmbientLight.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_RectangularGrid.hxx>
#include <V3d_View.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ViewManager, RefObject)

//=================================================================================================

ViewManager::ViewManager(const Handle(Graphic3d_GraphicDriver)& theDriver)
    : myDriver(theDriver),
      myStructureManager(new Graphic3d_StructureManager(theDriver)),
      myZLayerGenId(1, IntegerLast()),
      myBackground(Quantity_NOC_GRAY30),
      myViewSize(1000.0),
      myViewProj(V3d_XposYnegZpos),
      myVisualization(V3d_ZBUFFER),
      myDefaultTypeOfView(V3d_ORTHOGRAPHIC),
      myComputedMode(Standard_True),
      myDefaultComputedMode(Standard_False),
      myPrivilegedPlane(Ax3(Point3d(0., 0., 0), Dir3d(0., 0., 1.), Dir3d(1., 0., 0.))),
      myDisplayPlane(Standard_False),
      myDisplayPlaneLength(1000.0),
      myGridType(Aspect_GT_Rectangular),
      myGridEcho(Standard_True),
      myGridEchoLastVert(ShortRealLast(), ShortRealLast(), ShortRealLast())
{
  //
}

//=================================================================================================

Handle(ViewWindow) ViewManager::CreateView()
{
  return new ViewWindow(this, myDefaultTypeOfView);
}

//=================================================================================================

void ViewManager::SetViewOn()
{
  for (V3d_ListOfView::Iterator aDefViewIter(myDefinedViews); aDefViewIter.More();
       aDefViewIter.Next())
  {
    SetViewOn(aDefViewIter.Value());
  }
}

//=================================================================================================

void ViewManager::SetViewOff()
{
  for (V3d_ListOfView::Iterator aDefViewIter(myDefinedViews); aDefViewIter.More();
       aDefViewIter.Next())
  {
    SetViewOff(aDefViewIter.Value());
  }
}

//=================================================================================================

void ViewManager::SetViewOn(const Handle(ViewWindow)& theView)
{
  Handle(Graphic3d_CView) aViewImpl = theView->View();
  if (!aViewImpl->IsDefined() || myActiveViews.Contains(theView))
  {
    return;
  }

  myActiveViews.Append(theView);
  aViewImpl->Activate();
  for (V3d_ListOfLight::Iterator anActiveLightIter(myActiveLights); anActiveLightIter.More();
       anActiveLightIter.Next())
  {
    theView->SetLightOn(anActiveLightIter.Value());
  }
  if (Handle(Aspect_Grid) aGrid = Grid(false))
  {
    theView->SetGrid(myPrivilegedPlane, aGrid);
    theView->SetGridActivity(aGrid->IsActive());
  }
  if (theView->SetImmediateUpdate(Standard_False))
  {
    theView->Redraw();
    theView->SetImmediateUpdate(Standard_True);
  }
}

//=================================================================================================

void ViewManager::SetViewOff(const Handle(ViewWindow)& theView)
{
  Handle(Graphic3d_CView) aViewImpl = theView->View();
  if (aViewImpl->IsDefined() && myActiveViews.Contains(theView))
  {
    myActiveViews.Remove(theView);
    aViewImpl->Deactivate();
  }
}

//=================================================================================================

void ViewManager::Redraw() const
{
  for (int aSubViewPass = 0; aSubViewPass < 2; ++aSubViewPass)
  {
    // redraw subviews first
    const bool isSubViewPass = (aSubViewPass == 0);
    for (const Handle(ViewWindow)& aViewIter : myDefinedViews)
    {
      if (isSubViewPass && aViewIter->IsSubview())
      {
        aViewIter->Redraw();
      }
      else if (!isSubViewPass && !aViewIter->IsSubview())
      {
        aViewIter->Redraw();
      }
    }
  }
}

//=================================================================================================

void ViewManager::RedrawImmediate() const
{
  for (int aSubViewPass = 0; aSubViewPass < 2; ++aSubViewPass)
  {
    // redraw subviews first
    const bool isSubViewPass = (aSubViewPass == 0);
    for (const Handle(ViewWindow)& aViewIter : myDefinedViews)
    {
      if (isSubViewPass && aViewIter->IsSubview())
      {
        aViewIter->RedrawImmediate();
      }
      else if (!isSubViewPass && !aViewIter->IsSubview())
      {
        aViewIter->RedrawImmediate();
      }
    }
  }
}

//=================================================================================================

void ViewManager::Invalidate() const
{
  for (V3d_ListOfView::Iterator aDefViewIter(myDefinedViews); aDefViewIter.More();
       aDefViewIter.Next())
  {
    aDefViewIter.Value()->Invalidate();
  }
}

//=================================================================================================

void ViewManager::Remove()
{
  myStructureManager->Remove();
}

//=================================================================================================

void ViewManager::Erase() const
{
  myStructureManager->Erase();
}

//=================================================================================================

void ViewManager::UnHighlight() const
{
  myStructureManager->UnHighlight();
}

void ViewManager::SetDefaultViewSize(const Standard_Real theSize)
{
  if (theSize <= 0.0)
    throw V3d_BadValue("ViewManager::SetDefaultViewSize, bad size");
  myViewSize = theSize;
}

//=================================================================================================

Standard_Boolean ViewManager::IfMoreViews() const
{
  return myDefinedViews.Size() < myStructureManager->MaxNumOfViews();
}

//=================================================================================================

void ViewManager::AddView(const Handle(ViewWindow)& theView)
{
  if (!myDefinedViews.Contains(theView))
  {
    myDefinedViews.Append(theView);
  }
}

//=================================================================================================

void ViewManager::DelView(const ViewWindow* theView)
{
  for (V3d_ListOfView::Iterator aViewIter(myActiveViews); aViewIter.More(); aViewIter.Next())
  {
    if (aViewIter.Value() == theView)
    {
      myActiveViews.Remove(aViewIter);
      break;
    }
  }
  for (V3d_ListOfView::Iterator aViewIter(myDefinedViews); aViewIter.More(); aViewIter.Next())
  {
    if (aViewIter.Value() == theView)
    {
      myDefinedViews.Remove(aViewIter);
      break;
    }
  }
}

//=================================================================================================

Standard_Boolean ViewManager::InsertLayerBefore(Graphic3d_ZLayerId&             theNewLayerId,
                                               const Graphic3d_ZLayerSettings& theSettings,
                                               const Graphic3d_ZLayerId        theLayerAfter)
{
  if (myZLayerGenId.Next(theNewLayerId))
  {
    myLayerIds.Add(theNewLayerId);
    myDriver->InsertLayerBefore(theNewLayerId, theSettings, theLayerAfter);
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ViewManager::InsertLayerAfter(Graphic3d_ZLayerId&             theNewLayerId,
                                              const Graphic3d_ZLayerSettings& theSettings,
                                              const Graphic3d_ZLayerId        theLayerBefore)
{
  if (myZLayerGenId.Next(theNewLayerId))
  {
    myLayerIds.Add(theNewLayerId);
    myDriver->InsertLayerAfter(theNewLayerId, theSettings, theLayerBefore);
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ViewManager::RemoveZLayer(const Graphic3d_ZLayerId theLayerId)
{
  if (!myLayerIds.Contains(theLayerId) || theLayerId < myZLayerGenId.Lower()
      || theLayerId > myZLayerGenId.Upper())
  {
    return Standard_False;
  }

  myDriver->RemoveZLayer(theLayerId);
  myLayerIds.Remove(theLayerId);
  myZLayerGenId.Free(theLayerId);

  return Standard_True;
}

//=================================================================================================

void ViewManager::GetAllZLayers(TColStd_SequenceOfInteger& theLayerSeq) const
{
  myDriver->ZLayers(theLayerSeq);
}

//=================================================================================================

void ViewManager::SetZLayerSettings(const Graphic3d_ZLayerId        theLayerId,
                                   const Graphic3d_ZLayerSettings& theSettings)
{
  myDriver->SetZLayerSettings(theLayerId, theSettings);
}

//=================================================================================================

const Graphic3d_ZLayerSettings& ViewManager::ZLayerSettings(
  const Graphic3d_ZLayerId theLayerId) const
{
  return myDriver->ZLayerSettings(theLayerId);
}

//=================================================================================================

void ViewManager::UpdateLights()
{
  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->UpdateLights();
  }
}

//=================================================================================================

void ViewManager::SetLightOn(const Handle(V3d_Light)& theLight)
{
  if (!myActiveLights.Contains(theLight))
  {
    myActiveLights.Append(theLight);
  }

  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->SetLightOn(theLight);
  }
}

//=================================================================================================

void ViewManager::SetLightOff(const Handle(V3d_Light)& theLight)
{
  myActiveLights.Remove(theLight);
  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->SetLightOff(theLight);
  }
}

//=================================================================================================

void ViewManager::SetLightOn()
{
  for (V3d_ListOfLight::Iterator aDefLightIter(myDefinedLights); aDefLightIter.More();
       aDefLightIter.Next())
  {
    if (!myActiveLights.Contains(aDefLightIter.Value()))
    {
      myActiveLights.Append(aDefLightIter.Value());
      for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
           anActiveViewIter.Next())
      {
        anActiveViewIter.Value()->SetLightOn(aDefLightIter.Value());
      }
    }
  }
}

//=================================================================================================

void ViewManager::SetLightOff()
{
  for (V3d_ListOfLight::Iterator anActiveLightIter(myActiveLights); anActiveLightIter.More();
       anActiveLightIter.Next())
  {
    for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
         anActiveViewIter.Next())
    {
      anActiveViewIter.Value()->SetLightOff(anActiveLightIter.Value());
    }
  }
  myActiveLights.Clear();
}

//=================================================================================================

Standard_Boolean ViewManager::IsGlobalLight(const Handle(V3d_Light)& theLight) const
{
  return myActiveLights.Contains(theLight);
}

//=================================================================================================

void ViewManager::AddLight(const Handle(V3d_Light)& theLight)
{
  if (!myDefinedLights.Contains(theLight))
  {
    myDefinedLights.Append(theLight);
  }
}

//=================================================================================================

void ViewManager::DelLight(const Handle(V3d_Light)& theLight)
{
  SetLightOff(theLight);
  myDefinedLights.Remove(theLight);
}

//=================================================================================================

void ViewManager::SetDefaultLights()
{
  while (!myDefinedLights.IsEmpty())
  {
    Handle(V3d_Light) aLight = myDefinedLights.First();
    DelLight(aLight);
  }

  Handle(V3d_DirectionalLight) aDirLight = new V3d_DirectionalLight(V3d_Zneg, Quantity_NOC_WHITE);
  aDirLight->SetName("headlight");
  aDirLight->SetHeadlight(true);
  Handle(V3d_AmbientLight) anAmbLight = new V3d_AmbientLight(Quantity_NOC_WHITE);
  anAmbLight->SetName("amblight");
  AddLight(aDirLight);
  AddLight(anAmbLight);
  SetLightOn(aDirLight);
  SetLightOn(anAmbLight);
}

//=================================================================================================

void ViewManager::SetPrivilegedPlane(const Ax3& thePlane)
{
  myPrivilegedPlane         = thePlane;
  Handle(Aspect_Grid) aGrid = Grid(true);
  aGrid->SetDrawMode(aGrid->DrawMode()); // aGrid->UpdateDisplay();
  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->SetGrid(myPrivilegedPlane, aGrid);
  }

  if (myDisplayPlane)
  {
    DisplayPrivilegedPlane(Standard_True, myDisplayPlaneLength);
  }
}

//=================================================================================================

void ViewManager::DisplayPrivilegedPlane(const Standard_Boolean theOnOff,
                                        const Standard_Real    theSize)
{
  myDisplayPlane       = theOnOff;
  myDisplayPlaneLength = theSize;

  if (!myDisplayPlane)
  {
    if (!myPlaneStructure.IsNull())
    {
      myPlaneStructure->Erase();
    }
    return;
  }

  if (myPlaneStructure.IsNull())
  {
    myPlaneStructure = new Graphic3d_Structure(StructureManager());
    myPlaneStructure->SetInfiniteState(Standard_True);
    myPlaneStructure->Display();
  }
  else
  {
    myPlaneStructure->Clear();
  }

  Handle(Graphic3d_Group) aGroup = myPlaneStructure->NewGroup();

  Handle(Graphic3d_AspectLine3d) aLineAttrib =
    new Graphic3d_AspectLine3d(Quantity_NOC_GRAY60, Aspect_TOL_SOLID, 1.0);
  aGroup->SetGroupPrimitivesAspect(aLineAttrib);

  Handle(Graphic3d_AspectText3d) aTextAttrib = new Graphic3d_AspectText3d();
  aTextAttrib->SetColor(Color1(Quantity_NOC_ROYALBLUE1));
  aGroup->SetGroupPrimitivesAspect(aTextAttrib);

  Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(6);

  const Point3d& p0 = myPrivilegedPlane.Location();

  const Point3d pX(p0.XYZ() + myDisplayPlaneLength * myPrivilegedPlane.XDirection().XYZ());
  aPrims->AddVertex(p0);
  aPrims->AddVertex(pX);
  Handle(Graphic3d_Text) aText = new Graphic3d_Text(1.0f / 81.0f);
  aText->SetText("X");
  aText->SetPosition(pX);
  aGroup->AddText(aText);

  const Point3d pY(p0.XYZ() + myDisplayPlaneLength * myPrivilegedPlane.YDirection().XYZ());
  aPrims->AddVertex(p0);
  aPrims->AddVertex(pY);
  aText = new Graphic3d_Text(1.0f / 81.0f);
  aText->SetText("Y");
  aText->SetPosition(pY);
  aGroup->AddText(aText);

  const Point3d pZ(p0.XYZ() + myDisplayPlaneLength * myPrivilegedPlane.Direction().XYZ());
  aPrims->AddVertex(p0);
  aPrims->AddVertex(pZ);
  aText = new Graphic3d_Text(1.0f / 81.0f);
  aText->SetText("Z");
  aText->SetPosition(pZ);
  aGroup->AddText(aText);

  aGroup->AddPrimitiveArray(aPrims);

  myPlaneStructure->Display();
}

//=================================================================================================

Handle(Aspect_Grid) ViewManager::Grid(Aspect_GridType theGridType, bool theToCreate)
{
  switch (theGridType)
  {
    case Aspect_GT_Circular: {
      if (myCGrid.IsNull() && theToCreate)
      {
        myCGrid = new V3d_CircularGrid(this,
                                       Color1(Quantity_NOC_GRAY50),
                                       Color1(Quantity_NOC_GRAY70));
      }
      return Handle(Aspect_Grid)(myCGrid);
    }
    case Aspect_GT_Rectangular: {
      if (myRGrid.IsNull() && theToCreate)
      {
        myRGrid = new V3d_RectangularGrid(this,
                                          Color1(Quantity_NOC_GRAY50),
                                          Color1(Quantity_NOC_GRAY70));
      }
      return Handle(Aspect_Grid)(myRGrid);
    }
  }
  return Handle(Aspect_Grid)();
}

//=================================================================================================

Aspect_GridDrawMode ViewManager::GridDrawMode()
{
  Handle(Aspect_Grid) aGrid = Grid(false);
  return !aGrid.IsNull() ? aGrid->DrawMode() : Aspect_GDM_Lines;
}

//=================================================================================================

void ViewManager::ActivateGrid(const Aspect_GridType theType, const Aspect_GridDrawMode theMode)
{
  if (Handle(Aspect_Grid) anOldGrid = Grid(false))
  {
    anOldGrid->Erase();
  }

  myGridType                = theType;
  Handle(Aspect_Grid) aGrid = Grid(true);
  aGrid->SetDrawMode(theMode);
  if (theMode != Aspect_GDM_None)
  {
    aGrid->Display();
  }
  aGrid->Activate();
  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->SetGrid(myPrivilegedPlane, aGrid);
  }
}

//=================================================================================================

void ViewManager::DeactivateGrid()
{
  Handle(Aspect_Grid) aGrid = Grid(false);
  if (aGrid.IsNull())
  {
    return;
  }

  aGrid->Erase();
  aGrid->Deactivate();

  myGridType = Aspect_GT_Rectangular;
  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->SetGridActivity(Standard_False);
    if (myGridEcho && !myGridEchoStructure.IsNull())
    {
      myGridEchoStructure->Erase();
    }
  }
}

//=================================================================================================

Standard_Boolean ViewManager::IsGridActive()
{
  Handle(Aspect_Grid) aGrid = Grid(false);
  return !aGrid.IsNull() && aGrid->IsActive();
}

//=================================================================================================

void ViewManager::RectangularGridValues(Standard_Real& theXOrigin,
                                       Standard_Real& theYOrigin,
                                       Standard_Real& theXStep,
                                       Standard_Real& theYStep,
                                       Standard_Real& theRotationAngle)
{
  Grid(Aspect_GT_Rectangular, true);
  theXOrigin       = myRGrid->XOrigin();
  theYOrigin       = myRGrid->YOrigin();
  theXStep         = myRGrid->XStep();
  theYStep         = myRGrid->YStep();
  theRotationAngle = myRGrid->RotationAngle();
}

//=================================================================================================

void ViewManager::SetRectangularGridValues(const Standard_Real theXOrigin,
                                          const Standard_Real theYOrigin,
                                          const Standard_Real theXStep,
                                          const Standard_Real theYStep,
                                          const Standard_Real theRotationAngle)
{
  Grid(Aspect_GT_Rectangular, true);
  myRGrid->SetGridValues(theXOrigin, theYOrigin, theXStep, theYStep, theRotationAngle);
  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->SetGrid(myPrivilegedPlane, myRGrid);
  }
}

//=================================================================================================

void ViewManager::CircularGridValues(Standard_Real&    theXOrigin,
                                    Standard_Real&    theYOrigin,
                                    Standard_Real&    theRadiusStep,
                                    Standard_Integer& theDivisionNumber,
                                    Standard_Real&    theRotationAngle)
{
  Grid(Aspect_GT_Circular, true);
  theXOrigin        = myCGrid->XOrigin();
  theYOrigin        = myCGrid->YOrigin();
  theRadiusStep     = myCGrid->RadiusStep();
  theDivisionNumber = myCGrid->DivisionNumber();
  theRotationAngle  = myCGrid->RotationAngle();
}

//=================================================================================================

void ViewManager::SetCircularGridValues(const Standard_Real    theXOrigin,
                                       const Standard_Real    theYOrigin,
                                       const Standard_Real    theRadiusStep,
                                       const Standard_Integer theDivisionNumber,
                                       const Standard_Real    theRotationAngle)
{
  Grid(Aspect_GT_Circular, true);
  myCGrid->SetGridValues(theXOrigin,
                         theYOrigin,
                         theRadiusStep,
                         theDivisionNumber,
                         theRotationAngle);
  for (V3d_ListOfView::Iterator anActiveViewIter(myActiveViews); anActiveViewIter.More();
       anActiveViewIter.Next())
  {
    anActiveViewIter.Value()->SetGrid(myPrivilegedPlane, myCGrid);
  }
}

//=================================================================================================

void ViewManager::RectangularGridGraphicValues(Standard_Real& theXSize,
                                              Standard_Real& theYSize,
                                              Standard_Real& theOffSet)
{
  Grid(Aspect_GT_Rectangular, true);
  myRGrid->GraphicValues(theXSize, theYSize, theOffSet);
}

//=================================================================================================

void ViewManager::SetRectangularGridGraphicValues(const Standard_Real theXSize,
                                                 const Standard_Real theYSize,
                                                 const Standard_Real theOffSet)
{
  Grid(Aspect_GT_Rectangular, true);
  myRGrid->SetGraphicValues(theXSize, theYSize, theOffSet);
}

//=================================================================================================

void ViewManager::CircularGridGraphicValues(Standard_Real& theRadius, Standard_Real& theOffSet)
{
  Grid(Aspect_GT_Circular, true);
  myCGrid->GraphicValues(theRadius, theOffSet);
}

//=================================================================================================

void ViewManager::SetCircularGridGraphicValues(const Standard_Real theRadius,
                                              const Standard_Real theOffSet)
{
  Grid(Aspect_GT_Circular, true);
  myCGrid->SetGraphicValues(theRadius, theOffSet);
}

//=================================================================================================

void ViewManager::SetGridEcho(const Standard_Boolean theToShowGrid)
{
  if (myGridEcho == theToShowGrid)
  {
    return;
  }

  myGridEcho = theToShowGrid;
  if (theToShowGrid || myGridEchoStructure.IsNull())
  {
    return;
  }

  myGridEchoStructure->Erase();
}

//=================================================================================================

void ViewManager::SetGridEcho(const Handle(Graphic3d_AspectMarker3d)& theMarker)
{
  if (myGridEchoStructure.IsNull())
  {
    myGridEchoStructure = new Graphic3d_Structure(StructureManager());
    myGridEchoGroup     = myGridEchoStructure->NewGroup();
  }

  myGridEchoAspect = theMarker;
  myGridEchoGroup->SetPrimitivesAspect(theMarker);
}

//=================================================================================================

void ViewManager::ShowGridEcho(const Handle(ViewWindow)& theView, const Vertex1& theVertex)
{
  if (!myGridEcho)
  {
    return;
  }

  if (myGridEchoStructure.IsNull())
  {
    myGridEchoStructure = new Graphic3d_Structure(StructureManager());
    myGridEchoGroup     = myGridEchoStructure->NewGroup();

    myGridEchoAspect =
      new Graphic3d_AspectMarker3d(Aspect_TOM_STAR, Color1(Quantity_NOC_GRAY90), 3.0);
    myGridEchoGroup->SetPrimitivesAspect(myGridEchoAspect);
  }

  if (theVertex.X() == myGridEchoLastVert.X() && theVertex.Y() == myGridEchoLastVert.Y()
      && theVertex.Z() == myGridEchoLastVert.Z())
  {
    return;
  }

  myGridEchoLastVert = theVertex;
  myGridEchoGroup->Clear();
  myGridEchoGroup->SetPrimitivesAspect(myGridEchoAspect);

  Handle(Graphic3d_ArrayOfPoints) anArrayOfPoints = new Graphic3d_ArrayOfPoints(1);
  anArrayOfPoints->AddVertex(theVertex.X(), theVertex.Y(), theVertex.Z());
  myGridEchoGroup->AddPrimitiveArray(anArrayOfPoints);

  myGridEchoStructure->SetZLayer(Graphic3d_ZLayerId_Topmost);
  myGridEchoStructure->SetInfiniteState(Standard_True);
  myGridEchoStructure->CStructure()->ViewAffinity = new Graphic3d_ViewAffinity();
  myGridEchoStructure->CStructure()->ViewAffinity->SetVisible(Standard_False);
  myGridEchoStructure->CStructure()->ViewAffinity->SetVisible(theView->View()->Identification(),
                                                              true);
  myGridEchoStructure->Display();
}

//=================================================================================================

void ViewManager::HideGridEcho(const Handle(ViewWindow)& theView)
{
  if (myGridEchoStructure.IsNull())
  {
    return;
  }

  myGridEchoLastVert.SetCoord(ShortRealLast(), ShortRealLast(), ShortRealLast());
  const Handle(Graphic3d_ViewAffinity)& anAffinity =
    myGridEchoStructure->CStructure()->ViewAffinity;
  if (!anAffinity.IsNull() && anAffinity->IsVisible(theView->View()->Identification()))
  {
    myGridEchoStructure->Erase();
  }
}

//=================================================================================================

void ViewManager::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myDriver.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myStructureManager.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myZLayerGenId)

  for (V3d_ListOfView::Iterator anIter(myDefinedViews); anIter.More(); anIter.Next())
  {
    const Handle(ViewWindow)& aDefinedView = anIter.Value();
    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, aDefinedView.get())
  }

  for (V3d_ListOfView::Iterator anIter(myActiveViews); anIter.More(); anIter.Next())
  {
    const Handle(ViewWindow)& anActiveView = anIter.Value();
    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, anActiveView.get())
  }

  for (V3d_ListOfLight::Iterator anIter(myDefinedLights); anIter.More(); anIter.Next())
  {
    const Handle(Graphic3d_CLight)& aDefinedLight = anIter.Value();
    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, aDefinedLight.get())
  }

  for (V3d_ListOfLight::Iterator anIter(myActiveLights); anIter.More(); anIter.Next())
  {
    const Handle(Graphic3d_CLight)& anActiveLight = anIter.Value();
    OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, anActiveLight.get())
  }

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myBackground)
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myGradientBackground)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myViewSize)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myViewProj)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myVisualization)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myDefaultTypeOfView)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myDefaultRenderingParams)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myComputedMode)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myDefaultComputedMode)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myPrivilegedPlane)
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myPlaneStructure.get())
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myDisplayPlane)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myDisplayPlaneLength)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myRGrid.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myCGrid.get())

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myGridType)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myGridEcho)
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myGridEchoStructure.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myGridEchoGroup.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, myGridEchoAspect.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myGridEchoLastVert)
}
