// Created on: 1997-12-18
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

#ifndef _PCDM_Reader_HeaderFile
#define _PCDM_Reader_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <PCDM_ReaderStatus.hxx>
#include <Standard_Transient.hxx>
#include <Standard_IStream.hxx>
#include <Storage_Data.hxx>
#include <Message_ProgressRange.hxx>

class CDM_Document;
class UtfString;
class CDM_Application;
class ReaderFilter;

class Reader1;
DEFINE_STANDARD_HANDLE(Reader1, RefObject)

class Reader1 : public RefObject
{

public:
  //! retrieves the content of the file into a new Document.
  Standard_EXPORT virtual void Read(
    const UtfString& aFileName,
    const Handle(CDM_Document)&       aNewDocument,
    const Handle(CDM_Application)&    anApplication,
    const Handle(ReaderFilter)&  theFilter   = Handle(ReaderFilter)(),
    const Message_ProgressRange&      theProgress = Message_ProgressRange()) = 0;

  Standard_EXPORT virtual void Read(
    Standard_IStream&                theIStream,
    const Handle(Storage_Data)&      theStorageData,
    const Handle(CDM_Document)&      theDoc,
    const Handle(CDM_Application)&   theApplication,
    const Handle(ReaderFilter)& theFilter   = Handle(ReaderFilter)(),
    const Message_ProgressRange&     theProgress = Message_ProgressRange()) = 0;

  PCDM_ReaderStatus GetStatus() const;

  DEFINE_STANDARD_RTTIEXT(Reader1, RefObject)

protected:
  PCDM_ReaderStatus myReaderStatus;

private:
};

#include <PCDM_Reader.lxx>

#endif // _PCDM_Reader_HeaderFile
