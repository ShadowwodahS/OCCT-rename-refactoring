// Author: Kirill Gavrilov
// Copyright (c) 2015-2019 OPEN CASCADE SAS
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

#ifndef _RWObj_MtlReader_HeaderFile
#define _RWObj_MtlReader_HeaderFile

#include <Graphic3d_Vec3.hxx>
#include <RWObj_Material.hxx>
#include <NCollection_DataMap.hxx>

//! Reader of mtl files.
class MTLReader
{
public:
  //! Main constructor.
  MTLReader(NCollection_DataMap<AsciiString1, Material1>& theMaterials);

  //! Destructor.
  ~MTLReader();

  //! Read the file.
  bool Read(const AsciiString1& theFolder, const AsciiString1& theFile);

private:
  //! Validate scalar value
  bool validateScalar(const Standard_Real theValue);

  //! Validate RGB color
  bool validateColor(const Graphic3d_Vec3& theVec);

  //! Process texture path.
  void processTexturePath(AsciiString1&       theTexturePath,
                          const AsciiString1& theFolder);

private:
  FILE*                                                         myFile;
  AsciiString1                                       myPath;
  NCollection_DataMap<AsciiString1, Material1>* myMaterials;
  int                                                           myNbLines;
};

#endif // _RWObj_MtlReader_HeaderFile
