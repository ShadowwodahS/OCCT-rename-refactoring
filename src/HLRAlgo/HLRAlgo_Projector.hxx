// Created on: 1992-03-12
// Created by: Christophe MARION
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _HLRAlgo_Projector_HeaderFile
#define _HLRAlgo_Projector_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <Standard_Integer.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec2d.hxx>
class Frame3d;
class Vector3d;
class Point3d;
class gp_Pnt2d;
class gp_Lin;

//! Implements a  projector object.
//! To transform and project Points and Planes.
//! This object is designed to be used in the
//! removal of hidden lines and is returned by the
//! Prs3d_Projector::Projector function.
//! You define the projection of the selected shape
//! by calling one of the following functions:
//! -   HLRBRep_Algo::Projector, or
//! -   HLRBRep_PolyAlgo::Projector
//! The choice depends on the algorithm, which you are using.
//! The parameters of the view are defined at the
//! time of construction of a Prs3d_Projector object.
class HLRAlgo_Projector
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT HLRAlgo_Projector();

  //! Creates   an axonometric  projector.   <CS> is the
  //! viewing coordinate system.
  Standard_EXPORT HLRAlgo_Projector(const Frame3d& CS);

  //! Creates  a  perspective  projector.   <CS>  is the
  //! viewing coordinate system.
  Standard_EXPORT HLRAlgo_Projector(const Frame3d& CS, const Standard_Real Focus);

  //! build a Projector with automatic minmax directions.
  Standard_EXPORT HLRAlgo_Projector(const Transform3d&         T,
                                    const Standard_Boolean Persp,
                                    const Standard_Real    Focus);

  //! build a Projector with given minmax directions.
  Standard_EXPORT HLRAlgo_Projector(const Transform3d&         T,
                                    const Standard_Boolean Persp,
                                    const Standard_Real    Focus,
                                    const gp_Vec2d&        v1,
                                    const gp_Vec2d&        v2,
                                    const gp_Vec2d&        v3);

  Standard_EXPORT void Set(const Transform3d&         T,
                           const Standard_Boolean Persp,
                           const Standard_Real    Focus);

  void Directions(gp_Vec2d& D1, gp_Vec2d& D2, gp_Vec2d& D3) const;

  //! to compute with the given scale and translation.
  Standard_EXPORT void Scaled(const Standard_Boolean On = Standard_False);

  //! Returns True if there is a perspective transformation.
  Standard_Boolean Perspective() const;

  //! Returns the active transformation.
  Standard_EXPORT const Transform3d& Transformation() const;

  //! Returns the active inverted transformation.
  const Transform3d& InvertedTransformation() const;

  //! Returns the original transformation.
  const Transform3d& FullTransformation() const;

  //! Returns the focal length.
  Standard_Real Focus() const;

  void Transform(Vector3d& D) const;

  void Transform(Point3d& Pnt) const;

  //! Transform and apply perspective if needed.
  Standard_EXPORT void Project(const Point3d& P, gp_Pnt2d& Pout) const;

  //! Transform and apply perspective if needed.
  Standard_EXPORT void Project(const Point3d&  P,
                               Standard_Real& X,
                               Standard_Real& Y,
                               Standard_Real& Z) const;

  //! Transform and apply perspective if needed.
  Standard_EXPORT void Project(const Point3d& P,
                               const Vector3d& D1,
                               gp_Pnt2d&     Pout,
                               gp_Vec2d&     D1out) const;

  //! return a line going through the eye towards the
  //! 2d point <X,Y>.
  Standard_EXPORT gp_Lin Shoot(const Standard_Real X, const Standard_Real Y) const;

protected:
private:
  Standard_EXPORT void SetDirection();

  Standard_Integer myType;
  Standard_Boolean myPersp;
  Standard_Real    myFocus;
  Transform3d          myScaledTrsf;
  Transform3d          myTrsf;
  Transform3d          myInvTrsf;
  gp_Vec2d         myD1;
  gp_Vec2d         myD2;
  gp_Vec2d         myD3;
};

#include <HLRAlgo_Projector.lxx>

#endif // _HLRAlgo_Projector_HeaderFile
