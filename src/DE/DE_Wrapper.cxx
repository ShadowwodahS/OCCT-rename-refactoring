// Copyright (c) 2022 OPEN CASCADE SAS
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

#include <DE_Wrapper.hxx>

#include <DE_ConfigurationContext.hxx>
#include <DE_ConfigurationNode.hxx>
#include <DE_Provider.hxx>
#include <Message_ProgressRange.hxx>
#include <NCollection_Buffer.hxx>
#include <OSD_File.hxx>
#include <OSD_FileSystem.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <Standard_ErrorHandler.hxx>
#include <TopoDS_Shape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DataExchangeWrapper, RefObject)

namespace
{
static const AsciiString1& THE_CONFIGURATION_SCOPE()
{
  static const AsciiString1 aScope("global");
  return aScope;
}

static Handle(DataExchangeWrapper)& THE_GLOBAL_CONFIGURATION()
{
  static Handle(DataExchangeWrapper) aConf = new DataExchangeWrapper();
  return aConf;
}
} // namespace

//=================================================================================================

DataExchangeWrapper::DataExchangeWrapper()
    : myKeepUpdates(Standard_False)
{
}

//=================================================================================================

DataExchangeWrapper::DataExchangeWrapper(const Handle(DataExchangeWrapper)& theWrapper)
    : DataExchangeWrapper()
{
  if (theWrapper.IsNull())
  {
    return;
  }
  GlobalParameters = theWrapper->GlobalParameters;
  for (DE_ConfigurationFormatMap::Iterator aFormatIter(theWrapper->Nodes()); aFormatIter.More();
       aFormatIter.Next())
  {
    for (DE_ConfigurationVendorMap::Iterator aVendorIter(aFormatIter.Value()); aVendorIter.More();
         aVendorIter.Next())
    {
      Bind(aVendorIter.Value());
    }
  }
  theWrapper->myKeepUpdates = myKeepUpdates;
}

//=================================================================================================

const Handle(DataExchangeWrapper)& DataExchangeWrapper::GlobalWrapper()
{
  return THE_GLOBAL_CONFIGURATION();
}

//=================================================================================================

void DataExchangeWrapper::SetGlobalWrapper(const Handle(DataExchangeWrapper)& theWrapper)
{
  if (!theWrapper.IsNull())
  {
    THE_GLOBAL_CONFIGURATION() = theWrapper;
  }
}

//=================================================================================================

Standard_Mutex& DataExchangeWrapper::GlobalLoadMutex()
{
  static Standard_Mutex THE_GLOBAL_LOAD_MUTEX;
  return THE_GLOBAL_LOAD_MUTEX;
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Read(const AsciiString1&  thePath,
                                  const Handle(AppDocument)& theDocument,
                                  Handle(ExchangeSession)&  theWS,
                                  const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    return Standard_False;
  }
  if (theWS.IsNull())
  {
    return Read(thePath, theDocument, theProgress);
  }
  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_True, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Read(thePath, theDocument, theWS, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Write(const AsciiString1&  thePath,
                                   const Handle(AppDocument)& theDocument,
                                   Handle(ExchangeSession)&  theWS,
                                   const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    return Standard_False;
  }
  if (theWS.IsNull())
  {
    return Write(thePath, theDocument, theProgress);
  }
  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_False, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Write(thePath, theDocument, theWS, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Read(const AsciiString1&  thePath,
                                  const Handle(AppDocument)& theDocument,
                                  const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    return Standard_False;
  }
  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_True, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Read(thePath, theDocument, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Write(const AsciiString1&  thePath,
                                   const Handle(AppDocument)& theDocument,
                                   const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    return Standard_False;
  }
  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_False, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Write(thePath, theDocument, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Read(const AsciiString1& thePath,
                                  TopoShape&                  theShape,
                                  Handle(ExchangeSession)& theWS,
                                  const Message_ProgressRange&   theProgress)
{
  if (theWS.IsNull())
  {
    return Read(thePath, theShape, theProgress);
  }
  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_True, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Read(thePath, theShape, theWS, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Write(const AsciiString1& thePath,
                                   const TopoShape&            theShape,
                                   Handle(ExchangeSession)& theWS,
                                   const Message_ProgressRange&   theProgress)
{
  if (theWS.IsNull())
  {
    return Write(thePath, theShape, theProgress);
  }
  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_False, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Write(thePath, theShape, theWS, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Read(const AsciiString1& thePath,
                                  TopoShape&                  theShape,
                                  const Message_ProgressRange&   theProgress)
{

  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_True, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Read(thePath, theShape, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Write(const AsciiString1& thePath,
                                   const TopoShape&            theShape,
                                   const Message_ProgressRange&   theProgress)
{
  Handle(DE_Provider) aProvider;
  if (!FindProvider(thePath, Standard_False, aProvider))
  {
    return Standard_False;
  }
  return aProvider->Write(thePath, theShape, theProgress);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Load(const AsciiString1& theResource,
                                  const Standard_Boolean         theIsRecursive)
{
  Handle(ConfigurationContext) aResource = new ConfigurationContext();
  aResource->Load(theResource);
  return Load(aResource, theIsRecursive);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Load(const Handle(ConfigurationContext)& theResource,
                                  const Standard_Boolean                 theIsRecursive)
{
  GlobalParameters.LengthUnit = theResource->RealVal("general.length.unit",
                                                     GlobalParameters.LengthUnit,
                                                     THE_CONFIGURATION_SCOPE());
  GlobalParameters.SystemUnit = theResource->RealVal("general.system.unit",
                                                     GlobalParameters.SystemUnit,
                                                     THE_CONFIGURATION_SCOPE());
  if (theIsRecursive)
  {
    for (DE_ConfigurationFormatMap::Iterator aFormatIter(myConfiguration); aFormatIter.More();
         aFormatIter.Next())
    {
      for (DE_ConfigurationVendorMap::Iterator aVendorIter(aFormatIter.Value()); aVendorIter.More();
           aVendorIter.Next())
      {
        aVendorIter.Value()->Load(theResource);
      }
    }
    sort(theResource);
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Save(const AsciiString1&   theResourcePath,
                                  const Standard_Boolean           theIsRecursive,
                                  const TColStd_ListOfAsciiString& theFormats,
                                  const TColStd_ListOfAsciiString& theVendors)
{
  SystemPath       aPath = theResourcePath;
  SystemFile       aFile(aPath);
  Protection1 aProt;
  {
    try
    {
      OCC_CATCH_SIGNALS
      aFile.Build(OSD_ReadWrite, aProt);
    }
    catch (ExceptionBase const&)
    {
      return Standard_False;
    }
  }
  if (aFile.Failed())
  {
    return Standard_False;
  }
  AsciiString1 aResConfiguration = Save(theIsRecursive, theFormats, theVendors);
  aFile.Write(aResConfiguration, aResConfiguration.Length());
  aFile.Close();
  return Standard_True;
}

//=================================================================================================

AsciiString1 DataExchangeWrapper::Save(const Standard_Boolean           theIsRecursive,
                                         const TColStd_ListOfAsciiString& theFormats,
                                         const TColStd_ListOfAsciiString& theVendors)
{
  AsciiString1 aResult;
  aResult += "!Description of the config file for DE toolkit\n";
  aResult += "!*****************************************************************************\n";
  aResult += "!\n";
  aResult += "!Format of the file is compliant with the standard Open CASCADE resource files\n";
  aResult += "!Each key defines a sequence of either further keys.\n";
  aResult += "!Keys can be nested down to an arbitrary level.\n";
  aResult += "!\n";
  aResult += "!*****************************************************************************\n";
  aResult += "!DataExchangeWrapper\n";
  aResult += "!Priority vendor list. For every CAD format set indexed list of vendors\n";
  for (DE_ConfigurationFormatMap::Iterator aFormatIter(myConfiguration); aFormatIter.More();
       aFormatIter.Next())
  {
    const AsciiString1& aFormat = aFormatIter.Key1();
    aResult += THE_CONFIGURATION_SCOPE() + '.' + "priority" + '.' + aFormat + " :\t ";
    for (DE_ConfigurationVendorMap::Iterator aVendorIter(aFormatIter.Value()); aVendorIter.More();
         aVendorIter.Next())
    {
      const AsciiString1& aVendorName = aVendorIter.Value()->GetVendor();
      aResult += aVendorName + " ";
    }
    aResult += "\n";
  }
  aResult += "!Global parameters. Used for all providers\n";
  aResult += "!Length scale unit value. Should be more than 0. Default value: 1.0(MM)\n";
  aResult +=
    THE_CONFIGURATION_SCOPE() + ".general.length.unit :\t " + GlobalParameters.LengthUnit + "\n";
  aResult += "!System unit value. Should be more than 0. Default value: 1.0(MM)\n";
  aResult +=
    THE_CONFIGURATION_SCOPE() + ".general.system.unit :\t " + GlobalParameters.SystemUnit + "\n";
  if (theIsRecursive)
  {
    for (DE_ConfigurationFormatMap::Iterator aFormatIter(myConfiguration); aFormatIter.More();
         aFormatIter.Next())
    {
      if (!theFormats.IsEmpty() && !theFormats.Contains(aFormatIter.Key1()))
      {
        continue;
      }
      for (DE_ConfigurationVendorMap::Iterator aVendorIter(aFormatIter.Value()); aVendorIter.More();
           aVendorIter.Next())
      {
        if (!theVendors.IsEmpty() && !theVendors.Contains(aVendorIter.Key1()))
        {
          continue;
        }
        aResult += "!\n";
        aResult += aVendorIter.Value()->Save();
        aResult += "!\n";
      }
    }
  }
  aResult += "!*****************************************************************************\n";
  return aResult;
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Bind(const Handle(ConfigurationNode)& theNode)
{
  if (theNode.IsNull())
  {
    return Standard_False;
  }
  const AsciiString1 aFileFormat = theNode->GetFormat();
  const AsciiString1 aVendorName = theNode->GetVendor();
  DE_ConfigurationVendorMap*    aVendorMap  = myConfiguration.ChangeSeek(aFileFormat);
  if (aVendorMap == NULL)
  {
    DE_ConfigurationVendorMap aTmpVendorMap;
    aVendorMap = myConfiguration.Bound(aFileFormat, aTmpVendorMap);
  }
  return aVendorMap->Add(aVendorName, theNode) > 0;
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::UnBind(const Handle(ConfigurationNode)& theNode)
{
  if (theNode.IsNull())
  {
    return false;
  }
  const AsciiString1 aFileFormat = theNode->GetFormat();
  const AsciiString1 aVendorName = theNode->GetVendor();
  DE_ConfigurationVendorMap*    aVendorMap  = myConfiguration.ChangeSeek(aFileFormat);
  if (aVendorMap == NULL)
  {
    return false;
  }
  const auto aPrevSize = aVendorMap->Size();
  aVendorMap->RemoveKey(aVendorName);
  return aVendorMap->Size() != aPrevSize;
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::Find(const AsciiString1& theFormat,
                                  const AsciiString1& theVendor,
                                  Handle(ConfigurationNode)&  theNode) const
{
  const DE_ConfigurationVendorMap* aVendorMap = myConfiguration.Seek(theFormat);
  return aVendorMap != nullptr && aVendorMap->FindFromKey(theVendor, theNode);
}

//=================================================================================================

void DataExchangeWrapper::ChangePriority(const AsciiString1&   theFormat,
                                const TColStd_ListOfAsciiString& theVendorPriority,
                                const Standard_Boolean           theToDisable)
{
  DE_ConfigurationVendorMap aVendorMap;
  if (!myConfiguration.Find(theFormat, aVendorMap))
  {
    return;
  }
  DE_ConfigurationVendorMap aNewVendorMap;
  // Sets according to the input priority
  for (TColStd_ListOfAsciiString::Iterator aPriorIter(theVendorPriority); aPriorIter.More();
       aPriorIter.Next())
  {
    const AsciiString1& aVendorName = aPriorIter.Value();
    Handle(ConfigurationNode)   aNode;
    if (aVendorMap.FindFromKey(aVendorName, aNode))
    {
      aNode->SetEnabled(Standard_True);
      aNewVendorMap.Add(aVendorName, aNode);
    }
  }
  // Sets not used elements
  for (DE_ConfigurationVendorMap::Iterator aVendorIter(aVendorMap); aVendorIter.More();
       aVendorIter.Next())
  {
    const AsciiString1& aVendorName = aVendorIter.Key1();
    if (!theVendorPriority.Contains(aVendorName))
    {
      const Handle(ConfigurationNode)& aNode = aVendorIter.Value();
      if (theToDisable)
      {
        aNode->SetEnabled(Standard_False);
      }
      aNewVendorMap.Add(aVendorName, aNode);
    }
  }
  myConfiguration.Bind(theFormat, aNewVendorMap);
}

//=================================================================================================

void DataExchangeWrapper::ChangePriority(const TColStd_ListOfAsciiString& theVendorPriority,
                                const Standard_Boolean           theToDisable)
{
  for (DE_ConfigurationFormatMap::Iterator aFormatIter(myConfiguration); aFormatIter.More();
       aFormatIter.Next())
  {
    ChangePriority(aFormatIter.Key1(), theVendorPriority, theToDisable);
  }
}

//=================================================================================================

const DE_ConfigurationFormatMap& DataExchangeWrapper::Nodes() const
{
  return myConfiguration;
}

//=================================================================================================

Handle(DataExchangeWrapper) DataExchangeWrapper::Copy() const
{
  return new DataExchangeWrapper(*this);
}

//=================================================================================================

Standard_Boolean DataExchangeWrapper::FindProvider(const AsciiString1& thePath,
                                          const Standard_Boolean         theToImport,
                                          Handle(DE_Provider)&           theProvider) const
{
  Handle(NCollection_Buffer) aBuffer;
  if (theToImport)
  {
    const Handle(OSD_FileSystem)& aFileSystem = OSD_FileSystem::DefaultFileSystem();
    std::shared_ptr<std::istream> aStream =
      aFileSystem->OpenIStream(thePath, std::ios::in | std::ios::binary);
    if (aStream.get() != nullptr)
    {
      aBuffer = new NCollection_Buffer(NCollection_BaseAllocator::CommonBaseAllocator(), 2048);
      aStream->read((char*)aBuffer->ChangeData(), 2048);
      aBuffer->ChangeData()[2047] = '\0';
    }
  }
  SystemPath                      aPath(thePath);
  const AsciiString1 anExtr = aPath.Extension();
  for (DE_ConfigurationFormatMap::Iterator aFormatIter(myConfiguration); aFormatIter.More();
       aFormatIter.Next())
  {
    for (DE_ConfigurationVendorMap::Iterator aVendorIter(aFormatIter.Value()); aVendorIter.More();
         aVendorIter.Next())
    {
      const Handle(ConfigurationNode)& aNode = aVendorIter.Value();
      if (aNode->IsEnabled()
          && ((theToImport && aNode->IsImportSupported())
              || (!theToImport && aNode->IsExportSupported()))
          && (aNode->CheckExtension(anExtr) || (theToImport && aNode->CheckContent(aBuffer)))
          && aNode->UpdateLoad(theToImport, myKeepUpdates))
      {
        theProvider             = aNode->BuildProvider();
        aNode->GlobalParameters = GlobalParameters;
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_EXPORT void DataExchangeWrapper::UpdateLoad(const Standard_Boolean theToForceUpdate) const
{
  for (DE_ConfigurationFormatMap::Iterator aFormatIter(myConfiguration); aFormatIter.More();
       aFormatIter.Next())
  {
    for (DE_ConfigurationVendorMap::Iterator aVendorIter(aFormatIter.Value()); aVendorIter.More();
         aVendorIter.Next())
    {
      const Handle(ConfigurationNode)& aNode = aVendorIter.Value();
      aNode->UpdateLoad(Standard_True, Standard_True);
      aNode->UpdateLoad(Standard_False, Standard_True);
      if (!theToForceUpdate)
        continue;
      aNode->SetEnabled(aNode->IsExportSupported() || aNode->IsImportSupported());
    }
  }
}

//=================================================================================================

void DataExchangeWrapper::sort(const Handle(ConfigurationContext)& theResource)
{
  const AsciiString1 aScope(THE_CONFIGURATION_SCOPE() + '.' + "priority");
  for (DE_ConfigurationFormatMap::Iterator aFormatIter(myConfiguration); aFormatIter.More();
       aFormatIter.Next())
  {
    TColStd_ListOfAsciiString aVendorPriority;
    if (!theResource->GetStringSeq(aFormatIter.Key1(), aVendorPriority, aScope))
    {
      continue;
    }
    ChangePriority(aFormatIter.Key1(), aVendorPriority, Standard_True);
  }
}
