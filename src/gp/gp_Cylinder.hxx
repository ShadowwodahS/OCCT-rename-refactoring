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

#ifndef _gp_Cylinder_HeaderFile
#define _gp_Cylinder_HeaderFile

#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>

//! Describes an infinite cylindrical surface.
//! A cylinder is defined by its radius and positioned in space
//! with a coordinate system (a Ax3 object), the "main
//! Axis" of which is the axis of the cylinder. This coordinate
//! system is the "local coordinate system" of the cylinder.
//! Note: when a Cylinder1 cylinder is converted into a
//! Geom_CylindricalSurface cylinder, some implicit
//! properties of its local coordinate system are used explicitly:
//! -   its origin, "X Direction", "Y Direction" and "main
//! Direction" are used directly to define the parametric
//! directions on the cylinder and the origin of the parameters,
//! -   its implicit orientation (right-handed or left-handed)
//! gives an orientation (direct or indirect) to the
//! Geom_CylindricalSurface cylinder.
//! See Also
//! CylinderBuilder which provides functions for more
//! complex1 cylinder constructions
//! Geom_CylindricalSurface which provides additional
//! functions for constructing cylinders and works, in
//! particular, with the parametric equations of cylinders Ax3
class Cylinder1
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates a indefinite cylinder.
  Cylinder1() { radius = RealLast(); }

  //! Creates a cylinder of radius Radius, whose axis is the "main
  //! Axis" of theA3. theA3 is the local coordinate system of the cylinder.   Raises
  //! ConstructionErrord if theRadius < 0.0
  Cylinder1(const Ax3& theA3, const Standard_Real theRadius)
      : pos(theA3),
        radius(theRadius)
  {
    Standard_ConstructionError_Raise_if(theRadius < 0.0,
                                        "Cylinder1() - radius should be positive number");
  }

  //! Changes the symmetry axis of the cylinder. Raises ConstructionError if the direction of theA1
  //! is parallel to the "XDirection" of the coordinate system of the cylinder.
  void SetAxis(const Axis3d& theA1) { pos.SetAxis(theA1); }

  //! Changes the location of the surface.
  void SetLocation(const Point3d& theLoc) { pos.SetLocation(theLoc); }

  //! Change the local coordinate system of the surface.
  void SetPosition(const Ax3& theA3) { pos = theA3; }

  //! Modifies the radius of this cylinder.
  //! Exceptions
  //! Standard_ConstructionError if theR is negative.
  void SetRadius(const Standard_Real theR)
  {
    Standard_ConstructionError_Raise_if(
      theR < 0.0,
      "Cylinder1::SetRadius() - radius should be positive number");
    radius = theR;
  }

  //! Reverses the   U   parametrization of   the cylinder
  //! reversing the YAxis.
  void UReverse() { pos.YReverse(); }

  //! Reverses the   V   parametrization of   the  plane
  //! reversing the Axis.
  void VReverse() { pos.ZReverse(); }

  //! Returns true if the local coordinate system of this cylinder is right-handed.
  Standard_Boolean Direct() const { return pos.Direct(); }

  //! Returns the symmetry axis of the cylinder.
  const Axis3d& Axis() const { return pos.Axis(); }

  //! Computes the coefficients of the implicit equation of the quadric
  //! in the absolute cartesian coordinate system :
  //! theA1.X**2 + theA2.Y**2 + theA3.Z**2 + 2.(theB1.X.Y + theB2.X.Z + theB3.Y.Z) +
  //! 2.(theC1.X + theC2.Y + theC3.Z) + theD = 0.0
  Standard_EXPORT void Coefficients(Standard_Real& theA1,
                                    Standard_Real& theA2,
                                    Standard_Real& theA3,
                                    Standard_Real& theB1,
                                    Standard_Real& theB2,
                                    Standard_Real& theB3,
                                    Standard_Real& theC1,
                                    Standard_Real& theC2,
                                    Standard_Real& theC3,
                                    Standard_Real& theD) const;

  //! Returns the "Location" point of the cylinder.
  const Point3d& Location() const { return pos.Location(); }

  //! Returns the local coordinate system of the cylinder.
  const Ax3& Position1() const { return pos; }

  //! Returns the radius of the cylinder.
  Standard_Real Radius() const { return radius; }

  //! Returns the axis X of the cylinder.
  Axis3d XAxis() const { return Axis3d(pos.Location(), pos.XDirection()); }

  //! Returns the axis Y of the cylinder.
  Axis3d YAxis() const { return Axis3d(pos.Location(), pos.YDirection()); }

  Standard_EXPORT void Mirror(const Point3d& theP);

  //! Performs the symmetrical transformation of a cylinder
  //! with respect to the point theP which is the center of the
  //! symmetry.
  Standard_NODISCARD Standard_EXPORT Cylinder1 Mirrored(const Point3d& theP) const;

  Standard_EXPORT void Mirror(const Axis3d& theA1);

  //! Performs the symmetrical transformation of a cylinder with
  //! respect to an axis placement which is the axis of the
  //! symmetry.
  Standard_NODISCARD Standard_EXPORT Cylinder1 Mirrored(const Axis3d& theA1) const;

  Standard_EXPORT void Mirror(const Frame3d& theA2);

  //! Performs the symmetrical transformation of a cylinder with respect
  //! to a plane. The axis placement theA2 locates the plane of the
  //! of the symmetry : (Location, XDirection, YDirection).
  Standard_NODISCARD Standard_EXPORT Cylinder1 Mirrored(const Frame3d& theA2) const;

  void Rotate(const Axis3d& theA1, const Standard_Real theAng) { pos.Rotate(theA1, theAng); }

  //! Rotates a cylinder. theA1 is the axis of the rotation.
  //! theAng is the angular value of the rotation in radians.
  Standard_NODISCARD Cylinder1 Rotated(const Axis3d& theA1, const Standard_Real theAng) const
  {
    Cylinder1 aCyl = *this;
    aCyl.pos.Rotate(theA1, theAng);
    return aCyl;
  }

  void Scale(const Point3d& theP, const Standard_Real theS);

  //! Scales a cylinder. theS is the scaling value.
  //! The absolute value of theS is used to scale the cylinder
  Standard_NODISCARD Cylinder1 Scaled(const Point3d& theP, const Standard_Real theS) const;

  void Transform(const Transform3d& theT);

  //! Transforms a cylinder with the transformation theT from class Trsf.
  Standard_NODISCARD Cylinder1 Transformed(const Transform3d& theT) const;

  void Translate(const Vector3d& theV) { pos.Translate(theV); }

  //! Translates a cylinder in the direction of the vector theV.
  //! The magnitude of the translation is the vector's magnitude.
  Standard_NODISCARD Cylinder1 Translated(const Vector3d& theV) const
  {
    Cylinder1 aCyl = *this;
    aCyl.pos.Translate(theV);
    return aCyl;
  }

  void Translate(const Point3d& theP1, const Point3d& theP2) { pos.Translate(theP1, theP2); }

  //! Translates a cylinder from the point theP1 to the point theP2.
  Standard_NODISCARD Cylinder1 Translated(const Point3d& theP1, const Point3d& theP2) const
  {
    Cylinder1 aCyl = *this;
    aCyl.pos.Translate(theP1, theP2);
    return aCyl;
  }

private:
  Ax3        pos;
  Standard_Real radius;
};

// =======================================================================
// function : Scale
// purpose  :
// =======================================================================
inline void Cylinder1::Scale(const Point3d& theP, const Standard_Real theS)
{
  pos.Scale(theP, theS);
  radius *= theS;
  if (radius < 0)
  {
    radius = -radius;
  }
}

// =======================================================================
// function : Scaled
// purpose  :
// =======================================================================
inline Cylinder1 Cylinder1::Scaled(const Point3d& theP, const Standard_Real theS) const
{
  Cylinder1 aCyl = *this;
  aCyl.pos.Scale(theP, theS);
  aCyl.radius *= theS;
  if (aCyl.radius < 0)
  {
    aCyl.radius = -aCyl.radius;
  }
  return aCyl;
}

// =======================================================================
// function : Transform
// purpose  :
// =======================================================================
inline void Cylinder1::Transform(const Transform3d& theT)
{
  pos.Transform(theT);
  radius *= theT.ScaleFactor();
  if (radius < 0)
  {
    radius = -radius;
  }
}

// =======================================================================
// function : Transformed
// purpose  :
// =======================================================================
inline Cylinder1 Cylinder1::Transformed(const Transform3d& theT) const
{
  Cylinder1 aCyl = *this;
  aCyl.pos.Transform(theT);
  aCyl.radius *= theT.ScaleFactor();
  if (aCyl.radius < 0)
  {
    aCyl.radius = -aCyl.radius;
  }
  return aCyl;
}

#endif // _gp_Cylinder_HeaderFile
