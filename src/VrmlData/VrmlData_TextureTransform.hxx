// Created on: 2006-05-25
// Created by: Alexander GRIGORIEV
// Copyright (c) 2006-2014 OPEN CASCADE SAS
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

#ifndef VrmlData_TextureTransform_HeaderFile
#define VrmlData_TextureTransform_HeaderFile

#include <VrmlData_Node.hxx>
#include <gp_XY.hxx>

///  Implementation of the TextureTransform node

class VrmlData_TextureTransform : public VrmlData_Node
{
public:
  // ---------- PUBLIC METHODS ----------

  /**
   * Empty constructor
   */
  inline VrmlData_TextureTransform()
      : myRotation(0.0)
  {
  }

  /**
   * Constructor
   */
  inline VrmlData_TextureTransform(const VrmlData_Scene& theScene, const char* theName)
      : VrmlData_Node(theScene, theName),
        myRotation(0.)
  {
  }

  /**
   * Query the Center
   */
  inline const Coords2d& Center() const { return myCenter; }

  /**
   * Query the Rotation
   */
  inline Standard_Real Rotation() const { return myRotation; }

  /**
   * Query the Scale
   */
  inline const Coords2d& Scale() const { return myScale; }

  /**
   * Query the Translation
   */
  inline const Coords2d& Translation() const { return myTranslation; }

  /**
   * Set the Center
   */
  inline void SetCenter(const Coords2d& V) { myCenter = V; }

  /**
   * Set the Rotation
   */
  inline void SetRotation(const Standard_Real V) { myRotation = V; }

  /**
   * Set the Scale
   */
  inline void SetScale(const Coords2d& V) { myScale = V; }

  /**
   * Set the Translation
   */
  inline void SetTranslation(const Coords2d& V) { myTranslation = V; }

protected:
  // ---------- PROTECTED METHODS ----------

private:
  // ---------- PRIVATE FIELDS ----------

  Coords2d         myCenter;
  Standard_Real myRotation;
  Coords2d         myScale;
  Coords2d         myTranslation;

public:
  // Declaration of CASCADE RTTI
  DEFINE_STANDARD_RTTI_INLINE(VrmlData_TextureTransform, VrmlData_Node)
};

// Definition of HANDLE object using Standard_DefineHandle.hxx
DEFINE_STANDARD_HANDLE(VrmlData_TextureTransform, VrmlData_Node)

#endif
