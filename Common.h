/** @file Common.h

@brief Funções comuns usadas por todo o projeto.

@author Wanderley Caloni <wanderley@caloni.com.br>
@date 2008-04
*/
#pragma once
#include "Resource.h"
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>

#define PROJECT_NAME _T("HouaissParaBabylon")

#define DEMO_MODE 0 ///< Modo de teste e/ou demonstração

// Bit 29: erro reservado para usuários
#define CUSTOM_ERROR     ( 0x20000000 )
#define CUSTOM_ERROR_END ( CUSTOM_ERROR + g_customErrorCount )
extern size_t g_customErrorCount;

#define ERROR_H2B_DICTIONARY_NOT_FOUND       ( CUSTOM_ERROR )
#define ERROR_H2B_BABYLONBUILDER_NOT_FOUND   ( CUSTOM_ERROR + 1 )
#define ERROR_H2B_INSERT_ERROR_HERE          ( CUSTOM_ERROR + 2 )


template<typename T, size_t Sz>
inline size_t SizeofArray(const T(& array)[Sz]) { return Sz; }


/** Camada de abstração "View". */
class HouaissParaBabylonDialog;


/** Representa um arquivo aberto durante a operação do programa. */
struct H2BFile
{
	CString path;
	CHandle handle;
};


/** O conjunto de arquivos de operação do programa.

Esse conjunto de arquivo é passado para todas as partes do código, como
um recurso descentralizado. Isso permite distribuir logicamente os 
algoritmos de conversão sem perder a unicidade da operação.
*/
struct H2BFiles
{
	H2BFile workingDir; ///< Pasta de trabalho.

	H2BFile houOutputFile;
	H2BFile babInputFile;

	H2BFile babOutputFile;
	H2BFile babIconFile;
	H2BFile babProjFile;
};


/** Representa toda a operação do programa.

Esse conjunto de itens deve ser passado para todas as partes do código,
como um recurso descentralizado. Isso permite distribuir os algoritmos,
mantendo a unicidade da operação.
*/
struct H2BOperation
{
	HouaissParaBabylonDialog* dlg;
	int totalSteps;
	int currentStep;
	int currentPercent;

	void SetOperationStatus(int percent);
	void SetStatusMessage(const CString& message);
	void SetStatusMessageDescription(const CString& message);
	bool Cancel();
};


DWORD GetLastOperationError();
void SetLastOperationError(DWORD err);
void Log(const char* format, ...);
DWORD ExportLogMessages();
DWORD AskForPath(HWND hwnd, PCTSTR description, CString& path);
CString MessageDescription(DWORD err);
DWORD GetWorkingDir(H2BFiles* files);
CString GetPathFromRegistry(PCTSTR regName, PCTSTR valueName);
DWORD ExtractResourceToFile(PCTSTR resName, PCTSTR filePath);
