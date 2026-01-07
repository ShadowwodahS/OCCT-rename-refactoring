// Created on: 1999-01-13
// Created by: Philippe MANGIN
// Copyright (c) 1999 Matra Datavision
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

#ifndef _BRepOffsetAPI_MakeDraft_HeaderFile
#define _BRepOffsetAPI_MakeDraft_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <BRepFill_Draft.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <Standard_Real.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <TopTools_ListOfShape.hxx>
class TopoShape;
class Dir3d;
class GeomSurface;
class TopoShell;

//! Build a draft surface along a wire
class BRepOffsetAPI_MakeDraft : public BRepBuilderAPI_MakeShape
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructs the draft surface object defined by the shape
  //! Shape, the direction Dir, and the angle Angle.
  //! Shape must be a TopoWire, Topo_DS_Face or
  //! TopoShell with free boundaries.
  //! Exceptions
  //! Standard_NotDone if Shape is not a TopoWire,
  //! Topo_DS_Face or TopoShell with free boundaries.
  Standard_EXPORT BRepOffsetAPI_MakeDraft(const TopoShape& Shape,
                                          const Dir3d&       Dir,
                                          const Standard_Real Angle);

  //! Sets the options of this draft tool.
  //! If a transition has to be performed, it can be defined by
  //! the mode Style as RightCorner or RoundCorner,
  //! RightCorner being a corner defined by a sharp angle,
  //! and RoundCorner being a rounded corner.
  //! AngleMin is an angular tolerance used to detect
  //! whether a transition has to be performed or not.
  //! AngleMax sets the maximum value within which a
  //! RightCorner transition can be performed.
  //! AngleMin and AngleMax are expressed in radians.
  Standard_EXPORT void SetOptions(
    const BRepBuilderAPI_TransitionMode Style    = BRepBuilderAPI_RightCorner,
    const Standard_Real                 AngleMin = 0.01,
    const Standard_Real                 AngleMax = 3.0);

  //! Sets the direction of the draft for this object.
  //! If IsInternal is true, the draft is internal to the argument
  //! Shape used in the constructor.
  Standard_EXPORT void SetDraft(const Standard_Boolean IsInternal = Standard_False);

  //! Performs the draft using the length LengthMax as the
  //! maximum length for the corner edge between two draft faces.
  Standard_EXPORT void Perform(const Standard_Real LengthMax);

  //! Performs the draft up to the surface Surface.
  //! If KeepInsideSurface is true, the part of Surface inside
  //! the draft is kept in the result.
  Standard_EXPORT void Perform(const Handle(GeomSurface)& Surface,
                               const Standard_Boolean      KeepInsideSurface = Standard_True);

  //! Performs the draft up to the shape StopShape.
  //! If KeepOutSide is true, the part of StopShape which is
  //! outside the Draft is kept in the result.
  Standard_EXPORT void Perform(const TopoShape&    StopShape,
                               const Standard_Boolean KeepOutSide = Standard_True);

  //! Returns the shell resulting from performance of the
  //! draft along the wire.
  Standard_EXPORT TopoShell Shell() const;

  //! Returns the  list   of shapes generated   from the
  //! shape <S>.
  Standard_EXPORT virtual const ShapeList& Generated(const TopoShape& S)
    Standard_OVERRIDE;

protected:
private:
  BRepFill_Draft myDraft;
};

#endif // _BRepOffsetAPI_MakeDraft_HeaderFile
