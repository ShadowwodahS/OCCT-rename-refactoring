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

#include <IGESBasic_AssocGroupType.hxx>
#include <IGESBasic_ExternalReferenceFile.hxx>
#include <IGESBasic_ExternalRefFile.hxx>
#include <IGESBasic_ExternalRefFileIndex.hxx>
#include <IGESBasic_ExternalRefFileName.hxx>
#include <IGESBasic_ExternalRefLibName.hxx>
#include <IGESBasic_ExternalRefName.hxx>
#include <IGESBasic_GeneralModule.hxx>
#include <IGESBasic_GroupWithoutBackP.hxx>
#include <IGESBasic_Hierarchy.hxx>
#include <IGESBasic_Name.hxx>
#include <IGESBasic_OrderedGroup.hxx>
#include <IGESBasic_OrderedGroupWithoutBackP.hxx>
#include <IGESBasic_SingleParent.hxx>
#include <IGESBasic_SingularSubfigure.hxx>
#include <IGESBasic_SubfigureDef.hxx>
#include <IGESBasic_ToolAssocGroupType.hxx>
#include <IGESBasic_ToolExternalReferenceFile.hxx>
#include <IGESBasic_ToolExternalRefFile.hxx>
#include <IGESBasic_ToolExternalRefFileIndex.hxx>
#include <IGESBasic_ToolExternalRefFileName.hxx>
#include <IGESBasic_ToolExternalRefLibName.hxx>
#include <IGESBasic_ToolExternalRefName.hxx>
#include <IGESBasic_ToolGroup.hxx>
#include <IGESBasic_ToolGroupWithoutBackP.hxx>
#include <IGESBasic_ToolHierarchy.hxx>
#include <IGESBasic_ToolName.hxx>
#include <IGESBasic_ToolOrderedGroup.hxx>
#include <IGESBasic_ToolOrderedGroupWithoutBackP.hxx>
#include <IGESBasic_ToolSingleParent.hxx>
#include <IGESBasic_ToolSingularSubfigure.hxx>
#include <IGESBasic_ToolSubfigureDef.hxx>
#include <IGESData_DirChecker.hxx>
#include <IGESData_IGESEntity.hxx>
#include <Interface_Category.hxx>
#include <Interface_Check.hxx>
#include <Interface_CopyTool.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESBasic_GeneralModule, IGESData_GeneralModule)

//  Each Module is attached to a Protocol : it must interpret Case Numbers
//  (arguments <CN> of various methods) in accordance to values returned by
//  the method TypeNumber from this Protocol
IGESBasic_GeneralModule::IGESBasic_GeneralModule() {}

void IGESBasic_GeneralModule::OwnSharedCase(const Standard_Integer             CN,
                                            const Handle(IGESData_IGESEntity)& ent,
                                            Interface_EntityIterator&          iter) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESBasic_AssocGroupType, anent, ent);
      if (anent.IsNull())
        return;
      AssocGroupTypeTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 2: {
      DeclareAndCast(IGESBasic_ExternalRefFile, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefFileTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 3: {
      DeclareAndCast(IGESBasic_ExternalRefFileIndex, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefFileIndexTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 4: {
      DeclareAndCast(IGESBasic_ExternalRefFileName, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefFileNameTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 5: {
      DeclareAndCast(IGESBasic_ExternalRefLibName, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefLibNameTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 6: {
      DeclareAndCast(IGESBasic_ExternalRefName, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefNameTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 7: {
      DeclareAndCast(IGESBasic_ExternalReferenceFile, anent, ent);
      if (anent.IsNull())
        return;
      ExternalReferenceFileTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 8: {
      DeclareAndCast(IGESBasic_Group, anent, ent);
      if (anent.IsNull())
        return;
      GroupTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 9: {
      DeclareAndCast(IGESBasic_GroupWithoutBackP, anent, ent);
      if (anent.IsNull())
        return;
      GroupWithoutBackPTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 10: {
      DeclareAndCast(IGESBasic_Hierarchy, anent, ent);
      if (anent.IsNull())
        return;
      HierarchyTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 11: {
      DeclareAndCast(IGESBasic_Name, anent, ent);
      if (anent.IsNull())
        return;
      NameTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 12: {
      DeclareAndCast(IGESBasic_OrderedGroup, anent, ent);
      if (anent.IsNull())
        return;
      OrderedGroupTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 13: {
      DeclareAndCast(IGESBasic_OrderedGroupWithoutBackP, anent, ent);
      if (anent.IsNull())
        return;
      OrderedGroupWithoutBackPTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 14: {
      DeclareAndCast(IGESBasic_SingleParent, anent, ent);
      if (anent.IsNull())
        return;
      SingleParentTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 15: {
      DeclareAndCast(IGESBasic_SingularSubfigure, anent, ent);
      if (anent.IsNull())
        return;
      SingularSubfigureTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    case 16: {
      DeclareAndCast(IGESBasic_SubfigureDef, anent, ent);
      if (anent.IsNull())
        return;
      SubfigureDefTool tool;
      tool.OwnShared(anent, iter);
    }
    break;
    default:
      break;
  }
}

DirectoryChecker IGESBasic_GeneralModule::DirChecker(
  const Standard_Integer             CN,
  const Handle(IGESData_IGESEntity)& ent) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESBasic_AssocGroupType, anent, ent);
      if (anent.IsNull())
        break;
      AssocGroupTypeTool tool;
      return tool.DirChecker(anent);
    }
    case 2: {
      DeclareAndCast(IGESBasic_ExternalRefFile, anent, ent);
      if (anent.IsNull())
        break;
      ExternalRefFileTool tool;
      return tool.DirChecker(anent);
    }
    case 3: {
      DeclareAndCast(IGESBasic_ExternalRefFileIndex, anent, ent);
      if (anent.IsNull())
        break;
      ExternalRefFileIndexTool tool;
      return tool.DirChecker(anent);
    }
    case 4: {
      DeclareAndCast(IGESBasic_ExternalRefFileName, anent, ent);
      if (anent.IsNull())
        break;
      ExternalRefFileNameTool tool;
      return tool.DirChecker(anent);
    }
    case 5: {
      DeclareAndCast(IGESBasic_ExternalRefLibName, anent, ent);
      if (anent.IsNull())
        break;
      ExternalRefLibNameTool tool;
      return tool.DirChecker(anent);
    }
    case 6: {
      DeclareAndCast(IGESBasic_ExternalRefName, anent, ent);
      if (anent.IsNull())
        break;
      ExternalRefNameTool tool;
      return tool.DirChecker(anent);
    }
    case 7: {
      DeclareAndCast(IGESBasic_ExternalReferenceFile, anent, ent);
      if (anent.IsNull())
        break;
      ExternalReferenceFileTool tool;
      return tool.DirChecker(anent);
    }
    case 8: {
      DeclareAndCast(IGESBasic_Group, anent, ent);
      if (anent.IsNull())
        break;
      GroupTool tool;
      return tool.DirChecker(anent);
    }
    case 9: {
      DeclareAndCast(IGESBasic_GroupWithoutBackP, anent, ent);
      if (anent.IsNull())
        break;
      GroupWithoutBackPTool tool;
      return tool.DirChecker(anent);
    }
    case 10: {
      DeclareAndCast(IGESBasic_Hierarchy, anent, ent);
      if (anent.IsNull())
        break;
      HierarchyTool tool;
      return tool.DirChecker(anent);
    }
    case 11: {
      DeclareAndCast(IGESBasic_Name, anent, ent);
      if (anent.IsNull())
        break;
      NameTool tool;
      return tool.DirChecker(anent);
    }
    case 12: {
      DeclareAndCast(IGESBasic_OrderedGroup, anent, ent);
      if (anent.IsNull())
        break;
      OrderedGroupTool tool;
      return tool.DirChecker(anent);
    }
    case 13: {
      DeclareAndCast(IGESBasic_OrderedGroupWithoutBackP, anent, ent);
      if (anent.IsNull())
        break;
      OrderedGroupWithoutBackPTool tool;
      return tool.DirChecker(anent);
    }
    case 14: {
      DeclareAndCast(IGESBasic_SingleParent, anent, ent);
      if (anent.IsNull())
        break;
      SingleParentTool tool;
      return tool.DirChecker(anent);
    }
    case 15: {
      DeclareAndCast(IGESBasic_SingularSubfigure, anent, ent);
      if (anent.IsNull())
        break;
      SingularSubfigureTool tool;
      return tool.DirChecker(anent);
    }
    case 16: {
      DeclareAndCast(IGESBasic_SubfigureDef, anent, ent);
      if (anent.IsNull())
        break;
      SubfigureDefTool tool;
      return tool.DirChecker(anent);
    }
    default:
      break;
  }
  return DirectoryChecker(); // by default, no specific criterium
}

void IGESBasic_GeneralModule::OwnCheckCase(const Standard_Integer             CN,
                                           const Handle(IGESData_IGESEntity)& ent,
                                           const Interface_ShareTool&         shares,
                                           Handle(Interface_Check)&           ach) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESBasic_AssocGroupType, anent, ent);
      if (anent.IsNull())
        return;
      AssocGroupTypeTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 2: {
      DeclareAndCast(IGESBasic_ExternalRefFile, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefFileTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 3: {
      DeclareAndCast(IGESBasic_ExternalRefFileIndex, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefFileIndexTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 4: {
      DeclareAndCast(IGESBasic_ExternalRefFileName, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefFileNameTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 5: {
      DeclareAndCast(IGESBasic_ExternalRefLibName, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefLibNameTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 6: {
      DeclareAndCast(IGESBasic_ExternalRefName, anent, ent);
      if (anent.IsNull())
        return;
      ExternalRefNameTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 7: {
      DeclareAndCast(IGESBasic_ExternalReferenceFile, anent, ent);
      if (anent.IsNull())
        return;
      ExternalReferenceFileTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 8: {
      DeclareAndCast(IGESBasic_Group, anent, ent);
      if (anent.IsNull())
        return;
      GroupTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 9: {
      DeclareAndCast(IGESBasic_GroupWithoutBackP, anent, ent);
      if (anent.IsNull())
        return;
      GroupWithoutBackPTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 10: {
      DeclareAndCast(IGESBasic_Hierarchy, anent, ent);
      if (anent.IsNull())
        return;
      HierarchyTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 11: {
      DeclareAndCast(IGESBasic_Name, anent, ent);
      if (anent.IsNull())
        return;
      NameTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 12: {
      DeclareAndCast(IGESBasic_OrderedGroup, anent, ent);
      if (anent.IsNull())
        return;
      OrderedGroupTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 13: {
      DeclareAndCast(IGESBasic_OrderedGroupWithoutBackP, anent, ent);
      if (anent.IsNull())
        return;
      OrderedGroupWithoutBackPTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 14: {
      DeclareAndCast(IGESBasic_SingleParent, anent, ent);
      if (anent.IsNull())
        return;
      SingleParentTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 15: {
      DeclareAndCast(IGESBasic_SingularSubfigure, anent, ent);
      if (anent.IsNull())
        return;
      SingularSubfigureTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    case 16: {
      DeclareAndCast(IGESBasic_SubfigureDef, anent, ent);
      if (anent.IsNull())
        return;
      SubfigureDefTool tool;
      tool.OwnCheck(anent, shares, ach);
    }
    break;
    default:
      break;
  }
}

Standard_Boolean IGESBasic_GeneralModule::NewVoid(const Standard_Integer      CN,
                                                  Handle(RefObject)& ent) const
{
  switch (CN)
  {
    case 1:
      ent = new IGESBasic_AssocGroupType;
      break;
    case 2:
      ent = new IGESBasic_ExternalRefFile;
      break;
    case 3:
      ent = new IGESBasic_ExternalRefFileIndex;
      break;
    case 4:
      ent = new IGESBasic_ExternalRefFileName;
      break;
    case 5:
      ent = new IGESBasic_ExternalRefLibName;
      break;
    case 6:
      ent = new IGESBasic_ExternalRefName;
      break;
    case 7:
      ent = new IGESBasic_ExternalReferenceFile;
      break;
    case 8:
      ent = new IGESBasic_Group;
      break;
    case 9:
      ent = new IGESBasic_GroupWithoutBackP;
      break;
    case 10:
      ent = new IGESBasic_Hierarchy;
      break;
    case 11:
      ent = new IGESBasic_Name;
      break;
    case 12:
      ent = new IGESBasic_OrderedGroup;
      break;
    case 13:
      ent = new IGESBasic_OrderedGroupWithoutBackP;
      break;
    case 14:
      ent = new IGESBasic_SingleParent;
      break;
    case 15:
      ent = new IGESBasic_SingularSubfigure;
      break;
    case 16:
      ent = new IGESBasic_SubfigureDef;
      break;
    default:
      return Standard_False; // by default, Failure on Recognize
  }
  return Standard_True;
}

void IGESBasic_GeneralModule::OwnCopyCase(const Standard_Integer             CN,
                                          const Handle(IGESData_IGESEntity)& entfrom,
                                          const Handle(IGESData_IGESEntity)& entto,
                                          Interface_CopyTool&                TC) const
{
  switch (CN)
  {
    case 1: {
      DeclareAndCast(IGESBasic_AssocGroupType, enfr, entfrom);
      DeclareAndCast(IGESBasic_AssocGroupType, ento, entto);
      AssocGroupTypeTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 2: {
      DeclareAndCast(IGESBasic_ExternalRefFile, enfr, entfrom);
      DeclareAndCast(IGESBasic_ExternalRefFile, ento, entto);
      ExternalRefFileTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 3: {
      DeclareAndCast(IGESBasic_ExternalRefFileIndex, enfr, entfrom);
      DeclareAndCast(IGESBasic_ExternalRefFileIndex, ento, entto);
      ExternalRefFileIndexTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 4: {
      DeclareAndCast(IGESBasic_ExternalRefFileName, enfr, entfrom);
      DeclareAndCast(IGESBasic_ExternalRefFileName, ento, entto);
      ExternalRefFileNameTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 5: {
      DeclareAndCast(IGESBasic_ExternalRefLibName, enfr, entfrom);
      DeclareAndCast(IGESBasic_ExternalRefLibName, ento, entto);
      ExternalRefLibNameTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 6: {
      DeclareAndCast(IGESBasic_ExternalRefName, enfr, entfrom);
      DeclareAndCast(IGESBasic_ExternalRefName, ento, entto);
      ExternalRefNameTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 7: {
      DeclareAndCast(IGESBasic_ExternalReferenceFile, enfr, entfrom);
      DeclareAndCast(IGESBasic_ExternalReferenceFile, ento, entto);
      ExternalReferenceFileTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 8: {
      DeclareAndCast(IGESBasic_Group, enfr, entfrom);
      DeclareAndCast(IGESBasic_Group, ento, entto);
      GroupTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 9: {
      DeclareAndCast(IGESBasic_GroupWithoutBackP, enfr, entfrom);
      DeclareAndCast(IGESBasic_GroupWithoutBackP, ento, entto);
      GroupWithoutBackPTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 10: {
      DeclareAndCast(IGESBasic_Hierarchy, enfr, entfrom);
      DeclareAndCast(IGESBasic_Hierarchy, ento, entto);
      HierarchyTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 11: {
      DeclareAndCast(IGESBasic_Name, enfr, entfrom);
      DeclareAndCast(IGESBasic_Name, ento, entto);
      NameTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 12: {
      DeclareAndCast(IGESBasic_OrderedGroup, enfr, entfrom);
      DeclareAndCast(IGESBasic_OrderedGroup, ento, entto);
      OrderedGroupTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 13: {
      DeclareAndCast(IGESBasic_OrderedGroupWithoutBackP, enfr, entfrom);
      DeclareAndCast(IGESBasic_OrderedGroupWithoutBackP, ento, entto);
      OrderedGroupWithoutBackPTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 14: {
      DeclareAndCast(IGESBasic_SingleParent, enfr, entfrom);
      DeclareAndCast(IGESBasic_SingleParent, ento, entto);
      SingleParentTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 15: {
      DeclareAndCast(IGESBasic_SingularSubfigure, enfr, entfrom);
      DeclareAndCast(IGESBasic_SingularSubfigure, ento, entto);
      SingularSubfigureTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    case 16: {
      DeclareAndCast(IGESBasic_SubfigureDef, enfr, entfrom);
      DeclareAndCast(IGESBasic_SubfigureDef, ento, entto);
      SubfigureDefTool tool;
      tool.OwnCopy(enfr, ento, TC);
    }
    break;
    default:
      break;
  }
}

Standard_Integer IGESBasic_GeneralModule::CategoryNumber(const Standard_Integer CN,
                                                         const Handle(RefObject)&,
                                                         const Interface_ShareTool&) const
{
  if (CN == 10 || CN == 11)
    return Interface_Category::Number("Auxiliary");
  if (CN >= 2 && CN <= 6)
    return Interface_Category::Number("Description");
  return Interface_Category::Number("Structure");
}
