// Created on: 1992-04-13
// Created by: Jacques GOUSSARD
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

#ifndef _IntSurf_Quadric_HeaderFile
#define _IntSurf_Quadric_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <gp_Ax3.hxx>
#include <gp_Lin.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Torus.hxx>
#include <Standard_Integer.hxx>
class gp_Pln;
class Cylinder1;
class Sphere3;
class Cone1;
class gp_Torus;
class Point3d;
class Vector3d;

class Quadric1
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT Quadric1();

  Standard_EXPORT Quadric1(const gp_Pln& P);

  Standard_EXPORT Quadric1(const Cylinder1& C);

  Standard_EXPORT Quadric1(const Sphere3& S);

  Standard_EXPORT Quadric1(const Cone1& C);

  Standard_EXPORT Quadric1(const gp_Torus& T);

  Standard_EXPORT void SetValue(const gp_Pln& P);

  Standard_EXPORT void SetValue(const Cylinder1& C);

  Standard_EXPORT void SetValue(const Sphere3& S);

  Standard_EXPORT void SetValue(const Cone1& C);

  Standard_EXPORT void SetValue(const gp_Torus& T);

  Standard_EXPORT Standard_Real Distance(const Point3d& P) const;

  Standard_EXPORT Vector3d Gradient(const Point3d& P) const;

  Standard_EXPORT void ValAndGrad(const Point3d& P, Standard_Real& Dist, Vector3d& Grad) const;

  GeomAbs_SurfaceType TypeQuadric() const;

  gp_Pln Plane1() const;

  Sphere3 Sphere() const;

  Cylinder1 Cylinder() const;

  Cone1 Cone() const;

  gp_Torus Torus() const;

  Standard_EXPORT Point3d Value(const Standard_Real U, const Standard_Real V) const;

  Standard_EXPORT void D1(const Standard_Real U,
                          const Standard_Real V,
                          Point3d&             P,
                          Vector3d&             D1U,
                          Vector3d&             D1V) const;

  Standard_EXPORT Vector3d DN(const Standard_Real    U,
                            const Standard_Real    V,
                            const Standard_Integer Nu,
                            const Standard_Integer Nv) const;

  Standard_EXPORT Vector3d Normale(const Standard_Real U, const Standard_Real V) const;

  Standard_EXPORT void Parameters(const Point3d& P, Standard_Real& U, Standard_Real& V) const;

  Standard_EXPORT Vector3d Normale(const Point3d& P) const;

protected:
private:
  Ax3              ax3;
  gp_Lin              lin;
  GeomAbs_SurfaceType typ;
  Standard_Real       prm1;
  Standard_Real       prm2;
  Standard_Real       prm3;
  Standard_Real       prm4;
  Standard_Boolean    ax3direc;
};

#include <IntSurf_Quadric.lxx>

#endif // _IntSurf_Quadric_HeaderFile
