#pragma once

#include "antlr4-runtime.h"
#include "AslBaseListener.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/SemErrors.h"

// using namespace std;


//////////////////////////////////////////////////////////////////////
// Class SymbolListener:  derived from AslBaseListener.
// The tree walker go through the parse tree and call the methods of
// this listener to register the symbols of the program in the symbol
// table. If an enter/exit method does not have an associated task,
// it does not have to be redefined.

class SymbolsListener final : public AslBaseListener {

public:

  // Constructor
  SymbolsListener(TypesMgr       & Types,
		  SymTable       & Symbols,
		  TreeDecoration & TreeNodeProps,
		  SemErrors      & Errors);

  void enterProgram(AslParser::ProgramContext *ctx);
  void exitProgram(AslParser::ProgramContext *ctx);

  void enterFunction(AslParser::FunctionContext *ctx);
  void exitFunction(AslParser::FunctionContext *ctx);

  void enterFunc_decl_params(AslParser::Func_decl_paramsContext *ctx);
  void exitFunc_decl_params(AslParser::Func_decl_paramsContext *ctx);

  void enterDeclarations(AslParser::DeclarationsContext *ctx);
  void exitDeclarations(AslParser::DeclarationsContext *ctx);

  void enterArray_decl(AslParser::Array_declContext * ctx);
  void exitArray_decl(AslParser::Array_declContext * ctx);

  void enterVariable_decl(AslParser::Variable_declContext *ctx);
  void exitVariable_decl(AslParser::Variable_declContext *ctx);

  void enterType(AslParser::TypeContext *ctx);
  void exitType(AslParser::TypeContext *ctx);

  void enterBasic_type(AslParser::Basic_typeContext *ctx);
  void exitBasic_type(AslParser::Basic_typeContext *ctx);

  void enterStatements(AslParser::StatementsContext *ctx);
  void exitStatements(AslParser::StatementsContext *ctx);

  void enterAssignStmt(AslParser::AssignStmtContext *ctx);
  void exitAssignStmt(AslParser::AssignStmtContext *ctx);

  void enterIfStmt(AslParser::IfStmtContext *ctx);
  void exitIfStmt(AslParser::IfStmtContext *ctx);

  void enterProcCall(AslParser::ProcCallContext *ctx);
  void exitProcCall(AslParser::ProcCallContext *ctx);

  void enterReadStmt(AslParser::ReadStmtContext *ctx);
  void exitReadStmt(AslParser::ReadStmtContext *ctx);

  void enterWriteExpr(AslParser::WriteExprContext *ctx);
  void exitWriteExpr(AslParser::WriteExprContext *ctx);

  void enterWriteString(AslParser::WriteStringContext *ctx);
  void exitWriteString(AslParser::WriteStringContext *ctx);

  void enterLeft_expr(AslParser::Left_exprContext *ctx);
  void exitLeft_expr(AslParser::Left_exprContext *ctx);

  void enterArithmetic(AslParser::ArithmeticContext *ctx);
  void exitArithmetic(AslParser::ArithmeticContext *ctx);

  void enterRelational(AslParser::RelationalContext *ctx);
  void exitRelational(AslParser::RelationalContext *ctx);

  void enterValue(AslParser::ValueContext *ctx);
  void exitValue(AslParser::ValueContext *ctx);

  void enterLogical(AslParser::LogicalContext *ctx);
  void exitLogical(AslParser::LogicalContext *ctx);

  void enterExprIdent(AslParser::ExprIdentContext *ctx);
  void exitExprIdent(AslParser::ExprIdentContext *ctx);

  void enterIdent(AslParser::IdentContext *ctx);
  void exitIdent(AslParser::IdentContext *ctx);

  // void enterEveryRule(antlr4::ParserRuleContext *ctx);
  // void exitEveryRule(antlr4::ParserRuleContext *ctx);
  // void visitTerminal(antlr4::tree::TerminalNode *node);
  // void visitErrorNode(antlr4::tree::ErrorNode *node);

private:

  // Attributes:
  TypesMgr       & Types;
  SymTable       & Symbols;
  TreeDecoration & Decorations;
  SemErrors      & Errors;

  // Getters for the necessary tree node atributes:
  //   Scope and Type
  SymTable::ScopeId getScopeDecor (antlr4::ParserRuleContext *ctx);
  TypesMgr::TypeId  getTypeDecor  (antlr4::ParserRuleContext *ctx);

  // Setters for the necessary tree node attributes:
  //   Scope and Type
  void putScopeDecor (antlr4::ParserRuleContext *ctx, SymTable::ScopeId s);
  void putTypeDecor  (antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t);

};  // class SymbolsListener
