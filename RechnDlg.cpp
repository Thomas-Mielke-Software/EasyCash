// RechnDlg.cpp : implementation file
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2020  Thomas Mielke
// 
// Dies ist freie Software; Sie dürfen sie unter den Bedingungen der 
// GNU General Public License, wie von der Free Software Foundation 
// veröffentlicht, weiterverteilen und/oder modifizieren; entweder gemäß 
// Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA. 

#include "stdafx.h"
#include "easycash.h"
#include "RechnDlg.h"
#include "ECTIFace\EasyCashDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRechnDlg dialog


CRechnDlg::CRechnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRechnDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRechnDlg)
	//}}AFX_DATA_INIT
}


void CRechnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRechnDlg)
	DDX_Control(pDX, IDC_FORMEL10, m_f10);
	DDX_Control(pDX, IDC_FORMEL9, m_f9);
	DDX_Control(pDX, IDC_FORMEL8, m_f8);
	DDX_Control(pDX, IDC_FORMEL7, m_f7);
	DDX_Control(pDX, IDC_FORMEL6, m_f6);
	DDX_Control(pDX, IDC_FORMEL5, m_f5);
	DDX_Control(pDX, IDC_FORMEL4, m_f4);
	DDX_Control(pDX, IDC_FORMEL3, m_f3);
	DDX_Control(pDX, IDC_FORMEL2, m_f2);
	DDX_Control(pDX, IDC_FORMEL1, m_f1);
	DDX_Control(pDX, IDC_UEBERNEHMEN, m_uebernehmen);
	DDX_Control(pDX, IDC_BERECHNEN, m_berechnen);
	DDX_Control(pDX, IDC_EINGABE, m_eingabe);
	DDX_Control(pDX, IDC_VARIABLEN_LBOX, m_variablen);
	DDX_Control(pDX, IDC_ERGEBNISSE_LBOX, m_ergebnisse);
	DDX_Control(pDX, IDC_FUNKTIONEN_LBOX, m_funktionen);
	DDX_Control(pDX, IDC_ERGEBNIS, m_ergebnis);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRechnDlg, CDialog)
	//{{AFX_MSG_MAP(CRechnDlg)
	ON_LBN_DBLCLK(IDC_ERGEBNISSE_LBOX, OnDblclkErgebnisseLbox)
	ON_LBN_DBLCLK(IDC_FUNKTIONEN_LBOX, OnDblclkFunktionenLbox)
	ON_LBN_DBLCLK(IDC_VARIABLEN_LBOX, OnDblclkVariablenLbox)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BERECHNEN, OnBerechnen)
	ON_BN_CLICKED(IDC_UEBERNEHMEN, OnUebernehmen)
	ON_BN_CLICKED(IDC_FORMEL1, OnFormel1)
	ON_BN_CLICKED(IDC_FORMEL2, OnFormel2)
	ON_BN_CLICKED(IDC_FORMEL3, OnFormel3)
	ON_BN_CLICKED(IDC_FORMEL4, OnFormel4)
	ON_BN_CLICKED(IDC_FORMEL5, OnFormel5)
	ON_BN_CLICKED(IDC_FORMEL6, OnFormel6)
	ON_BN_CLICKED(IDC_FORMEL7, OnFormel7)
	ON_BN_CLICKED(IDC_FORMEL8, OnFormel8)
	ON_BN_CLICKED(IDC_FORMEL9, OnFormel9)
	ON_BN_CLICKED(IDC_FORMEL10, OnFormel10)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRechnDlg message handlers

BOOL CRechnDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    // init the listboxes' contents
    //this->UpdateAnswersList();
    //this->UpdateVariablesList();
    //this->UpdateFunctionsList();

	char buffer[1000], buffer2[1000];
	char EasyCashIniFilenameBuffer[1000];
	if (!GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); TRUE; }

	GetPrivateProfileString("Formeln", "1", "betrag*(100+mwst)/100", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&1. "); strcat(buffer2, buffer); m_f1.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "2", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&2. "); strcat(buffer2, buffer); m_f2.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "3", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&3. "); strcat(buffer2, buffer); m_f3.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "4", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&4. "); strcat(buffer2, buffer); m_f4.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "5", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&5. "); strcat(buffer2, buffer); m_f5.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "6", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&6. "); strcat(buffer2, buffer); m_f6.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "7", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&7. "); strcat(buffer2, buffer); m_f7.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "8", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&8. "); strcat(buffer2, buffer); m_f8.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "9", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&9. "); strcat(buffer2, buffer); m_f9.SetWindowText(buffer2);
	GetPrivateProfileString("Formeln", "10", "<Formel nicht belegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	strcpy(buffer2, "&10. "); strcat(buffer2, buffer); m_f10.SetWindowText(buffer2);

	SetTimer(1, 1, NULL);

	return TRUE;  
}

void CRechnDlg::OnBerechnen() 
{
    CString strSource, strDest;
    m_eingabe.GetWindowText(strSource);
	strSource.Replace(',', '.');

    // Initializes and calls the Parser
    try {
        VALUES_TYPE valResult = m_Parser.Evaluate((LPCTSTR)strSource);
        strDest.Format("%.16f", valResult);
        if (m_Parser.HasWarning()) {
			CString strDestCopy = strDest;
            // Adds the warning after the result
            strDest.Format("%s ; %s", strDestCopy, 
                m_Parser.GetWarningMsg().c_str());
        }
        m_eingabe.SetSel(0, -1);
    }
    catch (CSyntaxException ex) {
        strDest.Format("Syntax error %d : %s", 
            ex.GetExceptionNumber(), ex.GetMessage().c_str());
        m_eingabe.SetSel(ex.GetErrorPos(), ex.GetErrorPos());
    }
    catch (CMathematicException ex) {
        strDest.Format("Mathematic error %d : %s", 
            ex.GetExceptionNumber(), ex.GetMessage().c_str());
        m_eingabe.SetSel(ex.GetErrorPos(), ex.GetErrorPos());
    }
    catch (CFunctionException ex) {
        strDest.Format("Function error %d : %s", 
            ex.GetExceptionNumber(), ex.GetMessage().c_str());
        m_eingabe.SetSel(ex.GetErrorPos(), ex.GetErrorPos());
    }
    catch (CParameterException ex) {
        strDest.Format("Parameter error %d : %s", 
            ex.GetExceptionNumber(), ex.GetMessage().c_str());
        m_eingabe.SetSel(ex.GetErrorPos(), ex.GetErrorPos());
    }
    catch (CVariableException ex) {
        strDest.Format("Variable error %d : %s", 
            ex.GetExceptionNumber(), ex.GetMessage().c_str());
        m_eingabe.SetSel(ex.GetErrorPos(), ex.GetErrorPos());
    }
    catch (CDomainException ex) {
        strDest.Format("Domain error %d : %s", 
            ex.GetExceptionNumber(), ex.GetMessage().c_str());
        m_eingabe.SetSel(ex.GetErrorPos(), ex.GetErrorPos());
    }
    catch (CParserException ex) {
        strDest.Format("Parser error %d : %s", 
            ex.GetExceptionNumber(), ex.GetMessage().c_str());
        m_eingabe.SetSel(ex.GetErrorPos(), ex.GetErrorPos());
    }
    catch (...) {
        strDest.Format("Unknown parser internal error");
    }

    // Prints the result...
    m_ergebnis.SetWindowText(strDest);

    // Updates the listboxes' contents
    this->UpdateAnswersList();
    this->UpdateVariablesList();
    this->UpdateFunctionsList();

    // Give the focus back to the input EditBox
    m_eingabe.SetFocus();
}

void CRechnDlg::OnUebernehmen() 
{
    CString strSource;
    m_ergebnis.GetWindowText(strSource);
	if (strSource.IsEmpty()) 
	{
		m_eingabe.GetWindowText(strSource);
		if (strSource.IsEmpty()) 
		{
			AfxMessageBox("Zuerst muss mit Hilfe einer Formel ein Ergebnis berechnet werden.");
			m_eingabe.SetFocus();
			return;
		}
		else
		{
			OnBerechnen();
			m_ergebnis.GetWindowText(strSource);
		}
	}
	m_betrag_neu = atof(strSource);

	CDialog::OnOK();
}


// Updates the content of the Answers History ListBox
void CRechnDlg::UpdateAnswersList() {
	// Clears the Answers and Formulas ListBoxes entries
	int iNbAnswersToDelete = m_ergebnisse.GetCount();
	int iCpt;
	for (iCpt = 0; iCpt < iNbAnswersToDelete; iCpt++) {
		// Both ListBoxes contain the same number of lines
	   m_ergebnisse.DeleteString(0);
//	   m_plFormulas->DeleteString(0);
	}

	// Retrieves the new parser answers history
	const std::deque<AnswerItem> dqeAnswersHistory = m_Parser.GetAnswersHistory();
	std::deque<AnswerItem>::const_iterator iter;
	char pszHistoryEntry[100];
	for (iter = dqeAnswersHistory.begin(); iter != dqeAnswersHistory.end(); iter++) {
		::sprintf(pszHistoryEntry, "%.16f", iter->m_valResult);
		m_ergebnisse.AddString(pszHistoryEntry);
//		m_plFormulas->AddString(iter->m_strFormula.c_str());
	}
}


// Updates the content of the Variables ListBox
void CRechnDlg::UpdateVariablesList() {
	// Clears the ListBox entries
	int iNbVariablesToDelete = m_variablen.GetCount();
	int iCpt;
	for (iCpt = 0; iCpt < iNbVariablesToDelete; iCpt++) {
	   m_variablen.DeleteString(0);
	}

	// Retrieves the new parser variables list
	const std::map<std::string, VALUES_TYPE> mapVariables = m_Parser.GetVariables();
	std::map<std::string, VALUES_TYPE>::const_iterator iter;
	for (iter = mapVariables.begin(); iter != mapVariables.end(); iter++) {
		m_variablen.AddString(iter->first.c_str());
	}
}


// Updates the content of the Functions ListBox
void CRechnDlg::UpdateFunctionsList() {
	// Clears the ListBox entries
	int iNbFunctionsToDelete = m_funktionen.GetCount();
	int iCpt;
	for (iCpt = 0; iCpt < iNbFunctionsToDelete; iCpt++) {
	   m_funktionen.DeleteString(0);
	}

	// Retrieves the new parser functions list
	const std::list<std::string> lstFunctions = m_Parser.GetFunctions();
	std::list<std::string>::const_iterator iter;
	for (iter = lstFunctions.begin(); iter != lstFunctions.end(); iter++) {
		m_funktionen.AddString(iter->c_str());
	}
}


void CRechnDlg::OnDblclkErgebnisseLbox() 
{
	CString strAnswer;
	m_ergebnisse.GetText(m_ergebnisse.GetCurSel(), strAnswer);
	m_eingabe.ReplaceSel(strAnswer);
	m_eingabe.SetFocus();
}

void CRechnDlg::OnDblclkFunktionenLbox() 
{
	CString strFunction;
	m_funktionen.GetText(m_funktionen.GetCurSel(), strFunction);
	m_eingabe.ReplaceSel(strFunction);
	m_eingabe.SetFocus();
}

void CRechnDlg::OnDblclkVariablenLbox() 
{
	CString strVariable;
	m_variablen.GetText(m_variablen.GetCurSel(), strVariable);
	m_eingabe.ReplaceSel(strVariable);
	m_eingabe.SetFocus();
}

void CRechnDlg::OnTimer(UINT nIDEvent) 
{
	KillTimer(1);
	
	// erstmal Variablen einspeisen
	CString strSource, strDest;
	strSource.Format("betrag=%g", m_betrag);
	m_eingabe.SetWindowText(strSource);
	OnBerechnen();

	strSource.Format("mwst=%g", m_mwst);
	m_eingabe.SetWindowText(strSource);
	OnBerechnen();

	// wieder alles löschen, nur die Variablen bleiben in der Listbox erhalten
	m_eingabe.SetWindowText("");
	m_ergebnis.SetWindowText("");
	
	// Clears the Parser answers history
	m_Parser.ResetAnswersHistory();
	this->UpdateAnswersList();

	CDialog::OnTimer(nIDEvent);
}

void CRechnDlg::OnFormel(int n, CButton *pbutton)
{
    CString strSource;
    m_eingabe.GetWindowText(strSource);
	if (strSource.IsEmpty())
	{
		pbutton->GetWindowText(strSource);
		strSource = strSource.Mid(4);
		strSource.TrimLeft();
		if (strSource != "<Formel nicht belegt>")
		{
			m_eingabe.SetWindowText(strSource);
			OnUebernehmen();
		}
	}
	else
	{
		m_eingabe.GetWindowText(strSource);
		CString strPrefix;
		strPrefix.Format("&%d. ", n);
		pbutton->SetWindowText(strPrefix + strSource);

		char EasyCashIniFilenameBuffer[1000];
		if (!GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht zum Schreiben der Formel öffnen"); }		
		strPrefix.Format("%d", n);
		WritePrivateProfileString("Formeln", (LPCSTR)strPrefix, (LPCSTR)strSource, EasyCashIniFilenameBuffer);
		
		m_eingabe.SetFocus();
	}
}

void CRechnDlg::OnFormel1() 
{
	OnFormel(1, &m_f1);
}

void CRechnDlg::OnFormel2() 
{
	OnFormel(2, &m_f2);
}

void CRechnDlg::OnFormel3() 
{
	OnFormel(3, &m_f3);
}

void CRechnDlg::OnFormel4() 
{
	OnFormel(4, &m_f4);
}

void CRechnDlg::OnFormel5() 
{
	OnFormel(5, &m_f5);
}

void CRechnDlg::OnFormel6() 
{
	OnFormel(6, &m_f6);
}

void CRechnDlg::OnFormel7() 
{
	OnFormel(7, &m_f7);	
}

void CRechnDlg::OnFormel8() 
{
	OnFormel(8, &m_f8);	
}

void CRechnDlg::OnFormel9() 
{
	OnFormel(9, &m_f9);
}

void CRechnDlg::OnFormel10() 
{
	OnFormel(10, &m_f10);
}
