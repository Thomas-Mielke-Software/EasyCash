// CWebUpdate.h : header file
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

#ifndef _CWEBUPDATE_H
#define _CWEBUPDATE_H

/////////////////////////////////////////////////////////////////////////////
// CWebUpdate definitions

class CWebUpdate
{
// Construction
public:
	CWebUpdate();   // Standard constructor

	// Base directory, specify true to use the exe's path
	void SetLocalDirectory(LPCSTR pathTo, bool generate);
	CString GetLocalDirectory() { return localDir; }

	// Remote URLs/dirs
	void SetUpdateFileURL(LPCSTR url) { updateURL = url; }
	void SetRemoteURL(LPCSTR url) { remoteURL = url; }
	CString GetRemoteURL() { return remoteURL; }
	CString GetUpdateFileURL() { return updateURL; }

	// Give back some information
	int GetNumberSuccessful() { return numSuccess; }
	int GetNumberDifferent() { return numDifferent; }
	int GetNumberMissing() { return numMissing; }
	int GetNumberAvailable() { return numAvailable; }
	CString GetSuccessfulAt(int i) { return successfulFiles.GetAt(i); }
	CString GetDifferentAt(int i) { return differentFiles.GetAt(i); }
	CString GetMissingAt(int i) { return missingFiles.GetAt(i); }
	CString GetAvailableAt(int i) { return availableFiles.GetAt(i); }
	CString GetAvailableNameAt(int i) { return availableFilesName.GetAt(i); }
	CString GetAvailableSizeAt(int i) { return availableFilesSize.GetAt(i); }
	CString GetAvailableActionAt(int i) { return availableFilesAction.GetAt(i); }
	CString GetAvailableNoteAt(int i) { return availableFilesNote.GetAt(i); }

	// Download new, missing, and available files
	bool DownloadMissing(int i);
	bool DownloadDifferent(int i);
	bool DownloadAvailable(int i);

	// Do the update
	bool DoUpdateCheck();

// Implementation
protected:
	CString    remoteURL;
	CString    updateURL;
	CString    localDir;
	CString    lastError;

	CStringArray    successfulFiles;
	int             numSuccess;
	CStringArray    differentFiles;
	int             numDifferent;
	CStringArray    missingFiles;
	int             numMissing;
	CStringArray    availableFiles;
	int				numAvailable;
	CStringArray    availableFilesName;
	CStringArray    availableFilesSize;
	CStringArray    availableFilesAction;
	CStringArray    availableFilesNote;

private:
	CString DoSHA1Hash(LPCSTR filePath);
};

#endif
