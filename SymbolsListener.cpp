#include "SymbolsListener.h"

#include "antlr4-runtime.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/SemErrors.h"

#include <iostream>
#include <string>
#include <vector>

#include <cstddef>    // std::size_t

// uncomment the following line to enable debugging messages with DEBUG*
// #define DEBUG_BUILD
#include "../common/debug.h"

// using namespace std;


// Constructor
SymbolsListener::SymbolsListener(TypesMgr       & Types,
				 SymTable       & Symbols,
				 TreeDecoration & Decorations,
				 SemErrors      & Errors) :
  Types{Types},
  Symbols{Symbols},
  Decorations{Decorations},
  Errors{Errors} {
}

void SymbolsListener::enterProgram(AslParser::ProgramContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = Symbols.pushNewScope("$global$");
  putScopeDecor(ctx, sc);
}
void SymbolsListener::exitProgram(AslParser::ProgramContext *ctx) {
  // Symbols.print();
  Symbols.popScope();
  DEBUG_EXIT();
}

void SymbolsListener::enterFunction(AslParser::FunctionContext *ctx) {
  DEBUG_ENTER();
  std::string funcName = ctx->ID()->getText();
  SymTable::ScopeId sc = Symbols.pushNewScope(funcName);
  putScopeDecor(ctx, sc);
}
void SymbolsListener::exitFunction(AslParser::FunctionContext *ctx) {
  // Symbols.print();
  Symbols.popScope();
  std::string ident = ctx->ID()->getText();
  if (Symbols.findInCurrentScope(ident)) {
    Errors.declaredIdent(ctx->ID());
  }
  else {
  
    TypesMgr::TypeId tRet; // return type
    if (ctx->basic_type() != NULL) {
      if      (ctx->basic_type()->INT())    tRet = Types.createIntegerTy();
      else if (ctx->basic_type()->FLOAT())  tRet = Types.createFloatTy();
      else if (ctx->basic_type()->BOOL())   tRet = Types.createBooleanTy();
      else if (ctx->basic_type()->CHAR())   tRet = Types.createCharacterTy();
    }
    else                                    tRet = Types.createVoidTy();

    std::vector<TypesMgr::TypeId> lParamsTy; // parameter types
    for (auto i : ctx->func_decl_params()->type()) {
      lParamsTy.push_back(getTypeDecor(i));
    } 

    TypesMgr::TypeId tFunc = Types.createFunctionTy(lParamsTy, tRet);
    Symbols.addFunction(ident, tFunc);
  }
  DEBUG_EXIT();
}

void SymbolsListener::enterFunc_decl_params(AslParser::Func_decl_paramsContext *ctx) {

}
void SymbolsListener::exitFunc_decl_params(AslParser::Func_decl_paramsContext *ctx) {
  for (unsigned int i = 0; i < ctx->ID().size(); i++) {
    std::string id = ctx->ID(i)->getText();
    TypesMgr::TypeId t = getTypeDecor(ctx->type(i));
    Symbols.addParameter(id,t);
  }
}

void SymbolsListener::enterDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterArray_decl(AslParser::Array_declContext * ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitArray_decl(AslParser::Array_declContext * ctx) {
  
  // Size of array
  unsigned int size = std::stoi(ctx->INTVAL()->getText());
  // Element type of array
  TypesMgr::TypeId elemType = getTypeDecor(ctx->basic_type());

  TypesMgr::TypeId t = Types.createArrayTy(size,elemType);
  putTypeDecor(ctx, t);
  DEBUG_EXIT();
}

void SymbolsListener::enterVariable_decl(AslParser::Variable_declContext *ctx) {
  DEBUG_ENTER();
}

void SymbolsListener::exitVariable_decl(AslParser::Variable_declContext *ctx) {

  for (auto i : ctx->ID()) {
    std::string ident = i->getText();
    
    // Variable redeclaration
    if (Symbols.findInCurrentScope(ident)) {
      Errors.declaredIdent(i);
    }
    // Add variable to SymTable [id,type]
    else {
      TypesMgr::TypeId t1 = getTypeDecor(ctx->type());
      Symbols.addLocalVar(ident, t1);
    }

  }
  DEBUG_EXIT(); 
}

void SymbolsListener::enterType(AslParser::TypeContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitType(AslParser::TypeContext *ctx) {

  if (ctx->basic_type())
    putTypeDecor(ctx,getTypeDecor(ctx->basic_type()));
  
  else 
    putTypeDecor(ctx,getTypeDecor(ctx->array_decl()));

  DEBUG_EXIT();
}

void SymbolsListener::enterBasic_type(AslParser::Basic_typeContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitBasic_type(AslParser::Basic_typeContext *ctx) {
  
  TypesMgr::TypeId  t = Types.createErrorTy();
  if (ctx->INT())   t = Types.createIntegerTy();
  if (ctx->FLOAT()) t = Types.createFloatTy();
  if (ctx->BOOL())  t = Types.createBooleanTy();
  if (ctx->CHAR())  t = Types.createCharacterTy();

  putTypeDecor(ctx,t);
  DEBUG_EXIT();
}

void SymbolsListener::enterStatements(AslParser::StatementsContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitStatements(AslParser::StatementsContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterAssignStmt(AslParser::AssignStmtContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitAssignStmt(AslParser::AssignStmtContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterIfStmt(AslParser::IfStmtContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitIfStmt(AslParser::IfStmtContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterProcCall(AslParser::ProcCallContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitProcCall(AslParser::ProcCallContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterReadStmt(AslParser::ReadStmtContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitReadStmt(AslParser::ReadStmtContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterWriteExpr(AslParser::WriteExprContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitWriteExpr(AslParser::WriteExprContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterLeft_expr(AslParser::Left_exprContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitLeft_expr(AslParser::Left_exprContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterArithmetic(AslParser::ArithmeticContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitArithmetic(AslParser::ArithmeticContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterRelational(AslParser::RelationalContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitRelational(AslParser::RelationalContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterValue(AslParser::ValueContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitValue(AslParser::ValueContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterLogical(AslParser::LogicalContext *ctx) {

}
void SymbolsListener::exitLogical(AslParser::LogicalContext *ctx) {

}

void SymbolsListener::enterExprIdent(AslParser::ExprIdentContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitExprIdent(AslParser::ExprIdentContext *ctx) {
  DEBUG_EXIT();
}

void SymbolsListener::enterIdent(AslParser::IdentContext *ctx) {
  DEBUG_ENTER();
}
void SymbolsListener::exitIdent(AslParser::IdentContext *ctx) {
  DEBUG_EXIT();
}

// void SymbolsListener::enterEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_ENTER();
// }
// void SymbolsListener::exitEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_EXIT();
// }
// void SymbolsListener::visitTerminal(antlr4::tree::TerminalNode *node) {
//   DEBUG("visitTerminal");
// }
// void SymbolsListener::visitErrorNode(antlr4::tree::ErrorNode *node) {
// }

// Getters for the necessary tree node atributes:
//   Scope and Type
SymTable::ScopeId SymbolsListener::getScopeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getScope(ctx);
}
TypesMgr::TypeId SymbolsListener::getTypeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getType(ctx);
}

// Setters for the necessary tree node attributes:
//   Scope and Type
void SymbolsListener::putScopeDecor(antlr4::ParserRuleContext *ctx, SymTable::ScopeId s) {
  Decorations.putScope(ctx, s);
}
void SymbolsListener::putTypeDecor(antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t) {
  Decorations.putType(ctx, t);
}
