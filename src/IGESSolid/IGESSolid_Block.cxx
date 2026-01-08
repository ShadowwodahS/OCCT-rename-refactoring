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
#include <gp_Pnt.hxx>
#include <gp_XYZ.hxx>
#include <IGESSolid_Block.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESSolid_Block, IGESData_IGESEntity)

IGESSolid_Block::IGESSolid_Block() {}

void IGESSolid_Block::Init(const Coords3d& aSize,
                           const Coords3d& aCorner,
                           const Coords3d& aXAxis,
                           const Coords3d& aZAxis)
{
  theSize   = aSize;
  theCorner = aCorner; // default (0,0,0)
  theXAxis  = aXAxis;  // default (1,0,0)
  theZAxis  = aZAxis;  // default (0,0,1)
  InitTypeAndForm(150, 0);
}

Coords3d IGESSolid_Block::Size() const
{
  return theSize;
}

Standard_Real IGESSolid_Block::XLength() const
{
  return theSize.X();
}

Standard_Real IGESSolid_Block::YLength() const
{
  return theSize.Y();
}

Standard_Real IGESSolid_Block::ZLength() const
{
  return theSize.Z();
}

Point3d IGESSolid_Block::Corner() const
{
  return Point3d(theCorner);
}

Point3d IGESSolid_Block::TransformedCorner() const
{
  if (!HasTransf())
    return Point3d(theCorner);
  else
  {
    Coords3d tmp = theCorner;
    Location().Transforms(tmp);
    return Point3d(tmp);
  }
}

Dir3d IGESSolid_Block::XAxis() const
{
  return Dir3d(theXAxis);
}

Dir3d IGESSolid_Block::TransformedXAxis() const
{
  if (!HasTransf())
    return Dir3d(theXAxis);
  else
  {
    Coords3d   xyz = theXAxis;
    GeneralTransform loc = Location();
    loc.SetTranslationPart(Coords3d(0., 0., 0.));
    loc.Transforms(xyz);
    return Dir3d(xyz);
  }
}

Dir3d IGESSolid_Block::YAxis() const
{
  return Dir3d(theXAxis ^ theZAxis); // ^ overloaded
}

Dir3d IGESSolid_Block::TransformedYAxis() const
{
  if (!HasTransf())
    return Dir3d(theXAxis ^ theZAxis);
  else
  {
    Coords3d   xyz = theXAxis ^ theZAxis;
    GeneralTransform loc = Location();
    loc.SetTranslationPart(Coords3d(0., 0., 0.));
    loc.Transforms(xyz);
    return Dir3d(xyz);
  }
}

Dir3d IGESSolid_Block::ZAxis() const
{
  return Dir3d(theZAxis);
}

Dir3d IGESSolid_Block::TransformedZAxis() const
{
  if (!HasTransf())
    return Dir3d(theZAxis);
  else
  {
    Coords3d   xyz(theZAxis);
    GeneralTransform loc = Location();
    loc.SetTranslationPart(Coords3d(0., 0., 0.));
    loc.Transforms(xyz);
    return Dir3d(xyz);
  }
}
