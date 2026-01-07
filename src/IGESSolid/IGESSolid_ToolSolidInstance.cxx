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

#include <IGESData_DirChecker.hxx>
#include <IGESData_IGESDumper.hxx>
#include <IGESData_IGESReaderData.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESData_ParamReader.hxx>
#include <IGESSolid_SolidInstance.hxx>
#include <IGESSolid_ToolSolidInstance.hxx>
#include <Interface_Check.hxx>
#include <Interface_CopyTool.hxx>
#include <Interface_EntityIterator.hxx>
#include <Interface_Macros.hxx>
#include <Interface_ShareTool.hxx>

SolidInstanceTool::SolidInstanceTool() {}

void SolidInstanceTool::ReadOwnParams(const Handle(IGESSolid_SolidInstance)& ent,
                                                const Handle(IGESData_IGESReaderData)& IR,
                                                IGESData_ParamReader&                  PR) const
{
  Handle(IGESData_IGESEntity) tempEntity;
  // Standard_Boolean st; //szv#4:S4163:12Mar99 not needed

  // clang-format off
  PR.ReadEntity(IR, PR.Current(), "Solid Entity", tempEntity); //szv#4:S4163:12Mar99 `st=` not needed
  // clang-format on

  DirChecker(ent).CheckTypeAndForm(PR.CCheck(), ent);
  ent->Init(tempEntity);
}

void SolidInstanceTool::WriteOwnParams(const Handle(IGESSolid_SolidInstance)& ent,
                                                 IGESData_IGESWriter&                   IW) const
{
  IW.Send(ent->Entity());
}

void SolidInstanceTool::OwnShared(const Handle(IGESSolid_SolidInstance)& ent,
                                            Interface_EntityIterator&              iter) const
{
  iter.GetOneItem(ent->Entity());
}

void SolidInstanceTool::OwnCopy(const Handle(IGESSolid_SolidInstance)& another,
                                          const Handle(IGESSolid_SolidInstance)& ent,
                                          Interface_CopyTool&                    TC) const
{
  DeclareAndCast(IGESData_IGESEntity, tempEntity, TC.Transferred(another->Entity()));
  ent->Init(tempEntity);
}

DirectoryChecker SolidInstanceTool::DirChecker(
  const Handle(IGESSolid_SolidInstance)& /*ent*/) const
{
  DirectoryChecker DC(430, 0, 1);

  DC.Structure(IGESData_DefVoid);
  DC.LineFont(IGESData_DefAny);
  DC.Color(IGESData_DefAny);

  DC.GraphicsIgnored(1);
  return DC;
}

void SolidInstanceTool::OwnCheck(const Handle(IGESSolid_SolidInstance)& /*ent*/,
                                           const Interface_ShareTool&,
                                           Handle(Interface_Check)& /*ach*/) const
{
}

void SolidInstanceTool::OwnDump(const Handle(IGESSolid_SolidInstance)& ent,
                                          const IGESData_IGESDumper&             dumper,
                                          Standard_OStream&                      S,
                                          const Standard_Integer                 level) const
{
  S << "IGESSolid_SolidInstance\n"
    << "Solid entity : ";
  dumper.Dump(ent->Entity(), S, (level <= 4) ? 0 : 1);
  S << std::endl;
}
