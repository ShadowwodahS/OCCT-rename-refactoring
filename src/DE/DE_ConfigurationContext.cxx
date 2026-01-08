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

#include <DE_ConfigurationContext.hxx>

#include <Message.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <OSD_StreamBuffer.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_HAsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ConfigurationContext, RefObject)

enum DE_ConfigurationContext_KindOfLine
{
  DE_ConfigurationContext_KindOfLine_End,
  DE_ConfigurationContext_KindOfLine_Empty,
  DE_ConfigurationContext_KindOfLine_Comment,
  DE_ConfigurationContext_KindOfLine_Resource,
  DE_ConfigurationContext_KindOfLine_Error
};

namespace
{
//=================================================================================================

static Standard_Boolean GetLine(SystemFile& theFile, AsciiString1& theLine)
{
  AsciiString1 aBuffer;
  Standard_Integer        aBufSize = 10;
  Standard_Integer        aLen;
  theLine.Clear();
  do
  {
    theFile.ReadLine(aBuffer, aBufSize, aLen);
    theLine += aBuffer;
    if (theFile.IsAtEnd())
    {
      if (!theLine.Length())
      {
        return Standard_False;
      }
      else
      {
        theLine += "\n";
      }
    }
  } while (theLine.Value(theLine.Length()) != '\n');
  return Standard_True;
}

//=================================================================================================

static DE_ConfigurationContext_KindOfLine WhatKindOfLine(const AsciiString1& theLine,
                                                         AsciiString1&       theToken1,
                                                         AsciiString1&       theToken2)
{
  static const AsciiString1 aWhiteSpace = " \t\r\n";
  Standard_Integer                     aPos1 = 0, aPos2 = 0, aPos = 0;
  AsciiString1              aLine(theLine);
  aLine.LeftAdjust();
  aLine.RightAdjust();
  if (!aLine.EndsWith(':')
      && (!aLine.EndsWith(' ') || !aLine.EndsWith('\t') || !aLine.EndsWith('\n')))
  {
    aLine.InsertAfter(aLine.Length(), " ");
  }

  if (aLine.Value(1) == '!')
  {
    return DE_ConfigurationContext_KindOfLine_Comment;
  }
  aPos1 = aLine.FirstLocationNotInSet(aWhiteSpace, 1, aLine.Length());
  if (aLine.Value(aPos1) == '\n')
  {
    return DE_ConfigurationContext_KindOfLine_Empty;
  }

  aPos2 = aLine.Location(1, ':', aPos1, aLine.Length());
  if (aPos2 == 0 || aPos1 == aPos2)
  {
    return DE_ConfigurationContext_KindOfLine_Error;
  }

  for (aPos = aPos2 - 1; aLine.Value(aPos) == '\t' || aLine.Value(aPos) == ' '; aPos--)
    ;

  theToken1 = aLine.SubString(aPos1, aPos);
  if (aPos2 != aLine.Length())
  {
    aPos2++;
  }
  aPos = aLine.FirstLocationNotInSet(aWhiteSpace, aPos2, aLine.Length());
  if (aPos != 0)
  {
    if (aLine.Value(aPos) == '\\')
    {
      switch (aLine.Value(aPos + 1))
      {
        case '\\':
        case ' ':
        case '\t':
          aPos++;
          break;
      }
    }
  }
  if (aPos == aLine.Length() || aPos == 0)
  {
    theToken2.Clear();
  }
  else
  {
    aLine.Remove(1, aPos - 1);
    aLine.Remove(aLine.Length());
    theToken2 = aLine;
  }
  return DE_ConfigurationContext_KindOfLine_Resource;
}

//=================================================================================================

static AsciiString1 MakeName(const AsciiString1& theScope,
                                        const AsciiString1& theParam)
{
  AsciiString1 aStr(theScope);
  if (!aStr.IsEmpty())
  {
    aStr += '.';
  }
  aStr += theParam;
  return aStr;
}
} // namespace

//=================================================================================================

ConfigurationContext::ConfigurationContext() {}

//=================================================================================================

Standard_Boolean ConfigurationContext::Load(const AsciiString1& theConfiguration)
{
  SystemPath aPath = theConfiguration;
  SystemFile aFile(aPath);
  if (!aFile.Exists())
  {
    if (!LoadStr(theConfiguration))
    {
      return false;
    }
  }
  else
  {
    if (!LoadFile(theConfiguration))
    {
      return false;
    }
  }
  return true;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::LoadFile(const AsciiString1& theFile)
{
  myResource.Clear();
  SystemPath                aPath(theFile);
  SystemFile                aFile    = aPath;
  AsciiString1 FileName = aPath.Name();
  aFile.Open(OSD_ReadOnly, Protection1());
  if (aFile.Failed())
  {
    Message1::SendFail("Error: DE Context loading is stopped. Can't open the file");
    return Standard_True;
  }
  AsciiString1 aLine;
  while (GetLine(aFile, aLine))
  {
    if (!load(aLine))
    {
      Message1::SendFail() << "Error: DE Context loading is stopped. Syntax error: " << aLine;
      return Standard_False;
    }
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::LoadStr(const AsciiString1& theResource)
{
  myResource.Clear();
  AsciiString1 aLine   = "";
  const Standard_Integer  aLength = theResource.Length();
  for (Standard_Integer anInd = 1; anInd <= aLength; anInd++)
  {
    const Standard_Character aChar = theResource.Value(anInd);
    if (aChar != '\n')
      aLine += aChar;
    if ((aChar == '\n' || anInd == aLength) && !aLine.IsEmpty())
    {
      if (!load(aLine))
      {
        Message1::SendFail() << "Error: DE Context loading is stopped. Syntax error: " << aLine;
        return Standard_False;
      }
      aLine.Clear();
    }
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::IsParamSet(const AsciiString1& theParam,
                                                     const AsciiString1& theScope) const
{
  AsciiString1 aResource(MakeName(theScope, theParam));
  return myResource.IsBound(aResource);
}

//=================================================================================================

Standard_Real ConfigurationContext::RealVal(const AsciiString1& theParam,
                                               const Standard_Real            theDefValue,
                                               const AsciiString1& theScope) const
{
  Standard_Real aVal = 0.;
  return GetReal(theParam, aVal, theScope) ? aVal : theDefValue;
}

//=================================================================================================

Standard_Integer ConfigurationContext::IntegerVal(const AsciiString1& theParam,
                                                     const Standard_Integer         theDefValue,
                                                     const AsciiString1& theScope) const
{
  Standard_Integer aVal = 0;
  return GetInteger(theParam, aVal, theScope) ? aVal : theDefValue;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::BooleanVal(const AsciiString1& theParam,
                                                     const Standard_Boolean         theDefValue,
                                                     const AsciiString1& theScope) const
{
  Standard_Boolean aVal = Standard_False;
  return GetBoolean(theParam, aVal, theScope) ? aVal : theDefValue;
}

//=================================================================================================

AsciiString1 ConfigurationContext::StringVal(
  const AsciiString1& theParam,
  const AsciiString1& theDefValue,
  const AsciiString1& theScope) const
{
  AsciiString1 aVal = "";
  return GetString(theParam, aVal, theScope) ? aVal : theDefValue;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::GetReal(const AsciiString1& theParam,
                                                  Standard_Real&                 theValue,
                                                  const AsciiString1& theScope) const
{
  AsciiString1 aStr;
  if (!GetString(theParam, aStr, theScope))
  {
    return Standard_False;
  }
  if (aStr.IsRealValue())
  {
    theValue = aStr.RealValue();
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::GetInteger(const AsciiString1& theParam,
                                                     Standard_Integer&              theValue,
                                                     const AsciiString1& theScope) const
{
  AsciiString1 aStr;
  if (!GetString(theParam, aStr, theScope))
  {
    return Standard_False;
  }
  if (aStr.IsIntegerValue())
  {
    theValue = aStr.IntegerValue();
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::GetBoolean(const AsciiString1& theParam,
                                                     Standard_Boolean&              theValue,
                                                     const AsciiString1& theScope) const
{
  AsciiString1 aStr;
  if (!GetString(theParam, aStr, theScope))
  {
    return Standard_False;
  }
  if (aStr.IsIntegerValue())
  {
    theValue = aStr.IntegerValue() != 0;
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::GetString(const AsciiString1& theParam,
                                                    AsciiString1&       theStr,
                                                    const AsciiString1& theScope) const
{
  AsciiString1 aResource = MakeName(theScope, theParam);
  return myResource.Find(aResource, theStr);
}

//=================================================================================================

Standard_Boolean ConfigurationContext::GetStringSeq(
  const AsciiString1& theParam,
  TColStd_ListOfAsciiString&     theValue,
  const AsciiString1& theScope) const
{
  AsciiString1 aStr;
  if (!GetString(theParam, aStr, theScope))
  {
    return Standard_False;
  }
  theValue.Clear();
  AsciiString1 anElem;
  const Standard_Integer  aLength = aStr.Length();
  for (Standard_Integer anInd = 1; anInd <= aLength; anInd++)
  {
    const Standard_Character aChar = aStr.Value(anInd);
    anElem += aChar;
    if ((aChar == ' ' || anInd == aLength) && !anElem.IsEmpty())
    {
      anElem.RightAdjust();
      anElem.LeftAdjust();
      theValue.Append(anElem);
      anElem.Clear();
    }
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean ConfigurationContext::load(const AsciiString1& theResourceLine)
{
  if (theResourceLine.IsEmpty())
  {
    return Standard_False;
  }
  AsciiString1            aToken1, aToken2;
  DE_ConfigurationContext_KindOfLine aKind = WhatKindOfLine(theResourceLine, aToken1, aToken2);
  switch (aKind)
  {
    case DE_ConfigurationContext_KindOfLine_End:
    case DE_ConfigurationContext_KindOfLine_Comment:
    case DE_ConfigurationContext_KindOfLine_Empty:
      break;
    case DE_ConfigurationContext_KindOfLine_Resource:
      myResource.Bind(aToken1, aToken2);
      break;
    case DE_ConfigurationContext_KindOfLine_Error:
      break;
  }
  return Standard_True;
}
