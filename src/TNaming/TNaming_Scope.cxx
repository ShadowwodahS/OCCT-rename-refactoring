// Created on: 1999-11-03
// Created by: Denis PASCAL
// Copyright (c) 1999-1999 Matra Datavision
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

#include <TDF_ChildIterator.hxx>
#include <TDF_Label.hxx>
#include <TNaming_Scope.hxx>
#include <TNaming_Tool.hxx>
#include <TopoDS_Shape.hxx>

//=================================================================================================

NamingScope::NamingScope()
    : myWithValid(Standard_False)
{
}

//=================================================================================================

NamingScope::NamingScope(TDF_LabelMap& map)
{
  myWithValid = Standard_True;
  myValid     = map;
}

//=================================================================================================

NamingScope::NamingScope(const Standard_Boolean with)
    : myWithValid(with)
{
}

//=================================================================================================

Standard_Boolean NamingScope::WithValid() const
{
  return myWithValid;
}

//=================================================================================================

void NamingScope::WithValid(const Standard_Boolean mode)
{
  myWithValid = mode;
}

//=================================================================================================

void NamingScope::ClearValid()
{
  myValid.Clear();
}

//=================================================================================================

void NamingScope::Valid(const DataLabel& L)
{
  myValid.Add(L);
}

//=================================================================================================

void NamingScope::ValidChildren(const DataLabel& L, const Standard_Boolean withroot)
{
  if (L.HasChild())
  {
    ChildIterator itc(L, Standard_True);
    for (; itc.More(); itc.Next())
      myValid.Add(itc.Value());
  }
  if (withroot)
    myValid.Add(L);
}

//=================================================================================================

void NamingScope::Unvalid(const DataLabel& L)
{
  myValid.Remove(L);
}

//=================================================================================================

void NamingScope::UnvalidChildren(const DataLabel& L, const Standard_Boolean withroot)
{
  if (L.HasChild())
  {
    ChildIterator itc(L, Standard_True);
    for (; itc.More(); itc.Next())
      myValid.Remove(itc.Value());
  }
  if (withroot)
    myValid.Remove(L);
}

//=================================================================================================

Standard_Boolean NamingScope::IsValid(const DataLabel& L) const
{
  if (myWithValid)
    return myValid.Contains(L);
  return Standard_True;
}

//=================================================================================================

const TDF_LabelMap& NamingScope::GetValid() const
{
  return myValid;
}

//=================================================================================================

TDF_LabelMap& NamingScope::ChangeValid()
{
  return myValid;
}

//=================================================================================================

TopoShape NamingScope::CurrentShape(const Handle(ShapeAttribute)& NS) const
{
  if (myWithValid)
    return Tool11::CurrentShape(NS, myValid);
  return Tool11::CurrentShape(NS);
}
