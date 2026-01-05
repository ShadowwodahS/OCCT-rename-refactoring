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

#ifndef _gp_Ax1_HeaderFile
#define _gp_Ax1_HeaderFile

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>

class Frame3d;
class Transform3d;
class Vector3d;

//! Describes an axis in 3D space.
//! An axis is defined by:
//! -   its origin (also referred to as its "Location point"), and
//! -   its unit vector (referred to as its "Direction" or "main   Direction").
//! An axis is used:
//! -   to describe 3D geometric entities (for example, the
//! axis of a revolution entity). It serves the same purpose
//! as the STEP function "axis placement one axis", or
//! -   to define geometric transformations (axis of
//! symmetry, axis of rotation, and so on).
//! For example, this entity can be used to locate a geometric entity
//! or to define a symmetry axis.
class Axis3d
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates an axis object representing Z axis of
  //! the reference coordinate system.
  Axis3d()
      : loc(0., 0., 0.),
        vdir(0., 0., 1.)
  {
  }

  //! P is the location point and V is the direction of <me>.
  Axis3d(const Point3d& theP, const Dir3d& theV)
      : loc(theP),
        vdir(theV)
  {
  }

  //! Assigns V as the "Direction"  of this axis.
  void SetDirection(const Dir3d& theV) { vdir = theV; }

  //! Assigns  P as the origin of this axis.
  void SetLocation(const Point3d& theP) { loc = theP; }

  //! Returns the direction of <me>.
  const Dir3d& Direction() const { return vdir; }

  //! Returns the location point of <me>.
  const Point3d& Location() const { return loc; }

  //! Returns True if  :
  //! . the angle between <me> and <Other> is lower or equal
  //! to <AngularTolerance> and
  //! . the distance between <me>.Location() and <Other> is lower
  //! or equal to <LinearTolerance> and
  //! . the distance between <Other>.Location() and <me> is lower
  //! or equal to LinearTolerance.
  Standard_EXPORT Standard_Boolean IsCoaxial(const Axis3d&       Other,
                                             const Standard_Real AngularTolerance,
                                             const Standard_Real LinearTolerance) const;

  //! Returns True if the direction of this and another axis are normal to each other.
  //! That is, if the angle between the two axes is equal to Pi/2.
  //! Note: the tolerance criterion is given by theAngularTolerance.
  Standard_Boolean IsNormal(const Axis3d& theOther, const Standard_Real theAngularTolerance) const
  {
    return vdir.IsNormal(theOther.vdir, theAngularTolerance);
  }

  //! Returns True if the direction of this and another axis are parallel with opposite orientation.
  //! That is, if the angle between the two axes is equal to Pi.
  //! Note: the tolerance criterion is given by theAngularTolerance.
  Standard_Boolean IsOpposite(const Axis3d& theOther, const Standard_Real theAngularTolerance) const
  {
    return vdir.IsOpposite(theOther.vdir, theAngularTolerance);
  }

  //! Returns True if the direction of this and another axis are parallel with same orientation or
  //! opposite orientation. That is, if the angle between the two axes is equal to 0 or Pi. Note:
  //! the tolerance criterion is given by theAngularTolerance.
  Standard_Boolean IsParallel(const Axis3d& theOther, const Standard_Real theAngularTolerance) const
  {
    return vdir.IsParallel(theOther.vdir, theAngularTolerance);
  }

  //! Computes the angular value, in radians, between this.Direction() and theOther.Direction().
  //! Returns the angle between 0 and 2*PI radians.
  Standard_Real Angle(const Axis3d& theOther) const { return vdir.Angle(theOther.vdir); }

  //! Reverses the unit vector of this axis and assigns the result to this axis.
  void Reverse() { vdir.Reverse(); }

  //! Reverses the unit vector of this axis and creates a new one.
  Standard_NODISCARD Axis3d Reversed() const
  {
    Dir3d D = vdir.Reversed();
    return Axis3d(loc, D);
  }

  //! Performs the symmetrical transformation of an axis
  //! placement with respect to the point P which is the
  //! center of the symmetry and assigns the result to this axis.
  Standard_EXPORT void Mirror(const Point3d& P);

  //! Performs the symmetrical transformation of an axis
  //! placement with respect to the point P which is the
  //! center of the symmetry and creates a new axis.
  Standard_NODISCARD Standard_EXPORT Axis3d Mirrored(const Point3d& P) const;

  //! Performs the symmetrical transformation of an axis
  //! placement with respect to an axis placement which
  //! is the axis of the symmetry and assigns the result to this axis.
  Standard_EXPORT void Mirror(const Axis3d& A1);

  //! Performs the symmetrical transformation of an axis
  //! placement with respect to an axis placement which
  //! is the axis of the symmetry and creates a new axis.
  Standard_NODISCARD Standard_EXPORT Axis3d Mirrored(const Axis3d& A1) const;

  //! Performs the symmetrical transformation of an axis
  //! placement with respect to a plane. The axis placement
  //! <A2> locates the plane of the symmetry :
  //! (Location, XDirection, YDirection) and assigns the result to this axis.
  Standard_EXPORT void Mirror(const Frame3d& A2);

  //! Performs the symmetrical transformation of an axis
  //! placement with respect to a plane. The axis placement
  //! <A2> locates the plane of the symmetry :
  //! (Location, XDirection, YDirection) and creates a new axis.
  Standard_NODISCARD Standard_EXPORT Axis3d Mirrored(const Frame3d& A2) const;

  //! Rotates this axis at an angle theAngRad (in radians) about the axis theA1
  //! and assigns the result to this axis.
  void Rotate(const Axis3d& theA1, const Standard_Real theAngRad)
  {
    loc.Rotate(theA1, theAngRad);
    vdir.Rotate(theA1, theAngRad);
  }

  //! Rotates this axis at an angle theAngRad (in radians) about the axis theA1
  //! and creates a new one.
  Standard_NODISCARD Axis3d Rotated(const Axis3d& theA1, const Standard_Real theAngRad) const
  {
    Axis3d A = *this;
    A.Rotate(theA1, theAngRad);
    return A;
  }

  //! Applies a scaling transformation to this axis with:
  //! - scale factor theS, and
  //! - center theP and assigns the result to this axis.
  void Scale(const Point3d& theP, const Standard_Real theS)
  {
    loc.Scale(theP, theS);
    if (theS < 0.0)
    {
      vdir.Reverse();
    }
  }

  //! Applies a scaling transformation to this axis with:
  //! - scale factor theS, and
  //! - center theP and creates a new axis.
  Standard_NODISCARD Axis3d Scaled(const Point3d& theP, const Standard_Real theS) const
  {
    Axis3d A1 = *this;
    A1.Scale(theP, theS);
    return A1;
  }

  //! Applies the transformation theT to this axis and assigns the result to this axis.
  void Transform(const Transform3d& theT)
  {
    loc.Transform(theT);
    vdir.Transform(theT);
  }

  //! Applies the transformation theT to this axis and creates a new one.
  //!
  //! Translates an axis plaxement in the direction of the vector <V>.
  //! The magnitude of the translation is the vector's magnitude.
  Standard_NODISCARD Axis3d Transformed(const Transform3d& theT) const
  {
    Axis3d A1 = *this;
    A1.Transform(theT);
    return A1;
  }

  //! Translates this axis by the vector theV, and assigns the result to this axis.
  void Translate(const Vector3d& theV) { loc.Translate(theV); }

  //! Translates this axis by the vector theV,
  //! and creates a new one.
  Standard_NODISCARD Axis3d Translated(const Vector3d& theV) const
  {
    Axis3d A1 = *this;
    (A1.loc).Translate(theV);
    return A1;
  }

  //! Translates this axis by:
  //! the vector (theP1, theP2) defined from point theP1 to point theP2.
  //! and assigns the result to this axis.
  void Translate(const Point3d& theP1, const Point3d& theP2) { loc.Translate(theP1, theP2); }

  //! Translates this axis by:
  //! the vector (theP1, theP2) defined from point theP1 to point theP2.
  //! and creates a new one.
  Standard_NODISCARD Axis3d Translated(const Point3d& theP1, const Point3d& theP2) const
  {
    Axis3d A1 = *this;
    (A1.loc).Translate(theP1, theP2);
    return A1;
  }

  //! Dumps the content of me into the stream
  Standard_EXPORT void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const;

  //! Inits the content of me from the stream
  Standard_EXPORT Standard_Boolean InitFromJson(const Standard_SStream& theSStream,
                                                Standard_Integer&       theStreamPos);

private:
  Point3d loc;
  Dir3d vdir;
};

#endif // _gp_Ax1_HeaderFile
