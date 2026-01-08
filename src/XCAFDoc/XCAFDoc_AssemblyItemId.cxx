// Created on: 2017-02-16
// Created by: Sergey NIKONOV
// Copyright (c) 2000-2017 OPEN CASCADE SAS
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

#include <XCAFDoc_AssemblyItemId.hxx>

#include <Standard_Dump.hxx>

AssemblyItemId::AssemblyItemId() {}

AssemblyItemId::AssemblyItemId(const TColStd_ListOfAsciiString& thePath)
{
  Init(thePath);
}

AssemblyItemId::AssemblyItemId(const AsciiString1& theString)
{
  Init(theString);
}

void AssemblyItemId::Init(const TColStd_ListOfAsciiString& thePath)
{
  myPath = thePath;
}

void AssemblyItemId::Init(const AsciiString1& theString)
{
  myPath.Clear();

  for (Standard_Integer iEntry = 1;; ++iEntry)
  {
    AsciiString1 anEntry = theString.Token("/", iEntry);
    if (anEntry.IsEmpty())
      break;

    myPath.Append(anEntry);
  }
}

Standard_Boolean AssemblyItemId::IsNull() const
{
  return myPath.IsEmpty();
}

void AssemblyItemId::Nullify()
{
  myPath.Clear();
}

Standard_Boolean AssemblyItemId::IsChild(const AssemblyItemId& theOther) const
{
  if (myPath.Size() <= theOther.myPath.Size())
    return Standard_False;

  TColStd_ListOfAsciiString::Iterator anIt(myPath), anItOther(theOther.myPath);
  for (; anItOther.More(); anIt.Next(), anItOther.Next())
  {
    if (anIt.Value() != anItOther.Value())
      return Standard_False;
  }

  return Standard_True;
}

Standard_Boolean AssemblyItemId::IsDirectChild(const AssemblyItemId& theOther) const
{
  return ((myPath.Size() == theOther.myPath.Size() - 1) && IsChild(theOther));
}

Standard_Boolean AssemblyItemId::IsEqual(const AssemblyItemId& theOther) const
{
  if (this == &theOther)
    return Standard_True;

  if (myPath.Size() != theOther.myPath.Size())
    return Standard_False;

  TColStd_ListOfAsciiString::Iterator anIt(myPath), anItOther(theOther.myPath);
  for (; anIt.More() && anItOther.More(); anIt.Next(), anItOther.Next())
  {
    if (anIt.Value() != anItOther.Value())
      return Standard_False;
  }

  return Standard_True;
}

const TColStd_ListOfAsciiString& AssemblyItemId::GetPath() const
{
  return myPath;
}

AsciiString1 AssemblyItemId::ToString() const
{
  AsciiString1 aStr;
  for (TColStd_ListOfAsciiString::Iterator anIt(myPath); anIt.More(); anIt.Next())
  {
    aStr += '/';
    aStr += anIt.Value();
  }
  aStr.Remove(1, 1);
  return aStr;
}

//=================================================================================================

void AssemblyItemId::DumpJson(Standard_OStream& theOStream, Standard_Integer) const
{
  OCCT_DUMP_CLASS_BEGIN(theOStream, AssemblyItemId)

  for (TColStd_ListOfAsciiString::Iterator aPathIt(myPath); aPathIt.More(); aPathIt.Next())
  {
    AsciiString1 aPath = aPathIt.Value();
    OCCT_DUMP_FIELD_VALUE_STRING(theOStream, aPath)
  }
}
