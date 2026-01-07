// Created on: 2016-10-19
// Created by: Irina KRYLOVA
// Copyright (c) 2016 OPEN CASCADE SAS
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

#ifndef _XCAFDoc_ViewTool_HeaderFile
#define _XCAFDoc_ViewTool_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TDataStd_GenericEmpty.hxx>
#include <Standard_Boolean.hxx>
#include <TDF_LabelSequence.hxx>
#include <Standard_Integer.hxx>

class DataLabel;
class Standard_GUID;

class XCAFDoc_ViewTool;
DEFINE_STANDARD_HANDLE(XCAFDoc_ViewTool, TDataStd_GenericEmpty)

//! Provides tools to store and retrieve Views
//! in and from AppDocument
//! Each View contains parts XCAFDoc_View attribute
//! with all information about camera and view window.
//! Also each view contain information of displayed shapes and GDTs
//! as sets of shape and GDT labels.
class XCAFDoc_ViewTool : public TDataStd_GenericEmpty
{

public:
  Standard_EXPORT XCAFDoc_ViewTool();

  //! Creates (if not exist) ViewTool.
  Standard_EXPORT static Handle(XCAFDoc_ViewTool) Set(const DataLabel& L);

  Standard_EXPORT static const Standard_GUID& GetID();

  //! Returns the label under which Views are stored
  Standard_EXPORT DataLabel BaseLabel() const;

  //! Returns True if label belongs to a View table and
  //! is a View definition
  Standard_EXPORT Standard_Boolean IsView(const DataLabel& theLabel) const;

  //! Returns a sequence of View labels currently stored
  //! in the View table
  Standard_EXPORT void GetViewLabels(TDF_LabelSequence& theLabels) const;

  //! Sets a link with GUID
  Standard_EXPORT void SetView(const TDF_LabelSequence& theShapes,
                               const TDF_LabelSequence& theGDTs,
                               const TDF_LabelSequence& theClippingPlanes,
                               const TDF_LabelSequence& theNotes,
                               const TDF_LabelSequence& theAnnotations,
                               const DataLabel&         theViewL) const;

  //! Sets a link with GUID
  Standard_EXPORT void SetView(const TDF_LabelSequence& theShapes,
                               const TDF_LabelSequence& theGDTs,
                               const TDF_LabelSequence& theClippingPlanes,
                               const DataLabel&         theViewL) const;

  //! Sets a link with GUID
  Standard_EXPORT void SetView(const TDF_LabelSequence& theShapes,
                               const TDF_LabelSequence& theGDTs,
                               const DataLabel&         theViewL) const;

  //! Set Clipping planes to  given View
  Standard_EXPORT void SetClippingPlanes(const TDF_LabelSequence& theClippingPlaneLabels,
                                         const DataLabel&         theViewL) const;

  //! Remove View
  Standard_EXPORT void RemoveView(const DataLabel& theViewL);

  //! Returns all View labels defined for label ShapeL
  Standard_EXPORT Standard_Boolean GetViewLabelsForShape(const DataLabel&   theShapeL,
                                                         TDF_LabelSequence& theViews) const;

  //! Returns all View labels defined for label GDTL
  Standard_EXPORT Standard_Boolean GetViewLabelsForGDT(const DataLabel&   theGDTL,
                                                       TDF_LabelSequence& theViews) const;

  //! Returns all View labels defined for label ClippingPlaneL
  Standard_EXPORT Standard_Boolean GetViewLabelsForClippingPlane(const DataLabel& theClippingPlaneL,
                                                                 TDF_LabelSequence& theViews) const;

  //! Returns all View labels defined for label NoteL
  Standard_EXPORT Standard_Boolean GetViewLabelsForNote(const DataLabel&   theNoteL,
                                                        TDF_LabelSequence& theViews) const;

  //! Returns all View labels defined for label AnnotationL
  Standard_EXPORT Standard_Boolean GetViewLabelsForAnnotation(const DataLabel&   theAnnotationL,
                                                              TDF_LabelSequence& theViews) const;

  //! Adds a view definition to a View table and returns its label
  Standard_EXPORT DataLabel AddView();

  //! Returns shape labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefShapeLabel(const DataLabel&   theViewL,
                                                    TDF_LabelSequence& theShapeLabels) const;

  //! Returns GDT labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefGDTLabel(const DataLabel&   theViewL,
                                                  TDF_LabelSequence& theGDTLabels) const;

  //! Returns ClippingPlane labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean
    GetRefClippingPlaneLabel(const DataLabel&   theViewL,
                             TDF_LabelSequence& theClippingPlaneLabels) const;

  //! Returns Notes labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean GetRefNoteLabel(const DataLabel&   theViewL,
                                                   TDF_LabelSequence& theNoteLabels) const;

  //! Returns Annotation labels defined for label theViewL
  //! Returns False if the theViewL is not in View table
  Standard_EXPORT Standard_Boolean
    GetRefAnnotationLabel(const DataLabel& theViewL, TDF_LabelSequence& theAnnotationLabels) const;

  //! Returns true if the given View is marked as locked
  Standard_EXPORT Standard_Boolean IsLocked(const DataLabel& theViewL) const;

  //! Mark the given View as locked
  Standard_EXPORT void Lock(const DataLabel& theViewL) const;

  //! Unlock the given View
  Standard_EXPORT void Unlock(const DataLabel& theViewL) const;

  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;

  DEFINE_DERIVED_ATTRIBUTE(XCAFDoc_ViewTool, TDataStd_GenericEmpty)
};
#endif // _XCAFDoc_ViewTool_HeaderFile
