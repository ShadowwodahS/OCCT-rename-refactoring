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

#include <StdLPersistent.hxx>
#include <StdObjMgt_MapOfInstantiators.hxx>

#include <StdLPersistent_Document.hxx>
#include <StdLPersistent_Data.hxx>
#include <StdLPersistent_HArray1.hxx>
#include <StdLPersistent_Void.hxx>
#include <StdLPersistent_Real.hxx>
#include <StdLPersistent_Value.hxx>
#include <StdLPersistent_Collection.hxx>
#include <StdLPersistent_Dependency.hxx>
#include <StdLPersistent_Variable.hxx>
#include <StdLPersistent_XLink.hxx>
#include <StdLPersistent_Function.hxx>
#include <StdLPersistent_TreeNode.hxx>
#include <StdLPersistent_NamedData.hxx>

//=======================================================================
// function : BindTypes
// purpose  : Register types
//=======================================================================
void StdLPersistent1::BindTypes(MapOfInstantiators& theMap)
{
  // Non-attribute data
  theMap.Bind<StdLPersistent_Document>("PDocStd_Document");
  theMap.Bind<StdLPersistent_Data>("PDF_Data");

  theMap.Bind<HString::Ascii1>("PCollection_HAsciiString");
  theMap.Bind<HString::Extended1>("PCollection_HExtendedString");

  theMap.Bind<HArray1::Integer1>("PColStd_HArray1OfInteger");
  theMap.Bind<HArray1::Real>("PColStd_HArray1OfReal");
  theMap.Bind<HArray1::Persistent>("PColStd_HArray1OfExtendedString");
  theMap.Bind<HArray1::Persistent>("PDF_HAttributeArray1");
  theMap.Bind<HArray1::Persistent>("PDataStd_HArray1OfHAsciiString");
  theMap.Bind<HArray1::Persistent>("PDataStd_HArray1OfHArray1OfInteger");
  theMap.Bind<HArray1::Persistent>("PDataStd_HArray1OfHArray1OfReal");
  theMap.Bind<HArray1::Byte>("PDataStd_HArray1OfByte");

  theMap.Bind<HArray2::Integer1>("PColStd_HArray2OfInteger");

  // Attributes
  theMap.Bind<Void::Directory>("PDataStd_Directory");
  theMap.Bind<Void::Tick>("PDataStd_Tick");
  theMap.Bind<Void::NoteBook>("PDataStd_NoteBook");

  theMap.Bind<Value::Integer1>("PDataStd_Integer");
  theMap.Bind<Value::TagSource1>("PDF_TagSource");
  theMap.Bind<Value::Reference1>("PDF_Reference");
  theMap.Bind<Value::UAttribute1>("PDataStd_UAttribute");

  theMap.Bind<Value::Name>("PDataStd_Name");
  theMap.Bind<Value::Comment1>("PDataStd_Comment");
  theMap.Bind<Value::AsciiString2>("PDataStd_AsciiString");

  theMap.Bind<Collection::IntegerArray>("PDataStd_IntegerArray");
  theMap.Bind<Collection::RealArray>("PDataStd_RealArray");
  theMap.Bind<Collection::ByteArray>("PDataStd_ByteArray");
  theMap.Bind<Collection::ExtStringArray>("PDataStd_ExtStringArray");
  theMap.Bind<Collection::BooleanArray>("PDataStd_BooleanArray");
  theMap.Bind<Collection::ReferenceArray>("PDataStd_ReferenceArray");

  theMap.Bind<Collection::IntegerArray_1>("PDataStd_IntegerArray_1");
  theMap.Bind<Collection::RealArray_1>("PDataStd_RealArray_1");
  theMap.Bind<Collection::ByteArray_1>("PDataStd_ByteArray_1");
  theMap.Bind<Collection::ExtStringArray_1>("PDataStd_ExtStringArray_1");

  theMap.Bind<Collection::IntegerList>("PDataStd_IntegerList");
  theMap.Bind<Collection::RealList>("PDataStd_RealList");
  theMap.Bind<Collection::BooleanList>("PDataStd_BooleanList");
  theMap.Bind<Collection::ExtStringList>("PDataStd_ExtStringList");
  theMap.Bind<Collection::ReferenceList>("PDataStd_ReferenceList");

  theMap.Bind<Collection::IntPackedMap>("PDataStd_IntPackedMap");
  theMap.Bind<Collection::IntPackedMap_1>("PDataStd_IntPackedMap_1");

  theMap.Bind<Real>("PDataStd_Real");
  theMap.Bind<Dependency::Expression>("PDataStd_Expression");
  theMap.Bind<Dependency::Relation>("PDataStd_Relation");
  theMap.Bind<StdLPersistent_Variable>("PDataStd_Variable");
  theMap.Bind<StdLPersistent_XLink>("PDocStd_XLink");
  theMap.Bind<StdLPersistent_Function>("PFunction_Function");
  theMap.Bind<TreeNode>("PDataStd_TreeNode");
  theMap.Bind<StdLPersistent_NamedData>("PDataStd_NamedData");
}
