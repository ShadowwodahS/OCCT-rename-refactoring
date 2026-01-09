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

#include <TCollection_AsciiString.hxx>
#include <Vrml_WWWInline.hxx>

WWWInline::WWWInline()
{
  myName = "";
  Vector3d tmpVec(0, 0, 0);
  myBboxSize   = tmpVec;
  myBboxCenter = tmpVec;
}

WWWInline::WWWInline(const AsciiString1& aName,
                               const Vector3d&                  aBboxSize,
                               const Vector3d&                  aBboxCenter)
{
  myName       = aName;
  myBboxSize   = aBboxSize;
  myBboxCenter = aBboxCenter;
}

void WWWInline::SetName(const AsciiString1& aName)
{
  myName = aName;
}

AsciiString1 WWWInline::Name() const
{
  return myName;
}

void WWWInline::SetBboxSize(const Vector3d& aBboxSize)
{
  myBboxSize = aBboxSize;
}

Vector3d WWWInline::BboxSize() const
{
  return myBboxSize;
}

void WWWInline::SetBboxCenter(const Vector3d& aBboxCenter)
{
  myBboxCenter = aBboxCenter;
}

Vector3d WWWInline::BboxCenter() const
{
  return myBboxCenter;
}

Standard_OStream& WWWInline::Print(Standard_OStream& anOStream) const
{
  anOStream << "WWWInline {\n";

  if (!(myName.IsEqual("")))
  {
    anOStream << "    name\t";
    anOStream << '"' << myName << '"' << "\n";
  }

  if (Abs(myBboxSize.X() - 0) > 0.0001 || Abs(myBboxSize.Y() - 0) > 0.0001
      || Abs(myBboxSize.Z() - 0) > 0.0001)
  {
    anOStream << "    bboxSize\t";
    anOStream << myBboxSize.X() << " " << myBboxSize.Y() << " " << myBboxSize.Z() << "\n";
  }

  if (Abs(myBboxCenter.X() - 0) > 0.0001 || Abs(myBboxCenter.Y() - 0) > 0.0001
      || Abs(myBboxCenter.Z() - 0) > 0.0001)
  {
    anOStream << "    bboxCenter\t";
    anOStream << myBboxCenter.X() << " " << myBboxCenter.Y() << " " << myBboxCenter.Z() << "\n";
  }

  anOStream << "}\n";
  return anOStream;
}
