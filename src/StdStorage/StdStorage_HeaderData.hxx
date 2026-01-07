// Copyright (c) 2017 OPEN CASCADE SAS
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

#ifndef _StdStorage_HeaderData_HeaderFile
#define _StdStorage_HeaderData_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Standard_Integer.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TColStd_SequenceOfExtendedString.hxx>
#include <Storage_Error.hxx>
#include <Standard_Transient.hxx>
class Storage_BaseDriver;

class StdStorage_HeaderData;
DEFINE_STANDARD_HANDLE(StdStorage_HeaderData, RefObject)

//! Storage header data section that contains some
//! auxiliary information (application name, schema version,
//! creation date, comments and so on...)
class StdStorage_HeaderData : public RefObject
{
  friend class StdStorage_Data;

public:
  DEFINE_STANDARD_RTTIEXT(StdStorage_HeaderData, RefObject)

  //! Reads the header data section from the container defined by theDriver.
  //! Returns Standard_True in case of success. Otherwise, one need to get
  //! an error code and description using ErrorStatus and ErrorStatusExtension
  //! functions correspondingly.
  Standard_EXPORT Standard_Boolean Read(const Handle(Storage_BaseDriver)& theDriver);

  //! Writes the header data section to the container defined by theDriver.
  //! Returns Standard_True in case of success. Otherwise, one need to get
  //! an error code and description using ErrorStatus and ErrorStatusExtension
  //! functions correspondingly.
  Standard_EXPORT Standard_Boolean Write(const Handle(Storage_BaseDriver)& theDriver);

  //! Return the creation date
  Standard_EXPORT AsciiString1 CreationDate() const;

  //! Return the Storage package version
  Standard_EXPORT AsciiString1 StorageVersion() const;

  //! Get the version of the schema
  Standard_EXPORT AsciiString1 SchemaVersion() const;

  //! Set the version of the application
  Standard_EXPORT void SetApplicationVersion(const AsciiString1& aVersion);

  //! Get the version of the application
  Standard_EXPORT AsciiString1 ApplicationVersion() const;

  //! Set the name of the application
  Standard_EXPORT void SetApplicationName(const UtfString& aName);

  //! Get the name of the application
  Standard_EXPORT UtfString ApplicationName() const;

  //! Set the data type
  Standard_EXPORT void SetDataType(const UtfString& aType);

  //! Returns data type
  Standard_EXPORT UtfString DataType() const;

  //! Add <theUserInfo> to the user information
  Standard_EXPORT void AddToUserInfo(const AsciiString1& theUserInfo);

  //! Return the user information
  Standard_EXPORT const TColStd_SequenceOfAsciiString& UserInfo() const;

  //! Add <theUserInfo> to the user information
  Standard_EXPORT void AddToComments(const UtfString& aComment);

  //! Return the user information
  Standard_EXPORT const TColStd_SequenceOfExtendedString& Comments() const;

  //! Returns the number of persistent objects
  Standard_EXPORT Standard_Integer NumberOfObjects() const;

  //! Returns a status of the latest call to Read / Write functions
  Standard_EXPORT Storage_Error ErrorStatus() const;

  //! Returns an error message if any of the latest call to Read / Write functions
  Standard_EXPORT AsciiString1 ErrorStatusExtension() const;

  //! Clears error status
  Standard_EXPORT void ClearErrorStatus();

  Standard_EXPORT void SetNumberOfObjects(const Standard_Integer anObjectNumber);

  Standard_EXPORT void SetStorageVersion(const AsciiString1& aVersion);

  Standard_EXPORT void SetCreationDate(const AsciiString1& aDate);

  Standard_EXPORT void SetSchemaVersion(const AsciiString1& aVersion);

  Standard_EXPORT void SetSchemaName(const AsciiString1& aName);

private:
  Standard_EXPORT StdStorage_HeaderData();

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

#endif // _StdStorage_HeaderData_HeaderFile
