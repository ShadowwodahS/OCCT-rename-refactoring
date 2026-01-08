// Copyright (c) 1998-1999 Matra Datavision
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

#include <Resource_Manager.hxx>

#include <OSD_Directory.hxx>
#include <OSD_Environment.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <Resource_LexicalCompare.hxx>
#include <Resource_NoSuchResource.hxx>
#include <Resource_Unicode.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_Type.hxx>
#include <Standard_TypeMismatch.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TColStd_Array1OfAsciiString.hxx>

#include <algorithm>
#include <errno.h>

IMPLEMENT_STANDARD_RTTIEXT(Resource_Manager, RefObject)

//! Auxiliary enumeration for function WhatKindOfLine().
enum Resource_KindOfLine
{
  Resource_KOL_End,
  Resource_KOL_Empty,
  Resource_KOL_Comment,
  Resource_KOL_Resource,
  Resource_KOL_Error
};

static Resource_KindOfLine WhatKindOfLine(SystemFile&                aFile,
                                          AsciiString1& aToken1,
                                          AsciiString1& aToken2);

static Standard_Integer GetLine(SystemFile& aFile, AsciiString1& aLine);

static Standard_Boolean Debug;

//=================================================================================================

Resource_Manager::Resource_Manager(const AsciiString1& theName,
                                   const AsciiString1& theDefaultsDirectory,
                                   const AsciiString1& theUserDefaultsDirectory,
                                   const Standard_Boolean         theIsVerbose)
    : myName(theName),
      myVerbose(theIsVerbose),
      myInitialized(Standard_False)
{
  if (!theDefaultsDirectory.IsEmpty())
  {
    SystemPath anOSDPath(theDefaultsDirectory);
    if (!anOSDPath.Name().IsEmpty())
    {
      anOSDPath.DownTrek(anOSDPath.Name() + anOSDPath.Extension());
    }
    anOSDPath.SetName(theName);
    anOSDPath.SetExtension("");
    AsciiString1 aPath;
    anOSDPath.SystemName(aPath);
    Load(aPath, myRefMap);
  }
  else if (myVerbose)
  {
    std::cout << "Resource Manager Warning: aDefaultsDirectory is empty." << std::endl;
  }

  if (!theUserDefaultsDirectory.IsEmpty())
  {
    SystemPath anOSDPath(theUserDefaultsDirectory);
    if (!anOSDPath.Name().IsEmpty())
    {
      anOSDPath.DownTrek(anOSDPath.Name() + anOSDPath.Extension());
    }
    anOSDPath.SetName(theName);
    anOSDPath.SetExtension("");
    AsciiString1 aPath;
    anOSDPath.SystemName(aPath);
    Load(aPath, myRefMap);
  }
  else if (myVerbose)
  {
    std::cout << "Resource Manager Warning: anUserDefaultsDirectory is empty." << std::endl;
  }
}

Resource_Manager::Resource_Manager(const Standard_CString aName, const Standard_Boolean Verbose)
    : myName(aName),
      myVerbose(Verbose),
      myInitialized(Standard_False)
{
  OSD_Environment envDebug("ResourceDebug");
  Debug = (!envDebug.Value().IsEmpty());

  AsciiString1 Directory;

  OSD_Environment envVerbose("CSF_ResourceVerbose");
  if (!envVerbose.Value().IsEmpty())
    myVerbose = Standard_True;

  AsciiString1 aPath, aUserPath;
  GetResourcePath(aPath, aName, Standard_False);
  GetResourcePath(aUserPath, aName, Standard_True);

  if (!aPath.IsEmpty())
    Load(aPath, myRefMap);
  else if (myVerbose)
    std::cout << "Resource Manager Warning: Environment variable \"CSF_" << aName
              << "Defaults\" not set." << std::endl;

  if (!aUserPath.IsEmpty())
    Load(aUserPath, myRefMap);
  else if (myVerbose)
    std::cout << "Resource Manager Warning: Environment variable \"CSF_" << aName
              << "UserDefaults\" not set." << std::endl;
}

//=================================================================================================

Resource_Manager::Resource_Manager()
    : myName(""),
      myVerbose(Standard_False),
      myInitialized(Standard_False)
{
}

//=================================================================================================

void Resource_Manager::Load(const AsciiString1&            thePath,
                            Resource_DataMapOfAsciiStringAsciiString& aMap)
{
  Resource_KindOfLine     aKind;
  AsciiString1 Token1, Token2;
  SystemPath                Path(thePath);
  SystemFile                File     = Path;
  AsciiString1 FileName = Path.Name();
  File.Open(OSD_ReadOnly, Protection1());
  if (File.Failed())
  {
    if (myVerbose)
      std::cout << "Resource Manager Warning: Cannot read file \"" << FileName
                << "\". File not found or permission denied." << std::endl;
    return;
  }
  myInitialized               = Standard_True;
  Standard_Integer LineNumber = 1;
  while ((aKind = WhatKindOfLine(File, Token1, Token2)) != Resource_KOL_End)
  {
    switch (aKind)
    {
      case Resource_KOL_End:
      case Resource_KOL_Comment:
      case Resource_KOL_Empty:
        break;
      case Resource_KOL_Resource:
        if (!aMap.Bind(Token1, Token2))
          aMap(Token1) = Token2;
        break;
      case Resource_KOL_Error:
        if (myVerbose)
          std::cout << "Resource Manager: Syntax error at line " << LineNumber
                    << " in file : " << FileName << std::endl;
        break;
    }
    LineNumber++;
  }
  File.Close();
  if (myVerbose)
    std::cout << "Resource Manager: " << ((&aMap == &myUserMap) ? "User" : "Reference1")
              << " file \"" << FileName << "\" loaded" << std::endl;
}

static Resource_KindOfLine WhatKindOfLine(SystemFile&                aFile,
                                          AsciiString1& aToken1,
                                          AsciiString1& aToken2)
{
  AsciiString1 WhiteSpace = " \t";
  Standard_Integer        Pos1, Pos2, Pos;
  AsciiString1 Line;

  if (!GetLine(aFile, Line))
    return Resource_KOL_End;

  if (Line.Value(1) == '!')
    return Resource_KOL_Comment;

  Pos1 = Line.FirstLocationNotInSet(WhiteSpace, 1, Line.Length());
  if (Line.Value(Pos1) == '\n')
    return Resource_KOL_Empty;

  Pos2 = Line.Location(1, ':', Pos1, Line.Length());
  if (!Pos2 || Pos1 == Pos2)
    return Resource_KOL_Error;

  for (Pos = Pos2 - 1; Line.Value(Pos) == '\t' || Line.Value(Pos) == ' '; Pos--)
    ;
  aToken1 = Line.SubString(Pos1, Pos);

  if (Debug)
    std::cout << "Key1 = '" << aToken1 << std::flush;

  Pos = Line.FirstLocationNotInSet(WhiteSpace, Pos2 + 1, Line.Length());
  if (Pos)
  {
    if (Line.Value(Pos) == '\\')
      switch (Line.Value(Pos + 1))
      {
        case '\\':
        case ' ':
        case '\t':
          Pos++;
          break;
      }
  }
  if (Pos == Line.Length())
    aToken2.Clear();
  else
  {
    Line.Remove(1, Pos - 1);
    const Standard_Integer aLineLength = Line.Length();
    if (aLineLength >= 2)
    {
      if (Line.Value(aLineLength - 1) == '\r')
      {
        Line.Remove(aLineLength - 1);
      }
    }
    Line.Remove(Line.Length());
    aToken2 = Line;
  }
  if (Debug)
    std::cout << "'\t Value = '" << aToken2 << "'" << std::endl << std::flush;
  return Resource_KOL_Resource;
}

// Retourne 0 (EOF) ou une ligne toujours terminee par <NL>.

static Standard_Integer GetLine(SystemFile& aFile, AsciiString1& aLine)
{
  AsciiString1 Buffer;
  Standard_Integer        BufSize = 10;
  Standard_Integer        Len;

  aLine.Clear();
  do
  {
    aFile.ReadLine(Buffer, BufSize, Len);
    aLine += Buffer;
    if (aFile.IsAtEnd())
    {
      if (!aLine.Length())
        return 0;
      else
        aLine += "\n";
    }
  } while (aLine.Value(aLine.Length()) != '\n');

  return 1;
}

//=======================================================================
// function : Save
// purpose  : Sort and save the user resources in the user file.
//           Creates the file if it does not exist.
//=======================================================================
Standard_Boolean Resource_Manager::Save() const
{
  AsciiString1 anEnvVar("CSF_");
  anEnvVar += myName;
  anEnvVar += "UserDefaults";

  AsciiString1 dir;
  OSD_Environment         anEnv(anEnvVar);
  dir = anEnv.Value();
  if (dir.IsEmpty())
  {
    if (myVerbose)
      std::cout << "Resource Manager Warning: environment variable \"" << anEnvVar
                << "\" not set.  Cannot save resources." << std::endl;
    return Standard_False;
  }

  AsciiString1 aFilePath(dir);
  SystemPath                anOSDPath(aFilePath);
  OSD_Directory           Dir     = anOSDPath;
  Standard_Boolean        aStatus = Standard_True;
  if (!Dir.Exists())
  {
    {
      try
      {
        OCC_CATCH_SIGNALS
        Dir.Build(Protection1(OSD_RX, OSD_RWXD, OSD_RX, OSD_RX));
      }
      catch (ExceptionBase const&)
      {
        aStatus = Standard_False;
      }
    }
    aStatus = aStatus && !Dir.Failed();
    if (!aStatus)
    {
      if (myVerbose)
        std::cout << "Resource Manager: Error opening or creating directory \"" << aFilePath
                  << "\". Permission denied. Cannot save resources." << std::endl;
      return Standard_False;
    }
  }

  if (!anOSDPath.Name().IsEmpty())
  {
    anOSDPath.DownTrek(anOSDPath.Name() + anOSDPath.Extension());
  }
  anOSDPath.SetName(myName);
  anOSDPath.SetExtension("");
  anOSDPath.SystemName(aFilePath);

  SystemFile       File = anOSDPath;
  Protection1 theProt;
  aStatus = Standard_True;
  {
    try
    {
      OCC_CATCH_SIGNALS
      File.Build(OSD_ReadWrite, theProt);
    }
    catch (ExceptionBase const&)
    {
      aStatus = Standard_False;
    }
  }
  aStatus = aStatus && !File.Failed();
  if (!aStatus)
  {
    if (myVerbose)
      std::cout << "Resource Manager: Error opening or creating file \"" << aFilePath
                << "\". Permission denied. Cannot save resources." << std::endl;
    return Standard_False;
  }

  const Standard_Integer NbKey = myUserMap.Extent();
  if (NbKey)
  {
    TColStd_Array1OfAsciiString                               KeyArray(1, NbKey);
    Resource_DataMapIteratorOfDataMapOfAsciiStringAsciiString Iter(myUserMap);

    Standard_Integer Index;
    for (Index = 1; Iter.More(); Iter.Next())
      KeyArray(Index++) = Iter.Key1();

    std::sort(KeyArray.begin(), KeyArray.end());

    AsciiString1 Line, Value;
    for (Index = 1; Index <= NbKey; Index++)
    {
      Value = myUserMap(KeyArray(Index));
      if (!Value.IsEmpty())
        switch (Value.Value(1))
        {
          case '\\':
          case ' ':
          case '\t':
            Value.Insert(1, '\\');
            break;
        }
      Line = KeyArray(Index) + ":\t" + Value + "\n";

      if (Debug)
        std::cout << "Line = '" << Line << "'" << std::endl;

      File.Write(Line, Line.Length());
    }
    if (myVerbose)
      std::cout << "Resource Manager: Resources saved in file " << aFilePath << std::endl;
  }
  File.Close();
  return Standard_True;
}

//=======================================================================
// function : Integer1
// purpose  : Gets the value of an integer resource
//=======================================================================

Standard_Integer Resource_Manager::Integer1(const Standard_CString aResourceName) const
{
  AsciiString1 Result = Value(aResourceName);
  if (!Result.IsIntegerValue())
  {
    AsciiString1 n("Value of resource `");
    n += aResourceName;
    n += "` is not an integer";
    throw Standard_TypeMismatch(n.ToCString());
  }
  return Result.IntegerValue();
}

//=======================================================================
// function : Real
// purpose  : Gets the value of a real resource
//=======================================================================

Standard_Real Resource_Manager::Real(const Standard_CString aResourceName) const
{
  AsciiString1 Result = Value(aResourceName);
  if (!Result.IsRealValue())
  {
    AsciiString1 n("Value of resource `");
    n += aResourceName;
    n += "` is not a real";
    throw Standard_TypeMismatch(n.ToCString());
  }
  return Result.RealValue();
}

//=======================================================================
// function : Value
// purpose  : Gets the value of a CString resource
//=======================================================================

Standard_CString Resource_Manager::Value(const Standard_CString aResource) const
{
  AsciiString1 Resource(aResource);
  if (myUserMap.IsBound(Resource))
    return myUserMap(Resource).ToCString();
  if (myRefMap.IsBound(Resource))
    return myRefMap(Resource).ToCString();
  throw Resource_NoSuchResource(aResource);
}

//=======================================================================
// function : ExtValue
// purpose  : Gets the value of a ExtString resource
//=======================================================================

Standard_ExtString Resource_Manager::ExtValue(const Standard_CString aResource)
{
  AsciiString1 Resource(aResource);
  if (myExtStrMap.IsBound(Resource))
    return myExtStrMap(Resource).ToExtString();

  AsciiString1    Result = Value(aResource);
  UtfString ExtResult;

  UnicodeTools::ConvertFormatToUnicode(Result.ToCString(), ExtResult);

  myExtStrMap.Bind(Resource, ExtResult);
  return myExtStrMap(Resource).ToExtString();
}

//=======================================================================
// function : SetResource
// purpose  : Sets the new value of an integer resource.
//           If the resource does not exist, it is created.
//=======================================================================
void Resource_Manager::SetResource(const Standard_CString aResourceName,
                                   const Standard_Integer aValue)
{
  SetResource(aResourceName, AsciiString1(aValue).ToCString());
}

//=======================================================================
// function : SetResource
// purpose  : Sets the new value of a real resource.
//           If the resource does not exist, it is created.
//=======================================================================
void Resource_Manager::SetResource(const Standard_CString aResourceName, const Standard_Real aValue)
{
  SetResource(aResourceName, AsciiString1(aValue).ToCString());
}

//=======================================================================
// function : SetResource
// purpose  : Sets the new value of ExtString resource.
//           If the resource does not exist, it is created.
//=======================================================================
void Resource_Manager::SetResource(const Standard_CString   aResource,
                                   const Standard_ExtString aValue)
{
  Standard_PCharacter        pStr;
  AsciiString1    Resource = aResource;
  UtfString ExtValue = aValue;
  AsciiString1    FormatStr(ExtValue.Length() * 3 + 10, ' ');

  if (!myExtStrMap.Bind(Resource, ExtValue))
  {
    myExtStrMap(Resource) = ExtValue;
  }
  //
  pStr = (Standard_PCharacter)FormatStr.ToCString();
  //
  UnicodeTools::ConvertUnicodeToFormat(ExtValue,
                                           pStr, // FormatStr.ToCString(),
                                           FormatStr.Length());
  SetResource(aResource, FormatStr.ToCString());
}

//=======================================================================
// function : SetResource
// purpose  : Sets the new value of an enum resource.
//           If the resource does not exist, it is created.
//=======================================================================
void Resource_Manager::SetResource(const Standard_CString aResource, const Standard_CString aValue)
{
  AsciiString1 Resource = aResource;
  AsciiString1 Value    = aValue;
  if (!myUserMap.Bind(Resource, Value))
    myUserMap(Resource) = Value;
}

//=======================================================================
// function : Find
// purpose  : Tells if a resource exits.
//=======================================================================
Standard_Boolean Resource_Manager::Find(const Standard_CString aResource) const
{
  AsciiString1 Resource(aResource);
  if (myUserMap.IsBound(Resource) || myRefMap.IsBound(Resource))
    return Standard_True;
  return Standard_False;
}

//=================================================================================================

Standard_Boolean Resource_Manager::Find(const AsciiString1& theResource,
                                        AsciiString1&       theValue) const
{
  return myUserMap.Find(theResource, theValue) || myRefMap.Find(theResource, theValue);
}

//=================================================================================================

void Resource_Manager::GetResourcePath(AsciiString1& aPath,
                                       const Standard_CString   aName,
                                       const Standard_Boolean   isUserDefaults)
{
  aPath.Clear();

  AsciiString1 anEnvVar("CSF_");
  anEnvVar += aName;
  anEnvVar += isUserDefaults ? "UserDefaults" : "Defaults";

  AsciiString1 dir;
  OSD_Environment         anEnv(anEnvVar);
  dir = anEnv.Value();
  if (dir.IsEmpty())
    return;

  AsciiString1 aResPath(dir);

  SystemPath anOSDPath(aResPath);

  if (!anOSDPath.Name().IsEmpty())
  {
    anOSDPath.DownTrek(anOSDPath.Name() + anOSDPath.Extension());
  }
  anOSDPath.SetName(aName);
  anOSDPath.SetExtension("");

  anOSDPath.SystemName(aPath);
}

//=================================================================================================

Resource_DataMapOfAsciiStringAsciiString& Resource_Manager::GetMap(Standard_Boolean theRefMap)
{
  return theRefMap ? myRefMap : myUserMap;
}
