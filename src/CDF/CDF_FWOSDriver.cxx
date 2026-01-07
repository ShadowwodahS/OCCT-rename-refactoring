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

#include <CDF_FWOSDriver.hxx>
#include <CDM_MetaData.hxx>
#include <OSD_Directory.hxx>
#include <OSD_File.hxx>
#include <OSD_FileNode.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <OSD_SingleProtection.hxx>
#include <Standard_Type.hxx>
#include <TCollection_ExtendedString.hxx>
#include <UTL.hxx>

IMPLEMENT_STANDARD_RTTIEXT(CDF_FWOSDriver, CDF_MetaDataDriver)

#ifdef _MSC_VER
  #include <tchar.h>
#endif // _MSC_VER

//=================================================================================================

static void PutSlash(UtfString& anXSTRING)
{
#ifdef _WIN32
  anXSTRING += "\\";
#else
  anXSTRING += "/";
#endif // _WIN32
}

//=================================================================================================

CDF_FWOSDriver::CDF_FWOSDriver(CDM_MetaDataLookUpTable& theLookUpTable)
    : myLookUpTable(&theLookUpTable)
{
}

//=================================================================================================

Standard_Boolean CDF_FWOSDriver::Find(const UtfString& aFolder,
                                      const UtfString& aName,
                                      const UtfString& /*aVersion*/)
{

  SystemPath      thePath = UTL1::Path(aFolder);
  OSD_Directory theDirectory(thePath);
  if (theDirectory.Exists())
  {
    UtfString f(aFolder);
    PutSlash(f);
    f += aName;
    SystemPath p2 = UTL1::Path(f);
    SystemFile theFile(p2);
    return theFile.Exists();
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean CDF_FWOSDriver::HasReadPermission(const UtfString& aFolder,
                                                   const UtfString& aName,
                                                   const UtfString& /*aVersion*/)
{
  OSD_SingleProtection theProtection =
    SystemFile(UTL1::Path(Concatenate(aFolder, aName))).Protection().User();
  switch (theProtection)
  {
    case OSD_None:
    case OSD_R:
    case OSD_RW:
    case OSD_RX:
    case OSD_WX:
    case OSD_RWX:
    case OSD_RD:
    case OSD_RWD:
    case OSD_RXD:
    case OSD_RWXD:
      return Standard_True;
    default:
      return Standard_False;
  }
}

//=================================================================================================

Handle(CDM_MetaData) CDF_FWOSDriver::MetaData(const UtfString& aFolder,
                                              const UtfString& aName,
                                              const UtfString& /*aVersion*/)
{
  UtfString p = Concatenate(aFolder, aName);
  return CDM_MetaData::LookUp(*myLookUpTable, aFolder, aName, p, p, UTL1::IsReadOnly(p));
}

//=================================================================================================

Handle(CDM_MetaData) CDF_FWOSDriver::CreateMetaData(const Handle(CDM_Document)&       aDocument,
                                                    const UtfString& aFileName)
{
  return CDM_MetaData::LookUp(*myLookUpTable,
                              aDocument->RequestedFolder(),
                              aDocument->RequestedName(),
                              Concatenate(aDocument->RequestedFolder(), aDocument->RequestedName()),
                              aFileName,
                              UTL1::IsReadOnly(aFileName));
}

//=================================================================================================

UtfString CDF_FWOSDriver::BuildFileName(const Handle(CDM_Document)& aDocument)
{

  UtfString retstr = UtfString(aDocument->RequestedFolder());
  PutSlash(retstr);
  retstr += aDocument->RequestedName();
  return retstr;
}

//=================================================================================================

Standard_Boolean CDF_FWOSDriver::FindFolder(const UtfString& aFolder)
{

  SystemPath      thePath = UTL1::Path(aFolder);
  OSD_Directory theDirectory(thePath);
  return theDirectory.Exists();
}

//=================================================================================================

UtfString CDF_FWOSDriver::Concatenate(const UtfString& aFolder,
                                                       const UtfString& aName)
{
  UtfString ff(aFolder);
  PutSlash(ff);
  ff += aName;
  return ff;
}

//=================================================================================================

UtfString CDF_FWOSDriver::DefaultFolder()
{
  UtfString theDefaultFolder;
  if (theDefaultFolder.Length() == 0)
  {

#ifdef _WIN32
    UtfString hd = UTL1::xgetenv("HOMEDRIVE");
    if (hd.Length() != 0)
    {
      theDefaultFolder = hd;
      theDefaultFolder += UTL1::xgetenv("HOMEPATH");
    }
    else
    {
      theDefaultFolder = UTL1::xgetenv("TEMP");
      if (theDefaultFolder.Length() == 0)
        theDefaultFolder = ".";
    }
#else
    UtfString home = UTL1::xgetenv("HOME");
    if (home.Length() != 0)
      theDefaultFolder = home;
    else
      theDefaultFolder = UtfString("/tmp");
#endif
  }
  return theDefaultFolder;
}

//=================================================================================================

UtfString CDF_FWOSDriver::SetName(const Handle(CDM_Document)&       aDocument,
                                                   const UtfString& aName)
{

  UtfString xn(aName), n(aName);

#ifdef _WIN32
  // windows is not case sensitive
  // make the extension lower case
  for (int i = 1; i <= xn.Length(); i++)
  {
    Standard_ExtCharacter echar = xn.Value(i);
    echar                       = towlower(echar);
    xn.SetValue(i, echar);
  }
#endif

  UtfString e(aDocument->FileExtension());
  UtfString xe(e);
  if (e.Length() > 0)
  {
#ifdef _WIN32
    // windows is not case sensitive
    // make the extension lower case
    for (int i = 1; i <= xe.Length(); i++)
    {
      Standard_ExtCharacter echar = xe.Value(i);
      echar                       = towlower(echar);
      xe.SetValue(i, echar);
    }
#endif
    xe.Insert(1, '.');
    e.Insert(1, '.');
    Standard_Integer ln                      = xn.Length();
    Standard_Integer le                      = xe.Length();
    Standard_Boolean ExtensionIsAlreadyThere = Standard_False;
    if (ln >= le)
    {
      Standard_Integer ind    = xn.SearchFromEnd(xe);
      ExtensionIsAlreadyThere = ind + le - 1 == ln;
    }
    if (!ExtensionIsAlreadyThere)
      n += e;
  }
  return n;
}
