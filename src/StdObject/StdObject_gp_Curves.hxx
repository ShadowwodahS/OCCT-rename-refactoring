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

#ifndef _StdObject_gp_Curves_HeaderFile
#define _StdObject_gp_Curves_HeaderFile

#include <StdObject_gp_Axes.hxx>

#include <gp_Lin2d.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Hypr2d.hxx>
#include <gp_Parab2d.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Parab.hxx>

inline ReadData& operator>>(ReadData& theReadData, gp_Lin2d& theLin)
{
  gp_Ax2d anAx;
  theReadData >> anAx;
  theLin.SetPosition(anAx);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Lin2d& theLin)
{
  const gp_Ax2d& anAx = theLin.Position1();
  write(theWriteData, anAx);
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Circ2d& theCirc)
{
  Ax22d      anAx;
  Standard_Real aRadius;

  theReadData >> anAx >> aRadius;

  theCirc.SetAxis(anAx);
  theCirc.SetRadius(aRadius);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Circ2d& theCirc)
{
  const Ax22d& anAx    = theCirc.Position1();
  Standard_Real   aRadius = theCirc.Radius();
  theWriteData << anAx << aRadius;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Elips2d& theElips)
{
  Ax22d      anAx;
  Standard_Real aMajorRadius, aMinorRadius;

  theReadData >> anAx >> aMajorRadius >> aMinorRadius;

  theElips.SetAxis(anAx);
  theElips.SetMajorRadius(aMajorRadius);
  theElips.SetMinorRadius(aMinorRadius);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData,
                                       const gp_Elips2d&    theElips)
{
  const Ax22d& anAx         = theElips.Axis();
  Standard_Real   aMajorRadius = theElips.MajorRadius();
  Standard_Real   aMinorRadius = theElips.MinorRadius();
  theWriteData << anAx << aMajorRadius << aMinorRadius;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Hypr2d& theHypr)
{
  Ax22d      anAx;
  Standard_Real aMajorRadius, aMinorRadius;

  theReadData >> anAx >> aMajorRadius >> aMinorRadius;

  theHypr.SetAxis(anAx);
  theHypr.SetMajorRadius(aMajorRadius);
  theHypr.SetMinorRadius(aMinorRadius);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Hypr2d& theHypr)
{
  const Ax22d& anAx         = theHypr.Axis();
  Standard_Real   aMajorRadius = theHypr.MajorRadius();
  Standard_Real   aMinorRadius = theHypr.MinorRadius();
  theWriteData << anAx << aMajorRadius << aMinorRadius;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Parab2d& theParab)
{
  Ax22d      anAx;
  Standard_Real aFocalLength;

  theReadData >> anAx >> aFocalLength;

  theParab.SetAxis(anAx);
  theParab.SetFocal(aFocalLength);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData,
                                       const gp_Parab2d&    theParab)
{
  const Ax22d& anAx         = theParab.Axis();
  Standard_Real   aFocalLength = theParab.Focal();
  theWriteData << anAx << aFocalLength;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Lin& theLin)
{
  Axis3d anAx;
  theReadData >> anAx;
  theLin.SetPosition(anAx);
  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Lin& theLin)
{
  const Axis3d& anAx = theLin.Position1();
  write(theWriteData, anAx);
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Circ& theCirc)
{
  Frame3d        anAx;
  Standard_Real aRadius;

  theReadData >> anAx >> aRadius;

  theCirc.SetPosition(anAx);
  theCirc.SetRadius(aRadius);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Circ& theCirc)
{
  const Frame3d& anAx    = theCirc.Position1();
  Standard_Real aRadius = theCirc.Radius();
  theWriteData << anAx << aRadius;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Elips& theElips)
{
  Frame3d        anAx;
  Standard_Real aMajorRadius, aMinorRadius;

  theReadData >> anAx >> aMajorRadius >> aMinorRadius;

  theElips.SetPosition(anAx);
  theElips.SetMajorRadius(aMajorRadius);
  theElips.SetMinorRadius(aMinorRadius);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Elips& theElips)
{
  const Frame3d& anAx         = theElips.Position1();
  Standard_Real aMajorRadius = theElips.MajorRadius();
  Standard_Real aMinorRadius = theElips.MinorRadius();
  theWriteData << anAx << aMajorRadius << aMinorRadius;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Hypr& theHypr)
{
  Frame3d        anAx;
  Standard_Real aMajorRadius, aMinorRadius;

  theReadData >> anAx >> aMajorRadius >> aMinorRadius;

  theHypr.SetPosition(anAx);
  theHypr.SetMajorRadius(aMajorRadius);
  theHypr.SetMinorRadius(aMinorRadius);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Hypr& theHypr)
{
  const Frame3d& anAx         = theHypr.Position1();
  Standard_Real aMajorRadius = theHypr.MajorRadius();
  Standard_Real aMinorRadius = theHypr.MinorRadius();
  theWriteData << anAx << aMajorRadius << aMinorRadius;
  return theWriteData;
}

inline ReadData& operator>>(ReadData& theReadData, gp_Parab& theParab)
{
  Frame3d        anAx;
  Standard_Real aFocalLength;

  theReadData >> anAx >> aFocalLength;

  theParab.SetPosition(anAx);
  theParab.SetFocal(aFocalLength);

  return theReadData;
}

inline WriteData& operator<<(WriteData& theWriteData, const gp_Parab& theParab)
{
  const Frame3d& anAx         = theParab.Position1();
  Standard_Real aFocalLength = theParab.Focal();
  theWriteData << anAx << aFocalLength;
  return theWriteData;
}

#endif
