#include "stdafx.h"
#include "main.h"
#include <io.h>

#pragma comment(lib, "version.lib")


static wchar_t *g_szString[] = {
	L"CompanyName",			//��Ж� 
	L"FileDescription",		//���� 
	L"FileVersion",			//�t�@�C���E�o�[�W���� 
	L"InternalName",		//������ 
	L"LegalCopyright",		//���쌠 
	L"OriginalFileName",	 //�����t�@�C���� 
	L"ProductName",			 //���i�� 
	L"ProductVersion",		 //���i�o�[�W���� 
	L"Comments",			 //�R�����g 
	L"LegalTrademarks",		 //���W 
	L"PrivateBuild",		 //�v���C�x�[�g�E�r���h��� 
	L"SpecialBuild",		 //�X�y�V�����E�r���h���
	NULL
};

OPTION g_opt;

int getverinfo( FILE *outfp )
{
	DWORD dwLen = 0;
	DWORD dwHandle = 0;
	LPCTSTR pszFilePath = *g_opt.argv;
	
	fwprintf(outfp, L"%s\n---\n\n", pszFilePath);
	
	// �o�[�W������񃊃\�[�X�̃T�C�Y���擾
	dwLen = ::GetFileVersionInfoSize(pszFilePath, &dwHandle);
	if ( dwLen > 0 )
	{
		// �o�b�t�@�̊m��
		BYTE *lpData = new BYTE[dwLen + 1];
		if ( lpData )
		{
			// �o�[�W������񃊃\�[�X���o�b�t�@�ɓǂݍ���
			if ( ::GetFileVersionInfo(pszFilePath, 0, dwLen, lpData) )
			{
				UINT nLen;
				DWORD *dwLang;
				wchar_t lpSubBlock[50] = L"\\VarFileInfo\\Translation";

				// lang-codepage���擾
				if ( ::VerQueryValue(lpData, lpSubBlock, (void **)&dwLang, &nLen) )
				{
					// lang-codepage���t�H�[�}�b�g
					wchar_t lpLang[9];
					swprintf(lpLang, L"%04x%04x", LOWORD(*dwLang), HIWORD(*dwLang));

					wchar_t **ppszString = g_szString;
					while ( *ppszString ) {
						// �����擾����
						wchar_t *lpBuffer;
						//wchar_t *lpString = L"ProductVersion";  // ���i�o�[�W������
						swprintf(lpSubBlock, L"\\StringFileInfo\\%s\\%s", lpLang, *ppszString);
						::VerQueryValue(lpData, lpSubBlock, (void **)&lpBuffer, &nLen);
						// �\��
						fwprintf(outfp, L"%s = %s\n", *ppszString, lpBuffer);

						ppszString++;
					}

				}
			}
			delete [] lpData;
		}
	}

	return 0;
}

int find_recursive( FILE *outfp, wchar_t *pszPath, size_t len );
int getverinfo2( FILE *outfp, LPCTSTR pszFilePath );

int getverinfodir( FILE *outfp )
{
	int retval = 0;
	wchar_t buffer[MAX_PATH] = {0};

	wcscpy(buffer, *g_opt.argv);
	
	/* header */
	fwprintf(outfp, L"Target Path = \"%s\"\n", buffer);
	fwprintf(outfp, L"File Name,File Time,");
	wchar_t **ppszString = g_szString;
	while ( *ppszString ) {
		fwprintf(outfp, L"\"%s\",", *ppszString );
		ppszString++;
	}
	fputws(L"\n", outfp);

	
	/* information list */
	retval = find_recursive( outfp, buffer, wcslen(buffer) ); 

	return retval;
}

int find_recursive( FILE *outfp, wchar_t *pszPath, size_t len )
{
	int found_num = 0;
	long lHandle = 0;
	struct _tfinddata_t tFiles = {0};
	size_t suffix_len = 0;

	wcscpy( pszPath + len, L"\\*" );

	lHandle = _wfindfirst( pszPath, &tFiles );
	if ( lHandle != -1L )
	{
		do {
			if ( wcscmp( tFiles.name, L"." ) == 0 ) continue;
			if ( wcscmp( tFiles.name, L".." ) == 0 ) continue;

			if ( tFiles.attrib & _A_SUBDIR ) 
			{
				/* find subdirectory */
				suffix_len = wcslen(tFiles.name);
				wcscpy( pszPath + len + 1, tFiles.name );
				find_recursive( outfp, pszPath, len + 1 + suffix_len );
			}
			else 
			{
				wcscpy( pszPath + len + 1, tFiles.name );
				getverinfo2(outfp, pszPath);
			}
		} while ( _wfindnext( lHandle, &tFiles ) == 0 );

		_findclose( lHandle );
	}
	
	return found_num;
}

//---------------------------------------------------------------
// [NAME   ] GetLastWriteTime
// [ACTION ] �w�肵���t�@�C���̍X�V����(LocalTime)���擾
// [ARGMENT] pszFilePath   [in]  �t�@�C���̃t���p�X
//           pstLastWrite  [out] �X�V�������i�[����SYSTEMTIME�\���̂ւ̃|�C���^
// [RETURN ] ����������TRUE, ���s������FALSE
// [REMARK ] 
//---------------------------------------------------------------
BOOL GetLastWriteTime( LPCTSTR pszFilePath, SYSTEMTIME *pstLastWrite )
{
	BOOL bRet;
	
	if ( !pszFilePath || !pstLastWrite ) {
		return FALSE;
	}
	
	// �O�̂���
	if ( pstLastWrite )   memset( pstLastWrite, 0x00, sizeof(SYSTEMTIME) );
	
	// �Ώۃt�@�C�����J���A�n���h�����擾
	HANDLE hFile = ::CreateFile(
						pszFilePath,
						0,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if ( hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	// FILETIME���擾
	FILETIME ftLastWrite;

	if ( !::GetFileTime(hFile, NULL, NULL, &ftLastWrite) ) {
		return FALSE;
	}

	// UTC����Local Time�֕ϊ�
	FILETIME ftLocalLastWrite;
	bRet = ::FileTimeToLocalFileTime(&ftLastWrite,  &ftLocalLastWrite);
	if ( !bRet ) {
		return FALSE;
	}

	// FILETIME��SYSTEMTIME�֕ϊ�
	bRet = ::FileTimeToSystemTime(&ftLocalLastWrite,  pstLastWrite);
	if ( !bRet ) {
		return FALSE;
	}
	
	::CloseHandle(hFile);
	return TRUE;
}

int getverinfo2( FILE *outfp, LPCTSTR pszFilePath )
{
	DWORD dwLen = 0;
	DWORD dwHandle = 0;
	BYTE *lpData = NULL;
	UINT nLen;
	DWORD *dwLang;
	wchar_t lpSubBlock[50] = L"\\VarFileInfo\\Translation";
	SYSTEMTIME st;
	
	fwprintf(outfp, L"%s,", pszFilePath);

	// �ŏI�X�V�����̎擾
	GetLastWriteTime(pszFilePath, &st);
	fwprintf(outfp, L"\"%04d/%02d/%02d %02d/%02d/%02d\",", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	// �o�[�W������񃊃\�[�X�̃T�C�Y���擾
	dwLen = ::GetFileVersionInfoSize(pszFilePath, &dwHandle);
	if ( dwLen == 0 ) {
		fwprintf(stderr, L"No version resource.\n");
		goto EXIT;
	}

	// �o�b�t�@�̊m��
	lpData = (BYTE*)malloc(dwLen + 1);
	if ( !lpData ) {
		fwprintf(stderr, L"Can't alloc enouth memory!\n");
		goto EXIT;
	}

	// �o�[�W������񃊃\�[�X���o�b�t�@�ɓǂݍ���
	if ( !::GetFileVersionInfo(pszFilePath, 0, dwLen, lpData) ) {
		fwprintf(stderr, L"[FAIL] GetFileVersionInfo\n");
		goto EXIT;
	}

	// lang-codepage���擾
	if ( ::VerQueryValue(lpData, lpSubBlock, (void **)&dwLang, &nLen) )
	{
		wchar_t **ppszString = g_szString;
		wchar_t szLang[9] = L"";

		// lang-codepage���t�H�[�}�b�g
		swprintf(szLang, L"%04x%04x", LOWORD(*dwLang), HIWORD(*dwLang));
		
		// �e���\�[�X�̎擾
		while ( *ppszString ) {
			BOOL bRead;
			wchar_t *pszBuffer;
			
			swprintf(lpSubBlock, L"\\StringFileInfo\\%s\\%s", szLang, *ppszString);
			bRead = ::VerQueryValue(lpData, lpSubBlock, (void **)&pszBuffer, &nLen);

			// �\��
			fwprintf(outfp, L"\"%s\",", (bRead) ? pszBuffer : L"" );
			ppszString++;
		}

	}

EXIT:
	fputws(L"\n", outfp);
	if ( lpData ) free(lpData);

	return 0;
}

int getverinfolist( FILE *outfp )
{
	int retval = 0;
	wchar_t buffer[MAX_PATH] = {0};
	FILE *infp = NULL;

	wcscpy(buffer, *g_opt.argv);
	
	infp = _wfopen( buffer, L"r" );
	if ( !infp ) {
		fwprintf(stderr, L"[ERROR] cannot open %s.\n", buffer);
		goto EXIT;
	}

	{
		/* header */
		fwprintf(outfp, L"Target Path = \"%s\"\n", buffer);
		fwprintf(outfp, L"File Name,File Time,");
		wchar_t **ppszString = g_szString;
		while ( *ppszString ) {
			fwprintf(outfp, L"\"%s\",", *ppszString );
			ppszString++;
		}
		fputws(L"\n", outfp);

		
		/* information list */
		wchar_t szLine[MAX_PATH + 1];
		
		while ( fgetws(szLine, MAX_PATH, infp) != NULL ) 
		{
			size_t size = wcslen(szLine);
			if ( szLine[size - 1] == L'\n' ) {
				szLine[size - 1] = L'\0';
			}
			retval = getverinfo2( outfp, szLine ); 
		}
	}

EXIT:
	if ( infp ) fclose(infp);
	return retval;
}

int application_main( FILE *outfp, OPTION &opt )
{
	int retval = 0;
	DWORD dwRet;
	g_opt = opt;
	
	dwRet = ::GetFileAttributesW( *g_opt.argv );
	if ( dwRet != -1 && dwRet & FILE_ATTRIBUTE_DIRECTORY ) {
		retval = getverinfodir(outfp);
	} 
	else if ( wcsstr(*g_opt.argv, L".list") != NULL ) {
		retval = getverinfolist(outfp);
	}
	else {
		retval = getverinfo(outfp);
	}

	return retval;
}
	
