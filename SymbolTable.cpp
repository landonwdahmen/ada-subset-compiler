/*
 * SymbolTable.cpp
 *
 * CSC 446 - Assignment #4
 * Compiler Construction - Symbol Table Module Implementation
 *
 * Author: Landon Dahmen
 * Due Date: March 5, 2025
 *
 * Description:
 *   This file implements the symbol table module declared in SymbolTable.h.
 *   It includes functions for inserting records into the table, looking up
 *   identifiers, deleting records based on scope depth, and printing the table.
 *   The module uses a linked hash table with chaining to handle collisions.
 */
#include <iostream>
#include <iomanip>
#include "SymbolTable.h"

using namespace std;

SymbolTable::SymbolTable()
{
    //
    for (int i = 0; i < TableSize; i++) {
        SymTab[i] = nullptr;
    }
}

SymbolTable::~SymbolTable()
{
}

void SymbolTable::Insert(string lex, Symbol token, int depth)
{
    // insert the lexeme, token and depth into a record in st
    int x = hash(lex);
    TableEntry *Entry = new TableEntry;

    Entry->token = token;
    Entry->lexeme = lex;
    Entry->depth = depth;
    Entry->next = SymTab[x];
    SymTab[x] = Entry;
}

TableEntry *SymbolTable::Lookup(string lex)
{
    // lookup uses the lexeme to find the entry and returns a pointer to that entry
    int index = hash(lex);              // Compute hash index
    TableEntry* Entry = SymTab[index];  // Get the first entry in the bucket

    while (Entry != nullptr) {
        if (Entry->lexeme == lex) {
            return Entry;               // Found the entry, return pointer
        }
        Entry = Entry->next;
    }

    return nullptr;
}

void SymbolTable::DeleteDepth(int depth)
{
    // delete is passed the depth and deletes all entries at that depth
    for (int i = 0; i < TableSize; i++) {
        TableEntry* Entry = SymTab[i];
        TableEntry* Prev = nullptr;

        while (Entry != nullptr) {
            if (Entry->depth == depth && Entry->TypeOfEntry != functionEntry) {
                // Assignment 8 
                // Remove the entry
                TableEntry* toDelete = Entry;
                Entry = Entry->next; // Move to the next before deleting
               
                if (Prev == nullptr) {
                    // If deleting the first node in the list
                    SymTab[i] = Entry;
                } else {
                    Prev->next = Entry;
                }

                delete toDelete; // Free memory
            } else {
                // Move to the next entry
                Prev = Entry;
                Entry = Entry->next;
            }
        }
    }
}

void SymbolTable::WriteTable(int depth)
{
    cout << "\033[1;4m" << "Symbol Table for Depth " << depth << ":" << "\033[0m" << endl;
    cout << "\033[1;4;90m" << left << setw(15) << "Index"
         << setw(20) << "Lexeme"
         << setw(15) << "Type"
         << setw(15) << "DataType"
         << setw(15) << "Size"
         << setw(15) << "Offset"
         << setw(20) << "Value/Params" << "\033[0m" << endl;
   
    int count = 0;
    for (int i = 0; i < TableSize; i++) {
        TableEntry *Entry = SymTab[i];
        while (Entry != nullptr) {
            if (Entry->depth == depth) {
                count++;
                // Print index and lexeme
                cout << left << setw(15) << "[" + to_string(i) + "]"
                     << setw(20) << Entry->lexeme;
               
                // Print type and other fields based on entry type
                switch(Entry->TypeOfEntry) {
                    case constEntry:
                        cout << setw(15) << "Constant";
                       
                        if (Entry->constant.TypeOfConstant == intType) {
                            cout << setw(15) << "IntType"
                                 << setw(15) << "2"
                                 << setw(15) << Entry->constant.Offset
                                 << setw(20) << Entry->constant.Value;
                        } else if (Entry->constant.TypeOfConstant == floatType) {
                            cout << setw(15) << "FloatType"
                                 << setw(15) << "4"
                                 << setw(15) << Entry->constant.Offset
                                 << setw(20) << Entry->constant.ValueR;
                        }
                        break;
                       
                    case varEntry:
                        cout << setw(15) << "Variable";
                       
                        if (Entry->var.TypeOfVariable == intType) {
                            cout << setw(15) << "IntType";
                        } else if (Entry->var.TypeOfVariable == floatType) {
                            cout << setw(15) << "FloatType";
                        } else if (Entry->var.TypeOfVariable == charType) {
                            cout << setw(15) << "CharType";
                        }
                       
                        cout << setw(15) << Entry->var.size
                             << setw(15) << Entry->var.Offset;
                        break;
                       
                    case functionEntry:
                        cout << setw(15) << "Procedure"
                             << setw(15) << "-"  // No data type for procedures
                             << setw(15) << Entry->function.SizeOfLocal
                             << setw(15) << "-"  // No offset for procedures
                             << setw(20) << "Params: " + to_string(Entry->function.NumberOfParameters);
                        break;
                }
                cout << endl;
            }
            Entry = Entry->next;
        }
    }
    if (count == 0) {
        cout << "No entries at this depth" << endl;
    }
}

int SymbolTable::GetLocalSize(string procName)
{
    TableEntry* entry = Lookup(procName);
    if (entry && entry->TypeOfEntry == functionEntry) {
        return entry->function.SizeOfLocal;
    }
    return 0;
}

int  SymbolTable::hash(string lexeme)
{
    // passed a lexeme and return the location for that lexeme.
    unsigned int h = 0;
    unsigned int g = 0;

    for(char c : lexeme) {
        h = (h << 4) + c;
        if (g = h&0xf0000000) {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    return h % TableSize;
}