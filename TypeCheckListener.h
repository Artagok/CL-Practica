
#pragma once

#include "antlr4-runtime.h"
#include "AslBaseListener.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/SemErrors.h"

// using namespace std;


//////////////////////////////////////////////////////////////////////
// Class TypeCheckListener: derived from AslBaseListener.
// The tree walker go through the parser tree and call the methods of
// this listener to do the semantic typecheck of the program. This is
// done once the SymbolsListener has finish and all the symbols of the
// program has been added to their respective scope. If an enter/exit
// method does not have an associated task, it does not have to be
// redefined.

class TypeCheckListener final : public AslBaseListener {

public:

  // Constructor
  TypeCheckListener(TypesMgr       & Types,
		    SymTable       & Symbols,
		    TreeDecoration & Decorations,
		    SemErrors      & Errors);

  void enterProgram(AslParser::ProgramContext *ctx);
  void exitProgram(AslParser::ProgramContext *ctx);

  void enterFunction(AslParser::FunctionContext *ctx);
  void exitFunction(AslParser::FunctionContext *ctx);

  void enterDeclarations(AslParser::DeclarationsContext *ctx);
  void exitDeclarations(AslParser::DeclarationsContext *ctx);

  void enterVariable_decl(AslParser::Variable_declContext *ctx);
  void exitVariable_decl(AslParser::Variable_declContext *ctx);

  void enterType(AslParser::TypeContext *ctx);
  void exitType(AslParser::TypeContext *ctx);

  void enterStatements(AslParser::StatementsContext *ctx);
  void exitStatements(AslParser::StatementsContext *ctx);

  void enterAssignStmt(AslParser::AssignStmtContext *ctx);
  void exitAssignStmt(AslParser::AssignStmtContext *ctx);

  void enterIfStmt(AslParser::IfStmtContext *ctx);
  void exitIfStmt(AslParser::IfStmtContext *ctx);

  void enterWhileStmt(AslParser::WhileStmtContext * ctx); 
  void exitWhileStmt(AslParser::WhileStmtContext * ctx);

  void enterProcCall(AslParser::ProcCallContext *ctx);
  void exitProcCall(AslParser::ProcCallContext *ctx);

  void enterReturnStmt(AslParser::ReturnStmtContext * ctx);
  void exitReturnStmt(AslParser::ReturnStmtContext * ctx);

  void enterReadStmt(AslParser::ReadStmtContext *ctx);
  void exitReadStmt(AslParser::ReadStmtContext *ctx);

  void enterWriteExpr(AslParser::WriteExprContext *ctx);
  void exitWriteExpr(AslParser::WriteExprContext *ctx);

  void enterWriteString(AslParser::WriteStringContext *ctx);
  void exitWriteString(AslParser::WriteStringContext *ctx);

  void enterLeft_expr(AslParser::Left_exprContext *ctx);
  void exitLeft_expr(AslParser::Left_exprContext *ctx);

  void enterArrayIndex(AslParser::ArrayIndexContext *ctx);
  void exitArrayIndex(AslParser::ArrayIndexContext *ctx);

  void enterFuncCall(AslParser::FuncCallContext *ctx);
  void exitFuncCall(AslParser::FuncCallContext *ctx);

  void enterUnary(AslParser::UnaryContext * ctx);
  void exitUnary(AslParser::UnaryContext * ctx);

  void enterArithmetic(AslParser::ArithmeticContext *ctx);
  void exitArithmetic(AslParser::ArithmeticContext *ctx);

  void enterRelational(AslParser::RelationalContext *ctx);
  void exitRelational(AslParser::RelationalContext *ctx);

  void enterLogical(AslParser::LogicalContext * ctx);
  void exitLogical(AslParser::LogicalContext * ctx);

  void enterParenthesis(AslParser::ParenthesisContext * ctx);
  void exitParenthesis(AslParser::ParenthesisContext * ctx);

  void enterValue(AslParser::ValueContext *ctx);
  void exitValue(AslParser::ValueContext *ctx);

  void enterExprIdent(AslParser::ExprIdentContext *ctx);
  void exitExprIdent(AslParser::ExprIdentContext *ctx);

  void enterIdent(AslParser::IdentContext *ctx);
  void exitIdent(AslParser::IdentContext *ctx);

  // void enterEveryRule(antlr4::ParserRuleContext *ctx);
  // void exitEveryRule(antlr4::ParserRuleContext *ctx);
  // void visitTerminal(antlr4::tree::TerminalNode *node);
  // void visitErrorNode(antlr4::tree::ErrorNode *node);

private:

  // Attributes
  TypesMgr       & Types;
  SymTable       & Symbols;
  TreeDecoration & Decorations;
  SemErrors      & Errors;

  // Getters for the necessary tree node atributes:
  //   Scope, Type ans IsLValue
  SymTable::ScopeId getScopeDecor    (antlr4::ParserRuleContext *ctx);
  TypesMgr::TypeId  getTypeDecor     (antlr4::ParserRuleContext *ctx);
  bool              getIsLValueDecor (antlr4::ParserRuleContext *ctx);

  // Setters for the necessary tree node attributes:
  //   Scope, Type ans IsLValue
  void putScopeDecor    (antlr4::ParserRuleContext *ctx, SymTable::ScopeId s);
  void putTypeDecor     (antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t);
  void putIsLValueDecor (antlr4::ParserRuleContext *ctx, bool b);

};  // class TypeCheckListener
