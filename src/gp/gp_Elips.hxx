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

#ifndef _gp_Elips_HeaderFile
#define _gp_Elips_HeaderFile

#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <Standard_ConstructionError.hxx>

//! Describes an ellipse in 3D space.
//! An ellipse is defined by its major and minor radii and
//! positioned in space with a coordinate system (a Frame3d object) as follows:
//! -   the origin of the coordinate system is the center of the ellipse,
//! -   its "X Direction" defines the major axis of the ellipse, and
//! - its "Y Direction" defines the minor axis of the ellipse.
//! Together, the origin, "X Direction" and "Y Direction" of
//! this coordinate system define the plane of the ellipse.
//! This coordinate system is the "local coordinate system"
//! of the ellipse. In this coordinate system, the equation of
//! the ellipse is:
//! @code
//! X*X / (MajorRadius**2) + Y*Y / (MinorRadius**2) = 1.0
//! @endcode
//! The "main Direction" of the local coordinate system gives
//! the normal vector to the plane of the ellipse. This vector
//! gives an implicit orientation to the ellipse (definition of the
//! trigonometric sense). We refer to the "main Axis" of the
//! local coordinate system as the "Axis" of the ellipse.
//! See Also
//! gce_MakeElips which provides functions for more
//! complex1 ellipse constructions
//! Geom_Ellipse which provides additional functions for
//! constructing ellipses and works, in particular, with the
//! parametric equations of ellipses
class gp_Elips
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates an indefinite ellipse.
  gp_Elips()
      : majorRadius(RealLast()),
        minorRadius(RealSmall())
  {
  }

  //! The major radius of the ellipse is on the "XAxis" and the
  //! minor radius is on the "YAxis" of the ellipse. The "XAxis"
  //! is defined with the "XDirection" of theA2 and the "YAxis" is
  //! defined with the "YDirection" of theA2.
  //! Warnings :
  //! It is not forbidden to create an ellipse with theMajorRadius =
  //! theMinorRadius.
  //! Raises ConstructionError if theMajorRadius < theMinorRadius or theMinorRadius < 0.
  gp_Elips(const Frame3d&       theA2,
           const Standard_Real theMajorRadius,
           const Standard_Real theMinorRadius)
      : pos(theA2),
        majorRadius(theMajorRadius),
        minorRadius(theMinorRadius)
  {
    Standard_ConstructionError_Raise_if(theMinorRadius < 0.0 || theMajorRadius < theMinorRadius,
                                        "gp_Elips() - invalid construction parameters");
  }

  //! Changes the axis normal to the plane of the ellipse.
  //! It modifies the definition of this plane.
  //! The "XAxis" and the "YAxis" are recomputed.
  //! The local coordinate system is redefined so that:
  //! -   its origin and "main Direction" become those of the
  //! axis theA1 (the "X Direction" and "Y Direction" are then
  //! recomputed in the same way as for any Frame3d), or
  //! Raises ConstructionError if the direction of theA1
  //! is parallel to the direction of the "XAxis" of the ellipse.
  void SetAxis(const Axis3d& theA1) { pos.SetAxis(theA1); }

  //! Modifies this ellipse, by redefining its local coordinate
  //! so that its origin becomes theP.
  void SetLocation(const Point3d& theP) { pos.SetLocation(theP); }

  //! The major radius of the ellipse is on the "XAxis" (major axis)
  //! of the ellipse.
  //! Raises ConstructionError if theMajorRadius < MinorRadius.
  void SetMajorRadius(const Standard_Real theMajorRadius)
  {
    Standard_ConstructionError_Raise_if(
      theMajorRadius < minorRadius,
      "gp_Elips::SetMajorRadius() - major radius should be greater or equal to minor radius");
    majorRadius = theMajorRadius;
  }

  //! The minor radius of the ellipse is on the "YAxis" (minor axis)
  //! of the ellipse.
  //! Raises ConstructionError if theMinorRadius > MajorRadius or MinorRadius < 0.
  void SetMinorRadius(const Standard_Real theMinorRadius)
  {
    Standard_ConstructionError_Raise_if(theMinorRadius < 0.0 || majorRadius < theMinorRadius,
                                        "gp_Elips::SetMinorRadius() - minor radius should be a "
                                        "positive number lesser or equal to major radius");
    minorRadius = theMinorRadius;
  }

  //! Modifies this ellipse, by redefining its local coordinate
  //! so that it becomes theA2.
  void SetPosition(const Frame3d& theA2) { pos = theA2; }

  //! Computes the area of the Ellipse.
  Standard_Real Area() const { return M_PI * majorRadius * minorRadius; }

  //! Computes the axis normal to the plane of the ellipse.
  const Axis3d& Axis() const { return pos.Axis(); }

  //! Computes the first or second directrix of this ellipse.
  //! These are the lines, in the plane of the ellipse, normal to
  //! the major axis, at a distance equal to
  //! MajorRadius/e from the center of the ellipse, where
  //! e is the eccentricity of the ellipse.
  //! The first directrix (Directrix1) is on the positive side of
  //! the major axis. The second directrix (Directrix2) is on
  //! the negative side.
  //! The directrix is returned as an axis (Axis3d object), the
  //! origin of which is situated on the "X Axis" of the local
  //! coordinate system of this ellipse.
  //! Exceptions
  //! Standard_ConstructionError if the eccentricity is null
  //! (the ellipse has degenerated into a circle).
  Axis3d Directrix1() const;

  //! This line is obtained by the symmetrical transformation
  //! of "Directrix1" with respect to the "YAxis" of the ellipse.
  //! Exceptions
  //! Standard_ConstructionError if the eccentricity is null
  //! (the ellipse has degenerated into a circle).
  Axis3d Directrix2() const;

  //! Returns the eccentricity of the ellipse  between 0.0 and 1.0
  //! If f is the distance between the center of the ellipse and
  //! the Focus1 then the eccentricity e = f / MajorRadius.
  //! Raises ConstructionError if MajorRadius = 0.0
  Standard_Real Eccentricity() const;

  //! Computes the focal distance. It is the distance between the
  //! two focus focus1 and focus2 of the ellipse.
  Standard_Real Focal() const
  {
    return 2.0 * sqrt(majorRadius * majorRadius - minorRadius * minorRadius);
  }

  //! Returns the first focus of the ellipse. This focus is on the
  //! positive side of the "XAxis" of the ellipse.
  Point3d Focus1() const;

  //! Returns the second focus of the ellipse. This focus is on the
  //! negative side of the "XAxis" of the ellipse.
  Point3d Focus2() const;

  //! Returns the center of the ellipse. It is the "Location"
  //! point of the coordinate system of the ellipse.
  const Point3d& Location() const { return pos.Location(); }

  //! Returns the major radius of the ellipse.
  Standard_Real MajorRadius() const { return majorRadius; }

  //! Returns the minor radius of the ellipse.
  Standard_Real MinorRadius() const { return minorRadius; }

  //! Returns p = (1 - e * e) * MajorRadius where e is the eccentricity
  //! of the ellipse.
  //! Returns 0 if MajorRadius = 0
  Standard_Real Parameter() const;

  //! Returns the coordinate system of the ellipse.
  const Frame3d& Position() const { return pos; }

  //! Returns the "XAxis" of the ellipse whose origin
  //! is the center of this ellipse. It is the major axis of the
  //! ellipse.
  Axis3d XAxis() const { return Axis3d(pos.Location(), pos.XDirection()); }

  //! Returns the "YAxis" of the ellipse whose unit vector is the "X Direction" or the "Y Direction"
  //! of the local coordinate system of this ellipse.
  //! This is the minor axis of the ellipse.
  Axis3d YAxis() const { return Axis3d(pos.Location(), pos.YDirection()); }

  Standard_EXPORT void Mirror(const Point3d& theP);

  //! Performs the symmetrical transformation of an ellipse with
  //! respect to the point theP which is the center of the symmetry.
  Standard_NODISCARD Standard_EXPORT gp_Elips Mirrored(const Point3d& theP) const;

  Standard_EXPORT void Mirror(const Axis3d& theA1);

  //! Performs the symmetrical transformation of an ellipse with
  //! respect to an axis placement which is the axis of the symmetry.
  Standard_NODISCARD Standard_EXPORT gp_Elips Mirrored(const Axis3d& theA1) const;

  Standard_EXPORT void Mirror(const Frame3d& theA2);

  //! Performs the symmetrical transformation of an ellipse with
  //! respect to a plane. The axis placement theA2 locates the plane
  //! of the symmetry (Location, XDirection, YDirection).
  Standard_NODISCARD Standard_EXPORT gp_Elips Mirrored(const Frame3d& theA2) const;

  void Rotate(const Axis3d& theA1, const Standard_Real theAng) { pos.Rotate(theA1, theAng); }

  //! Rotates an ellipse. theA1 is the axis of the rotation.
  //! theAng is the angular value of the rotation in radians.
  Standard_NODISCARD gp_Elips Rotated(const Axis3d& theA1, const Standard_Real theAng) const
  {
    gp_Elips anE = *this;
    anE.pos.Rotate(theA1, theAng);
    return anE;
  }

  void Scale(const Point3d& theP, const Standard_Real theS);

  //! Scales an ellipse. theS is the scaling value.
  Standard_NODISCARD gp_Elips Scaled(const Point3d& theP, const Standard_Real theS) const;

  void Transform(const Transform3d& theT);

  //! Transforms an ellipse with the transformation theT from class Trsf.
  Standard_NODISCARD gp_Elips Transformed(const Transform3d& theT) const;

  void Translate(const Vector3d& theV) { pos.Translate(theV); }

  //! Translates an ellipse in the direction of the vector theV.
  //! The magnitude of the translation is the vector's magnitude.
  Standard_NODISCARD gp_Elips Translated(const Vector3d& theV) const
  {
    gp_Elips anE = *this;
    anE.pos.Translate(theV);
    return anE;
  }

  void Translate(const Point3d& theP1, const Point3d& theP2) { pos.Translate(theP1, theP2); }

  //! Translates an ellipse from the point theP1 to the point theP2.
  Standard_NODISCARD gp_Elips Translated(const Point3d& theP1, const Point3d& theP2) const
  {
    gp_Elips anE = *this;
    anE.pos.Translate(theP1, theP2);
    return anE;
  }

private:
  Frame3d        pos;
  Standard_Real majorRadius;
  Standard_Real minorRadius;
};

// =======================================================================
// function : Directrix1
// purpose  :
// =======================================================================
inline Axis3d gp_Elips::Directrix1() const
{
  Standard_Real anE = Eccentricity();
  Standard_ConstructionError_Raise_if(anE <= gp1::Resolution(),
                                      "gp_Elips::Directrix1() - zero eccentricity");
  Coords3d anOrig = pos.XDirection().XYZ();
  anOrig.Multiply(majorRadius / anE);
  anOrig.Add(pos.Location().XYZ());
  return Axis3d(Point3d(anOrig), pos.YDirection());
}

// =======================================================================
// function : Directrix2
// purpose  :
// =======================================================================
inline Axis3d gp_Elips::Directrix2() const
{
  Standard_Real anE = Eccentricity();
  Standard_ConstructionError_Raise_if(anE <= gp1::Resolution(),
                                      "gp_Elips::Directrix2() - zero eccentricity");
  Coords3d anOrig = pos.XDirection().XYZ();
  anOrig.Multiply(-majorRadius / anE);
  anOrig.Add(pos.Location().XYZ());
  return Axis3d(Point3d(anOrig), pos.YDirection());
}

// =======================================================================
// function : Eccentricity
// purpose  :
// =======================================================================
inline Standard_Real gp_Elips::Eccentricity() const
{
  if (majorRadius == 0.0)
  {
    return 0.0;
  }
  else
  {
    return sqrt(majorRadius * majorRadius - minorRadius * minorRadius) / majorRadius;
  }
}

// =======================================================================
// function : Focus1
// purpose  :
// =======================================================================
inline Point3d gp_Elips::Focus1() const
{
  Standard_Real aC  = sqrt(majorRadius * majorRadius - minorRadius * minorRadius);
  const Point3d& aPP = pos.Location();
  const Dir3d& aDD = pos.XDirection();
  return Point3d(aPP.X() + aC * aDD.X(), aPP.Y() + aC * aDD.Y(), aPP.Z() + aC * aDD.Z());
}

// =======================================================================
// function : Focus2
// purpose  :
// =======================================================================
inline Point3d gp_Elips::Focus2() const
{
  Standard_Real aC  = sqrt(majorRadius * majorRadius - minorRadius * minorRadius);
  const Point3d& aPP = pos.Location();
  const Dir3d& aDD = pos.XDirection();
  return Point3d(aPP.X() - aC * aDD.X(), aPP.Y() - aC * aDD.Y(), aPP.Z() - aC * aDD.Z());
}

// =======================================================================
// function : Parameter
// purpose  :
// =======================================================================
inline Standard_Real gp_Elips::Parameter() const
{
  if (majorRadius == 0.0)
  {
    return 0.0;
  }
  else
  {
    return (minorRadius * minorRadius) / majorRadius;
  }
}

// =======================================================================
// function : Scale
// purpose  :
// =======================================================================
inline void gp_Elips::Scale(const Point3d& theP, const Standard_Real theS)
//  Modified by skv - Fri Apr  8 10:28:10 2005 OCC8559 Begin
// { pos.Scale(P, S); }
{
  majorRadius *= theS;
  if (majorRadius < 0)
  {
    majorRadius = -majorRadius;
  }
  minorRadius *= theS;
  if (minorRadius < 0)
  {
    minorRadius = -minorRadius;
  }
  pos.Scale(theP, theS);
}

//  Modified by skv - Fri Apr  8 10:28:10 2005 OCC8559 End

// =======================================================================
// function : Scaled
// purpose  :
// =======================================================================
inline gp_Elips gp_Elips::Scaled(const Point3d& theP, const Standard_Real theS) const
{
  gp_Elips anE = *this;
  anE.majorRadius *= theS;
  if (anE.majorRadius < 0)
  {
    anE.majorRadius = -anE.majorRadius;
  }
  anE.minorRadius *= theS;
  if (anE.minorRadius < 0)
  {
    anE.minorRadius = -anE.minorRadius;
  }
  anE.pos.Scale(theP, theS);
  return anE;
}

// =======================================================================
// function : Transform
// purpose  :
// =======================================================================
inline void gp_Elips::Transform(const Transform3d& theT)
{
  majorRadius *= theT.ScaleFactor();
  if (majorRadius < 0)
  {
    majorRadius = -majorRadius;
  }
  minorRadius *= theT.ScaleFactor();
  if (minorRadius < 0)
  {
    minorRadius = -minorRadius;
  }
  pos.Transform(theT);
}

// =======================================================================
// function : Transformed
// purpose  :
// =======================================================================
inline gp_Elips gp_Elips::Transformed(const Transform3d& theT) const
{
  gp_Elips anE = *this;
  anE.majorRadius *= theT.ScaleFactor();
  if (anE.majorRadius < 0)
  {
    anE.majorRadius = -anE.majorRadius;
  }
  anE.minorRadius *= theT.ScaleFactor();
  if (anE.minorRadius < 0)
  {
    anE.minorRadius = -anE.minorRadius;
  }
  anE.pos.Transform(theT);
  return anE;
}

#endif // _gp_Elips_HeaderFile
