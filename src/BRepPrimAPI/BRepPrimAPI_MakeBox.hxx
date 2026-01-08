// Created on: 1993-07-21
// Created by: Remi LEQUETTE
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

#ifndef _BRepPrimAPI_MakeBox_HeaderFile
#define _BRepPrimAPI_MakeBox_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <BRepPrim_Wedge.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
class Point3d;
class Frame3d;
class TopoShell;
class TopoSolid;
class TopoFace;

//! Describes functions to build parallelepiped boxes.
//! A MakeBox object provides a framework for:
//! -   defining the construction of a box,
//! -   implementing the construction algorithm, and
//! -   consulting the result.
//! Constructs a box such that its sides are parallel to the axes of
//! -   the global coordinate system, or
//! -   the local coordinate system Axis. and
//! -   with a corner at (0, 0, 0) and of size (dx, dy, dz), or
//! -   with a corner at point P and of size (dx, dy, dz), or
//! -   with corners at points P1 and P2.
//! Exceptions
//! Standard_DomainError if: dx, dy, dz are less than or equal to
//! Precision1::Confusion(), or
//! -   the vector joining the points P1 and P2 has a
//! component projected onto the global coordinate
//! system less than or equal to Precision1::Confusion().
//! In these cases, the box would be flat.

class BoxMaker : public BRepBuilderAPI_MakeShape
{
public:
  DEFINE_STANDARD_ALLOC

  //! Default constructor
  BoxMaker() {}

  //! Make a box with a corner at 0,0,0 and the other dx,dy,dz
  Standard_EXPORT BoxMaker(const Standard_Real dx,
                                      const Standard_Real dy,
                                      const Standard_Real dz);

  //! Make a box with a corner at P and size dx, dy, dz.
  Standard_EXPORT BoxMaker(const Point3d&       P,
                                      const Standard_Real dx,
                                      const Standard_Real dy,
                                      const Standard_Real dz);

  //! Make a box with corners P1,P2.
  Standard_EXPORT BoxMaker(const Point3d& P1, const Point3d& P2);

  //! Make a box with Ax2 (the left corner and the axis) and size dx, dy, dz.
  Standard_EXPORT BoxMaker(const Frame3d&       Axes,
                                      const Standard_Real dx,
                                      const Standard_Real dy,
                                      const Standard_Real dz);

  //! Init a box with a corner at 0,0,0 and the other theDX, theDY, theDZ
  Standard_EXPORT void Init(const Standard_Real theDX,
                            const Standard_Real theDY,
                            const Standard_Real theDZ);

  //! Init a box with a corner at thePnt and size theDX, theDY, theDZ.
  Standard_EXPORT void Init(const Point3d&       thePnt,
                            const Standard_Real theDX,
                            const Standard_Real theDY,
                            const Standard_Real theDZ);

  //! Init a box with corners thePnt1, thePnt2.
  Standard_EXPORT void Init(const Point3d& thePnt1, const Point3d& thePnt2);

  //! Init a box with Ax2 (the left corner and the theAxes) and size theDX, theDY, theDZ.
  Standard_EXPORT void Init(const Frame3d&       theAxes,
                            const Standard_Real theDX,
                            const Standard_Real theDY,
                            const Standard_Real theDZ);

  //! Returns the internal algorithm.
  Standard_EXPORT BRepPrim_Wedge& Wedge();

  //! Stores the solid in myShape.
  Standard_EXPORT virtual void Build(
    const Message_ProgressRange& theRange = Message_ProgressRange()) Standard_OVERRIDE;

  //! Returns the constructed box as a shell.
  Standard_EXPORT const TopoShell& Shell();
  Standard_EXPORT                     operator TopoShell();

  //! Returns the constructed box as a solid.
  Standard_EXPORT const TopoSolid& Solid();
  Standard_EXPORT                     operator TopoSolid();

  //! Returns ZMin face
  Standard_EXPORT const TopoFace& BottomFace();

  //! Returns XMin face
  Standard_EXPORT const TopoFace& BackFace();

  //! Returns XMax face
  Standard_EXPORT const TopoFace& FrontFace();

  //! Returns YMin face
  Standard_EXPORT const TopoFace& LeftFace();

  //! Returns YMax face
  Standard_EXPORT const TopoFace& RightFace();

  //! Returns ZMax face
  Standard_EXPORT const TopoFace& TopFace();

protected:
  BRepPrim_Wedge myWedge;

private:
};

#endif // _BRepPrimAPI_MakeBox_HeaderFile
