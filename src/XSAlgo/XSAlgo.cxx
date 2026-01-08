// Created on: 2000-01-19
// Created by: data exchange team
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

#include <XSAlgo.hxx>

#include <Interface_Static.hxx>
#include <ShapeAlgo.hxx>
#include <ShapeProcess_OperLibrary.hxx>
#include <XSAlgo_AlgoContainer.hxx>

static Handle(XSAlgo_AlgoContainer) theContainer;

//=================================================================================================

void XSAlgo::Init()
{
  static Standard_Boolean init = Standard_False;
  if (init)
    return;
  init = Standard_True;
  ShapeAlgo1::Init();
  theContainer = new XSAlgo_AlgoContainer;

  // init parameters
  ExchangeConfig::Standards();

  // #74 rln 10.03.99 S4135: adding new parameter for handling use of BRepLib::SameParameter
  ExchangeConfig::Init("XSTEP", "read.stdsameparameter.mode", 'e', "");
  ExchangeConfig::Init("XSTEP", "read.stdsameparameter.mode", '&', "ematch 0");
  ExchangeConfig::Init("XSTEP", "read.stdsameparameter.mode", '&', "eval Off");
  ExchangeConfig::Init("XSTEP", "read.stdsameparameter.mode", '&', "eval On");
  ExchangeConfig::SetIVal("read.stdsameparameter.mode", 0);

  // unit: supposed to be cascade unit (target unit for reading)
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", 'e', "");
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "enum 1");
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval INCH"); // 1
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval MM");   // 2
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval ??");   // 3
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval FT");   // 4
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval MI");   // 5
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval M");    // 6
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval KM");   // 7
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval MIL");  // 8
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval UM");   // 9
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval CM");   // 10
  ExchangeConfig::Init("XSTEP", "xstep.cascade.unit", '&', "eval UIN");  // 11
  ExchangeConfig::SetCVal("xstep.cascade.unit", "MM");

  // init Standard Shape Processing operators
  ShapeProcess_OperLibrary::Init();
}

//=================================================================================================

void XSAlgo::SetAlgoContainer(const Handle(XSAlgo_AlgoContainer)& aContainer)
{
  theContainer = aContainer;
}

//=================================================================================================

Handle(XSAlgo_AlgoContainer) XSAlgo::AlgoContainer()
{
  return theContainer;
}
