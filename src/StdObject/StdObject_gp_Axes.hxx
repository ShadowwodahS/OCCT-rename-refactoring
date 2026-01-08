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

#ifndef _StdObject_gp_Axes_HeaderFile
#define _StdObject_gp_Axes_HeaderFile

#include <StdObject_gp_Vectors.hxx>

#include <gp_Ax2d.hxx>
#include <gp_Ax22d.hxx>
#include <gp_Ax3.hxx>

inline ReadData& operator>>(ReadData& theReadData, gp_Ax2d& theAx)
{
  ReadData::ObjectSentry aSentry(theReadData);
  gp_Pnt2d                         aLoc;
  gp_Dir2d                         aDir;
  theReadData >> aLoc >> aDir;
  theAx = gp_Ax2d(aLoc, aDir);
  return theReadData;
}

inline WriteData& write(WriteData& theWriteData, const gp_Ax2d& theAx)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  const gp_Pnt2d& aLoc = theAx.Location();
  const gp_Dir2d& aDir = theAx.Direction();
  theWriteData << aLoc << aDir;
  return theWriteData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Ax2d& theAx)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  const gp_Pnt2d& aLoc = theAx.Location();
  const gp_Dir2d& aDir = theAx.Direction();
  theWriteData << aLoc << aDir;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Ax22d& theAx)
{
  ReadData::ObjectSentry aSentry(theReadData);
  gp_Pnt2d                         aLoc;
  gp_Dir2d                         aYDir, aXDir;
  theReadData >> aLoc >> aYDir >> aXDir;
  theAx = Ax22d(aLoc, aXDir, aYDir);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Ax22d& theAx)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  const gp_Pnt2d& aLoc  = theAx.Location();
  const gp_Dir2d& aYDir = theAx.YDirection();
  const gp_Dir2d& aXDir = theAx.XDirection();
  theWriteData << aLoc << aYDir << aXDir;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Axis3d& theAx)
{
  ReadData::ObjectSentry aSentry(theReadData);
  Point3d                           aLoc;
  Dir3d                           aDir;
  theReadData >> aLoc >> aDir;
  theAx = Axis3d(aLoc, aDir);
  return theReadData;
}

inline WriteData& write(WriteData& theWriteData, const Axis3d& theAx)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  const Point3d& aLoc = theAx.Location();
  const Dir3d& aDir = theAx.Direction();
  theWriteData << aLoc << aDir;
  return theWriteData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Axis3d& theAx)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  const Point3d& aLoc = theAx.Location();
  const Dir3d& aDir = theAx.Direction();
  theWriteData << aLoc << aDir;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Frame3d& theAx)
{
  ReadData::ObjectSentry aSentry(theReadData);
  Axis3d                           anAx;
  Dir3d                           aYDir, aXDir;
  theReadData >> anAx >> aYDir >> aXDir;
  theAx = Frame3d(anAx.Location(), anAx.Direction(), aXDir);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Frame3d& theAx)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  const Axis3d& anAx  = theAx.Axis();
  const Dir3d& aYDir = theAx.YDirection();
  const Dir3d& aXDir = theAx.XDirection();
  theWriteData << anAx << aYDir << aXDir;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, Ax3& theAx)
{
  ReadData::ObjectSentry aSentry(theReadData);
  Axis3d                           anAx;
  Dir3d                           aYDir, aXDir;
  theReadData >> anAx >> aYDir >> aXDir;
  theAx = Ax3(anAx.Location(), anAx.Direction(), aXDir);
  if (aYDir * theAx.YDirection() < 0.)
    theAx.YReverse();
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const Ax3& theAx)
{
  WriteData::ObjectSentry aSentry(theWriteData);

  const Axis3d& anAx  = theAx.Axis();
  const Dir3d& aYDir = theAx.YDirection();
  const Dir3d& aXDir = theAx.XDirection();
  theWriteData << anAx << aYDir << aXDir;
  return theWriteData;
}

#endif
