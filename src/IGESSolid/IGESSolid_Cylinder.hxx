// Created on: 1993-01-09
// Created by: CKY / Contract Toubro-Larsen ( SIVA )
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

#ifndef _IGESSolid_Cylinder_HeaderFile
#define _IGESSolid_Cylinder_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <gp_XYZ.hxx>
#include <IGESData_IGESEntity.hxx>
class Point3d;
class Dir3d;

class IGESSolid_Cylinder;
DEFINE_STANDARD_HANDLE(IGESSolid_Cylinder, IGESData_IGESEntity)

//! defines Cylinder, Type <154> Form Number <0>
//! in package IGESSolid1
//! This defines a solid cylinder
class IGESSolid_Cylinder : public IGESData_IGESEntity
{

public:
  Standard_EXPORT IGESSolid_Cylinder();

  //! This method is used to set the fields of the class
  //! Cylinder
  //! - aHeight : Cylinder height
  //! - aRadius : Cylinder radius
  //! - aCenter : First face center coordinates (default (0,0,0))
  //! - anAxis  : Unit vector in axis direction (default (0,0,1))
  Standard_EXPORT void Init(const Standard_Real aHeight,
                            const Standard_Real aRadius,
                            const Coords3d&       aCenter,
                            const Coords3d&       anAxis);

  //! returns the cylinder height
  Standard_EXPORT Standard_Real Height() const;

  //! returns the cylinder radius
  Standard_EXPORT Standard_Real Radius() const;

  //! returns the first face center coordinates.
  Standard_EXPORT Point3d FaceCenter() const;

  //! returns the first face center after applying TransformationMatrix
  Standard_EXPORT Point3d TransformedFaceCenter() const;

  //! returns the vector in axis direction
  Standard_EXPORT Dir3d Axis() const;

  //! returns the vector in axis direction after applying
  //! TransformationMatrix
  Standard_EXPORT Dir3d TransformedAxis() const;

  DEFINE_STANDARD_RTTIEXT(IGESSolid_Cylinder, IGESData_IGESEntity)

protected:
private:
  Standard_Real theHeight;
  Standard_Real theRadius;
  Coords3d        theFaceCenter;
  Coords3d        theAxis;
};

#endif // _IGESSolid_Cylinder_HeaderFile
