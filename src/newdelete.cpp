/**
 *	@file	newdelete.cpp
 *	@brief	operator new, delete ��������ꍇ�̏���.
 *	@note
 *	+++ stdafx.cpp �ɂ����� �f�o�b�O���O�p��operator new,delete �������������̂𕪗�.
 */

#include "stdafx.h"


//+++ USE_ZEROFILL_NEW, USE_MEMORYLOG �� stdafx.h �Œ�`.

//+++ 0�N���A�łȂ��A�A�h���X�G���[���₷�����ȓK���Ȓl�œh��Ԃ��ꍇ�ɒ�`.
//#define USE_BAD_FIL


//#define USE_MEMORYLOG
#if (defined _DEBUG) == 0		//+++ �f�o�b�O�ȊO�̂Ƃ��̓��O���o���Ȃ�.
#undef USE_MEMORYLOG
#endif


#if !(defined _CRTDBG_MAP_ALLOC) && ((defined USE_ZEROFILL_NEW) || (defined USE_MEMORYLOG))

#undef new
#undef delete


void *operator new (size_t bytes)
{
	if (bytes == 0)
		bytes = 1;
 #if defined USE_DL_PREFIX && defined dlmalloc == 0
  #if defined USE_BAD_FIL
	void *p  = ::dlmalloc(bytes);
	if (p) memset(p,0xbb,bytes);
  #elif defined USE_ZEROFILL_NEW
	void *p  = ::dlcalloc(1, bytes);
  #endif
 #else
  #if defined USE_BAD_FIL
	void *p  = ::malloc(bytes);
	if (p) memset(p,0xbb,bytes);
  #elif defined USE_ZEROFILL_NEW
	void *p  = ::calloc(1, bytes);
  #endif
 #endif

  #if defined USE_MEMORYLOG
	FILE *fp = fopen("memory_log.txt", "aw+");
	fprintf(fp, "new   %.8x :%d\n", p, bytes);
	fclose(fp);
  #endif

	return p;
}



void *operator new[] (size_t bytes)
{
	if (bytes == 0)
		bytes = 1;

 #if defined USE_DL_PREFIX && defined dlmalloc == 0
  #if defined USE_BAD_FIL
	void *p  = ::dlmalloc(bytes);
	if (p) memset(p,0xdd,bytes);
  #elif defined USE_ZEROFILL_NEW
	void *p  = ::dlcalloc(1, bytes);
  #endif
 #else
  #if defined USE_BAD_FIL
	void *p  = ::malloc(bytes);
	if (p) memset(p,0xdd,bytes);
  #elif defined USE_ZEROFILL_NEW
	void *p  = ::calloc(1, bytes);
  #endif
 #endif

  #if defined USE_MEMORYLOG
	FILE *fp = fopen("memory_log.txt", "aw+");
	fprintf(fp, "new[] %.8x :%d\n", p, bytes);
	fclose(fp);
  #endif

	return p;
}



void operator delete(void *p)
{
	if (p == NULL)
		return ;

 #ifndef USE_DL_PREFIX
  #if defined USE_BAD_FIL
	size_t sz = ::_msize(p);
	if (sz) memset(p, 0x99, sz);
  #elif defined USE_ZEROFILL_NEW	//����ɋ����ɃN���A
	size_t sz = ::_msize(p);
	if (sz) memset(p, 0, sz);
  #endif
 #endif

  #if defined USE_MEMORYLOG
	FILE *fp = fopen("debug_memory.txt", "aw+");
	fprintf(fp, "del   %.8x\n", p);
	fclose(fp);
  #endif

 #if defined USE_DL_PREFIX && defined dlmalloc == 0
	::dlfree(p);
 #else
	::free(p);
 #endif
}



void operator delete[] (void *p)
{
	if (p == NULL)
		return ;

 #ifndef USE_DL_PREFIX
	size_t sz = ::_msize(p);
  #if defined USE_BAD_FIL
	if (sz) memset(p, 0x77, sz);
  #elif defined USE_ZEROFILL_NEW	//����ɋ����ɃN���A
	if (sz) memset(p, 0, sz);
  #endif
 #endif

  #if defined USE_MEMORYLOG
	FILE *fp = fopen("debug_memory.txt", "aw+");
	fprintf(fp, "del[] %.8x\n", p);
	fclose(fp);
  #endif

 #if defined USE_DL_PREFIX && defined dlmalloc == 0
	::dlfree(p);
 #else
	::free(p);
 #endif
}


#endif

