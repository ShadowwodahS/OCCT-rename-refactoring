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

#include <IGESData_IGESDumper.hxx>
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
#include <IGESDimen_SpecificModule.hxx>
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
#include <Interface_Macros.hxx>
#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESDimen_SpecificModule, IGESData_SpecificModule)

//  Each Module is attached to a Protocol : it must interpret Case Numbers
//  (arguments <CN> of various methods) in accordance to values returned by
//  the method TypeNumber from this Protocol
IGESDimen_SpecificModule::IGESDimen_SpecificModule() {}

void IGESDimen_SpecificModule::OwnDump(const Standard_Integer             CN,
                                       const Handle(IGESData_IGESEntity)& ent,
                                       const IGESData_IGESDumper&         dumper,
                                       Standard_OStream&                  S,
                                       const Standard_Integer             own) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESDimen_AngularDimension, anent, ent);
      if (anent.IsNull())
        return;
      AngularDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 2: {
      DeclareAndCast(IGESDimen_BasicDimension, anent, ent);
      if (anent.IsNull())
        return;
      BasicDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 3: {
      DeclareAndCast(IGESDimen_CenterLine, anent, ent);
      if (anent.IsNull())
        return;
      CenterLineTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 4: {
      DeclareAndCast(IGESDimen_CurveDimension, anent, ent);
      if (anent.IsNull())
        return;
      CurveDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 5: {
      DeclareAndCast(IGESDimen_DiameterDimension, anent, ent);
      if (anent.IsNull())
        return;
      DiameterDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 6: {
      DeclareAndCast(IGESDimen_DimensionDisplayData, anent, ent);
      if (anent.IsNull())
        return;
      DimensionDisplayDataTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 7: {
      DeclareAndCast(IGESDimen_DimensionTolerance, anent, ent);
      if (anent.IsNull())
        return;
      DimensionToleranceTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 8: {
      DeclareAndCast(IGESDimen_DimensionUnits, anent, ent);
      if (anent.IsNull())
        return;
      DimensionUnitsTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 9: {
      DeclareAndCast(IGESDimen_DimensionedGeometry, anent, ent);
      if (anent.IsNull())
        return;
      DimensionedGeometryTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 10: {
      DeclareAndCast(IGESDimen_FlagNote, anent, ent);
      if (anent.IsNull())
        return;
      FlagNoteTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 11: {
      DeclareAndCast(IGESDimen_GeneralLabel, anent, ent);
      if (anent.IsNull())
        return;
      GeneralLabelTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 12: {
      DeclareAndCast(IGESDimen_GeneralNote, anent, ent);
      if (anent.IsNull())
        return;
      GeneralNoteTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 13: {
      DeclareAndCast(IGESDimen_GeneralSymbol, anent, ent);
      if (anent.IsNull())
        return;
      GeneralSymbolTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 14: {
      DeclareAndCast(IGESDimen_LeaderArrow, anent, ent);
      if (anent.IsNull())
        return;
      LeaderArrowTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 15: {
      DeclareAndCast(IGESDimen_LinearDimension, anent, ent);
      if (anent.IsNull())
        return;
      LinearDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 16: {
      DeclareAndCast(IGESDimen_NewDimensionedGeometry, anent, ent);
      if (anent.IsNull())
        return;
      NewDimensionedGeometryTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 17: {
      DeclareAndCast(IGESDimen_NewGeneralNote, anent, ent);
      if (anent.IsNull())
        return;
      NewGeneralNoteTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 18: {
      DeclareAndCast(IGESDimen_OrdinateDimension, anent, ent);
      if (anent.IsNull())
        return;
      OrdinateDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 19: {
      DeclareAndCast(IGESDimen_PointDimension, anent, ent);
      if (anent.IsNull())
        return;
      PointDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 20: {
      DeclareAndCast(IGESDimen_RadiusDimension, anent, ent);
      if (anent.IsNull())
        return;
      RadiusDimensionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 21: {
      DeclareAndCast(IGESDimen_Section, anent, ent);
      if (anent.IsNull())
        return;
      SectionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 22: {
      DeclareAndCast(IGESDimen_SectionedArea, anent, ent);
      if (anent.IsNull())
        return;
      SectionedAreaTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 23: {
      DeclareAndCast(IGESDimen_WitnessLine, anent, ent);
      if (anent.IsNull())
        return;
      WitnessLineTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    default:
      break;
  }
}

Standard_Boolean IGESDimen_SpecificModule::OwnCorrect(const Standard_Integer             CN,
                                                      const Handle(IGESData_IGESEntity)& ent) const
{
  //   Applies only on some types
  switch (CN)
  {
    case 2: {
      DeclareAndCast(IGESDimen_BasicDimension, anent, ent);
      if (anent.IsNull())
        break;
      BasicDimensionTool tool;
      return tool.OwnCorrect(anent);
    }
    case 3: {
      DeclareAndCast(IGESDimen_CenterLine, anent, ent);
      if (anent.IsNull())
        break;
      CenterLineTool tool;
      return tool.OwnCorrect(anent);
    }
    case 6: {
      DeclareAndCast(IGESDimen_DimensionDisplayData, anent, ent);
      if (anent.IsNull())
        break;
      DimensionDisplayDataTool tool;
      return tool.OwnCorrect(anent);
    }
    case 7: {
      DeclareAndCast(IGESDimen_DimensionTolerance, anent, ent);
      if (anent.IsNull())
        break;
      DimensionToleranceTool tool;
      return tool.OwnCorrect(anent);
    }
    case 8: {
      DeclareAndCast(IGESDimen_DimensionUnits, anent, ent);
      if (anent.IsNull())
        break;
      DimensionUnitsTool tool;
      return tool.OwnCorrect(anent);
    }
    case 9: {
      DeclareAndCast(IGESDimen_DimensionedGeometry, anent, ent);
      if (anent.IsNull())
        break;
      DimensionedGeometryTool tool;
      return tool.OwnCorrect(anent);
    }
    case 16: {
      DeclareAndCast(IGESDimen_NewDimensionedGeometry, anent, ent);
      if (anent.IsNull())
        break;
      NewDimensionedGeometryTool tool;
      return tool.OwnCorrect(anent);
    }
    case 21: {
      DeclareAndCast(IGESDimen_Section, anent, ent);
      if (anent.IsNull())
        break;
      SectionTool tool;
      return tool.OwnCorrect(anent);
    }
    case 23: {
      DeclareAndCast(IGESDimen_WitnessLine, anent, ent);
      if (anent.IsNull())
        break;
      WitnessLineTool tool;
      return tool.OwnCorrect(anent);
    }
    default:
      break;
  }
  return Standard_False;
}
