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

#include <StdLPersistent_Collection.hxx>

#include <TColStd_HPackedMapOfInteger.hxx>
#include <TCollection_HExtendedString.hxx>

struct Collection::noConversion
{
  noConversion(const Handle(TDF_Data)&) {}

  template <class Type>
  Type operator()(Type theValue) const
  {
    return theValue;
  }
};

struct Collection::byteConverter
{
  byteConverter(const Handle(TDF_Data)&) {}

  Standard_Byte operator()(Standard_Integer theValue) const
  {
    return static_cast<Standard_Byte>(theValue);
  }
};

struct Collection::boolConverter
{
  boolConverter(const Handle(TDF_Data)&) {}

  Standard_Boolean operator()(Standard_Integer theValue) const { return theValue != 0; }
};

struct Collection::stringConverter
{
  stringConverter(const Handle(TDF_Data)&) {}

  const UtfString& operator()(const Handle(StdObjMgt_Persistent)& theValue) const
  {
    static UtfString anEmptyString;
    if (theValue.IsNull())
      return anEmptyString;

    Handle(TCollection_HExtendedString) aString = theValue->ExtString();
    return aString ? aString->String() : anEmptyString;
  }
};

struct Collection::referenceConverter
{
  referenceConverter(const Handle(TDF_Data)& theDF)
      : myDF(theDF)
  {
  }

  DataLabel operator()(const Handle(StdObjMgt_Persistent)& theValue) const
  {
    return theValue->Label(myDF);
  }

private:
  Handle(TDF_Data) myDF;
};

template <class Base>
template <class ArrayHandle, class Converter>
void Collection::booleanArrayBase<Base>::import(const ArrayHandle& theArray,
                                                               Converter theConverter) const
{
  Handle(TColStd_HArray1OfByte) aByteArray =
    new TColStd_HArray1OfByte(theArray->Lower(), theArray->Upper());

  for (Standard_Integer i = theArray->Lower(); i <= theArray->Upper(); i++)
    aByteArray->SetValue(i, theConverter(theArray->Value(i)));

  this->myTransient->Init(myLower, myUpper);
  this->myTransient->SetInternalArray(aByteArray);
}

template <class Base>
template <class ArrayHandle, class Converter>
void Collection::directArrayBase<Base>::import(const ArrayHandle& theArray,
                                                              Converter) const
{
  this->myTransient->ChangeArray(theArray);
}

template <class Base>
template <class ArrayHandle, class Converter>
void Collection::arrayBase<Base>::import(const ArrayHandle& theArray,
                                                        Converter          theConverter) const
{
  this->myTransient->Init(theArray->Lower(), theArray->Upper());
  for (Standard_Integer i = theArray->Lower(); i <= theArray->Upper(); i++)
    this->myTransient->SetValue(i, theConverter(theArray->Value(i)));
}

template <class Base>
template <class ArrayHandle, class Converter>
void Collection::listBase<Base>::import(const ArrayHandle& theArray,
                                                       Converter          theConverter) const
{
  for (Standard_Integer i = theArray->Lower(); i <= theArray->Upper(); i++)
    this->myTransient->Append(theConverter(theArray->Value(i)));
}

template <class Base>
template <class ArrayHandle, class Converter>
void Collection::mapBase<Base>::import(const ArrayHandle& theArray,
                                                      Converter          theConverter) const
{
  Handle(TColStd_HPackedMapOfInteger) anHMap = new TColStd_HPackedMapOfInteger;
  for (Standard_Integer i = theArray->Lower(); i <= theArray->Upper(); i++)
    anHMap->ChangeMap().Add(theConverter(theArray->Value(i)));
  this->myTransient->ChangeMap(anHMap);
}

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
template <template <class> class BaseT, class HArrayClass, class AttribClass, class Converter>
void Collection::instance<BaseT, HArrayClass, AttribClass, Converter>::
  ImportAttribute()
{
  Handle(HArrayClass) anHArray = Handle(HArrayClass)::DownCast(this->myData);
  if (anHArray)
  {
    typename HArrayClass::ArrayHandle anArray = anHArray->Array();
    if (anArray)
      this->import(anArray, Converter(this->myTransient->Label().Data()));
    this->myData.Nullify();
  }
}

//=======================================================================
// function : Read
// purpose  : Read persistent data from a file
//=======================================================================
template <class Instance>
void Collection::instance_1<Instance>::Read(ReadData& theReadData)
{
  Instance::Read(theReadData);
  theReadData >> myDelta;
}

//=======================================================================
// function : ImportAttribute
// purpose  : Import transient attribute from the persistent data
//=======================================================================
template <class Instance>
void Collection::instance_1<Instance>::ImportAttribute()
{
  Instance::ImportAttribute();
  this->myTransient->SetDelta(myDelta);
}

template class Collection::instance<Collection::booleanArrayBase,
                                                   Collection::integer,
                                                   TDataStd_BooleanArray,
                                                   Collection::byteConverter>;

template class Collection::instance<Collection::directArrayBase,
                                                   Collection::integer,
                                                   TDataStd_IntegerArray,
                                                   Collection::noConversion>;

template class Collection::instance<Collection::directArrayBase,
                                                   Collection::real,
                                                   TDataStd_RealArray,
                                                   Collection::noConversion>;

template class Collection::instance<Collection::arrayBase,
                                                   Collection::integer,
                                                   TDataStd_ByteArray,
                                                   Collection::byteConverter>;

template class Collection::instance<Collection::arrayBase,
                                                   Collection::persistent,
                                                   TDataStd_ExtStringArray,
                                                   Collection::stringConverter>;

template class Collection::instance<Collection::arrayBase,
                                                   Collection::persistent,
                                                   TDataStd_ReferenceArray,
                                                   Collection::referenceConverter>;

template class Collection::instance<Collection::listBase,
                                                   Collection::integer,
                                                   TDataStd_IntegerList,
                                                   Collection::noConversion>;

template class Collection::instance<Collection::listBase,
                                                   Collection::real,
                                                   TDataStd_RealList,
                                                   Collection::noConversion>;

template class Collection::instance<Collection::listBase,
                                                   Collection::integer,
                                                   TDataStd_BooleanList,
                                                   Collection::boolConverter>;

template class Collection::instance<Collection::listBase,
                                                   Collection::persistent,
                                                   TDataStd_ExtStringList,
                                                   Collection::stringConverter>;

template class Collection::instance<Collection::listBase,
                                                   Collection::persistent,
                                                   TDataStd_ReferenceList,
                                                   Collection::referenceConverter>;

template class Collection::instance<Collection::mapBase,
                                                   Collection::integer,
                                                   TDataStd_IntPackedMap,
                                                   Collection::noConversion>;

template class Collection::instance_1<Collection::IntegerArray>;

template class Collection::instance_1<Collection::RealArray>;

template class Collection::instance_1<Collection::ByteArray>;

template class Collection::instance_1<Collection::ExtStringArray>;

template class Collection::instance_1<Collection::IntPackedMap>;
