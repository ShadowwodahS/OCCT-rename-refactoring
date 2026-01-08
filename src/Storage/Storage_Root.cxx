// Copyright (c) 1998-1999 Matra Datavision
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

#include <Standard_Type.hxx>
#include <Storage_Root.hxx>
#include <Storage_Schema.hxx>
#include <TCollection_AsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Storage_Root, RefObject)

Storage_Root::Storage_Root()
    : myRef(0)
{
}

Storage_Root::Storage_Root(const AsciiString1&     theName,
                           const Handle(DbObject)& theObject)
    : myName(theName),
      myObject(theObject),
      myRef(0)
{
}

Storage_Root::Storage_Root(const AsciiString1& theName,
                           const Standard_Integer         theRef,
                           const AsciiString1& theType)
    : myName(theName),
      myType(theType),
      myRef(theRef)
{
}

void Storage_Root::SetName(const AsciiString1& theName)
{
  myName = theName;
}

AsciiString1 Storage_Root::Name() const
{
  return myName;
}

void Storage_Root::SetObject(const Handle(DbObject)& anObject)
{
  myObject = anObject;
}

Handle(DbObject) Storage_Root::Object() const
{
  return myObject;
}

AsciiString1 Storage_Root::Type() const
{
  return myType;
}

void Storage_Root::SetReference(const Standard_Integer aRef)
{
  myRef = aRef;
}

Standard_Integer Storage_Root::Reference1() const
{
  return myRef;
}

void Storage_Root::SetType(const AsciiString1& aType)
{
  myType = aType;
}
