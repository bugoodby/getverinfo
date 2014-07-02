#include "stdafx.h"
#include "main.h"
#include <io.h>

#pragma comment(lib, "version.lib")


static wchar_t *g_szString[] = {
	L"CompanyName",			//会社名 
	L"FileDescription",		//説明 
	L"FileVersion",			//ファイル・バージョン 
	L"InternalName",		//内部名 
	L"LegalCopyright",		//著作権 
	L"OriginalFileName",	 //正式ファイル名 
	L"ProductName",			 //製品名 
	L"ProductVersion",		 //製品バージョン 
	L"Comments",			 //コメント 
	L"LegalTrademarks",		 //商標 
	L"PrivateBuild",		 //プライベート・ビルド情報 
	L"SpecialBuild",		 //スペシャル・ビルド情報
	NULL
};

OPTION g_opt;

int getverinfo( FILE *outfp )
{
	DWORD dwLen = 0;
	DWORD dwHandle = 0;
	LPCTSTR pszFilePath = *g_opt.argv;
	
	fwprintf(outfp, L"%s\n---\n\n", pszFilePath);
	
	// バージョン情報リソースのサイズを取得
	dwLen = ::GetFileVersionInfoSize(pszFilePath, &dwHandle);
	if ( dwLen > 0 )
	{
		// バッファの確保
		BYTE *lpData = new BYTE[dwLen + 1];
		if ( lpData )
		{
			// バージョン情報リソースをバッファに読み込む
			if ( ::GetFileVersionInfo(pszFilePath, 0, dwLen, lpData) )
			{
				UINT nLen;
				DWORD *dwLang;
				wchar_t lpSubBlock[50] = L"\\VarFileInfo\\Translation";

				// lang-codepageを取得
				if ( ::VerQueryValue(lpData, lpSubBlock, (void **)&dwLang, &nLen) )
				{
					// lang-codepageをフォーマット
					wchar_t lpLang[9];
					swprintf(lpLang, L"%04x%04x", LOWORD(*dwLang), HIWORD(*dwLang));

					wchar_t **ppszString = g_szString;
					while ( *ppszString ) {
						// 情報を取得する
						wchar_t *lpBuffer;
						//wchar_t *lpString = L"ProductVersion";  // 製品バージョン名
						swprintf(lpSubBlock, L"\\StringFileInfo\\%s\\%s", lpLang, *ppszString);
						::VerQueryValue(lpData, lpSubBlock, (void **)&lpBuffer, &nLen);
						// 表示
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
// [ACTION ] 指定したファイルの更新日時(LocalTime)を取得
// [ARGMENT] pszFilePath   [in]  ファイルのフルパス
//           pstLastWrite  [out] 更新日時を格納するSYSTEMTIME構造体へのポインタ
// [RETURN ] 成功したらTRUE, 失敗したらFALSE
// [REMARK ] 
//---------------------------------------------------------------
BOOL GetLastWriteTime( LPCTSTR pszFilePath, SYSTEMTIME *pstLastWrite )
{
	BOOL bRet;
	
	if ( !pszFilePath || !pstLastWrite ) {
		return FALSE;
	}
	
	// 念のため
	if ( pstLastWrite )   memset( pstLastWrite, 0x00, sizeof(SYSTEMTIME) );
	
	// 対象ファイルを開き、ハンドルを取得
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

	// FILETIMEを取得
	FILETIME ftLastWrite;

	if ( !::GetFileTime(hFile, NULL, NULL, &ftLastWrite) ) {
		return FALSE;
	}

	// UTCからLocal Timeへ変換
	FILETIME ftLocalLastWrite;
	bRet = ::FileTimeToLocalFileTime(&ftLastWrite,  &ftLocalLastWrite);
	if ( !bRet ) {
		return FALSE;
	}

	// FILETIMEをSYSTEMTIMEへ変換
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

	// 最終更新日時の取得
	GetLastWriteTime(pszFilePath, &st);
	fwprintf(outfp, L"\"%04d/%02d/%02d %02d/%02d/%02d\",", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	// バージョン情報リソースのサイズを取得
	dwLen = ::GetFileVersionInfoSize(pszFilePath, &dwHandle);
	if ( dwLen == 0 ) {
		fwprintf(stderr, L"No version resource.\n");
		goto EXIT;
	}

	// バッファの確保
	lpData = (BYTE*)malloc(dwLen + 1);
	if ( !lpData ) {
		fwprintf(stderr, L"Can't alloc enouth memory!\n");
		goto EXIT;
	}

	// バージョン情報リソースをバッファに読み込む
	if ( !::GetFileVersionInfo(pszFilePath, 0, dwLen, lpData) ) {
		fwprintf(stderr, L"[FAIL] GetFileVersionInfo\n");
		goto EXIT;
	}

	// lang-codepageを取得
	if ( ::VerQueryValue(lpData, lpSubBlock, (void **)&dwLang, &nLen) )
	{
		wchar_t **ppszString = g_szString;
		wchar_t szLang[9] = L"";

		// lang-codepageをフォーマット
		swprintf(szLang, L"%04x%04x", LOWORD(*dwLang), HIWORD(*dwLang));
		
		// 各リソースの取得
		while ( *ppszString ) {
			BOOL bRead;
			wchar_t *pszBuffer;
			
			swprintf(lpSubBlock, L"\\StringFileInfo\\%s\\%s", szLang, *ppszString);
			bRead = ::VerQueryValue(lpData, lpSubBlock, (void **)&pszBuffer, &nLen);

			// 表示
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
	
