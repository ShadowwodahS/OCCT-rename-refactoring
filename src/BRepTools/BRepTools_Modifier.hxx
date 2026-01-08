// Created on: 1994-08-25
// Created by: Jacques GOUSSARD
// Copyright (c) 1994-1999 Matra Datavision
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

#ifndef _BRepTools_Modifier_HeaderFile
#define _BRepTools_Modifier_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

#include <NCollection_DataMap.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <TopLoc_Location.hxx>
#include <Message_ProgressRange.hxx>

class ShapeModification;
class GeomCurve3d;
class GeomSurface;

//! Performs geometric modifications on a shape.
class ShapeModifier
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates an empty Modifier.
  Standard_EXPORT ShapeModifier(Standard_Boolean theMutableInput = Standard_False);

  //! Creates a modifier on the shape <S>.
  Standard_EXPORT ShapeModifier(const TopoShape& S);

  //! Creates a modifier on  the shape <S>, and performs
  //! the modifications described by <M>.
  Standard_EXPORT ShapeModifier(const TopoShape&                   S,
                                     const Handle(ShapeModification)& M);

  //! Initializes the modifier with the shape <S>.
  Standard_EXPORT void Init(const TopoShape& S);

  //! Performs the modifications described by <M>.
  Standard_EXPORT void Perform(const Handle(ShapeModification)& M,
                               const Message_ProgressRange& theProgress = Message_ProgressRange());

  //! Returns Standard_True if the modification has
  //! been computed successfully.
  Standard_Boolean IsDone() const;

  //! Returns the current mutable input state
  Standard_EXPORT Standard_Boolean IsMutableInput() const;

  //! Sets the mutable input state
  //! If true then the input (original) shape can be modified
  //! during modification process
  Standard_EXPORT void SetMutableInput(Standard_Boolean theMutableInput);

  //! Returns the modified shape corresponding to <S>.
  const TopoShape& ModifiedShape(const TopoShape& S) const;

protected:
private:
  struct NewCurveInfo
  {
    Handle(GeomCurve3d) myCurve;
    TopLoc_Location    myLoc;
    Standard_Real      myToler;
  };

  struct NewSurfaceInfo
  {
    Handle(GeomSurface) mySurface;
    TopLoc_Location      myLoc;
    Standard_Real        myToler;
    Standard_Boolean     myRevWires;
    Standard_Boolean     myRevFace;
  };

  Standard_EXPORT void Put(const TopoShape& S);

  Standard_EXPORT Standard_Boolean
    Rebuild(const TopoShape&                   S,
            const Handle(ShapeModification)& M,
            Standard_Boolean&                     theNewGeom,
            const Message_ProgressRange&          theProgress = Message_ProgressRange());

  Standard_EXPORT void CreateNewVertices(const TopTools_IndexedDataMapOfShapeListOfShape& theMVE,
                                         const Handle(ShapeModification)&            M);

  Standard_EXPORT void FillNewCurveInfo(const TopTools_IndexedDataMapOfShapeListOfShape& theMEF,
                                        const Handle(ShapeModification)&            M);

  Standard_EXPORT void FillNewSurfaceInfo(const Handle(ShapeModification)& M);

  Standard_EXPORT void CreateOtherVertices(const TopTools_IndexedDataMapOfShapeListOfShape& theMVE,
                                           const TopTools_IndexedDataMapOfShapeListOfShape& theMEF,
                                           const Handle(ShapeModification)&            M);

  TopTools_DataMapOfShapeShape                                              myMap;
  TopoShape                                                              myShape;
  Standard_Boolean                                                          myDone;
  NCollection_DataMap<TopoEdge, NewCurveInfo, ShapeHasher>   myNCInfo;
  NCollection_DataMap<TopoFace, NewSurfaceInfo, ShapeHasher> myNSInfo;
  TopTools_MapOfShape                                                       myNonUpdFace;
  TopTools_MapOfShape                                                       myHasNewGeom;
  Standard_Boolean                                                          myMutableInput;
};

#include <BRepTools_Modifier.lxx>

#endif // _BRepTools_Modifier_HeaderFile
