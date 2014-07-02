#ifndef __MAIN_H__
#define __MAIN_H__

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <windows.h>

#define GET_ERROR_VAL( val )	((val) = -1 * __LINE__)


/*----------------------------------------*/
/* �v���O�������A�o�[�W����               */
/*----------------------------------------*/
#define MODULENAME_STRING	L"getverinfo"
#define VERSION_STRING		L"0.01"


/*----------------------------------------*/
/* �R�}���h���C����͌��ʂ��i�[����\���� */
/*----------------------------------------*/
typedef struct tagOPTION {
	int			num;					/* number of input files */
	_TCHAR**	argv;					/* pointer to a file name array */
	bool		wait_key_input;			/* if true, wait key input */
	_TCHAR*		out_file;				/* output file path */
} OPTION;

/*----------------------------------------*/
/* �֐��̃v���g�^�C�v�錾                 */
/*----------------------------------------*/
int application_main( FILE *outfp, OPTION &opt );

#endif /* __MAIN_H__ */
