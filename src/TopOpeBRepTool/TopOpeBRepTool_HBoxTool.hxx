// Created on: 1993-07-08
// Created by: Jean Yves LEBEY
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

#ifndef _TopOpeBRepTool_HBoxTool_HeaderFile
#define _TopOpeBRepTool_HBoxTool_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TopOpeBRepTool_IndexedDataMapOfShapeBox.hxx>
#include <Standard_Transient.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <Standard_Integer.hxx>
class TopoShape;
class Box2;

class HBoxTool1;
DEFINE_STANDARD_HANDLE(HBoxTool1, RefObject)

class HBoxTool1 : public RefObject
{

public:
  Standard_EXPORT HBoxTool1();

  Standard_EXPORT void Clear();

  Standard_EXPORT void AddBoxes(const TopoShape&    S,
                                const TopAbs_ShapeEnum TS,
                                const TopAbs_ShapeEnum TA = TopAbs_SHAPE);

  Standard_EXPORT void AddBox(const TopoShape& S);

  Standard_EXPORT static void ComputeBox(const TopoShape& S, Box2& B);

  Standard_EXPORT static void ComputeBoxOnVertices(const TopoShape& S, Box2& B);

  Standard_EXPORT static void DumpB(const Box2& B);

  Standard_EXPORT const Box2& Box1(const TopoShape& S);

  Standard_EXPORT const Box2& Box1(const Standard_Integer I) const;

  Standard_EXPORT Standard_Boolean HasBox(const TopoShape& S) const;

  Standard_EXPORT const TopoShape& Shape(const Standard_Integer I) const;

  Standard_EXPORT Standard_Integer Index(const TopoShape& S) const;

  Standard_EXPORT Standard_Integer Extent() const;

  Standard_EXPORT TopOpeBRepTool_IndexedDataMapOfShapeBox& ChangeIMS();

  Standard_EXPORT const TopOpeBRepTool_IndexedDataMapOfShapeBox& IMS() const;

  DEFINE_STANDARD_RTTIEXT(HBoxTool1, RefObject)

protected:
private:
  TopOpeBRepTool_IndexedDataMapOfShapeBox myIMS;
};

#endif // _TopOpeBRepTool_HBoxTool_HeaderFile
