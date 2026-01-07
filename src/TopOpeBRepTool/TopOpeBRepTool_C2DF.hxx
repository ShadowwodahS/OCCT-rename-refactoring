// Created on: 1998-03-23
// Created by: Jean Yves LEBEY
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

#ifndef _TopOpeBRepTool_C2DF_HeaderFile
#define _TopOpeBRepTool_C2DF_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Face.hxx>
class GeomCurve2d;

class TopOpeBRepTool_C2DF
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepTool_C2DF();

  Standard_EXPORT TopOpeBRepTool_C2DF(const Handle(GeomCurve2d)& PC,
                                      const Standard_Real         f2d,
                                      const Standard_Real         l2d,
                                      const Standard_Real         tol,
                                      const TopoFace&          F);

  Standard_EXPORT void SetPC(const Handle(GeomCurve2d)& PC,
                             const Standard_Real         f2d,
                             const Standard_Real         l2d,
                             const Standard_Real         tol);

  Standard_EXPORT void SetFace(const TopoFace& F);

  Standard_EXPORT const Handle(GeomCurve2d)& PC(Standard_Real& f2d,
                                                 Standard_Real& l2d,
                                                 Standard_Real& tol) const;

  Standard_EXPORT const TopoFace& Face() const;

  Standard_EXPORT Standard_Boolean IsPC(const Handle(GeomCurve2d)& PC) const;

  Standard_EXPORT Standard_Boolean IsFace(const TopoFace& F) const;

protected:
private:
  Handle(GeomCurve2d) myPC;
  Standard_Real        myf2d;
  Standard_Real        myl2d;
  Standard_Real        mytol;
  TopoFace          myFace;
};

#endif // _TopOpeBRepTool_C2DF_HeaderFile
