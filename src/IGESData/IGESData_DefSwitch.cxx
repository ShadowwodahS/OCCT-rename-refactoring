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

#include <IGESData_DefSwitch.hxx>

//  DefSwitch : represente une definition, soit vide (-> valeur = 0),
//  soit comme rang dans une table (-> valeur > 0 ce rang),
//  soit comme reference (-> valeur < 0), la reference elle-meme est ailleurs
//=======================================================================
// function : DefinitionSwitch
// purpose  : Default constructor.
//=======================================================================
DefinitionSwitch::DefinitionSwitch()
    : theval(0)
{
}

//=================================================================================================

void DefinitionSwitch::SetVoid()
{
  theval = 0;
}

//=================================================================================================

void DefinitionSwitch::SetReference()
{
  theval = -1;
}

//=================================================================================================

void DefinitionSwitch::SetRank(const Standard_Integer theRank)
{
  theval = theRank;
}

//=================================================================================================

IGESData_DefType DefinitionSwitch::DefType() const
{
  if (theval < 0)
    return IGESData_DefReference;

  if (theval > 0)
    return IGESData_DefValue;

  return IGESData_DefVoid;
}

//=================================================================================================

Standard_Integer DefinitionSwitch::Value() const
{
  return theval;
}
