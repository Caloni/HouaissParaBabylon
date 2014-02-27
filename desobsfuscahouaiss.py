#!/usr/bin/python2
# -*- coding: utf-8 -*-

#
# Coloque esse script na pasta com os arquivos dhx.
# O resultado estará em iso-8859-1
#

#
# Segui o tutorial em http://www.caloni.com.br/blog/archives/conversor-de-houaiss-para-babylon-parte-1
#

import os

files = os.listdir('.')

for arq in files:
    if not arq.endswith('dhx'):
        continue

    print 'Abrindo "%s"' % arq
    origin = open(arq, 'r')
    target = open('%s.txt' % arq, 'w+')

    char = origin.read(1)
    while char:
        byte = ord(char) + 0x0B
        new_char = chr(byte % 256)
        target.write(new_char)
        char = origin.read(1)

    origin.close()
    target.close()
