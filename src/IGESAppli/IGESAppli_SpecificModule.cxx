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

#include <IGESAppli_DrilledHole.hxx>
#include <IGESAppli_ElementResults.hxx>
#include <IGESAppli_FiniteElement.hxx>
#include <IGESAppli_Flow.hxx>
#include <IGESAppli_FlowLineSpec.hxx>
#include <IGESAppli_LevelFunction.hxx>
#include <IGESAppli_LevelToPWBLayerMap.hxx>
#include <IGESAppli_LineWidening.hxx>
#include <IGESAppli_NodalConstraint.hxx>
#include <IGESAppli_NodalDisplAndRot.hxx>
#include <IGESAppli_NodalResults.hxx>
#include <IGESAppli_Node.hxx>
#include <IGESAppli_PartNumber.hxx>
#include <IGESAppli_PinNumber.hxx>
#include <IGESAppli_PipingFlow.hxx>
#include <IGESAppli_PWBArtworkStackup.hxx>
#include <IGESAppli_PWBDrilledHole.hxx>
#include <IGESAppli_ReferenceDesignator.hxx>
#include <IGESAppli_RegionRestriction.hxx>
#include <IGESAppli_SpecificModule.hxx>
#include <IGESAppli_ToolDrilledHole.hxx>
#include <IGESAppli_ToolElementResults.hxx>
#include <IGESAppli_ToolFiniteElement.hxx>
#include <IGESAppli_ToolFlow.hxx>
#include <IGESAppli_ToolFlowLineSpec.hxx>
#include <IGESAppli_ToolLevelFunction.hxx>
#include <IGESAppli_ToolLevelToPWBLayerMap.hxx>
#include <IGESAppli_ToolLineWidening.hxx>
#include <IGESAppli_ToolNodalConstraint.hxx>
#include <IGESAppli_ToolNodalDisplAndRot.hxx>
#include <IGESAppli_ToolNodalResults.hxx>
#include <IGESAppli_ToolNode.hxx>
#include <IGESAppli_ToolPartNumber.hxx>
#include <IGESAppli_ToolPinNumber.hxx>
#include <IGESAppli_ToolPipingFlow.hxx>
#include <IGESAppli_ToolPWBArtworkStackup.hxx>
#include <IGESAppli_ToolPWBDrilledHole.hxx>
#include <IGESAppli_ToolReferenceDesignator.hxx>
#include <IGESAppli_ToolRegionRestriction.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESEntity.hxx>
#include <Interface_Macros.hxx>
#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESAppli_SpecificModule, IGESData_SpecificModule)

//  Each Module is attached to a Protocol : it must interpret Case Numbers1
//  (arguments <CN> of various methods) in accordance to values returned by
//  the method TypeNumber from this Protocol
IGESAppli_SpecificModule::IGESAppli_SpecificModule() {}

void IGESAppli_SpecificModule::OwnDump(const Standard_Integer             CN,
                                       const Handle(IGESData_IGESEntity)& ent,
                                       const IGESData_IGESDumper&         dumper,
                                       Standard_OStream&                  S,
                                       const Standard_Integer             own) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESAppli_DrilledHole, anent, ent);
      if (anent.IsNull())
        return;
      DrilledHoleTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 2: {
      DeclareAndCast(IGESAppli_ElementResults, anent, ent);
      if (anent.IsNull())
        return;
      ElementResultsTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 3: {
      DeclareAndCast(IGESAppli_FiniteElement, anent, ent);
      if (anent.IsNull())
        return;
      FiniteElementTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 4: {
      DeclareAndCast(IGESAppli_Flow, anent, ent);
      if (anent.IsNull())
        return;
      FlowTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 5: {
      DeclareAndCast(IGESAppli_FlowLineSpec, anent, ent);
      if (anent.IsNull())
        return;
      FlowLineSpecTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 6: {
      DeclareAndCast(IGESAppli_LevelFunction, anent, ent);
      if (anent.IsNull())
        return;
      LevelFunctionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 7: {
      DeclareAndCast(IGESAppli_LevelToPWBLayerMap, anent, ent);
      if (anent.IsNull())
        return;
      LevelToPWBLayerMapTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 8: {
      DeclareAndCast(IGESAppli_LineWidening, anent, ent);
      if (anent.IsNull())
        return;
      LineWideningTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 9: {
      DeclareAndCast(IGESAppli_NodalConstraint, anent, ent);
      if (anent.IsNull())
        return;
      NodalConstraintTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 10: {
      DeclareAndCast(IGESAppli_NodalDisplAndRot, anent, ent);
      if (anent.IsNull())
        return;
      NodalDisplRotTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 11: {
      DeclareAndCast(IGESAppli_NodalResults, anent, ent);
      if (anent.IsNull())
        return;
      NodalResultsTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 12: {
      DeclareAndCast(IGESAppli_Node, anent, ent);
      if (anent.IsNull())
        return;
      NodeTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 13: {
      DeclareAndCast(IGESAppli_PWBArtworkStackup, anent, ent);
      if (anent.IsNull())
        return;
      PWBArtworkStackupTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 14: {
      DeclareAndCast(IGESAppli_PWBDrilledHole, anent, ent);
      if (anent.IsNull())
        return;
      PWBDrilledHoleTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 15: {
      DeclareAndCast(IGESAppli_PartNumber, anent, ent);
      if (anent.IsNull())
        return;
      PartNumberTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 16: {
      DeclareAndCast(IGESAppli_PinNumber, anent, ent);
      if (anent.IsNull())
        return;
      PinNumberTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 17: {
      DeclareAndCast(IGESAppli_PipingFlow, anent, ent);
      if (anent.IsNull())
        return;
      PipingFlowTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 18: {
      DeclareAndCast(IGESAppli_ReferenceDesignator, anent, ent);
      if (anent.IsNull())
        return;
      ReferenceDesignatorTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    case 19: {
      DeclareAndCast(IGESAppli_RegionRestriction, anent, ent);
      if (anent.IsNull())
        return;
      RegionRestrictionTool tool;
      tool.OwnDump(anent, dumper, S, own);
    }
    break;
    default:
      break;
  }
}

Standard_Boolean IGESAppli_SpecificModule::OwnCorrect(const Standard_Integer             CN,
                                                      const Handle(IGESData_IGESEntity)& ent) const
{
  //   Applies only on some types
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESAppli_DrilledHole, anent, ent);
      if (anent.IsNull())
        break;
      DrilledHoleTool tool;
      return tool.OwnCorrect(anent);
    }
    case 4: {
      DeclareAndCast(IGESAppli_Flow, anent, ent);
      if (anent.IsNull())
        break;
      FlowTool tool;
      return tool.OwnCorrect(anent);
    }
    case 6: {
      DeclareAndCast(IGESAppli_LevelFunction, anent, ent);
      if (anent.IsNull())
        break;
      LevelFunctionTool tool;
      return tool.OwnCorrect(anent);
    }
    case 8: {
      DeclareAndCast(IGESAppli_LineWidening, anent, ent);
      if (anent.IsNull())
        break;
      LineWideningTool tool;
      return tool.OwnCorrect(anent);
    }
    case 14: {
      DeclareAndCast(IGESAppli_PWBDrilledHole, anent, ent);
      if (anent.IsNull())
        break;
      PWBDrilledHoleTool tool;
      return tool.OwnCorrect(anent);
    }
    case 15: {
      DeclareAndCast(IGESAppli_PartNumber, anent, ent);
      if (anent.IsNull())
        break;
      PartNumberTool tool;
      return tool.OwnCorrect(anent);
    }
    case 16: {
      DeclareAndCast(IGESAppli_PinNumber, anent, ent);
      if (anent.IsNull())
        break;
      PinNumberTool tool;
      return tool.OwnCorrect(anent);
    }
    case 17: {
      DeclareAndCast(IGESAppli_PipingFlow, anent, ent);
      if (anent.IsNull())
        break;
      PipingFlowTool tool;
      return tool.OwnCorrect(anent);
    }
    case 18: {
      DeclareAndCast(IGESAppli_ReferenceDesignator, anent, ent);
      if (anent.IsNull())
        break;
      ReferenceDesignatorTool tool;
      return tool.OwnCorrect(anent);
    }
    case 19: {
      DeclareAndCast(IGESAppli_RegionRestriction, anent, ent);
      if (anent.IsNull())
        break;
      RegionRestrictionTool tool;
      return tool.OwnCorrect(anent);
    }
    default:
      break;
  }
  return Standard_False;
}
