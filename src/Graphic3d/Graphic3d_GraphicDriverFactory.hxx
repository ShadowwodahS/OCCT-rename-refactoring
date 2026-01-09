// Copyright (c) 2021 OPEN CASCADE SAS
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

#ifndef _Graphic3d_GraphicDriverFactory_HeaderFile
#define _Graphic3d_GraphicDriverFactory_HeaderFile

#include <NCollection_List.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>

class DisplayConnection1;
class Graphic3d_GraphicDriver;
class GraphicDriverFactory;
typedef NCollection_List<Handle(GraphicDriverFactory)> Graphic3d_GraphicDriverFactoryList;

//! This class for creation of Graphic3d_GraphicDriver.
class GraphicDriverFactory : public RefObject
{
  DEFINE_STANDARD_RTTIEXT(GraphicDriverFactory, RefObject)
public:
  //! Registers factory.
  //! @param[in] theFactory      factory to register
  //! @param[in] theIsPreferred  add to the beginning of the list when TRUE, or add to the end
  //! otherwise
  Standard_EXPORT static void RegisterFactory(
    const Handle(GraphicDriverFactory)& theFactory,
    bool                                          theIsPreferred = false);

  //! Unregisters factory.
  Standard_EXPORT static void UnregisterFactory(const AsciiString1& theName);

  //! Return default driver factory or NULL if no one was registered.
  Standard_EXPORT static Handle(GraphicDriverFactory) DefaultDriverFactory();

  //! Return the global map of registered driver factories.
  Standard_EXPORT static const Graphic3d_GraphicDriverFactoryList& DriverFactories();

public:
  //! Creates new empty graphic driver.
  virtual Handle(Graphic3d_GraphicDriver) CreateDriver(
    const Handle(DisplayConnection1)& theDisp) = 0;

  //! Return driver factory name.
  const AsciiString1& Name() const { return myName; }

protected:
  //! Empty constructor.
  Standard_EXPORT GraphicDriverFactory(const AsciiString1& theName);

protected:
  AsciiString1 myName;
};

#endif // _Graphic3d_GraphicDriverFactory_HeaderFile
