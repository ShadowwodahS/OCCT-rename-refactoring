// Copyright (c) 1991-1999 Matra Datavision
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

#ifndef _gp_Dir_HeaderFile
#define _gp_Dir_HeaderFile

#include <gp_XYZ.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_DomainError.hxx>
#include <Standard_OutOfRange.hxx>

class Vector3d;
class Axis3d;
class Frame3d;
class Transform3d;

//! Describes a unit vector in 3D space. This unit vector is also called "Direction".
//! See Also
//! DirectionBuilder which provides functions for more complex1
//! unit vector constructions
//! Geom_Direction which provides additional functions for
//! constructing unit vectors and works, in particular, with the
//! parametric equations of unit vectors.
class Dir3d
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates a direction corresponding to X axis.
  Dir3d()
      : coord(1., 0., 0.)
  {
  }

  //! Normalizes the vector theV and creates a direction. Raises ConstructionError if
  //! theV.Magnitude() <= Resolution.
  Dir3d(const Vector3d& theV);

  //! Creates a direction from a triplet of coordinates. Raises ConstructionError if
  //! theCoord.Modulus() <= Resolution from gp1.
  Dir3d(const Coords3d& theCoord);

  //! Creates a direction with its 3 cartesian coordinates. Raises ConstructionError if
  //! Sqrt(theXv*theXv + theYv*theYv + theZv*theZv) <= Resolution Modification of the direction's
  //! coordinates If Sqrt (theXv*theXv + theYv*theYv + theZv*theZv) <= Resolution from gp1 where
  //! theXv, theYv ,theZv are the new coordinates it is not possible to
  //! construct the direction and the method raises the
  //! exception ConstructionError.
  Dir3d(const Standard_Real theXv, const Standard_Real theYv, const Standard_Real theZv);

  //! For this unit vector,  assigns the value Xi to:
  //! -   the X coordinate if theIndex is 1, or
  //! -   the Y coordinate if theIndex is 2, or
  //! -   the Z coordinate if theIndex is 3,
  //! and then normalizes it.
  //! Warning
  //! Remember that all the coordinates of a unit vector are
  //! implicitly modified when any single one is changed directly.
  //! Exceptions
  //! Standard_OutOfRange if theIndex is not 1, 2, or 3.
  //! Standard_ConstructionError if either of the following
  //! is less than or equal to gp1::Resolution():
  //! -   Sqrt(Xv*Xv + Yv*Yv + Zv*Zv), or
  //! -   the modulus of the number triple formed by the new
  //! value theXi and the two other coordinates of this vector
  //! that were not directly modified.
  void SetCoord(const Standard_Integer theIndex, const Standard_Real theXi);

  //! For this unit vector,  assigns the values theXv, theYv and theZv to its three coordinates.
  //! Remember that all the coordinates of a unit vector are
  //! implicitly modified when any single one is changed directly.
  void SetCoord(const Standard_Real theXv, const Standard_Real theYv, const Standard_Real theZv);

  //! Assigns the given value to the X coordinate of this   unit vector.
  void SetX(const Standard_Real theX);

  //! Assigns the given value to the Y coordinate of this   unit vector.
  void SetY(const Standard_Real theY);

  //! Assigns the given value to the Z  coordinate of this   unit vector.
  void SetZ(const Standard_Real theZ);

  //! Assigns the three coordinates of theCoord to this unit vector.
  void SetXYZ(const Coords3d& theCoord);

  //! Returns the coordinate of range theIndex :
  //! theIndex = 1 => X is returned
  //! Ithendex = 2 => Y is returned
  //! theIndex = 3 => Z is returned
  //! Exceptions
  //! Standard_OutOfRange if theIndex is not 1, 2, or 3.
  Standard_Real Coord(const Standard_Integer theIndex) const { return coord.Coord(theIndex); }

  //! Returns for the  unit vector  its three coordinates theXv, theYv, and theZv.
  void Coord(Standard_Real& theXv, Standard_Real& theYv, Standard_Real& theZv) const
  {
    coord.Coord(theXv, theYv, theZv);
  }

  //! Returns the X coordinate for a  unit vector.
  Standard_Real X() const { return coord.X(); }

  //! Returns the Y coordinate for a  unit vector.
  Standard_Real Y() const { return coord.Y(); }

  //! Returns the Z coordinate for a  unit vector.
  Standard_Real Z() const { return coord.Z(); }

  //! for this unit vector, returns  its three coordinates as a number triplea.
  const Coords3d& XYZ() const { return coord; }

  //! Returns True if the angle between the two directions is
  //! lower or equal to theAngularTolerance.
  Standard_Boolean IsEqual(const Dir3d& theOther, const Standard_Real theAngularTolerance) const
  {
    return Angle(theOther) <= theAngularTolerance;
  }

  //! Returns True if  the angle between this unit vector and the unit vector theOther is equal to
  //! Pi/2 (normal).
  Standard_Boolean IsNormal(const Dir3d& theOther, const Standard_Real theAngularTolerance) const
  {
    Standard_Real anAng = M_PI / 2.0 - Angle(theOther);
    if (anAng < 0)
    {
      anAng = -anAng;
    }
    return anAng <= theAngularTolerance;
  }

  //! Returns True if  the angle between this unit vector and the unit vector theOther is equal to
  //! Pi (opposite).
  Standard_Boolean IsOpposite(const Dir3d& theOther, const Standard_Real theAngularTolerance) const
  {
    return M_PI - Angle(theOther) <= theAngularTolerance;
  }

  //! Returns true if the angle between this unit vector and the
  //! unit vector theOther is equal to 0 or to Pi.
  //! Note: the tolerance criterion is given by theAngularTolerance.
  Standard_Boolean IsParallel(const Dir3d& theOther, const Standard_Real theAngularTolerance) const
  {
    Standard_Real anAng = Angle(theOther);
    return anAng <= theAngularTolerance || M_PI - anAng <= theAngularTolerance;
  }

  //! Computes the angular value in radians between <me> and
  //! <theOther>. This value is always positive in 3D space.
  //! Returns the angle in the range [0, PI]
  Standard_EXPORT Standard_Real Angle(const Dir3d& theOther) const;

  //! Computes the angular value between <me> and <theOther>.
  //! <theVRef> is the direction of reference normal to <me> and <theOther>
  //! and its orientation gives the positive sense of rotation.
  //! If the cross product <me> ^ <theOther> has the same orientation
  //! as <theVRef> the angular value is positive else negative.
  //! Returns the angular value in the range -PI and PI (in radians). Raises  DomainError if <me>
  //! and <theOther> are not parallel this exception is raised when <theVRef> is in the same plane
  //! as <me> and <theOther> The tolerance criterion is Resolution from package gp1.
  Standard_EXPORT Standard_Real AngleWithRef(const Dir3d& theOther, const Dir3d& theVRef) const;

  //! Computes the cross product between two directions
  //! Raises the exception ConstructionError if the two directions
  //! are parallel because the computed vector cannot be normalized
  //! to create a direction.
  void Cross(const Dir3d& theRight);

  void operator^=(const Dir3d& theRight) { Cross(theRight); }

  //! Computes the triple vector product.
  //! <me> ^ (V1 ^ V2)
  //! Raises the exception ConstructionError if V1 and V2 are parallel
  //! or <me> and (V1^V2) are parallel because the computed vector
  //! can't be normalized to create a direction.
  Standard_NODISCARD Dir3d Crossed(const Dir3d& theRight) const;

  Standard_NODISCARD Dir3d operator^(const Dir3d& theRight) const { return Crossed(theRight); }

  void CrossCross(const Dir3d& theV1, const Dir3d& theV2);

  //! Computes the double vector product this ^ (theV1 ^ theV2).
  //! -   CrossCrossed creates a new unit vector.
  //! Exceptions
  //! Standard_ConstructionError if:
  //! -   theV1 and theV2 are parallel, or
  //! -   this unit vector and (theV1 ^ theV2) are parallel.
  //! This is because, in these conditions, the computed vector
  //! is null and cannot be normalized.
  Standard_NODISCARD Dir3d CrossCrossed(const Dir3d& theV1, const Dir3d& theV2) const;

  //! Computes the scalar product
  Standard_Real Dot(const Dir3d& theOther) const { return coord.Dot(theOther.coord); }

  Standard_Real operator*(const Dir3d& theOther) const { return Dot(theOther); }

  //! Computes the triple scalar product <me> * (theV1 ^ theV2).
  //! Warnings :
  //! The computed vector theV1' = theV1 ^ theV2 is not normalized
  //! to create a unitary vector. So this method never
  //! raises an exception even if theV1 and theV2 are parallel.
  Standard_Real DotCross(const Dir3d& theV1, const Dir3d& theV2) const
  {
    return coord.Dot(theV1.coord.Crossed(theV2.coord));
  }

  void Reverse() { coord.Reverse(); }

  //! Reverses the orientation of a direction
  //! geometric transformations
  //! Performs the symmetrical transformation of a direction
  //! with respect to the direction V which is the center of
  //! the  symmetry.]
  Standard_NODISCARD Dir3d Reversed() const
  {
    Dir3d aV = *this;
    aV.coord.Reverse();
    return aV;
  }

  Standard_NODISCARD Dir3d operator-() const { return Reversed(); }

  Standard_EXPORT void Mirror(const Dir3d& theV);

  //! Performs the symmetrical transformation of a direction
  //! with respect to the direction theV which is the center of
  //! the  symmetry.
  Standard_NODISCARD Standard_EXPORT Dir3d Mirrored(const Dir3d& theV) const;

  Standard_EXPORT void Mirror(const Axis3d& theA1);

  //! Performs the symmetrical transformation of a direction
  //! with respect to an axis placement which is the axis
  //! of the symmetry.
  Standard_NODISCARD Standard_EXPORT Dir3d Mirrored(const Axis3d& theA1) const;

  Standard_EXPORT void Mirror(const Frame3d& theA2);

  //! Performs the symmetrical transformation of a direction
  //! with respect to a plane. The axis placement theA2 locates
  //! the plane of the symmetry : (Location, XDirection, YDirection).
  Standard_NODISCARD Standard_EXPORT Dir3d Mirrored(const Frame3d& theA2) const;

  void Rotate(const Axis3d& theA1, const Standard_Real theAng);

  //! Rotates a direction. theA1 is the axis of the rotation.
  //! theAng is the angular value of the rotation in radians.
  Standard_NODISCARD Dir3d Rotated(const Axis3d& theA1, const Standard_Real theAng) const
  {
    Dir3d aV = *this;
    aV.Rotate(theA1, theAng);
    return aV;
  }

  Standard_EXPORT void Transform(const Transform3d& theT);

  //! Transforms a direction with a "Trsf" from gp1.
  //! Warnings :
  //! If the scale factor of the "Trsf" theT is negative then the
  //! direction <me> is reversed.
  Standard_NODISCARD Dir3d Transformed(const Transform3d& theT) const
  {
    Dir3d aV = *this;
    aV.Transform(theT);
    return aV;
  }

  //! Dumps the content of me into the stream
  Standard_EXPORT void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const;

  //! Inits the content of me from the stream
  Standard_EXPORT Standard_Boolean InitFromJson(const Standard_SStream& theSStream,
                                                Standard_Integer&       theStreamPos);

private:
  Coords3d coord;
};

#include <gp_Trsf.hxx>

// =======================================================================
// function : Dir3d
// purpose  :
// =======================================================================
inline Dir3d::Dir3d(const Vector3d& theV)
{
  const Coords3d& aXYZ = theV.XYZ();
  Standard_Real aX   = aXYZ.X();
  Standard_Real aY   = aXYZ.Y();
  Standard_Real aZ   = aXYZ.Z();
  Standard_Real aD   = sqrt(aX * aX + aY * aY + aZ * aZ);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d() - input vector has zero norm");
  coord.SetX(aX / aD);
  coord.SetY(aY / aD);
  coord.SetZ(aZ / aD);
}

// =======================================================================
// function : Dir3d
// purpose  :
// =======================================================================
inline Dir3d::Dir3d(const Coords3d& theXYZ)
{
  Standard_Real aX = theXYZ.X();
  Standard_Real aY = theXYZ.Y();
  Standard_Real aZ = theXYZ.Z();
  Standard_Real aD = sqrt(aX * aX + aY * aY + aZ * aZ);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d() - input vector has zero norm");
  coord.SetX(aX / aD);
  coord.SetY(aY / aD);
  coord.SetZ(aZ / aD);
}

// =======================================================================
// function : Dir3d
// purpose  :
// =======================================================================
inline Dir3d::Dir3d(const Standard_Real theXv,
                      const Standard_Real theYv,
                      const Standard_Real theZv)
{
  Standard_Real aD = sqrt(theXv * theXv + theYv * theYv + theZv * theZv);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d() - input vector has zero norm");
  coord.SetX(theXv / aD);
  coord.SetY(theYv / aD);
  coord.SetZ(theZv / aD);
}

// =======================================================================
// function : SetCoord
// purpose  :
// =======================================================================
inline void Dir3d::SetCoord(const Standard_Integer theIndex, const Standard_Real theXi)
{
  Standard_Real aX = coord.X();
  Standard_Real aY = coord.Y();
  Standard_Real aZ = coord.Z();
  Standard_OutOfRange_Raise_if(theIndex < 1 || theIndex > 3,
                               "Dir3d::SetCoord() - index is out of range [1, 3]");
  if (theIndex == 1)
  {
    aX = theXi;
  }
  else if (theIndex == 2)
  {
    aY = theXi;
  }
  else
  {
    aZ = theXi;
  }
  Standard_Real aD = sqrt(aX * aX + aY * aY + aZ * aZ);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::SetCoord() - result vector has zero norm");
  coord.SetX(aX / aD);
  coord.SetY(aY / aD);
  coord.SetZ(aZ / aD);
}

// =======================================================================
// function : SetCoord
// purpose  :
// =======================================================================
inline void Dir3d::SetCoord(const Standard_Real theXv,
                             const Standard_Real theYv,
                             const Standard_Real theZv)
{
  Standard_Real aD = sqrt(theXv * theXv + theYv * theYv + theZv * theZv);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::SetCoord() - input vector has zero norm");
  coord.SetX(theXv / aD);
  coord.SetY(theYv / aD);
  coord.SetZ(theZv / aD);
}

// =======================================================================
// function : SetX
// purpose  :
// =======================================================================
inline void Dir3d::SetX(const Standard_Real theX)
{
  Standard_Real anY = coord.Y();
  Standard_Real aZ  = coord.Z();
  Standard_Real aD  = sqrt(theX * theX + anY * anY + aZ * aZ);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::SetX() - result vector has zero norm");
  coord.SetX(theX / aD);
  coord.SetY(anY / aD);
  coord.SetZ(aZ / aD);
}

// =======================================================================
// function : SetY
// purpose  :
// =======================================================================
inline void Dir3d::SetY(const Standard_Real theY)
{
  Standard_Real aZ = coord.Z();
  Standard_Real aX = coord.X();
  Standard_Real aD = sqrt(aX * aX + theY * theY + aZ * aZ);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::SetY() - result vector has zero norm");
  coord.SetX(aX / aD);
  coord.SetY(theY / aD);
  coord.SetZ(aZ / aD);
}

// =======================================================================
// function : SetZ
// purpose  :
// =======================================================================
inline void Dir3d::SetZ(const Standard_Real theZ)
{
  Standard_Real aX  = coord.X();
  Standard_Real anY = coord.Y();
  Standard_Real aD  = sqrt(aX * aX + anY * anY + theZ * theZ);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::SetZ() - result vector has zero norm");
  coord.SetX(aX / aD);
  coord.SetY(anY / aD);
  coord.SetZ(theZ / aD);
}

// =======================================================================
// function : SetXYZ
// purpose  :
// =======================================================================
inline void Dir3d::SetXYZ(const Coords3d& theXYZ)
{
  Standard_Real aX  = theXYZ.X();
  Standard_Real anY = theXYZ.Y();
  Standard_Real aZ  = theXYZ.Z();
  Standard_Real aD  = sqrt(aX * aX + anY * anY + aZ * aZ);
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::SetX() - input vector has zero norm");
  coord.SetX(aX / aD);
  coord.SetY(anY / aD);
  coord.SetZ(aZ / aD);
}

// =======================================================================
// function : Cross
// purpose  :
// =======================================================================
inline void Dir3d::Cross(const Dir3d& theRight)
{
  coord.Cross(theRight.coord);
  Standard_Real aD = coord.Modulus();
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::Cross() - result vector has zero norm");
  coord.Divide(aD);
}

// =======================================================================
// function : Crossed
// purpose  :
// =======================================================================
inline Dir3d Dir3d::Crossed(const Dir3d& theRight) const
{
  Dir3d aV = *this;
  aV.coord.Cross(theRight.coord);
  Standard_Real aD = aV.coord.Modulus();
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::Crossed() - result vector has zero norm");
  aV.coord.Divide(aD);
  return aV;
}

// =======================================================================
// function : CrossCross
// purpose  :
// =======================================================================
inline void Dir3d::CrossCross(const Dir3d& theV1, const Dir3d& theV2)
{
  coord.CrossCross(theV1.coord, theV2.coord);
  Standard_Real aD = coord.Modulus();
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::CrossCross() - result vector has zero norm");
  coord.Divide(aD);
}

// =======================================================================
// function : CrossCrossed
// purpose  :
// =======================================================================
inline Dir3d Dir3d::CrossCrossed(const Dir3d& theV1, const Dir3d& theV2) const
{
  Dir3d aV = *this;
  (aV.coord).CrossCross(theV1.coord, theV2.coord);
  Standard_Real aD = aV.coord.Modulus();
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Dir3d::CrossCrossed() - result vector has zero norm");
  aV.coord.Divide(aD);
  return aV;
}

// =======================================================================
// function : Rotate
// purpose  :
// =======================================================================
inline void Dir3d::Rotate(const Axis3d& theA1, const Standard_Real theAng)
{
  Transform3d aT;
  aT.SetRotation(theA1, theAng);
  coord.Multiply(aT.HVectorialPart());
}

#endif // _gp_Dir_HeaderFile
