// Copyright (c) 1998-1999 Matra Datavision
// Copyright (c) 1999-2023 OPEN CASCADE SAS
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

#include <Standard_Transient.hxx>

#include <Standard_Type.hxx>
#include <Standard_CString.hxx>
#include <Standard_ProgramError.hxx>

const Handle(TypeInfo)& RefObject::get_type_descriptor()
{
  static const Handle(TypeInfo) THE_TYPE_INSTANCE =
    TypeInfo::Register(typeid(RefObject),
                            get_type_name(),
                            sizeof(RefObject),
                            nullptr);
  return THE_TYPE_INSTANCE;
}

//
//
const Handle(TypeInfo)& RefObject::DynamicType() const
{
  return get_type_descriptor();
}

//
//
Standard_Boolean RefObject::IsInstance(const Handle(TypeInfo)& AType) const
{
  return (AType == DynamicType());
}

//
//
Standard_Boolean RefObject::IsInstance(const Standard_CString theTypeName) const
{
  return IsEqual(DynamicType()->Name(), theTypeName);
}

//
//
Standard_Boolean RefObject::IsKind(const Handle(TypeInfo)& aType) const
{
  return DynamicType()->SubType(aType);
}

//
//
Standard_Boolean RefObject::IsKind(const Standard_CString theTypeName) const
{
  return DynamicType()->SubType(theTypeName);
}

//
//
RefObject* RefObject::This() const
{
  if (GetRefCount() == 0)
    throw Standard_ProgramError(
      "Attempt to create handle to object created in stack, not yet constructed, or destroyed");
  return const_cast<RefObject*>(this);
}
