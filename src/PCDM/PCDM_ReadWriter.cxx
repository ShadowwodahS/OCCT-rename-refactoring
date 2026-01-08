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
#include <PCDM.hxx>
#include <PCDM_DOMHeaderParser.hxx>
#include <PCDM_ReadWriter.hxx>
#include <PCDM_ReadWriter_1.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Type.hxx>
#include <Storage_BaseDriver.hxx>
#include <Storage_Data.hxx>
#include <Storage_HeaderData.hxx>
#include <Storage_TypeData.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TColStd_HSequenceOfAsciiString.hxx>
#include <UTL.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ReadWriter, RefObject)

#define FILE_FORMAT "FILE_FORMAT: "

static UtfString TryXmlDriverType(const AsciiString1& theFileName);

static UtfString TryXmlDriverType(Standard_IStream& theIStream);

//=================================================================================================

void ReadWriter::Open(const Handle(Storage_BaseDriver)& aDriver,
                           const UtfString& aFileName,
                           const Storage_OpenMode            aMode)
{
  Storage_Error error = UTL1::OpenFile(aDriver, aFileName, aMode);
  if (error != Storage_VSOk)
  {
    Standard_SStream aMsg;
    aMsg << "could not open the file: ";
    aMsg << aFileName;
    switch (error)
    {
      case Storage_VSOpenError:
        aMsg << "; file was not found or permission denied";
        break;
      case Storage_VSAlreadyOpen:
        aMsg << "; file was already opened";
      default:
        break;
    }
    aMsg << (char)0;
    throw ExceptionBase(aMsg.str().c_str());
  }
}

//=================================================================================================

Handle(ReadWriter) ReadWriter::Reader(const UtfString&)
{
  return (new PCDM_ReadWriter_1);
}

//=================================================================================================

Handle(ReadWriter) ReadWriter::Writer()
{
  return (new PCDM_ReadWriter_1);
}

//=================================================================================================

void ReadWriter::WriteFileFormat(const Handle(Storage_Data)& aData,
                                      const Handle(CDM_Document)& aDocument)
{
  AsciiString1 ligne(FILE_FORMAT);
  ligne += AsciiString1(aDocument->StorageFormat(), '?');

  aData->AddToUserInfo(ligne);
}

//=================================================================================================

UtfString ReadWriter::FileFormat(const UtfString& aFileName)
{
  UtfString theFormat;

  Handle(Storage_BaseDriver) theFileDriver;

  // conversion to UTF-8 is done inside
  AsciiString1 theFileName(aFileName);
  if (PCDM1::FileDriverType(theFileName, theFileDriver) == PCDM_TOFD_Unknown)
    return ::TryXmlDriverType(theFileName);

  Standard_Boolean theFileIsOpen(Standard_False);
  try
  {
    OCC_CATCH_SIGNALS

    Open(theFileDriver, aFileName, Storage_VSRead);
    theFileIsOpen = Standard_True;
    Storage_HeaderData hd;
    hd.Read(theFileDriver);
    const TColStd_SequenceOfAsciiString& refUserInfo = hd.UserInfo();
    Standard_Boolean                     found       = Standard_False;
    for (Standard_Integer i = 1; !found && i <= refUserInfo.Length(); i++)
    {
      if (refUserInfo(i).Search(FILE_FORMAT) != -1)
      {
        found = Standard_True;
        theFormat =
          UtfString(refUserInfo(i).Token(" ", 2).ToCString(), Standard_True);
      }
    }
    if (!found)
    {
      Storage_TypeData td;
      td.Read(theFileDriver);
      theFormat = td.Types()->Value(1);
    }
  }
  catch (ExceptionBase const&)
  {
  }

  if (theFileIsOpen)
  {
    theFileDriver->Close();
  }

  return theFormat;
}

//=================================================================================================

UtfString ReadWriter::FileFormat(Standard_IStream&     theIStream,
                                                       Handle(Storage_Data)& theData)
{
  UtfString aFormat;

  Handle(Storage_BaseDriver) aFileDriver;
  if (PCDM1::FileDriverType(theIStream, aFileDriver) == PCDM_TOFD_XmlFile)
  {
    return ::TryXmlDriverType(theIStream);
  }
  if (!aFileDriver)
  {
    // type is not recognized, return empty string
    return aFormat;
  }

  aFileDriver->ReadCompleteInfo(theIStream, theData);

  for (Standard_Integer i = 1; i <= theData->HeaderData()->UserInfo().Length(); i++)
  {
    const AsciiString1& aLine = theData->HeaderData()->UserInfo().Value(i);

    if (aLine.Search(FILE_FORMAT) != -1)
    {
      aFormat = UtfString(aLine.Token(" ", 2).ToCString(), Standard_True);
    }
  }

  return aFormat;
}

//=======================================================================
// function : ::TryXmlDriverType
// purpose  : called from FileFormat()
//=======================================================================

static UtfString TryXmlDriverType(const AsciiString1& theFileName)
{
  UtfString theFormat;
  PCDM_DOMHeaderParser       aParser;
  const char*                aDocumentElementName = "document";
  aParser.SetStartElementName(Standard_CString(aDocumentElementName));

  // Parse the file; if there is no error or an error appears before retrieval
  // of the DocumentElement, the XML format cannot be defined
  if (aParser.parse(theFileName.ToCString()))
  {
    const LDOM_Element& anElement = aParser.GetElement();
    if (anElement.getTagName().equals(LDOMString(aDocumentElementName)))
      theFormat = anElement.getAttribute("format");
  }
  return theFormat;
}

//=======================================================================
// function : ::TryXmlDriverType
// purpose  : called from FileFormat()
//=======================================================================

static UtfString TryXmlDriverType(Standard_IStream& theIStream)
{
  UtfString theFormat;
  PCDM_DOMHeaderParser       aParser;
  const char*                aDocumentElementName = "document";
  aParser.SetStartElementName(Standard_CString(aDocumentElementName));

  if (theIStream.good())
  {
    // Parse the file; if there is no error or an error appears before retrieval
    // of the DocumentElement, the XML format cannot be defined
    if (aParser.parse(theIStream, Standard_True))
    {
      const LDOM_Element& anElement = aParser.GetElement();
      if (anElement.getTagName().equals(LDOMString(aDocumentElementName)))
        theFormat = anElement.getAttribute("format");
    }
  }

  return theFormat;
}
