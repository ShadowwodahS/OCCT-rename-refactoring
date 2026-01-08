// Created on: 1998-06-03
// Created by: data exchange team
// Copyright (c) 1998-1999 Matra Datavision
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

#ifndef _BRepTools_ReShape_HeaderFile
#define _BRepTools_ReShape_HeaderFile

#include <BRepTools_History.hxx>

#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <Standard_Integer.hxx>
#include <Standard_Transient.hxx>
#include <TopAbs_ShapeEnum.hxx>

class TopoVertex;

// resolve name collisions with X11 headers
#ifdef Status
  #undef Status
#endif

class ShapeReShaper;
DEFINE_STANDARD_HANDLE(ShapeReShaper, RefObject)

//! Rebuilds a Shape by making pre-defined substitutions on some
//! of its components
//!
//! In a first phase, it records requests to replace or remove
//! some individual shapes
//! For each shape, the last given request is recorded
//! Requests may be applied "Oriented" (i.e. only to an item with
//! the SAME orientation) or not (the orientation of replacing
//! shape is respectful of that of the original one)
//!
//! Then, these requests may be applied to any shape which may
//! contain one or more of these individual shapes
//!
//! Supports the 'ShapeHistory' history by method 'History'.
class ShapeReShaper : public RefObject
{
public:
  //! Returns an empty Reshape
  Standard_EXPORT ShapeReShaper();

  //! Clears all substitutions requests
  Standard_EXPORT virtual void Clear();

  //! Sets a request to Remove a Shape whatever the orientation
  Standard_EXPORT virtual void Remove(const TopoShape& shape);

  //! Sets a request to Replace a Shape by a new one.
  virtual void Replace(const TopoShape& shape, const TopoShape& newshape)
  {
    replace(shape, newshape, TReplacementKind_Modify);
  }

  //! Merges the parts to the single product.
  //! The first part is replaced by the product.
  //! The other parts are removed.
  //! The history of the merged shapes is presented by equal ways.
  template <typename TCollection1>
  void Merge(const TCollection1& theParts, const TopoShape& theProduct)
  {
    typename TCollection1::Iterator aPIt(theParts);

    if (aPIt.More())
    {
      replace(aPIt.Value(), theProduct, TReplacementKind_Merge_Main);

      aPIt.Next();
    }

    const TReplacementKind aKind = TReplacementKind_Merge_Ordinary;
    for (; aPIt.More(); aPIt.Next())
    {
      replace(aPIt.Value(), theProduct, aKind);
    }
  }

  //! Tells if a shape is recorded for Replace/Remove
  Standard_EXPORT virtual Standard_Boolean IsRecorded(const TopoShape& shape) const;

  //! Returns the new value for an individual shape
  //! If not recorded, returns the original shape itself
  //! If to be Removed, returns a Null Shape
  //! Else, returns the replacing item
  Standard_EXPORT virtual TopoShape Value(const TopoShape& shape) const;

  //! Returns a complete substitution status for a shape
  //! 0  : not recorded,   <newsh> = original <shape>
  //! < 0: to be removed,  <newsh> is NULL
  //! > 0: to be replaced, <newsh> is a new item
  //! If <last> is False, returns status and new shape recorded in
  //! the map directly for the shape, if True and status > 0 then
  //! recursively searches for the last status and new shape.
  Standard_EXPORT virtual Standard_Integer Status(const TopoShape&    shape,
                                                  TopoShape&          newsh,
                                                  const Standard_Boolean last = Standard_False);

  //! Applies the substitutions requests to a shape.
  //!
  //! theUntil gives the level of type until which requests are taken into account.
  //! For subshapes of the type <until> no rebuild and further exploring are done.
  //!
  //! NOTE: each subshape can be replaced by shape of the same type
  //! or by shape containing only shapes of that type
  //! (for example, TopoEdge can be replaced by TopoEdge,
  //! TopoWire or TopoCompound containing TopoDS_Edges).
  //! If incompatible shape type is encountered, it is ignored and flag FAIL1 is set in Status.
  Standard_EXPORT virtual TopoShape Apply(const TopoShape&    theShape,
                                             const TopAbs_ShapeEnum theUntil = TopAbs_SHAPE);

  //! Returns (modifiable) the flag which defines whether Location of shape take into account
  //! during replacing shapes.
  virtual Standard_Boolean& ModeConsiderLocation() { return myConsiderLocation; }

  //! Returns modified copy of vertex if original one is not recorded or returns modified original
  //! vertex otherwise.
  //@param theV - original vertex.
  //@param theTol - new tolerance of vertex, optional.
  Standard_EXPORT TopoVertex CopyVertex(const TopoVertex& theV,
                                           const Standard_Real  theTol = -1.0);

  //! Returns modified copy of vertex if original one is not recorded or returns modified original
  //! vertex otherwise.
  //@param theV - original vertex.
  //@param theNewPos - new position for vertex copy.
  //@param theTol - new tolerance of vertex.
  Standard_EXPORT TopoVertex CopyVertex(const TopoVertex& theV,
                                           const Point3d&        theNewPos,
                                           const Standard_Real  aTol);

  //! Checks if shape has been recorded by reshaper as a value
  //@param theShape is the given shape
  Standard_EXPORT Standard_Boolean IsNewShape(const TopoShape& theShape) const;

  //! Returns the history of the substituted shapes.
  Standard_EXPORT Handle(ShapeHistory) History() const;

  DEFINE_STANDARD_RTTIEXT(ShapeReShaper, RefObject)

protected:
  //! The kinds of the replacements.
  enum TReplacementKind
  {
    TReplacementKind_Remove         = 1,
    TReplacementKind_Modify         = 2,
    TReplacementKind_Merge_Main     = 4,
    TReplacementKind_Merge_Ordinary = 8
  };

  //! Replaces the first shape by the second one
  //! after the following reorientation.
  //!
  //! If the first shape has the reversed orientation
  //! then the both shapes are reversed.
  //! If the first shape has the internal or external orientation then: <br>
  //! - the second shape is oriented forward (reversed) if it's orientation
  //!   is equal (not equal) to the orientation of the first shape; <br>
  //! - the first shape is oriented forward.
  Standard_EXPORT virtual void replace(const TopoShape&    shape,
                                       const TopoShape&    newshape,
                                       const TReplacementKind theKind);

private:
  //! Returns 'true' if the kind of a replacement is an ordinary merging.
  static Standard_Boolean isOrdinaryMerged(const TReplacementKind theKind)
  {
    return (theKind == TReplacementKind_Merge_Ordinary);
  }

  //! A replacement of an initial shape.
  struct TReplacement
  {
  public:
    //! The default constructor.
    TReplacement()
        : myKind(TReplacementKind_Remove)
    {
    }

    //! The initializing constructor.
    TReplacement(const TopoShape& theResult, const TReplacementKind theKind)
        : myResult(theResult),
          myKind(theKind)
    {
    }

    //! Returns the result of the replacement.
    TopoShape Result() const
    {
      return (myKind != TReplacementKind_Merge_Ordinary) ? myResult : TopoShape();
    }

    //! Returns the result of the relation.
    const TopoShape& RelationResult() const { return myResult; }

    //! Returns the kind of the relation
    //! between an initial shape and the result of the replacement.
    ShapeHistory::TRelationType RelationKind() const
    {
      return (myKind == TReplacementKind_Remove) ? ShapeHistory::TRelationType_Removed
                                                 : ShapeHistory::TRelationType_Modified;
    }

  private:
    TopoShape     myResult; //!< The result of the replacement.
    TReplacementKind myKind;   //!< The kind of the replacement.
  };

  typedef NCollection_DataMap<TopoShape, TReplacement, ShapeHasher>
    TShapeToReplacement;

private:
  //! Maps each shape to its replacement.
  //! If a shape is not bound to the map then the shape is replaced by itself.
  TShapeToReplacement myShapeToReplacement;

protected:
  TopTools_MapOfShape myNewShapes;
  Standard_Integer    myStatus;

private:
  Standard_Boolean myConsiderLocation;
};

#endif // _BRepTools_ReShape_HeaderFile
