// Created on: 1996-12-20
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

#include <AIS_Shape.hxx>

#include <AIS_GraphicTool.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRepBndLib.hxx>
#include <BRepTools.hxx>
#include <BRepTools_ShapeSet.hxx>
#include <Graphic3d_AspectFillArea3d.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Graphic3d_Structure.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <HLRBRep.hxx>
#include <OSD_Timer.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_BndBox.hxx>
#include <StdPrs_ToolTriangulatedShape.hxx>
#include <Quantity_Color.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Type.hxx>
#include <StdPrs_HLRPolyShape.hxx>
#include <StdPrs_HLRShape.hxx>
#include <StdPrs_ShadedShape.hxx>
#include <StdPrs_WFShape.hxx>
#include <StdSelect.hxx>
#include <StdSelect_BRepSelectionTool.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VisualShape, VisualEntity)

// Auxiliary macros
#define replaceAspectWithDef(theMap, theAspect)                                                    \
  if (myDrawer->Link1()->theAspect()->Aspect() != myDrawer->theAspect()->Aspect())                  \
  {                                                                                                \
    theMap.Bind(myDrawer->theAspect()->Aspect(), myDrawer->Link1()->theAspect()->Aspect());         \
  }

// Auxiliary macros for replaceWithNewOwnAspects()
#define replaceAspectWithOwn(theMap, theAspect)                                                    \
  if (myDrawer->Link1()->theAspect()->Aspect() != myDrawer->theAspect()->Aspect())                  \
  {                                                                                                \
    theMap.Bind(myDrawer->Link1()->theAspect()->Aspect(), myDrawer->theAspect()->Aspect());         \
  }

//=================================================================================================

void VisualShape::replaceWithNewOwnAspects()
{
  Graphic3d_MapOfAspectsToAspects aReplaceMap;

  replaceAspectWithOwn(aReplaceMap, ShadingAspect);
  replaceAspectWithOwn(aReplaceMap, LineAspect);
  replaceAspectWithOwn(aReplaceMap, WireAspect);
  replaceAspectWithOwn(aReplaceMap, FreeBoundaryAspect);
  replaceAspectWithOwn(aReplaceMap, UnFreeBoundaryAspect);
  replaceAspectWithOwn(aReplaceMap, SeenLineAspect);
  replaceAspectWithOwn(aReplaceMap, FaceBoundaryAspect);
  replaceAspectWithOwn(aReplaceMap, PointAspect);

  replaceAspects(aReplaceMap);
}

//=================================================================================================

VisualShape::VisualShape(const TopoShape& theShape)
    : VisualEntity(PrsMgr_TOP_ProjectorDependent),
      myshape(theShape),
      myUVOrigin(0.0, 0.0),
      myUVRepeat(1.0, 1.0),
      myUVScale(1.0, 1.0),
      myInitAng(0.0),
      myCompBB(Standard_True)
{
  //
}

//=================================================================================================

void VisualShape::Compute(const Handle(PrsMgr_PresentationManager)&,
                        const Handle(Prs3d_Presentation)& thePrs,
                        const Standard_Integer            theMode)
{
  if (myshape.IsNull() || (myshape.ShapeType() == TopAbs_COMPOUND && myshape.NbChildren() == 0))
  {
    return;
  }

  // wire,edge,vertex -> pas de HLR + priorite display superieure
  if (myshape.ShapeType() >= TopAbs_WIRE && myshape.ShapeType() <= TopAbs_VERTEX)
  {
    // TopAbs_WIRE -> 7, TopAbs_EDGE -> 8, TopAbs_VERTEX -> 9 (Graphic3d_DisplayPriority_Highlight)
    const Standard_Integer aPrior = (Standard_Integer)Graphic3d_DisplayPriority_Above1
                                    + (Standard_Integer)myshape.ShapeType() - TopAbs_WIRE;
    thePrs->SetVisual(Graphic3d_TOS_ALL);
    thePrs->SetDisplayPriority((Graphic3d_DisplayPriority)aPrior);
  }

  if (IsInfinite())
  {
    thePrs->SetInfiniteState(Standard_True); // not taken in account during FITALL
  }

  switch (theMode)
  {
    case AIS_WireFrame: {
      StdPrs_ToolTriangulatedShape::ClearOnOwnDeflectionChange(myshape, myDrawer, Standard_True);
      try
      {
        OCC_CATCH_SIGNALS
        StdPrs_WFShape::Add(thePrs, myshape, myDrawer);
      }
      catch (ExceptionBase const& anException)
      {
        Message::SendFail(
          AsciiString1(
            "Error: VisualShape::Compute() wireframe presentation builder has failed (")
          + anException.GetMessageString() + ")");
      }
      break;
    }
    case AIS_Shaded: {
      StdPrs_ToolTriangulatedShape::ClearOnOwnDeflectionChange(myshape, myDrawer, Standard_True);
      if ((Standard_Integer)myshape.ShapeType() > 4)
      {
        StdPrs_WFShape::Add(thePrs, myshape, myDrawer);
      }
      else
      {
        if (IsInfinite())
        {
          StdPrs_WFShape::Add(thePrs, myshape, myDrawer);
        }
        else
        {
          try
          {
            OCC_CATCH_SIGNALS
            StdPrs_ShadedShape::Add(
              thePrs,
              myshape,
              myDrawer,
              myDrawer->ShadingAspect()->Aspect()->ToMapTexture()
                && !myDrawer->ShadingAspect()->Aspect()->TextureMap().IsNull(),
              myUVOrigin,
              myUVRepeat,
              myUVScale);
          }
          catch (ExceptionBase const& anException)
          {
            Message::SendFail(
              AsciiString1(
                "Error: VisualShape::Compute() shaded presentation builder has failed (")
              + anException.GetMessageString() + ")");
            StdPrs_WFShape::Add(thePrs, myshape, myDrawer);
          }
        }
      }
      Standard_Real aTransparency = Transparency();
      if (aTransparency > 0.0)
      {
        SetTransparency(aTransparency);
      }
      break;
    }

    // Bounding box.
    case 2: {
      if (IsInfinite())
      {
        StdPrs_WFShape::Add(thePrs, myshape, myDrawer);
      }
      else
      {
        Prs3d_BndBox::Add(thePrs, BoundingBox(), myDrawer);
      }
    }
  }

  // Recompute hidden line presentation (if necessary).
  thePrs->ReCompute();
}

//=================================================================================================

void VisualShape::computeHlrPresentation(const Handle(CameraOn3d)&   theProjector,
                                       const Handle(Prs3d_Presentation)& thePrs,
                                       const TopoShape&               theShape,
                                       const Handle(StyleDrawer)&       theDrawer)
{
  if (theShape.IsNull())
  {
    return;
  }

  switch (theShape.ShapeType())
  {
    case TopAbs_VERTEX:
    case TopAbs_EDGE:
    case TopAbs_WIRE: {
      thePrs->SetDisplayPriority(Graphic3d_DisplayPriority_Below);
      StdPrs_WFShape::Add(thePrs, theShape, theDrawer);
      return;
    }
    case TopAbs_COMPOUND: {
      if (theShape.NbChildren() == 0)
      {
        return;
      }
      break;
    }
    default: {
      break;
    }
  }

  const Handle(StyleDrawer)& aDefDrawer = theDrawer->Link1();
  if (aDefDrawer->DrawHiddenLine())
  {
    theDrawer->EnableDrawHiddenLine();
  }
  else
  {
    theDrawer->DisableDrawHiddenLine();
  }

  const Aspect_TypeOfDeflection aPrevDef = aDefDrawer->TypeOfDeflection();
  aDefDrawer->SetTypeOfDeflection(Aspect_TOD_RELATIVE);
  if (theDrawer->IsAutoTriangulation())
  {
    StdPrs_ToolTriangulatedShape::ClearOnOwnDeflectionChange(theShape, theDrawer, Standard_True);
  }

  {
    try
    {
      OCC_CATCH_SIGNALS
      switch (theDrawer->TypeOfHLR())
      {
        case Prs3d_TOH_Algo: {
          StdPrs_HLRShape aBuilder;
          aBuilder.ComputeHLR(thePrs, theShape, theDrawer, theProjector);
          break;
        }
        case Prs3d_TOH_PolyAlgo:
        case Prs3d_TOH_NotSet: {
          StdPrs_HLRPolyShape aBuilder;
          aBuilder.ComputeHLR(thePrs, theShape, theDrawer, theProjector);
          break;
        }
      }
    }
    catch (ExceptionBase const& anException)
    {
      Message::SendFail(
        AsciiString1("Error: VisualShape::Compute() HLR Algorithm has failed (")
        + anException.GetMessageString() + ")");
      StdPrs_WFShape::Add(thePrs, theShape, theDrawer);
    }
  }

  aDefDrawer->SetTypeOfDeflection(aPrevDef);
}

//=================================================================================================

void VisualShape::ComputeSelection(const Handle(SelectionContainer)& aSelection,
                                 const Standard_Integer             aMode)
{
  if (myshape.IsNull())
    return;
  if (myshape.ShapeType() == TopAbs_COMPOUND && myshape.NbChildren() == 0)
  {
    // empty Shape -> empty Assembly.
    return;
  }

  TopAbs_ShapeEnum TypOfSel = VisualShape::SelectionType(aMode);
  TopoShape     shape    = myshape;

  // POP protection against crash in low layers

  Standard_Real aDeflection = StdPrs_ToolTriangulatedShape::GetDeflection(shape, myDrawer);
  try
  {
    OCC_CATCH_SIGNALS
    StdSelect_BRepSelectionTool::Load(aSelection,
                                      this,
                                      shape,
                                      TypOfSel,
                                      aDeflection,
                                      myDrawer->DeviationAngle(),
                                      myDrawer->IsAutoTriangulation());
  }
  catch (ExceptionBase const& anException)
  {
    Message::SendFail(AsciiString1("Error: VisualShape::ComputeSelection(") + aMode
                      + ") has failed (" + anException.GetMessageString() + ")");
    if (aMode == 0)
    {
      aSelection->Clear();
      Box2                       B             = BoundingBox();
      Handle(StdSelect_BRepOwner)   aOwner        = new StdSelect_BRepOwner(shape, this);
      Handle(Select3D_SensitiveBox) aSensitiveBox = new Select3D_SensitiveBox(aOwner, B);
      aSelection->Add(aSensitiveBox);
    }
  }

  // insert the drawer in the BrepOwners for hilight...
  StdSelect::SetDrawerForBRepOwner(aSelection, myDrawer);
}

void VisualShape::Color(Quantity_Color& theColor) const
{
  if (const Handle(Prs3d_ShadingAspect)& aShading = myDrawer->ShadingAspect())
  {
    theColor = aShading->Color(myCurrentFacingModel);
  }
}

Graphic3d_NameOfMaterial VisualShape::Material() const
{
  const Handle(Prs3d_ShadingAspect)& aShading = myDrawer->ShadingAspect();
  return !aShading.IsNull() ? aShading->Material(myCurrentFacingModel).Name()
                            : Graphic3d_NameOfMaterial_DEFAULT;
}

Standard_Real VisualShape::Transparency() const
{
  const Handle(Prs3d_ShadingAspect)& aShading = myDrawer->ShadingAspect();
  return !aShading.IsNull() ? aShading->Transparency(myCurrentFacingModel) : 0.0;
}

//=================================================================================================

bool VisualShape::setColor(const Handle(StyleDrawer)& theDrawer,
                         const Quantity_Color&       theColor) const
{
  bool toRecompute = false;
  toRecompute      = theDrawer->SetupOwnShadingAspect() || toRecompute;
  toRecompute      = theDrawer->SetOwnLineAspects() || toRecompute;
  toRecompute      = theDrawer->SetupOwnPointAspect() || toRecompute;

  // override color
  theDrawer->ShadingAspect()->SetColor(theColor, myCurrentFacingModel);
  theDrawer->LineAspect()->SetColor(theColor);
  theDrawer->WireAspect()->SetColor(theColor);
  theDrawer->PointAspect()->SetColor(theColor);
  theDrawer->FreeBoundaryAspect()->SetColor(theColor);
  theDrawer->UnFreeBoundaryAspect()->SetColor(theColor);
  theDrawer->SeenLineAspect()->SetColor(theColor);
  theDrawer->FaceBoundaryAspect()->SetColor(theColor);
  return toRecompute;
}

//=================================================================================================

void VisualShape::SetColor(const Quantity_Color& theColor)
{
  const bool toRecompute = setColor(myDrawer, theColor);
  myDrawer->SetColor(theColor);
  hasOwnColor = Standard_True;

  if (!toRecompute || !myDrawer->HasLink())
  {
    SynchronizeAspects();
  }
  else
  {
    replaceWithNewOwnAspects();
  }
  recomputeComputed();
}

//=================================================================================================

void VisualShape::UnsetColor()
{
  if (!HasColor())
  {
    return;
  }

  hasOwnColor = Standard_False;
  myDrawer->SetColor(myDrawer->HasLink() ? myDrawer->Link1()->Color()
                                         : Quantity_Color(Quantity_NOC_WHITE));

  Graphic3d_MapOfAspectsToAspects aReplaceMap;
  if (!HasWidth())
  {
    replaceAspectWithDef(aReplaceMap, LineAspect);
    replaceAspectWithDef(aReplaceMap, WireAspect);
    replaceAspectWithDef(aReplaceMap, FreeBoundaryAspect);
    replaceAspectWithDef(aReplaceMap, UnFreeBoundaryAspect);
    replaceAspectWithDef(aReplaceMap, SeenLineAspect);
    replaceAspectWithDef(aReplaceMap, FaceBoundaryAspect);
    myDrawer->SetLineAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetWireAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetFreeBoundaryAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetUnFreeBoundaryAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetSeenLineAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetFaceBoundaryAspect(Handle(Prs3d_LineAspect)());
  }
  else
  {
    Quantity_Color aColor = Quantity_NOC_YELLOW;
    if (myDrawer->HasLink())
    {
      AIS_GraphicTool::GetLineColor(myDrawer->Link1(), AIS_TOA_Line, aColor);
    }
    myDrawer->LineAspect()->SetColor(aColor);
    aColor = Quantity_NOC_RED;
    if (myDrawer->HasLink())
    {
      AIS_GraphicTool::GetLineColor(myDrawer->Link1(), AIS_TOA_Wire, aColor);
    }
    myDrawer->WireAspect()->SetColor(aColor);
    aColor = Quantity_NOC_GREEN;
    if (myDrawer->HasLink())
    {
      AIS_GraphicTool::GetLineColor(myDrawer->Link1(), AIS_TOA_Free, aColor);
    }
    myDrawer->FreeBoundaryAspect()->SetColor(aColor);
    aColor = Quantity_NOC_YELLOW;
    if (myDrawer->HasLink())
    {
      AIS_GraphicTool::GetLineColor(myDrawer->Link1(), AIS_TOA_UnFree, aColor);
    }
    myDrawer->UnFreeBoundaryAspect()->SetColor(aColor);
    if (myDrawer->HasLink())
    {
      AIS_GraphicTool::GetLineColor(myDrawer->Link1(), AIS_TOA_Seen, aColor);
    }
    myDrawer->SeenLineAspect()->SetColor(aColor);
    aColor = Quantity_NOC_BLACK;
    if (myDrawer->HasLink())
    {
      AIS_GraphicTool::GetLineColor(myDrawer->Link1(), AIS_TOA_FaceBoundary, aColor);
    }
    myDrawer->FaceBoundaryAspect()->SetColor(aColor);
  }

  if (!myDrawer->HasOwnShadingAspect())
  {
    //
  }
  else if (HasMaterial() || IsTransparent() || myDrawer->ShadingAspect()->Aspect()->ToMapTexture())
  {
    const Graphic3d_MaterialAspect aDefaultMat(Graphic3d_NameOfMaterial_Brass);
    Graphic3d_MaterialAspect       mat                 = aDefaultMat;
    Quantity_Color                 anInteriorColors[2] = {Quantity_NOC_CYAN1, Quantity_NOC_CYAN1};
    if (myDrawer->HasLink())
    {
      anInteriorColors[0] = myDrawer->Link1()->ShadingAspect()->Aspect()->InteriorColor();
      anInteriorColors[1] = myDrawer->Link1()->ShadingAspect()->Aspect()->BackInteriorColor();
    }
    if (HasMaterial() || myDrawer->HasLink())
    {
      const Handle(Graphic3d_AspectFillArea3d)& aSrcAspect =
        (HasMaterial() ? myDrawer : myDrawer->Link1())->ShadingAspect()->Aspect();
      mat = myCurrentFacingModel != Aspect_TOFM_BACK_SIDE ? aSrcAspect->FrontMaterial()
                                                          : aSrcAspect->BackMaterial();
    }
    if (HasMaterial())
    {
      const Quantity_Color aColor =
        myDrawer->HasLink() ? myDrawer->Link1()->ShadingAspect()->Color(myCurrentFacingModel)
                            : aDefaultMat.AmbientColor();
      mat.SetColor(aColor);
    }
    if (IsTransparent())
    {
      Standard_Real aTransp = myDrawer->ShadingAspect()->Transparency(myCurrentFacingModel);
      mat.SetTransparency(Standard_ShortReal(aTransp));
    }
    myDrawer->ShadingAspect()->SetMaterial(mat, myCurrentFacingModel);
    myDrawer->ShadingAspect()->Aspect()->SetInteriorColor(anInteriorColors[0]);
    myDrawer->ShadingAspect()->Aspect()->SetBackInteriorColor(anInteriorColors[1]);
  }
  else
  {
    replaceAspectWithDef(aReplaceMap, ShadingAspect);
    myDrawer->SetShadingAspect(Handle(Prs3d_ShadingAspect)());
  }
  if (myDrawer->HasOwnPointAspect())
  {
    replaceAspectWithDef(aReplaceMap, PointAspect);
    myDrawer->SetPointAspect(Handle(Prs3d_PointAspect)());
  }
  replaceAspects(aReplaceMap);
  SynchronizeAspects();
  recomputeComputed();
}

//=================================================================================================

bool VisualShape::setWidth(const Handle(StyleDrawer)& theDrawer,
                         const Standard_Real         theLineWidth) const
{
  bool toRecompute = theDrawer->SetOwnLineAspects();

  // override width
  theDrawer->LineAspect()->SetWidth(theLineWidth);
  theDrawer->WireAspect()->SetWidth(theLineWidth);
  theDrawer->FreeBoundaryAspect()->SetWidth(theLineWidth);
  theDrawer->UnFreeBoundaryAspect()->SetWidth(theLineWidth);
  theDrawer->SeenLineAspect()->SetWidth(theLineWidth);
  theDrawer->FaceBoundaryAspect()->SetWidth(theLineWidth);
  return toRecompute;
}

//=================================================================================================

void VisualShape::SetWidth(const Standard_Real theLineWidth)
{
  myOwnWidth = (Standard_ShortReal)theLineWidth;

  if (!setWidth(myDrawer, theLineWidth) || !myDrawer->HasLink())
  {
    SynchronizeAspects();
  }
  else
  {
    replaceWithNewOwnAspects();
  }
  recomputeComputed();
}

//=================================================================================================

void VisualShape::UnsetWidth()
{
  if (myOwnWidth == 0.0f)
  {
    return;
  }

  myOwnWidth = 0.0f;
  if (!HasColor())
  {
    Graphic3d_MapOfAspectsToAspects aReplaceMap;
    replaceAspectWithDef(aReplaceMap, LineAspect);
    replaceAspectWithDef(aReplaceMap, WireAspect);
    replaceAspectWithDef(aReplaceMap, FreeBoundaryAspect);
    replaceAspectWithDef(aReplaceMap, UnFreeBoundaryAspect);
    replaceAspectWithDef(aReplaceMap, SeenLineAspect);
    replaceAspectWithDef(aReplaceMap, FaceBoundaryAspect);
    myDrawer->SetLineAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetWireAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetFreeBoundaryAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetUnFreeBoundaryAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetSeenLineAspect(Handle(Prs3d_LineAspect)());
    myDrawer->SetFaceBoundaryAspect(Handle(Prs3d_LineAspect)());
    replaceAspects(aReplaceMap);
  }
  else
  {
    myDrawer->LineAspect()->SetWidth(
      myDrawer->HasLink() ? AIS_GraphicTool::GetLineWidth(myDrawer->Link1(), AIS_TOA_Line) : 1.);
    myDrawer->WireAspect()->SetWidth(
      myDrawer->HasLink() ? AIS_GraphicTool::GetLineWidth(myDrawer->Link1(), AIS_TOA_Wire) : 1.);
    myDrawer->FreeBoundaryAspect()->SetWidth(
      myDrawer->HasLink() ? AIS_GraphicTool::GetLineWidth(myDrawer->Link1(), AIS_TOA_Free) : 1.);
    myDrawer->UnFreeBoundaryAspect()->SetWidth(
      myDrawer->HasLink() ? AIS_GraphicTool::GetLineWidth(myDrawer->Link1(), AIS_TOA_UnFree) : 1.);
    myDrawer->SeenLineAspect()->SetWidth(
      myDrawer->HasLink() ? AIS_GraphicTool::GetLineWidth(myDrawer->Link1(), AIS_TOA_Seen) : 1.);
    myDrawer->FaceBoundaryAspect()->SetWidth(
      myDrawer->HasLink() ? AIS_GraphicTool::GetLineWidth(myDrawer->Link1(), AIS_TOA_FaceBoundary)
                          : 1.);
    SynchronizeAspects();
  }
  recomputeComputed();
}

//=================================================================================================

void VisualShape::setMaterial(const Handle(StyleDrawer)&     theDrawer,
                            const Graphic3d_MaterialAspect& theMaterial,
                            const Standard_Boolean          theToKeepColor,
                            const Standard_Boolean          theToKeepTransp) const
{
  theDrawer->SetupOwnShadingAspect();

  const Quantity_Color aColor  = theDrawer->ShadingAspect()->Color(myCurrentFacingModel);
  const Standard_Real  aTransp = theDrawer->ShadingAspect()->Transparency(myCurrentFacingModel);
  theDrawer->ShadingAspect()->SetMaterial(theMaterial, myCurrentFacingModel);

  if (theToKeepColor)
  {
    theDrawer->ShadingAspect()->SetColor(aColor, myCurrentFacingModel);
  }
  if (theToKeepTransp)
  {
    theDrawer->ShadingAspect()->SetTransparency(aTransp, myCurrentFacingModel);
  }
}

//=================================================================================================

void VisualShape::SetMaterial(const Graphic3d_MaterialAspect& theMat)
{
  const bool toRecompute = !myDrawer->HasOwnShadingAspect();
  setMaterial(myDrawer, theMat, HasColor(), IsTransparent());
  hasOwnMaterial = Standard_True;

  if (!toRecompute || !myDrawer->HasLink())
  {
    SynchronizeAspects();
  }
  else
  {
    replaceWithNewOwnAspects();
  }
}

//=================================================================================================

void VisualShape::UnsetMaterial()
{
  if (!HasMaterial())
  {
    return;
  }

  if (!myDrawer->HasOwnShadingAspect())
  {
    //
  }
  else if (HasColor() || IsTransparent() || myDrawer->ShadingAspect()->Aspect()->ToMapTexture())
  {
    if (myDrawer->HasLink())
    {
      myDrawer->ShadingAspect()->SetMaterial(
        myDrawer->Link1()->ShadingAspect()->Material(myCurrentFacingModel),
        myCurrentFacingModel);
    }
    if (HasColor())
    {
      myDrawer->ShadingAspect()->SetColor(myDrawer->Color(), myCurrentFacingModel);
      myDrawer->ShadingAspect()->SetTransparency(myDrawer->Transparency(), myCurrentFacingModel);
    }
    SynchronizeAspects();
  }
  else
  {
    Graphic3d_MapOfAspectsToAspects aReplaceMap;
    replaceAspectWithDef(aReplaceMap, ShadingAspect);
    myDrawer->SetShadingAspect(Handle(Prs3d_ShadingAspect)());
    replaceAspects(aReplaceMap);
  }
}

//=================================================================================================

void VisualShape::setTransparency(const Handle(StyleDrawer)& theDrawer,
                                const Standard_Real         theValue) const
{
  theDrawer->SetupOwnShadingAspect();
  // override transparency
  theDrawer->ShadingAspect()->SetTransparency(theValue, myCurrentFacingModel);
}

//=================================================================================================

void VisualShape::SetTransparency(const Standard_Real theValue)
{
  const bool toRecompute = !myDrawer->HasOwnShadingAspect();
  setTransparency(myDrawer, theValue);
  myDrawer->SetTransparency((Standard_ShortReal)theValue);

  if (!toRecompute || !myDrawer->HasLink())
  {
    SynchronizeAspects();
  }
  else
  {
    replaceWithNewOwnAspects();
  }
}

//=================================================================================================

void VisualShape::UnsetTransparency()
{
  myDrawer->SetTransparency(0.0f);
  if (!myDrawer->HasOwnShadingAspect())
  {
    return;
  }
  else if (HasColor() || HasMaterial() || myDrawer->ShadingAspect()->Aspect()->ToMapTexture())
  {
    myDrawer->ShadingAspect()->SetTransparency(0.0, myCurrentFacingModel);
    SynchronizeAspects();
  }
  else
  {
    Graphic3d_MapOfAspectsToAspects aReplaceMap;
    replaceAspectWithDef(aReplaceMap, ShadingAspect);
    myDrawer->SetShadingAspect(Handle(Prs3d_ShadingAspect)());
    replaceAspects(aReplaceMap);
  }
}

//=================================================================================================

const Box2& VisualShape::BoundingBox()
{
  if (myshape.ShapeType() == TopAbs_COMPOUND && myshape.NbChildren() == 0)
  {
    // empty Shape  -> empty Assembly.
    myBB.SetVoid();
    return myBB;
  }

  if (myCompBB)
  {
    BRepBndLib::Add(myshape, myBB, false);
    myCompBB = Standard_False;
  }
  return myBB;
}

//*****
//***** Reset
//=======================================================================
// function : SetOwnDeviationCoefficient
// purpose  : resets myhasOwnDeviationCoefficient to Standard_False and
//           returns Standard_True if it change
//=======================================================================

Standard_Boolean VisualShape::SetOwnDeviationCoefficient()
{
  Standard_Boolean itSet = myDrawer->HasOwnDeviationCoefficient();
  if (itSet)
    myDrawer->SetDeviationCoefficient();
  return itSet;
}

//=======================================================================
// function : SetOwnDeviationAngle
// purpose  : resets myhasOwnDeviationAngle to Standard_False and
//           returns Standard_True if it change
//=======================================================================

Standard_Boolean VisualShape::SetOwnDeviationAngle()
{
  Standard_Boolean itSet = myDrawer->HasOwnDeviationAngle();
  if (itSet)
    myDrawer->SetDeviationAngle();
  return itSet;
}

//=================================================================================================

void VisualShape::SetOwnDeviationCoefficient(const Standard_Real aCoefficient)
{
  myDrawer->SetDeviationCoefficient(aCoefficient);
  SetToUpdate();
}

//=================================================================================================

void VisualShape::SetOwnDeviationAngle(const Standard_Real theAngle)
{
  myDrawer->SetDeviationAngle(theAngle);
  SetToUpdate(AIS_WireFrame);
}

//=================================================================================================

void VisualShape::SetAngleAndDeviation(const Standard_Real anAngle)
{
  Standard_Real OutAngl, OutDefl;
  HLRBRep1::PolyHLRAngleAndDeflection(anAngle, OutAngl, OutDefl);
  SetOwnDeviationAngle(anAngle);
  SetOwnDeviationCoefficient(OutDefl);
  myInitAng = anAngle;
  SetToUpdate();
}

//=================================================================================================

Standard_Real VisualShape::UserAngle() const
{
  return myInitAng == 0. ? GetContext()->DeviationAngle() : myInitAng;
}

//=================================================================================================

Standard_Boolean VisualShape::OwnDeviationCoefficient(Standard_Real& aCoefficient,
                                                    Standard_Real& aPreviousCoefficient) const
{
  aCoefficient         = myDrawer->DeviationCoefficient();
  aPreviousCoefficient = myDrawer->PreviousDeviationCoefficient();
  return myDrawer->HasOwnDeviationCoefficient();
}

//=================================================================================================

Standard_Boolean VisualShape::OwnDeviationAngle(Standard_Real& anAngle,
                                              Standard_Real& aPreviousAngle) const
{
  anAngle        = myDrawer->DeviationAngle();
  aPreviousAngle = myDrawer->PreviousDeviationAngle();
  return myDrawer->HasOwnDeviationAngle();
}

//=================================================================================================

void VisualShape::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)
  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, VisualEntity)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myshape)
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myBB)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myInitAng)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myCompBB)
}
