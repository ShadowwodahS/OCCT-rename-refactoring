// Created on: 1993-03-09
// Created by: JCV
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

#include <Geom_Axis2Placement.hxx>
#include <Geom_Geometry.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Geom_Axis2Placement, Geom_AxisPlacement)

typedef Geom_Axis2Placement Axis2Placement;
typedef Axis3d              Ax1;
typedef Dir3d              Dir;
typedef Point3d              Pnt;
typedef Transform3d             Trsf;
typedef Vector3d              Vec;

//=================================================================================================

Handle(Geom_Geometry) Geom_Axis2Placement::Copy() const
{

  Handle(Geom_Axis2Placement) A2;
  A2 = new Axis2Placement(axis.Location(), axis.Direction(), vxdir, vydir);
  return A2;
}

//=================================================================================================

Geom_Axis2Placement::Geom_Axis2Placement(const Frame3d& A2)
{

  vxdir = A2.XDirection();
  vydir = A2.YDirection();
  axis  = A2.Axis();
}

//=================================================================================================

Geom_Axis2Placement::Geom_Axis2Placement(

  const Point3d& P,
  const Dir3d& N,
  const Dir3d& Vx)
{

  axis  = Axis3d(P, N);
  vxdir = N.CrossCrossed(Vx, N);
  vydir = N.Crossed(vxdir);
}

//=================================================================================================

Geom_Axis2Placement::Geom_Axis2Placement(

  const Point3d& P,
  const Dir3d& Vz,
  const Dir3d& Vx,
  const Dir3d& Vy

  )
    : vxdir(Vx),
      vydir(Vy)
{

  axis.SetLocation(P);
  axis.SetDirection(Vz);
}

//=================================================================================================

const Dir3d& Geom_Axis2Placement::XDirection() const
{
  return vxdir;
}

//=================================================================================================

const Dir3d& Geom_Axis2Placement::YDirection() const
{
  return vydir;
}

//=================================================================================================

void Geom_Axis2Placement::SetAx2(const Frame3d& A2)
{

  vxdir = A2.XDirection();
  vydir = A2.YDirection();
  axis  = A2.Axis();
}

//=================================================================================================

void Geom_Axis2Placement::SetDirection(const Dir3d& V)
{

  axis.SetDirection(V);
  vxdir = V.CrossCrossed(vxdir, V);
  vydir = V.Crossed(vxdir);
}

//=================================================================================================

void Geom_Axis2Placement::SetXDirection(const Dir3d& Vx)
{

  vxdir = axis.Direction().CrossCrossed(Vx, axis.Direction());
  vydir = axis.Direction().Crossed(vxdir);
}

//=================================================================================================

void Geom_Axis2Placement::SetYDirection(const Dir3d& Vy)
{

  vxdir = Vy.Crossed(axis.Direction());
  vydir = (axis.Direction()).Crossed(vxdir);
}

//=================================================================================================

Frame3d Geom_Axis2Placement::Ax2() const
{

  return Frame3d(axis.Location(), axis.Direction(), vxdir);
}

//=================================================================================================

void Geom_Axis2Placement::Transform(const Transform3d& T)
{

  // axis.Location().Transform (T);
  axis.SetLocation(axis.Location().Transformed(T)); // 10-03-93
  vxdir.Transform(T);
  vydir.Transform(T);
  axis.SetDirection(vxdir.Crossed(vydir));
}
