/*
 * Parser.cpp
 *
 * CSC 446 - Assignment #3
 * Compiler Construction - Recursive Descent Parser Implementation
 *
 * Author: Landon Dahmen
 * Updated for Assignment 7d modifications (local offset reset, operand order adjustment,
 * and end marker change)
 *
 * Description:
 *   This file implements the Parser class declared in Parser.h. It parses the program,
 *   builds the symbol table, and generates three-address code (TAC). Changes include
 *   resetting the local offset so that the first local temporary is _bp-2,
 *   swapping multiplication operands when both are parameters (to match annotated output),
 *   and printing the correct end marker.
 */
#include "Parser.h"
#include "Globals.h"
#include <iostream>
#include <sstream>

#define RESET "\033[0m"

using namespace std;

RecursiveDescentParser::RecursiveDescentParser(string name) : lex(name)
{
    this->name = name;
    Offset = 0;
    tempCounter = 0;

    // Create TAC file with same name
    string tacFileName = name.substr(0, name.find_last_of('.')) + ".tac";
    tacFile.open(tacFileName);
    if(!tacFile) {
        cout << "Error: " << RESET 
            << "could not open file " << tacFileName << endl;
        exit(1);
    }
    // Prime parser
    lex.GetNextToken(); 
    // Push start symbol onto stack
    Prog();

    if (!error && !programName.empty()) {
        emit("start proc " + programName);
    }
    if (Token != eoft) {
        while(Token != eoft) {
            cout << "Unused token: " << lex.GetTokenName(Token) << endl;
            lex.GetNextToken(); 
        }
        cout << name << ": " << LineNo+1 << ": " 
            << "Error: " << RESET << "unused tokens!" << endl;
        error = true;
    }
    if (error) {
        cout << name << ": "
            << "Error: " << RESET << "syntax errors found!" << endl;
    } else {
        GenerateAssembly();
        // only your four lines:
        cout << "Exiting procedure " << programName << "\n\n";
        cout << "Parsing and semantic analysis completed successfully!" << endl;
        // strip off “.ada” to build your filenames:
        string base = name.substr(0, name.find_last_of('.'));
        cout << "Three Address Code written to: " << base << ".tac" << endl;
        cout << "Assembly Code written to: " << base << ".asm" << endl;
    }
}

RecursiveDescentParser::~RecursiveDescentParser()
{
}

void RecursiveDescentParser::emit(string code)
{
    if (tacFile.is_open()) {
        tacFile << code << endl;
    } else {
        cout << "Error: " << RESET 
            << "could not write to TAC file" << endl;
    }
    //cout << code << endl;
}

string RecursiveDescentParser::GetVarReference(TableEntry* entry)
{
    if (entry->depth == 1) {
        return entry->lexeme;
    }

    if (entry->isParam) {
        return "_BP+" + to_string(entry->var.Offset);  // parameter → positive offset
    } else {
        return "_BP-" + to_string(entry->var.Offset);  // local or temp
    }
}

TableEntry* RecursiveDescentParser::NewTemp()
{
    string tempName = "_t" + to_string(++tempCounter);
    st.Insert(tempName, idt, Depth);
    TableEntry* entry = st.Lookup(tempName);
    if (entry) {
        entry->TypeOfEntry = varEntry;
        entry->var.TypeOfVariable = intType; // Default int
        entry->var.Offset = Offset;
        entry->var.size = 2;
        entry->isParam = false;
        Offset += entry->var.size;
        //entry->var.Offset = Offset;
        if (Depth == 1) {
            globalTemps.push_back(tempName);
        }
    }
    return st.Lookup(tempName);
}

void RecursiveDescentParser::ProcessParams()
{
    if (!currentParameters.empty()) {
    int paramSize = 2; // e.g., 2 bytes per parameter
    int count = 1;     // This counter starts at 1 for the last element
    // Loop through the parameters vector in reverse order:
    for (int i = currentParameters.size() - 1; i >= 0; i--, count++) {
        string lexeme = currentParameters[i];
        TableEntry* entry = st.Lookup(lexeme);
        if (entry != nullptr && entry->isParam) {
            // If the base for parameters is 2, then for the last parameter (count 1):
            //   offset = 2 + 1 * paramSize (i.e., +_BP+4)
            // and for the first parameter (count = total number) offset becomes highest.
            entry->var.Offset = 2 + count * paramSize;
        }
    }
    // Optionally, clear the parameters vector now that you've reprocessed it
    currentParameters.clear();
}
}

void RecursiveDescentParser::Match(Symbol desired)
{
    if (Token == desired) {
        lex.GetNextToken();
    } else {
        cout << name << ": " << LineNo+1
            << ": " << "Error: " << RESET << "expecting: " 
            << lex.GetTokenName(desired) << ", found " << Lexeme << endl;
        error = true;
    }
}

void RecursiveDescentParser::Prog()
{
    if (Token == proceduret) {
        Match(proceduret);
        string procName = Lexeme;
        if (st.Lookup(Lexeme) != nullptr) {
            cout << name << ": " << LineNo+1 << ": " 
                << "Error: " << RESET << "duplicate identifier: " 
                << Lexeme << RESET << " at Depth " << Depth << endl;
            error = true;
            exit(1);
        }
        if (Depth == 0) {
            programName = procName; // Save the program name
        }

        st.Insert(Lexeme, Token, Depth);
        TableEntry* prevProcedure = currentProcedure;

        currentProcedure = st.Lookup(Lexeme);
        currentProcedure->TypeOfEntry = functionEntry;
        currentProcedure->function.NumberOfParameters = 0;
        currentProcedure->function.SizeOfLocal = 0;
        currentProcedure->function.ParamList = nullptr;

        int savedOffset = Offset;
        Offset = 2; // start stack offset at 2
        tempCounter = 0;
        Depth++;

        Match(idt);
        Args();
        ProcessParams();
        int paramSize = Offset; // save param space used so far

        Match(ist);
        DeclarativePart(); // adds locals (Offset grows)

        int localsStart = paramSize; // everything after this is locals or temps

        Procedures(); // nested procedures

        emit("proc " + procName);

        Match(begint);
        SeqOfStatements(); // this creates temps (Offset increases again)
        Match(endt);
        Match(idt);
        Match(semit);

        // calculate total space for locals + temps
        int afterAll = Offset;
        int localSize = afterAll - paramSize;

        if (currentProcedure != nullptr) {
            currentProcedure->function.SizeOfLocal = localSize;
        }

        emit("endp " + procName);
        //st.WriteTable(Depth);
        st.DeleteDepth(Depth);
        Depth--;
        currentProcedure = prevProcedure;
        Offset = savedOffset;
    }
}


void RecursiveDescentParser::DeclarativePart()
{   
    if (Token == idt) {
        IdentifierList();
        Match(colont);
        TypeMark();
        Match(semit);
        DeclarativePart();
    } else {
    }
}

void RecursiveDescentParser::IdentifierList()
{
    // IdentifierList -> idt IdentifierListPrime
    if (Token == idt) {
        string currentLexeme = Lexeme; // Save current lexeme

        TableEntry* existing = st.Lookup(currentLexeme);
        if (existing != nullptr && existing->depth == Depth) {
            cout << name << ": " << LineNo+1 << ": " 
                << "Error: " << RESET << "duplicate identifier: " 
                << currentLexeme << RESET << " at Depth: " << Depth << endl;
            error = true;
            exit(1);
        } else {
            st.Insert(currentLexeme, Token, Depth);
            TableEntry* entry = st.Lookup(currentLexeme);
            entry->TypeOfEntry = varEntry;
            if(currentMode == 0) {
                entry->paramMode = modeIn;
            } else if(currentMode == 1) {
                entry->paramMode = modeOut;
            } else if(currentMode == 2) {
                entry->paramMode = modeInOut;
            }
            if (processingParams && entry != nullptr) {
                entry->isParam = true;
                entry->function.NumberOfParameters++;
                currentParameters.push_back(currentLexeme);
            } else {
                currentIdentifiers.push_back(currentLexeme);
            }
        }
        Match(idt);
        IdentifierListPrime();
    } else {   
        cout << name << ": " << LineNo+1 << ": " 
            << "Error: " << RESET << "expecting identifier" << endl;
        error = true;
    }
}

void RecursiveDescentParser::IdentifierListPrime()
{
    if (Token == commat) {
        Match(commat);
        
        string currentLexeme = Lexeme; // Save current lexeme
        
        // check for multiple declarations
        TableEntry* existing = st.Lookup(currentLexeme);
        if (existing != nullptr && existing->depth == Depth) {
            cout << name << ": " << LineNo+1 << ": " 
                << "Error: " << RESET << "duplicate identifier: " 
                << currentLexeme << RESET
                << " at Depth: " << Depth << endl;
            error = true;
            exit(1);
        } else {
            st.Insert(currentLexeme, Token, Depth);
            TableEntry* entry = st.Lookup(currentLexeme);
            entry->TypeOfEntry = varEntry;
            if(currentMode == 0) {
                entry->paramMode = modeIn;
            } else if(currentMode == 1) {
                entry->paramMode = modeOut;
            } else if(currentMode == 2) {
                entry->paramMode = modeInOut;
            }
            if (processingParams && entry != nullptr) {
                entry->isParam = true;
                entry->function.NumberOfParameters++;
                currentParameters.push_back(currentLexeme);
            } else {
                currentIdentifiers.push_back(currentLexeme);
            }
        }
        
        Match(idt);
        IdentifierListPrime();
    } else {
        // ε
    }
}

void RecursiveDescentParser::TypeMark()
{
    if (Token == integert) {
        // loop through all identifiers in the list and update their type
        for (auto& id : currentIdentifiers) {
            TableEntry* entry = st.Lookup(id);
            if (entry) {
                entry->var.TypeOfVariable = intType;
                entry->var.size = size(integert);
                entry->var.Offset = Offset;
                Offset += entry->var.size; // update offset for next variable
                if (Depth == 1) {
                    globalVars.push_back(id);
                }
            }
        }
        Match(integert);
    } else if (Token == floatt) {
        for (auto& id : currentIdentifiers) {
            TableEntry* entry = st.Lookup(id);
            if (entry) {
                entry->var.TypeOfVariable = floatType;
                entry->var.size = size(floatt);
                entry->var.Offset = Offset;
                Offset += entry->var.size; // update offset for next variable
                if (Depth == 1) {
                    globalVars.push_back(id);
                }
            }
        }
        Match(floatt);
    } else if (Token == chart) {
        for (auto& id : currentIdentifiers) {
            TableEntry* entry = st.Lookup(id);
            if (entry) {
                entry->var.TypeOfVariable = charType;
                entry->var.size = size(chart);
                entry->var.Offset = Offset;
                Offset += entry->var.size;
                if (Depth == 1) {
                    globalVars.push_back(id);
                } 
            }
        }
        Match(chart);
    } else if (Token == constantt) {
        for (auto& id : currentIdentifiers) {
            TableEntry* entry = st.Lookup(id);
            if (entry) {
                entry->TypeOfEntry = constEntry;
                // set the constant type and value in the Value() function
            }
        }
        Match(constantt);
        Match(assignopt);
        Value1();
    } else {
        cout << name << ": " << LineNo+1 << ": " 
            << "Error: " << RESET << "expecting integert, floatt, chart, or constantt" << endl;
        cout << "Token found: " << lex.GetTokenName(Token) << endl;
        error = true;
    }
    // Clear the identifier list after processing
    currentIdentifiers.clear();
}

void RecursiveDescentParser::Value1()
{
    // Value -> NumericalLiteral (numt)
    if (Token == numt) {
        bool isFloat = (Lexeme.find('.') != string::npos);
        
        for (auto& id : currentIdentifiers) {
            TableEntry* entry = st.Lookup(id);
            if (entry && entry->TypeOfEntry == constEntry) {
                if (isFloat) {
                    entry->constant.TypeOfConstant = floatType;
                    entry->constant.ValueR = ValueR; // From lexical analyzer
                    entry->constant.Offset = Offset;
                    Offset += 4;
                } else {
                    entry->constant.TypeOfConstant = intType;
                    entry->constant.Value = Value;   // From lexical analyzer
                    entry->constant.Offset = Offset;
                    Offset += 2;
                }
            }
        }
        Match(numt);
    } else {
        // Error handling
        cout << name << ": " << LineNo+1 << ": " 
            << "Error: " << RESET << "expecting numerical literal" << endl;
        error = true;
    }
}

void RecursiveDescentParser::Procedures()
{
    if (Token == proceduret) {
        Prog();
        Procedures();
    } else {
        // ε
    }
}

void RecursiveDescentParser::Args()
{
    if (Token == lparent) {
        Match(lparent);
        //cout << "DEBUG: Starting parameter list at Depth = " << Depth << endl;
        processingParams = true;
        ArgList();
        processingParams = false;
        Match(rparent);
    } else {
        // ε
    }
}

void RecursiveDescentParser::ArgList()
{
    if (Token == in || Token == out || Token == inout || Token == idt) {
        // process mode if present
        if (Token == in || Token == out || Token == inout) {
            Mode();
        }
        
        // before processing identifiers, save the current count
        int startCount = currentIdentifiers.size();
        
        // process identifiers
        IdentifierList();

        // calculate how many new identifiers were added
        int paramCount = currentIdentifiers.size() - startCount;
        
        // update the procedure's parameter count
        if (currentProcedure != nullptr) {
            currentProcedure->function.NumberOfParameters += paramCount;
        }
        
        Match(colont);
        TypeMark();
        MoreArgs();
    } else {
        cout << name << ": " << LineNo+1 << ": " 
            << "Error: " << RESET << "expecting in, out, inout, or idt" << endl;
        error = true;
    }
}

void RecursiveDescentParser::MoreArgs()
{
    if (Token == semit) {
        Match(semit);
        ArgList();
    } else {
        // ε
    }
}

void RecursiveDescentParser::Mode()
{
    if (Token == in) {
        currentMode = 0; // in
        Match(in);
    } else if (Token == out) {
        currentMode = 1; // out
        Match(out);
    } else if (Token == inout) {
        currentMode = 2; // inout
        Match(inout);
    } else {
        // ε
    }
}

void RecursiveDescentParser::SeqOfStatements()
{
    // SeqOfStatements -> Statement ; StatTail | ε
    if(Token == idt || Token == ift || Token == whilet || Token == begint 
        || Token == gett || Token == putt || Token == putlnt) {
        Statement();
        Match(semit);
        StatTail();
    } else {
        // ε
    }
}

void RecursiveDescentParser::StatTail()
{
    // StatTail -> Statement ; StatTail | ε
    if(Token == idt || Token == ift || Token == whilet || Token == begint
        || Token == gett || Token == putt || Token == putlnt) {
        Statement();
        Match(semit);
        StatTail();
    } else {
        // ε
    }
}

void RecursiveDescentParser::Statement()
{
    // Statement -> AssignStat | IOStat
    if (Token == idt) {
        AssignStat();
    } else {
        IOStat();
    }
}

void RecursiveDescentParser::AssignStat() {
    // AssignStat -> idt := Expr
    if (Token == idt) {
        TableEntry* entry = st.Lookup(Lexeme);
        if (entry == nullptr) {
            cout << name << ": " << LineNo+1 << ": "
                << "Error: " << RESET << "undeclared identifier: " 
                << Lexeme << RESET << " at Depth: " << Depth << endl;
            error = true;
            return;
        }
        
        // Prepare left-hand side based on depth
        string leftSide;

        leftSide = GetVarReference(entry);

        string idName = Lexeme; // Save for ProcCall check
        Match(idt);
        
        if (Token == assignopt) {
            Match(assignopt);
            string rightSide = Expr();
            emit(leftSide + " = " + rightSide);
        } else if (Token == lparent) {
            // This is a procedure call, call the ProcCall method
            ProcCall(idName);
        }
    } else {
        cout << name << ": " << LineNo+1 << ": "
            << "Error: " << RESET << "expecting identifier" << endl;
        error = true;
    }
}

void RecursiveDescentParser::IOStat()
{
    // IOStat -> In_Stat | Out_Stat
    if (Token == gett) {
        InStat();
    } else if (Token == putt || Token == putlnt) {
        OutStat();
    } else {
        cout << name << ": " << LineNo+1 << ": "
             << "Error: " << RESET << "expecting get, put, or putln" << endl;
        error = true;
    }
}

void RecursiveDescentParser::InStat()
{
    // In_Stat -> get ( IdList )
    Match(gett);
    Match(lparent);  
    IdList();        
    Match(rparent);    
}

void RecursiveDescentParser::IdList()
{
    if (Token == idt) {
        string varName = Lexeme;
        TableEntry* entry = st.Lookup(varName);

        if (entry == nullptr) {
            cout << name << ": " << LineNo+1 << ": "
                 << "Error: " << RESET << "undeclared identifier: " << varName << RESET << endl;
            error = true;
        } else {
            emit("rdi " + GetVarReference(entry)); // Emit TAC
        }

        Match(idt);
        IdListTail();
    } else {
        cout << name << ": " << LineNo+1 << ": "
             << "Error: " << RESET << "expecting identifier in IdList" << endl;
        error = true;
    }
}

void RecursiveDescentParser::IdListTail()
{
    if (Token == commat) {
        Match(commat);
        IdList();
    }
    // else ε
}

void RecursiveDescentParser::OutStat()
{
    bool isPutln = false;

    if (Token == putlnt) {
        isPutln = true;
    }

    Match(Token);  // Match put or putln

    Match(lparent);
    WriteList();
    Match(rparent);

    if (isPutln) {
        emit("wrln");
    }
}

void RecursiveDescentParser::WriteList()
{
    WriteToken();
    WriteListTail();
}

void RecursiveDescentParser::WriteListTail()
{
    if (Token == commat) {
        Match(commat);
        WriteToken();
        WriteListTail();
    }
    // else ε
}

void RecursiveDescentParser::WriteToken()
{
    if (Token == idt) {
        TableEntry* entry = st.Lookup(Lexeme);
        if (entry == nullptr) {
            cout << name << ": " << LineNo+1 << ": "
                 << "Error: " << RESET << "undeclared identifier: " << Lexeme << RESET << endl;
            error = true;
        } else {
            emit("wri " + GetVarReference(entry));
        }
        Match(idt);
    } else if (Token == numt) {
        emit("wri " + Lexeme);
        Match(numt);
    } else if (Token == literalt) {
        string label = InsertStringLiteral(Lexeme);  // You will create this next
        emit("wrs " + label);
        Match(literalt);
    } else {
        cout << name << ": " << LineNo+1 << ": "
             << "Error: " << RESET << "expecting id, num, or literal in WriteToken" << endl;
        error = true;
    }
}

string RecursiveDescentParser::Expr()
{
    // IOStat -> Relation
    return Relation();
}

string RecursiveDescentParser::Relation() {
    // Relation -> SimpleExpr
    string leftOperand = SimpleExpr();
    
    if (Token == relopt) {
        string op = Lexeme; // Save the relational operator
        Match(Token);
        string rightOperand = SimpleExpr();
        TableEntry* temp = NewTemp();
        emit(GetVarReference(temp) + " = " + leftOperand + " " + op + " " + rightOperand);
        return GetVarReference(temp);
    }
    
    return leftOperand;
}

string RecursiveDescentParser::SimpleExpr()
{
    // SimpleExpr -> Term MoreTerm
    string result = Term();
    return MoreTerm(result);
}

string RecursiveDescentParser::MoreTerm(const string& inherited) {
    // MoreTerm -> addopt Term MoreTerm | ε
    if (Token == addopt) {
        string op = Lexeme; // Save the operator (+ - or)
        Match(Token);
        string rightOperand = Term();
        TableEntry* temp = NewTemp();
        emit(GetVarReference(temp) + " = " + inherited + " " + op + " " + rightOperand);
        return MoreTerm(GetVarReference(temp));
    } else {
        return inherited; // No more operations, return what we have
    }
}
    
string RecursiveDescentParser::Term() {
    // Term -> Factor MoreFactor
    string result = Factor();
    return MoreFactor(result);
}
   
string RecursiveDescentParser::MoreFactor(const string& inherited) {
    // MoreFactor -> mulopt Factor MoreFactor | ε
    if (Token == mulopt) {
        string op = Lexeme; // Save the operator (* / mod rem and)
        Match(mulopt);
        string rightOperand = Factor();
        TableEntry* temp = NewTemp();
        emit(GetVarReference(temp) + " = " + inherited + " " + op + " " + rightOperand);
        return MoreFactor(GetVarReference(temp));
    } else {
        return inherited; // No more operations, return what we have
    }
}

string RecursiveDescentParser::Factor() {
    string result;
    if (Token == idt) {
        // Get identifier from symbol table
        TableEntry* entry = st.Lookup(Lexeme);
        if (entry == nullptr) {
            cout << name << ": " << LineNo+1 << ": " 
                << "Error: " << RESET << "undeclared identifier: " 
                << Lexeme << RESET << " at Depth: " << Depth << endl;
            error = true;
            return "";
        }
        
        // If depth == 1, use variable name directly, else use offset notation
        if (entry->TypeOfEntry == constEntry) {
            // Directly substitute constant values
            if (entry->constant.TypeOfConstant == intType) {
                result = to_string(entry->constant.Value);
            } else {
                result = to_string(entry->constant.ValueR);
            }
        } else if (entry->depth == 1) {
            result = Lexeme;
        } else {
            // Handle variables at deeper depths with offset notation
            if (entry->TypeOfEntry == varEntry) {
                // Parameters use positive offsets
                if (entry->depth == 1) {
                    result = entry->lexeme; // global — use name
                } else {
                    if (entry->TypeOfEntry == varEntry) {
                        if (entry->isParam) {
                            result = "_BP+" + to_string(entry->var.Offset);  // parameter
                        } else {
                            result = "_BP-" + to_string(entry->var.Offset);  // local or temp
                        }
                    }
                }
            }
        }
        Match(idt);
    } else if (Token == numt) {
        // For number literals, just return the value
        result = Lexeme;
        Match(numt);
    } else if (Token == lparent) {
        Match(lparent);
        result = Expr();
        Match(rparent);
    } else if (Token == nott) {
        Match(nott);
        string operand = Factor();
        TableEntry* temp = NewTemp();
        emit(GetVarReference(temp) + " = not " + operand);
        result = GetVarReference(temp);
    } else if (Token == addopt) {
        string op = Lexeme; // Save the operator (+ or -)
        Match(addopt);
        string operand = Factor();
        if (op == "-") {
            TableEntry* temp = NewTemp();
            emit(GetVarReference(temp) + " = -" + operand);
            result = GetVarReference(temp);
        } else {
            // For + unary operator, just return the operand
            result = operand;
        }
    } else {
        cout << name << ": " << LineNo+1 << ": " 
            << "Error: " << RESET << "expecting identifier, number, '(', 'not', or sign operator" << endl;
        error = true;
        return "";
    }
    
    return result;
}

void RecursiveDescentParser::ProcCall(const string& procName) {
    Match(lparent);
    Params();
    Match(rparent);
    
    // Generate call instruction
    emit("call " + procName);
}

void RecursiveDescentParser::Params() {
    if (Token == idt) {
        TableEntry* entry = st.Lookup(Lexeme);
        if (entry == nullptr) {
            cout << name << ": " << LineNo+1 << ": " 
                << "Error: " << RESET << "undeclared identifier: " 
                << Lexeme << RESET << " at Depth: " << Depth << endl;
            error = true;
            return;
        }
        
        string paramName;
        if (entry->depth == 1) {
            paramName = Lexeme;
        } else {
            if (entry->TypeOfEntry == varEntry) {
                if (entry->isParam) {
                    paramName = "_BP+" + to_string(entry->var.Offset);
                } else {
                    paramName = "_BP-" + to_string(entry->var.Offset);
                }
            }
        }
        
        // Check for parameter mode
        if (entry->paramMode = modeOut) {
            emit("push @" + paramName);
        } else {
            emit("push " + paramName);
        }
        
        Match(idt);
        ParamsTail();
    } else if (Token == numt) {
        // For number literals, just push the value
        emit("push " + Lexeme);
        Match(numt);
        ParamsTail();
    }
}

void RecursiveDescentParser::ParamsTail() {
    if (Token == commat) {
        Match(commat);
        
        if (Token == idt) {
            TableEntry* entry = st.Lookup(Lexeme);
            if (entry == nullptr) {
                cout << name << ": " << LineNo+1 << ": " 
                    << "Error: " << RESET << "undeclared identifier: " 
                    << Lexeme << RESET << " at Depth: " << Depth << endl;
                error = true;
                return;
            }
            
            string paramName;
            if (entry->depth == 1) {
                paramName = Lexeme;
            } else {
                if (entry->TypeOfEntry == varEntry) {
                    if (entry->isParam) {
                        paramName = "_BP+" + to_string(entry->var.Offset);
                    } else {
                        paramName = "_BP-" + to_string(entry->var.Offset);
                    }
                }
            }
            
            
            if (entry->paramMode == modeOut) {
                emit("push @" + paramName);
            } else {
                emit("push " + paramName);
            }
            
            Match(idt);
            ParamsTail();
        } else if (Token == numt) {
            // For number literals, just push the value
            emit("push " + Lexeme);
            Match(numt);
            ParamsTail();
        }
    }
    // If not comma, we're done with parameters
}

void RecursiveDescentParser::GenerateAssembly()
{
    ifstream tacInput(name.substr(0, name.find_last_of('.')) + ".tac");
    ofstream asmOutput(name.substr(0, name.find_last_of('.')) + ".asm");

    if (!tacInput.is_open() || !asmOutput.is_open()) {
        cout << "Error: " << RESET << "Could not open TAC or ASM file" << endl;
        return;
    }

    WriteAsmHeader(asmOutput);
    WriteDataSection(asmOutput);
    WriteCodeSection(asmOutput, tacInput);
}

void RecursiveDescentParser::WriteAsmHeader(ofstream& asmOutput)
{
    asmOutput << ".model small\n";
    asmOutput << ".586\n";
    asmOutput << ".stack 100h\n";
    asmOutput << endl;
}

void RecursiveDescentParser::WriteDataSection(ofstream& asmOutput)
{
    asmOutput << ".data\n";
    for (const auto& [label, text] : stringLiterals) {
        asmOutput << label << " DB \"" << text << "\",\"$\"\n";
    }
    for (const auto& var : globalVars) {
        asmOutput << var << " DW ?\n";
    }
    for (const string& temp : globalTemps) {
        asmOutput << temp << " DW ?\n";
    }
    
    asmOutput << endl;
}

void RecursiveDescentParser::WriteCodeSection(ofstream& asmOutput, ifstream& tacInput)
{
    asmOutput << ".code\n";
    asmOutput << "include io.asm\n\n";

    string line;
    while (getline(tacInput, line)) {
        ParseTacLine(line, asmOutput);
    }
}

void RecursiveDescentParser::ParseTacLine(const string& line, ofstream& asmOutput)
{
    istringstream iss(line);
    string word;
    iss >> word;

    if (word == "proc") {
        string procName;
        iss >> procName;
        asmOutput << procName << " PROC\n";
        asmOutput << "push bp\nmov bp, sp\n";
        int size = st.GetLocalSize(procName);
        asmOutput << "sub sp, " << size << "\n";
    }
    else if (word == "endp") {
        string procName;
        iss >> procName;
        int size = st.GetLocalSize(procName);
        asmOutput << "add sp, " << size << "\n";
        asmOutput << "pop bp\nret 0\n";
        asmOutput << procName << " ENDP\n\n";
    }
    else if (word == "start") {
        string dummy, procName;
        iss >> dummy >> procName;
        asmOutput << "start PROC\n";
        asmOutput << "mov ax, @data\nmov ds, ax\n";
        asmOutput << "call " << procName << "\n";
        asmOutput << "mov ah, 4ch\nmov al, 0\nint 21h\n";
        asmOutput << "start ENDP\n\nEND start\n";
    }
    else if (word == "wrs") {
        string label;
        iss >> label;
        asmOutput << "mov dx, offset " << label << "\n";
        asmOutput << "call writestr\n";
    }
    else if (word == "wrln") {
        asmOutput << "call writeln\n";
    }
    else if (word == "wri") {
        string var;
        iss >> var;
        asmOutput << "mov dx, " << ResolveAddress(var) << "\n";
        asmOutput << "call writeint\n";
    }
    else if (word == "rdi") {
        string var;
        iss >> var;
        asmOutput << "call readint\n";
        asmOutput << "mov [bp" << FormatOffset(var) << "], bx\n";
    }
    else if (word == "call") {
        string procName;
        iss >> procName;
        asmOutput << "call " << procName << "\n";
    }
    else if (line.find('=') != string::npos) {
        HandleAssignment(line, asmOutput);
    }
    else {
        // Handle assignments later
    }
}

string RecursiveDescentParser::InsertStringLiteral(string literal)
{
    static int stringCounter = 0;
    string label = "_S" + to_string(stringCounter++);

    string cleaned = literal.substr(1, literal.length() - 2);
    stringLiterals.emplace_back(label, cleaned);

    if (st.Lookup(label) == nullptr) {
        st.Insert(label, idt, 1); // insert at depth 1
        TableEntry* entry = st.Lookup(label);
        entry->TypeOfEntry = varEntry;
        entry->var.TypeOfVariable = charType;
        entry->var.size = literal.length() + 1; // plus 1 for the '$'
    }

    return label;
}

void RecursiveDescentParser::HandleAssignment(const string& line, ofstream& asmOutput)
{
    string lhs, rhs;
    size_t eq = line.find('=');
    if (eq == string::npos) return;

    lhs = trim(line.substr(0, eq));
    rhs = trim(line.substr(eq + 1));

    // Case 1: constant assignment (e.g., _BP-4 = 10)
    if (isNumber(rhs)) {
        asmOutput << "mov ax, " << rhs << "\n";
        asmOutput << "mov " << ResolveAddress(lhs) << ", ax\n";
        return;
    }

    // Case 2: simple move (e.g., _BP-4 = _BP-6)
    if (rhs.find(' ') == string::npos) {
        asmOutput << "mov ax, " << ResolveAddress(rhs) << "\n";
        asmOutput << "mov " << ResolveAddress(lhs) << ", ax\n";
        return;
    }

    // Case 3: binary expression (e.g., _BP-4 = _BP-2 + _BP-6)
    istringstream iss(rhs);
    string left, op, right;
    iss >> left >> op >> right;

    asmOutput << "mov ax, " << ResolveAddress(left) << "\n";

    if (op == "+") {
        asmOutput << "add ax, " << ResolveAddress(right) << "\n";
    } else if (op == "-") {
        asmOutput << "sub ax, " << ResolveAddress(right) << "\n";
    } else if (op == "*") {
        asmOutput << "mov bx, " << ResolveAddress(right) << "\n";
        asmOutput << "imul bx\n";
    } else {
        asmOutput << "; unsupported operator: " << op << "\n";
    }

    asmOutput << "mov " << ResolveAddress(lhs) << ", ax\n";
}

string RecursiveDescentParser::ResolveAddress(const string& var)
{
    if (var.find("_BP") != string::npos) {
        return "[bp" + FormatOffset(var) + "]";
    }
    return var;
}

string RecursiveDescentParser::FormatOffset(const string& var)
{
    size_t pos = var.find('+');
    if (pos != string::npos) {
        return "+" + var.substr(pos+1);
    }
    pos = var.find('-');
    if (pos != string::npos) {
        return "-" + var.substr(pos+1);
    }
    return ""; // shouldn't happen
}

bool RecursiveDescentParser::isNumber(const string& s)
{
    return !s.empty() && all_of(s.begin(), s.end(), ::isdigit);
}

string RecursiveDescentParser::trim(const string& s)
{
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    return (start == string::npos || end == string::npos) ? "" : s.substr(start, end - start + 1);
}

int RecursiveDescentParser::size(Symbol type) {
    if (type == integert) {
        return 2;
    } else if (type == chart) {
        return 1;
    } else if (type == floatt) {
        return 4;
    }
    return 0; // Unknown type
}
