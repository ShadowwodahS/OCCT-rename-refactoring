// Created on: 1999-01-04
// Created by: Xuan PHAM PHU
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

#ifndef _TopOpeBRepTool_REGUS_HeaderFile
#define _TopOpeBRepTool_REGUS_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <Standard_Integer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
class TopoFace;
class TopoEdge;

class TopOpeBRepTool_REGUS
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepTool_REGUS();

  Standard_EXPORT void Init(const TopoShape& S);

  Standard_EXPORT const TopoShape& S() const;

  Standard_EXPORT Standard_Boolean MapS();

  Standard_EXPORT static Standard_Boolean WireToFace(const TopoFace&          Fanc,
                                                     const ShapeList& nWs,
                                                     ShapeList&       nFs);

  Standard_EXPORT static Standard_Boolean SplitF(const TopoFace&    Fanc,
                                                 ShapeList& FSplits);

  Standard_EXPORT Standard_Boolean SplitFaces();

  Standard_EXPORT Standard_Boolean REGU();

  Standard_EXPORT void SetFsplits(TopTools_DataMapOfShapeListOfShape& Fsplits);

  Standard_EXPORT void GetFsplits(TopTools_DataMapOfShapeListOfShape& Fsplits) const;

  Standard_EXPORT void SetOshNsh(TopTools_DataMapOfShapeListOfShape& OshNsh);

  Standard_EXPORT void GetOshNsh(TopTools_DataMapOfShapeListOfShape& OshNsh) const;

  Standard_EXPORT Standard_Boolean InitBlock();

  Standard_EXPORT Standard_Boolean NextinBlock();

  Standard_EXPORT Standard_Boolean NearestF(const TopoEdge&          e,
                                            const ShapeList& lof,
                                            TopoFace&                ffound) const;

protected:
private:
  Standard_Boolean                   hasnewsplits;
  TopTools_DataMapOfShapeListOfShape myFsplits;
  TopTools_DataMapOfShapeListOfShape myOshNsh;
  TopoShape                       myS;
  TopTools_DataMapOfShapeListOfShape mymapeFsstatic;
  TopTools_DataMapOfShapeListOfShape mymapeFs;
  TopTools_IndexedMapOfShape         mymapemult;
  Standard_Integer                   mynF;
  Standard_Integer                   myoldnF;
  TopoShape                       myf;
  TopTools_MapOfShape                myedstoconnect;
  ShapeList               mylFinBlock;
};

#endif // _TopOpeBRepTool_REGUS_HeaderFile
