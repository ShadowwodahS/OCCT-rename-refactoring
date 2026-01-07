// Copyright (c) 2022 OPEN CASCADE SAS
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

#ifndef _DE_ConfigurationContext_HeaderFile
#define _DE_ConfigurationContext_HeaderFile

#include <NCollection_DataMap.hxx>
#include <TColStd_ListOfAsciiString.hxx>

typedef NCollection_DataMap<AsciiString1, AsciiString1> DE_ResourceMap;

//! Provides convenient interface to resource file
//! Allows loading of the resource file and getting attributes'
//! values starting from some scope, for example
//! if scope is defined as "ToV4" and requested parameter
//! is "exec.op", value of "ToV4.exec.op" parameter from
//! the resource file will be returned
class DE_ConfigurationContext : public RefObject
{
public:
  DEFINE_STANDARD_RTTIEXT(DE_ConfigurationContext, RefObject)

  //! Creates an empty tool
  Standard_EXPORT DE_ConfigurationContext();

  //! Import the custom configuration
  //! Save all parameters with their values.
  //! @param[in] theConfiguration path to configuration file or string value
  //! @return true in case of success, false otherwise
  Standard_EXPORT Standard_Boolean Load(const AsciiString1& theConfiguration);

  //! Import the resource file.
  //! Save all parameters with their values.
  //! @param[in] theFile path to the resource file
  //! @return true in case of success, false otherwise
  Standard_EXPORT Standard_Boolean LoadFile(const AsciiString1& theFile);

  //! Import the resource string.
  //! Save all parameters with their values.
  //! @param[in] theResource string with resource content
  //! @return true in case of success, false otherwise
  Standard_EXPORT Standard_Boolean LoadStr(const AsciiString1& theResource);

  //! Checks for existing the parameter name
  //! @param[in] theParam complex parameter name
  //! @param[in] theScope base parameter name
  //! @return Standard_True if parameter is defined in the resource file
  Standard_EXPORT Standard_Boolean IsParamSet(const AsciiString1& theParam,
                                              const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[out] theValue value to get by parameter
  //! @param[in] theScope base parameter name
  //! @return Standard_False if parameter is not defined or has a wrong type
  Standard_EXPORT Standard_Boolean GetReal(const AsciiString1& theParam,
                                           Standard_Real&                 theValue,
                                           const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[out] theValue value to get by parameter
  //! @param[in] theScope base parameter name
  //! @return Standard_False if parameter is not defined or has a wrong type
  Standard_EXPORT Standard_Boolean GetInteger(const AsciiString1& theParam,
                                              Standard_Integer&              theValue,
                                              const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[out] theValue value to get by parameter
  //! @param[in] theScope base parameter name
  //! @return Standard_False if parameter is not defined or has a wrong type
  Standard_EXPORT Standard_Boolean GetBoolean(const AsciiString1& theParam,
                                              Standard_Boolean&              theValue,
                                              const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[out] theValue value to get by parameter
  //! @param[in] theScope base parameter name
  //! @return Standard_False if parameter is not defined or has a wrong type
  Standard_EXPORT Standard_Boolean GetString(const AsciiString1& theParam,
                                             AsciiString1&       theValue,
                                             const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[out] theValue value to get by parameter
  //! @param[in] theScope base parameter name
  //! @return Standard_False if parameter is not defined or has a wrong type
  Standard_EXPORT Standard_Boolean GetStringSeq(const AsciiString1& theParam,
                                                TColStd_ListOfAsciiString&     theValue,
                                                const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[in] theDefValue value by default if param is not found or has wrong type
  //! @param[in] theScope base parameter name
  //! @return specific type value
  Standard_EXPORT Standard_Real RealVal(const AsciiString1& theParam,
                                        const Standard_Real            theDefValue,
                                        const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[in] theDefValue value by default if param is not found or has wrong type
  //! @param[in] theScope base parameter name
  //! @return specific type value
  Standard_EXPORT Standard_Integer IntegerVal(const AsciiString1& theParam,
                                              const Standard_Integer         theDefValue,
                                              const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[in] theDefValue value by default if param is not found or has wrong type
  //! @param[in] theScope base parameter name
  //! @return specific type value
  Standard_EXPORT Standard_Boolean BooleanVal(const AsciiString1& theParam,
                                              const Standard_Boolean         theDefValue,
                                              const AsciiString1& theScope = "") const;

  //! Gets value of parameter as being of specific type
  //! @param[in] theParam complex parameter name
  //! @param[in] theDefValue value by default if param is not found or has wrong type
  //! @param[in] theScope base parameter name
  //! @return specific type value
  Standard_EXPORT AsciiString1
    StringVal(const AsciiString1& theParam,
              const AsciiString1& theDefValue,
              const AsciiString1& theScope = "") const;

  //! Gets internal resource map
  //! @return map with resource value
  Standard_EXPORT const DE_ResourceMap& GetInternalMap() const { return myResource; }

protected:
  //! Update the resource with param value from the line
  //! @paramp[in] theResourceLine line contains the parameter
  //! @return true if theResourceLine has loaded correctly
  Standard_Boolean load(const AsciiString1& theResourceLine);

private:
  DE_ResourceMap myResource; //!< Internal parameters map
};

#endif // _DE_ConfigurationContext_HeaderFile
