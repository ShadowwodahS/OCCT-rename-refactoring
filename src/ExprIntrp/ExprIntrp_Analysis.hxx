// Created on: 1992-02-21
// Created by: Arnaud BOUZY
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _ExprIntrp_Analysis_HeaderFile
#define _ExprIntrp_Analysis_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <ExprIntrp_StackOfGeneralExpression.hxx>
#include <ExprIntrp_StackOfGeneralRelation.hxx>
#include <ExprIntrp_StackOfGeneralFunction.hxx>
#include <TColStd_ListOfAsciiString.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <ExprIntrp_SequenceOfNamedFunction.hxx>
#include <ExprIntrp_SequenceOfNamedExpression.hxx>
#include <Standard_Integer.hxx>
class ExpressionGenerator;
class Expression1;
class Expr_GeneralRelation;
class AsciiString1;
class Expr_GeneralFunction;
class Expr_NamedFunction;
class Expr_NamedExpression;

class ExprIntrp_Analysis
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT ExprIntrp_Analysis();

  Standard_EXPORT void SetMaster(const Handle(ExpressionGenerator)& agen);

  Standard_EXPORT void Push(const Handle(Expression1)& exp);

  Standard_EXPORT void PushRelation(const Handle(Expr_GeneralRelation)& rel);

  Standard_EXPORT void PushName(const AsciiString1& name);

  Standard_EXPORT void PushValue(const Standard_Integer degree);

  Standard_EXPORT void PushFunction(const Handle(Expr_GeneralFunction)& func);

  Standard_EXPORT Handle(Expression1) Pop();

  Standard_EXPORT Handle(Expr_GeneralRelation) PopRelation();

  Standard_EXPORT AsciiString1 PopName();

  Standard_EXPORT Standard_Integer PopValue();

  Standard_EXPORT Handle(Expr_GeneralFunction) PopFunction();

  Standard_EXPORT Standard_Boolean IsExpStackEmpty() const;

  Standard_EXPORT Standard_Boolean IsRelStackEmpty() const;

  Standard_EXPORT void ResetAll();

  Standard_EXPORT void Use(const Handle(Expr_NamedFunction)& func);

  Standard_EXPORT void Use(const Handle(Expr_NamedExpression)& named);

  Standard_EXPORT Handle(Expr_NamedExpression) GetNamed(const AsciiString1& name);

  Standard_EXPORT Handle(Expr_NamedFunction) GetFunction(const AsciiString1& name);

protected:
private:
  ExprIntrp_StackOfGeneralExpression  myGEStack;
  ExprIntrp_StackOfGeneralRelation    myGRStack;
  ExprIntrp_StackOfGeneralFunction    myGFStack;
  TColStd_ListOfAsciiString           myNameStack;
  TColStd_ListOfInteger               myValueStack;
  ExprIntrp_SequenceOfNamedFunction   myFunctions;
  ExprIntrp_SequenceOfNamedExpression myNamed;
  Handle(ExpressionGenerator)         myMaster;
};

#endif // _ExprIntrp_Analysis_HeaderFile
