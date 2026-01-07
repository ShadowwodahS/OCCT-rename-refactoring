// Created on: 1997-11-21
// Created by: Mister rmi
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

#ifndef _UTL_HeaderFile
#define _UTL_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_CString.hxx>
#include <Storage_Error.hxx>
#include <Storage_OpenMode.hxx>
#include <Standard_Integer.hxx>
class UtfString;
class Storage_BaseDriver;
class Storage_Data;
class SystemPath;
class OSD_FileIterator;
class AsciiString1;
class Standard_GUID;
class Resource_Manager;

class UTL1
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT static UtfString xgetenv(const Standard_CString aCString);

  Standard_EXPORT static Storage_Error OpenFile(const Handle(Storage_BaseDriver)& aFile,
                                                const UtfString& aName,
                                                const Storage_OpenMode            aMode);

  Standard_EXPORT static void AddToUserInfo(const Handle(Storage_Data)&       aData,
                                            const UtfString& anInfo);

  Standard_EXPORT static SystemPath Path(const UtfString& aFileName);

  Standard_EXPORT static UtfString Disk(const SystemPath& aPath);

  Standard_EXPORT static UtfString Trek(const SystemPath& aPath);

  Standard_EXPORT static UtfString Name(const SystemPath& aPath);

  Standard_EXPORT static UtfString Extension(const SystemPath& aPath);

  Standard_EXPORT static OSD_FileIterator FileIterator(const SystemPath&                   aPath,
                                                       const UtfString& aMask);

  Standard_EXPORT static UtfString Extension(
    const UtfString& aFileName);

  Standard_EXPORT static UtfString LocalHost();

  Standard_EXPORT static UtfString ExtendedString(
    const AsciiString1& anAsciiString);

  Standard_EXPORT static Standard_GUID GUID(const UtfString& anXString);

  Standard_EXPORT static Standard_Boolean Find(const Handle(Resource_Manager)&   aResourceManager,
                                               const UtfString& aResourceName);

  Standard_EXPORT static UtfString Value(
    const Handle(Resource_Manager)&   aResourceManager,
    const UtfString& aResourceName);

  Standard_EXPORT static Standard_Integer IntegerValue(
    const UtfString& anExtendedString);

  Standard_EXPORT static Standard_CString CString(
    const UtfString& anExtendedString);

  Standard_EXPORT static Standard_Boolean IsReadOnly(const UtfString& aFileName);

protected:
private:
};

#endif // _UTL_HeaderFile
