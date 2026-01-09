// Created on: 2008-01-20
// Created by: Alexander A. BORODIN
// Copyright (c) 2008-2014 OPEN CASCADE SAS
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

#include <Font_SystemFont.hxx>

#include <Font_FontMgr.hxx>
#include <OSD_Path.hxx>

IMPLEMENT_STANDARD_RTTIEXT(SystemFont, RefObject)

//=================================================================================================

SystemFont::SystemFont(const AsciiString1& theFontName)
    : myFontKey(theFontName),
      myFontName(theFontName),
      myIsSingleLine(Standard_False)
{
  memset(myFaceIds, 0, sizeof(myFaceIds));
  if (theFontName.IsEmpty())
  {
    throw Standard_ProgramError("SystemFont constructor called with empty font name");
  }
  myFontKey.LowerCase();
}

//=================================================================================================

void SystemFont::SetFontPath(Font_FontAspect                theAspect,
                                  const AsciiString1& thePath,
                                  const Standard_Integer         theFaceId)
{
  if (theAspect == Font_FontAspect_UNDEFINED)
  {
    throw Standard_ProgramError("SystemFont::SetFontPath() called with UNDEFINED aspect");
  }
  myFilePaths[theAspect] = thePath;
  myFaceIds[theAspect]   = theFaceId;
}

//=================================================================================================

Standard_Boolean SystemFont::IsEqual(const Handle(SystemFont)& theOtherFont) const
{
  return theOtherFont.get() == this || myFontKey.IsEqual(theOtherFont->myFontKey);
}

//=================================================================================================

AsciiString1 SystemFont::ToString() const
{
  AsciiString1 aDesc;
  aDesc += AsciiString1() + "'" + myFontName + "'";

  bool isFirstAspect = true;
  aDesc += " [aspects: ";
  for (int anAspectIter = 0; anAspectIter < Font_FontAspect_NB; ++anAspectIter)
  {
    if (!HasFontAspect((Font_FontAspect)anAspectIter))
    {
      continue;
    }

    if (!isFirstAspect)
    {
      aDesc += ",";
    }
    else
    {
      isFirstAspect = false;
    }
    aDesc += FontMgr::FontAspectToString((Font_FontAspect)anAspectIter);
  }
  aDesc += "]";

  isFirstAspect = true;
  aDesc += " [paths: ";
  for (int anAspectIter = 0; anAspectIter < Font_FontAspect_NB; ++anAspectIter)
  {
    if (!HasFontAspect((Font_FontAspect)anAspectIter))
    {
      continue;
    }

    if (!isFirstAspect)
    {
      aDesc += ";";
    }
    else
    {
      isFirstAspect = false;
    }
    aDesc += FontPath((Font_FontAspect)anAspectIter);
    if (FontFaceId((Font_FontAspect)anAspectIter) != 0)
    {
      aDesc = aDesc + "," + FontFaceId((Font_FontAspect)anAspectIter);
    }
  }
  aDesc += "]";
  return aDesc;
}
