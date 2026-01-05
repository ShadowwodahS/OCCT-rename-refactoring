// Created on: 1994-02-24
// Created by: Laurent BOURESCHE
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

#ifndef _BRepLProp_SurfaceTool_HeaderFile
#define _BRepLProp_SurfaceTool_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <Standard_Integer.hxx>
class BRepAdaptor_Surface;
class Point3d;
class Vector3d;

class BRepLProp_SurfaceTool
{
public:
  DEFINE_STANDARD_ALLOC

  //! Computes the point <P> of parameter <U> and <V> on the
  //! Surface <S>.
  Standard_EXPORT static void Value(const BRepAdaptor_Surface& S,
                                    const Standard_Real        U,
                                    const Standard_Real        V,
                                    Point3d&                    P);

  //! Computes the point <P> and first derivative <D1*> of
  //! parameter <U> and <V> on the Surface <S>.
  Standard_EXPORT static void D1(const BRepAdaptor_Surface& S,
                                 const Standard_Real        U,
                                 const Standard_Real        V,
                                 Point3d&                    P,
                                 Vector3d&                    D1U,
                                 Vector3d&                    D1V);

  //! Computes the point <P>, the first derivative <D1*> and second
  //! derivative <D2*> of parameter <U> and <V> on the Surface <S>.
  Standard_EXPORT static void D2(const BRepAdaptor_Surface& S,
                                 const Standard_Real        U,
                                 const Standard_Real        V,
                                 Point3d&                    P,
                                 Vector3d&                    D1U,
                                 Vector3d&                    D1V,
                                 Vector3d&                    D2U,
                                 Vector3d&                    D2V,
                                 Vector3d&                    DUV);

  Standard_EXPORT static Vector3d DN(const BRepAdaptor_Surface& S,
                                   const Standard_Real        U,
                                   const Standard_Real        V,
                                   const Standard_Integer     IU,
                                   const Standard_Integer     IV);

  //! returns the order of continuity of the Surface <S>.
  //! returns 1 : first derivative only is computable
  //! returns 2 : first and second derivative only are computable.
  Standard_EXPORT static Standard_Integer Continuity(const BRepAdaptor_Surface& S);

  //! returns the bounds of the Surface.
  Standard_EXPORT static void Bounds(const BRepAdaptor_Surface& S,
                                     Standard_Real&             U1,
                                     Standard_Real&             V1,
                                     Standard_Real&             U2,
                                     Standard_Real&             V2);

protected:
private:
};

#endif // _BRepLProp_SurfaceTool_HeaderFile
