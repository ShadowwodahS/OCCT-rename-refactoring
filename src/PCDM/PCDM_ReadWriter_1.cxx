// Created on: 1997-12-09
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

#include <CDM_Document.hxx>
#include <CDM_MetaData.hxx>
#include <CDM_ReferenceIterator.hxx>
#include <OSD_Path.hxx>
#include <PCDM.hxx>
#include <PCDM_ReadWriter_1.hxx>
#include <PCDM_Reference.hxx>
#include <PCDM_TypeOfFileDriver.hxx>
#include <Message_Messenger.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Type.hxx>
#include <Storage_Data.hxx>
#include <Storage_HeaderData.hxx>
#include <Storage_Schema.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TColStd_SequenceOfExtendedString.hxx>
#include <UTL.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PCDM_ReadWriter_1, PCDM_ReadWriter)

#define START_REF "START_REF"
#define END_REF "END_REF"
#define START_EXT "START_EXT"
#define END_EXT "END_EXT"
#define MODIFICATION_COUNTER "MODIFICATION_COUNTER: "
#define REFERENCE_COUNTER "REFERENCE_COUNTER: "

//=================================================================================================

PCDM_ReadWriter_1::PCDM_ReadWriter_1() {}

static Standard_Integer RemoveExtraSeparator(AsciiString1& aString)
{

  Standard_Integer i, j, len;

  len = aString.Length();
#ifdef _WIN32
  // Case of network path, such as \\MACHINE\dir
  for (i = j = 2; j <= len; i++, j++)
  {
#else
  for (i = j = 1; j <= len; i++, j++)
  {
#endif
    Standard_Character c = aString.Value(j);
    aString.SetValue(i, c);
    if (c == '/')
      while (j < len && aString.Value(j + 1) == '/')
        j++;
  }
  len = i - 1;
  if (aString.Value(len) == '/')
    len--;
  aString.Trunc(len);
  return len;
}

static AsciiString1 AbsolutePath(const AsciiString1& aDirPath,
                                            const AsciiString1& aRelFilePath)
{
  AsciiString1 EmptyString = "";
#ifdef _WIN32
  if (aRelFilePath.Search(":") == 2
      || (aRelFilePath.Search("\\") == 1 && aRelFilePath.Value(2) == '\\'))
#else
  if (aRelFilePath.Search("/") == 1)
#endif
    return aRelFilePath;

  AsciiString1 DirPath = aDirPath, RelFilePath = aRelFilePath;
  Standard_Integer        i, len;

#ifdef _WIN32
  if (DirPath.Search(":") != 2 && (DirPath.Search("\\") != 1 || DirPath.Value(2) != '\\'))
#else
  if (DirPath.Search("/") != 1)
#endif
    return EmptyString;

#ifdef _WIN32
  DirPath.ChangeAll('\\', '/');
  RelFilePath.ChangeAll('\\', '/');
#endif

  RemoveExtraSeparator(DirPath);
  len = RemoveExtraSeparator(RelFilePath);

  while (RelFilePath.Search("../") == 1)
  {
    if (len == 3)
      return EmptyString;
    RelFilePath = RelFilePath.SubString(4, len);
    len -= 3;
    if (DirPath.IsEmpty())
      return EmptyString;
    i = DirPath.SearchFromEnd("/");
    if (i < 0)
      return EmptyString;
    DirPath.Trunc(i - 1);
  }
  AsciiString1 retx;
  retx = DirPath;
  retx += "/";
  retx += RelFilePath;
  return retx;
}

static AsciiString1 GetDirFromFile(const UtfString& aFileName)
{
  AsciiString1 theCFile(aFileName);
  AsciiString1 theDirectory;
  Standard_Integer        i = theCFile.SearchFromEnd("/");
#ifdef _WIN32
  //    if(i==-1) i=theCFile.SearchFromEnd("\\");
  if (theCFile.SearchFromEnd("\\") > i)
    i = theCFile.SearchFromEnd("\\");
#endif
  if (i != -1)
    theDirectory = theCFile.SubString(1, i);
  return theDirectory;
}

//=================================================================================================

AsciiString1 PCDM_ReadWriter_1::Version() const
{
  return "PCDM_ReadWriter_1";
}

//=================================================================================================

void PCDM_ReadWriter_1::WriteReferenceCounter(const Handle(Storage_Data)& aData,
                                              const Handle(CDM_Document)& aDocument) const
{
  AsciiString1 ligne(REFERENCE_COUNTER);
  ligne += aDocument->ReferenceCounter();
  aData->AddToUserInfo(ligne);
}

//=================================================================================================

void PCDM_ReadWriter_1::WriteReferences(
  const Handle(Storage_Data)&       aData,
  const Handle(CDM_Document)&       aDocument,
  const UtfString& theReferencerFileName) const
{

  Standard_Integer theNumber = aDocument->ToReferencesNumber();
  if (theNumber > 0)
  {

    aData->AddToUserInfo(START_REF);

    ReferenceIterator it(aDocument);

    UtfString ligne;

    AsciiString1 theAbsoluteDirectory = GetDirFromFile(theReferencerFileName);

    for (; it.More(); it.Next())
    {
      ligne = UtfString(it.ReferenceIdentifier());
      ligne += " ";
      ligne += UtfString(it.Document()->Modifications());
      ligne += " ";

      AsciiString1 thePath(it.Document()->MetaData()->FileName());
      AsciiString1 theRelativePath;
      if (!theAbsoluteDirectory.IsEmpty())
      {
        theRelativePath = SystemPath::RelativePath(theAbsoluteDirectory, thePath);
        if (!theRelativePath.IsEmpty())
          thePath = theRelativePath;
      }
      ligne += UtfString(thePath);
      UTL1::AddToUserInfo(aData, ligne);
    }
    aData->AddToUserInfo(END_REF);
  }
}

//=================================================================================================

void PCDM_ReadWriter_1::WriteExtensions(const Handle(Storage_Data)& aData,
                                        const Handle(CDM_Document)& aDocument) const
{

  TColStd_SequenceOfExtendedString theExtensions;
  aDocument->Extensions(theExtensions);
  Standard_Integer theNumber = theExtensions.Length();
  if (theNumber > 0)
  {

    aData->AddToUserInfo(START_EXT);
    for (Standard_Integer i = 1; i <= theNumber; i++)
    {
      UTL1::AddToUserInfo(aData, theExtensions(i));
    }
    aData->AddToUserInfo(END_EXT);
  }
}

//=================================================================================================

void PCDM_ReadWriter_1::WriteVersion(const Handle(Storage_Data)& aData,
                                     const Handle(CDM_Document)& aDocument) const
{
  AsciiString1 ligne(MODIFICATION_COUNTER);
  ligne += aDocument->Modifications();
  aData->AddToUserInfo(ligne);
}

//=================================================================================================

Standard_Integer PCDM_ReadWriter_1::ReadReferenceCounter(
  const UtfString& aFileName,
  const Handle(Message_Messenger)&  theMsgDriver) const
{

  Standard_Integer           theReferencesCounter(0);
  Standard_Integer           i;
  Handle(Storage_BaseDriver) theFileDriver;
  AsciiString1    aFileNameU(aFileName);
  if (PCDM1::FileDriverType(aFileNameU, theFileDriver) == PCDM_TOFD_Unknown)
    return theReferencesCounter;

  Standard_Boolean theFileIsOpen(Standard_False);
  try
  {
    OCC_CATCH_SIGNALS
    PCDM_ReadWriter::Open(theFileDriver, aFileName, Storage_VSRead);
    theFileIsOpen = Standard_True;

    Handle(Storage_Schema) s = new Storage_Schema;
    Storage_HeaderData     hd;
    hd.Read(theFileDriver);
    const TColStd_SequenceOfAsciiString& refUserInfo = hd.UserInfo();

    for (i = 1; i <= refUserInfo.Length(); i++)
    {
      if (refUserInfo(i).Search(REFERENCE_COUNTER) != -1)
      {
        try
        {
          OCC_CATCH_SIGNALS theReferencesCounter = refUserInfo(i).Token(" ", 2).IntegerValue();
        }
        catch (ExceptionBase const&)
        {
          //	  std::cout << "warning: could not read the reference counter in " << aFileName <<
          // std::endl;
          UtfString aMsg("Warning: ");
          aMsg = aMsg.Cat("could not read the reference counter in ").Cat(aFileName).Cat("\0");
          if (!theMsgDriver.IsNull())
            theMsgDriver->Send(aMsg.ToExtString());
        }
      }
    }
  }
  catch (ExceptionBase const&)
  {
  }

  if (theFileIsOpen)
  {
    theFileDriver->Close();
  }

  return theReferencesCounter;
}

//=================================================================================================

void PCDM_ReadWriter_1::ReadReferences(const UtfString& aFileName,
                                       PCDM_SequenceOfReference&         theReferences,
                                       const Handle(Message_Messenger)&  theMsgDriver) const
{

  TColStd_SequenceOfExtendedString ReadReferences;

  ReadUserInfo(aFileName, START_REF, END_REF, ReadReferences, theMsgDriver);

  Standard_Integer           theReferenceIdentifier;
  UtfString theFileName;
  Standard_Integer           theDocumentVersion;

  AsciiString1 theAbsoluteDirectory = GetDirFromFile(aFileName);

  for (Standard_Integer i = 1; i <= ReadReferences.Length(); i++)
  {
    Standard_Integer pos = ReadReferences(i).Search(" ");
    if (pos != -1)
    {
      UtfString theRest = ReadReferences(i).Split(pos);
      theReferenceIdentifier             = UTL1::IntegerValue(ReadReferences(i));

      Standard_Integer pos2 = theRest.Search(" ");

      theFileName        = theRest.Split(pos2);
      theDocumentVersion = UTL1::IntegerValue(theRest);

      AsciiString1 thePath(theFileName);
      AsciiString1 theAbsolutePath;
      if (!theAbsoluteDirectory.IsEmpty())
      {
        theAbsolutePath = AbsolutePath(theAbsoluteDirectory, thePath);
        if (!theAbsolutePath.IsEmpty())
          thePath = theAbsolutePath;
      }
      if (!theMsgDriver.IsNull())
      {
        //      std::cout << "reference found; ReferenceIdentifier: " << theReferenceIdentifier <<
        //      "; File:" << thePath << ", version:" << theDocumentVersion;
        UtfString aMsg("Warning: ");
        aMsg = aMsg.Cat("reference found; ReferenceIdentifier:  ")
                 .Cat(theReferenceIdentifier)
                 .Cat("; File:")
                 .Cat(thePath)
                 .Cat(", version:")
                 .Cat(theDocumentVersion)
                 .Cat("\0");
        theMsgDriver->Send(aMsg.ToExtString());
      }
      UtfString aPathW(thePath);
      theReferences.Append(PCDM_Reference(theReferenceIdentifier, aPathW, theDocumentVersion));
    }
  }
}

//=================================================================================================

void PCDM_ReadWriter_1::ReadExtensions(const UtfString& aFileName,
                                       TColStd_SequenceOfExtendedString& theExtensions,
                                       const Handle(Message_Messenger)&  theMsgDriver) const
{

  ReadUserInfo(aFileName, START_EXT, END_EXT, theExtensions, theMsgDriver);
}

//=================================================================================================

void PCDM_ReadWriter_1::ReadUserInfo(const UtfString& aFileName,
                                     const AsciiString1&    Start,
                                     const AsciiString1&    End,
                                     TColStd_SequenceOfExtendedString& theUserInfo,
                                     const Handle(Message_Messenger)&)
{
  Standard_Integer           i;
  Handle(Storage_BaseDriver) theFileDriver;
  AsciiString1    aFileNameU(aFileName);
  if (PCDM1::FileDriverType(aFileNameU, theFileDriver) == PCDM_TOFD_Unknown)
    return;

  PCDM_ReadWriter::Open(theFileDriver, aFileName, Storage_VSRead);
  Handle(Storage_Schema) s = new Storage_Schema;
  Storage_HeaderData     hd;
  hd.Read(theFileDriver);
  const TColStd_SequenceOfAsciiString& refUserInfo = hd.UserInfo();

  Standard_Integer debut = 0, fin = 0;

  for (i = 1; i <= refUserInfo.Length(); i++)
  {
    UtfString theLine = refUserInfo(i);
    if (refUserInfo(i) == Start)
      debut = i;
    if (refUserInfo(i) == End)
      fin = i;
  }
  if (debut != 0)
  {
    for (i = debut + 1; i < fin; i++)
    {
      UtfString aInfoW(refUserInfo(i));
      theUserInfo.Append(aInfoW);
    }
  }
  theFileDriver->Close();
}

//=================================================================================================

Standard_Integer PCDM_ReadWriter_1::ReadDocumentVersion(
  const UtfString& aFileName,
  const Handle(Message_Messenger)&  theMsgDriver) const
{

  Standard_Integer           theVersion(-1);
  Handle(Storage_BaseDriver) theFileDriver;
  AsciiString1    aFileNameU(aFileName);
  if (PCDM1::FileDriverType(aFileNameU, theFileDriver) == PCDM_TOFD_Unknown)
    return theVersion;

  Standard_Boolean theFileIsOpen(Standard_False);

  try
  {
    OCC_CATCH_SIGNALS
    PCDM_ReadWriter::Open(theFileDriver, aFileName, Storage_VSRead);
    theFileIsOpen            = Standard_True;
    Handle(Storage_Schema) s = new Storage_Schema;
    Storage_HeaderData     hd;
    hd.Read(theFileDriver);
    const TColStd_SequenceOfAsciiString& refUserInfo = hd.UserInfo();

    Standard_Integer i;
    for (i = 1; i <= refUserInfo.Length(); i++)
    {
      if (refUserInfo(i).Search(MODIFICATION_COUNTER) != -1)
      {
        try
        {
          OCC_CATCH_SIGNALS theVersion = refUserInfo(i).Token(" ", 2).IntegerValue();
        }
        catch (ExceptionBase const&)
        {
          //	  std::cout << "warning: could not read the version in " << aFileName << std::endl;
          UtfString aMsg("Warning: ");
          aMsg = aMsg.Cat("could not read the version in ").Cat(aFileName).Cat("\0");
          if (!theMsgDriver.IsNull())
            theMsgDriver->Send(aMsg.ToExtString());
        }
      }
    }
  }

  catch (ExceptionBase const&)
  {
  }

  if (theFileIsOpen)
  {
    theFileDriver->Close();
  }

  return theVersion;
}
