// Created on: 1995-03-01
// Created by: Remi LEQUETTE
// Copyright (c) 1995-1999 Matra Datavision
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

#include <Draw.hxx>
#include <Draw_Drawable3D.hxx>

void Draw1::Commands(DrawInterpreter& theCommands)
{
  Draw1::BasicCommands(theCommands);
  Draw1::MessageCommands(theCommands);
  Draw1::VariableCommands(theCommands);
  Draw1::GraphicCommands(theCommands);
  Draw1::PloadCommands(theCommands);
  Draw1::UnitCommands(theCommands);
}
