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

#include <DE_ConfigurationNode.hxx>

#include <DE_ConfigurationContext.hxx>
#include <DE_Provider.hxx>
#include <DE_Wrapper.hxx>
#include <Message.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ConfigurationNode, RefObject)

//=================================================================================================

ConfigurationNode::ConfigurationNode()
    : myIsEnabled(Standard_True)
{
}

//=================================================================================================

ConfigurationNode::ConfigurationNode(const Handle(ConfigurationNode)& theConfigurationNode)
{
  GlobalParameters = theConfigurationNode->GlobalParameters;
  myIsEnabled      = theConfigurationNode->IsEnabled();
}

//=================================================================================================

bool ConfigurationNode::Load(const AsciiString1& theResourcePath)
{
  Handle(ConfigurationContext) aResource = new ConfigurationContext();
  aResource->LoadFile(theResourcePath);
  return Load(aResource);
}

//=================================================================================================

bool ConfigurationNode::Save(const AsciiString1& theResourcePath) const
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
      Message1::SendFail() << "Error: Configuration writing process was stopped. Can't build file.";
      return false;
    }
  }
  if (aFile.Failed())
  {
    Message1::SendFail() << "Error: Configuration writing process was stopped. Can't build file.";
    return false;
  }
  AsciiString1 aResConfiguration = Save();
  aFile.Write(aResConfiguration, aResConfiguration.Length());
  aFile.Close();
  return true;
}

//=================================================================================================

bool ConfigurationNode::UpdateLoad(const Standard_Boolean theToImport,
                                      const Standard_Boolean theToKeep)
{
  (void)theToImport;
  (void)theToKeep;
  return true;
}

//=================================================================================================

bool ConfigurationNode::IsImportSupported() const
{
  return false;
}

//=================================================================================================

bool ConfigurationNode::IsExportSupported() const
{
  return false;
}

//=================================================================================================

bool ConfigurationNode::CheckExtension(const AsciiString1& theExtension) const
{
  AsciiString1 anExtension(theExtension);
  if (anExtension.IsEmpty())
  {
    return false;
  }
  if (anExtension.Value(1) == '.')
  {
    anExtension.Remove(1);
  }
  const TColStd_ListOfAsciiString& anExtensions = GetExtensions();
  for (TColStd_ListOfAsciiString::Iterator anIter(anExtensions); anIter.More(); anIter.Next())
  {
    if (AsciiString1::IsSameString(anIter.Value(), anExtension, Standard_False))
    {
      return true;
    }
  }
  return false;
}

//=================================================================================================

bool ConfigurationNode::CheckContent(const Handle(NCollection_Buffer)& theBuffer) const
{
  (void)theBuffer;
  return false;
}
