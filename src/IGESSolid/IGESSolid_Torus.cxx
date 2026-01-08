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
#include <IGESSolid_Torus.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IGESSolid_Torus, IGESData_IGESEntity)

IGESSolid_Torus::IGESSolid_Torus() {}

void IGESSolid_Torus::Init(const Standard_Real R1,
                           const Standard_Real R2,
                           const Coords3d&       Point,
                           const Coords3d&       Axisdir)
{
  theR1    = R1;
  theR2    = R2;
  thePoint = Point;   // default (0,0,0)
  theAxis  = Axisdir; // default (0,0,1)
  InitTypeAndForm(160, 0);
}

Standard_Real IGESSolid_Torus::MajorRadius() const
{
  return theR1;
}

Standard_Real IGESSolid_Torus::DiscRadius() const
{
  return theR2;
}

Point3d IGESSolid_Torus::AxisPoint() const
{
  return Point3d(thePoint);
}

Point3d IGESSolid_Torus::TransformedAxisPoint() const
{
  if (!HasTransf())
    return Point3d(thePoint);
  else
  {
    Coords3d pnt = thePoint;
    Location().Transforms(pnt);
    return Point3d(pnt);
  }
}

Dir3d IGESSolid_Torus::Axis() const
{
  return Dir3d(theAxis);
}

Dir3d IGESSolid_Torus::TransformedAxis() const
{
  if (!HasTransf())
    return Dir3d(theAxis);
  else
  {
    Coords3d   pnt = theAxis;
    GeneralTransform loc = Location();
    loc.SetTranslationPart(Coords3d(0., 0., 0.));
    loc.Transforms(pnt);
    return Dir3d(pnt);
  }
}
