// Created on: 1998-06-29
// Created by: Denis PASCAL
// Copyright (c) 1998-1999 Matra Datavision
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

#include <AppStd_Application.hxx>
#include <DDataStd.hxx>
#include <DDF.hxx>
#include <DDocStd.hxx>
#include <DNaming.hxx>
#include <DPrsStd.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_PluginMacro.hxx>

//=================================================================================================

void DPrsStd1::AllCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  DPrsStd1::AISPresentationCommands(theCommands);
  DPrsStd1::AISViewerCommands(theCommands);
  // DPrsStd1::BasicCommands(theCommands);
}

//==============================================================================
// DPrsStd1::Factory
//==============================================================================
void DPrsStd1::Factory(DrawInterpreter& theDI)
{
  static Standard_Boolean DPrsStdFactoryDone = Standard_False;
  if (DPrsStdFactoryDone)
    return;
  DPrsStdFactoryDone = Standard_True;

  DDF1::AllCommands(theDI);
  DNaming1::AllCommands(theDI);
  DDataStd1::AllCommands(theDI);
  DPrsStd1::AllCommands(theDI);
  DDocStd1::AllCommands(theDI);
#ifdef OCCT_DEBUG
  std::cout << "Draw1 Plugin : All DF commands are loaded" << std::endl;
#endif
}

// Declare entry point PLUGINFACTORY
DPLUGIN(DPrsStd1)
