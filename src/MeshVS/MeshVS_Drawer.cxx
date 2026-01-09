// Created on: 2003-11-27
// Created by: Alexander SOLOVYOV
// Copyright (c) 2003-2014 OPEN CASCADE SAS
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

#include <MeshVS_Drawer.hxx>
#include <Quantity_Color.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>

IMPLEMENT_STANDARD_RTTIEXT(MeshDrawer, RefObject)

//=================================================================================================

void MeshDrawer::Assign(const Handle(MeshDrawer)& aDrawer)
{
  if (!aDrawer.IsNull())
  {
    myIntegers    = aDrawer->myIntegers;
    myDoubles     = aDrawer->myDoubles;
    myBooleans    = aDrawer->myBooleans;
    myColors      = aDrawer->myColors;
    myMaterials   = aDrawer->myMaterials;
    myAsciiString = aDrawer->myAsciiString;
  }
}

//=================================================================================================

void MeshDrawer::SetInteger(const Standard_Integer Key1, const Standard_Integer Value)
{
  if (myIntegers.IsBound(Key1))
    myIntegers.ChangeFind(Key1) = Value;
  else
    myIntegers.Bind(Key1, Value);
}

//=================================================================================================

void MeshDrawer::SetDouble(const Standard_Integer Key1, const Standard_Real Value)
{
  if (myDoubles.IsBound(Key1))
    myDoubles.ChangeFind(Key1) = Value;
  else
    myDoubles.Bind(Key1, Value);
}

//=================================================================================================

void MeshDrawer::SetBoolean(const Standard_Integer Key1, const Standard_Boolean Value)
{
  if (myBooleans.IsBound(Key1))
    myBooleans.ChangeFind(Key1) = Value;
  else
    myBooleans.Bind(Key1, Value);
}

//=================================================================================================

void MeshDrawer::SetColor(const Standard_Integer Key1, const Color1& Value)
{
  if (myColors.IsBound(Key1))
    myColors.ChangeFind(Key1) = Value;
  else
    myColors.Bind(Key1, Value);
}

//=================================================================================================

void MeshDrawer::SetMaterial(const Standard_Integer Key1, const Graphic3d_MaterialAspect& Value)
{
  if (myMaterials.IsBound(Key1))
    myMaterials.ChangeFind(Key1) = Value;
  else
    myMaterials.Bind(Key1, Value);
}

//=================================================================================================

void MeshDrawer::SetAsciiString(const Standard_Integer Key1, const AsciiString1& Value)
{
  if (myAsciiString.IsBound(Key1))
    myAsciiString.ChangeFind(Key1) = Value;
  else
    myAsciiString.Bind(Key1, Value);
}

//=================================================================================================

Standard_Boolean MeshDrawer::GetInteger(const Standard_Integer Key1,
                                           Standard_Integer&      Value) const
{
  Standard_Boolean aRes = myIntegers.IsBound(Key1);
  if (aRes)
    Value = myIntegers.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::GetDouble(const Standard_Integer Key1, Standard_Real& Value) const
{
  Standard_Boolean aRes = myDoubles.IsBound(Key1);
  if (aRes)
    Value = myDoubles.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::GetBoolean(const Standard_Integer Key1,
                                           Standard_Boolean&      Value) const
{
  Standard_Boolean aRes = myBooleans.IsBound(Key1);
  if (aRes)
    Value = myBooleans.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::GetColor(const Standard_Integer Key1, Color1& Value) const
{
  Standard_Boolean aRes = myColors.IsBound(Key1);
  if (aRes)
    Value = myColors.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::GetMaterial(const Standard_Integer    Key1,
                                            Graphic3d_MaterialAspect& Value) const
{
  Standard_Boolean aRes = myMaterials.IsBound(Key1);
  if (aRes)
    Value = myMaterials.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::GetAsciiString(const Standard_Integer   Key1,
                                               AsciiString1& Value) const
{
  Standard_Boolean aRes = myAsciiString.IsBound(Key1);
  if (aRes)
    Value = myAsciiString.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::RemoveInteger(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myIntegers.IsBound(Key1);
  if (aRes)
    myIntegers.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::RemoveDouble(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myDoubles.IsBound(Key1);
  if (aRes)
    myDoubles.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::RemoveBoolean(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myBooleans.IsBound(Key1);
  if (aRes)
    myBooleans.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::RemoveColor(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myColors.IsBound(Key1);
  if (aRes)
    myColors.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::RemoveMaterial(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myMaterials.IsBound(Key1);
  if (aRes)
    myMaterials.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshDrawer::RemoveAsciiString(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myAsciiString.IsBound(Key1);
  if (aRes)
    myAsciiString.UnBind(Key1);
  return aRes;
}
