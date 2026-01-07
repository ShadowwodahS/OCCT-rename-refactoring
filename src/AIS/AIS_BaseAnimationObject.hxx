// Copyright (c) 2023 OPEN CASCADE SAS
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

#ifndef _AIS_BaseAnimationObject_HeaderFile
#define _AIS_BaseAnimationObject_HeaderFile

#include <AIS_Animation.hxx>
#include <AIS_InteractiveContext.hxx>

//! Animation defining object transformation.
class AIS_BaseAnimationObject : public AIS_Animation
{
  DEFINE_STANDARD_RTTIEXT(AIS_BaseAnimationObject, AIS_Animation)
protected:
  //! Constructor with initialization.
  //! @param[in] theAnimationName animation identifier
  //! @param[in] theContext       interactive context where object have been displayed
  //! @param[in] theObject        object to apply local transformation
  Standard_EXPORT AIS_BaseAnimationObject(const AsciiString1&        theAnimationName,
                                          const Handle(VisualContext)& theContext,
                                          const Handle(VisualEntity)&  theObject);

  //! Update the transformation.
  Standard_EXPORT void updateTrsf(const Transform3d& theTrsf);

private:
  //! Invalidate the viewer for proper update.
  Standard_EXPORT void invalidateViewer();

protected:
  Handle(VisualContext) myContext; //!< context where object is displayed
  Handle(VisualEntity)  myObject;  //!< presentation object to set location
};

#endif // _AIS_BaseAnimationObject_HeaderFile
