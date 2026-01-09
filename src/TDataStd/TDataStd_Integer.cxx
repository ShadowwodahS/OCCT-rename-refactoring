// Created on: 1997-03-06
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

#include <TDataStd_Integer.hxx>

#include <Standard_Dump.hxx>
#include <Standard_GUID.hxx>
#include <Standard_Type.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_Label.hxx>
#include <TDF_Reference.hxx>
#include <TDF_RelocationTable.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IntAttribute, TDF_Attribute)

//=================================================================================================

const Standard_GUID& IntAttribute::GetID()
{
  static Standard_GUID TDataStd_IntegerID("2a96b606-ec8b-11d0-bee7-080009dc3333");
  return TDataStd_IntegerID;
}

//=======================================================================
// function : SetAttr
// purpose  : Implements Set functionality
//=======================================================================
static Handle(IntAttribute) SetAttr(const DataLabel&       label,
                                        const Standard_Integer V,
                                        const Standard_GUID&   theGuid)
{
  Handle(IntAttribute) A;
  if (!label.FindAttribute(theGuid, A))
  {
    A = new IntAttribute();
    A->SetID(theGuid);
    label.AddAttribute(A);
  }
  A->Set(V);
  return A;
}

//=================================================================================================

Handle(IntAttribute) IntAttribute::Set(const DataLabel& L, const Standard_Integer V)

{
  return SetAttr(L, V, GetID());
}

//=======================================================================
// function : Set
// purpose  : Set user defined attribute
//=======================================================================

Handle(IntAttribute) IntAttribute::Set(const DataLabel&       L,
                                               const Standard_GUID&   theGuid,
                                               const Standard_Integer V)
{
  return SetAttr(L, V, theGuid);
}

//=======================================================================
// function : IntAttribute
// purpose  : Empty Constructor
//=======================================================================

IntAttribute::IntAttribute()
    : myValue(-1),
      myID(GetID())
{
}

//=================================================================================================

void IntAttribute::Set(const Standard_Integer v)
{
  // OCC2932 correction
  if (myValue == v)
    return;

  Backup();
  myValue = v;
}

//=================================================================================================

Standard_Integer IntAttribute::Get() const
{
  return myValue;
}

//=================================================================================================

Standard_Boolean IntAttribute::IsCaptured() const
{
  Handle(TDF_Reference) R;
  return (Label().FindAttribute(TDF_Reference::GetID(), R));
}

//=================================================================================================

const Standard_GUID& IntAttribute::ID() const
{
  return myID;
}

//=================================================================================================

void IntAttribute::SetID(const Standard_GUID& theGuid)
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
void IntAttribute::SetID()
{
  Backup();
  myID = GetID();
}

//=================================================================================================

Handle(TDF_Attribute) IntAttribute::NewEmpty() const
{
  return new IntAttribute();
}

//=================================================================================================

void IntAttribute::Restore(const Handle(TDF_Attribute)& With)
{
  Handle(IntAttribute) anInt = Handle(IntAttribute)::DownCast(With);
  myValue                        = anInt->Get();
  myID                           = anInt->ID();
}

//=================================================================================================

void IntAttribute::Paste(const Handle(TDF_Attribute)& Into,
                             const Handle(RelocationTable1)& /*RT*/) const
{
  Handle(IntAttribute) anInt = Handle(IntAttribute)::DownCast(Into);
  anInt->Set(myValue);
  anInt->SetID(myID);
}

//=================================================================================================

Standard_OStream& IntAttribute::Dump(Standard_OStream& anOS) const
{
  anOS << "Integer1:: " << this << " : ";
  anOS << myValue;
  Standard_Character sguid[Standard_GUID_SIZE_ALLOC];
  myID.ToCString(sguid);
  anOS << sguid;
  //
  anOS << "\nAttribute fields: ";
  TDF_Attribute::Dump(anOS);
  return anOS;
}

//=================================================================================================

void IntAttribute::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDF_Attribute)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myValue)
}
