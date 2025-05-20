/*
 * Parser.h
 *
 * CSC 446 - Assignment #3
 * Compiler Construction - Recursive Descent Parser Header
 *
 * Author: Landon Dahmen
 * Updated for Assignment 7d modifications
 *
 * Description:
 *   This header file declares the Parser class and its member functions used
 *   for recursive descent parsing of the language. It now includes prototypes
 *   for functions that generate three-address code (TAC) as well as members for
 *   storing TAC output.
 */
#ifndef _Parser_H
#define _Parser_H
#include "LexicalAnalyzer.h"
#include "SymbolTable.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

class RecursiveDescentParser {
    public:
        RecursiveDescentParser(string name);
        ~RecursiveDescentParser();

    private:
        ofstream tacFile;
        int tempCounter;
        string programName;
        void emit(string code);
        string GetVarReference(TableEntry* entry);
        TableEntry* NewTemp();
        SymbolTable st;
        LexicalAnalyzer lex;
        int Depth = 0;
        int Offset = 2;
        int ParamOffset = 0;
        vector<string> currentIdentifiers; // To keep track of identifiers being processed
        vector<string> currentParameters; // To keep track of parameters being processed
        void ProcessParams();
        int currentMode = 0; // 0 = in, 1 = out, 2 = inout
        TableEntry* currentProcedure = nullptr; // To keep track of current procedure
        bool processingParams = false; // Flag to indicate processing parameters
        string currentProcName = ""; //Assignment 8
        void Match(Symbol desired);
        void Prog();
        void DeclarativePart();
        void IdentifierList();
        void IdentifierListPrime();
        void TypeMark();
        void Value1();
        void Procedures();
        void Args();
        void ArgList();
        void MoreArgs();
        void Mode();
        void SeqOfStatements();
        void StatTail();
        void Statement();
        void AssignStat();
        void IOStat();
        void InStat();
        void IdList();
        void IdListTail();
        void OutStat();
        void WriteList();
        void WriteListTail();
        void WriteToken();
        string Expr();
        string Relation();
        string SimpleExpr();
        string MoreTerm(const string& inherited);
        string Term();
        string MoreFactor(const string& inherited);
        string Factor();
        void ProcCall(const string& procName);
        void Params();
        void ParamsTail();
        void GenerateAssembly();
        void WriteAsmHeader(ofstream& asmOutput);
        void WriteDataSection(ofstream& asmOutput);
        void WriteCodeSection(ofstream& asmOutput, ifstream& tacInput);
        void ParseTacLine(const string& line, ofstream& asmOutput);
        void HandleAssignment(const string& line, ofstream& asmOutput);
        string InsertStringLiteral(string literal);
        string ResolveAddress(const string& var);
        string FormatOffset(const string& var);
        bool isNumber(const string& s);
        string trim(const string& s);
        vector<pair<string, string>> stringLiterals;
        vector<string> globalVars;
        vector<string> globalTemps;
        int size(Symbol type);
        string name;
        bool error = false;
};
#endif