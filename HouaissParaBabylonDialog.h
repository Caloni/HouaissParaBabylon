#pragma once
#include "Common.h"
#include "Controller.h"

#define _WTL_USE_CSTRING
#include <atlbase.h> // classes base da ATL
#include <atlapp.h> // classes base da WTL

extern CAppModule _Module; // versão WTL do CComModule
#include <atlwin.h> // classes GUI da ATL
#include <atlframe.h> // classes de janelas frame da WTL
#include <atlmisc.h> // classes utilitárias da WTL (e.g. CString)
#include <atlcrack.h> // macros de mensagens melhoradas da WTL


// Alocação de memória no ATL 7.1.
//#pragma comment(lib, "atlthunk.lib")

// Não se esqueça disso se você for usar controles comuns!
#pragma comment(lib, "comctl32.lib")


class HouaissParaBabylonDialog : public CDialogImpl<HouaissParaBabylonDialog>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(HouaissParaBabylonDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDC_BTN_START, OnConvert)
		COMMAND_ID_HANDLER(IDC_BTN_INSTALL, OnInstall)
		COMMAND_ID_HANDLER(IDC_BTN_ABOUT, OnAbout)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		InitialState();
		CenterWindow();
		return TRUE;    // deixando o sistema cuidar do foco
	}

	void OnClose()
	{
		if( m_converting )
			m_cancelling = true;
		else
			EndDialog(IDCANCEL);
	}

	LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ShellExecute(m_hWnd, _T("open"), _T("http://www.caloni.com.br/blog/archives/houaissparababylon-versao-beta"), NULL, NULL, SW_SHOWMAXIMIZED);
		return 0;
	}

	void InitialState()
	{
		m_files = 0;
		m_converting = false;
		m_cancelling = false;
		::SetFocus(GetDlgItem(IDC_BTN_START));
		::SetWindowText(GetDlgItem(IDC_BTN_INSTALL), _T("&Cancelar"));
		::SetWindowText(GetDlgItem(IDC_STC_COPYRIGHT), 
			_T("Houaiss e Babylon são marcas registradas de seus respectivos ")
			_T("autores. Esse programa não se responsabiliza por possíveis atos ")
			_T("ilegais de pirataria e cópia ilegal de software, assim como ")
			_T("quaisquer danos provenientes do (mau) uso dessa ferramenta."));
	}

	bool Cancel()
	{
		bool ret = false;

		if( m_cancelling )
		{
			UINT cancel = MessageBox(_T("Deseja cancelar a operação?"), 
				PROJECT_NAME, MB_YESNO | MB_ICONINFORMATION);

			if( cancel == IDYES )
			{
				ret = true;
				SetLastError(ERROR_CANCELLED);
			}
			else
				m_cancelling = false;
		}

		return ret;
	}

	LRESULT OnConvert(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		UINT resp = MessageBox(_T("Você afirma ser possuidor de uma licença dos ")
			_T("produtos Dicionário Houaiss e Babylon instalados em seu computador?"),
			PROJECT_NAME, MB_YESNO | MB_ICONWARNING);

		if( resp == IDYES )
		{
			::EnableWindow(GetDlgItem(IDC_BTN_START), FALSE);

			DWORD err = StartConversion(this);

			if( err != ERROR_SUCCESS )
			{
				Message(_T("Erro tentando iniciar operação."), MB_ICONERROR);
				::EnableWindow(GetDlgItem(IDC_BTN_START), FALSE);
			}
			else
				m_converting = true;
		}
		else OnClose();

		return 0;
	}

	LRESULT OnInstall(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DWORD ret = ERROR_INVALID_FUNCTION;

		if( m_converting || ! m_files )
			OnClose();
		else
			ret = StartInstallation(this, m_files);

		return 0;
	}

	void ProgressSetRange(int min, int max)
	{
		SendDlgItemMessage(IDC_PGB_OPERATION, PBM_SETRANGE, 
			0, MAKELPARAM(min, max));
		SendDlgItemMessage(IDC_PGB_OPERATION, PBM_SETSTEP, 1);
	}

	void ProgressSetValue(int curr)
	{
		SendDlgItemMessage(IDC_PGB_OPERATION, PBM_SETPOS, curr);
	}

	void Status(PCTSTR statusMessage)
	{
		SendDlgItemMessage(IDC_SBR_STATUS, SB_SETTEXT, 
			SB_SIMPLEID, (LPARAM) statusMessage);
	}

	void StatusDescription(PCTSTR statusMessage)
	{
		::SetWindowText(GetDlgItem(IDC_STC_COPYRIGHT), statusMessage);
	}

	void Message(PCTSTR message, UINT icon)
	{
		MessageBox(message, PROJECT_NAME, icon);
	}

	void FinishConversion(H2BFiles* files, DWORD err)
	{
		InitialState();

		if( err == ERROR_SUCCESS )
		{
			::SetWindowText(GetDlgItem(IDC_BTN_INSTALL), _T("&Instalar Dicionário"));
			m_files = files;
		}
		else
		{
			::EnableWindow(GetDlgItem(IDC_BTN_START), TRUE);
			m_files = 0;
		}

		SendDlgItemMessage(IDC_PGB_OPERATION, PBM_SETPOS);
	}

	void FinishInstallation(DWORD err)
	{
		::EnableWindow(GetDlgItem(IDC_BTN_START), TRUE);
		InitialState();

		if( err == ERROR_SUCCESS )
			EndDialog(ERROR_SUCCESS);
	}

private:
	H2BFiles* m_files;
	bool m_converting;
	bool m_cancelling;
};
