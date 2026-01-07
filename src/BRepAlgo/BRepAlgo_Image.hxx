// Created on: 1995-10-26
// Created by: Yves FRICAUD
// Copyright (c) 1995-1999 Matra Datavision
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

#ifndef _BRepAlgo_Image_HeaderFile
#define _BRepAlgo_Image_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopTools_ListOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <Standard_Boolean.hxx>
#include <TopAbs_ShapeEnum.hxx>
class TopoShape;

//! Stores link between a shape <S> and a shape <NewS>
//! obtained from <S>. <NewS> is an image of <S>.
class ShapeImage
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT ShapeImage();

  Standard_EXPORT void SetRoot(const TopoShape& S);

  //! Links <NewS> as image of <OldS>.
  Standard_EXPORT void Bind(const TopoShape& OldS, const TopoShape& NewS);

  //! Links <NewS> as image of <OldS>.
  Standard_EXPORT void Bind(const TopoShape& OldS, const ShapeList& NewS);

  //! Add <NewS> to the image of <OldS>.
  Standard_EXPORT void Add(const TopoShape& OldS, const TopoShape& NewS);

  //! Add <NewS> to the image of <OldS>.
  Standard_EXPORT void Add(const TopoShape& OldS, const ShapeList& NewS);

  Standard_EXPORT void Clear();

  //! Remove <S> to set of images.
  Standard_EXPORT void Remove(const TopoShape& S);

  //! Removes the root <theRoot> from the list of roots and up and down maps.
  Standard_EXPORT void RemoveRoot(const TopoShape& Root);

  //! Replaces the <OldRoot> with the <NewRoot>, so all images
  //! of the <OldRoot> become the images of the <NewRoot>.
  //! The <OldRoot> is removed.
  Standard_EXPORT void ReplaceRoot(const TopoShape& OldRoot, const TopoShape& NewRoot);

  Standard_EXPORT const ShapeList& Roots() const;

  Standard_EXPORT Standard_Boolean IsImage(const TopoShape& S) const;

  //! Returns the generator of <S>
  Standard_EXPORT const TopoShape& ImageFrom(const TopoShape& S) const;

  //! Returns the upper generator of <S>
  Standard_EXPORT const TopoShape& Root(const TopoShape& S) const;

  Standard_EXPORT Standard_Boolean HasImage(const TopoShape& S) const;

  //! Returns the Image of <S>.
  //! Returns <S> in the list if HasImage(S) is false.
  Standard_EXPORT const ShapeList& Image(const TopoShape& S) const;

  //! Stores in <L> the images of images of...images of <S>.
  //! <L> contains only <S> if  HasImage(S) is false.
  Standard_EXPORT void LastImage(const TopoShape& S, ShapeList& L) const;

  //! Keeps only the link between roots and lastimage.
  Standard_EXPORT void Compact();

  //! Deletes in the images the shape of type <ShapeType>
  //! which are not in <S>.
  //! Warning:  Compact() must be call before.
  Standard_EXPORT void Filter(const TopoShape& S, const TopAbs_ShapeEnum ShapeType);

protected:
private:
  ShapeList               roots;
  TopTools_DataMapOfShapeShape       up;
  TopTools_DataMapOfShapeListOfShape down;
};

#endif // _BRepAlgo_Image_HeaderFile
