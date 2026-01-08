// Created by: CKY / Contract Toubro-Larsen
// Copyright (c) 1993-1999 Matra Datavision
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

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#include <gp_GTrsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <IGESGeom_Direction.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESGeom_Direction, IGESData_IGESEntity)

IGESGeom_Direction::IGESGeom_Direction() {}

void IGESGeom_Direction::Init(const Coords3d& aDirection)
{
  theDirection = aDirection;
  InitTypeAndForm(123, 0);
}

Vector3d IGESGeom_Direction::Value() const
{
  Vector3d direction(theDirection);
  return direction;
}

Vector3d IGESGeom_Direction::TransformedValue() const
{
  if (!HasTransf())
    return Vector3d(theDirection);
  Coords3d   xyz(theDirection);
  GeneralTransform loc = Location();
  loc.SetTranslationPart(Coords3d(0., 0., 0.));
  loc.Transforms(xyz);
  return Vector3d(xyz);
}
