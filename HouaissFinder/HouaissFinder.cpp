/** @file HouaissFinder.h

@brief Localiza dicionários do Houaiss, instalados ou não.

@author Wanderley Caloni <wanderley@caloni.com.br>
@date 2008-04
*/
#define _CRT_SECURE_NO_DEPRECATE
#include "..\Common\Files.h"
#include "..\Common\Common.h"
#include "HouaissFinder.h"

#include <windows.h>
#include <Shlwapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>

#pragma comment(lib, "shlwapi.lib")

using namespace std;


void HouaissFind()
{
	WritePrivateProfileString("Houaiss", "ArquivosDoDicionario", "D:\\Houaiss\\Dicionario", CONFIGURATION_FILE_PATH);

	// achar o lugar em que está instalado e partir pra próxima



	//houaissFileName = GetPathFromRegistry(
	//	_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Houaiss"),
	//	_T("UninstallString"));

	//if (!PathFileExists(houaissFileName))
	//{
	//	Log("Houaiss path not found; trying Houaiss 3 path");

	//	houaissFileName = GetPathFromRegistry(
	//		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Dicionário eletrônico Houaiss da língua portuguesa_is1"),
	//		_T("UninstallString"));
	//}


	//CString ret;
	//LONG err;
	//CRegKey key;

	//err = key.Open(HKEY_LOCAL_MACHINE, regName, KEY_QUERY_VALUE);

	//if (err == ERROR_SUCCESS)
	//{
	//	TCHAR path[MAX_PATH] = {};
	//	ULONG pathSz = SizeofArray(path);

	//	err = key.QueryStringValue(valueName, path, &pathSz);

	//	if (err == ERROR_SUCCESS)
	//		ret = path;

	//	key.Close();
	//}

	//if (!ret.IsEmpty())
	//{
	//	int find = ret.ReverseFind(_T('\\'));
	//	if (find != -1)
	//		ret.Delete(find, ret.GetLength());
	//	if (ret[0] == '\"')
	//		ret.Delete(0, 1);
	//}

	//SetLastError(err);

	//return ret;
}


int main()
{
	HouaissFind();
}
