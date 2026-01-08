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
#include <IGESDimen_AngularDimension.hxx>
#include <IGESDimen_BasicDimension.hxx>
#include <IGESDimen_CenterLine.hxx>
#include <IGESDimen_CurveDimension.hxx>
#include <IGESDimen_DiameterDimension.hxx>
#include <IGESDimen_DimensionDisplayData.hxx>
#include <IGESDimen_DimensionedGeometry.hxx>
#include <IGESDimen_DimensionTolerance.hxx>
#include <IGESDimen_DimensionUnits.hxx>
#include <IGESDimen_FlagNote.hxx>
#include <IGESDimen_GeneralLabel.hxx>
#include <IGESDimen_GeneralModule.hxx>
#include <IGESDimen_GeneralNote.hxx>
#include <IGESDimen_GeneralSymbol.hxx>
#include <IGESDimen_LeaderArrow.hxx>
#include <IGESDimen_LinearDimension.hxx>
#include <IGESDimen_NewDimensionedGeometry.hxx>
#include <IGESDimen_NewGeneralNote.hxx>
#include <IGESDimen_OrdinateDimension.hxx>
#include <IGESDimen_PointDimension.hxx>
#include <IGESDimen_RadiusDimension.hxx>
#include <IGESDimen_Section.hxx>
#include <IGESDimen_SectionedArea.hxx>
#include <IGESDimen_ToolAngularDimension.hxx>
#include <IGESDimen_ToolBasicDimension.hxx>
#include <IGESDimen_ToolCenterLine.hxx>
#include <IGESDimen_ToolCurveDimension.hxx>
#include <IGESDimen_ToolDiameterDimension.hxx>
#include <IGESDimen_ToolDimensionDisplayData.hxx>
#include <IGESDimen_ToolDimensionedGeometry.hxx>
#include <IGESDimen_ToolDimensionTolerance.hxx>
#include <IGESDimen_ToolDimensionUnits.hxx>
#include <IGESDimen_ToolFlagNote.hxx>
#include <IGESDimen_ToolGeneralLabel.hxx>
#include <IGESDimen_ToolGeneralNote.hxx>
#include <IGESDimen_ToolGeneralSymbol.hxx>
#include <IGESDimen_ToolLeaderArrow.hxx>
#include <IGESDimen_ToolLinearDimension.hxx>
#include <IGESDimen_ToolNewDimensionedGeometry.hxx>
#include <IGESDimen_ToolNewGeneralNote.hxx>
#include <IGESDimen_ToolOrdinateDimension.hxx>
#include <IGESDimen_ToolPointDimension.hxx>
#include <IGESDimen_ToolRadiusDimension.hxx>
#include <IGESDimen_ToolSection.hxx>
#include <IGESDimen_ToolSectionedArea.hxx>
#include <IGESDimen_ToolWitnessLine.hxx>
#include <IGESDimen_WitnessLine.hxx>
#include <Interface_Category.hxx>
#include <Interface_Check.hxx>
#include <Interface_CopyTool.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESDimen_GeneralModule, IGESData_GeneralModule)

//  Each Module is attached to a Protocol : it must interpret Case Numbers1
//  (arguments <CN> of various methods) in accordance to values returned by
//  the method TypeNumber from this Protocol
IGESDimen_GeneralModule::IGESDimen_GeneralModule() {}

void IGESDimen_GeneralModule::OwnSharedCase(const Standard_Integer             CN,
                                            const Handle(IGESData_IGESEntity)& ent,
                                            Interface_EntityIterator&          iter) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDimen_AngularDimension, anent, ent);
      if (anent.IsNull())
        return;
      AngularDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 2: {
      DeclareAndCast(IGESDimen_BasicDimension, anent, ent);
      if (anent.IsNull())
        return;
      BasicDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 3: {
      DeclareAndCast(IGESDimen_CenterLine, anent, ent);
      if (anent.IsNull())
        return;
      CenterLineTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 4: {
      DeclareAndCast(IGESDimen_CurveDimension, anent, ent);
      if (anent.IsNull())
        return;
      CurveDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 5: {
      DeclareAndCast(IGESDimen_DiameterDimension, anent, ent);
      if (anent.IsNull())
        return;
      DiameterDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 6: {
      DeclareAndCast(IGESDimen_DimensionDisplayData, anent, ent);
      if (anent.IsNull())
        return;
      DimensionDisplayDataTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 7: {
      DeclareAndCast(IGESDimen_DimensionTolerance, anent, ent);
      if (anent.IsNull())
        return;
      DimensionToleranceTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 8: {
      DeclareAndCast(IGESDimen_DimensionUnits, anent, ent);
      if (anent.IsNull())
        return;
      DimensionUnitsTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 9: {
      DeclareAndCast(IGESDimen_DimensionedGeometry, anent, ent);
      if (anent.IsNull())
        return;
      DimensionedGeometryTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 10: {
      DeclareAndCast(IGESDimen_FlagNote, anent, ent);
      if (anent.IsNull())
        return;
      FlagNoteTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 11: {
      DeclareAndCast(IGESDimen_GeneralLabel, anent, ent);
      if (anent.IsNull())
        return;
      GeneralLabelTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 12: {
      DeclareAndCast(IGESDimen_GeneralNote, anent, ent);
      if (anent.IsNull())
        return;
      GeneralNoteTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 13: {
      DeclareAndCast(IGESDimen_GeneralSymbol, anent, ent);
      if (anent.IsNull())
        return;
      GeneralSymbolTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDimen_LeaderArrow, anent, ent);
      if (anent.IsNull())
        return;
      LeaderArrowTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 15: {
      DeclareAndCast(IGESDimen_LinearDimension, anent, ent);
      if (anent.IsNull())
        return;
      LinearDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 16: {
      DeclareAndCast(IGESDimen_NewDimensionedGeometry, anent, ent);
      if (anent.IsNull())
        return;
      NewDimensionedGeometryTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 17: {
      DeclareAndCast(IGESDimen_NewGeneralNote, anent, ent);
      if (anent.IsNull())
        return;
      NewGeneralNoteTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 18: {
      DeclareAndCast(IGESDimen_OrdinateDimension, anent, ent);
      if (anent.IsNull())
        return;
      OrdinateDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 19: {
      DeclareAndCast(IGESDimen_PointDimension, anent, ent);
      if (anent.IsNull())
        return;
      PointDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 20: {
      DeclareAndCast(IGESDimen_RadiusDimension, anent, ent);
      if (anent.IsNull())
        return;
      RadiusDimensionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 21: {
      DeclareAndCast(IGESDimen_Section, anent, ent);
      if (anent.IsNull())
        return;
      SectionTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 22: {
      DeclareAndCast(IGESDimen_SectionedArea, anent, ent);
      if (anent.IsNull())
        return;
      SectionedAreaTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 23: {
      DeclareAndCast(IGESDimen_WitnessLine, anent, ent);
      if (anent.IsNull())
        return;
      WitnessLineTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    default:
      break;
  }
}

DirectoryChecker IGESDimen_GeneralModule::DirChecker(
  const Standard_Integer             CN,
  const Handle(IGESData_IGESEntity)& ent) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDimen_AngularDimension, anent, ent);
      if (anent.IsNull())
        break;
      AngularDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 2: {
      DeclareAndCast(IGESDimen_BasicDimension, anent, ent);
      if (anent.IsNull())
        break;
      BasicDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 3: {
      DeclareAndCast(IGESDimen_CenterLine, anent, ent);
      if (anent.IsNull())
        break;
      CenterLineTool tool;
      return tool.DirChecker(anent);
    }
    case 4: {
      DeclareAndCast(IGESDimen_CurveDimension, anent, ent);
      if (anent.IsNull())
        break;
      CurveDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 5: {
      DeclareAndCast(IGESDimen_DiameterDimension, anent, ent);
      if (anent.IsNull())
        break;
      DiameterDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 6: {
      DeclareAndCast(IGESDimen_DimensionDisplayData, anent, ent);
      if (anent.IsNull())
        break;
      DimensionDisplayDataTool tool;
      return tool.DirChecker(anent);
    }
    case 7: {
      DeclareAndCast(IGESDimen_DimensionTolerance, anent, ent);
      if (anent.IsNull())
        break;
      DimensionToleranceTool tool;
      return tool.DirChecker(anent);
    }
    case 8: {
      DeclareAndCast(IGESDimen_DimensionUnits, anent, ent);
      if (anent.IsNull())
        break;
      DimensionUnitsTool tool;
      return tool.DirChecker(anent);
    }
    case 9: {
      DeclareAndCast(IGESDimen_DimensionedGeometry, anent, ent);
      if (anent.IsNull())
        break;
      DimensionedGeometryTool tool;
      return tool.DirChecker(anent);
    }
    case 10: {
      DeclareAndCast(IGESDimen_FlagNote, anent, ent);
      if (anent.IsNull())
        break;
      FlagNoteTool tool;
      return tool.DirChecker(anent);
    }
    case 11: {
      DeclareAndCast(IGESDimen_GeneralLabel, anent, ent);
      if (anent.IsNull())
        break;
      GeneralLabelTool tool;
      return tool.DirChecker(anent);
    }
    case 12: {
      DeclareAndCast(IGESDimen_GeneralNote, anent, ent);
      if (anent.IsNull())
        break;
      GeneralNoteTool tool;
      return tool.DirChecker(anent);
    }
    case 13: {
      DeclareAndCast(IGESDimen_GeneralSymbol, anent, ent);
      if (anent.IsNull())
        break;
      GeneralSymbolTool tool;
      return tool.DirChecker(anent);
    }
    case 14: {
      DeclareAndCast(IGESDimen_LeaderArrow, anent, ent);
      if (anent.IsNull())
        break;
      LeaderArrowTool tool;
      return tool.DirChecker(anent);
    }
    case 15: {
      DeclareAndCast(IGESDimen_LinearDimension, anent, ent);
      if (anent.IsNull())
        break;
      LinearDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 16: {
      DeclareAndCast(IGESDimen_NewDimensionedGeometry, anent, ent);
      if (anent.IsNull())
        break;
      NewDimensionedGeometryTool tool;
      return tool.DirChecker(anent);
    }
    case 17: {
      DeclareAndCast(IGESDimen_NewGeneralNote, anent, ent);
      if (anent.IsNull())
        break;
      NewGeneralNoteTool tool;
      return tool.DirChecker(anent);
    }
    case 18: {
      DeclareAndCast(IGESDimen_OrdinateDimension, anent, ent);
      if (anent.IsNull())
        break;
      OrdinateDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 19: {
      DeclareAndCast(IGESDimen_PointDimension, anent, ent);
      if (anent.IsNull())
        break;
      PointDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 20: {
      DeclareAndCast(IGESDimen_RadiusDimension, anent, ent);
      if (anent.IsNull())
        break;
      RadiusDimensionTool tool;
      return tool.DirChecker(anent);
    }
    case 21: {
      DeclareAndCast(IGESDimen_Section, anent, ent);
      if (anent.IsNull())
        break;
      SectionTool tool;
      return tool.DirChecker(anent);
    }
    case 22: {
      DeclareAndCast(IGESDimen_SectionedArea, anent, ent);
      if (anent.IsNull())
        break;
      SectionedAreaTool tool;
      return tool.DirChecker(anent);
    }
    case 23: {
      DeclareAndCast(IGESDimen_WitnessLine, anent, ent);
      if (anent.IsNull())
        break;
      WitnessLineTool tool;
      return tool.DirChecker(anent);
    }
    default:
      break;
  }
  return DirectoryChecker(); // by default, no specific criterium
}

void IGESDimen_GeneralModule::OwnCheckCase(const Standard_Integer             CN,
                                           const Handle(IGESData_IGESEntity)& ent,
                                           const Interface_ShareTool&         shares,
                                           Handle(Interface_Check)&           ach) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDimen_AngularDimension, anent, ent);
      if (anent.IsNull())
        return;
      AngularDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 2: {
      DeclareAndCast(IGESDimen_BasicDimension, anent, ent);
      if (anent.IsNull())
        return;
      BasicDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 3: {
      DeclareAndCast(IGESDimen_CenterLine, anent, ent);
      if (anent.IsNull())
        return;
      CenterLineTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 4: {
      DeclareAndCast(IGESDimen_CurveDimension, anent, ent);
      if (anent.IsNull())
        return;
      CurveDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 5: {
      DeclareAndCast(IGESDimen_DiameterDimension, anent, ent);
      if (anent.IsNull())
        return;
      DiameterDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 6: {
      DeclareAndCast(IGESDimen_DimensionDisplayData, anent, ent);
      if (anent.IsNull())
        return;
      DimensionDisplayDataTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 7: {
      DeclareAndCast(IGESDimen_DimensionTolerance, anent, ent);
      if (anent.IsNull())
        return;
      DimensionToleranceTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 8: {
      DeclareAndCast(IGESDimen_DimensionUnits, anent, ent);
      if (anent.IsNull())
        return;
      DimensionUnitsTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 9: {
      DeclareAndCast(IGESDimen_DimensionedGeometry, anent, ent);
      if (anent.IsNull())
        return;
      DimensionedGeometryTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 10: {
      DeclareAndCast(IGESDimen_FlagNote, anent, ent);
      if (anent.IsNull())
        return;
      FlagNoteTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 11: {
      DeclareAndCast(IGESDimen_GeneralLabel, anent, ent);
      if (anent.IsNull())
        return;
      GeneralLabelTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 12: {
      DeclareAndCast(IGESDimen_GeneralNote, anent, ent);
      if (anent.IsNull())
        return;
      GeneralNoteTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 13: {
      DeclareAndCast(IGESDimen_GeneralSymbol, anent, ent);
      if (anent.IsNull())
        return;
      GeneralSymbolTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDimen_LeaderArrow, anent, ent);
      if (anent.IsNull())
        return;
      LeaderArrowTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 15: {
      DeclareAndCast(IGESDimen_LinearDimension, anent, ent);
      if (anent.IsNull())
        return;
      LinearDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 16: {
      DeclareAndCast(IGESDimen_NewDimensionedGeometry, anent, ent);
      if (anent.IsNull())
        return;
      NewDimensionedGeometryTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 17: {
      DeclareAndCast(IGESDimen_NewGeneralNote, anent, ent);
      if (anent.IsNull())
        return;
      NewGeneralNoteTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 18: {
      DeclareAndCast(IGESDimen_OrdinateDimension, anent, ent);
      if (anent.IsNull())
        return;
      OrdinateDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 19: {
      DeclareAndCast(IGESDimen_PointDimension, anent, ent);
      if (anent.IsNull())
        return;
      PointDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 20: {
      DeclareAndCast(IGESDimen_RadiusDimension, anent, ent);
      if (anent.IsNull())
        return;
      RadiusDimensionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 21: {
      DeclareAndCast(IGESDimen_Section, anent, ent);
      if (anent.IsNull())
        return;
      SectionTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 22: {
      DeclareAndCast(IGESDimen_SectionedArea, anent, ent);
      if (anent.IsNull())
        return;
      SectionedAreaTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 23: {
      DeclareAndCast(IGESDimen_WitnessLine, anent, ent);
      if (anent.IsNull())
        return;
      WitnessLineTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    default:
      break;
  }
}

Standard_Boolean IGESDimen_GeneralModule::NewVoid(const Standard_Integer      CN,
                                                  Handle(RefObject)& ent) const
{
  switch (CN)
  {
    case 1:
      ent = new IGESDimen_AngularDimension;
      break;
    case 2:
      ent = new IGESDimen_BasicDimension;
      break;
    case 3:
      ent = new IGESDimen_CenterLine;
      break;
    case 4:
      ent = new IGESDimen_CurveDimension;
      break;
    case 5:
      ent = new IGESDimen_DiameterDimension;
      break;
    case 6:
      ent = new IGESDimen_DimensionDisplayData;
      break;
    case 7:
      ent = new IGESDimen_DimensionTolerance;
      break;
    case 8:
      ent = new IGESDimen_DimensionUnits;
      break;
    case 9:
      ent = new IGESDimen_DimensionedGeometry;
      break;
    case 10:
      ent = new IGESDimen_FlagNote;
      break;
    case 11:
      ent = new IGESDimen_GeneralLabel;
      break;
    case 12:
      ent = new IGESDimen_GeneralNote;
      break;
    case 13:
      ent = new IGESDimen_GeneralSymbol;
      break;
    case 14:
      ent = new IGESDimen_LeaderArrow;
      break;
    case 15:
      ent = new IGESDimen_LinearDimension;
      break;
    case 16:
      ent = new IGESDimen_NewDimensionedGeometry;
      break;
    case 17:
      ent = new IGESDimen_NewGeneralNote;
      break;
    case 18:
      ent = new IGESDimen_OrdinateDimension;
      break;
    case 19:
      ent = new IGESDimen_PointDimension;
      break;
    case 20:
      ent = new IGESDimen_RadiusDimension;
      break;
    case 21:
      ent = new IGESDimen_Section;
      break;
    case 22:
      ent = new IGESDimen_SectionedArea;
      break;
    case 23:
      ent = new IGESDimen_WitnessLine;
      break;
    default:
      return Standard_False; // by default, Failure on Recognize
  }
  return Standard_True;
}

void IGESDimen_GeneralModule::OwnCopyCase(const Standard_Integer             CN,
                                          const Handle(IGESData_IGESEntity)& entfrom,
                                          const Handle(IGESData_IGESEntity)& entto,
                                          Interface_CopyTool&                TC) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDimen_AngularDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_AngularDimension, ento, entto);
      AngularDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 2: {
      DeclareAndCast(IGESDimen_BasicDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_BasicDimension, ento, entto);
      BasicDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 3: {
      DeclareAndCast(IGESDimen_CenterLine, enfr, entfrom);
      DeclareAndCast(IGESDimen_CenterLine, ento, entto);
      CenterLineTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 4: {
      DeclareAndCast(IGESDimen_CurveDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_CurveDimension, ento, entto);
      CurveDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 5: {
      DeclareAndCast(IGESDimen_DiameterDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_DiameterDimension, ento, entto);
      DiameterDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 6: {
      DeclareAndCast(IGESDimen_DimensionDisplayData, enfr, entfrom);
      DeclareAndCast(IGESDimen_DimensionDisplayData, ento, entto);
      DimensionDisplayDataTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 7: {
      DeclareAndCast(IGESDimen_DimensionTolerance, enfr, entfrom);
      DeclareAndCast(IGESDimen_DimensionTolerance, ento, entto);
      DimensionToleranceTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 8: {
      DeclareAndCast(IGESDimen_DimensionUnits, enfr, entfrom);
      DeclareAndCast(IGESDimen_DimensionUnits, ento, entto);
      DimensionUnitsTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 9: {
      DeclareAndCast(IGESDimen_DimensionedGeometry, enfr, entfrom);
      DeclareAndCast(IGESDimen_DimensionedGeometry, ento, entto);
      DimensionedGeometryTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 10: {
      DeclareAndCast(IGESDimen_FlagNote, enfr, entfrom);
      DeclareAndCast(IGESDimen_FlagNote, ento, entto);
      FlagNoteTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 11: {
      DeclareAndCast(IGESDimen_GeneralLabel, enfr, entfrom);
      DeclareAndCast(IGESDimen_GeneralLabel, ento, entto);
      GeneralLabelTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 12: {
      DeclareAndCast(IGESDimen_GeneralNote, enfr, entfrom);
      DeclareAndCast(IGESDimen_GeneralNote, ento, entto);
      GeneralNoteTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 13: {
      DeclareAndCast(IGESDimen_GeneralSymbol, enfr, entfrom);
      DeclareAndCast(IGESDimen_GeneralSymbol, ento, entto);
      GeneralSymbolTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDimen_LeaderArrow, enfr, entfrom);
      DeclareAndCast(IGESDimen_LeaderArrow, ento, entto);
      LeaderArrowTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 15: {
      DeclareAndCast(IGESDimen_LinearDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_LinearDimension, ento, entto);
      LinearDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 16: {
      DeclareAndCast(IGESDimen_NewDimensionedGeometry, enfr, entfrom);
      DeclareAndCast(IGESDimen_NewDimensionedGeometry, ento, entto);
      NewDimensionedGeometryTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 17: {
      DeclareAndCast(IGESDimen_NewGeneralNote, enfr, entfrom);
      DeclareAndCast(IGESDimen_NewGeneralNote, ento, entto);
      NewGeneralNoteTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 18: {
      DeclareAndCast(IGESDimen_OrdinateDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_OrdinateDimension, ento, entto);
      OrdinateDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 19: {
      DeclareAndCast(IGESDimen_PointDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_PointDimension, ento, entto);
      PointDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 20: {
      DeclareAndCast(IGESDimen_RadiusDimension, enfr, entfrom);
      DeclareAndCast(IGESDimen_RadiusDimension, ento, entto);
      RadiusDimensionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 21: {
      DeclareAndCast(IGESDimen_Section, enfr, entfrom);
      DeclareAndCast(IGESDimen_Section, ento, entto);
      SectionTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 22: {
      DeclareAndCast(IGESDimen_SectionedArea, enfr, entfrom);
      DeclareAndCast(IGESDimen_SectionedArea, ento, entto);
      SectionedAreaTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 23: {
      DeclareAndCast(IGESDimen_WitnessLine, enfr, entfrom);
      DeclareAndCast(IGESDimen_WitnessLine, ento, entto);
      WitnessLineTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    default:
      break;
  }
}

Standard_Integer IGESDimen_GeneralModule::CategoryNumber(const Standard_Integer /*CN*/,
                                                         const Handle(RefObject)&,
                                                         const Interface_ShareTool&) const
{
  return Interface_Category::Number("Drawing");
}
