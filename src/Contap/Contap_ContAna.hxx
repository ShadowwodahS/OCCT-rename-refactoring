// Created on: 1993-03-04
// Created by: Jacques GOUSSARD
// Copyright (c) 1993-1999 Matra Datavision
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

#ifndef _Contap_ContAna_HeaderFile
#define _Contap_ContAna_HeaderFile

#include <GeomAbs_CurveType.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <Standard.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

class gp_Sphere;
class Cylinder1;
class Cone1;
class gp_Lin;

//! This class provides the computation of the contours
//! for quadric surfaces.
class ContourAnalyzer
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT ContourAnalyzer();

  Standard_EXPORT void Perform(const gp_Sphere& S, const Dir3d& D);

  Standard_EXPORT void Perform(const gp_Sphere& S, const Dir3d& D, const Standard_Real Ang);

  Standard_EXPORT void Perform(const gp_Sphere& S, const Point3d& Eye);

  Standard_EXPORT void Perform(const Cylinder1& C, const Dir3d& D);

  Standard_EXPORT void Perform(const Cylinder1& C, const Dir3d& D, const Standard_Real Ang);

  Standard_EXPORT void Perform(const Cylinder1& C, const Point3d& Eye);

  Standard_EXPORT void Perform(const Cone1& C, const Dir3d& D);

  Standard_EXPORT void Perform(const Cone1& C, const Dir3d& D, const Standard_Real Ang);

  Standard_EXPORT void Perform(const Cone1& C, const Point3d& Eye);

  Standard_Boolean IsDone() const;

  Standard_Integer NbContours() const;

  //! Returns GeomAbs_Line or GeomAbs_Circle, when
  //! IsDone() returns True.
  GeomAbs_CurveType TypeContour() const;

  gp_Circ Circle() const;

  Standard_EXPORT gp_Lin Line(const Standard_Integer Index) const;

protected:
private:
  Standard_Boolean  done;
  Standard_Integer  nbSol;
  GeomAbs_CurveType typL;
  Point3d            pt1;
  Point3d            pt2;
  Point3d            pt3;
  Point3d            pt4;
  Dir3d            dir1;
  Dir3d            dir2;
  Dir3d            dir3;
  Dir3d            dir4;
  Standard_Real     prm;
};

#include <Contap_ContAna.lxx>

#endif // _Contap_ContAna_HeaderFile
