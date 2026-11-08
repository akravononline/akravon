#pragma once

BOOL ShiftJIS_IsLeadByte( const char chByte );
BOOL ShiftJIS_IsTrailByte( const char chByte );
int ShiftJIS_StringCompareCI( LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength );
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
