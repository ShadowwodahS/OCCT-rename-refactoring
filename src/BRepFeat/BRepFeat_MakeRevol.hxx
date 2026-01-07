// Created on: 1996-02-13
// Created by: Jacques GOUSSARD
// Copyright (c) 1996-1999 Matra Datavision
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

#ifndef _BRepFeat_MakeRevol_HeaderFile
#define _BRepFeat_MakeRevol_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <gp_Ax1.hxx>
#include <BRepFeat_StatusError.hxx>
#include <BRepFeat_Form.hxx>
#include <Standard_Integer.hxx>
class GeomCurve3d;
class TopoFace;
class TopoEdge;

//! Describes functions to build revolved shells from basis shapes.
class BRepFeat_MakeRevol : public BRepFeat_Form
{
public:
  DEFINE_STANDARD_ALLOC

  //! initializes the revolved shell class.
  BRepFeat_MakeRevol();

  //! a face Pbase is selected in the
  //! shape Sbase to serve as the basis for the
  //! revolved shell. The revolution will be defined
  //! by the axis Axis and Fuse offers a choice between:
  //! -   removing matter with a Boolean cut using the setting 0
  //! -   adding matter with Boolean fusion using the setting 1.
  //! The sketch face Skface serves to determine
  //! the type of operation. If it is inside the basis
  //! shape, a local operation such as glueing can be performed.
  BRepFeat_MakeRevol(const TopoShape&    Sbase,
                     const TopoShape&    Pbase,
                     const TopoFace&     Skface,
                     const Axis3d&          Axis,
                     const Standard_Integer Fuse,
                     const Standard_Boolean Modify);

  Standard_EXPORT void Init(const TopoShape&    Sbase,
                            const TopoShape&    Pbase,
                            const TopoFace&     Skface,
                            const Axis3d&          Axis,
                            const Standard_Integer Fuse,
                            const Standard_Boolean Modify);

  //! Indicates that the edge <E> will slide on the face
  //! <OnFace>. Raises ConstructionError if the  face does not belong to the
  //! basis shape, or the edge to the prismed shape.
  Standard_EXPORT void Add(const TopoEdge& E, const TopoFace& OnFace);

  Standard_EXPORT void Perform(const Standard_Real Angle);

  Standard_EXPORT void Perform(const TopoShape& Until);

  //! Reconstructs the feature topologically.
  Standard_EXPORT void Perform(const TopoShape& From, const TopoShape& Until);

  //! Builds an infinite shell. The infinite descendants
  //! will not be kept in the result.
  Standard_EXPORT void PerformThruAll();

  //! Assigns both a limiting shape, Until from
  //! TopoShape, and an angle, Angle at
  //! which to stop generation of the revolved shell feature.
  Standard_EXPORT void PerformUntilAngle(const TopoShape& Until, const Standard_Real Angle);

  Standard_EXPORT void Curves(TColGeom_SequenceOfCurve& S);

  Standard_EXPORT Handle(GeomCurve3d) BarycCurve();

protected:
private:
  TopoShape                       myPbase;
  TopTools_DataMapOfShapeListOfShape mySlface;
  Axis3d                             myAxis;
  TColGeom_SequenceOfCurve           myCurves;
  Handle(GeomCurve3d)                 myBCurve;
  BRepFeat_StatusError               myStatusError;
};

#include <BRepFeat_MakeRevol.lxx>

#endif // _BRepFeat_MakeRevol_HeaderFile
