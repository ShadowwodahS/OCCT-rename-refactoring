// Created on: 1997-12-01
// Created by: Jean-Louis Frenkel
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

#include <Message_Messenger.hxx>
#include <CDM_MetaData.hxx>
#include <OSD_Path.hxx>
#include <PCDM_ReferenceIterator.hxx>
#include <PCDM_RetrievalDriver.hxx>
#include <Standard_Type.hxx>
#include <UTL.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PCDM_ReferenceIterator, RefObject)

#ifdef _MSC_VER
  #include <tchar.h>
#endif // _MSC_VER

//=================================================================================================

PCDM_ReferenceIterator::PCDM_ReferenceIterator(const Handle(Message_Messenger)& theMsgDriver)
    : myIterator(0)
{
  myMessageDriver = theMsgDriver;
}

//=================================================================================================

void PCDM_ReferenceIterator::LoadReferences(const Handle(CDM_Document)&    aDocument,
                                            const Handle(CDM_MetaData)&    aMetaData,
                                            const Handle(CDM_Application)& anApplication,
                                            const Standard_Boolean         UseStorageConfiguration)
{
  for (Init(aMetaData); More(); Next())
  {
    aDocument->CreateReference(
      MetaData(anApplication->MetaDataLookUpTable(), UseStorageConfiguration),
      ReferenceIdentifier(),
      anApplication,
      DocumentVersion(),
      UseStorageConfiguration);
  }
}

//=================================================================================================

void PCDM_ReferenceIterator::Init(const Handle(CDM_MetaData)& theMetaData)
{

  myReferences.Clear();
  // mod. by szy
  PCDM_RetrievalDriver::References(theMetaData->FileName(), myReferences, myMessageDriver);
  myIterator = 1;
}

//=================================================================================================

Standard_Boolean PCDM_ReferenceIterator::More() const
{
  return myIterator <= myReferences.Length();
}

//=================================================================================================

void PCDM_ReferenceIterator::Next()
{
  myIterator++;
}

//=================================================================================================

Handle(CDM_MetaData) PCDM_ReferenceIterator::MetaData(CDM_MetaDataLookUpTable& theLookUpTable,
                                                      const Standard_Boolean) const
{

  UtfString theFolder, theName;
  UtfString theFile = myReferences(myIterator).FileName();
  UtfString f(theFile);
#ifndef _WIN32

  Standard_Integer           i = f.SearchFromEnd("/");
  UtfString n = f.Split(i);
  f.Trunc(f.Length() - 1);
  theFolder = f;
  theName   = n;
#else
  SystemPath                   p = UTL1::Path(f);
  Standard_ExtCharacter      chr;
  UtfString dir, dirRet, name;

  dir = UTL1::Disk(p);
  dir += UTL1::Trek(p);

  for (int i = 1; i <= dir.Length(); ++i)
  {

    chr = dir.Value(i);

    switch (chr)
    {

      case '|':
        dirRet += "/";
        break;

      case '^':

        dirRet += "..";
        break;

      default:
        dirRet += chr;
    }
  }
  theFolder = dirRet;
  theName   = UTL1::Name(p);
  theName += UTL1::Extension(p);
#endif // _WIN32

  return CDM_MetaData::LookUp(theLookUpTable,
                              theFolder,
                              theName,
                              theFile,
                              theFile,
                              UTL1::IsReadOnly(theFile));
}

//=================================================================================================

Standard_Integer PCDM_ReferenceIterator::ReferenceIdentifier() const
{
  return myReferences(myIterator).ReferenceIdentifier();
}

//=================================================================================================

Standard_Integer PCDM_ReferenceIterator::DocumentVersion() const
{
  return myReferences(myIterator).DocumentVersion();
}
