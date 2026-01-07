// Created on: 1997-06-10
// Created by: Yves FRICAUD
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _TNaming_Localizer_HeaderFile
#define _TNaming_Localizer_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Integer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TNaming_ListOfMapOfShape.hxx>
#include <TNaming_ListOfIndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TNaming_Evolution.hxx>
#include <TNaming_ListOfNamedShape.hxx>
#include <TNaming_MapOfNamedShape.hxx>
class TNaming_UsedShapes;
class TopoShape;
class DataLabel;
class ShapeAttribute;

class TNaming_Localizer
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TNaming_Localizer();

  Standard_EXPORT void Init(const Handle(TNaming_UsedShapes)& US, const Standard_Integer CurTrans);

  Standard_EXPORT const TopTools_MapOfShape& SubShapes(const TopoShape&    S,
                                                       const TopAbs_ShapeEnum Type);

  Standard_EXPORT const TopTools_IndexedDataMapOfShapeListOfShape& Ancestors(
    const TopoShape&    S,
    const TopAbs_ShapeEnum Type);

  Standard_EXPORT void FindFeaturesInAncestors(const TopoShape&  S,
                                               const TopoShape&  In,
                                               TopTools_MapOfShape& AncInFeatures);

  Standard_EXPORT void GoBack(const TopoShape&       S,
                              const DataLabel&          Lab,
                              const TNaming_Evolution   Evol,
                              ShapeList&     OldS,
                              TNaming_ListOfNamedShape& OldLab);

  Standard_EXPORT void Backward(const Handle(ShapeAttribute)& NS,
                                const TopoShape&               S,
                                TNaming_MapOfNamedShape&          Primitives,
                                TopTools_MapOfShape&              ValidShapes);

  Standard_EXPORT void FindNeighbourg(const TopoShape&  Cont,
                                      const TopoShape&  S,
                                      TopTools_MapOfShape& Neighbourg);

  Standard_EXPORT static Standard_Boolean IsNew(const TopoShape&               S,
                                                const Handle(ShapeAttribute)& NS);

  Standard_EXPORT static void FindGenerator(const Handle(ShapeAttribute)& NS,
                                            const TopoShape&               S,
                                            ShapeList&             theListOfGenerators);

  //! Finds context of the shape <S>.
  Standard_EXPORT static void FindShapeContext(const Handle(ShapeAttribute)& NS,
                                               const TopoShape&               theS,
                                               TopoShape&                     theSC);

protected:
private:
  Standard_Integer                               myCurTrans;
  Handle(TNaming_UsedShapes)                     myUS;
  ShapeList                           myShapeWithSubShapes;
  TNaming_ListOfMapOfShape                       mySubShapes;
  ShapeList                           myShapeWithAncestors;
  TNaming_ListOfIndexedDataMapOfShapeListOfShape myAncestors;
};

#endif // _TNaming_Localizer_HeaderFile
