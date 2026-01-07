// Created on: 1994-08-31
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

#ifndef _Draft_FaceInfo_HeaderFile
#define _Draft_FaceInfo_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Boolean.hxx>
#include <TopoDS_Face.hxx>
class GeomSurface;
class GeomCurve3d;

class Draft_FaceInfo
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT Draft_FaceInfo();

  Standard_EXPORT Draft_FaceInfo(const Handle(GeomSurface)& S,
                                 const Standard_Boolean      HasNewGeometry);

  Standard_EXPORT void RootFace(const TopoFace& F);

  Standard_EXPORT Standard_Boolean NewGeometry() const;

  Standard_EXPORT void Add(const TopoFace& F);

  Standard_EXPORT const TopoFace& FirstFace() const;

  Standard_EXPORT const TopoFace& SecondFace() const;

  Standard_EXPORT const Handle(GeomSurface)& Geometry() const;

  Standard_EXPORT Handle(GeomSurface)& ChangeGeometry();

  Standard_EXPORT const TopoFace& RootFace() const;

  Standard_EXPORT Handle(GeomCurve3d)& ChangeCurve();

  Standard_EXPORT const Handle(GeomCurve3d)& Curve() const;

protected:
private:
  Standard_Boolean     myNewGeom;
  Handle(GeomSurface) myGeom;
  TopoFace          myRootFace;
  TopoFace          myF1;
  TopoFace          myF2;
  Handle(GeomCurve3d)   myCurv;
};

#endif // _Draft_FaceInfo_HeaderFile
