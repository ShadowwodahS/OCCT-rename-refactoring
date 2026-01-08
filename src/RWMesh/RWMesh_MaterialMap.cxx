// Copyright (c) 2017-2019 OPEN CASCADE SAS
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

#include <RWMesh_MaterialMap.hxx>

#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OSD_Directory.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Label.hxx>

IMPLEMENT_STANDARD_RTTIEXT(RWMesh_MaterialMap, RefObject)

//=================================================================================================

RWMesh_MaterialMap::RWMesh_MaterialMap(const AsciiString1& theFile)
    : myFileName(theFile),
      myKeyPrefix("mat_"),
      myNbMaterials(0),
      myIsFailed(false),
      myMatNameAsKey(true)
{
  AsciiString1 aFileName, aFileExt;
  SystemPath::FolderAndFileFromPath(theFile, myFolder, aFileName);
  SystemPath::FileNameAndExtension(aFileName, myShortFileNameBase, aFileExt);
  if (myFolder.IsEmpty())
  {
    myFolder = ".";
  }
}

//=================================================================================================

RWMesh_MaterialMap::~RWMesh_MaterialMap()
{
  //
}

//=================================================================================================

AsciiString1 RWMesh_MaterialMap::AddMaterial(const XCAFPrs_Style& theStyle)
{
  if (myStyles.IsBound1(theStyle))
  {
    return myStyles.Find1(theStyle);
  }

  AsciiString1 aMatKey, aMatName, aMatNameSuffix;
  int                     aCounter    = 0;
  int*                    aCounterPtr = &myNbMaterials;
  if (myMatNameAsKey)
  {
    if (!theStyle.Material().IsNull() && !theStyle.Material()->IsEmpty())
    {
      aCounterPtr = &aCounter;
      Handle(NameAttribute) aNodeName;
      if (!theStyle.Material()->Label().IsNull()
          && theStyle.Material()->Label().FindAttribute(NameAttribute::GetID(), aNodeName))
      {
        aMatName = aNodeName->Get();
      }
      else
      {
        aMatName = "mat";
      }
      aMatNameSuffix = aMatName;
    }
    else
    {
      ++myNbMaterials;
      aMatNameSuffix = myKeyPrefix;
      aMatName       = aMatNameSuffix + myNbMaterials;
    }
    aMatKey = aMatName;
  }
  else
  {
    aMatKey        = myNbMaterials++; // starts from 0
    aMatNameSuffix = myKeyPrefix;
    aMatName       = aMatNameSuffix + aMatKey;
  }

  for (;; ++(*aCounterPtr))
  {
    if (myStyles.IsBound2(aMatKey))
    {
      if (myMatNameAsKey)
      {
        aMatName = aMatNameSuffix + (*aCounterPtr);
        aMatKey  = aMatName;
      }
      else
      {
        aMatKey  = *aCounterPtr;
        aMatName = aMatNameSuffix + aMatKey;
      }
      continue;
    }
    break;
  }

  myStyles.Bind(theStyle, aMatKey);
  DefineMaterial(theStyle, aMatKey, aMatName);
  return aMatKey;
}

//=================================================================================================

bool RWMesh_MaterialMap::copyFileTo(const AsciiString1& theFileSrc,
                                    const AsciiString1& theFileDst)
{
  if (theFileSrc.IsEmpty() || theFileDst.IsEmpty())
  {
    return false;
  }
  else if (theFileSrc == theFileDst)
  {
    return true;
  }

  try
  {
    SystemPath aSrcPath(theFileSrc);
    SystemPath aDstPath(theFileDst);
    SystemFile aFileSrc(aSrcPath);
    if (!aFileSrc.Exists())
    {
      Message1::SendFail(AsciiString1("Failed to copy file - source file '") + theFileSrc
                        + "' does not exist");
      return false;
    }
    aFileSrc.Copy(aDstPath);
    return !aFileSrc.Failed();
  }
  catch (ExceptionBase const& theException)
  {
    Message1::SendFail(AsciiString1("Failed to copy file\n")
                      + theException.GetMessageString());
    return false;
  }
}

//=================================================================================================

bool RWMesh_MaterialMap::CopyTexture(AsciiString1&       theResTexture,
                                     const Handle(Image_Texture)&   theTexture,
                                     const AsciiString1& theKey)
{
  CreateTextureFolder();

  AsciiString1 aTexFileName;
  AsciiString1 aTextureSrc = theTexture->FilePath();
  if (!aTextureSrc.IsEmpty() && theTexture->FileOffset() <= 0 && theTexture->FileLength() <= 0)
  {
    AsciiString1 aSrcTexFolder;
    SystemPath::FolderAndFileFromPath(aTextureSrc, aSrcTexFolder, aTexFileName);
    const AsciiString1 aResTexFile = myTexFolder + aTexFileName;
    theResTexture                             = myTexFolderShort + aTexFileName;
    return copyFileTo(aTextureSrc, aResTexFile);
  }

  AsciiString1 anExt = theTexture->ProbeImageFileFormat();
  if (anExt.IsEmpty())
  {
    anExt = "bin";
  }
  aTexFileName = theKey + "." + anExt;

  const AsciiString1 aResTexFile = myTexFolder + aTexFileName;
  theResTexture                             = myTexFolderShort + aTexFileName;
  return theTexture->WriteImage(aResTexFile);
}

//=================================================================================================

bool RWMesh_MaterialMap::CreateTextureFolder()
{
  if (!myTexFolder.IsEmpty())
  {
    return true;
  }

  myTexFolderShort = myShortFileNameBase + "_textures/";
  myTexFolder      = myFolder + "/" + myTexFolderShort;
  SystemPath      aTexFolderPath(myTexFolder);
  OSD_Directory aTexDir(aTexFolderPath);
  if (aTexDir.Exists())
  {
    return true;
  }

  SystemPath      aResFolderPath(myFolder);
  OSD_Directory aResDir(aResFolderPath);
  if (!aResDir.Exists())
  {
    Message1::SendFail() << "Failed to create textures folder '" << myFolder << "'";
    return false;
  }
  const Protection1 aParentProt = aResDir.Protection();
  Protection1       aProt       = aParentProt;
  if (aProt.User() == OSD_None)
  {
    aProt.SetUser(OSD_RWXD);
  }
  if (aProt.System() == OSD_None)
  {
    aProt.SetSystem(OSD_RWXD);
  }

  aTexDir.Build(aProt);
  if (aTexDir.Failed())
  {
    // fallback to the same folder as output model file
    Message1::SendFail() << "Failed to create textures folder '" << myTexFolder << "'";
    myTexFolder = myFolder;
    myTexFolderShort.Clear();
    return true;
  }
  return true;
}
