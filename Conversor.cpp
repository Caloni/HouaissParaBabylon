/** @file Conversor.cpp

@brief Conversor de Houaiss para Babylon.

@author Wanderley Caloni <wanderley@caloni.com.br>
@date 2008-04

@desc Permite que os arquivos do Houaiss sejam usados para construir
um dicionário do Babylon através do Babylon Builder.

Linque:
Houaiss: http://www.dicionariohouaiss.com.br
Babylon: http://www.babylon.com
Babylon Builder: http://www.babylon.com/display.php?id=15&tree=3&level=2

Obs.:
Esse programa não se destina à pirataria. Ele não fornece o dicionário, 
que devem ser adquirido legalmente. Sua única função é permitir o uso 
de um ótimo dicionários independente de sua única interface.

Bom proveito!
*/
#define _CRT_SECURE_NO_DEPRECATE

#include "Conversor.h"
#include "Common.h"

#include <windows.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#define HOUAISS_PATH "C:\\Tests\\Dicionario"


using namespace std;


void Format(string& str)
{
	string::size_type pos1 = 0;
	string::size_type pos2 = 0;

	while( (pos1 = str.find('<')) != string::npos )
		str.replace(pos1, 1, "&lt;");

	while( (pos1 = str.find('>')) != string::npos )
		str.replace(pos1, 1, "&gt;");

	while( (pos1 = str.find('{')) != string::npos )
	{
		if( pos1 && str[pos1 - 1] == '\\' ) // caractere de escape
			str.replace(pos1 - 1, 2, "&#123;");
		else
		{
			string subStr;

			pos2 = str.find('}', pos1);

			if( pos2 != string::npos )
				subStr = str.substr(pos1 + 1, pos2 - pos1 - 1);
			else
				subStr = str.substr(pos1 + 1);

			istringstream is(subStr);

			string fmt;
			string word;
			is >> fmt;
			getline(is, word);
			if( ! word.empty() && word[0] == ' ' )
				word.erase(0, 1);

			if( fmt.find("\\i") != string::npos )
				word = "<i>" + word + "</i>";

			if( fmt.find("\\b") != string::npos )
				word = "<b>" + word + "</b>";

			if( fmt.find("\\f20") != string::npos )
				word = "<font style=\"text-transform: uppercase;\">" + word + "</font>";

			if( fmt.find("\\super") != string::npos )
				word = "<font style=\"vertical-align: super;\">" + word + "</font>";

			if( pos2 != string::npos )
				str.replace(pos1, pos2 - pos1 + 1, word);
			else
				str.replace(pos1, pos2, word);
		}
	}
}


CString GetBabylonTempFileName(H2BFiles* files)
{
	CString ret;

	ret = files->workingDir.path + _T("\\")
		+ _T("Hou2Bab.txt");

	return ret;
}


void RemoveHtml(CString& text)
{
	int startTag = text.Find('<');
	int endTag = text.Find('>');

	while( startTag != -1 && endTag != -1 )
	{
		//Log("RemoveHtml Before: \"%s\"", (PCTSTR) text);
		text.Delete(startTag, endTag - startTag + 1);
		//Log("RemoveHtml After: \"%s\"", (PCTSTR) text);

		startTag = text.Find('<');
		endTag = text.Find('>');
	}
}


bool Convert(istream& is, ostream& os, DWORD fileSize, H2BOperation& op)
{
	bool ret = true;
	char cmd;
	string line;
	string word;
	int count = 0;
	unsigned int lastPercent = 0;
	unsigned int percent = 0;
	unsigned int currentOffset = 0;
	unsigned int factor = 2;
	DWORD before = 0, after;
	CString status;

	fileSize /= 2;
	while( fileSize > UINT_MAX / 2 )
	{
		fileSize /= 2;
		factor *= 2;
	}

	while( getline(is, line) )
	{
		cmd = line[0];
		line.erase(0, 1);
		Format(line);

		// até agora: *:VPnCS\-#dvTIRUDLp!c<&0-9rO
		switch( cmd )
		{
		case '*': // verbete
			++count;
			word = line;
			os << '\n' << word << '\n';
			break;

		case ':': // definição
			os << line << "<br>\n";

			// mostra última palavra convertida (e seu significado)
			after = GetTickCount();
			if( (after - before) / 1000 > (DWORD) status.GetLength() / 5 )
			{
				before = after;
				status = word.c_str();
				status += "\n";
				status += line.c_str();
				RemoveHtml(status);

				if( status.GetLength() > 150 )
				{
					status.Delete(250, status.GetLength());
					status += "...";
				}

				op.SetStatusMessageDescription(status);
			}
			break;

		case 'V': // alternativa
		case 'P':
			os << "tb. " << line << "<br>\n";
			break;

		case 'n': // número do verbete
			os << "<font style=\"vertical-align: super;\">" << line << "</font>";
			os << word << "<br>\n";
			break;

		case 'C': // classificação
			if( line != "#IND#" )
				os << "<font color=\"blue\">" << line << "</font><br>\n";
			break;

		case 'S': // classificação
			if( line != "#IND#" )
				os << "<font color=\"blue\">" << line << "</font><br>\n";
			break;

		case '-': // numeração
			os << line << " ";
			break;

		case '#': // locução
			os << "<font color=\"brown\">Locução: " << line << "</font><br>\n";
			break;

		case 'd': // datação
			os << "<font color=\"brown\">Datação: " << line << "</font><br>\n";
			break;

		case 'v': // variação
			os << "<font color=\"brown\">" << line << "</font><br>\n";
			break;

		case 'T': // rubrica
			os << "<font color=\"brown\">Rubrica: " << line << "</font><br>\n";
			break;

		case 'I': // diacronismo
			os << "<font color=\"brown\">Diacronismo: " << line << "</font><br>\n";
			break;

		case 'R': // regionalismo
		case 'r':
			os << "<font color=\"brown\">Regionalismo: " << line << "</font><br>\n";
			break;

		case 'U': // uso
			os << "<font color=\"brown\">Uso: " << line << "</font><br>\n";
			break;

		case 'D': // derivação
			os << "<font color=\"brown\">Derivação: " << line << "</font><br>\n";
			break;

		case 'L': // língua
			os << "<font color=\"brown\">Língua: " << line << "</font><br>\n";
			break;

		case 'p': // pronúncia
			os << "<font color=\"brown\">Pronúncia: " << line << "</font><br>\n";
			break;

		case '!': // observação
		case 'c':
			os << "<font color=\"purple\">Obs.: " << line << "</font><br>\n";
			break;

		case '<': // exemplo
			os << "<font color=\"green\">Ex.: " << line << "</font><br>\n";
			break;

		case 'O': // noção
			os << "<font color=\"blue\">Noção</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '&': // etimologia
			os << "<font color=\"blue\">Etimologia</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '0': // gramática
			os << "<font color=\"blue\">Gramática</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '1': // gramática e uso
			os << "<font color=\"blue\">Gramática e uso</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '2': // uso
			os << "<font color=\"blue\">Uso</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '3': // sinônimos/variantes
			os << "<font color=\"blue\">Sinônimos/Variantes</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '4': // antônimos
			os << "<font color=\"blue\">Antônimos</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '5': // coletivos
			os << "<font color=\"blue\">Coletivos</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '6': // homônimos
			os << "<font color=\"blue\">Homônimos</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '7': // parônimos
			os << "<font color=\"blue\">Parônimos</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '8': // desconhecido
			os << "<font color=\"blue\">Informação adicional 8</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		case '9': // vozes de animais
			os << "<font color=\"blue\">Vozes de Animais</font><br>\n";
			os << "<font color=\"gray\">" << line << "</font><br>\n";
			break;

		default:
			 //cerr << "Unknown command: " << line << '\n';
			break;
		}

		currentOffset += line.size();
		percent = ( currentOffset / factor ) * 100 / fileSize;

		if( op.Cancel() )
		{
			ret = false;
			SetLastError(ERROR_CANCELLED);
			break;
		}

		if( percent != lastPercent )
			op.SetOperationStatus(lastPercent = percent);
	}

	return ret;
}


DWORD Conversor(H2BFiles* files, H2BOperation& op)
{
	DWORD ret = ERROR_SUCCESS;
	CString babylonTempFileName = GetBabylonTempFileName(files);

	Log("Babylon temp file: %s", (PCTSTR) babylonTempFileName );

	if( ! babylonTempFileName.IsEmpty() )
	{
		HANDLE babylonTempFile = CreateFile(babylonTempFileName, 
			GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, CREATE_ALWAYS, 0, NULL);

		if( babylonTempFile != INVALID_HANDLE_VALUE )
		{
				ifstream inputFile(files->houOutputFile.path);
				fstream outputFile;
				
				outputFile.open(babylonTempFileName);

				if( ! inputFile )
					ret = ERROR_ACCESS_DENIED;

				if( ! outputFile )
					ret = ERROR_ACCESS_DENIED;

				if( inputFile && outputFile )
				{
					DWORD fileSize = GetFileSize(files->houOutputFile.handle, NULL);
					if( ! Convert(inputFile, outputFile, fileSize, op) )
						ret = GetLastError();
				}
				else 
				{
					ret = ERROR_ACCESS_DENIED;
					Log("Unable to open babylon input/output files");
				}
		}
		else 
		{
			ret = GetLastError();
			Log("Error %d opening babylon temp file", ret);
		}

		files->babInputFile.path = babylonTempFileName;
		files->babInputFile.handle.Attach(babylonTempFile);
	}
	else ret = GetLastError();

	return ret;
}
