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

#include <StdLPersistent_Value.hxx>

#include <TCollection_HExtendedString.hxx>
#include <Standard_GUID.hxx>

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
template <class AttribClass>
void Value::integer<AttribClass>::ImportAttribute()
{
  this->myTransient->Set(this->myData);
}

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
template <class AttribClass, class HStringClass>
void Value::string<AttribClass, HStringClass>::ImportAttribute()
{
  Handle(HStringClass) anHString = Handle(HStringClass)::DownCast(this->myData);
  if (anHString)
  {
    this->myTransient->Set(anHString->Value()->String());
    this->myData.Nullify();
  }
}

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
template <>
void Value::string<TDF_Reference>::ImportAttribute()
{
  if (this->myData)
  {
    DataLabel aLabel = myData->Label(this->myTransient->Label().Data());
    if (!aLabel.IsNull())
      this->myTransient->Set(aLabel);
    this->myData.Nullify();
  }
}

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
template <>
void Value::string<TDataStd_UAttribute>::ImportAttribute()
{
}

//=======================================================================
// function : CreateAttribute
// purpose  : Create an empty transient attribute
//=======================================================================
Handle(TDF_Attribute) Value::UAttribute1::CreateAttribute()
{
  string<TDataStd_UAttribute, HString::Extended1>::CreateAttribute();

  if (this->myData)
  {
    Handle(TCollection_HExtendedString) aString = this->myData->ExtString();
    if (aString)
      this->myTransient->SetID(Standard_GUID(aString->String().ToExtString()));
    this->myData.Nullify();
  }

  return this->myTransient;
}

//=======================================================================
Handle(TDF_Attribute) Value::Integer1::CreateAttribute()
{
  integer<IntAttribute>::CreateAttribute();

  if (this->myData)
  {
    this->myTransient->SetID(IntAttribute::GetID());
  }

  return this->myTransient;
}

//=======================================================================
Handle(TDF_Attribute) Value::Name::CreateAttribute()
{
  string<NameAttribute>::CreateAttribute();

  if (this->myData)
  {
    this->myTransient->SetID(NameAttribute::GetID());
  }

  return this->myTransient;
}

//=======================================================================
Handle(TDF_Attribute) Value::AsciiString2::CreateAttribute()
{
  string<TDataStd_AsciiString, HString::Ascii1>::CreateAttribute();

  if (this->myData)
  {
    this->myTransient->SetID(TDataStd_AsciiString::GetID());
  }

  return this->myTransient;
}

template class Value::integer<IntAttribute>;
template class Value::integer<TDF_TagSource>;

template class Value::string<TDF_Reference>;
template class Value::string<TDataStd_UAttribute>;
template class Value::string<NameAttribute>;
template class Value::string<TDataStd_Comment>;
template class Value::string<TDataStd_AsciiString, HString::Ascii1>;
