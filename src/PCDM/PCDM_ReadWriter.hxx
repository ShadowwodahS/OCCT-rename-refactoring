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

#ifndef _PCDM_ReadWriter_HeaderFile
#define _PCDM_ReadWriter_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Standard_Transient.hxx>
#include <Standard_Integer.hxx>
#include <PCDM_SequenceOfReference.hxx>
#include <TColStd_SequenceOfExtendedString.hxx>
#include <Storage_OpenMode.hxx>
class AsciiString1;
class Storage_Data;
class CDM_Document;
class UtfString;
class Message_Messenger;
class Storage_BaseDriver;

class ReadWriter;
DEFINE_STANDARD_HANDLE(ReadWriter, RefObject)

class ReadWriter : public RefObject
{

public:
  //! returns PCDM_ReadWriter_1.
  Standard_EXPORT virtual AsciiString1 Version() const = 0;

  Standard_EXPORT virtual void WriteReferenceCounter(
    const Handle(Storage_Data)& aData,
    const Handle(CDM_Document)& aDocument) const = 0;

  Standard_EXPORT virtual void WriteReferences(
    const Handle(Storage_Data)&       aData,
    const Handle(CDM_Document)&       aDocument,
    const UtfString& theReferencerFileName) const = 0;

  Standard_EXPORT virtual void WriteExtensions(const Handle(Storage_Data)& aData,
                                               const Handle(CDM_Document)& aDocument) const = 0;

  Standard_EXPORT virtual void WriteVersion(const Handle(Storage_Data)& aData,
                                            const Handle(CDM_Document)& aDocument) const = 0;

  Standard_EXPORT virtual Standard_Integer ReadReferenceCounter(
    const UtfString& theFileName,
    const Handle(Message_Messenger)&  theMsgDriver) const = 0;

  Standard_EXPORT virtual void ReadReferences(
    const UtfString& aFileName,
    PCDM_SequenceOfReference&         theReferences,
    const Handle(Message_Messenger)&  theMsgDriver) const = 0;

  Standard_EXPORT virtual void ReadExtensions(
    const UtfString& aFileName,
    TColStd_SequenceOfExtendedString& theExtensions,
    const Handle(Message_Messenger)&  theMsgDriver) const = 0;

  Standard_EXPORT virtual Standard_Integer ReadDocumentVersion(
    const UtfString& aFileName,
    const Handle(Message_Messenger)&  theMsgDriver) const = 0;

  Standard_EXPORT static void Open(const Handle(Storage_BaseDriver)& aDriver,
                                   const UtfString& aFileName,
                                   const Storage_OpenMode            anOpenMode);

  //! returns the convenient Reader for a File.
  Standard_EXPORT static Handle(ReadWriter) Reader(
    const UtfString& aFileName);

  Standard_EXPORT static Handle(ReadWriter) Writer();

  Standard_EXPORT static void WriteFileFormat(const Handle(Storage_Data)& aData,
                                              const Handle(CDM_Document)& aDocument);

  //! tries  to get a format  in the  file.  returns an empty
  //! string if the file could not be read or does not have
  //! a FileFormat information.
  Standard_EXPORT static UtfString FileFormat(
    const UtfString& aFileName);

  //! tries  to get a format  from the stream.  returns an empty
  //! string if the file could not be read or does not have
  //! a FileFormat information.
  Standard_EXPORT static UtfString FileFormat(Standard_IStream&     theIStream,
                                                               Handle(Storage_Data)& theData);

  DEFINE_STANDARD_RTTIEXT(ReadWriter, RefObject)

protected:
private:
};

#endif // _PCDM_ReadWriter_HeaderFile
