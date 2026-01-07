// Created on: 1992-09-03
// Created by: Remi GILET
// Copyright (c) 1992-1999 Matra Datavision
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

#include <gce_MakeRotation.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

//=========================================================================
//   Creation d une rotation 3d de gp d angle Angle par rapport a une     +
//   droite Line.                                                         +
//=========================================================================
RotationBuilder::RotationBuilder(const gp_Lin& Line, const Standard_Real Angle)
{
  TheRotation.SetRotation(Axis3d(Line.Position()), Angle);
}

//=========================================================================
//   Creation d une rotation 3d de gp d angle Angle par rapport a un      +
//   axe Axis.                                                            +
//=========================================================================

RotationBuilder::RotationBuilder(const Axis3d& Axis, const Standard_Real Angle)
{
  TheRotation.SetRotation(Axis, Angle);
}

//=========================================================================
//   Creation d une rotation 3d de gp d angle Angle par rapport a une     +
//   droite issue du point Point et de direction Direc.                   +
//=========================================================================

RotationBuilder::RotationBuilder(const Point3d&       Point,
                                   const Dir3d&       Direc,
                                   const Standard_Real Angle)
{
  TheRotation.SetRotation(Axis3d(Point, Direc), Angle);
}

const Transform3d& RotationBuilder::Value() const
{
  return TheRotation;
}

const Transform3d& RotationBuilder::Operator() const
{
  return TheRotation;
}

RotationBuilder::operator Transform3d() const
{
  return TheRotation;
}
