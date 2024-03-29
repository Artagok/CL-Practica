#include "CodeGenListener.h"

#include "antlr4-runtime.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/code.h"

#include <cstddef>    // std::size_t

// uncomment the following line to enable debugging messages with DEBUG*
// #define DEBUG_BUILD
#include "../common/debug.h"

// using namespace std;


// Constructor
CodeGenListener::CodeGenListener(TypesMgr       & Types,
				 SymTable       & Symbols,
				 TreeDecoration & Decorations,
				 code           & Code) :
  Types{Types},
  Symbols{Symbols},
  Decorations{Decorations},
  Code{Code} {
}

void CodeGenListener::enterProgram(AslParser::ProgramContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
}
void CodeGenListener::exitProgram(AslParser::ProgramContext *ctx) {
  Symbols.popScope();
  DEBUG_EXIT();
}

void CodeGenListener::enterFunction(AslParser::FunctionContext *ctx) {
  DEBUG_ENTER();
  subroutine subr(ctx->ID()->getText());
  // Hi ha un primer parametre que sera el resultat a retornar 
  // Nomes si es una funcio, no una accio (retorna alguna cosa)
  if(ctx->basic_type()) subr.add_param("_result");
  Code.add_subroutine(subr);
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
  codeCounters.reset();
}
void CodeGenListener::exitFunction(AslParser::FunctionContext *ctx) {
  subroutine & subrRef = Code.get_last_subroutine();
  instructionList code = getCodeDecor(ctx->statements());
  code = code || instruction::RETURN();
  subrRef.set_instructions(code);
  Symbols.popScope();
  DEBUG_EXIT();
}


void CodeGenListener::enterFunc_decl_params(AslParser::Func_decl_paramsContext * ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitFunc_decl_params(AslParser::Func_decl_paramsContext * ctx) {
  
  subroutine & subRef = Code.get_last_subroutine();
  
  for (auto i : ctx->ID())
    subRef.add_param(i->getText());
  
  DEBUG_EXIT();
}

void CodeGenListener::enterDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitDeclarations(AslParser::DeclarationsContext *ctx) {
  DEBUG_EXIT();
}

void CodeGenListener::enterVariable_decl(AslParser::Variable_declContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitVariable_decl(AslParser::Variable_declContext *ctx) {
  for (auto i : ctx->ID()) {
    subroutine       & subrRef = Code.get_last_subroutine();
    TypesMgr::TypeId        t1 = getTypeDecor(ctx->type());
    std::size_t           size = Types.getSizeOfType(t1);
    subrRef.add_var(i->getText(), size);
  }
  DEBUG_EXIT();
} 

void CodeGenListener::enterType(AslParser::TypeContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitType(AslParser::TypeContext *ctx) {
  DEBUG_EXIT();
}

void CodeGenListener::enterStatements(AslParser::StatementsContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitStatements(AslParser::StatementsContext *ctx) {
  instructionList code;
  for (auto stCtx : ctx->statement()) {
    code = code || getCodeDecor(stCtx);
  }
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterAssignStmt(AslParser::AssignStmtContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitAssignStmt(AslParser::AssignStmtContext *ctx) {
  
  instructionList  code;
  
  // LEFT_EXPR
  std::string     addrLE = getAddrDecor(ctx->left_expr());
  std::string     offsLE = getOffsetDecor(ctx->left_expr());
  instructionList codeLE = getCodeDecor(ctx->left_expr());
  TypesMgr::TypeId tidLE = getTypeDecor(ctx->left_expr());
  
  // EXPR
  std::string     addrE = getAddrDecor(ctx->expr());
  // std::string     offsE = getOffsetDecor(ctx->expr());
  instructionList codeE = getCodeDecor(ctx->expr());
  TypesMgr::TypeId tidE = getTypeDecor(ctx->expr());

  // ARRAY to ARRAY a,b:array and a = b
  if (Types.isArrayTy(tidLE) and Types.isArrayTy(tidE)) {

    bool isLocalLE = Symbols.isLocalVarClass(addrLE);
    bool isLocalE  = Symbols.isLocalVarClass(addrE);

    std::string tempAddrLE = "%"+codeCounters.newTEMP();
    std::string tempAddrE  = "%"+codeCounters.newTEMP();

    if (not isLocalLE) code = code || instruction::LOAD(tempAddrLE, addrLE);
    if (not isLocalE)  code = code || instruction::LOAD(tempAddrE, addrE);

    std::string tempIndex  = "%"+codeCounters.newTEMP();
    std::string tempIncrem = "%"+codeCounters.newTEMP();
    std::string tempSize   = "%"+codeCounters.newTEMP();
    std::string tempOffset = "%"+codeCounters.newTEMP();
    std::string tempOffHld = "%"+codeCounters.newTEMP();
    std::string tempCompar = "%"+codeCounters.newTEMP();
    std::string tempValue  = "%"+codeCounters.newTEMP();

    std::string labelWhile = "while"+codeCounters.newLabelWHILE();
    std::string labelEndWhile = "end"+labelWhile;

    code = code || instruction::ILOAD(tempIndex, "0");
    code = code || instruction::ILOAD(tempIncrem, "1");
    code = code || instruction::ILOAD(tempSize, std::to_string(Types.getArraySize(Symbols.getType(addrLE))));
    code = code || instruction::ILOAD(tempOffset, "1");

    code = code || instruction::LABEL(labelWhile);
    code = code || instruction::LT(tempCompar, tempIndex, tempSize);
    code = code || instruction::FJUMP(tempCompar, labelEndWhile);
    code = code || instruction::MUL(tempOffHld, tempOffset, tempIndex);
    code = code || instruction::LOADX(tempValue, isLocalE ? addrE : tempAddrE, tempOffHld);
    code = code || instruction::XLOAD(isLocalLE ? addrLE : tempAddrLE, tempOffHld, tempValue);
    code = code || instruction::ADD(tempIndex, tempIndex, tempIncrem);
    code = code || instruction::UJUMP(labelWhile);
    code = code || instruction::LABEL(labelEndWhile);
  }
  
  // int2float CAST for array or non array
  if (Types.isFloatTy(tidLE) and Types.isIntegerTy(tidE)) {
    
    std::string tempF = "%"+codeCounters.newTEMP();
    code = code || instruction::FLOAT(tempF, addrE);
    putAddrDecor(ctx->expr(), tempF);
    addrE = getAddrDecor(ctx->expr());
  }

  // ARRAY ASSIGNEMENT
  if (ctx->left_expr()->expr())
    code = code || instruction::XLOAD(addrLE, offsLE, addrE);
  
  // NOT AN ARRAY ASSIGNEMENT
  else 
    code = code || instruction::LOAD(addrLE, addrE);
  
  code = codeLE || codeE || code;
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterIfStmt(AslParser::IfStmtContext *ctx) {
  DEBUG_ENTER();
}

void CodeGenListener::exitIfStmt(AslParser::IfStmtContext *ctx) {
  // IF expr THEN statements(0) [ELSE statements(1)] ENDIF
  instructionList  code;
  std::string      addrExpr = getAddrDecor(ctx->expr());        // addr if(expr)
  instructionList  codeExpr = getCodeDecor(ctx->expr());        // code if(expr)
  instructionList  codeS0   = getCodeDecor(ctx->statements(0)); // code statements(0)

  std::string labelIf    = codeCounters.newLabelIF();   //  1
  std::string labelEndIf = "endif"+labelIf;             // endif1

  // IF ELSE
  if (ctx->ELSE()) {

    instructionList codeS1    = getCodeDecor(ctx->statements(1)); // code statements(1)
    std::string     labelElse = "else"+labelIf;   // else1

    code =  codeExpr || instruction::FJUMP(addrExpr, labelElse) ||
            codeS0 || instruction::UJUMP(labelEndIf) || instruction::LABEL(labelElse) ||
            codeS1 || instruction::LABEL(labelEndIf);
  }
  // IF
  else {

    code =  codeExpr || instruction::FJUMP(addrExpr, labelEndIf) ||
            codeS0 || instruction::LABEL(labelEndIf);
  }

  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterWhileStmt(AslParser::WhileStmtContext * ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitWhileStmt(AslParser::WhileStmtContext * ctx) {
  
  instructionList code;
  std::string     addrE = getAddrDecor(ctx->expr());
  instructionList codeE = getCodeDecor(ctx->expr());
  instructionList codeS = getCodeDecor(ctx->statements());

  std::string     labelWhile    = "while"+codeCounters.newLabelWHILE();
  std::string     labelEndWhile = "end"+labelWhile;

  code =  instruction::LABEL(labelWhile) || codeE || instruction::FJUMP(addrE, labelEndWhile) ||
          codeS || instruction::UJUMP(labelWhile) || instruction::LABEL(labelEndWhile);
  
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterProcCall(AslParser::ProcCallContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitProcCall(AslParser::ProcCallContext *ctx) {
  
  // Action call, so there is no RETURN whatsoever
  instructionList code;
  auto param_types = Types.getFuncParamsTypes(getTypeDecor(ctx->ident()));

  int k = 0;
  for (auto i : ctx->expr()) {
   
    code = code || getCodeDecor(i);

    // int 2 float CAST
    if (Types.isFloatTy(param_types[k]) and Types.isIntegerTy(getTypeDecor(i))) {
      
      std::string tempF = "%"+codeCounters.newTEMP();
      code = code || instruction::FLOAT(tempF, getAddrDecor(i));
      putAddrDecor(i, tempF);
    }
    // passing an ARRAY by REFERENCE
    else if (Types.isArrayTy(getTypeDecor(i))) {

      std::string tempA = "%"+codeCounters.newTEMP();
      code = code || instruction::ALOAD(tempA, getAddrDecor(i));
      putAddrDecor(i, tempA);
    }
    ++k;
  }

  code = code || instruction::PUSH();
  for (auto i : ctx->expr()) {
    code = code || instruction::PUSH(getAddrDecor(i));
  }
  code = code || instruction::CALL(getAddrDecor(ctx->ident()));

  // Traditional for instead of auto ranged based to avoid compiler warning
  for (uint i = 0; i < (ctx->expr()).size(); ++i)
    code = code || instruction::POP();

  code = code || instruction::POP();
  //putAddrDecor(ctx, temp);
  //putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);

  DEBUG_EXIT();
}


void CodeGenListener::enterReturnStmt(AslParser::ReturnStmtContext * ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitReturnStmt(AslParser::ReturnStmtContext * ctx) {

  std::string temp;
  instructionList code;

  if (ctx->expr()) {
    
    code                  = getCodeDecor(ctx->expr());
    subroutine & subRef   = Code.get_last_subroutine();
    temp                  = (subRef.params.begin())->name;
    std::string addrE     = getAddrDecor(ctx->expr());
    code = code || instruction::LOAD(temp, addrE) || instruction::RETURN(); 
  }
  
  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);

  DEBUG_EXIT();
}

void CodeGenListener::enterReadStmt(AslParser::ReadStmtContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitReadStmt(AslParser::ReadStmtContext *ctx) {
  
  instructionList  code;
  std::string     addrLE = getAddrDecor(ctx->left_expr());
  std::string     offsLE = getOffsetDecor(ctx->left_expr());
  instructionList codeLE = getCodeDecor(ctx->left_expr());
  TypesMgr::TypeId tLE   = getTypeDecor(ctx->left_expr());

  // read into an ARRAY (i.e. read x[3])
  if (ctx->left_expr()->expr()) {

    std::string tempR = "%"+codeCounters.newTEMP();

    if (Types.isIntegerTy(tLE) or Types.isBooleanTy(tLE)) code = code || instruction::READI(tempR);
    else if (Types.isFloatTy(tLE))                        code = code || instruction::READF(tempR);
    else /* isCharacter(tLE) */                           code = code || instruction::READC(tempR);
                                                          code = code || instruction::XLOAD(addrLE, offsLE, tempR);
  }
  // read into a VARIABLE (i.e. read x)
  else {
    
    if (Types.isIntegerTy(tLE) or Types.isBooleanTy(tLE)) code = code || instruction::READI(addrLE);
    else if (Types.isFloatTy(tLE))                        code = code || instruction::READF(addrLE);
    else /* isCharacter(tLE) */                           code = code || instruction::READC(addrLE);
  }

  // Add LeftExpression code before everything
  code = codeLE || code;

  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterWriteExpr(AslParser::WriteExprContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitWriteExpr(AslParser::WriteExprContext *ctx) {
  
  instructionList code;
  std::string     addrE = getAddrDecor(ctx->expr());
  // std::string     offs1 = getOffsetDecor(ctx->expr());
  instructionList codeE = getCodeDecor(ctx->expr());

  TypesMgr::TypeId t = getTypeDecor(ctx->expr());

  if (Types.isIntegerTy(t) or Types.isBooleanTy(t)) code = codeE || instruction::WRITEI(addrE);
  else if (Types.isFloatTy(t))                      code = codeE || instruction::WRITEF(addrE);
  else /* isCharacterTy(t) */                       code = codeE || instruction::WRITEC(addrE);

  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterWriteString(AslParser::WriteStringContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitWriteString(AslParser::WriteStringContext *ctx) {
  instructionList code;
  std::string s = ctx->STRING()->getText();
  std::string temp = "%"+codeCounters.newTEMP();
  int i = 1;
  while (i < int(s.size())-1) {
    if (s[i] != '\\') {
      code = code ||
	     instruction::CHLOAD(temp, s.substr(i,1)) ||
	     instruction::WRITEC(temp);
      i += 1;
    }
    else {
      assert(i < int(s.size())-2);
      if (s[i+1] == 'n') {
        code = code || instruction::WRITELN();
        i += 2;
      }
      else if (s[i+1] == 't' or s[i+1] == '"' or s[i+1] == '\\') {
        code = code ||
               instruction::CHLOAD(temp, s.substr(i,2)) ||
	       instruction::WRITEC(temp);
        i += 2;
      }
      else {
        code = code ||
               instruction::CHLOAD(temp, s.substr(i,1)) ||
	       instruction::WRITEC(temp);
        i += 1;
      }
    }
  }
  putCodeDecor(ctx, code);
  DEBUG_EXIT();
}

void CodeGenListener::enterLeft_expr(AslParser::Left_exprContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitLeft_expr(AslParser::Left_exprContext *ctx) {
  
  instructionList code = getCodeDecor(ctx->ident());
  std::string offset = "";
  std::string address = getAddrDecor(ctx->ident());
  
    
  // CAS ARRAY
  if (ctx->expr()) {

    std::string tempOff = "%"+codeCounters.newTEMP();
    offset = getAddrDecor(ctx->expr());

    // Array LOCAL
    if (Symbols.isLocalVarClass(address)) {
      
      code = code || getCodeDecor(ctx->expr());
      code = code || instruction::LOAD(tempOff, "1");
      code = code || instruction::MUL(tempOff, offset, tempOff);

      putAddrDecor(ctx, address);
    }
    // Array PARAM per REFERENCIA
    else {

      code = code || getCodeDecor(ctx->expr());
      std::string tempA = "%"+codeCounters.newTEMP();
      code = code || instruction::LOAD(tempA, address);
      code = code || instruction::LOAD(tempOff, "1");
      code = code || instruction::MUL(tempOff, offset, tempOff);

      putAddrDecor(ctx, tempA);
    }

    putOffsetDecor(ctx, tempOff);
    putCodeDecor(ctx, code);
  }

  // CAS IDENT
  else {
  
    putAddrDecor(ctx, address);
    putOffsetDecor(ctx, offset);
    putCodeDecor(ctx, code); 
  }

  DEBUG_EXIT();
}


void CodeGenListener::enterUnary(AslParser::UnaryContext * ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitUnary(AslParser::UnaryContext * ctx) {
  
  std::string     addrE = getAddrDecor(ctx->expr());
  instructionList codeE = getCodeDecor(ctx->expr());
  instructionList code  = codeE;

  TypesMgr::TypeId t  = getTypeDecor(ctx->expr());
  std::string temp    = "%"+codeCounters.newTEMP();

  if (ctx->NOT())       code = code || instruction::NOT(temp, addrE);
  else if (ctx->SUB())  code = code || (Types.isFloatTy(t) ? instruction::FNEG(temp, addrE) :
                                                             instruction::NEG(temp, addrE)) ;

  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);
  
  DEBUG_EXIT();
}

void CodeGenListener::enterArithmetic(AslParser::ArithmeticContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitArithmetic(AslParser::ArithmeticContext *ctx) {
  // addr and code of expr(0)
  std::string     addrE0 = getAddrDecor(ctx->expr(0));
  instructionList codeE0 = getCodeDecor(ctx->expr(0));
  // addr and code of expr(1)
  std::string     addrE1 = getAddrDecor(ctx->expr(1));
  instructionList codeE1 = getCodeDecor(ctx->expr(1));
  // eval(expr(0)) eval(expr(1))
  instructionList code   = codeE0 || codeE1;
  
  TypesMgr::TypeId t0 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(1));
  TypesMgr::TypeId t  = getTypeDecor(ctx);

  // stores the temporal FLOAT cast if isFloat(t0) XOR isFloat(t1)
  bool floatXor     = Types.isFloatTy(t0) != Types.isFloatTy(t1);
  std::string tempF = floatXor ? "%"+codeCounters.newTEMP() : "";
  std::string temp  = "%"+codeCounters.newTEMP();

  // INTEGER
  if (Types.isIntegerTy(t)) {
    
    if (ctx->MUL())       code = code || instruction::MUL(temp, addrE0, addrE1);
    else if (ctx->ADD())  code = code || instruction::ADD(temp, addrE0, addrE1);
    else if (ctx->DIV())  code = code || instruction::DIV(temp, addrE0, addrE1);
    else if (ctx->SUB())  code = code || instruction::SUB(temp, addrE0, addrE1);
    else /* ctx->MOD() */ code = code || instruction::DIV(temp, addrE0, addrE1) 
                                      || instruction::MUL(temp, temp, addrE1) 
                                      || instruction::SUB(temp, addrE0, temp);
  }

  // FLOAT [MOD not possible with FLOAT]
  else {

    instruction cast = Types.isIntegerTy(t0) ? instruction::FLOAT(tempF, addrE0) :
                                               instruction::FLOAT(tempF, addrE1) ;
    // One FLOAT but NOT both
    if (floatXor) {

      if (ctx->MUL())       code = code || cast || instruction::FMUL(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                           Types.isIntegerTy(t1) ? tempF : addrE1);
      else if (ctx->ADD())  code = code || cast || instruction::FADD(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                           Types.isIntegerTy(t1) ? tempF : addrE1);
      else if (ctx->DIV())  code = code || cast || instruction::FDIV(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                           Types.isIntegerTy(t1) ? tempF : addrE1);
      else /*ctx->SUB()*/   code = code || cast || instruction::FSUB(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                           Types.isIntegerTy(t1) ? tempF : addrE1);
    }
    // BOTH FLOAT
    else {

      if (ctx->MUL())       code = code || instruction::FMUL(temp, addrE0, addrE1);
      else if (ctx->ADD())  code = code || instruction::FADD(temp, addrE0, addrE1);
      else if (ctx->DIV())  code = code || instruction::FDIV(temp, addrE0, addrE1);
      else /*ctx->SUB()*/   code = code || instruction::FSUB(temp, addrE0, addrE1);
    }
  }

  putAddrDecor(ctx, temp); // temp addr
  putOffsetDecor(ctx, ""); // no offset
  putCodeDecor(ctx, code); // temp code

  DEBUG_EXIT();
}

void CodeGenListener::enterRelational(AslParser::RelationalContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitRelational(AslParser::RelationalContext *ctx) {
  
  std::string     addrE0 = getAddrDecor(ctx->expr(0));
  instructionList codeE0 = getCodeDecor(ctx->expr(0));
  std::string     addrE1 = getAddrDecor(ctx->expr(1));
  instructionList codeE1 = getCodeDecor(ctx->expr(1));
  instructionList code  = codeE0 || codeE1;

  TypesMgr::TypeId t0 = getTypeDecor(ctx->expr(0));
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(1));
  //TypesMgr::TypeId t  = getTypeDecor(ctx);

  // stores the temporal FLOAT cast if isFloat(t0) XOR isFloat(t1)
  bool floatXor     = Types.isFloatTy(t0) != Types.isFloatTy(t1);
  std::string tempF = floatXor ? "%"+codeCounters.newTEMP() : "";
  std::string temp  = "%"+codeCounters.newTEMP();

  // INT or CHAR or BOOL
  if (not Types.isFloatTy(t0) and not Types.isFloatTy(t1)) {

    if (ctx->EQ())        code = code || instruction::EQ(temp, addrE0, addrE1);
    else if (ctx->NEQ())  code = code || instruction::EQ(temp, addrE0, addrE1) || instruction::NOT(temp, temp);
    else if (ctx->LT())   code = code || instruction::LT(temp, addrE0, addrE1);
    else if (ctx->LTE())  code = code || instruction::LE(temp, addrE0, addrE1);
    else if (ctx->GT())   code = code || instruction::LE(temp, addrE0, addrE1) || instruction::NOT(temp, temp);
    else /*ctx->GTE()*/   code = code || instruction::LT(temp, addrE0, addrE1) || instruction::NOT(temp, temp);
  }

  // FLOAT
  else {

    instruction cast = Types.isIntegerTy(t0) ? instruction::FLOAT(tempF, addrE0) :
                                               instruction::FLOAT(tempF, addrE1) ;
    // One FLOAT but NOT both
    if (floatXor) {

      if (ctx->EQ())        code = code || cast || instruction::FEQ(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                          Types.isIntegerTy(t1) ? tempF : addrE1);
      else if (ctx->NEQ())  code = code || cast || instruction::FEQ(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                          Types.isIntegerTy(t1) ? tempF : addrE1) || instruction::NOT(temp, temp);
      else if (ctx->LT())   code = code || cast || instruction::FLT(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                          Types.isIntegerTy(t1) ? tempF : addrE1);
      else if (ctx->LTE())  code = code || cast || instruction::FLE(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                          Types.isIntegerTy(t1) ? tempF : addrE1);
      else if (ctx->GT())   code = code || cast || instruction::FLE(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                          Types.isIntegerTy(t1) ? tempF : addrE1) || instruction::NOT(temp, temp);
      else /*ctx->GTE()*/   code = code || cast || instruction::FLT(temp, Types.isIntegerTy(t0) ? tempF : addrE0,
                                                                          Types.isIntegerTy(t1) ? tempF : addrE1) || instruction::NOT(temp, temp);
    }
    // BOTH FLOAT
    else {

      if (ctx->EQ())        code = code || instruction::FEQ(temp, addrE0, addrE1);
      else if (ctx->NEQ())  code = code || instruction::FEQ(temp, addrE0, addrE1) || instruction::NOT(temp, temp);
      else if (ctx->LT())   code = code || instruction::FLT(temp, addrE0, addrE1);
      else if (ctx->LTE())  code = code || instruction::FLE(temp, addrE0, addrE1);
      else if (ctx->GT())   code = code || instruction::FLE(temp, addrE0, addrE1) || instruction::NOT(temp, temp);
      else /*ctx->GTE()*/   code = code || instruction::FLT(temp, addrE0, addrE1) || instruction::NOT(temp, temp);
    }
  }

  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);

  DEBUG_EXIT();
}

void CodeGenListener::enterValue(AslParser::ValueContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitValue(AslParser::ValueContext *ctx) {
  
  instructionList code;
  std::string temp = "%"+codeCounters.newTEMP();

  if (ctx->INTVAL())        code = instruction::ILOAD(temp, ctx->getText());
  else if (ctx->FLOATVAL()) code = instruction::FLOAD(temp, ctx->getText());
  else if (ctx->CHARVAL()) {std::string s = ctx->getText();
                            code = instruction::CHLOAD(temp, s.substr(1,s.size()-2));}
  else /* ctx->BOOLVAL() */ code = instruction::LOAD(temp, (ctx->getText()=="true" ? "1":"0"));

  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);

  DEBUG_EXIT();
}

void CodeGenListener::enterParenthesis(AslParser::ParenthesisContext * ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitParenthesis(AslParser::ParenthesisContext * ctx) {
  putAddrDecor(ctx, getAddrDecor(ctx->expr()));
  putOffsetDecor(ctx, getOffsetDecor(ctx->expr()));
  putCodeDecor(ctx, getCodeDecor(ctx->expr()));
  DEBUG_EXIT();
}

void CodeGenListener::enterLogical(AslParser::LogicalContext * ctx) {
    DEBUG_ENTER();
}
void CodeGenListener::exitLogical(AslParser::LogicalContext * ctx) {
  
  std::string     addrE0 = getAddrDecor(ctx->expr(0));
  instructionList codeE0 = getCodeDecor(ctx->expr(0));
  std::string     addrE1 = getAddrDecor(ctx->expr(1));
  instructionList codeE1 = getCodeDecor(ctx->expr(1));
  instructionList code  = codeE0 || codeE1;

  std::string temp = "%"+codeCounters.newTEMP();

  if (ctx->AND())     code = code || instruction::AND(temp, addrE0, addrE1);
  else /*ctx->OR()*/  code = code || instruction::OR(temp, addrE0, addrE1);

  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);

  DEBUG_EXIT();
}

void CodeGenListener::enterExprIdent(AslParser::ExprIdentContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitExprIdent(AslParser::ExprIdentContext *ctx) {
  putAddrDecor(ctx, getAddrDecor(ctx->ident()));
  putOffsetDecor(ctx, getOffsetDecor(ctx->ident()));
  putCodeDecor(ctx, getCodeDecor(ctx->ident()));
  DEBUG_EXIT();
}

void CodeGenListener::enterArrayIndex(AslParser::ArrayIndexContext * ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitArrayIndex(AslParser::ArrayIndexContext * ctx) {

  instructionList code    = getCodeDecor(ctx->expr());
  std::string     addrI   = getAddrDecor(ctx->ident());
  std::string     offset  = getAddrDecor(ctx->expr());
  
  std::string     temp    = "%"+codeCounters.newTEMP();
  std::string     tempOff = "%"+codeCounters.newTEMP();

  code = code || instruction::LOAD(tempOff, "1");
  code = code || instruction::MUL(tempOff, offset, tempOff);

  // LOCAL
  if (Symbols.isLocalVarClass(addrI)) {
    
    code = code || instruction::LOADX(temp, addrI, tempOff);
  }
  // PARAM REF
  else {

    std::string tempA = "%"+codeCounters.newTEMP();
    code = code || instruction::LOAD(tempA, addrI);
    code = code || instruction::LOADX(temp, tempA, tempOff);
  }

  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);

  DEBUG_EXIT();
}

void CodeGenListener::enterFuncCall(AslParser::FuncCallContext * ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitFuncCall(AslParser::FuncCallContext * ctx) {
  
  //std::string     addrE;
  //instructionList codeE;
  instructionList code;
  auto param_types = Types.getFuncParamsTypes(getTypeDecor(ctx->ident()));

  int k = 0;
  for (auto i : ctx->expr()) {
   
    code = code || getCodeDecor(i);
    
    // int 2 float CAST
    if (Types.isFloatTy(param_types[k]) and Types.isIntegerTy(getTypeDecor(i))) {

      std::string tempF = "%"+codeCounters.newTEMP();
      code = code || instruction::FLOAT(tempF,getAddrDecor(i));
      putAddrDecor(i, tempF);
    }
    // passing an ARRAY by REFERENCE
    else if (Types.isArrayTy(getTypeDecor(i))) {

      std::string tempA = "%"+codeCounters.newTEMP();
      code = code || instruction::ALOAD(tempA, getAddrDecor(i));
      putAddrDecor(i, tempA);
    }
    ++k;
  }

  // Empty push for the return variable
  code = code || instruction::PUSH();
  
  for (auto i : ctx->expr()) {
    code = code || instruction::PUSH(getAddrDecor(i));
  }

  code = code || instruction::CALL(getAddrDecor(ctx->ident()));

  // Traditional for instead of auto ranged based to avoid compiler warning
  for (uint i = 0; i < (ctx->expr()).size(); ++i)
    code = code || instruction::POP();

  std::string temp = "%"+codeCounters.newTEMP();
  code = code || instruction::POP(temp);

  putAddrDecor(ctx, temp);
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, code);

  DEBUG_EXIT();
}

void CodeGenListener::enterIdent(AslParser::IdentContext *ctx) {
  DEBUG_ENTER();
}
void CodeGenListener::exitIdent(AslParser::IdentContext *ctx) {
  putAddrDecor(ctx, ctx->ID()->getText());
  putOffsetDecor(ctx, "");
  putCodeDecor(ctx, instructionList());
  DEBUG_EXIT();
}

// void CodeGenListener::enterEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_ENTER();
// }
// void CodeGenListener::exitEveryRule(antlr4::ParserRuleContext *ctx) {
//   DEBUG_EXIT();
// }
// void CodeGenListener::visitTerminal(antlr4::tree::TerminalNode *node) {
//   DEBUG(">>> visit " << node->getSymbol()->getLine() << ":" << node->getSymbol()->getCharPositionInLine() << " CodeGen TerminalNode");
// }
// void CodeGenListener::visitErrorNode(antlr4::tree::ErrorNode *node) {
// }


// Getters for the necessary tree node atributes:
//   Scope, Type, Addr, Offset and Code
SymTable::ScopeId CodeGenListener::getScopeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getScope(ctx);
}
TypesMgr::TypeId CodeGenListener::getTypeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getType(ctx);
}
std::string CodeGenListener::getAddrDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getAddr(ctx);
}
std::string  CodeGenListener::getOffsetDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getOffset(ctx);
}
instructionList CodeGenListener::getCodeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getCode(ctx);
}

// Setters for the necessary tree node attributes:
//   Addr, Offset and Code
void CodeGenListener::putAddrDecor(antlr4::ParserRuleContext *ctx, const std::string & a) {
  Decorations.putAddr(ctx, a);
}
void CodeGenListener::putOffsetDecor(antlr4::ParserRuleContext *ctx, const std::string & o) {
  Decorations.putOffset(ctx, o);
}
void CodeGenListener::putCodeDecor(antlr4::ParserRuleContext *ctx, const instructionList & c) {
  Decorations.putCode(ctx, c);
}
