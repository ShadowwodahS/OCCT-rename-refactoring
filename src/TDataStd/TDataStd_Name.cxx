// Created on: 1997-07-31
// Created by: Denis PASCAL
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

#include <TDataStd_Name.hxx>
#include <TDF_Label.hxx>

IMPLEMENT_DERIVED_ATTRIBUTE(NameAttribute, TDataStd_GenericExtString)

//=================================================================================================

const Standard_GUID& NameAttribute::GetID()
{
  static Standard_GUID TDataStd_NameID("2a96b608-ec8b-11d0-bee7-080009dc3333");
  return TDataStd_NameID;
}

//=======================================================================
// function : SetAttr
// purpose  : Implements Set functionality
//=======================================================================
static Handle(NameAttribute) SetAttr(const DataLabel&                  label,
                                     const UtfString& theString,
                                     const Standard_GUID&              theGuid)
{
  Handle(NameAttribute) N;
  if (!label.FindAttribute(theGuid, N))
  {
    N = new NameAttribute();
    N->SetID(theGuid);
    label.AddAttribute(N);
  }
  N->Set(theString);
  return N;
}

//=================================================================================================

Handle(NameAttribute) NameAttribute::Set(const DataLabel&                  label,
                                         const UtfString& theString)
{
  return SetAttr(label, theString, GetID());
}

//=======================================================================
// function : Set
// purpose  : Set user defined attribute
//=======================================================================

Handle(NameAttribute) NameAttribute::Set(const DataLabel&                  label,
                                         const Standard_GUID&              theGuid,
                                         const UtfString& theString)
{
  return SetAttr(label, theString, theGuid);
}

//=================================================================================================

NameAttribute::NameAttribute()
{
  myID = GetID();
}

//=================================================================================================

void NameAttribute::Set(const UtfString& S)
{
  if (myString == S)
    return;

  Backup();
  myString = S;
}

//=================================================================================================

void NameAttribute::SetID(const Standard_GUID& theGuid)
{
  if (myID == theGuid)
    return;

  Backup();
  myID = theGuid;
}

//=======================================================================
// function : SetID
// purpose  : sets default ID
//=======================================================================

void NameAttribute::SetID()
{
  Backup();
  myID = GetID();
}

// TDF_Attribute methods
//=================================================================================================

Standard_OStream& NameAttribute::Dump(Standard_OStream& anOS) const
{
  TDF_Attribute::Dump(anOS);
  anOS << " Name=|" << myString << "|";
  Standard_Character sguid[Standard_GUID_SIZE_ALLOC];
  myID.ToCString(sguid);
  anOS << sguid << std::endl;
  return anOS;
}
