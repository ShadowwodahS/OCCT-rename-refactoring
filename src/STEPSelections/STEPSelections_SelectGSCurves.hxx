// Created on: 1999-03-22
// Created by: data exchange team
// Copyright (c) 1999 Matra Datavision
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

#ifndef _STEPSelections_SelectGSCurves_HeaderFile
#define _STEPSelections_SelectGSCurves_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <IFSelect_SelectExplore.hxx>
#include <Standard_Integer.hxx>
class RefObject;
class Interface_Graph;
class Interface_EntityIterator;
class AsciiString1;

class STEPSelections_SelectGSCurves;
DEFINE_STANDARD_HANDLE(STEPSelections_SelectGSCurves, IFSelect_SelectExplore)

//! This selection returns "curves in the geometric_set (except composite curves)"
class STEPSelections_SelectGSCurves : public IFSelect_SelectExplore
{

public:
  Standard_EXPORT STEPSelections_SelectGSCurves();

  Standard_EXPORT Standard_Boolean
    Explore(const Standard_Integer            level,
            const Handle(RefObject)& ent,
            const Interface_Graph&            G,
            Interface_EntityIterator&         explored) const Standard_OVERRIDE;

  //! Returns a text defining the criterium : "Curves"
  Standard_EXPORT AsciiString1 ExploreLabel() const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(STEPSelections_SelectGSCurves, IFSelect_SelectExplore)

protected:
private:
};

#endif // _STEPSelections_SelectGSCurves_HeaderFile
