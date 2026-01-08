// Created:	Tue Nov  2 14:40:06 1999
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

#include <Express_Enum.hxx>

#include <Express.hxx>
#include <Message.hxx>
#include <OSD_Directory.hxx>
#include <OSD_FileSystem.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Express_Enum, Express_Item)

//=================================================================================================

Express_Enum::Express_Enum(const Standard_CString                         theName,
                           const Handle(TColStd_HSequenceOfHAsciiString)& theNames)
    : Express_Item(theName),
      myNames(theNames)
{
}

//=================================================================================================

const Handle(TColStd_HSequenceOfHAsciiString)& Express_Enum::Names() const
{
  return myNames;
}

//=================================================================================================

Standard_Boolean Express_Enum::GenerateClass() const
{
  const AsciiString1 aCPPName = CPPName();
  Message1::SendInfo() << "Generating ENUMERATION " << aCPPName;

  // create a package directory (if not yet exist)
  Protection1          aProt(OSD_RWXD, OSD_RWXD, OSD_RX, OSD_RX);
  AsciiString1 aPack = GetPackageName();
  SystemPath                aPath(aPack);
  OSD_Directory           aDir(aPath);
  aDir.Build(aProt);
  aPack += "/";
  aPack += aCPPName;
  const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();

  // Open HXX file
  std::shared_ptr<std::ostream> aStreamPtr =
    aFileSystem->OpenOStream(aPack.Cat(".hxx"), std::ios::out | std::ios::ate);
  Standard_OStream& anOS = *aStreamPtr;

  // write header
  Express1::WriteFileStamp(anOS);

  // write defines
  anOS << "#ifndef _" << aCPPName
       << "_HeaderFile\n"
          "#define _"
       << aCPPName
       << "_HeaderFile\n"
          "\n"
          "enum "
       << aCPPName
       << "\n"
          "{\n";
  AsciiString1 aPrefix = Express1::EnumPrefix(Name());
  for (Standard_Integer i = 1; i <= myNames->Length(); i++)
  {
    if (i > 1)
    {
      anOS << ",\n";
    }
    anOS << "  " << GetPackageName() << "_" << aPrefix << myNames->Value(i)->String();
  }

  anOS << "\n"
          "};\n"
          "\n"
          "#endif // _"
       << aCPPName << "_HeaderFile\n";

  aStreamPtr.reset();

  return Standard_False;
}

//=================================================================================================

void Express_Enum::PropagateUse() const {}
