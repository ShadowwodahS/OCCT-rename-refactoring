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

#include <Vrml_Cone.hxx>

Cone::Cone(const Vrml_ConeParts aParts,
                     const Standard_Real  aBottomRadius,
                     const Standard_Real  aHeight)
{
  myParts        = aParts;
  myBottomRadius = aBottomRadius;
  myHeight       = aHeight;
}

void Cone::SetParts(const Vrml_ConeParts aParts)
{
  myParts = aParts;
}

Vrml_ConeParts Cone::Parts() const
{
  return myParts;
}

void Cone::SetBottomRadius(const Standard_Real aBottomRadius)
{
  myBottomRadius = aBottomRadius;
}

Standard_Real Cone::BottomRadius() const
{
  return myBottomRadius;
}

void Cone::SetHeight(const Standard_Real aHeight)
{
  myHeight = aHeight;
}

Standard_Real Cone::Height() const
{
  return myHeight;
}

Standard_OStream& Cone::Print(Standard_OStream& anOStream) const
{
  anOStream << "Cone {\n";

  switch (myParts)
  {
    case Vrml_ConeALL:
      break; // anOStream  << "    parts\t\tALL ";
    case Vrml_ConeSIDES:
      anOStream << "    parts\t\tSIDES\n";
      break;
    case Vrml_ConeBOTTOM:
      anOStream << "    parts\t\tBOTTOM\n";
      break;
  }

  if (Abs(myBottomRadius - 1) > 0.0001)
  {
    anOStream << "    bottomRadius\t";
    anOStream << myBottomRadius << "\n";
  }

  if (Abs(myHeight - 2) > 0.0001)
  {
    anOStream << "    height\t\t";
    anOStream << myHeight << "\n";
  }

  anOStream << "}\n";
  return anOStream;
}
