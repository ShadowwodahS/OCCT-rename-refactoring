// Created on: 1997-02-06
// Created by: Kernel
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

#ifndef _Storage_HeaderData_HeaderFile
#define _Storage_HeaderData_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Standard_Integer.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TColStd_SequenceOfExtendedString.hxx>
#include <Storage_Error.hxx>
#include <Standard_Transient.hxx>
class Storage_BaseDriver;

class Storage_HeaderData;
DEFINE_STANDARD_HANDLE(Storage_HeaderData, RefObject)

class Storage_HeaderData : public RefObject
{

public:
  Standard_EXPORT Storage_HeaderData();

  Standard_EXPORT Standard_Boolean Read(const Handle(Storage_BaseDriver)& theDriver);

  //! return the creation date
  Standard_EXPORT AsciiString1 CreationDate() const;

  //! return the Storage1 package version
  Standard_EXPORT AsciiString1 StorageVersion() const;

  //! get the version of the schema
  Standard_EXPORT AsciiString1 SchemaVersion() const;

  //! get the schema's name
  Standard_EXPORT AsciiString1 SchemaName() const;

  //! set the version of the application
  Standard_EXPORT void SetApplicationVersion(const AsciiString1& aVersion);

  //! get the version of the application
  Standard_EXPORT AsciiString1 ApplicationVersion() const;

  //! set the name of the application
  Standard_EXPORT void SetApplicationName(const UtfString& aName);

  //! get the name of the application
  Standard_EXPORT UtfString ApplicationName() const;

  //! set the data type
  Standard_EXPORT void SetDataType(const UtfString& aType);

  //! returns data type
  Standard_EXPORT UtfString DataType() const;

  //! add <theUserInfo> to the user information
  Standard_EXPORT void AddToUserInfo(const AsciiString1& theUserInfo);

  //! return the user information
  Standard_EXPORT const TColStd_SequenceOfAsciiString& UserInfo() const;

  //! add <theUserInfo> to the user information
  Standard_EXPORT void AddToComments(const UtfString& aComment);

  //! return the user information
  Standard_EXPORT const TColStd_SequenceOfExtendedString& Comments() const;

  //! the number of persistent objects
  //! Return:
  //! the number of persistent objects readed
  Standard_EXPORT Standard_Integer NumberOfObjects() const;

  Standard_EXPORT Storage_Error ErrorStatus() const;

  Standard_EXPORT AsciiString1 ErrorStatusExtension() const;

  Standard_EXPORT void ClearErrorStatus();

  friend class Storage_Schema;

  DEFINE_STANDARD_RTTIEXT(Storage_HeaderData, RefObject)

public:
  Standard_EXPORT void SetNumberOfObjects(const Standard_Integer anObjectNumber);

  Standard_EXPORT void SetStorageVersion(const AsciiString1& aVersion);

  void SetStorageVersion(const Standard_Integer theVersion)
  {
    SetStorageVersion(AsciiString1(theVersion));
  }

  Standard_EXPORT void SetCreationDate(const AsciiString1& aDate);

  Standard_EXPORT void SetSchemaVersion(const AsciiString1& aVersion);

  Standard_EXPORT void SetSchemaName(const AsciiString1& aName);

private:
  Standard_EXPORT void SetErrorStatus(const Storage_Error anError);

  Standard_EXPORT void SetErrorStatusExtension(const AsciiString1& anErrorExt);

  Standard_Integer                 myNBObj;
  AsciiString1          myStorageVersion;
  AsciiString1          mySchemaVersion;
  AsciiString1          mySchemaName;
  AsciiString1          myApplicationVersion;
  UtfString       myApplicationName;
  UtfString       myDataType;
  AsciiString1          myDate;
  TColStd_SequenceOfAsciiString    myUserInfo;
  TColStd_SequenceOfExtendedString myComments;
  Storage_Error                    myErrorStatus;
  AsciiString1          myErrorStatusExt;
};

#endif // _Storage_HeaderData_HeaderFile
