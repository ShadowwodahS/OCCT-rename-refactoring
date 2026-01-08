// Created on: 1991-01-23
// Created by: Christophe MARION
// Copyright (c) 1991-1999 Matra Datavision
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

#ifndef _TopLoc_Datum3D_HeaderFile
#define _TopLoc_Datum3D_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <gp_Trsf.hxx>
#include <Standard_Transient.hxx>
#include <Standard_OStream.hxx>

class TopLoc_Datum3D;
DEFINE_STANDARD_HANDLE(TopLoc_Datum3D, RefObject)

//! Describes a coordinate transformation, i.e. a change
//! to an elementary 3D coordinate system, or position in 3D space.
//! A Datum3D1 is always described relative to the default datum.
//! The default datum is described relative to itself: its
//! origin is (0,0,0), and its axes are (1,0,0) (0,1,0) (0,0,1).
class TopLoc_Datum3D : public RefObject
{

public:
  //! Constructs a default Datum3D1.
  Standard_EXPORT TopLoc_Datum3D();

  //! Constructs a Datum3D1 form a Trsf from gp1. An error is
  //! raised if the Trsf is not a rigid transformation.
  Standard_EXPORT TopLoc_Datum3D(const Transform3d& T);

  //! Returns a Transform3d which, when applied to this datum, produces the default datum.
  const Transform3d& Transformation() const { return myTrsf; }

  //! Returns a Transform3d which, when applied to this datum, produces the default datum.
  const Transform3d& Trsf() const { return myTrsf; }

  //! Return transformation form.
  gp_TrsfForm Form() const { return myTrsf.Form(); }

  //! Dumps the content of me into the stream
  Standard_EXPORT void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const;

  //! Writes the contents of this Datum3D1 to the stream S.
  Standard_EXPORT void ShallowDump(Standard_OStream& S) const;

  DEFINE_STANDARD_RTTIEXT(TopLoc_Datum3D, RefObject)

private:
  Transform3d myTrsf;
};

inline void ShallowDump(const Handle(TopLoc_Datum3D)& me, Standard_OStream& S)
{
  me->ShallowDump(S);
}

#endif // _TopLoc_Datum3D_HeaderFile
