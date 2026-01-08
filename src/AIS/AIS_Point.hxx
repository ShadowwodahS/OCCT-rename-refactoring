// Created on: 1995-08-09
// Created by: Arnaud BOUZY
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

#ifndef _AIS_Point_HeaderFile
#define _AIS_Point_HeaderFile

#include <AIS_InteractiveObject.hxx>

class Geom_Point;
class TopoVertex;

//! Constructs point datums to be used in construction of
//! composite shapes. The datum is displayed as the plus marker +.
class VisualPoint : public VisualEntity
{
  DEFINE_STANDARD_RTTIEXT(VisualPoint, VisualEntity)
public:
  //! Initializes the point aComponent from which the point
  //! datum will be built.
  Standard_EXPORT VisualPoint(const Handle(Geom_Point)& aComponent);

  //! Returns index 1, the default index for a point.
  virtual Standard_Integer Signature() const Standard_OVERRIDE { return 1; }

  //! Indicates that a point is a datum.
  virtual AIS_KindOfInteractive Type() const Standard_OVERRIDE
  {
    return AIS_KindOfInteractive_Datum;
  }

  //! Returns the component specified in SetComponent.
  Standard_EXPORT Handle(Geom_Point) Component();

  //! Constructs an instance of the point aComponent.
  Standard_EXPORT void SetComponent(const Handle(Geom_Point)& aComponent);

  //! Returns true if the display mode selected is valid for point datums.
  Standard_EXPORT Standard_Boolean
    AcceptDisplayMode(const Standard_Integer aMode) const Standard_OVERRIDE;

  //! Allows you to provide settings for the Color.
  Standard_EXPORT virtual void SetColor(const Color1& theColor) Standard_OVERRIDE;

  //! Allows you to remove color settings.
  Standard_EXPORT virtual void UnsetColor() Standard_OVERRIDE;

  //! Allows you to provide settings for a marker. These include
  //! -   type of marker,
  //! -   marker color,
  //! -   scale factor.
  Standard_EXPORT void SetMarker(const Aspect_TypeOfMarker aType);

  //! Removes the marker settings.
  Standard_EXPORT void UnsetMarker();

  //! Returns true if the point datum has a marker.
  Standard_Boolean HasMarker() const { return myHasTOM; }

  //! Converts a point into a vertex.
  Standard_EXPORT TopoVertex Vertex() const;

protected:
  Standard_EXPORT virtual void Compute(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                       const Handle(Prs3d_Presentation)&         thePrs,
                                       const Standard_Integer theMode) Standard_OVERRIDE;

private:
  Standard_EXPORT void ComputeSelection(const Handle(SelectionContainer)& aSelection,
                                        const Standard_Integer             aMode) Standard_OVERRIDE;

  Standard_EXPORT void UpdatePointValues();

  //! Replace aspects of already computed groups with the new value.
  void replaceWithNewPointAspect(const Handle(Prs3d_PointAspect)& theAspect);

private:
  Handle(Geom_Point)  myComponent;
  Standard_Boolean    myHasTOM;
  Aspect_TypeOfMarker myTOM;
};

DEFINE_STANDARD_HANDLE(VisualPoint, VisualEntity)

#endif // _AIS_Point_HeaderFile
