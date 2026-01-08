// Copyright (c) 2015 OPEN CASCADE SAS
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

#include <StdLPersistent_HString.hxx>
#include <StdObjMgt_ReadData.hxx>
#include <StdObjMgt_WriteData.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>

//=======================================================================
// function : Read
// purpose  : Read persistent data from a file
//=======================================================================
template <class StringClass, typename CharType>
void HString::instance<StringClass, CharType>::Read(ReadData& theReadData)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Standard_Integer aSize;
  theReadData >> aSize;
  myValue = new StringClass(aSize, 0);

  for (Standard_Integer i = 1; i <= aSize; i++)
  {
    CharType aChar;
    theReadData >> aChar;
    myValue->SetValue(i, aChar);
  }
}

//=======================================================================
// function : Write
// purpose  : Write persistent data to a file
//=======================================================================
template <class StringClass, typename CharType>
void HString::instance<StringClass, CharType>::Write(
  WriteData& theWriteData) const
{
  WriteData::ObjectSentry aSentry(theWriteData);

  Standard_Integer aSize = myValue->Length();
  theWriteData << aSize;

  for (Standard_Integer i = 1; i <= aSize; i++)
  {
    CharType aChar(0);
    theWriteData << aChar;
  }
}

//=======================================================================
// function : Label
// purpose  : Get/create a label defined by referenced string
//=======================================================================
template <class StringClass, typename CharType>
DataLabel HString::instance<StringClass, CharType>::Label(
  const Handle(TDF_Data)& theDF) const
{
  DataLabel aLabel;

  if (!myValue.IsNull())
    Tool3::Label(theDF, myValue->String(), aLabel, Standard_True);

  return aLabel;
}

//=======================================================================
// function : AsciiString2
// purpose  : Get referenced ASCII string
//=======================================================================
Handle(TCollection_HAsciiString) HString::Ascii1::AsciiString2() const
{
  return myValue;
}

//=======================================================================
// function : ExtString
// purpose  : Get referenced extended string
//=======================================================================
Handle(TCollection_HExtendedString) HString::Extended1::ExtString() const
{
  return myValue;
}

template class HString::instance<TCollection_HAsciiString, Standard_Character>;

template class HString::instance<TCollection_HExtendedString, Standard_ExtCharacter>;
