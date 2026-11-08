#pragma once
#include "stdafx.h"
#include <fstream>
#include <string>
#include <shellapi.h>
#include "PythonPlayer.h"
#include "PythonNetworkStream.h"

char chName[CHARACTER_NAME_MAX_LEN + 1];
void TitleCheckWindow()
{
	POINT p;
	HWND DebugerFound = 0;
	for (int qy = 0; qy < 100; qy++)
	{
		for (int qx = 0; qx < 100; qx++)
		{
			p.x = qx * 20;
			p.y = qy * 20;
			DebugerFound = WindowFromPoint(p);
			char t[255];
			GetWindowTextA(DebugerFound, t, 225);

			if (strstr(t, "Ýlanlarým"))
			{
				unsigned char * hack = (unsigned char*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "OpenProcess");
				if (*(hack + 6) == 0xEA)
				{
				}
				else
				{
					if (strcmp(chName, CPythonPlayer::Instance().GetName()) != 0)
					{
						char postDataStr[CHARACTER_NAME_MAX_LEN + 1];
						snprintf(postDataStr, sizeof(postDataStr), "ilanlarim");

						snprintf(chName, sizeof(chName), CPythonPlayer::Instance().GetName());

						if (strlen(chName) > 0)
						{
							CPythonNetworkStream::Instance().Rapor(postDataStr);
						}
					 }
				}
			}

			if (strstr(t, "Ýlan Ekle"))
			{
				unsigned char * hack = (unsigned char*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "OpenProcess");
				if (*(hack + 6) == 0xEA)
				{
				}
				else
				{
					if (strcmp(chName, CPythonPlayer::Instance().GetName()) != 0)
					{
						char postDataStr[CHARACTER_NAME_MAX_LEN + 1];
						snprintf(postDataStr, sizeof(postDataStr), "ilanekle");

						snprintf(chName, sizeof(chName), CPythonPlayer::Instance().GetName());

						if (strlen(chName) > 0)
						{
							CPythonNetworkStream::Instance().Rapor(postDataStr);
						}
					}
				}
			}

			if (strstr(t, "Mesajlar"))
			{
				unsigned char * hack = (unsigned char*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "OpenProcess");
				if (*(hack + 6) == 0xEA)
				{
				}
				else
				{
					if (strcmp(chName, CPythonPlayer::Instance().GetName()) != 0)
					{
						char postDataStr[CHARACTER_NAME_MAX_LEN + 1];
						snprintf(postDataStr, sizeof(postDataStr), "mesajlar");

						snprintf(chName, sizeof(chName), CPythonPlayer::Instance().GetName());

						if (strlen(chName) > 0)
						{
							CPythonNetworkStream::Instance().Rapor(postDataStr);
						}
					}
				}
			}
		}
	}	
}

void Window_Tekrar_Tara()
{
again:
	TitleCheckWindow();
	Sleep(3000);
	goto again;
}

void Anti_Windows_Tarama()
{
	snprintf(chName, sizeof(chName), "");
	CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(Window_Tekrar_Tara), NULL, 0, 0);
}
