// Created on: 1998-12-09
// Created by: Xuan PHAM PHU
// Copyright (c) 1998-1999 Matra Datavision
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

#include <TopOpeBRepTool_connexity.hxx>
#include <TopOpeBRepTool_define.hxx>
#include <TopOpeBRepTool_TOOL.hxx>

#define M_FORWARD(sta) (sta == TopAbs_FORWARD)
#define M_REVERSED(sta) (sta == TopAbs_REVERSED)
#define M_INTERNAL(sta) (sta == TopAbs_INTERNAL)
#define M_EXTERNAL(sta) (sta == TopAbs_EXTERNAL)

#define FORWARD (1)
#define REVERSED (2)
#define INTERNAL (3)
#define EXTERNAL (4)
#define CLOSING (5)

//=================================================================================================

TopOpeBRepTool_connexity::TopOpeBRepTool_connexity()
    : theItems(1, 5)
{
}

//=================================================================================================

TopOpeBRepTool_connexity::TopOpeBRepTool_connexity(const TopoShape& Key1)
    : theKey(Key1),
      theItems(1, 5)
{
}

//=================================================================================================

void TopOpeBRepTool_connexity::SetKey(const TopoShape& Key1)
{
  theKey = Key1;
}

//=================================================================================================

const TopoShape& TopOpeBRepTool_connexity::Key1() const
{
  return theKey;
}

/*static Standard_Integer FUN_toI(const TopAbs_Orientation& O)
{
  Standard_Integer Index = 0;
  if      (O == TopAbs_FORWARD)  Index = 1;
  else if (O == TopAbs_REVERSED) Index = 2;
  else if (O == TopAbs_INTERNAL) Index = 3;
  else if (O == TopAbs_EXTERNAL) Index = 0;
  return Index;
}*/

//=================================================================================================

Standard_Integer TopOpeBRepTool_connexity::Item(const Standard_Integer OriKey,
                                                ShapeList&  Item) const
{
  Item.Clear();
  Item = theItems(OriKey);
  return (Item.Extent());
}

//=================================================================================================

Standard_Integer TopOpeBRepTool_connexity::AllItems(ShapeList& Item) const
{
  Item.Clear();
  for (Standard_Integer i = 1; i <= 4; i++)
  {
    ShapeList copy;
    copy.Assign(theItems.Value(i));
    Item.Append(copy);
  }
  return Item.Extent();
}

//=================================================================================================

void TopOpeBRepTool_connexity::AddItem(const Standard_Integer      OriKey,
                                       const ShapeList& Item)
{
  ShapeList copy;
  copy.Assign(Item);
  theItems(OriKey).Append(copy);
}

void TopOpeBRepTool_connexity::AddItem(const Standard_Integer OriKey, const TopoShape& Item)
{
  ShapeList copy;
  copy.Append(Item);
  theItems(OriKey).Append(copy);
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_connexity::RemoveItem(const Standard_Integer OriKey,
                                                      const TopoShape&    Item)
{
  ShapeList&              item = theItems.ChangeValue(OriKey);
  TopTools_ListIteratorOfListOfShape it(item);
  while (it.More())
  {
    if (it.Value().IsEqual(Item))
    {
      item.Remove(it);
      return Standard_True;
    }
    else
      it.Next();
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_connexity::RemoveItem(const TopoShape& Item)
{
  Standard_Boolean removed = Standard_False;
  for (Standard_Integer i = 1; i <= 5; i++)
  {
    Standard_Boolean found = RemoveItem(i, Item);
    if (found)
      removed = Standard_True;
  }
  return removed;
}

//=================================================================================================

ShapeList& TopOpeBRepTool_connexity::ChangeItem(const Standard_Integer OriKey)
{
  return theItems.ChangeValue(OriKey);
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_connexity::IsMultiple() const
{
  ShapeList lfound;
  Standard_Integer     nkeyitem = Item(FORWARD, lfound);
  //  nkeyRitem += Item(INTERNAL,lfound); NOT VALID
  // if key is vertex : key appears F in closing E, only one time
  nkeyitem += Item(CLOSING, lfound);
  Standard_Boolean multiple = (nkeyitem > 1);
  return multiple;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_connexity::IsFaulty() const
{
  ShapeList lfound;
  Standard_Integer     nkeyRintem = Item(FORWARD, lfound);
  Standard_Integer     nkeyFitem  = Item(REVERSED, lfound);
  Standard_Boolean     faulty     = (nkeyRintem != nkeyFitem);
  return faulty;
}

//=================================================================================================

Standard_Integer TopOpeBRepTool_connexity::IsInternal(ShapeList& Item) const
{
  Item.Clear();

  // all subshapes of INTERNAL(EXTERNAL) are oriented INTERNAL(EXTERNAL)
  ShapeList lINT;
  lINT.Assign(theItems.Value(INTERNAL));
  TopTools_ListIteratorOfListOfShape it1(lINT);
  while (it1.More())
  {
    const TopoShape& item1 = it1.Value();
    TopAbs_Orientation  o1    = item1.Orientation();
    if (!M_INTERNAL(o1))
    {
      it1.Next();
      continue;
    }
    Standard_Integer oKey1 = TOOL1::OriinSor(theKey, item1.Oriented(TopAbs_FORWARD));
    if (oKey1 != INTERNAL)
      lINT.Remove(it1);
    else
      it1.Next();
  }

  ShapeList lEXT;
  lEXT.Assign(theItems.Value(EXTERNAL));
  TopTools_ListIteratorOfListOfShape it2(lEXT);
  while (it2.More())
  {
    const TopoShape& item2 = it2.Value();
    TopAbs_Orientation  o2    = item2.Orientation();
    if (!M_EXTERNAL(o2))
    {
      it2.Next();
      continue;
    }
    Standard_Integer oKey2 = TOOL1::OriinSor(theKey, item2.Oriented(TopAbs_FORWARD));
    if (oKey2 == INTERNAL)
      lINT.Append(item2);
    it2.Next();
  }

  Item.Append(lINT);
  return Item.Extent();
}
