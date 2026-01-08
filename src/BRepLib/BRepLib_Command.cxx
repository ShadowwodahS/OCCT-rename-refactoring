// Created on: 1993-07-23
// Created by: Remi LEQUETTE
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

#include <BRepLib_Command.hxx>
#include <StdFail_NotDone.hxx>

//=================================================================================================

Command1::Command1()
    : myDone(Standard_False)
{
}

Command1::~Command1() {}

//=================================================================================================

Standard_Boolean Command1::IsDone() const
{
  return myDone;
}

//=================================================================================================

void Command1::Check() const
{
  if (!myDone)
    throw StdFail_NotDone("BRep_API: command not done");
}

//=================================================================================================

void Command1::Done()
{
  myDone = Standard_True;
}

//=================================================================================================

void Command1::NotDone()
{
  myDone = Standard_False;
}
