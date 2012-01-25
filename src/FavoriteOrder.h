/**
 *	@file	FavoriteOrder.h
 *	@brief	���W�X�g���ɂ��邨�C�ɓ�����𑀍삵�ĕ��я�������������N���X
 */
#pragma once

#include "ItemIDList.h"
#include "MtlFile.h"
#include "MtlWeb.h"


#define REGKEY_IE_MENUORDER_ROOT	_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Favorites")



/// ���W�X�g���ɂ��邨�C�ɓ�����𑀍삵�ĕ��я�������������N���X
/// ��������Ƀf�[�^��j�����Ȃ��悤�ɋC�����邱��
class CFavoriteOrderHandler {
private:
	enum EIe_MenuOrder {
		IE_MENUORDER_ITEM		= 0x05,
		IE_MENUORDER_FOLDER 	= 0x0D,
		IE_MENUORDER_ZERO		= 0x00,

		IE_MENUORDER_UNKNOWN1	= 2,
		IE_MENUORDER_UNKNOWN2	= 1,
	};

	struct OrderDataHeader {
		DWORD		dwSize8;				///< 8�Œ�
		DWORD		dwUnknown;				///< �s�� �l�͏��2
		DWORD		dwFullSizem8;			///< �f�[�^�S�̂̃T�C�Y-8
		DWORD		dwUnknown2; 			///< �s�� �l�͏��1
		DWORD		dwItemCount;			///< �A�C�e���̌�
	};

	struct OrderItemTemplate {
		DWORD		dwSize;
		DWORD		dwPosition;
		ITEMIDLIST	idlName;
	};

	struct OrderItemData {
		DWORD		dwSize; 				///< ���ڂ̃f�[�^�T�C�Y
		DWORD		dwPosition; 			///< ���ڂ̈ʒuZero Origin
		CItemIDList idlName;				///< �t�@�C�����݂̂�ITEMIDLIST
		BOOL		bPadding;
		DWORD		dwFlag;
		CString 	strName;				///< �t�@�C����
		BOOL		bRef;

	  #if 1 //+++
		OrderItemData()
			: dwSize(0), dwPosition(0), idlName(), bPadding(0), dwFlag(0), strName(), bRef(0)
		{
		}
	  #endif
	};

	enum { s_itemsize_without_idl = 12 };	///< idlName���������\���̃T�C�Y


	typedef std::vector<OrderItemData>	FavoriteDataArray;

private:
	FavoriteDataArray	m_aryData;
	CString 			m_strDirPath;
	CString 			m_strRegKey;

public:
	// CFavoriteOrderHandler()  {}
	// ~CFavoriteOrderHandler() {}

	/// ���W�X�g������f�[�^��ǂݍ���
	int		ReadData(const CString &strDirPath);
	BOOL	MoveData(DWORD dwSrcPos, DWORD dwDstPos);
	BOOL	MoveData(CFavoriteOrderHandler& srcOrder, const CString &srcPath, DWORD dstPos);

	/// �f�[�^�����W�X�g���ɏ�������
	BOOL	SaveData();

	/// �f�[�^���\�z����
	BOOL	ReConstructData(CString strDirPath);

	/// �w�肵�����O�������ڂ̔ԍ���Ԃ�
	int 	FindName(const CString &strFileName);

	///+++ �w�肵�����O�������ڂ̈ʒu��Ԃ�
	int 	FindNameToPosition(const CString &strFileName) {
		int n = FindName(strFileName);
		if (n >= 0)
			return m_aryData[ n ].dwPosition;
		return -1;
	}


	/// �w�肵���ʒu�ɂ��鍀�ڂ̔ԍ���Ԃ�
	int 	FindPosition(DWORD dwPosition);

	CString GetOrderKeyName(const CString &strDirPath);

	void	setAryData_position(int idx, DWORD pos) { m_aryData[idx].dwPosition = pos; }

	const CString& DirPath() const { return  m_strDirPath; }	//+++ �����o�[�ϐ����B��


	/// �f�[�^��ǉ�����
	BOOL	AddData(CString strFileName, DWORD dwPosition); 	// m_strDirPath����̑��΃p�X�Ŏw��
	BOOL	DeleteData(CString strFileName, bool bSw=false);

private:

	struct _FavoriteItemCollector {
		FavoriteDataArray&	_aryData;
		CString &			_strDirPath;
		CItemIDList 		_idlFolder;

		_FavoriteItemCollector(CString &strDirPath, FavoriteDataArray &aryData)
			: _aryData(aryData)
			, _strDirPath(strDirPath)
			, _idlFolder( _strDirPath )
		{
		}


		void operator ()(const CString &strPath, bool bDir)
		{
			OrderItemData	item;

			item.strName	= MTL::MtlGetFileName(strPath);
			FavoriteDataArray::iterator it;
			for (it = _aryData.begin(); it != _aryData.end(); ++it) {
				if (item.strName == it->strName) {
					it->bRef = TRUE;
					return;
				}
			}

			item.idlName	= strPath;
			item.idlName   -= _idlFolder;
			item.dwPosition = FAVORITESORDER_NOTFOUND; //��ŏ��ԕt�����邽�߂̓���l
			int 	nSize	= item.idlName.GetSize();
			item.bPadding	= (nSize & 1);		//+++ ? TRUE : FALSE;
			item.dwSize 	= nSize + s_itemsize_without_idl + (item.bPadding /*+++ ? 1 : 0*/);
			item.dwFlag 	= bDir ? IE_MENUORDER_FOLDER : IE_MENUORDER_ITEM;
			item.bRef		= TRUE;
			_aryData.push_back(item);
		}
	};


	struct _FavoritesMenuNameCompare : public std::binary_function<const OrderItemData &, const OrderItemData &, bool> {
		CString 	_strDirPath;

		_FavoritesMenuNameCompare(CString strDirPath)
			: _strDirPath(strDirPath)
		{
		}


		bool operator ()(const OrderItemData &x, const OrderItemData &y) const
		{
			CItemIDList idlDir = _strDirPath;
			CItemIDList idlX   = idlDir 	;	idlX += x.idlName;
			CItemIDList idlY   = idlDir 	;	idlY += y.idlName;
			CString 	strX   = idlX.GetPath();
			CString 	strY   = idlY.GetPath();

			if ( MtlIsDirectory(strX) ) {
				if ( MtlIsDirectory(strY) ) {
					return ::lstrcmp(strX, strY) < 0;
				} else {
					return true;
				}
			} else {
				if ( MtlIsDirectory(strY) ) {
					return false;
				} else {
					return ::lstrcmp(strX, strY) < 0;
				}
			}
		}
	};

  #if 0	//+++ ���s
	struct _FavoritesMenuPosCompare : public std::binary_function<const OrderItemData &, const OrderItemData &, bool> {
		CString 	_strDirPath;

		_FavoritesMenuPosCompare(CString strDirPath)
			: _strDirPath(strDirPath)
		{
		}


		bool operator ()(const OrderItemData &x, const OrderItemData &y) const
		{
			return (x.dwPosition < y.dwPosition && x.dwPosition >= 0);
		}
	};
  #endif
};
