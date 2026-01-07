// Created on: 2000-03-01
// Created by: Denis PASCAL
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <DDocStd.hxx>

#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Standard_GUID.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>

#include <StdLDrivers.hxx>
#include <BinLDrivers.hxx>
#include <XmlLDrivers.hxx>
#include <StdDrivers.hxx>
#include <BinDrivers.hxx>
#include <XmlDrivers.hxx>

//=================================================================================================

const Handle(AppManager)& DDocStd1::GetApplication()
{
  static Handle(AppManager) anApp;
  if (anApp.IsNull())
  {
    anApp = new AppManager;

    // Initialize standard document formats at creation - they should
    // be available even if this DRAW plugin is not loaded by pload command
    StdLDrivers::DefineFormat(anApp);
    BinLDrivers1::DefineFormat(anApp);
    XmlLDrivers::DefineFormat(anApp);
    StdDrivers::DefineFormat(anApp);
    BinDrivers1::DefineFormat(anApp);
    XmlDrivers::DefineFormat(anApp);
  }
  return anApp;
}

//=================================================================================================

Standard_Boolean DDocStd1::GetDocument(Standard_CString&         Name,
                                      Handle(AppDocument)& DOC,
                                      const Standard_Boolean    Complain)
{
  Handle(DDocStd_DrawDocument) DD = Handle(DDocStd_DrawDocument)::DownCast(Draw1::GetExisting(Name));
  if (DD.IsNull())
  {
    if (Complain)
      std::cout << Name << " is not a Document" << std::endl;
    return Standard_False;
  }
  Handle(AppDocument) STDDOC = DD->GetDocument();
  if (!STDDOC.IsNull())
  {
    DOC = STDDOC;
    return Standard_True;
  }
  if (Complain)
    std::cout << Name << " is not a CAF Document" << std::endl;
  return Standard_False;
}

//=======================================================================
// function : Label
// purpose  : try to retrieve a label
//=======================================================================

Standard_Boolean DDocStd1::Find(const Handle(AppDocument)& D,
                               const Standard_CString          Entry,
                               DataLabel&                      Label,
                               const Standard_Boolean          Complain)
{
  Label.Nullify();
  TDF_Tool::Label(D->GetData(), Entry, Label, Standard_False);
  if (Label.IsNull() && Complain)
    std::cout << "No label for entry " << Entry << std::endl;
  return !Label.IsNull();
}

//=======================================================================
// function : Find
// purpose  : Try to retrieve an attribute.
//=======================================================================

Standard_Boolean DDocStd1::Find(const Handle(AppDocument)& D,
                               const Standard_CString          Entry,
                               const Standard_GUID&            ID,
                               Handle(TDF_Attribute)&          A,
                               const Standard_Boolean          Complain)
{
  DataLabel L;
  if (Find(D, Entry, L, Complain))
  {
    if (L.FindAttribute(ID, A))
      return Standard_True;
    if (Complain)
      std::cout << "attribute not found for entry : " << Entry << std::endl;
  }
  return Standard_False;
}

//=================================================================================================

DrawInterpreter& DDocStd1::ReturnLabel(DrawInterpreter& di, const DataLabel& L)
{
  AsciiString1 S;
  TDF_Tool::Entry(L, S);
  di << S.ToCString();
  return di;
}

//=================================================================================================

void DDocStd1::AllCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  // define commands
  DDocStd1::ApplicationCommands(theCommands);
  DDocStd1::DocumentCommands(theCommands);
  DDocStd1::ToolsCommands(theCommands);
  DDocStd1::MTMCommands(theCommands);
  DDocStd1::ShapeSchemaCommands(theCommands);
}
