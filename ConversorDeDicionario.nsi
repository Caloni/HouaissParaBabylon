!define PRODUCT_NAME "ConversorDeDicionario"
!define PRODUCT_VERSION "2.0"
!define PRODUCT_PUBLISHER "BitForge"
!define CONFIGURATION "Debug"

!include "x64.nsh"

SetCompressor lzma
SetCompress off

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "ConversorDeDicionario-v${PRODUCT_VERSION}.exe"
InstallDir C:\ConversorDeDicionario

InstallDirRegKey HKLM "Software\BitForge\ConversorDeDicionario\nsis" "Install_Dir"

!include "MUI.nsh"

; MUI Settings
!define MUI_ABOUTWARNING
!define MUI_ICON "installer.ico"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

UninstPage uninstConfirm
UninstPage instfiles


; Language files 
!insertmacro MUI_LANGUAGE "PortugueseBR"

; MUI End

Section -SETTINGS
	SetOutPath "$INSTDIR"
	SetOverwrite ifnewer
SectionEnd

Section "Houaiss" SEC01
	SectionIn RO

	SetOutPath "$INSTDIR\Binarios"
	File "${CONFIGURATION}\HouaissFinder.exe"
	File "${CONFIGURATION}\HouaissDecryptor.exe"
	ExecWait "$INSTDIR\HouaissFinder.exe"
	ExecWait "$INSTDIR\HouaissDecryptor.exe"
	
SectionEnd

