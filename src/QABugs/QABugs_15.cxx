// Created on: 2002-04-09
// Created by: QA Admin
// Copyright (c) 2002-2014 OPEN CASCADE SAS
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

#include <QABugs.hxx>

#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <AIS_InteractiveContext.hxx>
#include <ViewerTest.hxx>

#include <QABugs_PresentableObject.hxx>

Handle(QABugs_PresentableObject) theObject1 = NULL;
Handle(QABugs_PresentableObject) theObject2 = NULL;

static Standard_Integer BUC60720(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  Handle(VisualContext) myAISContext = ViewerTest1::GetAISContext();
  if (myAISContext.IsNull())
  {
    di << "use 'vinit' command before " << argv[0] << "\n";
    return -1;
  }

  if (argc != 2)
  {
    di << "Usage : " << argv[0] << " 0/1\n";
  }

  if (Draw1::Atoi(argv[1]) == 0)
  {
    if (theObject1.IsNull())
    {
      theObject1 = new QABugs_PresentableObject();
      theObject1->SetDisplayMode(0);
      myAISContext->Display(theObject1, Standard_True);
    }
  }
  else if (Draw1::Atoi(argv[1]) == 1)
  {
    if (theObject2.IsNull())
    {
      theObject2 = new QABugs_PresentableObject();
      theObject2->SetDisplayMode(1);
      myAISContext->Display(theObject2, Standard_True);
    }
  }
  else
  {
    di << "Usage : " << argv[0] << " 0/1\n";
    return -1;
  }
  return 0;
}

void QABugs1::Commands_15(DrawInterpreter& theCommands)
{
  const char* group = "QABugs1";
  theCommands.Add("BUC60720", "BUC60720 0/1", __FILE__, BUC60720, group);
}
