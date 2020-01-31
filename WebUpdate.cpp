// CWebUpdate.cpp : implementation file
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
#include "WebUpdate.h"
#include "Wininet.h"

#include "sha1.h" 

#include <process.h>
#include "atlbase.h"

#include <sys/stat.h>

// Global variables
CString remoteFile;
CString localFile;
HRESULT dloadResult;

// Thread function, downloads a file
//
void downloadFile(void* nullData)
{
	DeleteUrlCacheEntry(remoteFile);
	dloadResult = URLDownloadToFile(NULL, remoteFile, localFile, NULL, NULL);   

	return;
}

/////////////////////////////////////////////////////////////////////////////
// CWebUpdate class



CWebUpdate::CWebUpdate()
{
	// Nothing to see here
}



// Internal function, do the SHA-1 hash
//
CString CWebUpdate::DoSHA1Hash(LPCSTR filePath)
{
	sha1_ctx m_sha1;

	CString tempHash;
	FILE *fileToHash = NULL;

	unsigned char fileBuf[16000];
	unsigned long lenRead = 0;

	// The outputted hash
	CString outHash;

	if (!(fileToHash = fopen(filePath, "rb")))
		return (CString)"";

	// Temporary working buffers
	unsigned char* tempOut = new unsigned char[256];

	sha1_begin(&m_sha1);
	
	do
	{
		lenRead = fread(fileBuf, 1, 16000, fileToHash);
		if(lenRead != 0)
		{
			sha1_hash(fileBuf, lenRead, &m_sha1);
		}
	} while (lenRead == 16000);

	fclose(fileToHash); fileToHash = NULL;

	sha1_end(tempOut, &m_sha1);

	int i;
	for (i = 0 ; i < 20 ; i++)
	{
		char tmp[3];
		_itoa(tempOut[i], tmp, 16);
		if (strlen(tmp) == 1)
		{
			tmp[1] = tmp[0];
			tmp[0] = '0';
			tmp[2] = '\0';
		}
		tempHash += tmp;
	}

	delete[] tempOut;

	outHash = tempHash;

	return outHash;
}


#define CSIDL_COMMON_APPDATA            0x0023

// Set the local directory to download to
//
void CWebUpdate::SetLocalDirectory(LPCSTR pathTo, bool generate)
{
	if (generate)
	{
		char blankStr[MAX_PATH] = "";
		CString path;

		if (!SHGetSpecialFolderPath(AfxGetMainWnd()->GetSafeHwnd(), path.GetBuffer(MAX_PATH), CSIDL_COMMON_APPDATA, TRUE))
		{	// CSIDL_COMMON_APPDATA funktioniert nicht? Dann Programmverzeihnis nehmen!
			char progDir[MAX_PATH] = "";
			GetModuleFileName(0, progDir, sizeof(progDir) - 1);	
			path = blankStr;
			path = path.Left(path.ReverseFind('\\'));		
		}
		else 
		{
			path.ReleaseBuffer();
			path += (CString)"\\EasyCash&Tax\\download";
			DWORD dwAttribs = GetFileAttributes((LPCTSTR)path);
			if (dwAttribs == -1 || !(dwAttribs & FILE_ATTRIBUTE_DIRECTORY))
			{	// download verzeichnis neu anzulegen?	
				CreateDirectory(path.Left(path.ReverseFind('\\')), NULL);	// erst "\\EasyCash&Tax" vor "\\download" anlegen
				CreateDirectory(path, NULL);

				// dann im Programmverzeichnis nachkucken, ob noch alte Donload-Dateien existieren und diese dateien kopieren
				char progDir[MAX_PATH] = "";
				CString progPath;
				GetModuleFileName(0, progDir, sizeof(progDir) - 1);	
				progPath = progDir;
				progPath = progPath.Left(progPath.ReverseFind('\\'));
				progPath += "\\download\\*.*";

				// Dateien kopieren
				CFileFind finder;
				BOOL bWorking = finder.FindFile(progPath);
				while (bWorking)
				{
					bWorking = finder.FindNextFile();
					if (finder.GetFileName() != "." && finder.GetFileName() != "..")
					{
						CString source = finder.GetFilePath();
						CopyFile((LPCTSTR)source, path + "\\" + finder.GetFileName(), TRUE);
					}
				}		
			}
			
			path = path.Left(path.ReverseFind('\\'));
		}


		localDir = path;
	}

	else
	{
		localDir = pathTo;
	}
}



// Perform the actual update
//
bool CWebUpdate::DoUpdateCheck()
{
	// Reset previous attributes
	numMissing = 0;
	numDifferent = 0;
	numSuccess = 0;
	numAvailable = 0;

	missingFiles.RemoveAll();
	differentFiles.RemoveAll();
	successfulFiles.RemoveAll();
	availableFiles.RemoveAll();
	availableFilesName.RemoveAll();
	availableFilesSize.RemoveAll();
	availableFilesAction.RemoveAll();
	availableFilesNote.RemoveAll();

	// First of all, try and retrieve the update file
	remoteFile = updateURL;
	localFile = localDir + "\\" + "TempUpdate.txt";

	// Download
	HANDLE dloadHandle = (HANDLE)_beginthread(downloadFile, 0, (void*)"");

	// Wait for it to finish
	AtlWaitWithMessageLoop(dloadHandle);

	// The download failed, return false
	if (!SUCCEEDED(dloadResult))
		return false;

	// Then the bitmap file
	remoteFile = remoteFile.Left(remoteFile.GetLength()-3) + "png";
	localFile = localDir + "\\" + "update.png";

	// Download
	dloadHandle = (HANDLE)_beginthread(downloadFile, 0, (void*)"");

	// Wait for it to finish
	AtlWaitWithMessageLoop(dloadHandle);

	// The download failed, return false
	if (!SUCCEEDED(dloadResult))
		return false;

	// It downloaded, now we parse it
	// Read it line by line and work out what's what
	CStdioFile loadFile(localDir + "\\" + "TempUpdate.txt", CFile::modeRead | CFile::typeText);
	CString curLine;
	
	while(loadFile.ReadString(curLine))
	{
		CString fileName;
		AfxExtractSubString(fileName, curLine, 0, ' ');

		CString fileTo;
		AfxExtractSubString(fileTo, curLine, 1, ' ');

		CString fileSize;
		AfxExtractSubString(fileSize, curLine, 2, ' ');

		CString fileHash;
		AfxExtractSubString(fileHash, curLine, 3, ' ');

		// So the path = ..
		CString pathTo = localDir + "\\" + fileTo;

		if (fileHash.IsEmpty()) return false;
		if (fileHash[0] == 'v')
		{
			extern char version_string[50];
			if (!strcmp(version_string, (LPCSTR)fileHash))
			{
				numSuccess++;
				successfulFiles.Add(fileTo);
				availableFilesAction.Add("kein Update nötig");
				fileName += (CString)" (" + fileHash + ")";
			}
			else
			{
				numDifferent++;
				differentFiles.Add(fileTo);
				availableFilesAction.Add("Update");
				fileName += (CString)" (" + version_string + " --> " + fileHash + ")";
			}

		}
		else
		{
			struct stat checkFile;
			if(stat(pathTo, &checkFile) == 0)
			{
				// It exists, but is it the same file?
				CString verifyFile = DoSHA1Hash(pathTo);
				if (verifyFile == "")
				{
					AfxMessageBox("Ein unerwarteter Fehler ist beim Überpfüfen der existierenden Plugin-Setupdatei '" + pathTo + "' aufgetreten. Die Datei scheint zu existieren, konnte aber nich gelesen werden. Probieren Sie, EasyCash&Tax mit der Option 'als Administrator ausführen' zu starten (rechte Maustaste).");
					return false;
				}

				verifyFile.MakeUpper();
				fileHash.MakeUpper();

				// Now compare the hases
				if (verifyFile == fileHash)
				{
					// The files are the same, no worries
					numSuccess++;

					successfulFiles.Add(fileTo);
					availableFilesAction.Add("kein Update nötig");
				}

				else
				{
					// The files are different
					numDifferent++;

					differentFiles.Add(fileTo);
					availableFilesAction.Add("Update");
				}
			}

			else
			{
				// The files doesn't exist at all
				numMissing++;

				missingFiles.Add(fileTo);
				availableFilesAction.Add("Neuinstallation");
			}
		}

		availableFiles.Add(fileTo);
		availableFilesName.Add(fileName);
		availableFilesSize.Add(fileSize);
		int i, n;
		for (i = 0, n = 0; i < 10 && n != -1; i++, n++)
			n = curLine.Find(" ", n);
		if (n > 0)
		{
			availableFilesNote.Add(curLine.Mid(n));
			availableFilesNote[numAvailable].Replace("\\n", "\r\n");
		}

		numAvailable++;
	}

	return true;
}



// Download a missing file
//
bool CWebUpdate::DownloadMissing(int i)
{
	// Attempt to download the file
	remoteFile = remoteURL + "/" + missingFiles.GetAt(i);
	localFile = localDir + "\\" + missingFiles.GetAt(i);

	HANDLE dloadHandle = (HANDLE)_beginthread(downloadFile, 0, (void*)"");

	AtlWaitWithMessageLoop(dloadHandle);

	if (!SUCCEEDED(dloadResult))
		return false;

	else
		return true;
}



// Download an updated file
//
bool CWebUpdate::DownloadDifferent(int i)
{
	// Attempt to download the file
	remoteFile = remoteURL + "/" + differentFiles.GetAt(i);
	localFile = localDir + "\\" + differentFiles.GetAt(i);

	HANDLE dloadHandle = (HANDLE)_beginthread(downloadFile, 0, (void*)"");

	AtlWaitWithMessageLoop(dloadHandle);

	if (!SUCCEEDED(dloadResult))
		return false;

	else
		return true;
}


// Download an updated file
//
bool CWebUpdate::DownloadAvailable(int i)
{
	// Attempt to download the file
	remoteFile = remoteURL + "/" + availableFiles.GetAt(i);
	localFile = localDir + "\\" + availableFiles.GetAt(i);

	HANDLE dloadHandle = (HANDLE)_beginthread(downloadFile, 0, (void*)"");

	AtlWaitWithMessageLoop(dloadHandle);

	if (!SUCCEEDED(dloadResult))
		return false;

	else
		return true;
}