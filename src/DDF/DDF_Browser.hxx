// Created by: DAUTRY Philippe
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _DDF_Browser_HeaderFile
#define _DDF_Browser_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TDF_AttributeIndexedMap.hxx>
#include <Draw_Drawable3D.hxx>
#include <Standard_OStream.hxx>
#include <Draw_Interpretor.hxx>
#include <Standard_Integer.hxx>
class Data2;
class DrawDisplay;
class AsciiString1;
class DataLabel;

class DDF_Browser;
DEFINE_STANDARD_HANDLE(DDF_Browser, Drawable3D)

//! Browses a data framework from TDF1.
class DDF_Browser : public Drawable3D
{

public:
  Standard_EXPORT DDF_Browser(const Handle(Data2)& aDF);

  Standard_EXPORT void DrawOn(DrawDisplay& dis) const Standard_OVERRIDE;

  Standard_EXPORT virtual Handle(Drawable3D) Copy() const Standard_OVERRIDE;

  Standard_EXPORT virtual void Dump(Standard_OStream& S) const Standard_OVERRIDE;

  Standard_EXPORT virtual void Whatis(DrawInterpreter& I) const Standard_OVERRIDE;

  Standard_EXPORT void Data(const Handle(Data2)& aDF);

  Standard_EXPORT Handle(Data2) Data() const;

  //! Returns a string composed with the sub-label
  //! entries of <myDF>.
  Standard_EXPORT AsciiString1 OpenRoot() const;

  //! Returns a string composed with the sub-label
  //! entries of <aLab>.
  Standard_EXPORT AsciiString1 OpenLabel(const DataLabel& aLab) const;

  //! Returns a string composed with the attribute index
  //! (found in <myAttMap>) of <aLab>.
  Standard_EXPORT AsciiString1 OpenAttributeList(const DataLabel& aLab);

  //! Returns a string composed with the list of
  //! referenced attribute index of the attribute
  //! <anIndex>. For example, it is useful for
  //! TDataStd_Group. It uses a mechanism based on a
  //! DDF_AttributeBrowser.
  Standard_EXPORT AsciiString1 OpenAttribute(const Standard_Integer anIndex = 0);

  //! Returns information about <me> to be displayed in
  //! information window.
  Standard_EXPORT AsciiString1 Information() const;

  //! Returns information about <aLab> to be displayed
  //! in information window.
  Standard_EXPORT AsciiString1 Information(const DataLabel& aLab) const;

  //! Returns information about attribute <anIndex> to
  //! be displayed in information window.
  Standard_EXPORT AsciiString1 Information(const Standard_Integer anIndex = 0) const;

  DEFINE_STANDARD_RTTIEXT(DDF_Browser, Drawable3D)

protected:
private:
  Handle(Data2)        myDF;
  TDF_AttributeIndexedMap myAttMap;
};

#endif // _DDF_Browser_HeaderFile
