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

#ifndef _BRepAlgo_AsDes_HeaderFile
#define _BRepAlgo_AsDes_HeaderFile

#include <Standard.hxx>

#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <Standard_Transient.hxx>
#include <TopTools_ListOfShape.hxx>
class TopoShape;

class BRepAlgo_AsDes;
DEFINE_STANDARD_HANDLE(BRepAlgo_AsDes, RefObject)

//! SD to store descendants and ascendants of Shapes.
class BRepAlgo_AsDes : public RefObject
{

public:
  //! Creates an empty AsDes.
  Standard_EXPORT BRepAlgo_AsDes();

  Standard_EXPORT void Clear();

  //! Stores <SS> as a futur subshape of <S>.
  Standard_EXPORT void Add(const TopoShape& S, const TopoShape& SS);

  //! Stores <SS> as futurs SubShapes of <S>.
  Standard_EXPORT void Add(const TopoShape& S, const ShapeList& SS);

  Standard_EXPORT Standard_Boolean HasAscendant(const TopoShape& S) const;

  Standard_EXPORT Standard_Boolean HasDescendant(const TopoShape& S) const;

  //! Returns the Shape containing <S>.
  Standard_EXPORT const ShapeList& Ascendant(const TopoShape& S) const;

  //! Returns futur subhapes of <S>.
  Standard_EXPORT const ShapeList& Descendant(const TopoShape& S) const;

  //! Returns futur subhapes of <S>.
  Standard_EXPORT ShapeList& ChangeDescendant(const TopoShape& S);

  //! Replace theOldS by theNewS.
  //! theOldS disappear from this.
  Standard_EXPORT void Replace(const TopoShape& theOldS, const TopoShape& theNewS);

  //! Remove theS from me.
  Standard_EXPORT void Remove(const TopoShape& theS);

  //! Returns  True if (S1> and <S2>  has  common
  //! Descendants.  Stores in <LC> the Commons Descendants.
  Standard_EXPORT Standard_Boolean HasCommonDescendant(const TopoShape&   S1,
                                                       const TopoShape&   S2,
                                                       ShapeList& LC) const;

  DEFINE_STANDARD_RTTIEXT(BRepAlgo_AsDes, RefObject)

private:
  //! Replace theOldS by theNewS.
  //! theOldS disappear from this.
  Standard_EXPORT void BackReplace(const TopoShape&         theOldS,
                                   const TopoShape&         theNewS,
                                   const ShapeList& theL,
                                   const Standard_Boolean      theInUp);

private:
  TopTools_DataMapOfShapeListOfShape up;
  TopTools_DataMapOfShapeListOfShape down;
};

#endif // _BRepAlgo_AsDes_HeaderFile
