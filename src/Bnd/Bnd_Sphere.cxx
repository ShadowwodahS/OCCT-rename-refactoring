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

#include <Bnd_Sphere.hxx>

Sphere2::Sphere2()
    : myCenter(0., 0., 0.),
      myRadius(0.),
      myIsValid(Standard_False),
      myU(0),
      myV(0)
{
}

Sphere2::Sphere2(const Coords3d&          theCenter,
                       const Standard_Real    theRadius,
                       const Standard_Integer theU,
                       const Standard_Integer theV)
    : myCenter(theCenter),
      myRadius(theRadius),
      myIsValid(Standard_False),
      myU(theU),
      myV(theV)
{
}

void Sphere2::SquareDistances(const Coords3d&  theXYZ,
                                 Standard_Real& theMin,
                                 Standard_Real& theMax) const
{
  theMax = (theXYZ - myCenter).SquareModulus();
  theMin = (theMax - myRadius < 0 ? 0.0 : theMax - myRadius * myRadius);
  theMax += myRadius * myRadius;
}

void Sphere2::Distances(const Coords3d& theXYZ, Standard_Real& theMin, Standard_Real& theMax) const
{
  theMax = (theXYZ - myCenter).Modulus();
  theMin = (theMax - myRadius < 0 ? 0.0 : theMax - myRadius);
  theMax += myRadius;
}

Standard_Boolean Sphere2::Project(const Coords3d&     theNode,
                                     Coords3d&           theProjNode,
                                     Standard_Real&    theDist,
                                     Standard_Boolean& theInside) const
{
  theProjNode = myCenter;
  theDist     = (theNode - theProjNode).Modulus();
  theInside   = Standard_True;
  return Standard_True;
}

Standard_Real Sphere2::Distance(const Coords3d& theNode) const
{
  return (theNode - myCenter).Modulus();
}

Standard_Real Sphere2::SquareDistance(const Coords3d& theNode) const
{
  return (theNode - myCenter).SquareModulus();
}

void Sphere2::Add(const Sphere2& theOther)
{
  if (myRadius < 0.0)
  {
    // not initialised yet
    *this = theOther;
    return;
  }

  const Standard_Real aDist = (myCenter - theOther.myCenter).Modulus();
  if (myRadius + aDist <= theOther.myRadius)
  {
    // the other sphere is larger and encloses this
    *this = theOther;
    return;
  }

  if (theOther.myRadius + aDist <= myRadius)
    return; // this sphere encloses other

  // expansion
  const Standard_Real dfR          = (aDist + myRadius + theOther.myRadius) * 0.5;
  const Standard_Real aParamOnDiam = (dfR - myRadius) / aDist;
  myCenter  = myCenter * (1.0 - aParamOnDiam) + theOther.myCenter * aParamOnDiam;
  myRadius  = dfR;
  myIsValid = Standard_False;
}

Standard_Boolean Sphere2::IsOut(const Sphere2& theOther) const
{
  return (myCenter - theOther.myCenter).SquareModulus()
         > (myRadius + theOther.myRadius) * (myRadius + theOther.myRadius);
}

Standard_Boolean Sphere2::IsOut(const Coords3d& theXYZ, Standard_Real& theMaxDist) const
{
  Standard_Real aCurMinDist, aCurMaxDist;
  Distances(theXYZ, aCurMinDist, aCurMaxDist);
  if (aCurMinDist > theMaxDist)
    return Standard_True;
  if (myIsValid && aCurMaxDist < theMaxDist)
    theMaxDist = aCurMaxDist;
  return Standard_False;
}

Standard_Real Sphere2::SquareExtent() const
{
  return 4 * myRadius * myRadius;
}
