/**
 *	@file	FavoriteOrder.cpp
 *	@brief	���W�X�g���ɂ��邨�C�ɓ�����𑀍삵�ĕ��я�������������N���X
 */
#include "stdafx.h"
#include "FavoriteOrder.h"


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/// ���W�X�g������f�[�^��ǂݍ���
int CFavoriteOrderHandler::ReadData(const CString &strDirPath)
{
	if (Misc::IsExistFile( strDirPath) == 0)
		return FALSE;
	CString 		strKeyName = GetOrderKeyName(strDirPath);

	if ( strKeyName.IsEmpty() )
		return FALSE;

	m_strRegKey  = strKeyName;
	m_strDirPath = strDirPath;

	//���W�X�g������o�C�i���f�[�^���擾
	Misc::CRegKey 	rkOrder;
	LONG			lRet	   = rkOrder.Open(HKEY_CURRENT_USER, strKeyName);

	if (lRet != ERROR_SUCCESS) {
		if ( ReConstructData(strDirPath) )
			return -1;	//TRUE;
		return FALSE;
	}

	DWORD			dwSize	   = 0;
	DWORD			dwType	   = REG_BINARY;
	lRet		 = ::RegQueryValueEx(rkOrder, _T("Order"), NULL, &dwType, NULL, &dwSize);

	if (lRet == ERROR_FILE_NOT_FOUND) {
		if ( ReConstructData(strDirPath) )
			return TRUE;
	}

	if ( lRet != ERROR_SUCCESS || dwSize < sizeof (OrderDataHeader) )
		return FALSE;

	BYTE *			pByte	   = (BYTE *) _alloca( (dwSize + 10) * sizeof (BYTE) );
	::memset(pByte, 0, dwSize + 10);

	lRet		 = ::RegQueryValueEx(rkOrder, _T("Order"), NULL, &dwType, pByte, &dwSize);

	if (lRet != ERROR_SUCCESS)
		return FALSE;

	rkOrder.Close();

	//�f�[�^��z��m_aryData�Ɋi�[����
	m_aryData.clear();
	OrderDataHeader header	   = *(OrderDataHeader *) pByte;
	pByte		+= sizeof (OrderDataHeader);
	LPBYTE			pEnd	   = pByte + ( header.dwFullSizem8 + 8 - sizeof (OrderDataHeader) );
	int 			nCount	   = header.dwItemCount;

	for (int i = 0; i < nCount; i++) {
		OrderItemTemplate*	pItem = (OrderItemTemplate *) pByte;			//����������
		OrderItemData		data;											//�������փf�[�^���R�s�[
		data.dwSize 	= pItem->dwSize;
		data.dwPosition = pItem->dwPosition;

		CItemIDList 	idl(&pItem->idlName);
		data.idlName	= idl;

		if (data.idlName.GetSize() & 1)
			data.bPadding = TRUE;
		else
			data.bPadding = FALSE;

		data.dwFlag 	= *(DWORD *) ( pByte + pItem->dwSize - sizeof (DWORD) );
		data.strName	= MtlGetFileName( data.idlName.GetPath() );

		m_aryData.push_back(data);
		pByte		   += pItem->dwSize;

		if (pByte > pEnd)
			break;
	}

	ReConstructData(m_strDirPath);

	return TRUE;
}




/// �f�[�^��ǉ�����.
/// m_strDirPath ����̑��΃p�X�Ŏw��.
BOOL CFavoriteOrderHandler::AddData(CString strSrcFileName, DWORD dwPosition)
{
	if (dwPosition == -1)
		dwPosition = (int)m_aryData.size();

	if (dwPosition < 0 || m_aryData.size() < dwPosition)
		return FALSE;

	if ( strSrcFileName.IsEmpty() )
		return FALSE;

	//�t�@�C�����̓t���p�X�ɒ���
	CString 					strDir	  = m_strDirPath;
	MtlMakeSureTrailingBackSlash(strDir);

	if (strSrcFileName.Find('\\') == -1)
		strSrcFileName = strDir + strSrcFileName;

	//�f�[�^�𖄂߂�
	OrderItemData	item;
	item.dwPosition = dwPosition;
  #if 1	//+++
	CString dirName = Misc::GetDirName(strSrcFileName);
	CString	baseName= Misc::GetFileBaseName(strSrcFileName);
	CString	dstName = m_strDirPath + _T('\\') + baseName;
	if (dirName.CompareNoCase(m_strDirPath) == 0) {
		if ( !MtlIsValidPath(strSrcFileName) )
			return FALSE;

		item.idlName	= strSrcFileName;
	} else {
		if ( !MtlIsValidPath(dstName) )
			return FALSE;

		//::MoveFile( strSrcFileName, dstName);
		item.idlName	= dstName;
	}
  #else
	if ( !MtlIsValidPath(strSrcFileName) )
		return FALSE;
	item.idlName	= strFileName;
  #endif
	CItemIDList 	idlFolder = m_strDirPath;
	item.idlName   -= idlFolder;

	int 	nSize	= item.idlName.GetSize();
	item.bPadding	= (nSize & 1);		//+++ ? TRUE : FALSE;
	item.dwSize 	= nSize + s_itemsize_without_idl + (item.bPadding /*+++ ? 1 : 0*/ );
	item.strName	= baseName;			//+++ MtlGetFileName(strSrcFileName);

	//�����̓����̍��ڂ����݂���΍폜����
	CString 					strPath   = item.idlName.GetPath();
	FavoriteDataArray::iterator it		  = m_aryData.begin();
	while (1) {
		if ( it == m_aryData.end() )
			break;

		if (it->idlName.GetPath() == strPath) {
			it = m_aryData.erase(it);
		} else {
			++it;
		}
	}

	//�t�H���_���ۂ��𔻒�
	//if ( MtlIsValidPath(strFileName) )
	if ( MtlIsValidPath(dstName) )
	{
		CString strFavDir;
		MtlGetFavoritesFolder(strFavDir);

		if (strFavDir != m_strDirPath) {
			//if ( MtlIsDirectory(strSrcFileName) )
			if ( MtlIsDirectory(dstName) )
				item.dwFlag = IE_MENUORDER_FOLDER;
			else
				item.dwFlag = IE_MENUORDER_ITEM;
		} else {
			item.dwFlag = IE_MENUORDER_ZERO;
		}
	} else {
		return FALSE;
	}

  #if 0	//+++
	//�|�W�V���������炷
	for (it = m_aryData.begin(); it != m_aryData.end(); it++) {
		if (dwPosition <= it->dwPosition)
			++it->dwPosition;
	}

   #if 1	//+++
	m_aryData.insert(m_aryData.begin()+dwPosition, item);
   #else
	m_aryData.push_back(item);

	// dwPotision�Ń\�[�g����
	_FavoritesMenuPosCompare	f(m_strDirPath);
	std::stable_sort(m_aryData.begin(), m_aryData.end(), f);
   #endif
  #else
//ErrorLogPrintf("%s  %s  %d\n", LPCTSTR(m_strDirPath), LPCTSTR(dstName), dwPosition);
	//�|�W�V���������炷
	for (it = m_aryData.begin(); it != m_aryData.end(); it++) {
		if (dwPosition <= it->dwPosition)
			++it->dwPosition;
	}

	item.bRef = true;
	m_aryData.push_back(item);
   #if 0	//+++
	ReConstructData(m_strDirPath);
   #else
	//������Ń\�[�g����
	_FavoritesMenuNameCompare	f(m_strDirPath);
	std::sort(m_aryData.begin(), m_aryData.end(), f);
   #endif
  #endif

	return TRUE;
}



BOOL	CFavoriteOrderHandler::MoveData(CFavoriteOrderHandler& srcOrder, const CString &srcPath, DWORD dstPos)
{
	CString srcDir   = Misc::GetDirName(srcPath);
	CString name     = Misc::GetFileBaseName(srcPath);
	CString& dstDir	 = m_strDirPath;
	CString dstName  = dstDir + _T('\\') + name;
	bool	bCopy	 = false;
	if (srcDir.CompareNoCase(dstDir) == 0) {
		DWORD srcPos = FindNameToPosition( name/*srcPath*/ );
		return MoveData(srcPos, dstPos);
	}
	if (srcOrder.FindName(srcPath) < 0) {
		ErrorLogPrintf(_T("���C�ɓ��胁�j���[�ł́A�t�H���_���܂������ړ��ŁA�\�[�X�t�H���_���s��(%s)\n"), LPCTSTR(srcPath));
		return FALSE;
	}
	if (::MoveFile( srcPath, dstName ) == 0) {
		ErrorLogPrintf(_T("���C�ɓ��胁�j���[�ł́A�t�H���_���܂������ړ��ŁA�t�@�C���ړ����s(%s)\n"), LPCTSTR(name));
		return FALSE;
	} else {
		bCopy = ::CopyFile(dstName, srcPath, TRUE) != 0;	//+++ ���ڂ̍폜���̎��̂Ƃ��āA�_�~�[���쐬
	}
	if (srcOrder.DeleteData( srcPath, true ) == 0) {
		ErrorLogPrintf(_T("���C�ɓ��胁�j���[�ł́A�t�H���_���܂������ړ��ŁA���t�H���_���ō폜������Ȃ�����(%s)\n"), LPCTSTR(name));
	}
	if (bCopy)	//+++ �����������s���Ă����ꍇ�̂��߂̍폜.
		::DeleteFile(srcPath);
	if (AddData(srcPath, dstPos) == 0) {
		ErrorLogPrintf(_T("���C�ɓ��胁�j���[�ł́A�t�H���_���܂������ړ��ŁA�s����ւ̒ǉ����s(%s)\n"), LPCTSTR(name));
		return FALSE;
	}
	return TRUE;
}



BOOL CFavoriteOrderHandler::MoveData(DWORD dwSrcPos, DWORD dwDstPos)
{
	if (dwSrcPos == dwDstPos)
		return TRUE;

	if (dwSrcPos < 0 || dwDstPos < 0)
		return FALSE;

	int 						nSrcIndex = FindPosition(dwSrcPos);

	if (nSrcIndex == -1)
		return FALSE;

	FavoriteDataArray::iterator 	it;

	if (dwSrcPos < dwDstPos) {
		//dwSrcPos�ȏ�dwDstPos�ȉ��̃|�W�V�������������
		for (it = m_aryData.begin(); it != m_aryData.end(); ++it) {
			if (dwSrcPos <= it->dwPosition && it->dwPosition <= dwDstPos)
				it->dwPosition--;
		}
	} else {
		//dwDstPos�ȏ�dwSrcPos�ȉ��̃|�W�V��������グ��
		for (it = m_aryData.begin(); it != m_aryData.end(); ++it) {
			if (dwDstPos <= it->dwPosition && it->dwPosition <= dwSrcPos)
				it->dwPosition++;
		}
	}

	m_aryData[nSrcIndex].dwPosition = dwDstPos;
	return TRUE;
}



BOOL CFavoriteOrderHandler::DeleteData(CString strFileName, bool bSw)
{
	if ( strFileName.IsEmpty() )
		return FALSE;

	//�t�@�C�����̓t���p�X�ɒ���
	CString 					strDir = m_strDirPath;
	MtlMakeSureTrailingBackSlash(strDir);

	if (strFileName.Find('\\') == -1) {
		strFileName = strDir + strFileName;
	} else {
	}

	if ( !MtlIsValidPath(strFileName)  && bSw == 0)
		return FALSE;

	CItemIDList 				idl 	  = strFileName;
	CItemIDList 				idlFolder = m_strDirPath;
	idl -= idlFolder;

	//�����t�@�C�����������ڂ��폜
	CString 					strPath   = idl.GetPath();
	FavoriteDataArray::iterator it		  = m_aryData.begin();

	while (1) {
		if ( it == m_aryData.end() )
			break;

		if (it->idlName.GetPath() == strPath) {
			DWORD dwPosition = it->dwPosition;
			m_aryData.erase(it);

			//���̍��ڂ̔ԍ���t������
			for (FavoriteDataArray::iterator it2 = m_aryData.begin(); it2 != m_aryData.end(); ++it2) {
				DWORD dwPos2 = it2->dwPosition;
				if (dwPosition < dwPos2)
					--it2->dwPosition;
				else if (dwPosition == dwPos2)
					ErrorLogPrintf(_T("���C�ɓ��胁�j���[��%s�Ɠ����ʒu�ԍ�(%d)�̃f�[�^������(%s)\n"), LPCTSTR(strPath), dwPos2, LPCTSTR(it2->strName));
			}

			return TRUE;
		} else {
			++it;
		}
	}

	return FALSE;
}



///�f�[�^�����W�X�g���ɏ�������
BOOL CFavoriteOrderHandler::SaveData()
{
  #if 0
	if ( !_check_flag(CFavoritesMenuOption::GetStyle(), EMS_CUSTOMIZE_ORDER) )
		return FALSE;
  #endif
	//�܂��T�C�Y���m�肳����
	DWORD			 dwFullSize = 0;

	for (FavoriteDataArray::iterator it = m_aryData.begin(); it != m_aryData.end(); ++it) {
		dwFullSize += it->dwSize;
	}

	dwFullSize			 += sizeof (OrderDataHeader); //�w�b�_���𑫂�

	//���������m��
	LPBYTE			 lpMem		= (LPBYTE) new BYTE[dwFullSize];

	if (!lpMem)
		return FALSE;

	LPBYTE			 lpData 	= lpMem;

	//�w�b�_�f�[�^�i�[
	OrderDataHeader *pHeader	= (OrderDataHeader *) lpData;
	pHeader->dwSize8	  = 8;
	pHeader->dwFullSizem8 = dwFullSize - pHeader->dwSize8;
	pHeader->dwUnknown	  = IE_MENUORDER_UNKNOWN1;
	pHeader->dwUnknown2   = IE_MENUORDER_UNKNOWN2;
	pHeader->dwItemCount  = (int)m_aryData.size();
	lpData				 += sizeof (OrderDataHeader);

	//�A�C�e���f�[�^���R�s�[
	int nCount = (int)m_aryData.size();
	for (int i = 0; i < nCount; i++) {
		LPBYTE		   lpStart = lpData;
		OrderItemData *pItem   = &m_aryData[i];
		*(DWORD *) lpData = pItem->dwSize;
		lpData			 += 4;
		*(DWORD *) lpData = pItem->dwPosition;
		lpData			 += 4;
		int 		   nSize   = pItem->idlName.GetSize();
		::memcpy(lpData, (LPITEMIDLIST) pItem->idlName, nSize);
		lpData			 += nSize;

		if (pItem->bPadding) {
			*(BYTE *) lpData = 0;
			lpData			+= 1;
		}

		*(DWORD *) lpData = pItem->dwFlag;
		lpData			 += 4;

		if (lpData - pItem->dwSize != lpStart)
			goto error;
	}

	//�f�[�^����������
	{
		Misc::CRegKey	rkOrder;
		LONG			lRet = rkOrder.Open(HKEY_CURRENT_USER, m_strRegKey);

		if (lRet != ERROR_SUCCESS) {
			lRet = rkOrder.Create(HKEY_CURRENT_USER, m_strRegKey);
			if (lRet != ERROR_SUCCESS)
				goto error;
		}

		lRet = ::RegSetValueEx(rkOrder, _T("Order"), 0, REG_BINARY, lpMem, dwFullSize);
		rkOrder.Close();

		if (lRet != ERROR_SUCCESS)
			goto error;
	}

	//�������J��
	delete[] lpMem;
	return TRUE;

  error:
	delete[] lpMem;
	return FALSE;
}



/// �f�[�^���\�z����
BOOL CFavoriteOrderHandler::ReConstructData(CString strDirPath)
{
	//�t�H���_�̃t�@�C����񋓂��ăf�[�^�Ɋ܂܂�Ă��Ȃ����̂�T���Ēǉ�
	int 							nFirstCount = (int)m_aryData.size();
	FavoriteDataArray::iterator 	it;

	for (it = m_aryData.begin(); it != m_aryData.end(); ++it)
		it->bRef = FALSE;

	_FavoriteItemCollector		funcClct(strDirPath, m_aryData);
	MtlForEachObject_OldShell(strDirPath, funcClct);
	int 						nValidCount = 0;
	it = m_aryData.begin();

	while ( it != m_aryData.end() ) {
		if (!it->bRef) {
			for (FavoriteDataArray::iterator it2 = m_aryData.begin(); it2 != m_aryData.end(); ++it2)
				if ( (int) it2->dwPosition > (int) it->dwPosition )
					it2->dwPosition--;

			it = m_aryData.erase(it);
		} else {
			if (it->dwPosition != FAVORITESORDER_NOTFOUND)
				nValidCount++;

			++it;
		}
	}

	//�\�[�g����
	_FavoritesMenuNameCompare	funcComp(strDirPath);
	std::sort(m_aryData.begin(), m_aryData.end(), funcComp);

	//�ǉ������f�[�^�ɔԍ�������
	int 						j			= nValidCount;
	int 						nCount		= (int)m_aryData.size();

	for (int i = 0; i < nCount; i++) {
		if (m_aryData[i].dwPosition == FAVORITESORDER_NOTFOUND) {
			m_aryData[i].dwPosition = j;
			j++;
		}
	}

	return TRUE;
}



///�w�肵�����O�������ڂ̔ԍ���Ԃ�
int CFavoriteOrderHandler::FindName(const CString &strFileName)
{
	CString 	strName = MtlGetFileName(strFileName);
	int 		nCount	= (int)m_aryData.size();

	for (int i = 0; i < nCount; i++) {
		if (strName == m_aryData[i].strName)
			return i;
	}

	return -1;
}



/// �w�肵���ʒu�ɂ��鍀�ڂ̔ԍ���Ԃ�
int CFavoriteOrderHandler::FindPosition(DWORD dwPosition)
{
	int nCount = (int)m_aryData.size();
	for (int i = 0; i < nCount; i++) {
		if (dwPosition == m_aryData[i].dwPosition)
			return i;
	}

	return -1;
}



CString CFavoriteOrderHandler::GetOrderKeyName(const CString &strDirPath)
{
	CString 	strFavRoot;

	MtlGetFavoritesFolder(strFavRoot);
	MtlMakeSureTrailingBackSlash(strFavRoot);

	CString 	strDir		= strDirPath;
	MtlMakeSureTrailingBackSlash(strDir);

	if (strDir.Find(strFavRoot) == -1)
		return _T("");

	CString 	strKeyName	= REGKEY_IE_MENUORDER_ROOT;
	CString 	strRelative = strDirPath.Right( strDirPath.GetLength() - strFavRoot.GetLength() );

	if ( !strRelative.IsEmpty() )
		strKeyName += _T('\\') + strRelative;

	MtlRemoveTrailingBackSlash(strKeyName);

	return strKeyName;
}

