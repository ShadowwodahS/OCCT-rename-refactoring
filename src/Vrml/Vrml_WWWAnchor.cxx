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

#include <Vrml_WWWAnchor.hxx>

WWWAnchor::WWWAnchor(const AsciiString1& aName,
                               const AsciiString1& aDescription,
                               const Vrml_WWWAnchorMap        aMap)
{
  myName        = aName;
  myDescription = aDescription;
  myMap         = aMap;
}

void WWWAnchor::SetName(const AsciiString1& aName)
{
  myName = aName;
}

AsciiString1 WWWAnchor::Name() const
{
  return myName;
}

void WWWAnchor::SetDescription(const AsciiString1& aDescription)
{
  myDescription = aDescription;
}

AsciiString1 WWWAnchor::Description() const
{
  return myDescription;
}

void WWWAnchor::SetMap(const Vrml_WWWAnchorMap aMap)
{
  myMap = aMap;
}

Vrml_WWWAnchorMap WWWAnchor::Map() const
{
  return myMap;
}

Standard_OStream& WWWAnchor::Print(Standard_OStream& anOStream) const
{
  anOStream << "WWWAnchor {\n";

  if (!(myName.IsEqual("")))
  {
    anOStream << "    name\t";
    anOStream << '"' << myName << '"' << "\n";
  }

  if (!(myDescription.IsEqual("")))
  {
    anOStream << "    description\t";
    anOStream << '"' << myDescription << '"' << "\n";
  }

  switch (myMap)
  {
    case Vrml_MAP_NONE:
      break; // anOStream  << "    map\tNONE ";
    case Vrml_POINT:
      anOStream << "    map\t\tPOINT\n";
      break;
  }

  anOStream << "}\n";
  return anOStream;
}
