// Created on: 1996-12-13
// Created by: Jean-Pierre COMBE/Odile Olivier
// Copyright (c) 1996-1999 Matra Datavision
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

#ifndef _AIS_PlaneTrihedron_HeaderFile
#define _AIS_PlaneTrihedron_HeaderFile

#include <AIS_InteractiveObject.hxx>
#include <TCollection_AsciiString.hxx>

class AIS_Line;
class VisualPoint;
class GeomPlane;

//! To construct a selectable 2d axis system in a 3d
//! drawing. This can be placed anywhere in the 3d
//! system, and provides a coordinate system for
//! drawing curves and shapes in a plane.
//! There are 3 selection modes:
//! -   mode 0   selection of the whole plane "trihedron"
//! -   mode 1   selection of the origin of the plane "trihedron"
//! -   mode 2   selection of the axes.
//! Warning
//! For the presentation of planes and trihedra, the
//! millimetre is default unit of length, and 100 the default
//! value for the representation of the axes. If you modify
//! these dimensions, you must temporarily recover the
//! Drawer object. From inside it, take the Aspects in
//! which   the values for length are stocked, for example,
//! PlaneAspect for planes and LineAspect for
//! trihedra. Change these values and recalculate the presentation.
class AIS_PlaneTrihedron : public VisualEntity
{
  DEFINE_STANDARD_RTTIEXT(AIS_PlaneTrihedron, VisualEntity)
public:
  //! Initializes the plane aPlane. The plane trihedron is
  //! constructed from this and an axis.
  Standard_EXPORT AIS_PlaneTrihedron(const Handle(GeomPlane)& aPlane);

  //! Returns the component specified in SetComponent.
  Standard_EXPORT Handle(GeomPlane) Component();

  //! Creates an instance of the component object aPlane.
  Standard_EXPORT void SetComponent(const Handle(GeomPlane)& aPlane);

  //! Returns the "XAxis".
  Standard_EXPORT Handle(AIS_Line) XAxis() const;

  //! Returns the "YAxis".
  Standard_EXPORT Handle(AIS_Line) YAxis() const;

  //! Returns the point of origin of the plane trihedron.
  Standard_EXPORT Handle(VisualPoint) Position1() const;

  //! Sets the length of the X and Y axes.
  Standard_EXPORT void SetLength(const Standard_Real theLength);

  //! Returns the length of X and Y axes.
  Standard_EXPORT Standard_Real GetLength() const;

  //! Returns true if the display mode selected, aMode, is valid.
  Standard_EXPORT Standard_Boolean
    AcceptDisplayMode(const Standard_Integer aMode) const Standard_OVERRIDE;

  virtual Standard_Integer Signature() const Standard_OVERRIDE { return 4; }

  //! Returns datum as the type of Interactive Object.
  virtual AIS_KindOfInteractive Type() const Standard_OVERRIDE
  {
    return AIS_KindOfInteractive_Datum;
  }

  //! Allows you to provide settings for the color aColor.
  Standard_EXPORT virtual void SetColor(const Color1& theColor) Standard_OVERRIDE;

  void SetXLabel(const AsciiString1& theLabel) { myXLabel = theLabel; }

  void SetYLabel(const AsciiString1& theLabel) { myYLabel = theLabel; }

protected:
  Standard_EXPORT virtual void Compute(const Handle(PrsMgr_PresentationManager)& theprsMgr,
                                       const Handle(Prs3d_Presentation)&         thePrs,
                                       const Standard_Integer theMode) Standard_OVERRIDE;

private:
  Standard_EXPORT virtual void ComputeSelection(const Handle(SelectionContainer)& theSel,
                                                const Standard_Integer theMode) Standard_OVERRIDE;

private:
  Handle(GeomPlane)            myPlane;
  Handle(VisualEntity) myShapes[3];
  AsciiString1       myXLabel;
  AsciiString1       myYLabel;
};

DEFINE_STANDARD_HANDLE(AIS_PlaneTrihedron, VisualEntity)

#endif // _AIS_PlaneTrihedron_HeaderFile
