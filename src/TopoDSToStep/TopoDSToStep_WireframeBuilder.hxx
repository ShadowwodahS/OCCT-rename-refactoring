// Created on: 1995-03-17
// Created by: Dieter THIEMANN
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

#ifndef _TopoDSToStep_WireframeBuilder_HeaderFile
#define _TopoDSToStep_WireframeBuilder_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TColStd_HSequenceOfTransient.hxx>
#include <TopoDSToStep_BuilderError.hxx>
#include <TopoDSToStep_Root.hxx>
#include <MoniTool_DataMapOfShapeTransient.hxx>
class TopoShape;
class TopoDSToStep_Tool;
class Transfer_FinderProcess;
class TopoEdge;
class TopoFace;

//! This builder Class provides services to build
//! a ProSTEP Wireframemodel from a Cas.Cad BRep.
class TopoDSToStep_WireframeBuilder : public Root3
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopoDSToStep_WireframeBuilder();

  Standard_EXPORT TopoDSToStep_WireframeBuilder(
    const TopoShape&     S,
    TopoDSToStep_Tool&      T,
    const ConversionFactors& theLocalFactors = ConversionFactors());

  Standard_EXPORT void Init(const TopoShape&     S,
                            TopoDSToStep_Tool&      T,
                            const ConversionFactors& theLocalFactors = ConversionFactors());

  Standard_EXPORT TopoDSToStep_BuilderError Error() const;

  Standard_EXPORT const Handle(TColStd_HSequenceOfTransient)& Value() const;

  //! Extraction of Trimmed Curves from TopoEdge for the
  //! Creation of a GeometricallyBoundedWireframeRepresentation
  Standard_EXPORT Standard_Boolean
    GetTrimmedCurveFromEdge(const TopoEdge&                    E,
                            const TopoFace&                    F,
                            MoniTool_DataMapOfShapeTransient&     M,
                            Handle(TColStd_HSequenceOfTransient)& L,
                            const ConversionFactors& theLocalFactors = ConversionFactors()) const;

  //! Extraction of Trimmed Curves from TopoFace for the
  //! Creation of a GeometricallyBoundedWireframeRepresentation
  Standard_EXPORT Standard_Boolean
    GetTrimmedCurveFromFace(const TopoFace&                    F,
                            MoniTool_DataMapOfShapeTransient&     M,
                            Handle(TColStd_HSequenceOfTransient)& L,
                            const ConversionFactors& theLocalFactors = ConversionFactors()) const;

  //! Extraction of Trimmed Curves from any TopoShape for the
  //! Creation of a GeometricallyBoundedWireframeRepresentation
  Standard_EXPORT Standard_Boolean
    GetTrimmedCurveFromShape(const TopoShape&                   S,
                             MoniTool_DataMapOfShapeTransient&     M,
                             Handle(TColStd_HSequenceOfTransient)& L,
                             const ConversionFactors& theLocalFactors = ConversionFactors()) const;

protected:
private:
  Handle(TColStd_HSequenceOfTransient) myResult;
  TopoDSToStep_BuilderError            myError;
};

#endif // _TopoDSToStep_WireframeBuilder_HeaderFile
