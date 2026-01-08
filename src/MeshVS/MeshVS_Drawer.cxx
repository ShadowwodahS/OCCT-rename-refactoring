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

IMPLEMENT_STANDARD_RTTIEXT(MeshVS_Drawer, RefObject)

//=================================================================================================

void MeshVS_Drawer::Assign(const Handle(MeshVS_Drawer)& aDrawer)
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

void MeshVS_Drawer::SetInteger(const Standard_Integer Key1, const Standard_Integer Value)
{
  if (myIntegers.IsBound(Key1))
    myIntegers.ChangeFind(Key1) = Value;
  else
    myIntegers.Bind(Key1, Value);
}

//=================================================================================================

void MeshVS_Drawer::SetDouble(const Standard_Integer Key1, const Standard_Real Value)
{
  if (myDoubles.IsBound(Key1))
    myDoubles.ChangeFind(Key1) = Value;
  else
    myDoubles.Bind(Key1, Value);
}

//=================================================================================================

void MeshVS_Drawer::SetBoolean(const Standard_Integer Key1, const Standard_Boolean Value)
{
  if (myBooleans.IsBound(Key1))
    myBooleans.ChangeFind(Key1) = Value;
  else
    myBooleans.Bind(Key1, Value);
}

//=================================================================================================

void MeshVS_Drawer::SetColor(const Standard_Integer Key1, const Quantity_Color& Value)
{
  if (myColors.IsBound(Key1))
    myColors.ChangeFind(Key1) = Value;
  else
    myColors.Bind(Key1, Value);
}

//=================================================================================================

void MeshVS_Drawer::SetMaterial(const Standard_Integer Key1, const Graphic3d_MaterialAspect& Value)
{
  if (myMaterials.IsBound(Key1))
    myMaterials.ChangeFind(Key1) = Value;
  else
    myMaterials.Bind(Key1, Value);
}

//=================================================================================================

void MeshVS_Drawer::SetAsciiString(const Standard_Integer Key1, const AsciiString1& Value)
{
  if (myAsciiString.IsBound(Key1))
    myAsciiString.ChangeFind(Key1) = Value;
  else
    myAsciiString.Bind(Key1, Value);
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::GetInteger(const Standard_Integer Key1,
                                           Standard_Integer&      Value) const
{
  Standard_Boolean aRes = myIntegers.IsBound(Key1);
  if (aRes)
    Value = myIntegers.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::GetDouble(const Standard_Integer Key1, Standard_Real& Value) const
{
  Standard_Boolean aRes = myDoubles.IsBound(Key1);
  if (aRes)
    Value = myDoubles.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::GetBoolean(const Standard_Integer Key1,
                                           Standard_Boolean&      Value) const
{
  Standard_Boolean aRes = myBooleans.IsBound(Key1);
  if (aRes)
    Value = myBooleans.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::GetColor(const Standard_Integer Key1, Quantity_Color& Value) const
{
  Standard_Boolean aRes = myColors.IsBound(Key1);
  if (aRes)
    Value = myColors.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::GetMaterial(const Standard_Integer    Key1,
                                            Graphic3d_MaterialAspect& Value) const
{
  Standard_Boolean aRes = myMaterials.IsBound(Key1);
  if (aRes)
    Value = myMaterials.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::GetAsciiString(const Standard_Integer   Key1,
                                               AsciiString1& Value) const
{
  Standard_Boolean aRes = myAsciiString.IsBound(Key1);
  if (aRes)
    Value = myAsciiString.Find(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::RemoveInteger(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myIntegers.IsBound(Key1);
  if (aRes)
    myIntegers.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::RemoveDouble(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myDoubles.IsBound(Key1);
  if (aRes)
    myDoubles.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::RemoveBoolean(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myBooleans.IsBound(Key1);
  if (aRes)
    myBooleans.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::RemoveColor(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myColors.IsBound(Key1);
  if (aRes)
    myColors.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::RemoveMaterial(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myMaterials.IsBound(Key1);
  if (aRes)
    myMaterials.UnBind(Key1);
  return aRes;
}

//=================================================================================================

Standard_Boolean MeshVS_Drawer::RemoveAsciiString(const Standard_Integer Key1)
{
  Standard_Boolean aRes = myAsciiString.IsBound(Key1);
  if (aRes)
    myAsciiString.UnBind(Key1);
  return aRes;
}
