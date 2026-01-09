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

//      	--------------------

// Version:	0.0
// Version	Date		Purpose
//		0.0	Sep 30 1997	Creation

#include <TDF_ClosureMode.hxx>
#include <TDF_ClosureTool.hxx>
#include <TDF_CopyTool.hxx>

#include <DDF.hxx>
#include <DDF_Data.hxx>

#include <Draw_Appli.hxx>
#include <Draw_Drawable3D.hxx>
#include <Draw_Interpretor.hxx>

#include <Standard_NotImplemented.hxx>

#include <TDF_Data.hxx>
#include <TDF_DataSet.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <TDF_CopyLabel.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_AttributeMap.hxx>

//=======================================================================
// function : MakeDF
// purpose  : Creates a new data framework.
//=======================================================================

static Standard_Integer MakeDF(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;

  Handle(Drawable3D) D = Draw1::Get(a[1]);
  Handle(DDF_Data)        NewDDF;

  if (!D.IsNull())
  {
    NewDDF = Handle(DDF_Data)::DownCast(D);
    if (!NewDDF.IsNull())
    {
      di << "DDF_BasicCommands::MakeDF - Sorry, this Data Framework already exists\n";
      return 0;
    }
  }

  Handle(TDF_Data) NewDF = new TDF_Data();
  NewDDF                 = new DDF_Data(NewDF);
  Draw1::Set(a[1], NewDDF);
  // DeltaDS.Nullify();
  return 0;
}

//=======================================================================
// function : ClearDF
// purpose  : Creates a new data framework.
//=======================================================================

static Standard_Integer ClearDF(DrawInterpreter& di, Standard_Integer n, const char** a)

{
  if (n < 2)
    return 1;

  Handle(Drawable3D) D = Draw1::Get(a[1]);
  Handle(DDF_Data)        DDF1;

  if (!D.IsNull())
  {
    DDF1 = Handle(DDF_Data)::DownCast(D);
    if (!DDF1.IsNull())
    {
      Handle(TDF_Data) DF = DDF1->DataFramework();
      if (!DF.IsNull())
      {
        Handle(TDF_Data) NewEmpty = new TDF_Data;
        DDF1->DataFramework(NewEmpty);
        // DeltaDS.Nullify();
      }
      return 0;
    }
  }

  di << "DDF_BasicCommands::MakeDF - Sorry, this Data Framework doesn't exist\n";

  return 0;
}

//=================================================================================================

static Standard_Integer CopyDF(DrawInterpreter& /*di*/, Standard_Integer n, const char** a)
{
  if (n < 4 || n > 5)
    return 1;

  Handle(TDF_Data) DF1;
  Handle(TDF_Data) DF2;
  Standard_CString Entry1;
  Standard_CString Entry2;

  if (!DDF1::GetDF(a[1], DF1))
    return 1;

  Entry1 = a[2];
  Entry2 = a[3];

  if (n == 4)
  {
    DF2    = DF1;
    Entry2 = a[3];
  }
  else if (n == 5)
  {
    if (!DDF1::GetDF(a[3], DF2))
      return 1;
    Entry2 = a[4];
  }

  DataLabel Label1;
  if (!DDF1::FindLabel(DF1, Entry1, Label1))
    return 1;

  DataLabel Label2;
  if (!DDF1::FindLabel(DF2, Entry2, Label2, Standard_False))
  {
    DDF1::AddLabel(DF2, Entry2, Label2);
  }

  Handle(TDF_DataSet) DataSet = new TDF_DataSet;
  DataSet->AddLabel(Label1);
  ClosureTool::Closure(DataSet);
  Handle(TDF_RelocationTable) Reloc = new TDF_RelocationTable();
  Reloc->SetRelocation(Label1, Label2);
  CopyTool::Copy(DataSet, Reloc);

  return 0;
}

//=================================================================================================

static Standard_Integer MiniDumpDF(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;

  Handle(Drawable3D) D;
  Handle(DDF_Data)        DDF1;

  D = Draw1::Get(a[1]);

  if (D.IsNull())
  {
    di << "DDF_BasicCommands : Sorry this Data Framework doesn't exist\n";
    return Standard_False;
  }

  DDF1 = Handle(DDF_Data)::DownCast(D);

  if (DDF1.IsNull())
  {
    di << "DDF_BasicCommands : Sorry this Data Framework doesn't exist\n";
    return Standard_False;
  }

  di << "*********** Dump of " << a[1] << " ***********\n";

  // DDF1->DataFramework()->Dump(std::cout);
  Standard_SStream aSStream;
  DDF1->DataFramework()->Dump(aSStream);
  aSStream << std::ends;
  di << aSStream << "\n";

  return 0;
}

//=======================================================================
// function : XDumpDF
// purpose  : eXtended deep dump of a DataFramework
//=======================================================================

static Standard_Integer XDumpDF(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;

  Handle(Drawable3D) D;
  Handle(DDF_Data)        DDF1;

  D = Draw1::Get(a[1]);

  if (D.IsNull())
  {
    di << "DDF_BasicCommands : Sorry this Data Framework doesn't exist\n";
    return Standard_False;
  }

  DDF1 = Handle(DDF_Data)::DownCast(D);

  if (DDF1.IsNull())
  {
    di << "DDF_BasicCommands : Sorry this Data Framework doesn't exist\n";
    return Standard_False;
  }

  di << "*********** Dump of " << a[1] << " ***********\n";

  IDFilter filter(Standard_False);
  // Tool3::ExtendedDeepDump(cout,DDF1->DataFramework(),filter);
  Standard_SStream aSStream;
  Tool3::ExtendedDeepDump(aSStream, DDF1->DataFramework(), filter);
  aSStream << std::ends;
  di << aSStream << "\n";

  return 0;
}

//=======================================================================
// function : CopyLabel_SCopy
// purpose  : CopyLabel (DF,fromlabel,tolabel)
//=======================================================================

static Standard_Integer CopyLabel_SCopy(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  DataLabel SOURCE, TARGET;
  if (n == 4)
  {
    Handle(TDF_Data) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    if (!DDF1::FindLabel(DF, a[2], SOURCE))
      return 1;
    if (DDF1::FindLabel(DF, a[3], TARGET))
    {
      di << " target label is already set \n";
      return 1;
    }
    DDF1::AddLabel(DF, a[3], TARGET);
    TDF_CopyLabel cop;
    cop.Load(SOURCE, TARGET);
    cop.Perform();
    if (!cop.IsDone())
      di << "copy not done\n";
    return 0;
  }
  di << "DDF_CopyLabel : Error\n";
  return 1;
}

//=======================================================================
// function : DDF_CheckAttr
// purpose  : CheckAttr (DOC,label1,label2)
//         : Checks references of attributes of label1 and label2
//         : in order to find shareable attributes
//=======================================================================

static Standard_Integer DDF_CheckAttrs(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  DataLabel SOURCE, TARGET;
  if (n == 4)
  {
    Handle(TDF_Data) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    if (!DDF1::FindLabel(DF, a[2], SOURCE))
      return 1;
    if (!DDF1::FindLabel(DF, a[3], TARGET))
      return 1;

    Handle(TDF_DataSet) ds1  = new TDF_DataSet();
    Handle(TDF_DataSet) ds2  = new TDF_DataSet();
    Standard_Boolean    Shar = Standard_False;
    for (TDF_AttributeIterator itr(SOURCE); itr.More(); itr.Next())
    {
      itr.Value()->References(ds1);
      //      std::cout<<"\tSource Attribute dynamic type =
      //      "<<itr.Value()->DynamicType()<<std::endl;
      const TDF_AttributeMap& attMap = ds1->Attributes(); // attMap
      for (TDF_MapIteratorOfAttributeMap attMItr(attMap); attMItr.More(); attMItr.Next())
      {
        const Handle(TDF_Attribute)& sAtt = attMItr.Key1();
        //	std::cout<<"\t\tSource references attribute dynamic type =
        //"<<sAtt->DynamicType()<<std::endl;
        for (TDF_AttributeIterator itr2(TARGET); itr2.More(); itr2.Next())
        {
          itr2.Value()->References(ds2);
          //	  std::cout<<"\t\t\tTARGET attribute dynamic type =
          //"<<itr2.Value()->DynamicType()<<std::endl;
          const TDF_AttributeMap& attMap2 = ds2->Attributes(); // attMap
          for (TDF_MapIteratorOfAttributeMap attMItr2(attMap2); attMItr2.More(); attMItr2.Next())
          {
            const Handle(TDF_Attribute)& tAtt = attMItr2.Key1();
            //	    std::cout<<"\t\t\t\tTarget reference attribute dynamic type =
            //"<<tAtt->DynamicType()<<std::endl;
            if (tAtt->IsInstance(sAtt->DynamicType()))
              if (tAtt == sAtt)
              {
                AsciiString1 entr1, entr2;
                if (!Shar)
                {
                  Tool3::Entry(SOURCE, entr1);
                  Tool3::Entry(TARGET, entr2);
                  // std::cout<<"\tSHAREABLE attribute(s) found between Lab1 = "<<entr1<<" and Lab2
                  // = "<<entr2<<std::endl;
                  di << "\tSHAREABLE attribute(s) found between Lab1 = " << entr1.ToCString()
                     << " and Lab2 = " << entr2.ToCString() << "\n";
                  Shar = Standard_True;
                }
                Tool3::Entry(sAtt->Label(), entr1);
                // std::cout<<"\tAttribute dynamic type = "<<sAtt->DynamicType()<<",\tlocated on
                // Label = "<<entr1<<std::endl;
                di << "\tAttribute dynamic type = " << sAtt->DynamicType()->Name();
                di << ",\tlocated on Label = " << entr1.ToCString() << "\n";
              }
          }
          ds2->Clear();
        }
      }
      ds1->Clear();
    }
    if (!Shar)
      di << "Shareable attributes not found\n";
    return 0;
  }
  di << "DDF_CheckAttrs : Error\n";
  return 1;
}

//=======================================================================
// function : DDF_Checklabel
// purpose  : CheckLabel (DOC,label1,label2)
//         : prints all structure of first level attributes with its references
//=======================================================================
static Standard_Integer DDF_CheckLabel(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  //  DataLabel SOURCE,TARGET;
  DataLabel SOURCE;
  if (n == 3)
  {
    Handle(TDF_Data) DF;
    if (!DDF1::GetDF(a[1], DF))
      return 1;
    if (!DDF1::FindLabel(DF, a[2], SOURCE))
      return 1;

    Handle(TDF_DataSet) ds1 = new TDF_DataSet();
    for (TDF_AttributeIterator itr(SOURCE); itr.More(); itr.Next())
    {
      itr.Value()->References(ds1);
      // std::cout<<"\tSource Attribute dynamic type = "<<itr.Value()->DynamicType()<<std::endl;
      di << "\tSource Attribute dynamic type = " << itr.Value()->DynamicType()->Name() << "\n";
      const TDF_AttributeMap& attMap = ds1->Attributes(); // attMap
      for (TDF_MapIteratorOfAttributeMap attMItr(attMap); attMItr.More(); attMItr.Next())
      {
        const Handle(TDF_Attribute)& sAtt = attMItr.Key1();
        AsciiString1      entry;
        Tool3::Entry(sAtt->Label(), entry);
        // std::cout<<"\t\tReferences attribute dynamic type = "<<sAtt->DynamicType()<<",\tLabel =
        // "<<entry<<std::endl;
        di << "\t\tReferences attribute dynamic type = " << sAtt->DynamicType()->Name();
        di << ",\tLabel = " << entry.ToCString() << "\n";
      }
      ds1->Clear();
    }

    return 0;
  }
  di << "DDF_ChecLabel : Error\n";
  return 1;
}

//=======================================================================
// function : DDF_SetAccessByEntry
// purpose  : SetAccessByEntry DOC 1|0
//=======================================================================
static Standard_Integer DDF_SetAccessByEntry(DrawInterpreter& di,
                                             Standard_Integer  nb,
                                             const char**      a)
{
  Standard_Integer aRet = 0;
  if (nb != 3)
  {
    di << "SetAccessByEntry DOC 1|0\n";
    aRet = 1;
  }
  else
  {
    Handle(TDF_Data) aDF;
    if (DDF1::GetDF(a[1], aDF))
    {
      Standard_Boolean aSet = (Draw1::Atoi(a[2]) == 1);
      aDF->SetAccessByEntries(aSet);
    }
    else
    {
      aRet = 1;
    }
  }
  return aRet;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=================================================================================================

void DDF1::DataCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;

  const char* g = "DF Data Framework commands";

  // Data Framework :
  // ++++++++++++++++
  theCommands.Add("MakeDF", "Makes a new DF: MakeDF dfname", __FILE__, MakeDF, g);

  theCommands.Add("ClearDF", "Clears a DF: ClearDF dfname", __FILE__, ClearDF, g);

  theCommands.Add("CopyDF",
                  "Copies a label: CopyDF dfname1 entry1 [dfname2] entry2",
                  __FILE__,
                  CopyDF,
                  g);

  theCommands.Add("XDumpDF",
                  "Exented deep dump of a DF (with attributes content): DumpDF dfname",
                  __FILE__,
                  XDumpDF,
                  g);

  theCommands.Add("MiniDumpDF",
                  "Mini dump of a DF (with attributes content): DumpDF dfname",
                  __FILE__,
                  MiniDumpDF,
                  g);

  theCommands.Add("CopyLabel", "CopyLabel (DOC, from, to)", __FILE__, CopyLabel_SCopy, g);

  theCommands.Add("CheckAttrs", "CheckAttrs DocName Lab1 Lab2 ", __FILE__, DDF_CheckAttrs, g);

  theCommands.Add("CheckLabel", "CheckLabel DocName Label ", __FILE__, DDF_CheckLabel, g);

  theCommands.Add("SetAccessByEntry",
                  "SetAccessByEntry DOC 1|0",
                  __FILE__,
                  DDF_SetAccessByEntry,
                  g);
}
