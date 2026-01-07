// Created on: 1997-11-24
// Created by: Mister rmi
// Copyright (c) 1997-1999 Matra Datavision
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

#include <OSD_Environment.hxx>
#include <OSD_FileIterator.hxx>
#include <OSD_Host.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <OSD_SingleProtection.hxx>
#include <Resource_Manager.hxx>
#include <Resource_Unicode.hxx>
#include <Standard_GUID.hxx>
#include <Storage_BaseDriver.hxx>
#include <Storage_Data.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <UTL.hxx>

UtfString UTL1::xgetenv(const Standard_CString aCString)
{
  UtfString x;
  OSD_Environment            theEnv(aCString);
  AsciiString1    theValue = theEnv.Value();
  if (!theValue.IsEmpty())
    x = UtfString(theValue);
  return x;
}

UtfString UTL1::Extension(const UtfString& aFileName)
{
  AsciiString1 aFileNameU(aFileName);
  SystemPath                p            = SystemPath(aFileNameU);
  AsciiString1 theExtension = p.Extension();
  if (theExtension.Value(1) == '.')
    theExtension.Remove(1, 1);
  return UtfString(theExtension);
}

Storage_Error UTL1::OpenFile(const Handle(Storage_BaseDriver)& aDriver,
                            const UtfString& aFileName,
                            const Storage_OpenMode            aMode)
{
  return aDriver->Open(AsciiString1(aFileName), aMode);
}

void UTL1::AddToUserInfo(const Handle(Storage_Data)& aData, const UtfString& anInfo)
{
  aData->AddToUserInfo(AsciiString1(anInfo));
}

SystemPath UTL1::Path(const UtfString& aFileName)
{
  SystemPath p = SystemPath(AsciiString1(aFileName));
  return p;
}

UtfString UTL1::Disk(const SystemPath& aPath)
{
  return UtfString(aPath.Disk());
}

UtfString UTL1::Trek(const SystemPath& aPath)
{
  return UtfString(aPath.Trek());
}

UtfString UTL1::Name(const SystemPath& aPath)
{
  return UtfString(aPath.Name());
}

UtfString UTL1::Extension(const SystemPath& aPath)
{
  return UtfString(aPath.Extension());
}

OSD_FileIterator UTL1::FileIterator(const SystemPath& aPath, const UtfString& aMask)
{
  OSD_FileIterator it = OSD_FileIterator(aPath, AsciiString1(aMask));
  return it;
}

UtfString UTL1::LocalHost()
{
  OSD_Host h;
  return UtfString(h.HostName());
}

UtfString UTL1::ExtendedString(const AsciiString1& anAsciiString)
{
  return UtfString(anAsciiString);
}

Standard_GUID UTL1::GUID(const UtfString& anXString)
{
  return Standard_GUID(AsciiString1(anXString, '?').ToCString());
}

Standard_Boolean UTL1::Find(const Handle(Resource_Manager)&   aResourceManager,
                           const UtfString& aResourceName)
{
  return aResourceManager->Find(AsciiString1(aResourceName).ToCString());
}

UtfString UTL1::Value(const Handle(Resource_Manager)&   aResourceManager,
                                      const UtfString& aResourceName)
{
  AsciiString1 aResourceNameU(aResourceName);
  return UtfString(aResourceManager->Value(aResourceNameU.ToCString()),
                                    Standard_True);
}

Standard_Integer UTL1::IntegerValue(const UtfString& anExtendedString)
{
  AsciiString1 a(anExtendedString);
  return a.IntegerValue();
}

Standard_CString UTL1::CString(const UtfString& anExtendedString)
{
  static AsciiString1 theValue;
  theValue = AsciiString1(anExtendedString);
  return theValue.ToCString();
}

Standard_Boolean UTL1::IsReadOnly(const UtfString& aFileName)
{
  switch (SystemFile(UTL1::Path(aFileName)).Protection().User())
  {
    case OSD_W:
    case OSD_RW:
    case OSD_WX:
    case OSD_RWX:
    case OSD_RWD:
    case OSD_WXD:
    case OSD_RWXD:
      return Standard_False;
    default:
      return Standard_True;
  }
}
