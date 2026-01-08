// Created by: Anastasia BORISOVA
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

#ifndef _AIS_AnimationCamera_HeaderFile
#define _AIS_AnimationCamera_HeaderFile

#include <AIS_Animation.hxx>

class CameraOn3d;
class ViewWindow;

//! Camera animation.
class AIS_AnimationCamera : public AIS_Animation
{
  DEFINE_STANDARD_RTTIEXT(AIS_AnimationCamera, AIS_Animation)
public:
  //! Main constructor.
  Standard_EXPORT AIS_AnimationCamera(const AsciiString1& theAnimationName,
                                      const Handle(ViewWindow)&        theView);

  //! Return the target view.
  const Handle(ViewWindow)& View() const { return myView; }

  //! Set target view.
  void SetView(const Handle(ViewWindow)& theView) { myView = theView; }

  //! Return camera start position.
  const Handle(CameraOn3d)& CameraStart() const { return myCamStart; }

  //! Define camera start position.
  void SetCameraStart(const Handle(CameraOn3d)& theCameraStart)
  {
    myCamStart = theCameraStart;
  }

  //! Return camera end position.
  const Handle(CameraOn3d)& CameraEnd() const { return myCamEnd; }

  //! Define camera end position.
  void SetCameraEnd(const Handle(CameraOn3d)& theCameraEnd) { myCamEnd = theCameraEnd; }

protected:
  //! Update the progress.
  Standard_EXPORT virtual void update(const AnimationProgress& theProgress) Standard_OVERRIDE;

protected:
  Handle(ViewWindow)         myView;     //!< view to setup camera
  Handle(CameraOn3d) myCamStart; //!< starting camera position
  Handle(CameraOn3d) myCamEnd;   //!< end camera position
};

DEFINE_STANDARD_HANDLE(AIS_AnimationCamera, AIS_Animation)

#endif // _AIS_AnimationCamera_HeaderFile
