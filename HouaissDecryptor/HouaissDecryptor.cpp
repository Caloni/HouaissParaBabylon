/** @file HouaissDecryptor.cpp

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
#include "..\Common\Files.h"
#include "HouaissDecryptor.h"

#include <windows.h>
#include <Shlwapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>

#pragma comment(lib, "shlwapi.lib")

using namespace std;


void HouaissDecrypt(ostream& os, istream& is)
{
	char c;

	while (is.get(c))
	{
		c = c + 0x0B;
		os.put(c);
	}
}


bool HouaissDecrypt()
{
	string filesPath;

	if ( GetConfigString("Houaiss", "ArquivosDoDicionario", filesPath) )
	{
		vector<string> files;

		if (FindFiles(filesPath + "\\*.dhx", files))
		{
			if (CreateFolder("C:\\ConversorDeDicionario\\Dicionarios\\Houaiss"))
			{
				for (string file : files)
				{
					string fullPathIn = filesPath + "\\" + file;
					string fullPathOut = "C:\\ConversorDeDicionario\\Dicionarios\\Houaiss\\" + file + ".txt";

					ifstream ifs(fullPathIn);
					ofstream ofs(fullPathOut);

					if (ifs && ofs)
					{
						HouaissDecrypt(ofs, ifs);
					}
				}
			}
		}
	}

	return true;
}


int main()
{
	HouaissDecrypt();
}
