// IconAuswahlBestandskonto.cpp : implementation file
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
#include "IconAuswahlBestandskonto.h"
#include "AnfangssaldoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlBestandskonto dialog


CIconAuswahlBestandskonto::CIconAuswahlBestandskonto(CEasyCashView* pParent /*=NULL*/)
	: CIconAuswahl(IDD_ICONAUSWAHL_BESTANDSKONTO, pParent)
{
	//{{AFX_DATA_INIT(CIconAuswahlBestandskonto)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CIconAuswahlBestandskonto::DoDataExchange(CDataExchange* pDX)
{
	CIconAuswahl::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIconAuswahlBestandskonto)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIconAuswahlBestandskonto, CIconAuswahl)
	//{{AFX_MSG_MAP(CIconAuswahlBestandskonto)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CIconAuswahlBestandskonto::InitDialog()
{
	if (m_nModus == 1) // Auswahl-Modus mit der Option zu ändern
	{
		SetDlgItemText(IDOK, "Sel. anzeigen");
		SetDlgItemText(IDCANCEL, "Alle anzeigen");
	}
	CIconAuswahl::InitDialog();
}

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlBestandskonto message handlers



/////////////////////////////////////////////////////////////////////////////
// CIconAuswahlMandant virtuals

static IconInfo Icons[] = {
	// Allgemein
	"cash",
	"bar",
	"Kasse",
	// Banken
	"SozialBank",
	"Citibank",
	"comdirect",
	"Commerzbank",
	"apoBank (Deutschland)",
	"Deutsche Bank",
	"DAB (Legacy)",
	"Dresdner Bank (Legacy)",
	"Volks- und Raiffeisenbanken",
	"HypoVereinsbank / Bank Austria",
	"ING-DiBa",
	"NetBank (Legacy)",
	"NIBC Bank",
	"Noa Bank (Legacy)",
	"norisbank",
	"Postbank",
	"PSD-Bank",
	"Santander Consumer Bank",
	"SEB",
	"Sparda-Bank (Deutschland)",
	"Raiffeisen (Österreich)",
	"Volksbank",
	"Sparkasse",
	"BAWAG P.S.K.",
	"Credit Suisse",
	"WIR Bank",
	"Coop Bank",
	"UBS",
	// Karten
	"MasterCard",
	"VISA",
	// Paypal
	"PayPal",
	"PayPal blau",
	"PayPal grün",
	"PayPal rot",
	"PayPal orange",
	"PayPal pink",
	"PayPal grau",
	// Allgemein
	"Sparschwein",
	"Aktenordner",
	"Schatulle",
	"Kreditkarte",
	"Geschäft",
	"Privat",
	"Tastatur",
	"Abacus",
	"Onlinebanking",
	// 36 neue Icons von Herbert Held
	"GEFA Bank",
	"Barclays Bank",
	"1822direkt",
	"DenizBank",
	"Creditplus Bank",
	"AgrarB@nk",
	"Ethikbank",
	"DKB",
	"TARGOBANK",
	"Wüstenrot",
	"Deutsche Skatbank",
	"Ikano Bank",
	"Fidor (Legacy)",
	"Rabobank",
	"Moneyou",
	"GarantiBank",
	"OYAK ANKER Bank",
	"Audi Bank",
	"Volkswagen Bank",
	"BMW Bank",
	"IWBank",
	"Ziraat Bank",
	"VTB",
	"Bank of Scotland",
	"GE Capital",
	"ING-DiBa (v2)",
	"PrivatBank",
	"SWK Bank",
	"Triodos Bank",
	"Bank11",
	"Credit Europe Bank",
	"Merkur Privatbank",
	"Degussa Bank",
	"Umweltbank",
	"DHB Bank",
	"Consors Finanz / BNP Paribas",
	"Cronbank",
	"smava",
	"ABK",
	"Naspa",
	"NordFinanz Bank (Legacy)",
	"National-Bank",
	"KD-Bank",
	"Bank 1 Saar",
	"levoBank",
	"Haspa",
	"Bankhaus August Lenz (Legacy)",
	"Bankhaus Gebr. Martin",
	"MKB",
	"BBBank",
	"GLS Bank",
	"Berliner Bank (Legacy)",
	"Aktivbank",
	"ICICI Bank",
	"IKB",
	"MLP",
	"American Express",
	// Bitcoin und Kryptowährungen
	"Bitcoin",
	"Lightning",
	"BTCPay Server",
	"Bitcart",
	"Lipa",
	"Opago",
	"Monero",
	"Digitalwährung",
	// FinTechs
	"Wise",
	"Revolut",
	"Paypal (Icon)",
	"Holvi",
	"N26",
	"Tomorrow",
	"SumUp",
	"Alipay",
	"myPOS",
	"Neteller",
	"Western Union",
	"Payoneer",
	"Skrill",
	"Strike",
	"Wirex",
	"Glint",
	"Billie",
	// Karten
	"Diners Club",
	"Discover",
	"JCB",
	"Maestro",
	"UnionPay",
	"V PAY",
	// Neue Banken Österreich
	"Österr. Ärzte- u. Apothekerbank",
	"Addiko Bank",
	"Alpen Privatbank",
	"Anadi Bank",
	"Bank 99",
	"Bank Burgenland",
	"Bank Gutmann",
	"Bank Winter",
	"Schelhammer Capital Bank",
	"Bankhaus Spängler",
	"BKS Bank",
	"BTV Vier Länder Bank",
	"DADAT Bank",
	"easybank",
	"CA Autobank",
	"HYPO-Gruppe",
	"Kathrein Privatbank",
	"Kommunalkredit Invest",
	"Oberbank",
	"OeKB",
	"Partner Bank",
	"Renault Bank direkt",
	"Sparda-Bank (Österreich)",
	"Erste Bank",
	"VakifBank",
	"WSK Bank",
	// Schweizer Banken
	"Züricher Kantonalbank",
	"Bank Cler",
	"Raiffeisen (Schweiz)",
	// Weitere Banken
	"Consorsbank",
	"Fideuram Direct",
	"WIBank"
};

IconInfo* CIconAuswahlBestandskonto::GetIcons()
{
	return Icons;
}

int CIconAuswahlBestandskonto::GetIconCount()
{
	return sizeof(Icons)/sizeof(Icons[0]);
}

void CIconAuswahlBestandskonto::Neu()
{

}

BOOL CIconAuswahlBestandskonto::ChooseProperty(CString &csProperty)
{
	CAnfangssaldoDlg dlg(this);
	dlg.m_csAnfangssaldo = csProperty;
	if (dlg.DoModal() == IDOK)
	{
		csProperty = dlg.m_csAnfangssaldo;
		return TRUE;
	}
	else
		return FALSE;

}

void CIconAuswahlBestandskonto::Umbenennen(int nElement)
{

}

void CIconAuswahlBestandskonto::Loeschen(int nElement)
{

}

int CIconAuswahlBestandskonto::IconAuswahl()
{
	// Icon wählen
	CIconAuswahlBestandskonto dlgIcon(m_pParent);
	dlgIcon.DoModal();

	return dlgIcon.m_nSelected;
}
