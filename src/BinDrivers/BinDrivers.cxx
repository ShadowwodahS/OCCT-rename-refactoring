// Created on: 2002-10-29
// Created by: Michael SAZONOV
// Copyright (c) 2002-2014 OPEN CASCADE SAS
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
#include <BinDrivers_DocumentRetrievalDriver.hxx>
#include <BinDrivers_DocumentStorageDriver.hxx>
#include <BinLDrivers.hxx>
#include <BinMDataStd.hxx>
#include <BinMDataXtd.hxx>
#include <BinMDF.hxx>
#include <BinMDF_ADriverTable.hxx>
#include <BinMDocStd.hxx>
#include <BinMFunction.hxx>
#include <BinMNaming.hxx>
#include <Plugin_Macro.hxx>
#include <Standard_Failure.hxx>
#include <Standard_GUID.hxx>
#include <Standard_Transient.hxx>
#include <TDocStd_Application.hxx>

static Standard_GUID BinStorageDriver("03a56835-8269-11d5-aab2-0050044b1af1");
static Standard_GUID BinRetrievalDriver("03a56836-8269-11d5-aab2-0050044b1af1");

//=======================================================================
// function : Factory
// purpose  : PLUGIN FACTORY
//=======================================================================
const Handle(RefObject)& BinDrivers1::Factory(const Standard_GUID& theGUID)
{
  if (theGUID == BinStorageDriver)
  {
#ifdef OCCT_DEBUG
    std::cout << "BinDrivers1 : Storage1 Plugin1" << std::endl;
#endif
    static Handle(RefObject) model_sd = new BinDrivers_DocumentStorageDriver;
    return model_sd;
  }

  if (theGUID == BinRetrievalDriver)
  {
#ifdef OCCT_DEBUG
    std::cout << "BinDrivers1 : Retrieval Plugin1" << std::endl;
#endif
    static Handle(RefObject) model_rd = new BinDrivers_DocumentRetrievalDriver;
    return model_rd;
  }

  throw ExceptionBase("BinDrivers1 : unknown GUID");
}

//=================================================================================================

void BinDrivers1::DefineFormat(const Handle(AppManager)& theApp)
{
  theApp->DefineFormat("BinOcaf",
                       "Binary OCAF Document",
                       "cbf",
                       new BinDrivers_DocumentRetrievalDriver,
                       new BinDrivers_DocumentStorageDriver);
}

//=================================================================================================

Handle(AttributeDriverTable) BinDrivers1::AttributeDrivers(const Handle(Message_Messenger)& aMsgDrv)
{
  Handle(AttributeDriverTable) aTable = new AttributeDriverTable;

  BinMDF1 ::AddDrivers(aTable, aMsgDrv);
  BinMDataStd1 ::AddDrivers(aTable, aMsgDrv);
  BinMDataXtd1 ::AddDrivers(aTable, aMsgDrv);
  BinMNaming1 ::AddDrivers(aTable, aMsgDrv);
  BinMDocStd1 ::AddDrivers(aTable, aMsgDrv);
  BinMFunction1 ::AddDrivers(aTable, aMsgDrv);
  return aTable;
}

PLUGIN(BinDrivers1)
