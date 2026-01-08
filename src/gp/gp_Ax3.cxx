// Created on: 1993-08-03
// Created by: Laurent BOURESCHE
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

#include <gp_Ax3.hxx>

#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_Dump.hxx>

//=================================================================================================

Ax3::Ax3(const Point3d& P, const Dir3d& V)
    : axis(P, V)
{
  Standard_Real A    = V.X();
  Standard_Real B    = V.Y();
  Standard_Real C    = V.Z();
  Standard_Real Aabs = A;
  if (Aabs < 0)
    Aabs = -Aabs;
  Standard_Real Babs = B;
  if (Babs < 0)
    Babs = -Babs;
  Standard_Real Cabs = C;
  if (Cabs < 0)
    Cabs = -Cabs;
  Dir3d D;

  //  pour determiner l axe X :
  //  on dit que le produit scalaire Vx.V = 0.
  //  et on recherche le max(A,B,C) pour faire la division.
  //  l une des coordonnees du vecteur est nulle.

  if (Babs <= Aabs && Babs <= Cabs)
  {
    if (Aabs > Cabs)
      D.SetCoord(-C, 0., A);
    else
      D.SetCoord(C, 0., -A);
  }
  else if (Aabs <= Babs && Aabs <= Cabs)
  {
    if (Babs > Cabs)
      D.SetCoord(0., -C, B);
    else
      D.SetCoord(0., C, -B);
  }
  else
  {
    if (Aabs > Babs)
      D.SetCoord(-B, A, 0.);
    else
      D.SetCoord(B, -A, 0.);
  }
  vxdir = D;
  vydir = V.Crossed(vxdir);
}

void Ax3::Mirror(const Point3d& P)
{
  axis.Mirror(P);
  vxdir.Reverse();
  vydir.Reverse();
}

Ax3 Ax3::Mirrored(const Point3d& P) const
{
  Ax3 Temp = *this;
  Temp.Mirror(P);
  return Temp;
}

void Ax3::Mirror(const Axis3d& A1)
{
  vydir.Mirror(A1);
  vxdir.Mirror(A1);
  axis.Mirror(A1);
}

Ax3 Ax3::Mirrored(const Axis3d& A1) const
{
  Ax3 Temp = *this;
  Temp.Mirror(A1);
  return Temp;
}

void Ax3::Mirror(const Frame3d& A2)
{
  vydir.Mirror(A2);
  vxdir.Mirror(A2);
  axis.Mirror(A2);
}

Ax3 Ax3::Mirrored(const Frame3d& A2) const
{
  Ax3 Temp = *this;
  Temp.Mirror(A2);
  return Temp;
}

void Ax3::DumpJson(Standard_OStream& theOStream, Standard_Integer) const {
  OCCT_DUMP_VECTOR_CLASS(theOStream, "Location", 3, Location().X(), Location().Y(), Location().Z())
    OCCT_DUMP_VECTOR_CLASS(theOStream,
                           "Direction",
                           3,
                           Direction().X(),
                           Direction().Y(),
                           Direction().Z())

      OCCT_DUMP_VECTOR_CLASS(theOStream,
                             "XDirection",
                             3,
                             XDirection().X(),
                             XDirection().Y(),
                             XDirection().Z()) OCCT_DUMP_VECTOR_CLASS(theOStream,
                                                                      "YDirection",
                                                                      3,
                                                                      YDirection().X(),
                                                                      YDirection().Y(),
                                                                      YDirection().Z())}

Standard_Boolean Ax3::InitFromJson(const Standard_SStream& theSStream,
                                      Standard_Integer&       theStreamPos)
{
  Standard_Integer        aPos       = theStreamPos;
  AsciiString1 aStreamStr = Standard_Dump::Text(theSStream);

  Coords3d anXYZLoc;
  OCCT_INIT_VECTOR_CLASS(aStreamStr,
                         "Location",
                         aPos,
                         3,
                         &anXYZLoc.ChangeCoord(1),
                         &anXYZLoc.ChangeCoord(2),
                         &anXYZLoc.ChangeCoord(3))
  SetLocation(anXYZLoc);

  Coords3d aDir;
  OCCT_INIT_VECTOR_CLASS(aStreamStr,
                         "Direction",
                         aPos,
                         3,
                         &aDir.ChangeCoord(1),
                         &aDir.ChangeCoord(2),
                         &aDir.ChangeCoord(3))
  Coords3d aXDir;
  OCCT_INIT_VECTOR_CLASS(aStreamStr,
                         "XDirection",
                         aPos,
                         3,
                         &aXDir.ChangeCoord(1),
                         &aXDir.ChangeCoord(2),
                         &aXDir.ChangeCoord(3))
  Coords3d anYDir;
  OCCT_INIT_VECTOR_CLASS(aStreamStr,
                         "YDirection",
                         aPos,
                         3,
                         &anYDir.ChangeCoord(1),
                         &anYDir.ChangeCoord(2),
                         &anYDir.ChangeCoord(3))

  axis.SetDirection(Dir3d(aDir));
  vxdir = Dir3d(aXDir);
  vydir = Dir3d(anYDir);

  if (!Direction().IsEqual(aDir, Precision::Angular()))
    return Standard_False;

  theStreamPos = aPos;
  return Standard_True;
}
