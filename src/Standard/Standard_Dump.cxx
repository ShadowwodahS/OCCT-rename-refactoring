// Copyright (c) 2019 OPEN CASCADE SAS
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

#include <Standard_Dump.hxx>

#include <stdarg.h>

//=================================================================================================

void DumpTool::AddValuesSeparator(Standard_OStream& theOStream)
{
  Standard_SStream aStream;
  aStream << theOStream.rdbuf();
  AsciiString1 aStreamStr = DumpTool::Text(aStream);
  if (!aStreamStr.IsEmpty() && !aStreamStr.EndsWith("{") && !aStreamStr.EndsWith(", "))
    theOStream << ", ";
}

//=================================================================================================

void DumpTool::DumpKeyToClass(Standard_OStream&              theOStream,
                                   const AsciiString1& theKey,
                                   const AsciiString1& theField)
{
  AddValuesSeparator(theOStream);
  theOStream << "\"" << theKey << "\": {" << theField << "}";
}

//=================================================================================================

void DumpTool::DumpCharacterValues(Standard_OStream& theOStream, int theCount, ...)
{
  va_list vl;
  va_start(vl, theCount);
  for (int i = 0; i < theCount; ++i)
  {
    if (i > 0)
      theOStream << ", ";
    theOStream << "\"" << va_arg(vl, char*) << "\"";
  }
  va_end(vl);
}

//=================================================================================================

void DumpTool::DumpRealValues(Standard_OStream& theOStream, int theCount, ...)
{
  va_list vl;
  va_start(vl, theCount);
  for (int i = 0; i < theCount; ++i)
  {
    if (i > 0)
      theOStream << ", ";
    theOStream << va_arg(vl, Standard_Real);
  }
  va_end(vl);
}

//=================================================================================================

Standard_Boolean DumpTool::ProcessStreamName(const AsciiString1& theStreamStr,
                                                  const AsciiString1& theName,
                                                  Standard_Integer&              theStreamPos)
{
  if (theStreamStr.IsEmpty())
    return Standard_False;

  if (theStreamStr.Length() < theStreamPos)
    return Standard_False;

  AsciiString1 aSubText = theStreamStr.SubString(theStreamPos, theStreamStr.Length());
  if (aSubText.StartsWith(JsonKeyToString(Standard_JsonKey_SeparatorValueToValue)))
  {
    theStreamPos += JsonKeyLength(Standard_JsonKey_SeparatorValueToValue);
    aSubText = theStreamStr.SubString(theStreamPos, theStreamStr.Length());
  }
  AsciiString1 aKeyName =
    AsciiString1(JsonKeyToString(Standard_JsonKey_Quote)) + theName
    + AsciiString1(JsonKeyToString(Standard_JsonKey_Quote))
    + JsonKeyToString(Standard_JsonKey_SeparatorKeyToValue);
  Standard_Boolean aResult = aSubText.StartsWith(aKeyName);
  if (aResult)
    theStreamPos += aKeyName.Length();

  return aResult;
}

//=================================================================================================

Standard_Boolean DumpTool::ProcessFieldName(const AsciiString1& theStreamStr,
                                                 const AsciiString1& theName,
                                                 Standard_Integer&              theStreamPos)
{
  if (theStreamStr.IsEmpty())
    return Standard_False;

  AsciiString1 aSubText = theStreamStr.SubString(theStreamPos, theStreamStr.Length());
  if (aSubText.StartsWith(JsonKeyToString(Standard_JsonKey_SeparatorValueToValue)))
  {
    theStreamPos += JsonKeyLength(Standard_JsonKey_SeparatorValueToValue);
    aSubText = theStreamStr.SubString(theStreamPos, theStreamStr.Length());
  }

  AsciiString1 aName = DumpTool::DumpFieldToName(theName.ToCString());
  AsciiString1 aKeyName =
    AsciiString1(JsonKeyToString(Standard_JsonKey_Quote)) + aName
    + AsciiString1(JsonKeyToString(Standard_JsonKey_Quote))
    + JsonKeyToString(Standard_JsonKey_SeparatorKeyToValue);

  Standard_Boolean aResult = aSubText.StartsWith(aKeyName);
  if (aResult)
    theStreamPos += aKeyName.Length();

  return aResult;
}

//=================================================================================================

Standard_Boolean DumpTool::InitRealValues(const AsciiString1& theStreamStr,
                                               Standard_Integer&              theStreamPos,
                                               int                            theCount,
                                               ...)
{
  Standard_Integer aStreamPos = theStreamPos + JsonKeyLength(Standard_JsonKey_OpenContainer);

  AsciiString1 aSubText = theStreamStr.SubString(aStreamPos, theStreamStr.Length());

  va_list vl;
  va_start(vl, theCount);
  aStreamPos                 = 1;
  Standard_Integer aClosePos = aSubText.Location(JsonKeyToString(Standard_JsonKey_CloseContainer),
                                                 aStreamPos,
                                                 aSubText.Length());
  for (int i = 0; i < theCount; ++i)
  {
    Standard_Integer aNextPos =
      (i < theCount - 1)
        ? aSubText.Location(JsonKeyToString(Standard_JsonKey_SeparatorValueToValue),
                            aStreamPos,
                            aSubText.Length())
        : aClosePos;

    AsciiString1 aValueText = aSubText.SubString(aStreamPos, aNextPos - 1);

    if (!aValueText.IsRealValue())
    {
      va_end(vl);
      return Standard_False;
    }

    Standard_Real aValue          = aValueText.RealValue();
    *(va_arg(vl, Standard_Real*)) = aValue;

    aStreamPos = aNextPos + JsonKeyLength(Standard_JsonKey_SeparatorValueToValue);
  }
  va_end(vl);
  aClosePos    = theStreamStr.Location(JsonKeyToString(Standard_JsonKey_CloseContainer),
                                    theStreamPos,
                                    theStreamStr.Length());
  theStreamPos = aClosePos + JsonKeyLength(Standard_JsonKey_CloseContainer);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean DumpTool::InitValue(const AsciiString1& theStreamStr,
                                          Standard_Integer&              theStreamPos,
                                          AsciiString1&       theValue)
{
  Standard_Integer aStreamPos = theStreamPos;

  AsciiString1 aSubText = theStreamStr.SubString(aStreamPos, theStreamStr.Length());

  aStreamPos = 1;
  Standard_Integer aNextPos =
    aSubText.Location(JsonKeyToString(Standard_JsonKey_SeparatorValueToValue),
                      aStreamPos,
                      aSubText.Length());
  Standard_JsonKey aNextKey = Standard_JsonKey_SeparatorValueToValue;

  Standard_Integer aCloseChildPos =
    aSubText.Location(JsonKeyToString(Standard_JsonKey_CloseChild), aStreamPos, aSubText.Length());
  Standard_Boolean isUseClosePos =
    (aNextPos > 0 && aCloseChildPos > 0 && aCloseChildPos < aNextPos) || !aNextPos;
  if (isUseClosePos)
  {
    aNextPos = aCloseChildPos;
    aNextKey = Standard_JsonKey_CloseChild;
  }

  theValue     = aNextPos ? aSubText.SubString(aStreamPos, aNextPos - 1) : aSubText;
  theStreamPos = aNextPos ? (theStreamPos + (aNextPos - aStreamPos) + JsonKeyLength(aNextKey))
                          : theStreamStr.Length();
  return Standard_True;
}

//=================================================================================================

AsciiString1 DumpTool::GetPointerInfo(const Handle(RefObject)& thePointer,
                                                      const bool                        isShortInfo)
{
  if (thePointer.IsNull())
    return AsciiString1();

  return GetPointerInfo(thePointer.get(), isShortInfo);
}

//=================================================================================================

AsciiString1 DumpTool::GetPointerInfo(const void* thePointer,
                                                      const bool  isShortInfo)
{
  if (!thePointer)
    return AsciiString1();

  std::ostringstream aPtrStr;
  aPtrStr << thePointer;
  if (!isShortInfo)
    return aPtrStr.str().c_str();

  AsciiString1 anInfoPtr(aPtrStr.str().c_str());
  for (int aSymbolId = 1; aSymbolId < anInfoPtr.Length(); aSymbolId++)
  {
    if (anInfoPtr.Value(aSymbolId) != '0')
    {
      anInfoPtr = anInfoPtr.SubString(aSymbolId, anInfoPtr.Length());
      anInfoPtr.Prepend(GetPointerPrefix());
      return anInfoPtr;
    }
  }
  return aPtrStr.str().c_str();
}

// =======================================================================
// DumpFieldToName
// =======================================================================
AsciiString1 DumpTool::DumpFieldToName(const AsciiString1& theField)
{
  AsciiString1 aName = theField;
  if (theField.StartsWith('&'))
  {
    aName.Remove(1, 1);
  }

  if (aName.Length() > 1 && aName.Value(1) == 'a')
  {
    if (aName.Length() > 2 && aName.Value(2) == 'n')
    {
      aName.Remove(1, 2);
    }
    else
      aName.Remove(1, 1);
  }
  else if (aName.Length() > 2 && ::LowerCase(aName.Value(1)) == 'm' && aName.Value(2) == 'y')
  {
    aName.Remove(1, 2);
  }

  if (aName.EndsWith(".get()"))
  {
    aName = aName.SubString(1, aName.Length() - AsciiString1(".get()").Length());
  }
  else if (aName.EndsWith("()"))
  {
    aName = aName.SubString(1, aName.Length() - AsciiString1("()").Length());
  }
  return aName;
}

// =======================================================================
// Text
// =======================================================================
AsciiString1 DumpTool::Text(const Standard_SStream& theStream)
{
  return AsciiString1(theStream.str().c_str());
}

// =======================================================================
// FormatJson
// =======================================================================
AsciiString1 DumpTool::FormatJson(const Standard_SStream& theStream,
                                                  const Standard_Integer  theIndent)
{
  AsciiString1 aStreamStr = Text(theStream);
  AsciiString1 anIndentStr;
  for (Standard_Integer anIndentId = 0; anIndentId < theIndent; anIndentId++)
    anIndentStr.AssignCat(' ');

  AsciiString1 aText;

  Standard_Integer anIndentCount   = 0;
  Standard_Boolean isMassiveValues = Standard_False;
  for (Standard_Integer anIndex = 1; anIndex <= aStreamStr.Length(); anIndex++)
  {
    Standard_Character aSymbol = aStreamStr.Value(anIndex);
    if (anIndex == 1 && aText.IsEmpty() && aSymbol != '{')
    {
      // append opening brace for json start
      aSymbol = '{';
      anIndex--;
    }
    if (aSymbol == '{')
    {
      anIndentCount++;

      aText += aSymbol;
      aText += '\n';

      for (int anIndent = 0; anIndent < anIndentCount; anIndent++)
      {
        aText += anIndentStr;
      }
    }
    else if (aSymbol == '}')
    {
      anIndentCount--;

      aText += '\n';
      for (int anIndent = 0; anIndent < anIndentCount; anIndent++)
        aText += anIndentStr;
      aText += aSymbol;
    }
    else if (aSymbol == '[')
    {
      isMassiveValues = Standard_True;
      aText += aSymbol;
    }
    else if (aSymbol == ']')
    {
      isMassiveValues = Standard_False;
      aText += aSymbol;
    }
    else if (aSymbol == ',')
    {
      if (!isMassiveValues)
      {
        aText += aSymbol;
        aText += '\n';
        for (int anIndent = 0; anIndent < anIndentCount; anIndent++)
          aText += anIndentStr;
        if (anIndex + 1 < aStreamStr.Length() && aStreamStr.Value(anIndex + 1) == ' ')
          anIndex++; // skip empty value after comma
      }
      else
        aText += aSymbol;
    }
    else if (aSymbol == '\n')
    {
      aText += ""; // json does not support multi-lined values, skip this symbol
    }
    else
      aText += aSymbol;

    if (anIndex == aStreamStr.Length() && aSymbol != '}')
    {
      // append closing brace for json end
      aSymbol = '}';

      anIndentCount--;
      aText += '\n';
      for (int anIndent = 0; anIndent < anIndentCount; anIndent++)
        aText += anIndentStr;
      aText += aSymbol;
    }
  }
  return aText;
}

// =======================================================================
// SplitJson
// =======================================================================
Standard_Boolean DumpTool::SplitJson(
  const AsciiString1&                                           theStreamStr,
  NCollection_IndexedDataMap<AsciiString1, Standard_DumpValue>& theKeyToValues)
{
  Standard_Integer aNextIndex = 1;
  while (aNextIndex < theStreamStr.Length())
  {
    Standard_JsonKey aKey = Standard_JsonKey_None;
    if (!jsonKey(theStreamStr, aNextIndex, aNextIndex, aKey))
      return Standard_False;

    Standard_Boolean aProcessed = Standard_False;
    switch (aKey)
    {
      case Standard_JsonKey_Quote: {
        aProcessed = splitKeyToValue(theStreamStr, aNextIndex, aNextIndex, theKeyToValues);
        break;
      }
      case Standard_JsonKey_OpenChild: {
        Standard_Integer aStartIndex = aNextIndex;
        Standard_Integer aClosePos   = nextClosePosition(theStreamStr,
                                                       aStartIndex,
                                                       Standard_JsonKey_OpenChild,
                                                       Standard_JsonKey_CloseChild);
        if (aClosePos == 0)
          return Standard_False;

        AsciiString1 aSubStreamStr =
          theStreamStr.SubString(aStartIndex + JsonKeyLength(aKey), aNextIndex - 2);
        if (!SplitJson(aSubStreamStr, theKeyToValues))
          return Standard_False;

        aNextIndex = aClosePos + Standard_Integer(JsonKeyLength(Standard_JsonKey_CloseChild));
        break;
      }
      case Standard_JsonKey_SeparatorValueToValue: {
        continue;
      }
      default:
        break;
    }
    if (!aProcessed)
      return Standard_False;
  }
  return Standard_True;
}

// =======================================================================
// HierarchicalValueIndices
// =======================================================================
NCollection_List<Standard_Integer> DumpTool::HierarchicalValueIndices(
  const NCollection_IndexedDataMap<AsciiString1, AsciiString1>& theValues)
{
  NCollection_List<Standard_Integer> anIndices;

  for (Standard_Integer anIndex = 1; anIndex <= theValues.Extent(); anIndex++)
  {
    if (HasChildKey(theValues.FindFromIndex(anIndex)))
      anIndices.Append(anIndex);
  }
  return anIndices;
}

// =======================================================================
// splitKeyToValue
// =======================================================================
Standard_Boolean DumpTool::splitKeyToValue(
  const AsciiString1&                                           theStreamStr,
  Standard_Integer                                                         theStartIndex,
  Standard_Integer&                                                        theNextIndex,
  NCollection_IndexedDataMap<AsciiString1, Standard_DumpValue>& theValues)
{
  // find key value: "key"
  Standard_Integer aStartIndex = theStartIndex;
  Standard_Integer aCloseIndex =
    nextClosePosition(theStreamStr, aStartIndex + 1, Standard_JsonKey_None, Standard_JsonKey_Quote);
  if (aCloseIndex == 0)
    return Standard_False;

  AsciiString1 aSplitKey = theStreamStr.SubString(aStartIndex, aCloseIndex - 1);

  // key to value
  aStartIndex           = aCloseIndex + 1;
  Standard_JsonKey aKey = Standard_JsonKey_None;
  if (!jsonKey(theStreamStr, aStartIndex, aCloseIndex, aKey))
    return Standard_False;

  // find value
  aStartIndex = aCloseIndex;
  aKey        = Standard_JsonKey_None;
  jsonKey(theStreamStr, aStartIndex, aCloseIndex, aKey);
  aStartIndex = aCloseIndex;

  AsciiString1 aSplitValue;
  theNextIndex = -1;
  switch (aKey)
  {
    case Standard_JsonKey_OpenChild: {
      aCloseIndex = nextClosePosition(theStreamStr,
                                      aStartIndex,
                                      Standard_JsonKey_OpenChild,
                                      Standard_JsonKey_CloseChild);
      if (aCloseIndex > aStartIndex)
        aSplitValue = theStreamStr.SubString(aStartIndex, aCloseIndex);
      theNextIndex = aCloseIndex + 1;
      break;
    }
    case Standard_JsonKey_OpenContainer: {
      aCloseIndex = nextClosePosition(theStreamStr,
                                      aStartIndex,
                                      Standard_JsonKey_OpenContainer,
                                      Standard_JsonKey_CloseContainer);
      if (aCloseIndex > aStartIndex)
        aSplitValue = theStreamStr.SubString(aStartIndex, aCloseIndex - 1);
      theNextIndex = aCloseIndex + 1;
      break;
    }
    case Standard_JsonKey_Quote: {
      Standard_JsonKey aKeyTmp = Standard_JsonKey_None;
      if (jsonKey(theStreamStr, aStartIndex, aCloseIndex, aKeyTmp)
          && aKeyTmp == Standard_JsonKey_Quote) // emptyValue
      {
        aSplitValue  = "";
        theNextIndex = aCloseIndex;
      }
      else
      {
        aCloseIndex  = nextClosePosition(theStreamStr,
                                        aStartIndex + 1,
                                        Standard_JsonKey_None,
                                        Standard_JsonKey_Quote);
        aSplitValue  = theStreamStr.SubString(aStartIndex, aCloseIndex - 1);
        theNextIndex = aCloseIndex + 1;
      }
      break;
    }
    case Standard_JsonKey_None: {
      if (aStartIndex == theStreamStr.Length())
      {
        aSplitValue =
          aStartIndex <= aCloseIndex ? theStreamStr.SubString(aStartIndex, aCloseIndex) : "";
        aSplitValue = theStreamStr.SubString(aStartIndex, aCloseIndex);
        aCloseIndex = aStartIndex;
      }
      else
      {
        Standard_Integer aCloseIndex1 = nextClosePosition(theStreamStr,
                                                          aStartIndex,
                                                          Standard_JsonKey_None,
                                                          Standard_JsonKey_CloseChild)
                                        - 1;
        Standard_Integer aCloseIndex2 = nextClosePosition(theStreamStr,
                                                          aStartIndex,
                                                          Standard_JsonKey_None,
                                                          Standard_JsonKey_SeparatorValueToValue)
                                        - 1;
        aCloseIndex = aCloseIndex1 < aCloseIndex2 ? aCloseIndex1 : aCloseIndex2;
        aSplitValue =
          aStartIndex <= aCloseIndex ? theStreamStr.SubString(aStartIndex, aCloseIndex) : "";
      }
      theNextIndex = aCloseIndex + 1;
      break;
    }
    default:
      return Standard_False;
  }

  Standard_DumpValue aValue;
  if (theValues.FindFromKey(aSplitKey, aValue))
  {
    Standard_Integer anIndex = 1;
    // increment key until the new key does not exist in the container
    AsciiString1 anIndexedSuffix =
      AsciiString1("_") + AsciiString1(anIndex);
    while (theValues.FindFromKey(AsciiString1(aSplitKey + anIndexedSuffix), aValue))
    {
      anIndex++;
      anIndexedSuffix = AsciiString1("_") + AsciiString1(anIndex);
    }
    aSplitKey = aSplitKey + anIndexedSuffix;
  }

  theValues.Add(aSplitKey, Standard_DumpValue(aSplitValue, aStartIndex));
  return Standard_True;
}

// =======================================================================
// jsonKey
// =======================================================================
Standard_Boolean DumpTool::jsonKey(const AsciiString1& theStreamStr,
                                        Standard_Integer               theStartIndex,
                                        Standard_Integer&              theNextIndex,
                                        Standard_JsonKey&              theKey)
{
  AsciiString1 aSubStreamStr =
    theStreamStr.SubString(theStartIndex, theStreamStr.Length());
  for (Standard_Integer aKeyId = (Standard_Integer)Standard_JsonKey_OpenChild;
       aKeyId <= Standard_JsonKey_SeparatorValueToValue;
       aKeyId++)
  {
    Standard_JsonKey aKey      = (Standard_JsonKey)aKeyId;
    Standard_CString aKeyToStr = JsonKeyToString(aKey);
    if (!aSubStreamStr.StartsWith(aKeyToStr))
      continue;

    theNextIndex = theStartIndex + Standard_Integer(JsonKeyLength(aKey));
    theKey       = aKey;
    return Standard_True;
  }
  return Standard_False;
}

// =======================================================================
// HasChildKey
// =======================================================================
Standard_Boolean DumpTool::HasChildKey(const AsciiString1& theSourceValue)
{
  return theSourceValue.Search(JsonKeyToString(Standard_JsonKey_SeparatorKeyToValue)) >= 0;
}

// =======================================================================
// JsonKeyToString
// =======================================================================
Standard_CString DumpTool::JsonKeyToString(const Standard_JsonKey theKey)
{
  switch (theKey)
  {
    case Standard_JsonKey_None:
      return "";
    case Standard_JsonKey_OpenChild:
      return "{";
    case Standard_JsonKey_CloseChild:
      return "}";
    case Standard_JsonKey_OpenContainer:
      return "[";
    case Standard_JsonKey_CloseContainer:
      return "]";
    case Standard_JsonKey_Quote:
      return "\"";
    case Standard_JsonKey_SeparatorKeyToValue:
      return ": ";
    case Standard_JsonKey_SeparatorValueToValue:
      return ", ";
  }

  return "";
}

// =======================================================================
// JsonKeyLength
// =======================================================================
Standard_Integer DumpTool::JsonKeyLength(const Standard_JsonKey theKey)
{
  return (Standard_Integer)strlen(JsonKeyToString(theKey));
}

// =======================================================================
// nextClosePosition
// =======================================================================
Standard_Integer DumpTool::nextClosePosition(const AsciiString1& theSourceValue,
                                                  const Standard_Integer         theStartPosition,
                                                  const Standard_JsonKey         theOpenKey,
                                                  const Standard_JsonKey         theCloseKey)
{
  Standard_CString anOpenKey    = JsonKeyToString(theOpenKey);
  Standard_CString aCloseKeyStr = JsonKeyToString(theCloseKey);

  Standard_Integer aStartPos = theStartPosition;
  Standard_Integer aDepthKey = 0;

  while (aStartPos < theSourceValue.Length())
  {
    Standard_Integer anOpenKeyPos =
      theSourceValue.Location(anOpenKey, aStartPos, theSourceValue.Length());
    Standard_Integer aCloseKeyPos =
      theSourceValue.Location(aCloseKeyStr, aStartPos, theSourceValue.Length());
    if (aCloseKeyPos == 0)
      break;

    if (anOpenKeyPos != 0 && anOpenKeyPos <= aCloseKeyPos)
    {
      aDepthKey++;
      aStartPos = anOpenKeyPos + 1;
    }
    else
    {
      if (aDepthKey == 0)
        return aCloseKeyPos;
      else
      {
        aDepthKey--;
        aStartPos = aCloseKeyPos + 1;
      }
    }
  }
  return theSourceValue.Length();
}
