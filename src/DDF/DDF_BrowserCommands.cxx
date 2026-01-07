// Created by: DAUTRY Philippe
// Copyright (c) 1997-1999 Matra Datavision
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

#include <stdio.h>

#include <DDF.hxx>
#include <DDF_Browser.hxx>

#include <Draw_Appli.hxx>
#include <Draw_Interpretor.hxx>
#include <Message.hxx>

#include <TDF_Label.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>

#include <TCollection_AsciiString.hxx>
#include <OSD_File.hxx>

//=======================================================================
// function : DFBrowse
// purpose  :
//  arg 1  : DF name
// [arg 2] : Browser name
//=======================================================================

static Standard_Integer DFBrowse(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
  {
    Message::SendFail() << "Use: " << a[0] << " document [brower_name]";
    return 1;
  }

  Handle(TDF_Data) DF;
  if (!DDF1::GetDF(a[1], DF))
  {
    Message::SendFail() << "Error: document " << a[1] << " is not found";
    return 1;
  }

  Handle(DDF_Browser)     NewDDFBrowser = new DDF_Browser(DF);
  AsciiString1 name("browser_");
  name += ((n == 3) ? a[2] : a[1]);
  Draw1::Set(name.ToCString(), NewDDFBrowser);

  // Load Tcl Script
  AsciiString1 aTclScript(getenv("CSF_DrawPluginDefaults"));
  aTclScript.AssignCat("/dftree.tcl");
  SystemFile aTclScriptFile(aTclScript);
  if (aTclScriptFile.Exists())
  {
#ifdef OCCT_DEBUG
    std::cout << "Load " << aTclScript << std::endl;
#endif
    di.EvalFile(aTclScript.ToCString());
  }
  else
  {
    Message::SendFail() << "Error: Could not load script " << aTclScript << "\n"
                        << "Check environment variable CSF_DrawPluginDefaults";
  }

  // Call command dftree defined in dftree.tcl
  AsciiString1 aCommand = "dftree ";
  aCommand.AssignCat(name);
  di.Eval(aCommand.ToCString());
  return 0;
}

//=======================================================================
// function : DFOpenLabel
// purpose  :
//  arg 1  : Browser name
// [arg 2] : Label name
//=======================================================================

static Standard_Integer DFOpenLabel(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;

  Handle(DDF_Browser) browser = Handle(DDF_Browser)::DownCast(Draw1::GetExisting(a[1]));
  if (browser.IsNull())
  {
    Message::SendFail() << "Syntax error: browser '" << a[1] << "' not found";
    return 1;
  }

  DataLabel lab;
  if (n == 3)
    TDF_Tool::Label(browser->Data(), a[2], lab);

  AsciiString1 list(lab.IsNull() ? browser->OpenRoot() : browser->OpenLabel(lab));
  di << list.ToCString();
  return 0;
}

//=======================================================================
// function : DFOpenAttributeList
// purpose  :
//  arg 1  : Browser name
//  arg 2  : Label name
//=======================================================================

static Standard_Integer DFOpenAttributeList(DrawInterpreter& di,
                                            Standard_Integer  n,
                                            const char**      a)
{
  if (n < 3)
    return 1;

  Handle(DDF_Browser) browser = Handle(DDF_Browser)::DownCast(Draw1::GetExisting(a[1]));
  if (browser.IsNull())
  {
    Message::SendFail() << "Syntax error: browser '" << a[1] << "' not found";
    return 1;
  }

  DataLabel lab;
  TDF_Tool::Label(browser->Data(), a[2], lab);

  if (lab.IsNull())
    return 1;

  AsciiString1 list(browser->OpenAttributeList(lab));
  di << list.ToCString();
  return 0;
}

//=======================================================================
// function : DFOpenAttribute
// purpose  :
//  arg 1  : Browser name
//  arg 2  : Attribute index
//=======================================================================

static Standard_Integer DFOpenAttribute(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
    return 1;

  Handle(DDF_Browser) browser = Handle(DDF_Browser)::DownCast(Draw1::GetExisting(a[1]));
  if (browser.IsNull())
  {
    Message::SendFail() << "Syntax error: browser '" << a[1] << "' not found";
    return 1;
  }

  const Standard_Integer  index = Draw1::Atoi(a[2]);
  AsciiString1 list  = browser->OpenAttribute(index);
  di << list.ToCString();
  return 0;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=================================================================================================

void DDF1::BrowserCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  const char* g = "DF browser commands";

  theCommands.Add("DFBrowse",
                  "Creates a browser on a df: DFBrowse dfname [browsername]",
                  __FILE__,
                  DFBrowse,
                  g);

  theCommands.Add("DFOpenLabel",
                  "DON'T USE THIS COMMAND RESERVED TO THE BROWSER!\nReturns the list of sub-label "
                  "entries: DFOpenLabel browsername [label]",
                  __FILE__,
                  DFOpenLabel,
                  g);

  theCommands.Add("DFOpenAttributeList",
                  "DON'T USE THIS COMMAND RESERVED TO THE BROWSER!\nReturns the attribute list of "
                  "a label: DFOpenLabel browsername label",
                  __FILE__,
                  DFOpenAttributeList,
                  g);

  theCommands.Add("DFOpenAttribute",
                  "DON'T USE THIS COMMAND RESERVED TO THE BROWSER!\nReturns the reference list of "
                  "an attribute: DFOpenLabel browsername attributeindex",
                  __FILE__,
                  DFOpenAttribute,
                  g);
#if 0
  theCommands.Add
    ("DFDisplayInfo",
     "DON'T USE THIS COMMAND RESERVED TO THE BROWSER!\nReturns information about an attribute, a df or a label: DFDisplayInfo {#} | {browsername [label]}",
     __FILE__, DFDisplayInfo, g);
#endif
}
