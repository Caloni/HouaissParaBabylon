#define _CRT_SECURE_NO_DEPRECATE
#include "Files.h"
#include "Common.h"

#include <windows.h>
#include <Shlwapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>

using namespace std;


bool GetConfigString(const string& app, const string& key, string& value)
{
	CHAR val[MAX_PATH] = {};
	::GetPrivateProfileString(app.c_str(), key.c_str(), value.c_str(), val, MAX_PATH, CONFIGURATION_FILE_PATH);
	value = val;
	return ! value.empty();
}


bool FindFiles(const string& wildcard, vector <string> &files)
{
	WIN32_FIND_DATAA findData = {};

	HANDLE findH = FindFirstFileA(wildcard.c_str(), &findData);

	if (findH != INVALID_HANDLE_VALUE)
	{
		do
		{
			files.push_back(findData.cFileName);
		} 
		while (FindNextFileA(findH, &findData));

		FindClose(findH);
	}
	else return false;

	return true;
}


bool CreateFolder(const std::string& path)
{
	bool ret = true;

	if ( PathFileExistsA(path.c_str()) == FALSE )
	{
		size_t lastSlash = path.find_last_of('\\');
		if (lastSlash != path.npos)
		{
			string rootPath = path.substr(0, lastSlash);

			if (PathFileExistsA(rootPath.c_str()) == FALSE)
			{
				ret = CreateFolder(rootPath);
			}
		}

		if (ret)
		{
			if (CreateDirectoryA(path.c_str(), NULL))
				ret = true;
			else
				ret = false;
		}
	}

	return ret;
}
