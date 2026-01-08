// Created on: 1993-06-17
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

#ifndef _TopOpeBRepBuild_ShellFaceClassifier_HeaderFile
#define _TopOpeBRepBuild_ShellFaceClassifier_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <gp_Pnt.hxx>
#include <TopOpeBRepTool_SolidClassifier.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepBuild_CompositeClassifier.hxx>
#include <TopAbs_State.hxx>
class BlockBuilder;

//! Classify faces and shells.
//! shapes are Shells, Elements are Faces.
class TopOpeBRepBuild_ShellFaceClassifier : public CompositeClassifier
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates a classifier in 3D space, to compare :
  //! a face with a set of faces
  //! a shell with a set of faces
  //! a shell with a shell
  Standard_EXPORT TopOpeBRepBuild_ShellFaceClassifier(const BlockBuilder& BB);

  Standard_EXPORT void Clear();

  //! classify shell <B1> with shell <B2>
  Standard_EXPORT TopAbs_State CompareShapes(const TopoShape& B1, const TopoShape& B2);

  //! classify face <F> with shell <S>
  Standard_EXPORT TopAbs_State CompareElementToShape(const TopoShape& F, const TopoShape& S);

  //! prepare classification involving shell <S>
  //! calls ResetElement on first face of <S>
  Standard_EXPORT void ResetShape(const TopoShape& S);

  //! prepare classification involving face <F>
  //! define 3D point (later used in Compare()) on first vertex of face <F>.
  Standard_EXPORT void ResetElement(const TopoShape& F);

  //! Add the face <F> in the set of faces used in 3D point
  //! classification. Returns FALSE if the face <F> has been already
  //! added to the set of faces, otherwise returns TRUE.
  Standard_EXPORT Standard_Boolean CompareElement(const TopoShape& F);

  //! Returns state of classification of 3D point, defined by
  //! ResetElement, with the current set of faces, defined by Compare.
  Standard_EXPORT TopAbs_State State();

protected:
private:
  Standard_Boolean               myFirstCompare;
  Point3d                         myPoint3d;
  TopoShell                   myShell;
  ShapeBuilder                   myBuilder;
  TopOpeBRepTool_SolidClassifier mySolidClassifier;
  TopTools_DataMapOfShapeShape   myFaceShellMap;
  TopoShape                   myShape;
};

#endif // _TopOpeBRepBuild_ShellFaceClassifier_HeaderFile
