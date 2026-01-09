// Created:	Tue Nov  2 15:13:31 1999
// Author:	Andrey BETENEV
// Copyright (c) 1999-2020 OPEN CASCADE SAS
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

#include <Express_ComplexType.hxx>

#include <TCollection_AsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Express_ComplexType, ExpressType)

//=================================================================================================

Express_ComplexType::Express_ComplexType(const Standard_Integer      theImin,
                                         const Standard_Integer      theImax,
                                         const Handle(ExpressType)& theType)
{
  myMin  = theImin;
  myMax  = theImax;
  myType = theType;
}

//=================================================================================================

const Handle(ExpressType)& Express_ComplexType::Type() const
{
  return myType;
}

//=================================================================================================

const AsciiString1 Express_ComplexType::CPPName() const
{
  // check if array 2
  Handle(ExpressType) aType = myType;
  if (aType->IsKind(STANDARD_TYPE(Express_ComplexType)))
  {
    Handle(Express_ComplexType) aType2 = Handle(Express_ComplexType)::DownCast(aType);
    aType                              = aType2->Type();
  }

  // parse name of array argument
  AsciiString1 aName     = aType->CPPName();
  Standard_Integer        aSplitIdx = aName.Location(1, '_', 1, aName.Length());
  AsciiString1 aClassName;
  if (aSplitIdx)
  {
    aClassName = aName.Split(aSplitIdx);
  }
  else
  {
    aClassName = aName;
  }
  Standard_Integer anIdx = aName.Location("TCollection1", 1, aName.Length());
  if (anIdx)
  {
    aName = "Interface_";
  }
  // generate name
  if (aType->IsStandard() || !aSplitIdx)
  {
    aName = "TColStd_";
  }
  if (aType == myType)
  {
    aName += "HArray1Of";
  }
  else
  {
    aName += "HArray2Of";
  }
  aName += aClassName;

  return aName;
}

//=================================================================================================

Standard_Boolean Express_ComplexType::Use() const
{
  return myType->Use();
}

//=================================================================================================

void Express_ComplexType::Use2(const AsciiString1& theRefName,
                               const AsciiString1& theRefPack) const
{
  myType->Use2(theRefName, theRefPack);
}
