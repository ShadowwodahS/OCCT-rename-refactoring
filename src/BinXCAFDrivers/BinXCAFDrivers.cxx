// Created on: 2005-04-18
// Created by: Eugeny NAPALKOV
// Copyright (c) 2005-2014 OPEN CASCADE SAS
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

#include <BinDrivers.hxx>
#include <BinMDF_ADriverTable.hxx>
#include <BinMXCAFDoc.hxx>
#include <BinXCAFDrivers.hxx>
#include <BinXCAFDrivers_DocumentRetrievalDriver.hxx>
#include <BinXCAFDrivers_DocumentStorageDriver.hxx>
#include <Plugin_Macro.hxx>
#include <Standard_Failure.hxx>
#include <Standard_GUID.hxx>
#include <TDocStd_Application.hxx>

static Standard_GUID BinXCAFStorageDriver("a78ff496-a779-11d5-aab4-0050044b1af1");
static Standard_GUID BinXCAFRetrievalDriver("a78ff497-a779-11d5-aab4-0050044b1af1");

//=================================================================================================

const Handle(RefObject)& BinXCAFDrivers1::Factory(const Standard_GUID& theGUID)
{

  if (theGUID == BinXCAFStorageDriver)
  {
#ifdef OCCT_DEBUG
    std::cout << "BinXCAFDrivers1 : Storage1 Plugin1" << std::endl;
#endif
    static Handle(RefObject) model_sd = new BinXCAFDrivers_DocumentStorageDriver;
    return model_sd;
  }

  if (theGUID == BinXCAFRetrievalDriver)
  {
#ifdef OCCT_DEBUG
    std::cout << "BinXCAFDrivers1 : Retrieval Plugin1" << std::endl;
#endif
    static Handle(RefObject) model_rd = new BinXCAFDrivers_DocumentRetrievalDriver;
    return model_rd;
  }

  throw ExceptionBase("XCAFBinDrivers : unknown GUID");
}

//=================================================================================================

void BinXCAFDrivers1::DefineFormat(const Handle(AppManager)& theApp)
{
  theApp->DefineFormat("BinXCAF",
                       "Binary XCAF Document",
                       "xbf",
                       new BinXCAFDrivers_DocumentRetrievalDriver,
                       new BinXCAFDrivers_DocumentStorageDriver);
}

//=================================================================================================

Handle(BinMDF_ADriverTable) BinXCAFDrivers1::AttributeDrivers(
  const Handle(Message_Messenger)& aMsgDrv)
{
  // Standard1 Drivers
  Handle(BinMDF_ADriverTable) aTable = BinDrivers1::AttributeDrivers(aMsgDrv);

  // XCAF Drivers
  BinMXCAFDoc1::AddDrivers(aTable, aMsgDrv);

  return aTable;
}

PLUGIN(BinXCAFDrivers1)
