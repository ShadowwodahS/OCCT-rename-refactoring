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

#ifndef _gp_Vec_HeaderFile
#define _gp_Vec_HeaderFile

#include <gp_VectorWithNullMagnitude.hxx>
#include <gp_XYZ.hxx>
#include <Standard_DomainError.hxx>

class Dir3d;
class Point3d;
class Axis3d;
class Frame3d;
class Transform3d;

//! Defines a non-persistent vector in 3D space.
class Vector3d
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates a zero vector.
  Vector3d() {}

  //! Creates a unitary vector from a direction theV.
  Vector3d(const Dir3d& theV);

  //! Creates a vector with a triplet of coordinates.
  Vector3d(const Coords3d& theCoord)
      : coord(theCoord)
  {
  }

  //! Creates a point with its three cartesian coordinates.
  Vector3d(const Standard_Real theXv, const Standard_Real theYv, const Standard_Real theZv)
      : coord(theXv, theYv, theZv)
  {
  }

  //! Creates a vector from two points. The length of the vector
  //! is the distance between theP1 and theP2
  Vector3d(const Point3d& theP1, const Point3d& theP2);

  //! Changes the coordinate of range theIndex
  //! theIndex = 1 => X is modified
  //! theIndex = 2 => Y is modified
  //! theIndex = 3 => Z is modified
  //! Raised if theIndex != {1, 2, 3}.
  void SetCoord(const Standard_Integer theIndex, const Standard_Real theXi)
  {
    coord.SetCoord(theIndex, theXi);
  }

  //! For this vector, assigns
  //! -   the values theXv, theYv and theZv to its three coordinates.
  void SetCoord(const Standard_Real theXv, const Standard_Real theYv, const Standard_Real theZv)
  {
    coord.SetX(theXv);
    coord.SetY(theYv);
    coord.SetZ(theZv);
  }

  //! Assigns the given value to the X coordinate of this vector.
  void SetX(const Standard_Real theX) { coord.SetX(theX); }

  //! Assigns the given value to the X coordinate of this vector.
  void SetY(const Standard_Real theY) { coord.SetY(theY); }

  //! Assigns the given value to the X coordinate of this vector.
  void SetZ(const Standard_Real theZ) { coord.SetZ(theZ); }

  //! Assigns the three coordinates of theCoord to this vector.
  void SetXYZ(const Coords3d& theCoord) { coord = theCoord; }

  //! Returns the coordinate of range theIndex :
  //! theIndex = 1 => X is returned
  //! theIndex = 2 => Y is returned
  //! theIndex = 3 => Z is returned
  //! Raised if theIndex != {1, 2, 3}.
  Standard_Real Coord(const Standard_Integer theIndex) const { return coord.Coord(theIndex); }

  //! For this vector returns its three coordinates theXv, theYv, and theZv inline
  void Coord(Standard_Real& theXv, Standard_Real& theYv, Standard_Real& theZv) const
  {
    theXv = coord.X();
    theYv = coord.Y();
    theZv = coord.Z();
  }

  //! For this vector, returns its X coordinate.
  Standard_Real X() const { return coord.X(); }

  //! For this vector, returns its Y coordinate.
  Standard_Real Y() const { return coord.Y(); }

  //! For this vector, returns its Z  coordinate.
  Standard_Real Z() const { return coord.Z(); }

  //! For this vector, returns
  //! -   its three coordinates as a number triple
  const Coords3d& XYZ() const { return coord; }

  //! Returns True if the two vectors have the same magnitude value
  //! and the same direction. The precision values are theLinearTolerance
  //! for the magnitude and theAngularTolerance for the direction.
  Standard_EXPORT Standard_Boolean IsEqual(const Vector3d&       theOther,
                                           const Standard_Real theLinearTolerance,
                                           const Standard_Real theAngularTolerance) const;

  //! Returns True if abs(<me>.Angle(theOther) - PI/2.) <= theAngularTolerance
  //! Raises VectorWithNullMagnitude if <me>.Magnitude() <= Resolution or
  //! theOther.Magnitude() <= Resolution from gp1
  Standard_Boolean IsNormal(const Vector3d& theOther, const Standard_Real theAngularTolerance) const;

  //! Returns True if PI - <me>.Angle(theOther) <= theAngularTolerance
  //! Raises VectorWithNullMagnitude if <me>.Magnitude() <= Resolution or
  //! Other.Magnitude() <= Resolution from gp1
  Standard_Boolean IsOpposite(const Vector3d& theOther, const Standard_Real theAngularTolerance) const
  {
    Standard_Real anAng = M_PI - Angle(theOther);
    return anAng <= theAngularTolerance;
  }

  //! Returns True if Angle(<me>, theOther) <= theAngularTolerance or
  //! PI - Angle(<me>, theOther) <= theAngularTolerance
  //! This definition means that two parallel vectors cannot define
  //! a plane but two vectors with opposite directions are considered
  //! as parallel. Raises VectorWithNullMagnitude if <me>.Magnitude() <= Resolution or
  //! Other.Magnitude() <= Resolution from gp1
  Standard_Boolean IsParallel(const Vector3d& theOther, const Standard_Real theAngularTolerance) const
  {
    Standard_Real anAng = Angle(theOther);
    return anAng <= theAngularTolerance || M_PI - anAng <= theAngularTolerance;
  }

  //! Computes the angular value between <me> and <theOther>
  //! Returns the angle value between 0 and PI in radian.
  //! Raises VectorWithNullMagnitude if <me>.Magnitude() <= Resolution from gp1 or
  //! theOther.Magnitude() <= Resolution because the angular value is
  //! indefinite if one of the vectors has a null magnitude.
  Standard_Real Angle(const Vector3d& theOther) const;

  //! Computes the angle, in radians, between this vector and
  //! vector theOther. The result is a value between -Pi and Pi.
  //! For this, theVRef defines the positive sense of rotation: the
  //! angular value is positive, if the cross product this ^ theOther
  //! has the same orientation as theVRef relative to the plane
  //! defined by the vectors this and theOther. Otherwise, the
  //! angular value is negative.
  //! Exceptions
  //! gp_VectorWithNullMagnitude if the magnitude of this
  //! vector, the vector theOther, or the vector theVRef is less than or
  //! equal to gp1::Resolution().
  //! Standard_DomainError if this vector, the vector theOther,
  //! and the vector theVRef are coplanar, unless this vector and
  //! the vector theOther are parallel.
  Standard_Real AngleWithRef(const Vector3d& theOther, const Vector3d& theVRef) const;

  //! Computes the magnitude of this vector.
  Standard_Real Magnitude() const { return coord.Modulus(); }

  //! Computes the square magnitude of this vector.
  Standard_Real SquareMagnitude() const { return coord.SquareModulus(); }

  //! Adds two vectors
  void Add(const Vector3d& theOther) { coord.Add(theOther.coord); }

  void operator+=(const Vector3d& theOther) { Add(theOther); }

  //! Adds two vectors
  Standard_NODISCARD Vector3d Added(const Vector3d& theOther) const
  {
    Vector3d aV = *this;
    aV.coord.Add(theOther.coord);
    return aV;
  }

  Standard_NODISCARD Vector3d operator+(const Vector3d& theOther) const { return Added(theOther); }

  //! Subtracts two vectors
  void Subtract(const Vector3d& theRight) { coord.Subtract(theRight.coord); }

  void operator-=(const Vector3d& theRight) { Subtract(theRight); }

  //! Subtracts two vectors
  Standard_NODISCARD Vector3d Subtracted(const Vector3d& theRight) const
  {
    Vector3d aV = *this;
    aV.coord.Subtract(theRight.coord);
    return aV;
  }

  Standard_NODISCARD Vector3d operator-(const Vector3d& theRight) const { return Subtracted(theRight); }

  //! Multiplies a vector by a scalar
  void Multiply(const Standard_Real theScalar) { coord.Multiply(theScalar); }

  void operator*=(const Standard_Real theScalar) { Multiply(theScalar); }

  //! Multiplies a vector by a scalar
  Standard_NODISCARD Vector3d Multiplied(const Standard_Real theScalar) const
  {
    Vector3d aV = *this;
    aV.coord.Multiply(theScalar);
    return aV;
  }

  Standard_NODISCARD Vector3d operator*(const Standard_Real theScalar) const
  {
    return Multiplied(theScalar);
  }

  //! Divides a vector by a scalar
  void Divide(const Standard_Real theScalar) { coord.Divide(theScalar); }

  void operator/=(const Standard_Real theScalar) { Divide(theScalar); }

  //! Divides a vector by a scalar
  Standard_NODISCARD Vector3d Divided(const Standard_Real theScalar) const
  {
    Vector3d aV = *this;
    aV.coord.Divide(theScalar);
    return aV;
  }

  Standard_NODISCARD Vector3d operator/(const Standard_Real theScalar) const
  {
    return Divided(theScalar);
  }

  //! computes the cross product between two vectors
  void Cross(const Vector3d& theRight) { coord.Cross(theRight.coord); }

  void operator^=(const Vector3d& theRight) { Cross(theRight); }

  //! computes the cross product between two vectors
  Standard_NODISCARD Vector3d Crossed(const Vector3d& theRight) const
  {
    Vector3d aV = *this;
    aV.coord.Cross(theRight.coord);
    return aV;
  }

  Standard_NODISCARD Vector3d operator^(const Vector3d& theRight) const { return Crossed(theRight); }

  //! Computes the magnitude of the cross
  //! product between <me> and theRight.
  //! Returns || <me> ^ theRight ||
  Standard_Real CrossMagnitude(const Vector3d& theRight) const
  {
    return coord.CrossMagnitude(theRight.coord);
  }

  //! Computes the square magnitude of
  //! the cross product between <me> and theRight.
  //! Returns || <me> ^ theRight ||**2
  Standard_Real CrossSquareMagnitude(const Vector3d& theRight) const
  {
    return coord.CrossSquareMagnitude(theRight.coord);
  }

  //! Computes the triple vector product.
  //! <me> ^= (theV1 ^ theV2)
  void CrossCross(const Vector3d& theV1, const Vector3d& theV2)
  {
    coord.CrossCross(theV1.coord, theV2.coord);
  }

  //! Computes the triple vector product.
  //! <me> ^ (theV1 ^ theV2)
  Standard_NODISCARD Vector3d CrossCrossed(const Vector3d& theV1, const Vector3d& theV2) const
  {
    Vector3d aV = *this;
    aV.coord.CrossCross(theV1.coord, theV2.coord);
    return aV;
  }

  //! computes the scalar product
  Standard_Real Dot(const Vector3d& theOther) const { return coord.Dot(theOther.coord); }

  Standard_Real operator*(const Vector3d& theOther) const { return Dot(theOther); }

  //! Computes the triple scalar product <me> * (theV1 ^ theV2).
  Standard_Real DotCross(const Vector3d& theV1, const Vector3d& theV2) const
  {
    return coord.DotCross(theV1.coord, theV2.coord);
  }

  //! normalizes a vector
  //! Raises an exception if the magnitude of the vector is
  //! lower or equal to Resolution from gp1.
  void Normalize()
  {
    Standard_Real aD = coord.Modulus();
    Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                        "Vector3d::Normalize() - vector has zero norm");
    coord.Divide(aD);
  }

  //! normalizes a vector
  //! Raises an exception if the magnitude of the vector is
  //! lower or equal to Resolution from gp1.
  Standard_NODISCARD Vector3d Normalized() const;

  //! Reverses the direction of a vector
  void Reverse() { coord.Reverse(); }

  //! Reverses the direction of a vector
  Standard_NODISCARD Vector3d Reversed() const
  {
    Vector3d aV = *this;
    aV.coord.Reverse();
    return aV;
  }

  Standard_NODISCARD Vector3d operator-() const { return Reversed(); }

  //! <me> is set to the following linear form :
  //! theA1 * theV1 + theA2 * theV2 + theA3 * theV3 + theV4
  void SetLinearForm(const Standard_Real theA1,
                     const Vector3d&       theV1,
                     const Standard_Real theA2,
                     const Vector3d&       theV2,
                     const Standard_Real theA3,
                     const Vector3d&       theV3,
                     const Vector3d&       theV4)
  {
    coord.SetLinearForm(theA1, theV1.coord, theA2, theV2.coord, theA3, theV3.coord, theV4.coord);
  }

  //! <me> is set to the following linear form :
  //! theA1 * theV1 + theA2 * theV2 + theA3 * theV3
  void SetLinearForm(const Standard_Real theA1,
                     const Vector3d&       theV1,
                     const Standard_Real theA2,
                     const Vector3d&       theV2,
                     const Standard_Real theA3,
                     const Vector3d&       theV3)
  {
    coord.SetLinearForm(theA1, theV1.coord, theA2, theV2.coord, theA3, theV3.coord);
  }

  //! <me> is set to the following linear form :
  //! theA1 * theV1 + theA2 * theV2 + theV3
  void SetLinearForm(const Standard_Real theA1,
                     const Vector3d&       theV1,
                     const Standard_Real theA2,
                     const Vector3d&       theV2,
                     const Vector3d&       theV3)
  {
    coord.SetLinearForm(theA1, theV1.coord, theA2, theV2.coord, theV3.coord);
  }

  //! <me> is set to the following linear form :
  //! theA1 * theV1 + theA2 * theV2
  void SetLinearForm(const Standard_Real theA1,
                     const Vector3d&       theV1,
                     const Standard_Real theA2,
                     const Vector3d&       theV2)
  {
    coord.SetLinearForm(theA1, theV1.coord, theA2, theV2.coord);
  }

  //! <me> is set to the following linear form : theA1 * theV1 + theV2
  void SetLinearForm(const Standard_Real theA1, const Vector3d& theV1, const Vector3d& theV2)
  {
    coord.SetLinearForm(theA1, theV1.coord, theV2.coord);
  }

  //! <me> is set to the following linear form : theV1 + theV2
  void SetLinearForm(const Vector3d& theV1, const Vector3d& theV2)
  {
    coord.SetLinearForm(theV1.coord, theV2.coord);
  }

  Standard_EXPORT void Mirror(const Vector3d& theV);

  //! Performs the symmetrical transformation of a vector
  //! with respect to the vector theV which is the center of
  //! the  symmetry.
  Standard_NODISCARD Standard_EXPORT Vector3d Mirrored(const Vector3d& theV) const;

  Standard_EXPORT void Mirror(const Axis3d& theA1);

  //! Performs the symmetrical transformation of a vector
  //! with respect to an axis placement which is the axis
  //! of the symmetry.
  Standard_NODISCARD Standard_EXPORT Vector3d Mirrored(const Axis3d& theA1) const;

  Standard_EXPORT void Mirror(const Frame3d& theA2);

  //! Performs the symmetrical transformation of a vector
  //! with respect to a plane. The axis placement theA2 locates
  //! the plane of the symmetry : (Location, XDirection, YDirection).
  Standard_NODISCARD Standard_EXPORT Vector3d Mirrored(const Frame3d& theA2) const;

  void Rotate(const Axis3d& theA1, const Standard_Real theAng);

  //! Rotates a vector. theA1 is the axis of the rotation.
  //! theAng is the angular value of the rotation in radians.
  Standard_NODISCARD Vector3d Rotated(const Axis3d& theA1, const Standard_Real theAng) const
  {
    Vector3d aVres = *this;
    aVres.Rotate(theA1, theAng);
    return aVres;
  }

  void Scale(const Standard_Real theS) { coord.Multiply(theS); }

  //! Scales a vector. theS is the scaling value.
  Standard_NODISCARD Vector3d Scaled(const Standard_Real theS) const
  {
    Vector3d aV = *this;
    aV.coord.Multiply(theS);
    return aV;
  }

  //! Transforms a vector with the transformation theT.
  Standard_EXPORT void Transform(const Transform3d& theT);

  //! Transforms a vector with the transformation theT.
  Standard_NODISCARD Vector3d Transformed(const Transform3d& theT) const
  {
    Vector3d aV = *this;
    aV.Transform(theT);
    return aV;
  }

  //! Dumps the content of me into the stream
  Standard_EXPORT void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const;

private:
  Coords3d coord;
};

#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

//=======================================================================
// function :  Vector3d
// purpose :
//=======================================================================
inline Vector3d::Vector3d(const Dir3d& theV)
{
  coord = theV.XYZ();
}

//=======================================================================
// function :  Vector3d
// purpose :
//=======================================================================
inline Vector3d::Vector3d(const Point3d& theP1, const Point3d& theP2)
{
  coord = theP2.XYZ().Subtracted(theP1.XYZ());
}

//=======================================================================
// function :  IsNormal
// purpose :
//=======================================================================
inline Standard_Boolean Vector3d::IsNormal(const Vector3d&       theOther,
                                         const Standard_Real theAngularTolerance) const
{
  Standard_Real anAng = M_PI / 2.0 - Angle(theOther);
  if (anAng < 0)
  {
    anAng = -anAng;
  }
  return anAng <= theAngularTolerance;
}

//=======================================================================
// function :  Angle
// purpose :
//=======================================================================
inline Standard_Real Vector3d::Angle(const Vector3d& theOther) const
{
  gp_VectorWithNullMagnitude_Raise_if(coord.Modulus() <= gp1::Resolution()
                                        || theOther.coord.Modulus() <= gp1::Resolution(),
                                      " ");
  return (Dir3d(coord)).Angle(theOther);
}

//=======================================================================
// function :  AngleWithRef
// purpose :
//=======================================================================
inline Standard_Real Vector3d::AngleWithRef(const Vector3d& theOther, const Vector3d& theVRef) const
{
  gp_VectorWithNullMagnitude_Raise_if(coord.Modulus() <= gp1::Resolution()
                                        || theVRef.coord.Modulus() <= gp1::Resolution()
                                        || theOther.coord.Modulus() <= gp1::Resolution(),
                                      " ");
  return (Dir3d(coord)).AngleWithRef(theOther, theVRef);
}

//=======================================================================
// function :  Normalized
// purpose :
//=======================================================================
inline Vector3d Vector3d::Normalized() const
{
  Standard_Real aD = coord.Modulus();
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Vector3d::Normalized() - vector has zero norm");
  Vector3d aV = *this;
  aV.coord.Divide(aD);
  return aV;
}

//=======================================================================
// function :  Rotate
// purpose :
//=======================================================================
inline void Vector3d::Rotate(const Axis3d& theA1, const Standard_Real theAng)
{
  Transform3d aT;
  aT.SetRotation(theA1, theAng);
  coord.Multiply(aT.VectorialPart());
}

//=======================================================================
// function :  operator*
// purpose :
//=======================================================================
inline Vector3d operator*(const Standard_Real theScalar, const Vector3d& theV)
{
  return theV.Multiplied(theScalar);
}

#endif // _gp_Vec_HeaderFile
