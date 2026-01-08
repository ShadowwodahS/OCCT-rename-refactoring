// Created on: 2004-11-22
// Created by: Pavel TELKOV
// Copyright (c) 2004-2014 OPEN CASCADE SAS
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

// The original implementation Copyright: (C) RINA S.p.A

#include <TObj_Assistant.hxx>

#include <TObj_Model.hxx>
#include <TColStd_SequenceOfTransient.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TColStd_IndexedMapOfTransient.hxx>
#include <Standard_Type.hxx>

//=================================================================================================

TColStd_SequenceOfTransient& Assistant::getModels()
{
  static TColStd_SequenceOfTransient sModels;
  return sModels;
}

//=================================================================================================

TColStd_IndexedMapOfTransient& Assistant::getTypes()
{
  static TColStd_IndexedMapOfTransient sTypes;
  return sTypes;
}

//=================================================================================================

Handle(TObj_Model)& Assistant::getCurrentModel()
{
  static Handle(TObj_Model) sCurrentModel;
  return sCurrentModel;
}

//=================================================================================================

Standard_Integer& Assistant::getVersion()
{
  static Standard_Integer sVersion = 0;
  return sVersion;
}

//=================================================================================================

Handle(TObj_Model) Assistant::FindModel(const Standard_CString theName)
{
  UtfString aName(theName, Standard_True);
  Standard_Integer           i = getModels().Length();
  Handle(TObj_Model)         aModel;
  for (; i > 0; i--)
  {
    aModel = Handle(TObj_Model)::DownCast(getModels().Value(i));
    if (aName == aModel->GetModelName()->String())
      break;
  }
  if (i == 0)
    aModel.Nullify();

  return aModel;
}

//=================================================================================================

void Assistant::BindModel(const Handle(TObj_Model)& theModel)
{
  getModels().Append(theModel);
}

//=================================================================================================

void Assistant::ClearModelMap()
{
  getModels().Clear();
}

//=================================================================================================

Handle(TypeInfo) Assistant::FindType(const Standard_Integer theTypeIndex)
{
  if (theTypeIndex > 0 && theTypeIndex <= getTypes().Extent())
    return Handle(TypeInfo)::DownCast(getTypes().FindKey(theTypeIndex));

  return 0;
}

//=================================================================================================

Standard_Integer Assistant::FindTypeIndex(const Handle(TypeInfo)& theType)
{
  if (!getTypes().Contains(theType))
    return 0;

  return getTypes().FindIndex(theType);
}

//=================================================================================================

class TObj_Assistant_UnknownType : public RefObject
{
public:
  TObj_Assistant_UnknownType() {}
  // Empty constructor

  // CASCADE RTTI
  DEFINE_STANDARD_RTTI_INLINE(TObj_Assistant_UnknownType, RefObject)
};

// Define handle class for TObj_Assistant_UnknownType
DEFINE_STANDARD_HANDLE(TObj_Assistant_UnknownType, RefObject)

//=================================================================================================

Standard_Integer Assistant::BindType(const Handle(TypeInfo)& theType)
{
  if (theType.IsNull())
  {
    Handle(RefObject) anUnknownType;
    anUnknownType = new TObj_Assistant_UnknownType;
    return getTypes().Add(anUnknownType);
  }

  return getTypes().Add(theType);
}

//=================================================================================================

void Assistant::ClearTypeMap()
{
  getTypes().Clear();
}

//=================================================================================================

void Assistant::SetCurrentModel(const Handle(TObj_Model)& theModel)
{
  getCurrentModel() = theModel;
  getVersion()      = -1;
}

//=================================================================================================

Handle(TObj_Model) Assistant::GetCurrentModel()
{
  return getCurrentModel();
}

//=================================================================================================

void Assistant::UnSetCurrentModel()
{
  getCurrentModel().Nullify();
}

//=================================================================================================

Standard_Integer Assistant::GetAppVersion()
{
  Standard_Integer& aVersion = getVersion();
  if (aVersion < 0)
  {
    Handle(TObj_Model)& aModel = getCurrentModel();
    if (!aModel.IsNull())
      aVersion = aModel->GetFormatVersion();
  }
  return aVersion;
}
