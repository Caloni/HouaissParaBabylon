/** @file HouaissParaBabylon.cpp

@brief Desofuscador e conversor do dicionário Houaiss para Babylon.

@author Wanderley Caloni <wanderley@caloni.com.br>
@date 2008-04

@desc Permite ver o conteúdo dos arquivos do dicionário eletrônico Houaiss 
de forma a ser importado para outros dicionários, ou usado independente
da interface oficial.

Linque:
Houaiss: http://www.dicionariohouaiss.com.br

Obs.:
Esse programa não se destina à pirataria. Ele não fornece nenhum dos
dois dicionários, que devem ser adquiridos legalmente. Sua única
função é permitir o uso de um ótimo dicionários em uma excelente
interface de busca de significado de palavras.

@todo Geração de log
@todo Testes com várias versões

Bom proveito!
*/
#include "Decryptor.h"
#include "Conversor.h"
#include "Shell.h"
#include "HouaissParaBabylonDialog.h"
#include <stdio.h>


#pragma comment(lib, "atlthunk.lib")


int WINAPI _tWinMain(HINSTANCE inst, HINSTANCE, PTSTR cmdLine, int show)
{
	DWORD ret;
	HouaissParaBabylonDialog mainDlg;

	Log("Starting program. Compilation: %S", __DATE__);
	InitCommonControls();
	mainDlg.DoModal();
	Log("Finishing program");

	ret = GetLastOperationError();

	if( ret != ERROR_SUCCESS )
	{
		Log("Last error: %08X", ret);

		if( MessageBox(NULL, _T("Ocorreram erros durante a conversão ")
			_T("do dicionário. Gostaria de exportar um arquivo de log ")
			_T("para diagnosticar o problema?"),
			_T("Exportação de log"), MB_YESNO | MB_ICONINFORMATION) == IDYES )
		{
			DWORD logErr = ExportLogMessages();

			if( logErr != ERROR_SUCCESS )
			{
				CString errDesc = MessageDescription(logErr);
				
				MessageBox(NULL, errDesc, _T("Erro ao exportar arquivo de log"), 
					MB_ICONERROR | MB_OK);
			}
		}
	}

	return ret;
}
