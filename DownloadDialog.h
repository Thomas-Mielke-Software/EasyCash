// NewDownloadDialog.h : header file
//

//#define FTP_DOWNLOAD_SERVER "168.100.0.2"

#define FTP_DOWNLOAD_SERVER "scr957.no-ip.org"
#define FTP_DOWNLOAD_PATH   "/scr957ud/"


#define FTP_UPDATE_PATH   "/update/"
#define FTP_UPDATE_VERSION_FILENAME   "version.txt"

class CThreadParam : public CObject
{
public:
	DECLARE_DYNAMIC( CThreadParam )
	BOOL bOnlyUpdateCheck;
	char Server[1000];
	char FTP_Path[1000];
	char local_Path[1000];
	char Filename[1000];
	int  trans_bytes;
	int  total_bytes;
	char *status_text;
	BOOL cancel;	// Kommando an Thread
	BOOL canceled;	// Bestätigung von Thread
	BOOL check;		// Check for update?
};

/////////////////////////////////////////////////////////////////////////////
// CDownloadDialog dialog

class CDownloadDialog : public CDialog
{
// Construction
public:
	CDownloadDialog(INFOSTRUCT *info, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDownloadDialog)
	enum { IDD = IDD_NEW_DOWNLOAD_DIALOG };
	BOOL	m_check;
	//}}AFX_DATA

	void UpdatePercent(int nNewPos);
	int SetPos();
	void PumpMessages();
	void BuildComboBoxList();
	BOOL CheckForUpdate();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDownloadDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// zur Kalkulation der Restzeit:
#define  ETA_INTERVALL 500	// Timer-Intervall in Millisekunden
#define  ETA_ARRAY_SIZE 30
	int  eta_array[ETA_ARRAY_SIZE];
	int  eta_n; // Anzahl gespeicherter Elemente im Array

	INFOSTRUCT m_info;
	CString m_URL;
	CWinThread *m_thread;
	CThreadParam* m_pThreadParam;
    int m_nLower;
    int m_nUpper;
	BOOL bNotYetDownload;
	BOOL bClipDownloadSuccess;

	// Generated message map functions
	//{{AFX_MSG(CDownloadDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
