// Created on: 1999-06-30
// Created by: Denis PASCAL
// Copyright (c) 1999-1999 Matra Datavision
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

#include <TDocStd_Application.hxx>

#include <CDF_Directory.hxx>
#include <CDF_DirectoryIterator.hxx>
#include <CDF_Store.hxx>
#include <PCDM_RetrievalDriver.hxx>
#include <PCDM_StorageDriver.hxx>
#include <PCDM_ReaderFilter.hxx>
#include <Plugin_Failure.hxx>
#include <Resource_Manager.hxx>
#include <Standard_DomainError.hxx>
#include <Standard_Dump.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_NoSuchObject.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TDocStd_Document.hxx>
#include <TDocStd_Owner.hxx>
#include <TDocStd_PathParser.hxx>
#include <OSD_Thread.hxx>

IMPLEMENT_STANDARD_RTTIEXT(AppManager, CDF_Application)

// TDocStd_Owner attribute have pointer of closed AppDocument
//=================================================================================================

AppManager::AppManager()
    : myIsDriverLoaded(Standard_True)
{
  if (myMetaDataDriver.IsNull())
    myIsDriverLoaded = Standard_False;
}

//=================================================================================================

Standard_Boolean AppManager::IsDriverLoaded() const
{
  return myIsDriverLoaded;
}

//=================================================================================================

Handle(Resource_Manager) AppManager::Resources()
{
  if (myResources.IsNull())
  {
    myResources = new Resource_Manager(ResourcesName());
  }
  return myResources;
}

//=================================================================================================

Standard_CString AppManager::ResourcesName()
{
  return "";
}

//=================================================================================================

void AppManager::DefineFormat(const AsciiString1&      theFormat,
                                       const AsciiString1&      theDescription,
                                       const AsciiString1&      theExtension,
                                       const Handle(PCDM_RetrievalDriver)& theReader,
                                       const Handle(PCDM_StorageDriver)&   theWriter)
{
  // register resources for CDM mechanics to work
  Handle(Resource_Manager) aResources = Resources();
  aResources->SetResource((theFormat + ".Description").ToCString(), theDescription.ToCString());
  aResources->SetResource((theFormat + ".FileExtension").ToCString(), theExtension.ToCString());
  aResources->SetResource((theExtension + ".FileFormat").ToCString(), theFormat.ToCString());

  // set format ID in the drivers to allow them putting it in
  // the OCAF documents opened by these drivers
  if (!theReader.IsNull())
    theReader->SetFormat(theFormat);
  if (!theWriter.IsNull())
    theWriter->SetFormat(theFormat);

  // register drivers
  myReaders.Add(theFormat, theReader);
  myWriters.Add(theFormat, theWriter);
}

//=================================================================================================

void AppManager::ReadingFormats(TColStd_SequenceOfAsciiString& theFormats)
{
  theFormats.Clear();

  NCollection_IndexedDataMap<UtfString, Handle(PCDM_RetrievalDriver)>::Iterator
    anIter(myReaders);
  for (; anIter.More(); anIter.Next())
  {
    const Handle(PCDM_RetrievalDriver)& aDriver = anIter.Value();
    if (aDriver.IsNull() == Standard_False)
    {
      theFormats.Append(anIter.Key1());
    }
  }
}

//=================================================================================================

void AppManager::WritingFormats(TColStd_SequenceOfAsciiString& theFormats)
{
  theFormats.Clear();

  NCollection_IndexedDataMap<UtfString, Handle(PCDM_StorageDriver)>::Iterator
    anIter(myWriters);
  for (; anIter.More(); anIter.Next())
  {
    const Handle(PCDM_StorageDriver)& aDriver = anIter.Value();
    if (aDriver.IsNull() == Standard_False)
    {
      theFormats.Append(anIter.Key1());
    }
  }
}

//=================================================================================================

Standard_Integer AppManager::NbDocuments() const
{
  return myDirectory->Length();
}

//=================================================================================================

void AppManager::GetDocument(const Standard_Integer    index,
                                      Handle(AppDocument)& theDoc) const
{
  DirectoryIterator it(myDirectory);
  Standard_Integer      current = 0;
  for (; it.MoreDocument(); it.NextDocument())
  {
    current++;
    if (index == current)
    {
      Handle(AppDocument) D = Handle(AppDocument)::DownCast(it.Document());
      theDoc                     = D;
      return;
    }
  }
}

//=================================================================================================

void AppManager::NewDocument(const UtfString& format,
                                      Handle(CDM_Document)&             theDoc)
{
  Handle(AppDocument) D = new AppDocument(format);
  InitDocument(D);
  CDF_Application::Open(D); // add the document in the session
  theDoc = D;
}

//=======================================================================
// function : NewDocument
// purpose  : A non-virtual method taking a TDocStd_Documment object as an input.
//         : Internally it calls a virtual method NewDocument() with CDM_Document object.
//=======================================================================

void AppManager::NewDocument(const UtfString& format,
                                      Handle(AppDocument)&         theDoc)
{
  Handle(CDM_Document) aCDMDoc;
  NewDocument(format, aCDMDoc);
  theDoc = Handle(AppDocument)::DownCast(aCDMDoc);
}

//=================================================================================================

void AppManager::InitDocument(const Handle(CDM_Document)& /*aDoc*/) const {}

//=================================================================================================

void AppManager::Close(const Handle(AppDocument)& theDoc)
{
  if (theDoc.IsNull())
  {
    return;
  }

  Handle(TDocStd_Owner) Owner;
  if (theDoc->Main().Root().FindAttribute(TDocStd_Owner::GetID(), Owner))
  {
    Handle(AppDocument) emptyDoc;
    Owner->SetDocument(emptyDoc);
  }
  theDoc->BeforeClose();
  CDF_Application::Close(theDoc);
}

//=================================================================================================

Standard_Integer AppManager::IsInSession(const UtfString& path) const
{
  UtfString unifiedPath(path);
  unifiedPath.ChangeAll('/', '|');
  unifiedPath.ChangeAll('\\', '|');

  Standard_Integer         nbdoc = NbDocuments();
  Handle(AppDocument) D;
  for (Standard_Integer i = 1; i <= nbdoc; i++)
  {
    GetDocument(i, D);
    if (D->IsSaved())
    {
      UtfString unifiedDocPath(D->GetPath());
      unifiedDocPath.ChangeAll('/', '|');
      unifiedDocPath.ChangeAll('\\', '|');

      if (unifiedPath == unifiedDocPath)
        return i;
    }
  }
  return 0;
}

//=================================================================================================

PCDM_ReaderStatus AppManager::Open(const UtfString& path,
                                            Handle(AppDocument)&         theDoc,
                                            const Handle(ReaderFilter)&  theFilter,
                                            const Message_ProgressRange&      theRange)
{
  PCDM_ReaderStatus          status = PCDM_RS_DriverFailure;
  PathParser         tool(path);
  UtfString directory = tool.Trek();
  UtfString file      = tool.Name();
  file += ".";
  file += tool.Extension();
  status = CanRetrieve(directory, file, !theFilter.IsNull() && theFilter->IsAppendMode());

  if (status != PCDM_RS_OK)
  {
    return status;
  }

  try
  {
    OCC_CATCH_SIGNALS
    Handle(AppDocument) D = Handle(AppDocument)::DownCast(
      Retrieve(directory, file, Standard_True, theFilter, theRange));
    if (theFilter.IsNull() || !theFilter->IsAppendMode())
      CDF_Application::Open(D);
    theDoc = D;
  }
  catch (ExceptionBase const& anException)
  {
    //    status = GetRetrieveStatus();
    if (!MessageDriver().IsNull())
    {
      //      Standard_SStream aMsg;
      //      aMsg << ExceptionBase::Caught() << std::endl;
      //      std::cout << "AppManager::Open(): " << aMsg.rdbuf()->str() << std::endl;
      UtfString aString(anException.GetMessageString());
      MessageDriver()->Send(aString.ToExtString(), Message_Fail);
    }
  }
  status = GetRetrieveStatus();
#ifdef OCCT_DEBUG
  std::cout << "AppManager::Open(): The status = " << status << std::endl;
#endif
  return status;
}

//=================================================================================================

PCDM_ReaderStatus AppManager::Open(Standard_IStream&                theIStream,
                                            Handle(AppDocument)&        theDoc,
                                            const Handle(ReaderFilter)& theFilter,
                                            const Message_ProgressRange&     theRange)
{
  try
  {
    OCC_CATCH_SIGNALS
    Handle(CDM_Document) aCDMDoc = theDoc;
    Read(theIStream, aCDMDoc, theFilter, theRange);
    // Read calls NewDocument of AppManager, so, it should the AppDocument in the
    // result anyway
    theDoc = Handle(AppDocument)::DownCast(aCDMDoc);

    if (!theDoc.IsNull() && (theFilter.IsNull() || !theFilter->IsAppendMode()))
    {
      CDF_Application::Open(theDoc);
    }
  }

  catch (ExceptionBase const& anException)
  {
    if (!MessageDriver().IsNull())
    {
      UtfString aFailureMessage(anException.GetMessageString());
      MessageDriver()->Send(aFailureMessage.ToExtString(), Message_Fail);
    }
  }
  return GetRetrieveStatus();
}

//=================================================================================================

PCDM_StoreStatus AppManager::SaveAs(const Handle(AppDocument)&   theDoc,
                                             const UtfString& path,
                                             const Message_ProgressRange&      theRange)
{
  PathParser         tool(path);
  UtfString directory = tool.Trek();
  UtfString file      = tool.Name();
  file += ".";
  file += tool.Extension();
  theDoc->Open(this);
  CDF_Store storer(theDoc);
  if (!storer.SetFolder(directory))
  {
    UtfString aMsg("AppManager::SaveAs() - folder ");
    aMsg += directory;
    aMsg += " does not exist";
    if (!MessageDriver().IsNull())
      MessageDriver()->Send(aMsg.ToExtString(), Message_Fail);
    return storer.StoreStatus(); // CDF_SS_Failure;
  }
  storer.SetName(file);
  try
  {
    OCC_CATCH_SIGNALS
    storer.Realize(theRange);
  }
  catch (ExceptionBase const& anException)
  {
    if (!MessageDriver().IsNull())
    {
      UtfString aString(anException.GetMessageString());
      MessageDriver()->Send(aString.ToExtString(), Message_Fail);
    }
  }
  if (storer.StoreStatus() == PCDM_SS_OK)
    theDoc->SetSaved();
  else if (!MessageDriver().IsNull())
    MessageDriver()->Send(storer.AssociatedStatusText(), Message_Fail);
#ifdef OCCT_DEBUG
  std::cout << "AppManager::SaveAs(): The status = " << storer.StoreStatus() << std::endl;
#endif
  return storer.StoreStatus();
}

//=================================================================================================

PCDM_StoreStatus AppManager::SaveAs(const Handle(AppDocument)& theDoc,
                                             Standard_OStream&               theOStream,
                                             const Message_ProgressRange&    theRange)
{
  try
  {
    Handle(PCDM_StorageDriver) aDocStorageDriver = WriterFromFormat(theDoc->StorageFormat());

    if (aDocStorageDriver.IsNull())
    {
      return PCDM_SS_DriverFailure;
    }

    aDocStorageDriver->SetFormat(theDoc->StorageFormat());
    aDocStorageDriver->Write(theDoc, theOStream, theRange);

    if (aDocStorageDriver->GetStoreStatus() == PCDM_SS_OK)
    {
      theDoc->SetSaved();
    }

    return aDocStorageDriver->GetStoreStatus();
  }
  catch (ExceptionBase const& anException)
  {
    if (!MessageDriver().IsNull())
    {
      UtfString aString(anException.GetMessageString());
      MessageDriver()->Send(aString.ToExtString(), Message_Fail);
    }
  }
  return PCDM_SS_Failure;
}

//=================================================================================================

PCDM_StoreStatus AppManager::Save(const Handle(AppDocument)& D,
                                           const Message_ProgressRange&    theRange)
{
  PCDM_StoreStatus status = PCDM_SS_OK;
  if (D->IsSaved())
  {
    CDF_Store storer(D);
    try
    {
      OCC_CATCH_SIGNALS
      storer.Realize(theRange);
    }
    catch (ExceptionBase const& anException)
    {
      if (!MessageDriver().IsNull())
      {
        UtfString aString(anException.GetMessageString());
        MessageDriver()->Send(aString.ToExtString(), Message_Fail);
      }
    }
    if (storer.StoreStatus() == PCDM_SS_OK)
      D->SetSaved();
    status = storer.StoreStatus();
  }
  else
  {
    if (!MessageDriver().IsNull())
    {
      UtfString aMsg("Document has not been saved yet");
      MessageDriver()->Send(aMsg.ToExtString(), Message_Fail);
    }
    status = PCDM_SS_Failure;
  }
#ifdef OCCT_DEBUG
  std::cout << "AppManager::Save(): The status = " << status << std::endl;
#endif
  return status;
}

//=================================================================================================

PCDM_StoreStatus AppManager::SaveAs(const Handle(AppDocument)&   D,
                                             const UtfString& path,
                                             UtfString&       theStatusMessage,
                                             const Message_ProgressRange&      theRange)
{
  PathParser         tool(path);
  PCDM_StoreStatus           aStatus   = PCDM_SS_Failure;
  UtfString directory = tool.Trek();
  UtfString file      = tool.Name();
  file += ".";
  file += tool.Extension();
  D->Open(this);
  CDF_Store storer(D);
  if (storer.SetFolder(directory))
  {
    storer.SetName(file);
    try
    {
      OCC_CATCH_SIGNALS
      storer.Realize(theRange);
    }
    catch (ExceptionBase const& anException)
    {
      if (!MessageDriver().IsNull())
      {
        UtfString aString(anException.GetMessageString());
        MessageDriver()->Send(aString.ToExtString(), Message_Fail);
      }
    }
    if (storer.StoreStatus() == PCDM_SS_OK)
      D->SetSaved();
    theStatusMessage = storer.AssociatedStatusText();
    aStatus          = storer.StoreStatus();
  }
  else
  {
    theStatusMessage = UtfString("AppManager::SaveAs"
                                                  ": No such directory ")
                       + directory;
    aStatus = PCDM_SS_Failure;
  }
  return aStatus;
}

//=================================================================================================

PCDM_StoreStatus AppManager::SaveAs(const Handle(AppDocument)& theDoc,
                                             Standard_OStream&               theOStream,
                                             UtfString&     theStatusMessage,
                                             const Message_ProgressRange&    theRange)
{
  try
  {
    Handle(PCDM_StorageDriver) aDocStorageDriver = WriterFromFormat(theDoc->StorageFormat());
    if (aDocStorageDriver.IsNull())
    {
      theStatusMessage =
        UtfString("AppManager::SaveAs: no storage driver");
      return PCDM_SS_DriverFailure;
    }

    aDocStorageDriver->SetFormat(theDoc->StorageFormat());
    aDocStorageDriver->Write(theDoc, theOStream, theRange);

    if (aDocStorageDriver->GetStoreStatus() == PCDM_SS_OK)
    {
      theDoc->SetSaved();
    }

    return aDocStorageDriver->GetStoreStatus();
  }
  catch (ExceptionBase const& anException)
  {
    if (!MessageDriver().IsNull())
    {
      UtfString aString(anException.GetMessageString());
      MessageDriver()->Send(aString.ToExtString(), Message_Fail);
    }
  }
  return PCDM_SS_Failure;
}

//=================================================================================================

PCDM_StoreStatus AppManager::Save(const Handle(AppDocument)& D,
                                           UtfString&     theStatusMessage,
                                           const Message_ProgressRange&    theRange)
{
  PCDM_StoreStatus status = PCDM_SS_OK;
  if (D->IsSaved())
  {
    CDF_Store storer(D);
    try
    {
      OCC_CATCH_SIGNALS
      storer.Realize(theRange);
    }
    catch (ExceptionBase const& anException)
    {
      if (!MessageDriver().IsNull())
      {
        UtfString aString(anException.GetMessageString());
        MessageDriver()->Send(aString.ToExtString(), Message_Fail);
      }
    }
    if (storer.StoreStatus() == PCDM_SS_OK)
      D->SetSaved();
    status           = storer.StoreStatus();
    theStatusMessage = storer.AssociatedStatusText();
  }
  else
  {
    theStatusMessage = "AppManager::the document has not been saved yet";
    status           = PCDM_SS_Failure;
  }
  return status;
}

//=================================================================================================

void AppManager::OnOpenTransaction(const Handle(AppDocument)&)
{
  // nothing to do on this level
}

//=================================================================================================

void AppManager::OnAbortTransaction(const Handle(AppDocument)&)
{
  // nothing to do on this level
}

//=================================================================================================

void AppManager::OnCommitTransaction(const Handle(AppDocument)&)
{
  // nothing to do on this level
}

//=================================================================================================

void AppManager::DumpJson(Standard_OStream& theOStream, Standard_Integer) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myIsDriverLoaded)
}
