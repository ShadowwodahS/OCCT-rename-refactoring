// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <inspector/DFBrowser_OpenApplication.hxx>

#include <BinDrivers.hxx>
#include <BinLDrivers.hxx>
#include <BinXCAFDrivers.hxx>
#include <PCDM_ReadWriter.hxx>
#include <StdDrivers.hxx>
#include <StdLDrivers.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Controller.hxx>
#include <TPrsStd_DriverTable.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFPrs_Driver.hxx>
#include <XmlDrivers.hxx>
#include <XmlLDrivers.hxx>
#include <XmlXCAFDrivers.hxx>
#include <UTL.hxx>

namespace DFBrowser_OpenApplication
{

// =======================================================================
// function : OpenApplication
// purpose :
// =======================================================================
Handle(AppManager) OpenApplication(const AsciiString1& theFileName,
                                            bool&                          isSTEPFile)
{
  Handle(AppManager) anApplication = CreateApplicationBySTEPFile(theFileName);
  if (!anApplication.IsNull())
  {
    isSTEPFile = true;
    return anApplication;
  }

  // Load static variables for STEPCAF (ssv; 16.08.2012)
  STEPCAFControl_Controller::Init();

  anApplication = new AppManager();
  // Initialize standard document formats at creation - they should
  // be available even if this DRAW plugin is not loaded by pload command
  StdLDrivers::DefineFormat(anApplication);
  BinLDrivers1::DefineFormat(anApplication);
  XmlLDrivers::DefineFormat(anApplication);
  StdDrivers::DefineFormat(anApplication);
  BinDrivers1::DefineFormat(anApplication);
  XmlDrivers::DefineFormat(anApplication);

  // Initialize XCAF formats
  BinXCAFDrivers1::DefineFormat(anApplication);
  XmlXCAFDrivers::DefineFormat(anApplication);

  // Register driver in global table for displaying XDE documents
  // in 3d viewer using OCAF mechanics
  TPrsStd_DriverTable::Get()->AddDriver(XCAFPrs_Driver::GetID(), new XCAFPrs_Driver);

  Handle(AppDocument) aDocument;
  PCDM_ReaderStatus        aStatus = anApplication->Open(theFileName, aDocument);
  if (aStatus != PCDM_RS_OK)
    return Handle(AppManager)();
  return anApplication;
}

// =======================================================================
// function : CreateApplicationBySTEPFile
// purpose :
// =======================================================================
Handle(AppManager) CreateApplicationBySTEPFile(const AsciiString1& theFileName)
{
  if (!theFileName.EndsWith(".step") && !theFileName.EndsWith(".stp"))
    return Handle(AppManager)();

  Handle(AppManager) aTmpApplication = XCAFApp_Application::GetApplication();
  STEPCAFControl_Reader       aStepReader;

  const AsciiString1 aStr(theFileName);
  IFSelect_ReturnStatus         aStatus = aStepReader.ReadFile(aStr.ToCString());
  if (aStatus != IFSelect_RetDone)
    return Handle(AppManager)();

  aStepReader.SetColorMode(Standard_True);
  aStepReader.SetLayerMode(Standard_True);
  aStepReader.SetNameMode(Standard_True);

  Handle(AppDocument) aDocument;
  aTmpApplication->NewDocument("BinOcaf", aDocument);
  return aStepReader.Transfer(aDocument) ? aTmpApplication : Handle(AppManager)();
}
} // namespace DFBrowser_OpenApplication
