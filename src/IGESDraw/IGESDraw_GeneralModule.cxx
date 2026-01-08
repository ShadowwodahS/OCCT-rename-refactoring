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

#include <IGESData_DirChecker.hxx>
#include <IGESDraw_CircArraySubfigure.hxx>
#include <IGESDraw_Drawing.hxx>
#include <IGESDraw_DrawingWithRotation.hxx>
#include <IGESDraw_GeneralModule.hxx>
#include <IGESDraw_LabelDisplay.hxx>
#include <IGESDraw_NetworkSubfigure.hxx>
#include <IGESDraw_NetworkSubfigureDef.hxx>
#include <IGESDraw_PerspectiveView.hxx>
#include <IGESDraw_Planar.hxx>
#include <IGESDraw_RectArraySubfigure.hxx>
#include <IGESDraw_SegmentedViewsVisible.hxx>
#include <IGESDraw_ToolCircArraySubfigure.hxx>
#include <IGESDraw_ToolConnectPoint.hxx>
#include <IGESDraw_ToolDrawing.hxx>
#include <IGESDraw_ToolDrawingWithRotation.hxx>
#include <IGESDraw_ToolLabelDisplay.hxx>
#include <IGESDraw_ToolNetworkSubfigure.hxx>
#include <IGESDraw_ToolNetworkSubfigureDef.hxx>
#include <IGESDraw_ToolPerspectiveView.hxx>
#include <IGESDraw_ToolPlanar.hxx>
#include <IGESDraw_ToolRectArraySubfigure.hxx>
#include <IGESDraw_ToolSegmentedViewsVisible.hxx>
#include <IGESDraw_ToolView.hxx>
#include <IGESDraw_ToolViewsVisible.hxx>
#include <IGESDraw_ToolViewsVisibleWithAttr.hxx>
#include <IGESDraw_View.hxx>
#include <IGESDraw_ViewsVisible.hxx>
#include <IGESDraw_ViewsVisibleWithAttr.hxx>
#include <Interface_Category.hxx>
#include <Interface_Check.hxx>
#include <Interface_CopyTool.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESDraw_GeneralModule, IGESData_GeneralModule)

//  Each Module is attached to a Protocol : it must interpret Case Numbers1
//  (arguments <CN> of various methods) in accordance to values returned by
//  the method TypeNumber from this Protocol
IGESDraw_GeneralModule::IGESDraw_GeneralModule() {}

void IGESDraw_GeneralModule::OwnSharedCase(const Standard_Integer             CN,
                                           const Handle(IGESData_IGESEntity)& ent,
                                           Interface_EntityIterator&          iter) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDraw_CircArraySubfigure, anent, ent);
      if (anent.IsNull())
        return;
      CircArraySubfigureTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 2: {
      DeclareAndCast(IGESDraw_ConnectPoint, anent, ent);
      if (anent.IsNull())
        return;
      ConnectPointTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 3: {
      DeclareAndCast(IGESDraw_Drawing, anent, ent);
      if (anent.IsNull())
        return;
      DrawingTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 4: {
      DeclareAndCast(IGESDraw_DrawingWithRotation, anent, ent);
      if (anent.IsNull())
        return;
      DrawingWithRotationTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 5: {
      DeclareAndCast(IGESDraw_LabelDisplay, anent, ent);
      if (anent.IsNull())
        return;
      LabelDisplayTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 6: {
      DeclareAndCast(IGESDraw_NetworkSubfigure, anent, ent);
      if (anent.IsNull())
        return;
      NetworkSubfigureTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 7: {
      DeclareAndCast(IGESDraw_NetworkSubfigureDef, anent, ent);
      if (anent.IsNull())
        return;
      NetworkSubfigureDefTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 8: {
      DeclareAndCast(IGESDraw_PerspectiveView, anent, ent);
      if (anent.IsNull())
        return;
      PerspectiveViewTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 9: {
      DeclareAndCast(IGESDraw_Planar, anent, ent);
      if (anent.IsNull())
        return;
      PlanarTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 10: {
      DeclareAndCast(IGESDraw_RectArraySubfigure, anent, ent);
      if (anent.IsNull())
        return;
      RectArraySubfigureTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 11: {
      DeclareAndCast(IGESDraw_SegmentedViewsVisible, anent, ent);
      if (anent.IsNull())
        return;
      SegmentedViewsVisibleTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 12: {
      DeclareAndCast(IGESDraw_View, anent, ent);
      if (anent.IsNull())
        return;
      ViewTool1 tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 13: {
      DeclareAndCast(IGESDraw_ViewsVisible, anent, ent);
      if (anent.IsNull())
        return;
      ViewsVisibleTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, anent, ent);
      if (anent.IsNull())
        return;
      ViewsVisibleWithAttrTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    default:
      break;
  }
}

void IGESDraw_GeneralModule::OwnImpliedCase(const Standard_Integer             CN,
                                            const Handle(IGESData_IGESEntity)& ent,
                                            Interface_EntityIterator&          iter) const
{
  switch (CN)
  {
    case 13: {
      DeclareAndCast(IGESDraw_ViewsVisible, anent, ent);
      if (anent.IsNull())
        break;
      ViewsVisibleTool tool;
      tool.OwnImplied(anent, iter);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, anent, ent);
      if (anent.IsNull())
        break;
      ViewsVisibleWithAttrTool tool;
      tool.OwnImplied(anent, iter);
    }
    break;
    default:
      break;
  }
}

DirectoryChecker IGESDraw_GeneralModule::DirChecker(const Standard_Integer             CN,
                                                       const Handle(IGESData_IGESEntity)& ent) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDraw_CircArraySubfigure, anent, ent);
      if (anent.IsNull())
        break;
      CircArraySubfigureTool tool;
      return tool.DirChecker(anent);
    }
    case 2: {
      DeclareAndCast(IGESDraw_ConnectPoint, anent, ent);
      if (anent.IsNull())
        break;
      ConnectPointTool tool;
      return tool.DirChecker(anent);
    }
    case 3: {
      DeclareAndCast(IGESDraw_Drawing, anent, ent);
      if (anent.IsNull())
        break;
      DrawingTool tool;
      return tool.DirChecker(anent);
    }
    case 4: {
      DeclareAndCast(IGESDraw_DrawingWithRotation, anent, ent);
      if (anent.IsNull())
        break;
      DrawingWithRotationTool tool;
      return tool.DirChecker(anent);
    }
    case 5: {
      DeclareAndCast(IGESDraw_LabelDisplay, anent, ent);
      if (anent.IsNull())
        break;
      LabelDisplayTool tool;
      return tool.DirChecker(anent);
    }
    case 6: {
      DeclareAndCast(IGESDraw_NetworkSubfigure, anent, ent);
      if (anent.IsNull())
        break;
      NetworkSubfigureTool tool;
      return tool.DirChecker(anent);
    }
    case 7: {
      DeclareAndCast(IGESDraw_NetworkSubfigureDef, anent, ent);
      if (anent.IsNull())
        break;
      NetworkSubfigureDefTool tool;
      return tool.DirChecker(anent);
    }
    case 8: {
      DeclareAndCast(IGESDraw_PerspectiveView, anent, ent);
      if (anent.IsNull())
        break;
      PerspectiveViewTool tool;
      return tool.DirChecker(anent);
    }
    case 9: {
      DeclareAndCast(IGESDraw_Planar, anent, ent);
      if (anent.IsNull())
        break;
      PlanarTool tool;
      return tool.DirChecker(anent);
    }
    case 10: {
      DeclareAndCast(IGESDraw_RectArraySubfigure, anent, ent);
      if (anent.IsNull())
        break;
      RectArraySubfigureTool tool;
      return tool.DirChecker(anent);
    }
    case 11: {
      DeclareAndCast(IGESDraw_SegmentedViewsVisible, anent, ent);
      if (anent.IsNull())
        break;
      SegmentedViewsVisibleTool tool;
      return tool.DirChecker(anent);
    }
    case 12: {
      DeclareAndCast(IGESDraw_View, anent, ent);
      if (anent.IsNull())
        break;
      ViewTool1 tool;
      return tool.DirChecker(anent);
    }
    case 13: {
      DeclareAndCast(IGESDraw_ViewsVisible, anent, ent);
      if (anent.IsNull())
        break;
      ViewsVisibleTool tool;
      return tool.DirChecker(anent);
    }
    case 14: {
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, anent, ent);
      if (anent.IsNull())
        break;
      ViewsVisibleWithAttrTool tool;
      return tool.DirChecker(anent);
    }
    default:
      break;
  }
  return DirectoryChecker(); // by default, no specific criterium
}

void IGESDraw_GeneralModule::OwnCheckCase(const Standard_Integer             CN,
                                          const Handle(IGESData_IGESEntity)& ent,
                                          const Interface_ShareTool&         shares,
                                          Handle(Interface_Check)&           ach) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDraw_CircArraySubfigure, anent, ent);
      if (anent.IsNull())
        return;
      CircArraySubfigureTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 2: {
      DeclareAndCast(IGESDraw_ConnectPoint, anent, ent);
      if (anent.IsNull())
        return;
      ConnectPointTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 3: {
      DeclareAndCast(IGESDraw_Drawing, anent, ent);
      if (anent.IsNull())
        return;
      DrawingTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 4: {
      DeclareAndCast(IGESDraw_DrawingWithRotation, anent, ent);
      if (anent.IsNull())
        return;
      DrawingWithRotationTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 5: {
      DeclareAndCast(IGESDraw_LabelDisplay, anent, ent);
      if (anent.IsNull())
        return;
      LabelDisplayTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 6: {
      DeclareAndCast(IGESDraw_NetworkSubfigure, anent, ent);
      if (anent.IsNull())
        return;
      NetworkSubfigureTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 7: {
      DeclareAndCast(IGESDraw_NetworkSubfigureDef, anent, ent);
      if (anent.IsNull())
        return;
      NetworkSubfigureDefTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 8: {
      DeclareAndCast(IGESDraw_PerspectiveView, anent, ent);
      if (anent.IsNull())
        return;
      PerspectiveViewTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 9: {
      DeclareAndCast(IGESDraw_Planar, anent, ent);
      if (anent.IsNull())
        return;
      PlanarTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 10: {
      DeclareAndCast(IGESDraw_RectArraySubfigure, anent, ent);
      if (anent.IsNull())
        return;
      RectArraySubfigureTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 11: {
      DeclareAndCast(IGESDraw_SegmentedViewsVisible, anent, ent);
      if (anent.IsNull())
        return;
      SegmentedViewsVisibleTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 12: {
      DeclareAndCast(IGESDraw_View, anent, ent);
      if (anent.IsNull())
        return;
      ViewTool1 tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 13: {
      DeclareAndCast(IGESDraw_ViewsVisible, anent, ent);
      if (anent.IsNull())
        return;
      ViewsVisibleTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, anent, ent);
      if (anent.IsNull())
        return;
      ViewsVisibleWithAttrTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    default:
      break;
  }
}

Standard_Boolean IGESDraw_GeneralModule::NewVoid(const Standard_Integer      CN,
                                                 Handle(RefObject)& ent) const
{
  switch (CN)
  {
    case 1:
      ent = new IGESDraw_CircArraySubfigure;
      break;
    case 2:
      ent = new IGESDraw_ConnectPoint;
      break;
    case 3:
      ent = new IGESDraw_Drawing;
      break;
    case 4:
      ent = new IGESDraw_DrawingWithRotation;
      break;
    case 5:
      ent = new IGESDraw_LabelDisplay;
      break;
    case 6:
      ent = new IGESDraw_NetworkSubfigure;
      break;
    case 7:
      ent = new IGESDraw_NetworkSubfigureDef;
      break;
    case 8:
      ent = new IGESDraw_PerspectiveView;
      break;
    case 9:
      ent = new IGESDraw_Planar;
      break;
    case 10:
      ent = new IGESDraw_RectArraySubfigure;
      break;
    case 11:
      ent = new IGESDraw_SegmentedViewsVisible;
      break;
    case 12:
      ent = new IGESDraw_View;
      break;
    case 13:
      ent = new IGESDraw_ViewsVisible;
      break;
    case 14:
      ent = new IGESDraw_ViewsVisibleWithAttr;
      break;
    default:
      return Standard_False; // by default, Failure on Recognize
  }
  return Standard_True;
}

void IGESDraw_GeneralModule::OwnCopyCase(const Standard_Integer             CN,
                                         const Handle(IGESData_IGESEntity)& entfrom,
                                         const Handle(IGESData_IGESEntity)& entto,
                                         Interface_CopyTool&                TC) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDraw_CircArraySubfigure, enfr, entfrom);
      DeclareAndCast(IGESDraw_CircArraySubfigure, ento, entto);
      CircArraySubfigureTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 2: {
      DeclareAndCast(IGESDraw_ConnectPoint, enfr, entfrom);
      DeclareAndCast(IGESDraw_ConnectPoint, ento, entto);
      ConnectPointTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 3: {
      DeclareAndCast(IGESDraw_Drawing, enfr, entfrom);
      DeclareAndCast(IGESDraw_Drawing, ento, entto);
      DrawingTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 4: {
      DeclareAndCast(IGESDraw_DrawingWithRotation, enfr, entfrom);
      DeclareAndCast(IGESDraw_DrawingWithRotation, ento, entto);
      DrawingWithRotationTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 5: {
      DeclareAndCast(IGESDraw_LabelDisplay, enfr, entfrom);
      DeclareAndCast(IGESDraw_LabelDisplay, ento, entto);
      LabelDisplayTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 6: {
      DeclareAndCast(IGESDraw_NetworkSubfigure, enfr, entfrom);
      DeclareAndCast(IGESDraw_NetworkSubfigure, ento, entto);
      NetworkSubfigureTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 7: {
      DeclareAndCast(IGESDraw_NetworkSubfigureDef, enfr, entfrom);
      DeclareAndCast(IGESDraw_NetworkSubfigureDef, ento, entto);
      NetworkSubfigureDefTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 8: {
      DeclareAndCast(IGESDraw_PerspectiveView, enfr, entfrom);
      DeclareAndCast(IGESDraw_PerspectiveView, ento, entto);
      PerspectiveViewTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 9: {
      DeclareAndCast(IGESDraw_Planar, enfr, entfrom);
      DeclareAndCast(IGESDraw_Planar, ento, entto);
      PlanarTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 10: {
      DeclareAndCast(IGESDraw_RectArraySubfigure, enfr, entfrom);
      DeclareAndCast(IGESDraw_RectArraySubfigure, ento, entto);
      RectArraySubfigureTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 11: {
      DeclareAndCast(IGESDraw_SegmentedViewsVisible, enfr, entfrom);
      DeclareAndCast(IGESDraw_SegmentedViewsVisible, ento, entto);
      SegmentedViewsVisibleTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 12: {
      DeclareAndCast(IGESDraw_View, enfr, entfrom);
      DeclareAndCast(IGESDraw_View, ento, entto);
      ViewTool1 tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 13: {
      DeclareAndCast(IGESDraw_ViewsVisible, enfr, entfrom);
      DeclareAndCast(IGESDraw_ViewsVisible, ento, entto);
      ViewsVisibleTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, enfr, entfrom);
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, ento, entto);
      ViewsVisibleWithAttrTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    default:
      break;
  }
}

void IGESDraw_GeneralModule::OwnRenewCase(const Standard_Integer             CN,
                                          const Handle(IGESData_IGESEntity)& entfrom,
                                          const Handle(IGESData_IGESEntity)& entto,
                                          const Interface_CopyTool&          TC) const
{
  switch (CN)
  {
    case 13: {
      DeclareAndCast(IGESDraw_ViewsVisible, enfr, entfrom);
      DeclareAndCast(IGESDraw_ViewsVisible, ento, entto);
      ViewsVisibleTool tool;
      tool.OwnRenew(enfr, ento, TC);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, enfr, entfrom);
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, ento, entto);
      ViewsVisibleWithAttrTool tool;
      tool.OwnRenew(enfr, ento, TC);
    }
    break;
    default:
      break;
  }
}

void IGESDraw_GeneralModule::OwnDeleteCase(const Standard_Integer             CN,
                                           const Handle(IGESData_IGESEntity)& ent) const
{
  switch (CN)
  {
    case 13: {
      DeclareAndCast(IGESDraw_ViewsVisible, anent, ent);
      ViewsVisibleTool tool;
      tool.OwnWhenDelete(anent);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDraw_ViewsVisibleWithAttr, anent, ent);
      ViewsVisibleWithAttrTool tool;
      tool.OwnWhenDelete(anent);
    }
    break;
    default:
      break;
  }
}

Standard_Integer IGESDraw_GeneralModule::CategoryNumber(const Standard_Integer CN,
                                                        const Handle(RefObject)&,
                                                        const Interface_ShareTool&) const
{
  if (CN == 9)
    return Interface_Category::Number("Auxiliary");
  if (CN == 1 || CN == 2 || CN == 10)
    return Interface_Category::Number("Structure");
  return Interface_Category::Number("Drawing");
}
