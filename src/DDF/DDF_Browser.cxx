// Created by: DAUTRY Philippe
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

//      	---------------
// Version:	0.0
// Version	Date		Purpose
//		0.0	Oct  3 1997	Creation

#include <DDF_AttributeBrowser.hxx>
#include <DDF_Browser.hxx>
#include <Draw_Display.hxx>
#include <Draw_Drawable3D.hxx>
#include <Standard_Type.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Data.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DDF_Browser, Drawable3D)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Communication convention with tcl:
// tcl waits for a string of characters, being an information list.
// In this list, each item is separated from another by a separator: '\'.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define TDF_BrowserSeparator1 '\\'
#define TDF_BrowserSeparator2 ' '
#define TDF_BrowserSeparator3 '#'
#define TDF_BrowserSeparator4 ','

//=================================================================================================

DDF_Browser::DDF_Browser(const Handle(Data2)& aDF)
    : myDF(aDF)
{
}

//=================================================================================================

void DDF_Browser::DrawOn(DrawDisplay& /*dis*/) const
{
  // std::cout<<"DDF_Browser"<<std::endl;
}

//=================================================================================================

Handle(Drawable3D) DDF_Browser::Copy() const
{
  return new DDF_Browser(myDF);
}

//=================================================================================================

void DDF_Browser::Dump(Standard_OStream& S) const
{
  S << "DDF_Browser on a DF:" << std::endl;
  S << myDF;
}

//=================================================================================================

void DDF_Browser::Whatis(DrawInterpreter& I) const
{
  I << "Data Framework Browser";
}

//=================================================================================================

void DDF_Browser::Data(const Handle(Data2)& aDF)
{
  myDF = aDF;
}

//=================================================================================================

Handle(Data2) DDF_Browser::Data() const
{
  return myDF;
}

//=================================================================================================

AsciiString1 DDF_Browser::OpenRoot() const
{
  AsciiString1 list;
  const DataLabel&        root = myDF->Root();
  Tool3::Entry(root, list);
  Handle(NameAttribute) name;
  list.AssignCat(TDF_BrowserSeparator2);
  list.AssignCat("\"");
  if (root.FindAttribute(NameAttribute::GetID(), name))
  {
    AsciiString1 tmpStr(name->Get());
    tmpStr.ChangeAll(' ', '_');
    list.AssignCat(tmpStr);
  }
  list.AssignCat("\"");
  list.AssignCat(TDF_BrowserSeparator2);
  if (!root.MayBeModified())
    list.AssignCat("Not");
  list.AssignCat("Modified");
  list.AssignCat(TDF_BrowserSeparator2);
  list.AssignCat((root.HasAttribute() || root.HasChild()) ? "1" : "0");
  return list;
}

//=======================================================================
// function : OpenLabel
// purpose  :
// an item is composed as follows:
// "Entry "Name" Modified|NotModified 0|1"
// the end bit shows if the label has attributes or children.
// The 1st can be
// "AttributeList Modified|NotModified"
// The items are separated by "\\".
//=======================================================================

AsciiString1 DDF_Browser::OpenLabel(const DataLabel& aLab) const
{
  Standard_Boolean        split = Standard_False;
  AsciiString1 entry, list;
  if (aLab.HasAttribute() || aLab.AttributesModified())
  {
    list.AssignCat("AttributeList");
    list.AssignCat(TDF_BrowserSeparator2);
    if (!aLab.AttributesModified())
      list.AssignCat("Not");
    list.AssignCat("Modified");
    split = Standard_True;
  }
  Handle(NameAttribute) name;
  for (ChildIterator itr(aLab); itr.More(); itr.Next())
  {
    if (split)
      list.AssignCat(TDF_BrowserSeparator1);
    Tool3::Entry(itr.Value(), entry);
    list.AssignCat(entry);
    list.AssignCat(TDF_BrowserSeparator2);
    list.AssignCat("\"");
    if (itr.Value().FindAttribute(NameAttribute::GetID(), name))
    {
      AsciiString1 tmpStr(name->Get());
      tmpStr.ChangeAll(' ', '_');
      list.AssignCat(tmpStr);
    }
    list.AssignCat("\"");
    list.AssignCat(TDF_BrowserSeparator2);
    if (!itr.Value().MayBeModified())
      list.AssignCat("Not");
    list.AssignCat("Modified");
    list.AssignCat(TDF_BrowserSeparator2);
    // May be open.
    list.AssignCat((itr.Value().HasAttribute() || itr.Value().HasChild()) ? "1" : "0");
    split = Standard_True;
  }
  return list;
}

//=======================================================================
// function : OpenAttributeList
// purpose  :
// an item is composed as follows:
// "DynamicType#MapIndex TransactionIndex Valid|Notvalid Forgotten|NotForgotten
// Backuped|NotBackuped" The items are separated by "\\".
//=======================================================================

AsciiString1 DDF_Browser::OpenAttributeList(const DataLabel& aLab)
{
  AsciiString1 list;
  Standard_Boolean        split1 = Standard_False;
  for (TDF_AttributeIterator itr(aLab, Standard_False); itr.More(); itr.Next())
  {
    if (split1)
      list.AssignCat(TDF_BrowserSeparator1);
    const Handle(TDF_Attribute)& att   = itr.Value();
    const Standard_Integer       index = myAttMap.Add(att);
    AsciiString1      indexStr(index);
    list.AssignCat(att->DynamicType()->Name());
    list.AssignCat(TDF_BrowserSeparator3);
    list.AssignCat(indexStr);
    list.AssignCat(TDF_BrowserSeparator2);
    list.AssignCat(att->Transaction());
    // Valid.
    list.AssignCat(TDF_BrowserSeparator2);
    if (!att->IsValid())
      list.AssignCat("Not");
    list.AssignCat("Valid");
    // Forgotten.
    list.AssignCat(TDF_BrowserSeparator2);
    if (!att->IsForgotten())
      list.AssignCat("Not");
    list.AssignCat("Forgotten");
    // Backuped.
    list.AssignCat(TDF_BrowserSeparator2);
    if (!att->IsBackuped())
      list.AssignCat("Not");
    list.AssignCat("Backuped");
    // May be open.
    list.AssignCat(TDF_BrowserSeparator2);
    DDF_AttributeBrowser* br = DDF_AttributeBrowser::FindBrowser(att);
    list.AssignCat(br ? "1" : "0");
    split1 = Standard_True;
  }
  return list;
}

//=======================================================================
// function : OpenAttribute
// purpose  : Attribute's intrinsic information given by an attribute browser.
//=======================================================================

AsciiString1 DDF_Browser::OpenAttribute(const Standard_Integer anIndex)
{
  AsciiString1 list;
  Handle(TDF_Attribute)   att = myAttMap.FindKey(anIndex);
  DDF_AttributeBrowser*   br  = DDF_AttributeBrowser::FindBrowser(att);
  if (br)
    list = br->Open(att);
  return list;
}

//=======================================================================
// function : Information
// purpose  : Information about <myDF>.
//=======================================================================

AsciiString1 DDF_Browser::Information() const
{
  AsciiString1 list;
  return list;
}

//=======================================================================
// function : Information
// purpose  : Information about a label.
//=======================================================================

AsciiString1 DDF_Browser::Information(const DataLabel& /*aLab*/) const
{
  AsciiString1 list;
  return list;
}

//=======================================================================
// function : Information
// purpose  : Information about an attribute.
//=======================================================================

AsciiString1 DDF_Browser::Information(const Standard_Integer /*anIndex*/) const
{
  AsciiString1 list;
  return list;
}
