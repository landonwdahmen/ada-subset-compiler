/*
 * SymbolTable.h
 *
 * CSC 446 - Assignment #4
 * Compiler Construction - Symbol Table Module Header
 *
 * Author: Landon Dahmen
 * Due Date: March 5, 2025
 *
 * Description:
 *   This header file declares the data structures and function prototypes
 *   for the symbol table module. The symbol table is implemented as a linked
 *   hash table for storing records of variables, constants, and procedures.
 *   It provides functions to insert new records, lookup identifiers,
 *   delete records by scope depth, and output the table contents for debugging.
 */
#ifndef _SymbolTable_H
#define _SymbolTable_H
#include <string>
#include "LexicalAnalyzer.h"
#include "Globals.h"

const int TableSize = 211;      
enum VarType {charType, intType, floatType};
enum EntryType{constEntry, varEntry, functionEntry};
enum ParamMode{ none, modeIn, modeOut, modeInOut };

struct ParamNode {
    VarType typeOfParameter;
    ParamNode* next;
};   

struct TableEntry {
    string lexeme;
    Symbol token;
    int depth;
    EntryType TypeOfEntry;        // tag field for the union
    bool isParam = false;
    ParamMode paramMode = none;
    union {
        struct
        {    
            VarType TypeOfVariable;
            int Offset;
            int size;
        } var;
        struct
        {  // constant entry
            VarType TypeOfConstant; // int or float constant
            int Offset;
            union {
                int Value;
                float ValueR;
            };      
        } constant;
        struct
        {    // function entry
            int SizeOfLocal;
            int NumberOfParameters;
            VarType ReturnType;
            ParamNode* ParamList; // linked list of parameter types
        } function;
    }; // end of union
    TableEntry* next;  // ptr to next entry in the list
};

typedef TableEntry * EntryPtr;    // ptr to actual table entry

class SymbolTable {
    public:
        SymbolTable();
        ~SymbolTable();
        void Insert(string lex, Symbol token, int depth);
        TableEntry* Lookup(string lex);
        void DeleteDepth(int depth);
        void WriteTable(int depth);
        int GetLocalSize(string procName);

    private:
        int hash(string lexeme);
        TableEntry* SymTab[TableSize];
};
#endif