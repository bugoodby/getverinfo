/****************************************************************
;  typedef.h
;  
;  type definitions
;
;  coding : 080612 matsunaga
****************************************************************/

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

/*** typedef ***/

typedef signed int				INT;
typedef unsigned int			UINT;
typedef signed short			SHORT;
typedef unsigned short			USHORT;
typedef signed long				LONG;
typedef unsigned long			ULONG;
typedef signed char				CHAR;
typedef unsigned char			UCHAR;

typedef unsigned char			BYTE;
typedef unsigned short			WORD;
typedef unsigned long			DWORD;

/* for MBCS */
typedef unsigned char			MCHAR, *LPMSTR;
typedef const unsigned char		*LPCMSTR;


/*** define ***/

#ifndef TRUE
#define TRUE		true
#endif

#ifndef FALSE
#define FALSE		false
#endif

#ifndef MAX_PATH
#define MAX_PATH	260
#endif


#endif	/* _TYPEDEF_H_ */
