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

#include <Vrml_Transform.hxx>

Transform::Transform()
{
  Vector3d tmpV(0, 0, 0);
  myTranslation = tmpV;

  SFRotation tmpSFR(0, 0, 1, 0);
  myRotation = tmpSFR;

  tmpV.SetX(1);
  tmpV.SetY(1);
  tmpV.SetZ(1);
  myScaleFactor = tmpV;

  tmpSFR.SetRotationX(0);
  tmpSFR.SetRotationY(0);
  tmpSFR.SetRotationZ(1);
  tmpSFR.SetAngle(0);
  myScaleOrientation = tmpSFR;

  tmpV.SetX(0);
  tmpV.SetY(0);
  tmpV.SetZ(0);
  myCenter = tmpV;
}

Transform::Transform(const Vector3d&          aTranslation,
                               const SFRotation& aRotation,
                               const Vector3d&          aScaleFactor,
                               const SFRotation& aScaleOrientation,
                               const Vector3d&          aCenter)
{
  myTranslation      = aTranslation;
  myRotation         = aRotation;
  myScaleFactor      = aScaleFactor;
  myScaleOrientation = aScaleOrientation;
  myCenter           = aCenter;
}

void Transform::SetTranslation(const Vector3d& aTranslation)
{
  myTranslation = aTranslation;
}

Vector3d Transform::Translation() const
{
  return myTranslation;
}

void Transform::SetRotation(const SFRotation& aRotation)
{
  myRotation = aRotation;
}

SFRotation Transform::Rotation() const
{
  return myRotation;
}

void Transform::SetScaleFactor(const Vector3d& aScaleFactor)
{
  myScaleFactor = aScaleFactor;
}

Vector3d Transform::ScaleFactor() const
{
  return myScaleFactor;
}

void Transform::SetScaleOrientation(const SFRotation& aScaleOrientation)
{
  myScaleOrientation = aScaleOrientation;
}

SFRotation Transform::ScaleOrientation() const
{
  return myScaleOrientation;
}

void Transform::SetCenter(const Vector3d& aCenter)
{
  myCenter = aCenter;
}

Vector3d Transform::Center() const
{
  return myCenter;
}

Standard_OStream& Transform::Print(Standard_OStream& anOStream) const
{
  anOStream << "Transform {\n";

  if (Abs(myTranslation.X() - 0) > 0.0001 || Abs(myTranslation.Y() - 0) > 0.0001
      || Abs(myTranslation.Z() - 0) > 0.0001)
  {
    anOStream << "    translation\t\t";
    anOStream << myTranslation.X() << " " << myTranslation.Y() << " " << myTranslation.Z() << "\n";
  }

  if (Abs(myRotation.RotationX() - 0) > 0.0001 || Abs(myRotation.RotationY() - 0) > 0.0001
      || Abs(myRotation.RotationZ() - 1) > 0.0001 || Abs(myRotation.Angle() - 0) > 0.0001)
  {
    anOStream << "    rotation\t\t";
    anOStream << myRotation.RotationX() << " " << myRotation.RotationY() << " ";
    anOStream << myRotation.RotationZ() << " " << myRotation.Angle() << "\n";
  }

  if (Abs(myScaleFactor.X() - 1) > 0.0001 || Abs(myScaleFactor.Y() - 1) > 0.0001
      || Abs(myScaleFactor.Z() - 1) > 0.0001)
  {
    anOStream << "    scaleFactor\t\t";
    anOStream << myTranslation.X() << " " << myTranslation.Y() << " " << myTranslation.Z() << "\n";
  }

  if (Abs(myScaleOrientation.RotationX() - 0) > 0.0001
      || Abs(myScaleOrientation.RotationY() - 0) > 0.0001
      || Abs(myScaleOrientation.RotationZ() - 1) > 0.0001
      || Abs(myScaleOrientation.Angle() - 0) > 0.0001)
  {
    anOStream << "    scaleOrientation\t";
    anOStream << myScaleOrientation.RotationX() << " " << myScaleOrientation.RotationY() << " ";
    anOStream << myScaleOrientation.RotationZ() << " " << myScaleOrientation.Angle() << "\n";
  }

  if (Abs(myCenter.X() - 0) > 0.0001 || Abs(myCenter.Y() - 0) > 0.0001
      || Abs(myCenter.Z() - 0) > 0.0001)
  {
    anOStream << "    center\t\t";
    anOStream << myCenter.X() << " " << myCenter.Y() << " " << myCenter.Z() << "\n";
  }

  anOStream << "}\n";
  return anOStream;
}
