// Created by: CKY / Contract Toubro-Larsen
// Copyright (c) 1993-1999 Matra Datavision
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

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#include <IGESBasic_ExternalRefLibName.hxx>
#include <IGESBasic_ToolExternalRefLibName.hxx>
#include <IGESData_DirChecker.hxx>
#include <IGESData_Dump.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESReaderData.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_ParamReader.hxx>
#include <Interface_Check.hxx>
#include <Interface_CopyTool.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_ShareTool.hxx>
#include <TCollection_HAsciiString.hxx>

ExternalRefLibNameTool::ExternalRefLibNameTool() {}

void ExternalRefLibNameTool::ReadOwnParams(
  const Handle(IGESBasic_ExternalRefLibName)& ent,
  const Handle(IGESData_IGESReaderData)& /* IR */,
  IGESData_ParamReader& PR) const
{
  // Standard_Boolean st; //szv#4:S4163:12Mar99 not needed
  Handle(TCollection_HAsciiString) tempLibName;
  Handle(TCollection_HAsciiString) tempExtRefEntitySymbName;
  // clang-format off
  PR.ReadText(PR.Current(), "Name of Library", tempLibName); //szv#4:S4163:12Mar99 `st=` not needed
  // clang-format on
  PR.ReadText(PR.Current(),
              "External Reference1 Symbolic Name",
              tempExtRefEntitySymbName); // szv#4:S4163:12Mar99 `st=` not needed

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(), ent);
  ent->Init(tempLibName, tempExtRefEntitySymbName);
}

void ExternalRefLibNameTool::WriteOwnParams(
  const Handle(IGESBasic_ExternalRefLibName)& ent,
  IGESData_IGESWriter&                        IW) const
{
  IW.Send(ent->LibraryName());
  IW.Send(ent->ReferenceName());
}

void ExternalRefLibNameTool::OwnShared(
  const Handle(IGESBasic_ExternalRefLibName)& /* ent */,
  Interface_EntityIterator& /* iter */) const
{
}

void ExternalRefLibNameTool::OwnCopy(const Handle(IGESBasic_ExternalRefLibName)& another,
                                               const Handle(IGESBasic_ExternalRefLibName)& ent,
                                               Interface_CopyTool& /* TC */) const
{
  Handle(TCollection_HAsciiString) tempLibName =
    new TCollection_HAsciiString(another->LibraryName());
  Handle(TCollection_HAsciiString) tempRefName =
    new TCollection_HAsciiString(another->ReferenceName());
  ent->Init(tempLibName, tempRefName);
}

DirectoryChecker ExternalRefLibNameTool::DirChecker(
  const Handle(IGESBasic_ExternalRefLibName)& /* ent */) const
{
  DirectoryChecker DC(416, 4);
  DC.Structure(IGESData_DefVoid);
  DC.GraphicsIgnored();
  DC.LineFont(IGESData_DefVoid);
  DC.LineWeight(IGESData_DefVoid);
  DC.Color(IGESData_DefVoid);
  DC.BlankStatusRequired(0);
  DC.HierarchyStatusRequired(0);
  return DC;
}

void ExternalRefLibNameTool::OwnCheck(
  const Handle(IGESBasic_ExternalRefLibName)& /* ent */,
  const Interface_ShareTool&,
  Handle(Interface_Check)& /* ach */) const
{
}

void ExternalRefLibNameTool::OwnDump(const Handle(IGESBasic_ExternalRefLibName)& ent,
                                               const IGESData_IGESDumper& /* dumper */,
                                               Standard_OStream& S,
                                               const Standard_Integer /* level */) const
{
  S << "IGESBasic_ExternalRefLibName\n"
    << "Name of Library : ";
  IGESData_DumpString(S, ent->LibraryName());
  S << "\n"
    << "External Reference1 Symbolic Name : ";
  IGESData_DumpString(S, ent->ReferenceName());
  S << std::endl;
}
