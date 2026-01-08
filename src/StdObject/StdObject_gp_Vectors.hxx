// Copyright (c) 2015 OPEN CASCADE SAS
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

#ifndef _StdObject_gp_Vectors_HeaderFile
#define _StdObject_gp_Vectors_HeaderFile

#include <StdObjMgt_ReadData.hxx>
#include <StdObjMgt_WriteData.hxx>

#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>

inline ReadData& operator>>(ReadData& theReadData, Coords2d& theXY)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Standard_Real aX, aY;
  theReadData >> aX >> aY;
  theXY.SetCoord(aX, aY);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Coords2d& theXY)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  Standard_Real aX = theXY.X(), aY = theXY.Y();
  theWriteData << aX << aY;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Pnt2d& thePnt)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Coords2d aXY;
  theReadData >> aXY;
  thePnt.SetXY(aXY);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Pnt2d& thePnt)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  theWriteData << thePnt.XY();
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Vec2d& theVec)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Coords2d aXY;
  theReadData >> aXY;
  theVec.SetXY(aXY);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Vec2d& theVec)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  theWriteData << theVec.XY();
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Dir2d& theDir)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Coords2d aXY;
  theReadData >> aXY;
  theDir.SetXY(aXY);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Dir2d& theDir)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  theWriteData << theDir.XY();
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Coords3d& theXYZ)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Standard_Real aX, aY, aZ;
  theReadData >> aX >> aY >> aZ;
  theXYZ.SetCoord(aX, aY, aZ);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Coords3d& theXYZ)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  Standard_Real aX = theXYZ.X(), aY = theXYZ.Y(), aZ = theXYZ.Z();
  theWriteData << aX << aY << aZ;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Point3d& thePnt)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Coords3d aXYZ;
  theReadData >> aXYZ;
  thePnt.SetXYZ(aXYZ);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Point3d& thePnt)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  theWriteData << thePnt.XYZ();
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Vector3d& theVec)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Coords3d aXYZ;
  theReadData >> aXYZ;
  theVec.SetXYZ(aXYZ);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Vector3d& theVec)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  theWriteData << theVec.XYZ();
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Dir3d& theDir)
{
  ReadData::ObjectSentry aSentry(theReadData);

  Coords3d aXYZ;
  theReadData >> aXYZ;
  theDir.SetXYZ(aXYZ);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Dir3d& theDir)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  theWriteData << theDir.XYZ();
  return theWriteData;
}

#endif
