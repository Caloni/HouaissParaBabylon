/** @file Decryptor.h

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
#pragma once
#include "Common.h"
#include <windows.h>

DWORD Decryptor(CString houaissFileName, H2BFiles* files, H2BOperation& op);
