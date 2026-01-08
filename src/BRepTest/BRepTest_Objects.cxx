// Created on: 2018/03/21
// Created by: Eugeny MALTCHIKOV
// Copyright (c) 2018 OPEN CASCADE SAS
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

#include <BRepTest_Objects.hxx>

//=======================================================================
// function : BRepTest_Session
// purpose  : Class for the objects in the session
//=======================================================================
class BRepTest_Session
{
public:
  //! Empty constructor
  BRepTest_Session() { SetDefaultValues(); }

  //! Sets the default values for the options
  void SetDefaultValues() { myFillHistory = Standard_True; }

  //! Sets the History in the session
  void SetHistory(const Handle(ShapeHistory)& theHistory) { myHistory = theHistory; }

  //! Add the History to the history in the session
  void AddHistory(const Handle(ShapeHistory)& theHistory)
  {
    if (myHistory.IsNull())
      myHistory = new ShapeHistory;
    myHistory->Merge(theHistory);
  }

  //! Returns the history from the session
  const Handle(ShapeHistory)& History() const { return myHistory; }

  //! Enables/Disables the history saving
  void SetToFillHistory(const Standard_Boolean theFillHist) { myFillHistory = theFillHist; }

  //! Returns the flag controlling the history saving
  Standard_Boolean IsHistoryNeeded() const { return myFillHistory; }

private:
  Handle(ShapeHistory) myHistory;
  Standard_Boolean          myFillHistory;
};

//=================================================================================================

static BRepTest_Session& GetSession()
{
  static BRepTest_Session* pSession = new BRepTest_Session();
  return *pSession;
}

//=================================================================================================

void Objects1::SetHistory(const Handle(ShapeHistory)& theHistory)
{
  GetSession().SetHistory(theHistory);
}

//=================================================================================================

void Objects1::AddHistory(const Handle(ShapeHistory)& theHistory)
{
  GetSession().AddHistory(theHistory);
}

//=================================================================================================

Handle(ShapeHistory) Objects1::History()
{
  return GetSession().History();
}

//=================================================================================================

void Objects1::SetToFillHistory(const Standard_Boolean theFillHist)
{
  return GetSession().SetToFillHistory(theFillHist);
}

//=================================================================================================

Standard_Boolean Objects1::IsHistoryNeeded()
{
  return GetSession().IsHistoryNeeded();
}
