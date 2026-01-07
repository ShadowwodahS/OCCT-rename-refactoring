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

#include <IFSelect_SelectType.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IFSelect_SelectType, IFSelect_SelectAnyType)

IFSelect_SelectType::IFSelect_SelectType()
{
  thetype = STANDARD_TYPE(RefObject);
}

IFSelect_SelectType::IFSelect_SelectType(const Handle(TypeInfo)& atype)
{
  thetype = atype;
}

void IFSelect_SelectType::SetType(const Handle(TypeInfo)& atype)
{
  thetype = atype;
}

Handle(TypeInfo) IFSelect_SelectType::TypeForMatch() const
{
  return thetype;
}

AsciiString1 IFSelect_SelectType::ExtractLabel() const
{
  return AsciiString1(thetype->Name());
}
