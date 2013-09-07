#define _CRT_SECURE_NO_WARNINGS

#include "Common.h"
#include <atlcoll.h>
#include <atlfile.h>
#include <shlobj.h>
#include <stdarg.h>
#include <stdio.h>

#define MAX_LOG_MSG 500
#define ERR_STR_BUF_SIZE 1000


/// @see CUSTOM_ERROR.
PCTSTR g_customErrors[] = 
{
	_T("Arquivos do dicionário Houaiss não foram encontrados. ")
		_T("Por favor, reinstale o dicionário com a opção de copiar ")
		_T("arquivos no disco rígido habilitada antes de efetuar a conversão."),
	
	_T("Conversor de dicionários não foi encontrado. ")
		_T("Por favor instale o Babylon Builder antes de efetuar a conversão.")
};
size_t g_customErrorCount = sizeof(g_customErrors) / sizeof(PCTSTR);

CAtlArray<CString> g_logMessages;

DWORD g_lastError = ERROR_SUCCESS;


DWORD GetLastOperationError()
{
	return g_lastError;
}


void SetLastOperationError(DWORD err)
{
	g_lastError = err;
}


CString MessageDescription(DWORD err)
{
	CString ret;

	Log("Message code: 0x%08X (%d)", err, err);

	if( err >= CUSTOM_ERROR && err < CUSTOM_ERROR_END )
	{
		ret = g_customErrors[err - CUSTOM_ERROR];
	}
	else
	{
		PTSTR msgBuffer = NULL;
		DWORD msgErr;

		msgErr = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER 
			| FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), 
			(PTSTR) &msgBuffer, 0, NULL);

		if( msgErr != 0 && msgBuffer )
		{
			ret = msgBuffer;
			LocalFree(msgBuffer);
		}
		else
		{
			ret.Format(_T("Erro de sistema número %d."), err);
		}
	}

	Log("MessageDescription: %s", (PCTSTR) ret);
	return ret;
}


DWORD GetWorkingDir(H2BFiles* files)
{
	DWORD ret = ERROR_SUCCESS;
	TCHAR tempPath[MAX_PATH] = { };

	if( GetTempPath(SizeofArray(tempPath), tempPath) )
	{
		CString& str = files->workingDir.path;

		str = tempPath;
		if( str[str.GetLength() - 1] == _T('\\') )
			str.Delete(str.GetLength() - 1);

		Log("Working directory: %s", (PCTSTR) str );
	}
	else 
	{
		ret = GetLastError();
		Log("Error %d getting working directory", ret);
	}

	return ret;
}


CString GetPathFromRegistry(PCTSTR regName, PCTSTR valueName)
{
	CString ret;
	LONG err;
	CRegKey key;

	err = key.Open(HKEY_LOCAL_MACHINE, regName, KEY_QUERY_VALUE);

	if( err == ERROR_SUCCESS )
	{
		TCHAR path[MAX_PATH] = { };
		ULONG pathSz = SizeofArray(path);

		err = key.QueryStringValue(valueName, path, &pathSz);

		if( err == ERROR_SUCCESS )
			ret = path;

		key.Close();
	}

	if( ! ret.IsEmpty() )
	{
		int find = ret.ReverseFind(_T('\\'));
		if( find != -1 )
			ret.Delete(find, ret.GetLength());
		if( ret[0] == '\"' )
			ret.Delete(0, 1);
	}

	SetLastError(err);

	return ret;
}


DWORD ExtractResourceToFile(PCTSTR resName, PCTSTR filePath)
{
	DWORD ret = ERROR_SUCCESS;

	if( HRSRC resH = FindResource(NULL, resName, RT_RCDATA) )
	{
		if( HGLOBAL res = LoadResource(NULL, resH) )
		{
			DWORD rawSize = SizeofResource(NULL, resH);
			PVOID rawData = LockResource(res);

			if( rawSize != 0 && rawData )
			{
				HANDLE file = CreateFile(filePath, GENERIC_WRITE, 0, NULL, 
					CREATE_ALWAYS, 0, NULL);

				if( file != INVALID_HANDLE_VALUE )
				{
					DWORD written = 0;

					if( ! WriteFile(file, rawData, rawSize, &written, NULL) ||
						written != rawSize )
					{
						ret = GetLastError();
					}

					CloseHandle(file);
				}
				else ret = GetLastError();
			}
		}
		else ret = GetLastError();
	}
	else ret = GetLastError();

	if( ret != ERROR_SUCCESS )
		DeleteFile(filePath);

	return ret;
}


void Log(const char* format, ...)
{
	CString formatBuffer = format;
	SYSTEMTIME sysTime;
	TCHAR buffer[MAX_LOG_MSG];
	CString logLine;
	va_list vaList;

	va_start(vaList, format);
	_vsntprintf(buffer, SizeofArray(buffer), formatBuffer, vaList);
	va_end(vaList);

	_tcsncat(buffer, _T("\r\n"), SizeofArray(_T("\r\n")));
	OutputDebugString(buffer);

	GetLocalTime(&sysTime);
	logLine.Format(_T("%04d-%02d%-02d %02d:%02d:%02d %04d.%04d %14s %s"),
		sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
		GetCurrentProcessId() & 0xFFFF, GetCurrentThreadId() & 0xFFFF,
		PROJECT_NAME, buffer);

	g_logMessages.Add(logLine);
}


DWORD AskForPath(HWND hwnd, PCTSTR description, CString& path)
{
	DWORD ret = ERROR_SUCCESS;
	HRESULT hr = CoInitialize(NULL);

	Log("Asking for path: %s", description);

	if( SUCCEEDED(hr) )
	{
		TCHAR displayName[MAX_PATH] = _T("");
		BROWSEINFO browseInfo = { };

		browseInfo.hwndOwner = hwnd;
		browseInfo.pszDisplayName = displayName;
		browseInfo.lpszTitle = description;
		browseInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE
			| BIF_NONEWFOLDERBUTTON | BIF_RETURNONLYFSDIRS;

		PIDLIST_ABSOLUTE pidList = SHBrowseForFolder(&browseInfo);

		if( pidList )
		{
			TCHAR selectedPath[MAX_PATH] = _T("");
			
			if( SHGetPathFromIDList(pidList, selectedPath) )
			{
				path = selectedPath;
				Log("Path (from user): %s", selectedPath);
			}
			else 
			{
				ret = ERROR_PATH_NOT_FOUND;
				Log("SHGetPathFromIDList failed");
			}

			CoTaskMemFree(pidList);
		}
		else 
		{
			ret = ERROR_CANCELLED;
			Log("SHBrowseForFolder cancelled");
		}

		CoUninitialize();
	}
	else 
	{
		ret = ERROR_INIT_STATUS_NEEDED;
		Log("COM initialization failed");
	}

	return ret;
}


DWORD ExportLogMessages()
{
	DWORD ret = ERROR_SUCCESS;
	CString logPath;

	ret = AskForPath(NULL, _T("Escolha o diretório de saída do arquivo de log"), 
		logPath);

	if( ret == ERROR_SUCCESS && ! logPath.IsEmpty() )
	{
		logPath += _T("\\HouaissParaBabylon.log");

		CAtlFile logFile;
		HRESULT hr;

		hr = logFile.Create(logPath, GENERIC_WRITE, 0, CREATE_ALWAYS);

		if( SUCCEEDED(hr) )
		{
			for( size_t i = 0; i < g_logMessages.GetCount(); ++i )
			{
				const CString& logLine = g_logMessages[i];

				hr = logFile.Write(CT2A(logLine), logLine.GetLength());

				if( FAILED(hr) )
				{
					ret = HRESULT_CODE(hr);
					break;
				}
			}
		}
		else ret = HRESULT_CODE(hr);

		if( ret == ERROR_SUCCESS )
		{
			ShellExecute(NULL, NULL, logPath, NULL, NULL, SW_SHOWDEFAULT);
		}
	}

	return ret;
}
