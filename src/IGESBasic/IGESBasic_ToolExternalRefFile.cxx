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

#include <IGESBasic_ExternalRefFile.hxx>
#include <IGESBasic_ToolExternalRefFile.hxx>
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

ExternalRefFileTool::ExternalRefFileTool() {}

void ExternalRefFileTool::ReadOwnParams(const Handle(IGESBasic_ExternalRefFile)& ent,
                                                  const Handle(IGESData_IGESReaderData)& /* IR */,
                                                  IGESData_ParamReader& PR) const
{
  // Standard_Boolean st; //szv#4:S4163:12Mar99 not needed
  Handle(TCollection_HAsciiString) tempExtRefFileIdentifier;
  PR.ReadText(PR.Current(),
              "External Reference File Identifier",
              tempExtRefFileIdentifier); // szv#4:S4163:12Mar99 `st=` not needed

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(), ent);
  ent->Init(tempExtRefFileIdentifier);
}

void ExternalRefFileTool::WriteOwnParams(const Handle(IGESBasic_ExternalRefFile)& ent,
                                                   IGESData_IGESWriter& IW) const
{
  IW.Send(ent->FileId());
}

void ExternalRefFileTool::OwnShared(const Handle(IGESBasic_ExternalRefFile)& /* ent */,
                                              Interface_EntityIterator& /* iter */) const
{
}

void ExternalRefFileTool::OwnCopy(const Handle(IGESBasic_ExternalRefFile)& another,
                                            const Handle(IGESBasic_ExternalRefFile)& ent,
                                            Interface_CopyTool& /* TC */) const
{
  Handle(TCollection_HAsciiString) tempFileId = new TCollection_HAsciiString(another->FileId());
  ent->Init(tempFileId);
}

DirectoryChecker ExternalRefFileTool::DirChecker(
  const Handle(IGESBasic_ExternalRefFile)& /* ent */) const
{
  DirectoryChecker DC(416, 1);
  DC.Structure(IGESData_DefVoid);
  DC.GraphicsIgnored();
  DC.LineFont(IGESData_DefVoid);
  DC.LineWeight(IGESData_DefVoid);
  DC.Color(IGESData_DefVoid);
  DC.BlankStatusIgnored();
  DC.HierarchyStatusIgnored();
  return DC;
}

void ExternalRefFileTool::OwnCheck(const Handle(IGESBasic_ExternalRefFile)& /* ent */,
                                             const Interface_ShareTool&,
                                             Handle(Interface_Check)& /* ach */) const
{
}

void ExternalRefFileTool::OwnDump(const Handle(IGESBasic_ExternalRefFile)& ent,
                                            const IGESData_IGESDumper& /* dumper */,
                                            Standard_OStream& S,
                                            const Standard_Integer /* level */) const
{
  S << "IGESBasic_ExternalRefFile\n"
    << "External Reference File Identifier : ";
  IGESData_DumpString(S, ent->FileId());
  S << std::endl;
}
