// Copyright (c) 1995-1999 Matra Datavision
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

#include <gp.hxx>

#include <gp_Ax2.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

//=================================================================================================

const Point3d& gp::Origin()
{
  static Point3d gp_Origin(0, 0, 0);
  return gp_Origin;
}

//=================================================================================================

const Dir3d& gp::DX()
{
  static Dir3d gp_DX(1, 0, 0);
  return gp_DX;
}

//=================================================================================================

const Dir3d& gp::DY()
{
  static Dir3d gp_DY(0, 1, 0);
  return gp_DY;
}

//=================================================================================================

const Dir3d& gp::DZ()
{
  static Dir3d gp_DZ(0, 0, 1);
  return gp_DZ;
}

//=================================================================================================

const Axis3d& gp::OX()
{
  static Axis3d gp_OX(Point3d(0, 0, 0), Dir3d(1, 0, 0));
  return gp_OX;
}

//=================================================================================================

const Axis3d& gp::OY()
{
  static Axis3d gp_OY(Point3d(0, 0, 0), Dir3d(0, 1, 0));
  return gp_OY;
}

//=================================================================================================

const Axis3d& gp::OZ()
{
  static Axis3d gp_OZ(Point3d(0, 0, 0), Dir3d(0, 0, 1));
  return gp_OZ;
}

//=================================================================================================

const Frame3d& gp::XOY()
{
  static Frame3d gp_XOY(Point3d(0, 0, 0), Dir3d(0, 0, 1), Dir3d(1, 0, 0));
  return gp_XOY;
}

//=================================================================================================

const Frame3d& gp::ZOX()
{
  static Frame3d gp_ZOX(Point3d(0, 0, 0), Dir3d(0, 1, 0), Dir3d(0, 0, 1));
  return gp_ZOX;
}

//=================================================================================================

const Frame3d& gp::YOZ()
{
  static Frame3d gp_YOZ(Point3d(0, 0, 0), Dir3d(1, 0, 0), Dir3d(0, 1, 0));
  return gp_YOZ;
}

//=================================================================================================

const gp_Pnt2d& gp::Origin2d()
{
  static gp_Pnt2d gp_Origin2d(0, 0);
  return gp_Origin2d;
}

//=================================================================================================

const gp_Dir2d& gp::DX2d()
{
  static gp_Dir2d gp_DX2d(1, 0);
  return gp_DX2d;
}

//=================================================================================================

const gp_Dir2d& gp::DY2d()
{
  static gp_Dir2d gp_DY2d(0, 1);
  return gp_DY2d;
}

//=================================================================================================

const gp_Ax2d& gp::OX2d()
{
  static gp_Ax2d gp_OX2d(gp_Pnt2d(0, 0), gp_Dir2d(1, 0));
  return gp_OX2d;
}

//=================================================================================================

const gp_Ax2d& gp::OY2d()
{
  static gp_Ax2d gp_OY2d(gp_Pnt2d(0, 0), gp_Dir2d(0, 1));
  return gp_OY2d;
}
