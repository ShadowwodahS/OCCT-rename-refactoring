// Created by: DAUTRY Philippe & VAUTHIER Jean-Claude
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

// ---------------------

// Version: 0.0
// Version   Date            Purpose
//          0.0 Feb 10 1997  Creation

#include <DDF.hxx>

#include <TDF_ComparisonTool.hxx>

#include <DDF_Data.hxx>

#include <Draw_Appli.hxx>
#include <Draw_Interpretor.hxx>
#include <Standard_NotImplemented.hxx>

#include <TCollection_AsciiString.hxx>

#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Data.hxx>
#include <TDF_IDFilter.hxx>
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TDF_DerivedAttribute.hxx>

//=======================================================================
// function : Children
// purpose  : Returns a list of sub-label entries.
//=======================================================================

static Standard_Integer DDF_Children(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;

  Handle(Data2)        DF;
  AsciiString1 entry;

  if (!DDF1::GetDF(a[1], DF))
    return 1;

  DataLabel lab;
  if (n == 3)
    Tool3::Label(DF, a[2], lab);

  if (lab.IsNull())
  {
    di << "0";
  }
  else
  {
    for (ChildIterator itr(lab); itr.More(); itr.Next())
    {
      Tool3::Entry(itr.Value(), entry);
      // AsciiString1 entry(itr.Value().Tag());
      di << entry.ToCString() << " ";
    }
  }
  return 0;
}

//=======================================================================
// function : Attributes
// purpose  : Returns a list of label attributes.
//=======================================================================

static Standard_Integer DDF_Attributes(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 3)
    return 1;

  Handle(Data2) DF;

  if (!DDF1::GetDF(a[1], DF))
    return 1;

  DataLabel lab;
  Tool3::Label(DF, a[2], lab);

  if (lab.IsNull())
    return 1;

  for (TDF_AttributeIterator itr(lab); itr.More(); itr.Next())
  {
    di << itr.Value()->DynamicType()->Name() << " ";
  }
  return 0;
}

//=======================================================================
// function : SetEmptyAttribute
// purpose  : Adds an empty attribute to the label by its dynamic type.
//=======================================================================

static Standard_Integer DDF_SetEmptyAttribute(DrawInterpreter& di,
                                              Standard_Integer  n,
                                              const char**      a)
{
  if (n != 4)
    return 1;

  Handle(Data2) DF;

  if (!DDF1::GetDF(a[1], DF))
    return 1;

  DataLabel lab;
  Tool3::Label(DF, a[2], lab);

  if (lab.IsNull())
    return 1;

  Handle(TDF_Attribute) anAttrByType = DerivedAttribute::Attribute(a[3]);
  if (anAttrByType.IsNull())
  {
    di << "DDF1: Not registered attribute type '" << a[3] << "'\n";
    return 1;
  }

  lab.AddAttribute(anAttrByType);

  return 0;
}

//=======================================================================
// function : ForgetAll
// purpose  : "ForgetAll dfname Label"
//=======================================================================

static Standard_Integer DDF_ForgetAll(DrawInterpreter& /*di*/, Standard_Integer n, const char** a)
{
  if (n != 3)
    return 1;

  Handle(Data2) DF;

  if (!DDF1::GetDF(a[1], DF))
    return 1;

  DataLabel label;
  Tool3::Label(DF, a[2], label);
  if (label.IsNull())
    return 1;
  label.ForgetAllAttributes();
  // POP pour NT
  return 0;
}

//=======================================================================
// function : ForgetAttribute
// purpose  : "ForgetAtt dfname Label guid_or_type"
//=======================================================================

static Standard_Integer DDF_ForgetAttribute(DrawInterpreter& di,
                                            Standard_Integer  n,
                                            const char**      a)
{
  if (n != 4)
    return 1;
  Handle(Data2) DF;
  if (!DDF1::GetDF(a[1], DF))
    return 1;

  DataLabel aLabel;
  Tool3::Label(DF, a[2], aLabel);
  if (aLabel.IsNull())
    return 1;
  if (!Standard_GUID::CheckGUIDFormat(a[3]))
  {
    // check this may be derived attribute by its type
    Handle(TDF_Attribute) anAttrByType = DerivedAttribute::Attribute(a[3]);
    if (!anAttrByType.IsNull())
    {
      aLabel.ForgetAttribute(anAttrByType->ID());
      return 0;
    }
    di << "DDF1: The format of GUID is invalid\n";
    return 1;
  }
  Standard_GUID guid(a[3]);
  aLabel.ForgetAttribute(guid);
  return 0;
}

//=======================================================================
// function : DDF_SetTagger
// purpose  : SetTagger (DF, entry)
//=======================================================================

static Standard_Integer DDF_SetTagger(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  if (nb == 3)
  {
    Handle(Data2) DF;
    if (!DDF1::GetDF(arg[1], DF))
      return 1;
    DataLabel L;
    DDF1::AddLabel(DF, arg[2], L);
    TDF_TagSource::Set(L);
    return 0;
  }
  di << "DDF_SetTagger : Error\n";
  return 1;
}

//=======================================================================
// function : DDF_NewTag
// purpose  : NewTag (DF,[father]
//=======================================================================

static Standard_Integer DDF_NewTag(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  if (nb == 3)
  {
    Handle(Data2) DF;
    if (!DDF1::GetDF(arg[1], DF))
      return 1;
    Handle(TDF_TagSource) A;
    if (!DDF1::Find(DF, arg[2], TDF_TagSource::GetID(), A))
      return 1;
    di << A->NewTag();
    return 0;
  }
  di << "DDF_NewTag : Error\n";
  return 1;
}

//=======================================================================
// function : DDF_NewChild
// purpose  : NewChild(DF,[father])
//=======================================================================

static Standard_Integer DDF_NewChild(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  Handle(Data2) DF;
  if (nb >= 2)
  {
    if (!DDF1::GetDF(arg[1], DF))
      return 1;
    if (nb == 2)
    {
      DataLabel free = TDF_TagSource::NewChild(DF->Root());
      di << free.Tag();
      return 0;
    }
    else if (nb == 3)
    {
      DataLabel fatherlabel;
      if (!DDF1::FindLabel(DF, arg[2], fatherlabel))
        return 1;
      DataLabel free = TDF_TagSource::NewChild(fatherlabel);
      di << arg[2] << ":" << free.Tag();
      return 0;
    }
  }
  di << "DDF_NewChild : Error\n";
  return 1;
}

//=======================================================================
// function : Label (DF,freeentry)
//=======================================================================

static Standard_Integer DDF_Label(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n == 3)
  {
    Handle(Data2) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    DataLabel L;
    if (!DDF1::FindLabel(DF, a[2], L, Standard_False))
    {
      DDF1::AddLabel(DF, a[2], L);
      // di << "Label : " << a[2] << " created\n";
    }
    // else di << "Label : " << a[2] << " retrieved\n";
    DDF1::ReturnLabel(di, L);
    return 0;
  }
  di << "DDF_Label : Error\n";
  return 1;
}

//=================================================================================================

void DDF1::BasicCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  const char* g = "DF basic commands";

  // Label :

  theCommands.Add("SetTagger", "SetTagger (DF, entry)", __FILE__, DDF_SetTagger, g);

  theCommands.Add("NewTag", "NewTag (DF, tagger)", __FILE__, DDF_NewTag, g);

  theCommands.Add("NewChild", "NewChild (DF, [tagger])", __FILE__, DDF_NewChild, g);

  theCommands.Add("Children",
                  " Returns the list of label children: Children DF label",
                  __FILE__,
                  DDF_Children,
                  g);

  theCommands.Add("Attributes",
                  " Returns the list of label attributes: Attributes DF label",
                  __FILE__,
                  DDF_Attributes,
                  g);

  theCommands.Add(
    "SetEmptyAttribute",
    "Sets an empty attribute by its type (like TDataStd_Tick): SetEmptyAttribute DF label type",
    __FILE__,
    DDF_SetEmptyAttribute,
    g);

  theCommands.Add("ForgetAll",
                  "Forgets all attributes from the label: ForgetAll DF Label",
                  __FILE__,
                  DDF_ForgetAll,
                  g);

  theCommands.Add("ForgetAtt",
                  "Forgets the specified by guid attribute or type from the label: ForgetAtt DF "
                  "Label guid_or_type",
                  __FILE__,
                  DDF_ForgetAttribute,
                  g);

  theCommands.Add("Label", "Label DF entry", __FILE__, DDF_Label, g);
}
