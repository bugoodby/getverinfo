// getverinfo.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include "main.h"

void usage(void);
bool parse_cmdline( int argc, wchar_t **argv, OPTION &opt );


//--------------------------------------------------------------
// main
//--------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	OPTION opt = {0};
	int i = 0;
	int retval = 0;
	FILE *console = stderr;
	FILE *infp = stdin;
	FILE *outfp = stdout;

	setlocale(LC_CTYPE, "japanese");

	/* parse */
	if ( !parse_cmdline(argc, argv, opt) ) {
		goto EXIT;
	}
	
	if ( opt.num <= 0 ) {
		goto EXIT;
	}

	if ( opt.out_file ) {
		outfp = _wfopen( opt.out_file, L"w" );
		if ( !outfp ) {
			fwprintf(stderr, L"[ERROR] cannot open %s.\n", opt.out_file);
			goto EXIT;
		}
	}

	/* main function */
	retval = application_main(outfp, opt);
	if ( retval < 0 ) {
		fwprintf(console, L"[ERROR] fail in main routine.\n");
	}

EXIT:
	if ( opt.wait_key_input ) {
		fputws(L"--- press any key ---\n", outfp);
		getwc(stdin);
	}

	if ( outfp && outfp != stdout ) fclose(outfp);

	return retval;
}

//--------------------------------------------------------------
// show usage
//--------------------------------------------------------------
void usage(void)
{
	fwprintf(stderr, L"[ %s Version %s ]\n", MODULENAME_STRING, VERSION_STRING);
	fwprintf(stderr, L"get version informations from a file resource.\n");
	fwprintf(stderr, L"\n");
	fwprintf(stderr, L"Usage : %s [option] <input file>\n", MODULENAME_STRING);
	fwprintf(stderr, L"        -c               : do not wait a key input\n");
	fwprintf(stderr, L"        -o <file>        : output result to file\n");
	fwprintf(stderr, L"        <infput file>    : input file path\n");
}

//--------------------------------------------------------------
// parse command line
//--------------------------------------------------------------
bool parse_cmdline( int argc, wchar_t **argv, OPTION &opt )
{
	bool ret = true;
	wchar_t *s = NULL;

	/* initialize */
	opt.wait_key_input = true;

	/* parse */
	while ( --argc > 0 ) {
		s = *++argv;
		if ( *s == L'-' || *s == L'/' ) {
			switch ( *++s )
			{
			case L'h':
			case L'?':
				usage();
				ret = false;
				break;
			case L'c':
				opt.wait_key_input = false;
				break;
			case L'o':
				if ( --argc > 0 ) {
					opt.out_file = *++argv;
				}
				break;
			default:
				fwprintf(stderr, L"Unknown parameter : -%s\n", s);
				ret = false;
				break;
			}
		}
		else {
			opt.num = argc;
			opt.argv = argv;
			break;
		}
	}

	return ret;
}

