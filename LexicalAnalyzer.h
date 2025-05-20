/*
 * LexicalAnalyzer.h
 *
 * CSC 446 - Assignment #3
 * Compiler Construction - Lexical Analyzer Header
 *
 * Author: Landon Dahmen
 * Due Date: February 5, 2025
 *
 * Description:
 *   This header file declares the LexicalAnalyzer class and supporting data
 *   structures for tokenizing source code of an Ada subset. It defines the
 *   Token structure, token types, and the interface for reading and processing
 *   input from a source file.
 */
#ifndef _LEXICALANALYZER_H
#define _LEXICALANALYZER_H
#include <unordered_map>
#include <string>
#include <functional>       // hash

using namespace std;

enum Symbol {
    begint, modulet, constantt, proceduret, ist, ift, thent, elset, elsift, 
    whilet, loopt, floatt, integert, chart, gett, putt, putlnt, endt, relopt, 
    literalt, addopt, mulopt, assignopt, lparent, rparent, commat, colont, 
    semit, periodt, numt, idt, eoft, unknownt, in, out, inout, nott
};

class LexicalAnalyzer {
    public:

        LexicalAnalyzer(string name);
        ~LexicalAnalyzer();
        void GetNextToken();
        string GetTokenName(Symbol token);

        void GetNextCh();
        void ProcessToken();
        void ProcessWordToken();
        void ProcessNumToken();
        void ProcessComment();
        void ProcessDoubleToken();
        void ProcessSingleToken();
        void ProcessLiteralToken();
        void DisplayToken();

    private:  
        bool IsReservedWord(string& s);

        unordered_map<string, Symbol> reservedWords;

        // helpers
        string ToUpper(string s);
        string ToLower(string s);
};
#endif
