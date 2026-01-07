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

#include <Vrml_Scale.hxx>

Scale::Scale()
{
  Vector3d tmpV(1, 1, 1);
  myScaleFactor = tmpV;
}

Scale::Scale(const Vector3d& aScaleFactor)
{
  myScaleFactor = aScaleFactor;
}

void Scale::SetScaleFactor(const Vector3d& aScaleFactor)
{
  myScaleFactor = aScaleFactor;
}

Vector3d Scale::ScaleFactor() const
{
  return myScaleFactor;
}

Standard_OStream& Scale::Print(Standard_OStream& anOStream) const
{
  anOStream << "Scale {\n";

  if (Abs(myScaleFactor.X() - 1) > 0.0001 || Abs(myScaleFactor.Y() - 1) > 0.0001
      || Abs(myScaleFactor.Z() - 1) > 0.0001)
  {
    anOStream << "    scaleFactor\t";
    anOStream << myScaleFactor.X() << " " << myScaleFactor.Y() << " " << myScaleFactor.Z() << "\n";
  }

  anOStream << "}\n";
  return anOStream;
}
