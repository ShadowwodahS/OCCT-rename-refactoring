// Created on: 1997-12-15
// Created by: Philippe MANGIN
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _GeomFill_SectionPlacement_HeaderFile
#define _GeomFill_SectionPlacement_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <gp_Ax1.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Extrema_ExtPC.hxx>
#include <gp_Pnt.hxx>
class LocationLaw;
class GeomCurve3d;
class Geometry3;
class Transform3d;
class gp_Mat;
class Vector3d;

//! To place section in sweep Function
class GeomFill_SectionPlacement
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT GeomFill_SectionPlacement(const Handle(LocationLaw)& L,
                                            const Handle(Geometry3)&        Section);

  //! To change the section Law1
  Standard_EXPORT void SetLocation(const Handle(LocationLaw)& L);

  Standard_EXPORT void Perform(const Standard_Real Tol);

  Standard_EXPORT void Perform(const Handle(Curve5)& Path, const Standard_Real Tol);

  Standard_EXPORT void Perform(const Standard_Real ParamOnPath, const Standard_Real Tol);

  Standard_EXPORT Standard_Boolean IsDone() const;

  Standard_EXPORT Standard_Real ParameterOnPath() const;

  Standard_EXPORT Standard_Real ParameterOnSection() const;

  Standard_EXPORT Standard_Real Distance() const;

  Standard_EXPORT Standard_Real Angle() const;

  Standard_EXPORT Transform3d
    Transformation(const Standard_Boolean WithTranslation,
                   const Standard_Boolean WithCorrection = Standard_False) const;

  //! Compute the Section, in the coordinate system given by
  //! the Location Law1.
  //! If <WithTranslation> contact between
  //! <Section> and <Path> is forced.
  Standard_EXPORT Handle(GeomCurve3d) Section(const Standard_Boolean WithTranslation) const;

  //! Compute the Section, in the coordinate system given by
  //! the Location Law1.
  //! To have the Normal to section equal to the Location
  //! Law1 Normal.  If <WithTranslation> contact between
  //! <Section> and <Path> is forced.
  Standard_EXPORT Handle(GeomCurve3d) ModifiedSection(const Standard_Boolean WithTranslation) const;

protected:
private:
  Standard_EXPORT void SectionAxis(const gp_Mat& M, Vector3d& T, Vector3d& N, Vector3d& BN) const;

  Standard_EXPORT Standard_Boolean Choix(const Standard_Real Dist, const Standard_Real Angle) const;

  Standard_Boolean             done;
  Standard_Boolean             isplan;
  Axis3d                       TheAxe;
  Standard_Real                Gabarit;
  Handle(LocationLaw) myLaw;
  GeomAdaptor_Curve            myAdpSection;
  Handle(GeomCurve3d)           mySection;
  Standard_Real                SecParam;
  Standard_Real                PathParam;
  Standard_Real                Dist;
  Standard_Real                AngleMax;
  Extrema_ExtPC                myExt;
  Standard_Boolean             myIsPoint;
  Point3d                       myPoint;
};

#endif // _GeomFill_SectionPlacement_HeaderFile
