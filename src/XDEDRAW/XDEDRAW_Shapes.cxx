// Created on: 2000-08-04
// Created by: Pavel TELKOV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <BRep_Builder.hxx>
#include <DBRep.hxx>
#include <DDocStd.hxx>
#include <Draw.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Message.hxx>
#include <NCollection_DataMap.hxx>
#include <TCollection_AsciiString.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>
#include <TDocStd_Document.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XDEDRAW_Shapes.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_ShapeProcessor.hxx>
#include <UnitsMethods.hxx>

#include <stdio.h>

//=======================================================================
// Section: Work with shapes
//=======================================================================
static Standard_Integer addShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " DocName Shape [int makeAssembly (1/0)]\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  TopoShape aShape = DBRep1::Get(argv[2]);
  if (aShape.IsNull())
  {
    di << "Syntax error: shape '" << argv[2] << "' is undefined\n";
    return 1;
  }

  Standard_Real aLengthUnit = 1.;
  if (!XCAFDoc_DocumentTool::GetLengthUnit(Doc, aLengthUnit))
  {
    XSAlgo_ShapeProcessor::PrepareForTransfer(); // update unit info
    aLengthUnit = UnitsMethods::GetCasCadeLengthUnit(UnitsMethods_LengthUnit_Meter);
    XCAFDoc_DocumentTool::SetLengthUnit(Doc, aLengthUnit);
  }

  Handle(XCAFDoc_ShapeTool) myAssembly   = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  Standard_Boolean          makeAssembly = Standard_True;
  if (argc == 4 && Draw1::Atoi(argv[3]) == 0)
    makeAssembly = Standard_False;
  DataLabel aLabel;
  aLabel = myAssembly->AddShape(aShape, makeAssembly);
  if (aLabel.IsNull())
    di << "Null Label\n";
  AsciiString1 Entry;
  TDF_Tool::Entry(aLabel, Entry);
  di << Entry.ToCString();
  return 0;
}

static Standard_Integer newShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 2)
  {
    di << "Use: " << argv[0] << " DocName \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DataLabel                aLabel;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  // XCAFDoc_ShapeTool myAssembly;
  //   myAssembly.Init(Doc);
  aLabel = myAssembly->NewShape();
  //  di<<"New Shape at ChildTag"<<aLabel.Tag()<<"\n";
  AsciiString1 Entry;
  TDF_Tool::Entry(aLabel, Entry);
  di << Entry.ToCString();
  return 0;
}

static Standard_Integer setShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Use: " << argv[0] << " DocName Label Shape \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  TopoShape aShape;
  //   if (aLabel.IsNull()) di<<"Null Label\n";
  aShape = DBRep1::Get(argv[3]);
  //  XCAFDoc_ShapeTool myAssembly;
  //  myAssembly.Init(Doc);
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  myAssembly->SetShape(aLabel, aShape);
  return 0;
}

static Standard_Integer getShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Use: " << argv[0] << " Result DocName Label\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[2], Doc);
  if (Doc.IsNull())
  {
    di << argv[2] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[3], aLabel);
  if (aLabel.IsNull())
  {
    di << "No such Label\n";
    return 1;
  }
  TopoShape aShape;
  //  XCAFDoc_ShapeTool myAssembly;
  //  myAssembly.Init(Doc);
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  aShape                               = myAssembly->GetShape(aLabel);
  Standard_CString name1               = argv[1];
  DBRep1::Set(name1, aShape);

  return 0;
}

static Standard_Integer removeShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3 && argc != 4)
  {
    di << "Use: " << argv[0] << " DocName Label [int removeCompletely (1/0)]\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  if (aLabel.IsNull())
  {
    di << "No such Label\n";
    return 1;
  }
  TopoShape aShape;
  //  XCAFDoc_ShapeTool myAssembly;
  //  myAssembly.Init(Doc);
  Handle(XCAFDoc_ShapeTool) myAssembly       = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  Standard_Boolean          removeCompletely = Standard_True;
  if (argc == 4 && Draw1::Atoi(argv[3]) == 0)
    removeCompletely = Standard_False;
  myAssembly->RemoveShape(aLabel, removeCompletely);

  return 0;
}

static Standard_Integer findShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " DocName Shape [0/1]\n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel    aLabel;
  TopoShape aShape;
  aShape = DBRep1::Get(argv[2]);
  //  XCAFDoc_ShapeTool myAssembly;
  //  myAssembly.Init(Doc);
  Handle(XCAFDoc_ShapeTool) myAssembly   = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  Standard_Boolean          findInstance = ((argc == 4) && argv[3][0] == '1');
  aLabel                                 = myAssembly->FindShape(aShape, findInstance);
  AsciiString1 Entry;
  TDF_Tool::Entry(aLabel, Entry);
  di << Entry.ToCString();
  // di<<"Label with Shape is "<<Entry<<"\n";
  return 0;
}

static Standard_Integer findSubShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Use: " << argv[0] << " DocName Shape ParentLabel\n";
    return 1;
  }
  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(argv[1], aDoc);
  if (aDoc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  TopoShape aShape;
  aShape = DBRep1::Get(argv[2]);

  DataLabel aParentLabel;
  TDF_Tool::Label(aDoc->GetData(), argv[3], aParentLabel);

  DataLabel                 aLabel;
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  aShapeTool->FindSubShape(aParentLabel, aShape, aLabel);

  AsciiString1 anEntry;
  TDF_Tool::Entry(aLabel, anEntry);
  di << anEntry.ToCString();
  return 0;
}

static Standard_Integer findMainShape(DrawInterpreter& di,
                                      Standard_Integer  argc,
                                      const char**      argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " DocName SubShape\n";
    return 1;
  }
  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(argv[1], aDoc);
  if (aDoc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  TopoShape aShape;
  aShape = DBRep1::Get(argv[2]);

  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  DataLabel                 aLabel     = aShapeTool->FindMainShape(aShape);

  AsciiString1 anEntry;
  TDF_Tool::Entry(aLabel, anEntry);
  di << anEntry.ToCString();
  return 0;
}

static Standard_Integer addSubShape(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Use: " << argv[0] << " DocName Shape ParentLabel\n";
    return 1;
  }
  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(argv[1], aDoc);
  if (aDoc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  TopoShape aShape;
  aShape = DBRep1::Get(argv[2]);

  DataLabel aParentLabel;
  TDF_Tool::Label(aDoc->GetData(), argv[3], aParentLabel);

  DataLabel                 aLabel;
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  aLabel                               = aShapeTool->AddSubShape(aParentLabel, aShape);

  AsciiString1 anEntry;
  TDF_Tool::Entry(aLabel, anEntry);
  di << anEntry.ToCString();
  return 0;
}

static Standard_Integer labelInfo(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " DocName Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  //  XCAFDoc_ShapeTool myAssembly;
  //  myAssembly.Init(Doc);
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  AsciiString1   Entry;

  if (myAssembly->IsShape(aLabel))
  {
    // di<<"There are a TopLevelShape\n";
    Entry = "There are a TopLevelShape";
    di << Entry.ToCString();
  }
  if (myAssembly->IsComponent(aLabel))
  {
    // di<<"There are a Component\n";
    Entry = "There are a Component";
    di << Entry.ToCString();
  }
  if (myAssembly->IsAssembly(aLabel))
  {
    // di<<"There are an Assembly\n";
    Entry = "There are an Assembly";
    di << Entry.ToCString();
  }
  if (myAssembly->IsFree(aLabel))
  {
    // di<<"This Shape don't used\n";
    Entry = "This Shape don't used";
    di << Entry.ToCString();
  }
  return 0;
}

static Standard_Integer getUsers(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc Label [withSubChilds(int)]\n";
    return 1;
  }
  Standard_Boolean getsubchilds = Standard_False;
  if ((argc == 4) && (Draw1::Atoi(argv[3]) == 1))
    getsubchilds = Standard_True;

  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  TDF_LabelSequence         labseq;
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  AsciiString1   Entry;
  Entry = myAssembly->GetUsers(aLabel, labseq, getsubchilds);
  di << Entry.ToCString();
  // di<<myAssembly->GetUsers(aLabel, labseq, getsubchilds)<<" assemblies use this component\n";
  return 0;
}

static Standard_Integer nbComponents(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc Label [withSubChilds(int)]\n";
    return 1;
  }
  Standard_Boolean getsubchilds = Standard_False;
  if ((argc == 4) && (Draw1::Atoi(argv[3]) == 1))
    getsubchilds = Standard_True;
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  //  XCAFDoc_ShapeTool myAssembly->
  //  myAssembly->Init(Doc);
  // di<<"This assembly has ";
  AsciiString1 Entry;
  Entry = myAssembly->NbComponents(aLabel, getsubchilds);
  di << Entry.ToCString();
  // di<<" components\n";
  // di<<"This assembly has "<<myAssembly->NbComponents( aLabel, getsubchilds )<<" components\n";

  return 0;
}

static Standard_Integer addComponent(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc != 4)
  {
    di << "Use: " << argv[0] << " DocName Label Shape \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  TopoShape aShape;
  aShape                               = DBRep1::Get(argv[3]);
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  //  XCAFDoc_ShapeTool myAssembly->
  //  myAssembly->Init(Doc);
  myAssembly->AddComponent(aLabel, aShape);
  AsciiString1 Entry;
  TDF_Tool::Entry(aLabel, Entry);
  di << Entry.ToCString();

  return 0;
}

static Standard_Integer removeComponent(DrawInterpreter& di,
                                        Standard_Integer  argc,
                                        const char**      argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " DocName Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  //  XCAFDoc_ShapeTool myAssembly->
  //  myAssembly->Init(Doc);
  myAssembly->RemoveComponent(aLabel);
  return 0;
}

static Standard_Integer getReferredShape(DrawInterpreter& di,
                                         Standard_Integer  argc,
                                         const char**      argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " DocName Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel, RootLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  //  XCAFDoc_ShapeTool myAssembly->
  //  myAssembly->Init(Doc);
  myAssembly->GetReferredShape(aLabel, RootLabel);

  AsciiString1 Entry;
  TDF_Tool::Entry(RootLabel, Entry);
  // di<<"Label with Shape is ";
  di << Entry.ToCString();
  return 0;
}

static Standard_Integer getTopLevelShapes(DrawInterpreter& di,
                                          Standard_Integer  argc,
                                          const char**      argv)
{
  if (argc != 2)
  {
    di << "Use: " << argv[0] << " DocName \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel         aLabel;
  TDF_LabelSequence Labels;

  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  //  XCAFDoc_ShapeTool myAssembly->
  //  myAssembly->Init(Doc);
  myAssembly->GetShapes(Labels);
  AsciiString1 Entry;
  if (Labels.Length() >= 1)
  {
    for (Standard_Integer i = 1; i <= Labels.Length(); i++)
    {
      aLabel = Labels.Value(i);
      TDF_Tool::Entry(aLabel, Entry);
      di << Entry.ToCString() << " ";
    }
  }
  return 0;
}

static Standard_Integer getFreeShapes(DrawInterpreter& di,
                                      Standard_Integer  argc,
                                      const char**      argv)
{
  if (argc < 2)
  {
    di << "Use: " << argv[0] << " DocName [shape_prefix]\n";
    return 1;
  }

  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  TDF_LabelSequence         Labels;
  Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  STool->GetFreeShapes(Labels);
  if (Labels.Length() <= 0)
  {
    di << "Document " << argv[1] << " contain no shapes\n";
    return 0;
  }

  if (argc == 2)
  {
    for (Standard_Integer i = 1; i <= Labels.Length(); i++)
    {
      AsciiString1 Entry;
      TDF_Tool::Entry(Labels.Value(i), Entry);
      di << Entry.ToCString() << " ";
    }
  }
  else if (Labels.Length() == 1)
  {
    TopoShape S = STool->GetShape(Labels.Value(1));
    DBRep1::Set(argv[2], S);
    di << argv[2];
  }
  else
  {
    for (Standard_Integer i = 1; i <= Labels.Length(); i++)
    {
      TopoShape S = STool->GetShape(Labels.Value(i));
      char         string[260];
      Sprintf(string, "%s_%d", argv[2], i);
      DBRep1::Set(string, S);
      di << string << " ";
    }
  }
  return 0;
}

//=================================================================================================

static Standard_Integer getOneShape(DrawInterpreter& theDI,
                                    Standard_Integer  theNbArgs,
                                    const char**      theArgVec)
{
  if (theNbArgs != 3)
  {
    theDI << "Use: " << theArgVec[0] << " shape DocName \n";
    return 1;
  }

  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(theArgVec[2], aDoc);
  if (aDoc.IsNull())
  {
    theDI << "Error: " << theArgVec[2] << " is not a document\n";
    return 1;
  }

  Handle(XCAFDoc_ShapeTool) aSTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  TopoShape              aShape = aSTool->GetOneShape();
  if (aShape.IsNull())
  {
    theDI << "Error: Document " << theArgVec[2] << " contain no shapes\n";
    return 1;
  }
  DBRep1::Set(theArgVec[1], aShape);
  theDI << theArgVec[1];
  return 0;
}

//=======================================================================
// function : XDumpLocation
// purpose  : Dump Transformation() of XCAFDoc_Location attribute
//=======================================================================
static Standard_Integer XDumpLocation(DrawInterpreter& di,
                                      Standard_Integer  argc,
                                      const char**      argv)
{
  if (argc != 3)
  {
    di << "Use: " << argv[0] << " Doc Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  DataLabel aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[2], aLabel);

  Handle(XCAFDoc_Location) aLoc;
  if (!aLabel.FindAttribute(XCAFDoc_Location::GetID(), aLoc))
  {
    di << "Label " << argv[2] << " doesn't contain XCAFDoc_Location attribute\n";
    return 1;
  }

  TopLoc_Location aTopLoc = aLoc->Get();
  Transform3d         aTrsf   = aTopLoc.Transformation();

  di << "Transformation (3 rows * 4 columns matrix):";
  for (int i = 1; i <= 3; i++) // row number
  {
    di << " (";
    for (int j = 1; j <= 4; j++) // column number
    {
      if (j > 1)
        di << ",";
      di << AsciiString1(aTrsf.Value(i, j)).ToCString();
    }
    di << ")";
  }

  return 0;
}

static Standard_Integer setSHUO(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 4)
  {
    di << "Use: " << argv[0] << " Doc UU_Label NU_Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }

  TDF_LabelSequence aLabSeq;
  for (Standard_Integer i = 3; i <= argc; i++)
  {
    DataLabel L;
    TDF_Tool::Label(Doc->GetData(), argv[i - 1], L);
    if (!L.IsNull())
      aLabSeq.Append(L);
    else
      di << argv[i - 1] << " is null label" << "\n";
  }
  if (aLabSeq.Length() < 2)
  {
    di << "Error: couldnot set SHUO between on less then 2 labels\n";
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  Handle(XCAFDoc_GraphNode) aMainSHUO;
  myAssembly->SetSHUO(aLabSeq, aMainSHUO);
  if (aMainSHUO.IsNull())
  {
    di << "Error: cannot set the SHUO\n";
    return 1;
  }

  return 0;
}

static Standard_Integer getSHUOUpperUsage(DrawInterpreter& di,
                                          Standard_Integer  argc,
                                          const char**      argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc NU_Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  DataLabel NL;
  TDF_Tool::Label(Doc->GetData(), argv[2], NL);
  if (NL.IsNull())
  {
    di << argv[2] << " is null label" << "\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TDF_LabelSequence         labseq;
  myAssembly->GetSHUOUpperUsage(NL, labseq);
  AsciiString1 Entry;
  if (labseq.Length() >= 1)
  {
    for (Standard_Integer i = 1; i <= labseq.Length(); i++)
    {
      DataLabel aLabel = labseq.Value(i);
      TDF_Tool::Entry(aLabel, Entry);
      di << Entry.ToCString() << " ";
    }
  }
  return 0;
}

static Standard_Integer getSHUONextUsage(DrawInterpreter& di,
                                         Standard_Integer  argc,
                                         const char**      argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc UU_Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  DataLabel UL;
  TDF_Tool::Label(Doc->GetData(), argv[2], UL);
  if (UL.IsNull())
  {
    di << argv[2] << " is null label" << "\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TDF_LabelSequence         labseq;
  myAssembly->GetSHUONextUsage(UL, labseq);
  AsciiString1 Entry;
  if (labseq.Length() >= 1)
  {
    for (Standard_Integer i = 1; i <= labseq.Length(); i++)
    {
      DataLabel aLabel = labseq.Value(i);
      TDF_Tool::Entry(aLabel, Entry);
      di << Entry.ToCString() << " ";
    }
  }
  return 0;
}

static Standard_Integer removeSHUO(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc SHUOComponent_Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  DataLabel UL;
  TDF_Tool::Label(Doc->GetData(), argv[2], UL);
  if (UL.IsNull())
  {
    di << argv[2] << " is null label" << "\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  myAssembly->RemoveSHUO(UL);

  return 0;
}

static Standard_Integer hasSHUO(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc SHUO_Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  DataLabel UL;
  TDF_Tool::Label(Doc->GetData(), argv[2], UL);
  if (UL.IsNull())
  {
    di << argv[2] << " is null label" << "\n";
    return 1;
  }
  Handle(XCAFDoc_GraphNode) anAttrSHUO;
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  if (myAssembly->GetSHUO(UL, anAttrSHUO))
    di << 1;
  else
    di << 0;

  return 0;
}

static Standard_Integer getAllSHUO(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc SHUO_Label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  DataLabel                 UL;
  TDF_Tool::Label(Doc->GetData(), argv[2], UL);
  if (UL.IsNull())
  {
    di << argv[2] << " is null label" << "\n";
    return 1;
  }
  TDF_AttributeSequence SHUOAttrs;
  myAssembly->GetAllComponentSHUO(UL, SHUOAttrs);
  AsciiString1 Entry;
  if (SHUOAttrs.Length() >= 1)
  {
    for (Standard_Integer i = 1; i <= SHUOAttrs.Length(); i++)
    {
      DataLabel aLabel = SHUOAttrs.Value(i)->Label();
      TDF_Tool::Entry(aLabel, Entry);
      di << Entry.ToCString() << " ";
    }
  }
  return 0;
}

static Standard_Integer findComponent(DrawInterpreter& di,
                                      Standard_Integer  argc,
                                      const char**      argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc shape \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TopoShape              aShape;
  aShape = DBRep1::Get(argv[2]);
  TDF_LabelSequence labseq;
  myAssembly->FindComponent(aShape, labseq);
  AsciiString1 Entry;
  if (labseq.Length() >= 1)
  {
    for (Standard_Integer i = 1; i <= labseq.Length(); i++)
    {
      DataLabel aLabel = labseq.Value(i);
      TDF_Tool::Entry(aLabel, Entry);
      di << Entry.ToCString() << " ";
    }
  }
  return 0;
}

static Standard_Integer getStyledComponent(DrawInterpreter& di,
                                           Standard_Integer  argc,
                                           const char**      argv)
{
  if (argc < 4)
  {
    di << "Use: " << argv[0] << " Doc res SHUO_label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TopoShape              aShape;
  DataLabel                 aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[3], aLabel);
  Handle(XCAFDoc_GraphNode) SHUO;
  if (myAssembly->GetSHUO(aLabel, SHUO))
    aShape = myAssembly->GetSHUOInstance(SHUO);

  if (aShape.IsNull())
  {
    di << "cannot get component\n";
    return 1;
  }
  DBRep1::Set(argv[2], aShape);
  di << argv[2];
  return 0;
}

static Standard_Integer getAllStyledComponents(DrawInterpreter& di,
                                               Standard_Integer  argc,
                                               const char**      argv)
{
  if (argc < 4)
  {
    di << "Use: " << argv[0] << " Doc res SHUO_label \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TopTools_SequenceOfShape  aShapes;
  DataLabel                 aLabel;
  TDF_Tool::Label(Doc->GetData(), argv[3], aLabel);
  Handle(XCAFDoc_GraphNode) SHUO;
  if (myAssembly->GetSHUO(aLabel, SHUO))
    if (myAssembly->GetAllSHUOInstances(SHUO, aShapes))
    {
      TopoCompound aShape;
      ShapeBuilder    B;
      B.MakeCompound(aShape);
      for (Standard_Integer jj = 1; jj <= aShapes.Length(); jj++)
      {
        const TopoShape& aCurShape = aShapes.Value(jj);
        B.Add(aShape, aCurShape);
      }
      DBRep1::Set(argv[2], aShape);
      di << argv[2];
    }

  return 0;
}

static Standard_Integer findSHUO(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 4)
  {
    di << "Use: " << argv[0] << " Doc labels \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TDF_LabelSequence         aLabSeq;
  for (Standard_Integer i = 3; i <= argc; i++)
  {
    DataLabel L;
    TDF_Tool::Label(Doc->GetData(), argv[i - 1], L);
    if (!L.IsNull())
      aLabSeq.Append(L);
    else
      di << argv[i - 1] << " is null label" << "\n";
  }
  if (aLabSeq.Length() < 2)
  {
    di << "Error: couldnot find SHUO between on less then 2 labels\n";
  }
  Handle(XCAFDoc_GraphNode) SHUO;
  myAssembly->FindSHUO(aLabSeq, SHUO);
  if (SHUO.IsNull())
  {
    di << "cannot find SHUO\n";
    return 1;
  }
  AsciiString1 Entry;
  TDF_Tool::Entry(SHUO->Label(), Entry);
  di << Entry.ToCString() << " ";

  return 0;
}

static Standard_Integer setStyledComponent(DrawInterpreter& di,
                                           Standard_Integer  argc,
                                           const char**      argv)
{
  if (argc < 3)
  {
    di << "Use: " << argv[0] << " Doc shape \n";
    return 1;
  }
  Handle(AppDocument) Doc;
  DDocStd1::GetDocument(argv[1], Doc);
  if (Doc.IsNull())
  {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  TopoShape              aShape;
  aShape = DBRep1::Get(argv[2]);
  if (aShape.IsNull())
  {
    di << "Shape " << argv[2] << " is null\n";
    return 1;
  }
  Handle(XCAFDoc_GraphNode) aSHUOAttr;
  aSHUOAttr = myAssembly->SetInstanceSHUO(aShape);
  if (aSHUOAttr.IsNull())
  {
    di << "Error: cannot set a SHUO structure for indicated component" << "\n";
    return 1;
  }
  AsciiString1 Entry;
  TDF_Tool::Entry(aSHUOAttr->Label(), Entry);
  di << Entry.ToCString() << " ";

  return 0;
}

static Standard_Integer updateAssemblies(DrawInterpreter& di,
                                         Standard_Integer  argc,
                                         const char**      argv)
{
  if (argc != 2)
  {
    di << "Use: " << argv[0] << " Doc\n";
    return 1;
  }

  // Get XDE document
  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(argv[1], aDoc);
  if (aDoc.IsNull())
    return 1;

  // Get XDE shape tool
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());

  // Update assemblies
  aShapeTool->UpdateAssemblies();

  return 0;
}

static Standard_Integer XGetProperties(DrawInterpreter& theDI,
                                       Standard_Integer  theArgc,
                                       const char**      theArgv)
{
  if (theArgc < 2)
  {
    theDI.PrintHelp(theArgv[0]);
    return 1;
  }

  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(theArgv[1], aDoc);
  if (aDoc.IsNull())
  {
    theDI << "Syntax error: " << theArgv[1] << " is not a document\n";
    return 1;
  }
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  NCollection_IndexedDataMap<AsciiString1, Handle(TDataStd_NamedData)> aNameDataMap;
  for (Standard_Integer anInd = 2; anInd < theArgc; anInd++)
  {
    DataLabel                     aLabel;
    const AsciiString1 anEntry = theArgv[anInd];
    TDF_Tool::Label(aDoc->GetData(), anEntry, aLabel);
    if (aLabel.IsNull())
    {
      TopoShape aShape = DBRep1::Get(theArgv[anInd]);
      if (!aShape.IsNull())
      {
        aLabel = aShapeTool->FindShape(aShape);
      }
    }
    if (!aLabel.IsNull())
    {
      Handle(TDataStd_NamedData) aNamedData = aShapeTool->GetNamedProperties(aLabel);
      if (!aNamedData.IsNull())
      {
        aNameDataMap.Add(anEntry, aNamedData);
      }
    }
    else
    {
      Message::SendWarning() << "Warning: incorrect argument [" << theArgv[anInd] << "]"
                             << " is not a label";
    }
  }
  if (theArgc == 2)
  {
    for (TDF_ChildIterator anIter(aShapeTool->Label(), Standard_True); anIter.More(); anIter.Next())
    {
      const DataLabel&        aLabel = anIter.Value();
      AsciiString1 anEntry;
      TDF_Tool::Entry(aLabel, anEntry);
      Handle(TDataStd_NamedData) aNamedData = aShapeTool->GetNamedProperties(aLabel);
      if (!aNamedData.IsNull())
      {
        aNameDataMap.Add(anEntry, aNamedData);
      }
    }
  }
  for (NCollection_IndexedDataMap<AsciiString1, Handle(TDataStd_NamedData)>::Iterator
         aNamedDataIter(aNameDataMap);
       aNamedDataIter.More();
       aNamedDataIter.Next())
  {
    if (theArgc != 3)
    {
      theDI << "Property for [" << aNamedDataIter.Key() << "]:\n";
    }
    const Handle(TDataStd_NamedData)& aNamedData = aNamedDataIter.Value();
    aNamedData->LoadDeferredData();
    if (aNamedData->HasIntegers())
    {
      const TColStd_DataMapOfStringInteger& anIntProperties = aNamedData->GetIntegersContainer();
      for (TColStd_DataMapIteratorOfDataMapOfStringInteger anIter(anIntProperties); anIter.More();
           anIter.Next())
      {
        theDI << anIter.Key() << " : " << anIter.Value() << "\n";
      }
    }
    if (aNamedData->HasReals())
    {
      const TDataStd_DataMapOfStringReal& aRealProperties = aNamedData->GetRealsContainer();
      for (TDataStd_DataMapIteratorOfDataMapOfStringReal anIter(aRealProperties); anIter.More();
           anIter.Next())
      {
        theDI << anIter.Key() << " : " << anIter.Value() << "\n";
      }
    }
    if (aNamedData->HasStrings())
    {
      const TDataStd_DataMapOfStringString& aStringProperties = aNamedData->GetStringsContainer();
      for (TDataStd_DataMapIteratorOfDataMapOfStringString anIter(aStringProperties); anIter.More();
           anIter.Next())
      {
        theDI << anIter.Key() << " : " << anIter.Value() << "\n";
      }
    }
    if (aNamedData->HasBytes())
    {
      const TDataStd_DataMapOfStringByte& aByteProperties = aNamedData->GetBytesContainer();
      for (TDataStd_DataMapOfStringByte::Iterator anIter(aByteProperties); anIter.More();
           anIter.Next())
      {
        theDI << anIter.Key() << " : " << anIter.Value() << "\n";
      }
    }
    if (aNamedData->HasArraysOfIntegers())
    {
      const TDataStd_DataMapOfStringHArray1OfInteger& anArrayIntegerProperties =
        aNamedData->GetArraysOfIntegersContainer();
      for (TDataStd_DataMapOfStringHArray1OfInteger::Iterator anIter(anArrayIntegerProperties);
           anIter.More();
           anIter.Next())
      {
        AsciiString1 aMessage(anIter.Key() + " : ");
        for (TColStd_HArray1OfInteger::Iterator anSubIter(anIter.Value()->Array1());
             anSubIter.More();
             anSubIter.Next())
        {
          aMessage += " ";
          aMessage += anSubIter.Value();
        }
        theDI << aMessage << "\n";
      }
    }
    if (aNamedData->HasArraysOfReals())
    {
      const TDataStd_DataMapOfStringHArray1OfReal& anArrayRealsProperties =
        aNamedData->GetArraysOfRealsContainer();
      for (TDataStd_DataMapOfStringHArray1OfReal::Iterator anIter(anArrayRealsProperties);
           anIter.More();
           anIter.Next())
      {
        AsciiString1 aMessage(anIter.Key() + " : ");
        for (TColStd_HArray1OfReal::Iterator anSubIter(anIter.Value()->Array1()); anSubIter.More();
             anSubIter.Next())
        {
          aMessage += " ";
          aMessage += anSubIter.Value();
        }
        theDI << aMessage << "\n";
      }
    }
  }

  return 0;
}

static Standard_Integer XAutoNaming(DrawInterpreter& theDI,
                                    Standard_Integer  theNbArgs,
                                    const char**      theArgVec)
{
  if (theNbArgs != 2 && theNbArgs != 3)
  {
    theDI << "Syntax error: wrong number of arguments";
    return 1;
  }

  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(theArgVec[1], aDoc);
  if (aDoc.IsNull())
  {
    theDI << "Syntax error: '" << theArgVec[1] << "' is not a document";
    return 1;
  }

  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  if (theNbArgs == 2)
  {
    theDI << (aShapeTool->AutoNaming() ? "1" : "0");
    return 0;
  }

  bool toEnable = true;
  if (!Draw1::ParseOnOff(theArgVec[2], toEnable))
  {
    theDI << "Syntax error at '" << theArgVec[2] << "'";
    return 1;
  }

  aShapeTool->SetAutoNaming(toEnable);
  return 0;
}

//=======================================================================
// function : parseXYZ
// purpose  : Converts three string arguments, to gp_XYZ with check
//=======================================================================
static Standard_Boolean parseXYZ(const char** theArgVec, gp_XYZ& thePnt)
{
  const AsciiString1 aXYZ[3] = {theArgVec[0], theArgVec[1], theArgVec[2]};
  if (!aXYZ[0].IsRealValue(Standard_True) || !aXYZ[1].IsRealValue(Standard_True)
      || !aXYZ[2].IsRealValue(Standard_True))
  {
    return Standard_False;
  }

  thePnt.SetCoord(aXYZ[0].RealValue(), aXYZ[1].RealValue(), aXYZ[2].RealValue());
  return Standard_True;
}

//=======================================================================
// function : setLocation
// purpose  : Sets location to the shape at the label in XDE document
//=======================================================================
static Standard_Integer setLocation(DrawInterpreter&,
                                    Standard_Integer theArgNb,
                                    const char**     theArgVec)
{
  if (theArgNb < 4)
  {
    Message::SendFail() << "Error: not enough arguments, see help " << theArgVec[0]
                        << " for details";
    return 1;
  }
  // get and check the document
  Handle(AppDocument) aDoc;
  DDocStd1::GetDocument(theArgVec[1], aDoc);
  if (aDoc.IsNull())
  {
    Message::SendFail() << "Error: " << theArgVec[1] << " is not a document";
    return 1;
  }
  // get and check the label
  DataLabel aShapeLabel;
  TDF_Tool::Label(aDoc->GetData(), theArgVec[2], aShapeLabel);
  if (aShapeLabel.IsNull())
  {
    Message::SendFail() << "Error: no such Label: " << theArgVec[2];
    return 1;
  }
  // get the transformation
  Transform3d aTransformation;
  for (Standard_Integer anArgIter = 3; anArgIter < theArgNb; ++anArgIter)
  {
    Transform3d                 aCurTransformation;
    gp_XYZ                  aMoveXYZ, aRotPnt, aRotAxis, aScalePnt;
    Standard_Real           aRotAngle, aScale;
    AsciiString1 anArg = theArgVec[anArgIter];
    anArg.LowerCase();

    if (anArg == "-rotate" && anArgIter + 7 < theArgNb
        && parseXYZ(theArgVec + anArgIter + 1, aRotPnt)
        && parseXYZ(theArgVec + anArgIter + 4, aRotAxis)
        && Draw1::ParseReal(theArgVec[anArgIter + 7], aRotAngle))
    {
      anArgIter += 7;
      aCurTransformation.SetRotation(Axis3d(Point3d(aRotPnt), Dir3d(aRotAxis)),
                                     aRotAngle * (M_PI / 180.0));
    }
    else if (anArg == "-move" && anArgIter + 3 < theArgNb
             && parseXYZ(theArgVec + anArgIter + 1, aMoveXYZ))
    {
      anArgIter += 3;
      aCurTransformation.SetTranslation(aMoveXYZ);
    }
    // first check scale with base point
    else if (anArg == "-scale" && anArgIter + 4 < theArgNb
             && parseXYZ(theArgVec + anArgIter + 1, aScalePnt)
             && Draw1::ParseReal(theArgVec[anArgIter + 4], aScale))
    {
      anArgIter += 4;
      aCurTransformation.SetScale(Point3d(aScalePnt), aScale);
    }
    // second check for scale with scale factor only
    else if (anArg == "-scale" && anArgIter + 1 < theArgNb
             && Draw1::ParseReal(theArgVec[anArgIter + 1], aScale))
    {
      anArgIter += 1;
      aCurTransformation.SetScaleFactor(aScale);
    }
    else
    {
      Message::SendFail() << "Syntax error: unknown options '" << anArg
                          << "', or incorrect option parameters";
      return 1;
    }
    aTransformation.PreMultiply(aCurTransformation);
  }
  TopLoc_Location aLoc(aTransformation);

  // Create the ShapeTool and try to set location
  Handle(XCAFDoc_ShapeTool) anAssembly = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  DataLabel                 aRefLabel;
  if (anAssembly->SetLocation(aShapeLabel, aLoc, aRefLabel))
  {
    if (aShapeLabel == aRefLabel)
    {
      Message::SendInfo() << "New location was set";
    }
    else
    {
      AsciiString1 aLabelStr;
      TDF_Tool::Entry(aRefLabel, aLabelStr);
      Message::SendInfo() << "Reference to the shape at label " << aLabelStr
                          << " was created and location was set";
    }
  }
  else
  {
    Message::SendFail() << "Error: an attempt to set the location to a shape to which there is a "
                           "reference, or to not a shape at all";
  }

  return 0;
}

//=================================================================================================

void XDEDRAW_Shapes::InitCommands(DrawInterpreter& di)
{
  static Standard_Boolean initactor = Standard_False;
  if (initactor)
  {
    return;
  }
  initactor = Standard_True;

  //=====================================
  // Work with shapes
  //=====================================

  Standard_CString g = "XDE shape's commands";

  di.Add("XAddShape",
         "Doc Shape [makeAssembly = 1]\t: Add shape (or assembly) to Document",
         __FILE__,
         addShape,
         g);

  di.Add("XNewShape", "Doc \t: Create new empty top-level shape", __FILE__, newShape, g);

  di.Add("XSetShape", "Doc Label Shape \t: Set shape at indicated label", __FILE__, setShape, g);

  di.Add("XGetShape", "Result Doc Label \t: Put shape from tree to Result", __FILE__, getShape, g);

  di.Add("XRemoveShape", "Doc Label \t: Remove shape from document", __FILE__, removeShape, g);

  di.Add("XFindShape",
         "Doc Shape [findInstance (0/1), 0 by default]\t: Find and print label with indicated "
         "top-level shape",
         __FILE__,
         findShape,
         g);

  di.Add("XFindSubShape",
         "Doc Shape ParentLabel \t: Find subshape under given parent shape label",
         __FILE__,
         findSubShape,
         g);

  di.Add("XFindMainShape",
         "Doc SubShape \t: Find main shape for given subshape",
         __FILE__,
         findMainShape,
         g);

  di.Add("XAddSubShape",
         "Doc Shape ParentLabel \t: Add subshape under given parent shape label",
         __FILE__,
         addSubShape,
         g);

  di.Add("XLabelInfo",
         "Doc Label \t: Print information about object at following label",
         __FILE__,
         labelInfo,
         g);

  di.Add("XGetUsers",
         "Doc Label [withSubChilds(int)] \t: Print number of assemblies that use shape at "
         "following label",
         __FILE__,
         getUsers,
         g);

  di.Add("XNbComponents",
         "Doc Label [withSubChilds(int)] \t: Print number of component of assembly ",
         __FILE__,
         nbComponents,
         g);

  di.Add("XAddComponent",
         "Doc Label Shape \t: Add component shape to assembly",
         __FILE__,
         addComponent,
         g);

  di.Add("XRemoveComponent",
         "Doc Label \t: Remove component from components label",
         __FILE__,
         removeComponent,
         g);

  di.Add("XGetReferredShape",
         "Doc Label \t: Print label, that contain a top-level shape, that corresponds shape at "
         "following label",
         __FILE__,
         getReferredShape,
         g);

  di.Add("XGetTopLevelShapes",
         "Doc \t: Print labels, that contain a top-level shapes",
         __FILE__,
         getTopLevelShapes,
         g);

  di.Add("XGetFreeShapes",
         "Doc [shape_prefix]\t: Print labels or create DRAW shapes for all free shapes in the Doc",
         __FILE__,
         getFreeShapes,
         g);

  di.Add("XGetOneShape",
         "shape Doc \t: Put all free shapes of the Doc into single DRAW shape",
         __FILE__,
         getOneShape,
         g);

  di.Add("XDumpLocation",
         "Doc Label \t: Dump Transformation() of XCAFDoc_Location attribute",
         __FILE__,
         XDumpLocation,
         g);

  di.Add("XSetSHUO",
         "Doc UU_Label [ multi-level labels ] NU_Label \t: sets the SHUO structure between "
         "UpperUsage and NextUsage",
         __FILE__,
         setSHUO,
         g);

  di.Add("XGetUU_SHUO",
         "Doc NU_Label \t: prints the UpperUsages of indicated NextUsage",
         __FILE__,
         getSHUOUpperUsage,
         g);

  di.Add("XGetNU_SHUO",
         "Doc UU_Label \t: prints the NextUsages of indicated UpperUsage",
         __FILE__,
         getSHUONextUsage,
         g);

  di.Add("XRemoveSHUO",
         "Doc SHUO_Label \t: remove SHUO of indicated component",
         __FILE__,
         removeSHUO,
         g);

  di.Add("XIsHasSHUO",
         "Doc SHUO_Label \t: remove SHUO of indicated component",
         __FILE__,
         hasSHUO,
         g);

  di.Add("XGetAllSHUO",
         "Doc Comp_Label \t: remove SHUO of indicated component",
         __FILE__,
         getAllSHUO,
         g);

  di.Add("XFindComponent",
         "Doc Shape \t: prints sequence of labels of assembly path",
         __FILE__,
         findComponent,
         g);

  di.Add("XGetSHUOInstance",
         "Doc res SHUO_Label \t: returns SHUO_styled shape",
         __FILE__,
         getStyledComponent,
         g);

  di.Add("XGetAllSHUOInstances",
         "Doc res SHUO_Label \t: returns SHUO_styled shapes as compound",
         __FILE__,
         getAllStyledComponents,
         g);

  di.Add("XFindSHUO",
         "Doc labels of SHUO structure \t: prints label of SHUO that found by labels structure",
         __FILE__,
         findSHUO,
         g);

  di.Add("XSetInstanceSHUO",
         "Doc shape \t: sets the SHUO structure for indicated component",
         __FILE__,
         setStyledComponent,
         g);

  di.Add("XSetLocation",
         R"(
Doc Label transformation [transformation ... ]
Applies given complex transformation to the shape at Label from Document.
The label may contain a reference to a shape, an assembly or simple shape.
The assembly or simple shape should not be referred by any reference.
Transformations:
  '-move x y z'                     - move shape
  '-rotate x y z dx dy dz angle'    - rotate shape
  '-scale [x y z] factor'           - scale shape
Transformations are applied from left to right.
There can be more than one transformation of the same type.
At least one transformation must be specified.
)",
         __FILE__,
         setLocation,
         g);

  di.Add("XUpdateAssemblies", "Doc \t: updates assembly compounds", __FILE__, updateAssemblies, g);

  di.Add("XGetProperties",
         "Doc [label1, label2, ...] [shape1, shape2, ...]\t: prints named properties assigned to "
         "the all document's shape labels or chosen labels of shapes",
         __FILE__,
         XGetProperties,
         g);

  di.Add("XAutoNaming",
         "Doc [0|1]\t: Disable/enable autonaming to Document",
         __FILE__,
         XAutoNaming,
         g);
}
