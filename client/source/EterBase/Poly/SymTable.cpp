#include "StdAfx.h"
#include "SymTable.h"

using namespace std;

CSymTable::CSymTable(int aTok, string aStr) : dVal(0), token(aTok), strlex(aStr)
{
}

CSymTable::~CSymTable()
{
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
