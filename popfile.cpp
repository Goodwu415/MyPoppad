#include <Windows.h>
#include <commdlg.h>

static OPENFILENAME ofn;

void PopFileInitialize(HWND hwnd)
{
	static TCHAR szFilter[] = TEXT("Text Files (*.TXT)\0*.txt\0")\
		TEXT("ASCII Files (*.ASC)\0*.asc\0")\
		TEXT("All Files (*.*)\0*.*\0\0");
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.lpstrFile = NULL;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("txt");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
}

BOOL PopFileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

	return GetOpenFileName(&ofn);
}
BOOL PopFileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrFileName;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}
BOOL PopFileRead(HWND hwndEdit, PTSTR pstrFileName)
{
	BYTE bySwap;
	DWORD dwBytesRead;
	HANDLE hFile;
	int i, iFileLength, iUniTest;
	PBYTE pBuffer, pText, pConv;
	//Open the file
	if(INVALID_HANDLE_VALUE ==
		(hFile = CreateFile(pstrFileName, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL)))
	{
		return FALSE;
	}
	iFileLength = GetFileSize(hFile, NULL);
	pBuffer = new BYTE(iFileLength + 2);
	//读文件和put zeros 到文件尾部
	ReadFile(hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
	CloseHandle(hFile);
	pBuffer[iFileLength] = '\0';
	pBuffer[iFileLength + 1] = '\0';
	//查看文件是否是Unicode编码
	iUniTest = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;
	if(IsTextUnicode(pBuffer, iFileLength, &iUniTest))
	{
		pText = pBuffer + 2;
		iFileLength -= 2;
		if(iUniTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE)
		{
			for(i = 0; i < iFileLength / 2; i++)
			{
				bySwap = ((BYTE*)pText)[2 * i];
				((BYTE*)pText)[2 * i] = ((BYTE*)pText)[2 * i + 1];
				((BYTE*)pText)[2 * i + 1] = bySwap;
			}
		}
		pConv =new BYTE(iFileLength + 2);
#ifndef UNICODE	//宽字符转多字节
		WideCharToMultiByte(CP_ACP, 0, (PWSTR)pText, -1, (LPSTR)pConv,
			iFileLength + 2, NULL, NULL);
#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif
	} else  //这个文件不是unicode编码
	{
		pText = pBuffer;
		//pConv = (PBYTE)malloc(2 * iFileLength + 2);
		pConv = new BYTE(2 * iFileLength + 2);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, pText, -1, (PTSTR)pConv, iFileLength + 1);
#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif
	}
	SetWindowText(hwndEdit, (PTSTR)pConv);
	delete pBuffer;
	delete pConv;
	return TRUE;
}
BOOL PopFileWrite(HWND hwndEdit, PTSTR pstrFileName)
{
	DWORD dwBytesWritten;
	HANDLE hFile;
	int iLength;
	PTSTR pstrBuffer;
	WORD wByteOrderMark = 0xFEFF;

	if(INVALID_HANDLE_VALUE ==
		(hFile = CreateFile(pstrFileName, GENERIC_WRITE, 0,
			NULL, CREATE_ALWAYS, 0, NULL)))
		return FALSE;
	iLength = GetWindowTextLength(hwndEdit);
	pstrBuffer = (PTSTR)malloc((iLength + 1) * sizeof(TCHAR));
	if(!pstrBuffer)
	{
		CloseHandle(hFile);
		return FALSE;
	}
#ifdef UNICODE
	WriteFile(hFIle, &wByteOrderMark, 2, &dwBytesWritten, NULL);
#endif
	GetWindowText(hwndEdit, pstrBuffer, iLength + 1);
	WriteFile(hFile, pstrBuffer, iLength * sizeof(TCHAR),&dwBytesWritten, NULL);
	if((iLength * sizeof(TCHAR)) != (int)dwBytesWritten)
	{
		CloseHandle(hFile);
		free(pstrBuffer);
		return FALSE;
	}
	CloseHandle(hFile);
	free(pstrBuffer);
	return TRUE;
}