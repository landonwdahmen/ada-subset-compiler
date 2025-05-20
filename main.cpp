/*
 * main.cpp
 *
 * CSC 446 - Assignment #4
 * Compiler Construction - Symbol Table Test Driver
 *
 * Author: Landon Dahmen
 * Due Date: March 5, 2025
 *
 * Description:
 * Entry point for the semantic parser and symbol table integration (Assignment 5).
 * This program reads an Ada source file, invokes the parser, and then prints the
 * final symbol table for the global scope (depth 0).
 */
#include <iostream>
#include <fstream>
#include <iomanip>
#include "LexicalAnalyzer.h"
#include "Globals.h"
#include "Parser.h"
#include "SymbolTable.h"

using namespace std;

ifstream inputfile;
Symbol Token;
string Lexeme;
char ch;
int LineNo;
int Value;
double ValueR;
string Literal;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    } else {
        RecursiveDescentParser rdp(argv[1]);
    }
}