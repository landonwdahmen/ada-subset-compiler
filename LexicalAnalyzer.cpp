/*
 * LexicalAnalyzer.cpp
 *
 * CSC 446 - Assignment #3
 * Compiler Construction - Lexical Analyzer Implementation
 *
 * Author: Landon Dahmen
 * Due Date: February 5, 2025
 *
 * Description:
 *   This file implements the LexicalAnalyzer class declared in LexicalAnalyzer.h.
 *   It reads the source code from a file, tokenizes the input into identifiers,
 *   numbers, string literals, and symbols, and handles skipping whitespace and
 *   comments. The implementation includes error handling for unrecognized tokens
 *   and unterminated string literals.
 */
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>        
#include <string>
#include "LexicalAnalyzer.h"
#include "Globals.h"

using namespace std;

LexicalAnalyzer::LexicalAnalyzer(string name)
{
    Token = unknownt;
    ch = ' ';

    inputfile.open(name);
    if (!inputfile) {
        cout << "\033[31mError: could not open file \033[0m" << name << endl;
        Token = eoft;
    }

    reservedWords = {
        {"begin", begint}, {"module", modulet}, {"constant", constantt}, 
        {"procedure", proceduret}, {"is", ist}, {"if", ift}, {"then", thent}, 
        {"else", elset}, {"elsif", elsift}, {"while", whilet}, 
        {"loop", loopt}, {"float", floatt}, {"integer", integert}, 
        {"char", chart}, {"get", gett}, {"put", putt}, {"putln", putlnt}, {"end", endt},
        {"+", addopt}, {"-", addopt}, {"and", addopt}, {"or", addopt}, 
        {"*", mulopt}, {"/", addopt}, {"rem", mulopt}, {"mod", mulopt}, {"and", mulopt},
        {"in", in}, {"out", out}, {"inout", inout}, {"not", nott}
    };
}

LexicalAnalyzer::~LexicalAnalyzer()
{
    if(inputfile.is_open()) {
        inputfile.close();
    }
}

void LexicalAnalyzer::GetNextToken()
{
    while (isspace(ch)) {
        // 'eat' whitespace
        GetNextCh();
    }

    if (!inputfile.eof()) {
        ProcessToken();
    } else {
        inputfile.close();
        Token = eoft;
        Lexeme = "";
    }
}

void LexicalAnalyzer::GetNextCh()
{
    if (inputfile.get(ch)) {
        if (ch == '\n') {
            LineNo++;
        }
    } else {
        // reached end of file
        ch = EOF;
        Token = eoft;
    }
}

void LexicalAnalyzer::ProcessToken()
{
    Lexeme = "";
    Lexeme += ch; // store the first character of the lexeme
    GetNextCh();

    if (isalpha(Lexeme[0])) {
        ProcessWordToken();
    } else if (isdigit(Lexeme[0])) {
        ProcessNumToken();
    } else {
        switch (Lexeme[0]) {
            case '-':
                if (ch == '-') {
                    ProcessComment();
                } else {
                    ProcessSingleToken();
                }
                break;
            case '"': 
                ProcessLiteralToken();
                break;
            case '/': 
                if (ch == '=') {
                    ProcessDoubleToken(); 
                } else {
                    ProcessSingleToken();
                }
                break;
            case ':':
            case '<':
            case '>':
                if (ch == '=') {
                    ProcessDoubleToken();
                } else {
                    ProcessSingleToken();
                }
                break;
            default:
                ProcessSingleToken();
        }
    }
}

void LexicalAnalyzer::ProcessWordToken()
{
    while(isalnum(ch) || ch == '_') {
        Lexeme += ch;
        GetNextCh();
    }
    string temp = ToLower(Lexeme);
    if (IsReservedWord(temp)) {
        Token = reservedWords[temp];
        Lexeme = ToLower(Lexeme);
    } else if (temp == "real") {
        Token = reservedWords["float"];
        Lexeme = "float";
    } else {
        Token = idt;
    }
    if (Lexeme.length() > 17) {
        cout << "\033[31mError: identifier must be less than 18 characters\033[0m" << endl;
        Token = unknownt;
    }
}

void LexicalAnalyzer::ProcessNumToken()
{
    bool isFloat = false;
    while (isdigit(ch) || ch == '.') {
        if (ch == '.') {
            if (isFloat) {
                cout << "\033[31mError: multiple decimal points in number: \033[0m";
                cout << Lexeme << ch << endl;
                GetNextCh();
                GetNextToken();
                return;
            }
            isFloat = true;
        }
        Lexeme += ch;
        GetNextCh();
    }
    
    if (Lexeme.back() == '.') {
        cout << "\033[31mError: number cannot end with a decimal point: \033[0m";
        cout << Lexeme << endl;
        GetNextCh();
        GetNextToken();
        return;
    }
    if (isFloat) {
        Token = numt;
        ValueR = stod(Lexeme);
    } else {
        Token = numt;
        Value = stoi(Lexeme);
    }
}

void LexicalAnalyzer::ProcessComment()
{
    while (ch != '\n' && !inputfile.eof()) {
        GetNextCh();
    } 
    if (inputfile.eof()) {
        Token = eoft;
    } 
    // Do I need these 2 statements?
    GetNextCh();
    GetNextToken();
}

void LexicalAnalyzer::ProcessDoubleToken()
{
    Lexeme += ch;
    GetNextCh();
    if (Lexeme == ":=") {
        Token = assignopt;
    } else if (Lexeme == "<=" || Lexeme == ">=" || Lexeme == "/=") {
        Token = relopt;
    } 
}

void LexicalAnalyzer::ProcessSingleToken()
{
    switch(Lexeme[0]) {
        case '+': case '-': Token = addopt; break;
        case '*': case '/': Token = mulopt; break;
        case '=': case '<': case '>': Token = relopt; break;
        case '(': Token = lparent; break;
        case ')': Token = rparent; break;
        case ',': Token = commat; break;
        case ';': Token = semit; break;
        case ':': Token = colont; break;
        case '.': Token = periodt; break;
        default: 
            cout << "\033[31mError: unknown symbol: " << Lexeme[0] << "\033[0m" << endl;
            Token = unknownt; 
            break;
    }
}

void LexicalAnalyzer::ProcessLiteralToken()
{
    Token = literalt;
    while(ch!= '"') {
        Lexeme += ch;
        GetNextCh();
        if(ch == '\n') {
            cout << "\033[31mError: unterminated string literal\033[0m" << endl;
            Token = unknownt;
            Literal = Lexeme;
        }
    }
    Lexeme += ch;
    Literal = Lexeme;
    GetNextCh();
}

void LexicalAnalyzer::DisplayToken()
{
    cout << left << setw(20) << GetTokenName(Token) 
        << setw(20) << Lexeme;
    if (Token == numt) {
        if (Lexeme.find(".") != string::npos) {
            cout << "Value: " << left << setw(20) << ValueR;
        } else {
            cout << "Value: " << left << setw(20) << Value;
        }
    } 
    if (Token == literalt) {
        cout << left << setw(20) << Lexeme.substr(1, Lexeme.size() - 2);
    }
    cout << endl;
}

bool LexicalAnalyzer::IsReservedWord(string &s)
{
    auto it = reservedWords.find(s);
    if (it != reservedWords.end()) {
        Token = it->second;
        return true;
    }
    return false;
}

string LexicalAnalyzer::GetTokenName(Symbol token)
{
    static unordered_map<Symbol, string> tokenNames = {
        {begint, "begint"}, {modulet, "modulet"}, {constantt, "constantt"}, 
        {proceduret, "proceduret"}, {ist, "ist"}, {ift, "ift"}, {thent, "thent"},
        {elset, "elset"}, {elsift, "elsift"}, {whilet, "whilet"}, {loopt, "loopt"},
        {floatt, "floatt"}, {integert, "integert"}, {chart, "chart"}, {gett, "gett"},
        {putt, "putt"}, {endt, "endt"}, {relopt, "relopt"}, {literalt, "literalt"},
        {addopt, "addopt"}, {mulopt, "mulopt"}, {assignopt, "assignopt"}, 
        {lparent, "lparent"}, {rparent, "rparent"}, {commat, "commat"},
        {colont, "colont"}, {semit, "semit"}, {periodt, "periodt"}, {numt, "numt"},
        {idt, "idt"}, {eoft, "eoft"}, {unknownt, "unknownt"}
    };
    auto it = tokenNames.find(token);
    if (it != tokenNames.end()) {
        return it->second;
    }
    cout << "\033[31mError: Token not found\033[0m" << endl;
    return "UNKNOWN";
}

string LexicalAnalyzer::ToUpper(string s)
{
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

string LexicalAnalyzer::ToLower(string s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
