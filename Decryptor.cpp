/** @file Decryptor.cpp

@brief Desofuscador do dicionário Houaiss.

@author Wanderley Caloni <wanderley@caloni.com.br>
@date 2008-04

@desc Permite usar o conteúdo dos arquivos do dicionário eletrônico Houaiss 
por meio da interface do dicionário Babylon, independente
da interface oficial.

Linque:
Houaiss: http://www.dicionariohouaiss.com.br

Obs.:
Esse programa não se destina à pirataria. Ele não fornece nenhum dos
dois dicionários, que devem ser adquiridos legalmente. Sua única
função é permitir o uso de um ótimo dicionários em uma excelente
interface de busca de significado de palavras.

Bom proveito!
*/
#define _CRT_SECURE_NO_DEPRECATE

#include "Decryptor.h"
#include "Common.h"

#include <windows.h>
#include <stdio.h>

#define DEMO_SIZE ( 20 * 1024 * 1024 ) ///< 20 MB de arquivo em modo demo.


CString GetHouaissTempFileName(H2BFiles* files)
{
	CString ret;

	ret = files->workingDir.path + _T("\\")
		+ _T("deah001.dhx");

	return ret;
}


bool OpenHouaissFiles(PCTSTR houaissFileName, PCTSTR houaissTempFileName, 
					  HANDLE& houaissFile, HANDLE& houaissTempFile)
{
	houaissFile = CreateFile(houaissFileName, 
		GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);

	if( houaissFile == INVALID_HANDLE_VALUE )
		return false;

	houaissTempFile = CreateFile(houaissTempFileName, 
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
		NULL, CREATE_ALWAYS, 0, NULL);

	if( houaissTempFile == INVALID_HANDLE_VALUE )
		return false;

	return true;
}


bool ViewHouaissFiles(HANDLE houaissFile, HANDLE houaissTempFile,
					  HANDLE& houaissFileMap, HANDLE& houaissTempFileMap,
					  PVOID& houaissFileView, PVOID& houaissTempFileView,
					  DWORD& houaissFileSize)
{
	DWORD dwHouaissFileSizeHigh = 0;
	LONG lHouaissFileSizeHigh = 0;

	houaissFileSize = GetFileSize(houaissFile, &dwHouaissFileSizeHigh);

	if( houaissFileSize == INVALID_FILE_SIZE && GetLastError() != NO_ERROR )
		return false;

#if DEMO_MODE
	houaissFileSize = min(houaissFileSize, DEMO_SIZE);
#endif

	lHouaissFileSizeHigh = dwHouaissFileSizeHigh;
	if( SetFilePointer(houaissTempFile, houaissFileSize, &lHouaissFileSizeHigh, 
		FILE_BEGIN) == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
	{
		return false;
	}

	if( !SetEndOfFile(houaissTempFile) )
		return false;

	houaissFileMap = CreateFileMapping(houaissFile, NULL, 
		PAGE_READONLY, 0, 0, NULL);
	if( ! houaissFileMap || GetLastError() == ERROR_ALREADY_EXISTS )
		return false;

	houaissTempFileMap = CreateFileMapping(houaissTempFile, NULL, 
		PAGE_READWRITE, 0, 0, NULL);
	if( ! houaissTempFileMap || GetLastError() == ERROR_ALREADY_EXISTS )
		return false;

	houaissFileView = (PBYTE) MapViewOfFile(houaissFileMap, 
		FILE_MAP_READ, 0, 0, 0);
	if( ! houaissFileView )
		return false;

	houaissTempFileView = (PBYTE) MapViewOfFile(houaissTempFileMap, 
		FILE_MAP_WRITE, 0, 0, 0);
	if( ! houaissTempFileView )
		return false;

	return true;
}


void UnViewHouaissFiles(HANDLE& houaissFileMap, HANDLE& houaissTempFileMap,
						PVOID& houaissFileView, PVOID& houaissTempFileView)
{
	if( houaissFileView )
		UnmapViewOfFile(houaissFileView), houaissFileView = NULL;

	if( houaissTempFileView )
		UnmapViewOfFile(houaissTempFileView), houaissTempFileView = NULL;

	if( houaissFileMap )
		CloseHandle(houaissFileMap), houaissFileMap = NULL;

	if( houaissTempFileMap )
		CloseHandle(houaissTempFileMap), houaissTempFileMap = NULL;
}


bool DecryptHouaissFile(PVOID pvInput, DWORD size, PVOID pvOutput, H2BOperation& op)
{
	bool ret = true;
	PBYTE input = PBYTE(pvInput);
	PBYTE output = PBYTE(pvOutput);
	unsigned int sizePercent = size / 2;
	unsigned int factor = 2;
	unsigned int lastPercent = 0;
	unsigned int percent = 0;
	CString status;

	while( sizePercent > UINT_MAX / 100 )
	{
		sizePercent /= 2;
		factor *= 2;
	}

	for( DWORD inputCount = 0; inputCount < size; ++inputCount )
	{
		output[inputCount] = input[inputCount] + 0x0B;
		percent = ( inputCount / factor ) * 100 / sizePercent;

		if( op.Cancel() )
		{
			ret = false;
			SetLastError(ERROR_CANCELLED);
			break;
		}

		if( percent != lastPercent )
		{
			status.Format(_T("Desencriptando %d%%..."), percent);
			op.SetOperationStatus(lastPercent = percent);
			op.SetStatusMessage(status);
		}
	}

	return ret;
}


DWORD Decryptor(CString houaissFileName, H2BFiles* files, H2BOperation& op)
{
	DWORD ret = ERROR_SUCCESS;
	CString houaissTempFileName = GetHouaissTempFileName(files);

	Log("Houaiss temp file name: %s", (PCTSTR) houaissTempFileName );

	if( ! houaissFileName.IsEmpty() && ! houaissTempFileName.IsEmpty() )
	{
		HANDLE houaissFile;
		HANDLE houaissTempFile;

		if( OpenHouaissFiles(houaissFileName, houaissTempFileName,
			houaissFile, houaissTempFile) )
			{
				HANDLE houaissFileMap;
				HANDLE houaissTempFileMap;
				PVOID houaissFileView;
				PVOID houaissTempFileView;
				DWORD houaissFileSize;

				if( ViewHouaissFiles(houaissFile, houaissTempFile,
					houaissFileMap, houaissTempFileMap,
					houaissFileView, houaissTempFileView,
					houaissFileSize) )
				{
					if( ! DecryptHouaissFile(houaissFileView, houaissFileSize, 
						houaissTempFileView, op) )
					{
						ret = GetLastError();
						Log("Error %d decrypting Houaiss files", ret);
					}
				}
				else 
				{
					ret = GetLastError();
					Log("Error %d viewing Houaiss files", ret);
				}

				UnViewHouaissFiles(houaissFileMap, houaissTempFileMap,
					houaissFileView, houaissTempFileView);
		}
		else 
		{
			ret = GetLastError();
			Log("Error %d opening Houaiss files", ret);
		}

		files->houOutputFile.path = houaissTempFileName;
		files->houOutputFile.handle.Attach(houaissTempFile);

		if( houaissFile != INVALID_HANDLE_VALUE )
			CloseHandle(houaissFile);
	}
	else ret = GetLastError();

	return ret;
}
