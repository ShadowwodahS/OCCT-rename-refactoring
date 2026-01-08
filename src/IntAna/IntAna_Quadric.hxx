// Created on: 1992-07-01
// Created by: Laurent BUCHARD
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

#ifndef _IntAna_Quadric_HeaderFile
#define _IntAna_Quadric_HeaderFile

#include <NCollection_List.hxx>
#include <Standard_DefineAlloc.hxx>

class Cone1;
class Cylinder1;
class gp_Pln;
class Sphere3;

//! This class provides a description of Quadrics by their
//! Coefficients in natural coordinate system.
class Quadric2
{
public:
  DEFINE_STANDARD_ALLOC

  //! Empty Constructor
  Standard_EXPORT Quadric2();

  //! Creates a Quadric from a Pln
  Standard_EXPORT Quadric2(const gp_Pln& P);

  //! Creates a Quadric from a Sphere
  Standard_EXPORT Quadric2(const Sphere3& Sph);

  //! Creates a Quadric from a Cylinder
  Standard_EXPORT Quadric2(const Cylinder1& Cyl);

  //! Creates a Quadric from a Cone
  Standard_EXPORT Quadric2(const Cone1& Cone);

  //! Initializes the quadric with a Pln
  Standard_EXPORT void SetQuadric(const gp_Pln& P);

  //! Initialize the quadric with a Sphere
  Standard_EXPORT void SetQuadric(const Sphere3& Sph);

  //! Initializes the quadric with a Cone
  Standard_EXPORT void SetQuadric(const Cone1& Con);

  //! Initializes the quadric with a Cylinder
  Standard_EXPORT void SetQuadric(const Cylinder1& Cyl);

  //! Returns the coefficients of the polynomial equation
  //! which define the quadric:
  //! xCXX x**2 + xCYY y**2 + xCZZ z**2
  //! + 2 ( xCXY x y  + xCXZ x z  + xCYZ y z  )
  //! + 2 ( xCX x + xCY y + xCZ z )
  //! + xCCte
  Standard_EXPORT void Coefficients(Standard_Real& xCXX,
                                    Standard_Real& xCYY,
                                    Standard_Real& xCZZ,
                                    Standard_Real& xCXY,
                                    Standard_Real& xCXZ,
                                    Standard_Real& xCYZ,
                                    Standard_Real& xCX,
                                    Standard_Real& xCY,
                                    Standard_Real& xCZ,
                                    Standard_Real& xCCte) const;

  //! Returns the coefficients of the polynomial equation
  //! ( written in the natural coordinates system )
  //! in the local coordinates system defined by Axis
  Standard_EXPORT void NewCoefficients(Standard_Real& xCXX,
                                       Standard_Real& xCYY,
                                       Standard_Real& xCZZ,
                                       Standard_Real& xCXY,
                                       Standard_Real& xCXZ,
                                       Standard_Real& xCYZ,
                                       Standard_Real& xCX,
                                       Standard_Real& xCY,
                                       Standard_Real& xCZ,
                                       Standard_Real& xCCte,
                                       const Ax3&  Axis) const;

  //! Returns the list of special points (with singularities)
  const NCollection_List<Point3d>& SpecialPoints() const { return mySpecialPoints; }

protected:
private:
  Standard_Real            CXX;
  Standard_Real            CYY;
  Standard_Real            CZZ;
  Standard_Real            CXY;
  Standard_Real            CXZ;
  Standard_Real            CYZ;
  Standard_Real            CX;
  Standard_Real            CY;
  Standard_Real            CZ;
  Standard_Real            CCte;
  NCollection_List<Point3d> mySpecialPoints;
};

#endif // _IntAna_Quadric_HeaderFile
