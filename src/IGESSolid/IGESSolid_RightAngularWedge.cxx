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
#include <IGESSolid_RightAngularWedge.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESSolid_RightAngularWedge, IGESData_IGESEntity)

IGESSolid_RightAngularWedge::IGESSolid_RightAngularWedge() {}

void IGESSolid_RightAngularWedge::Init(const gp_XYZ&       aSize,
                                       const Standard_Real LowX,
                                       const gp_XYZ&       aCorner,
                                       const gp_XYZ&       anXAxis,
                                       const gp_XYZ&       anZAxis)
{
  theSize         = aSize;
  theXSmallLength = LowX;
  theCorner       = aCorner; // default (0,0,0)
  theXAxis        = anXAxis; // default (1,0,0)
  theZAxis        = anZAxis; // default (0,0,1)
  InitTypeAndForm(152, 0);
}

gp_XYZ IGESSolid_RightAngularWedge::Size() const
{
  return theSize;
}

Standard_Real IGESSolid_RightAngularWedge::XBigLength() const
{
  return theSize.X();
}

Standard_Real IGESSolid_RightAngularWedge::XSmallLength() const
{
  return theXSmallLength;
}

Standard_Real IGESSolid_RightAngularWedge::YLength() const
{
  return theSize.Y();
}

Standard_Real IGESSolid_RightAngularWedge::ZLength() const
{
  return theSize.Z();
}

Point3d IGESSolid_RightAngularWedge::Corner() const
{
  return Point3d(theCorner);
}

Point3d IGESSolid_RightAngularWedge::TransformedCorner() const
{
  if (!HasTransf())
    return Point3d(theCorner);
  else
  {
    gp_XYZ tmp = theCorner;
    Location().Transforms(tmp);
    return Point3d(tmp);
  }
}

Dir3d IGESSolid_RightAngularWedge::XAxis() const
{
  return Dir3d(theXAxis);
}

Dir3d IGESSolid_RightAngularWedge::TransformedXAxis() const
{
  if (!HasTransf())
    return Dir3d(theXAxis);
  else
  {
    gp_XYZ   tmp = theXAxis;
    gp_GTrsf loc = Location();
    loc.SetTranslationPart(gp_XYZ(0., 0., 0.));
    loc.Transforms(tmp);
    return Dir3d(tmp);
  }
}

Dir3d IGESSolid_RightAngularWedge::YAxis() const
{
  return Dir3d(theXAxis ^ theZAxis); // ^ overloaded
}

Dir3d IGESSolid_RightAngularWedge::TransformedYAxis() const
{
  if (!HasTransf())
    return Dir3d(theXAxis ^ theZAxis);
  else
  {
    gp_XYZ   tmp = theXAxis ^ theZAxis;
    gp_GTrsf loc = Location();
    loc.SetTranslationPart(gp_XYZ(0., 0., 0.));
    loc.Transforms(tmp);
    return Dir3d(tmp);
  }
}

Dir3d IGESSolid_RightAngularWedge::ZAxis() const
{
  return Dir3d(theZAxis);
}

Dir3d IGESSolid_RightAngularWedge::TransformedZAxis() const
{
  if (!HasTransf())
    return Dir3d(theZAxis);
  else
  {
    gp_XYZ   tmp = theZAxis;
    gp_GTrsf loc = Location();
    loc.SetTranslationPart(gp_XYZ(0., 0., 0.));
    loc.Transforms(tmp);
    return Dir3d(tmp);
  }
}
