// Copyright (c) 2018 OPEN CASCADE SAS
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

#ifndef _XCAFNoteObjects_NoteObject_HeaderFile
#define _XCAFNoteObjects_NoteObject_HeaderFile

#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <Standard.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>
#include <TopoDS_Shape.hxx>

//! object to store note auxiliary data
class XCAFNoteObjects_NoteObject : public RefObject
{
  DEFINE_STANDARD_RTTIEXT(XCAFNoteObjects_NoteObject, RefObject)
public:
  //! Empty object
  Standard_EXPORT XCAFNoteObjects_NoteObject();

  //! Copy constructor.
  Standard_EXPORT XCAFNoteObjects_NoteObject(const Handle(XCAFNoteObjects_NoteObject)& theObj);

  //! Returns True if plane is specified
  Standard_Boolean HasPlane() const { return myHasPlane; }

  //! Returns a right-handed coordinate system of the plane
  const Frame3d& GetPlane() const { return myPlane; }

  //! Sets a right-handed coordinate system of the plane
  Standard_EXPORT void SetPlane(const Frame3d& thePlane);

  //! Returns True if the attachment point on the annotated object is specified
  Standard_Boolean HasPoint() const { return myHasPnt; }

  //! Returns the attachment point on the annotated object
  const Point3d& GetPoint() const { return myPnt; }

  //! Sets the anchor point on the annotated object
  Standard_EXPORT void SetPoint(const Point3d& thePnt);

  //! Returns True if the text position is specified
  Standard_Boolean HasPointText() const { return myHasPntTxt; }

  //! Returns the text position
  const Point3d& GetPointText() const { return myPntTxt; }

  //! Sets the text position
  Standard_EXPORT void SetPointText(const Point3d& thePnt);

  //! Returns a tessellated annotation if specified
  const TopoShape& GetPresentation() const { return myPresentation; }

  //! Sets a tessellated annotation
  Standard_EXPORT void SetPresentation(const TopoShape& thePresentation);

  //! Resets data to the state after calling the default constructor
  Standard_EXPORT void Reset();

private:
  Frame3d           myPlane;
  Point3d           myPnt;
  Point3d           myPntTxt;
  TopoShape     myPresentation;
  Standard_Boolean myHasPlane;
  Standard_Boolean myHasPnt;
  Standard_Boolean myHasPntTxt;
};

DEFINE_STANDARD_HANDLE(XCAFNoteObjects_NoteObject, RefObject)

#endif // _XCAFNoteObjects_NoteObject_HeaderFile
