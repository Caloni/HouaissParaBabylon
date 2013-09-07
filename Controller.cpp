#include "Common.h"
#include "Conversor.h"
#include "Decryptor.h"
#include "Shell.h"
#include "HouaissParaBabylonDialog.h"

#include <atlbase.h>
#include <shlobj.h>

#define H2B_DICTIONARY_FILE _T("\\Dicionario\\deah001.dhx")
#define H2B_DICTIONARY_FILE2 _T("\\Dicionario\\deah001.dhn")

#define WAIT_EVT_TIMEOUT 1000


void ClearFile(H2BFile* file, bool onReboot)
{
	file->handle.Close();

	if( PathFileExists(file->path) )
	{
		if( onReboot )
			MoveFileEx(file->path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		else if( ! DeleteFile(file->path) )
			MoveFileEx(file->path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	}

	file->path.Empty();
}


void FinishOperation(H2BFiles* files, bool onReboot)
{
	ClearFile(&files->babProjFile, onReboot);
	ClearFile(&files->babIconFile, onReboot);
	ClearFile(&files->babOutputFile, onReboot);
	ClearFile(&files->babInputFile, onReboot);
	ClearFile(&files->houOutputFile, onReboot);
	delete files;
}


void H2BOperation::SetOperationStatus(int percent)
{
	int percentPart = 100 / totalSteps;
	int percentTotal = percentPart * ( currentStep - 1 );

	percent = percent * percentPart / 100;
	percentTotal += percent;

	dlg->ProgressSetValue(percentTotal * 10);
}


void H2BOperation::SetStatusMessage(const CString& message)
{
	dlg->Status(message);
}


void H2BOperation::SetStatusMessageDescription(const CString& message)
{
	dlg->StatusDescription(message);
}


bool H2BOperation::Cancel()
{
	bool ret = dlg->Cancel();
	return ret;
}


DWORD GetHouaissFileName(HWND hwnd, CString& houaissFileName)
{
	DWORD ret = ERROR_H2B_DICTIONARY_NOT_FOUND;

	houaissFileName = GetPathFromRegistry(
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Houaiss"),
		_T("UninstallString") );

	if( ! PathFileExists(houaissFileName) )
	{
		Log("Houaiss path not found; trying Houaiss 3 path");
		
		houaissFileName = GetPathFromRegistry(
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Dicionário eletrônico Houaiss da língua portuguesa_is1"),
			_T("UninstallString") );
	}

	if( PathFileExists(houaissFileName) )
	{
		CString test = houaissFileName + H2B_DICTIONARY_FILE;
		
		if( PathFileExists(test) )
		{
			Log("Found Houaiss 1 file");
			houaissFileName = test;
		}
		else
		{
			Log("Houaiss 1 file error; trying Houaiss 3 file");
			houaissFileName += H2B_DICTIONARY_FILE2;
		}
	}

	if( ! PathFileExists(houaissFileName) )
	{
		TCHAR progFiles[MAX_PATH];

		Log("Houaiss dictionary file does not exist: %s", (PCTSTR) houaissFileName);

		if( SHGetSpecialFolderPath(hwnd, progFiles, CSIDL_PROGRAM_FILES, FALSE) )
		{
			houaissFileName.Format(_T("%s\\%s%s"), progFiles, _T("Houaiss"),
				H2B_DICTIONARY_FILE);

			if( ! PathFileExists(houaissFileName) )
			{
				Log("Houaiss dictionary file does not exist: %s", (PCTSTR) houaissFileName);

				CString desc = _T("Por favor, selecione onde está o ")
					_T("diretório de instalação do dicionário Houaiss");

				MessageBox(hwnd, _T("O dicionário Houaiss não foi encontrado. Talvez seu ")
					_T("ambiente esteja um pouco diferente do esperado. Por favor, ")
					_T("coloque seu CD de instalação e me diga onde está a pasta ")
					_T("chamada \"Houaiss\" dentro dele."),
					_T("Arquivos do Houaiss não encontrados"),
					MB_ICONWARNING);
				AskForPath(hwnd, desc, houaissFileName);

				houaissFileName += H2B_DICTIONARY_FILE;

				if( ! PathFileExists(houaissFileName) )
				{
					Log("Houaiss dictionary file does not exist: %s", (PCTSTR) houaissFileName);
					ret = ERROR_H2B_DICTIONARY_NOT_FOUND;
				}
				else
				{
					ret = ERROR_SUCCESS;
					Log("Getting user houaiss path");
				}
			}
			else 
			{
				ret = ERROR_SUCCESS;
				Log("Getting default houaiss path");
			}
		}
		else Log("Error getting program files folder");
	}
	else 
	{
		ret = ERROR_SUCCESS;
		Log("Getting registry houaiss path");
	}

	if( ret == ERROR_SUCCESS )
		Log("Using Houaiss file: %s", (PCTSTR) houaissFileName);

	return ret;
}


DWORD GetBabylonBuilderFileName(HWND hwnd, CString& babylonBuilderFileName)
{
	DWORD ret = ERROR_SUCCESS;

	babylonBuilderFileName = GetPathFromRegistry(
		_T("SOFTWARE\\Classes\\CLSID\\{3A207676-6B05-4AE6-BAF0-9BF312F52444}\\InprocServer32"),
		_T("") );

	if( babylonBuilderFileName.IsEmpty() )
	{
		Log("Registry babylon builder entry empty, asking user...");
		CString desc = _T("Por favor, selecione onde está o ")
			_T("diretório de instalação do Babylon Builder");

		MessageBox(hwnd, 
			_T("Não consegui encontrar o Babylon Builder no seu ")
			_T("computador, essencial para a conversão do dicionário")
			_T(" Houaiss para o formato Babylon. Por favor, selecione a ")
			_T("pasta onde ele está instalado."),
			_T("Babylon Builder não encontrado"),
			MB_ICONWARNING);

		AskForPath(hwnd, desc, babylonBuilderFileName);
	}

	babylonBuilderFileName += _T("\\BuilderWizard.exe");

	if( ! PathFileExists(babylonBuilderFileName) )
	{
		Log("Babylon Builder not found: %s", (PCTSTR) babylonBuilderFileName);
		babylonBuilderFileName.Empty();
		ret = ERROR_H2B_BABYLONBUILDER_NOT_FOUND;
	}
	else  Log("Using Babylon Builder in: %s", (PCTSTR) babylonBuilderFileName);

	return ret;
}


DWORD WINAPI StartConversionThread(PVOID pvDlg)
{
	DWORD ret;
	H2BFiles *files = new H2BFiles;
	HouaissParaBabylonDialog* dlg = static_cast<HouaissParaBabylonDialog*>(pvDlg);
	H2BOperation op = { dlg, 1 };
	CString houaissFileName;
	CString babylonBuilderFileName;

	dlg->Status(_T("Iniciando operação..."));
	++op.currentStep;
	dlg->ProgressSetRange(0, 1000);
	ret = GetWorkingDir(files);

	if( ret == ERROR_SUCCESS )
		ret = GetHouaissFileName(dlg->m_hWnd, houaissFileName);

	if( ret == ERROR_SUCCESS )
		ret = GetBabylonBuilderFileName(dlg->m_hWnd, babylonBuilderFileName);

	if( ret == ERROR_SUCCESS )
	{
		dlg->Status(_T("Desencriptando arquivos..."));
		ret = Decryptor(houaissFileName, files, op);
	}

	if( ret == ERROR_SUCCESS )
	{
		op.SetOperationStatus(0);
		dlg->Status(_T("Convertendo arquivos..."));
		ret = Conversor(files, op);
	}

	if( ret == ERROR_SUCCESS )
	{
		op.SetOperationStatus(0);
		dlg->Status(_T("Construindo dicionário. Por favor aguarde..."));
		ret = BuildDictionary(babylonBuilderFileName, files, op);
	}

	if( ret == ERROR_SUCCESS )
	{
		op.SetOperationStatus(0);
		dlg->Message(_T("Operação concluída com sucesso!"), MB_ICONINFORMATION);
		dlg->Status(_T("O dicionário já pode ser instalado."));
	}
	else
	{
		FinishOperation(files, false), files = 0;

		if( ret == ERROR_CANCELLED )
		{
			dlg->Message(_T("Operação foi cancelada."), MB_ICONINFORMATION);
			dlg->Status(_T("Inicie novamente a conversão."));
		}
		else
		{
			CString errMsg = MessageDescription(ret);
			dlg->Message(errMsg, MB_ICONERROR);
			dlg->Status(_T("Não foi possível fazer a conversão."));
		}
	}

	SetLastOperationError(ret);
	dlg->FinishConversion(files, ret);
	return ret;
}


DWORD StartConversion(HouaissParaBabylonDialog* dlg)
{
	DWORD ret = ERROR_SUCCESS;
	HANDLE thread;
	DWORD tid = 0;

	thread = CreateThread(NULL, 0, StartConversionThread, dlg, 0, &tid);

	if( ! thread )
	{
		ret = GetLastError();
		SetLastOperationError(ret);
	}
	else
		CloseHandle(thread);

	return ret;
}


DWORD WINAPI StartInstallationThread(PVOID pvArgs)
{
	struct Args
	{
		HouaissParaBabylonDialog* dlg;
		H2BFiles* files;
		HANDLE receivedEvent;
	}
	args = *static_cast<Args*>(pvArgs);

	DWORD ret = ERROR_SUCCESS;

	SetEvent(args.receivedEvent);

	args.dlg->Status(_T("Instalando dicionário..."));
	ret = InstallDictionary(args.files);

	if( ret != ERROR_SUCCESS )
	{
		FinishOperation(args.files, false);
		args.dlg->Message(_T("Erro durante a instalação! Certifique-se ")
			_T("que o aplicativo Babylon está instalado."), 
			MB_ICONERROR);
		args.dlg->Status(_T("Ocorreram erros durante a operação."));
	}
	else
		FinishOperation(args.files, true);

	SetLastOperationError(ret);
	args.dlg->FinishInstallation(ERROR_SUCCESS);
	return ret;
}


DWORD StartInstallation(HouaissParaBabylonDialog* dlg, H2BFiles* files)
{
	DWORD ret = ERROR_SUCCESS;
	HANDLE receivedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if( receivedEvent )
	{
		HANDLE thread;
		DWORD tid = 0;
		PVOID args[] = { dlg, files, &receivedEvent };

		if( thread = CreateThread(NULL, 0, StartInstallationThread, args, 0, &tid) )
		{
			DWORD wait = WaitForSingleObject(receivedEvent, WAIT_EVT_TIMEOUT);

			if( wait != WAIT_OBJECT_0 )
			{
				ret = ERROR_INVALID_PARAMETER;
				SetLastOperationError(ret);
			}

			CloseHandle(thread);
		}
		else 
		{
			ret = GetLastError();
			SetLastOperationError(ret);
		}

		CloseHandle(receivedEvent);
	}
	else ret = GetLastError();

	return ret;
}
