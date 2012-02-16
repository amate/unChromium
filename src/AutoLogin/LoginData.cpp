/**
*	@file	LoginData.h
*	@brief	ログイン情報を管理します
*/

#include "stdafx.h"
#include "LoginData.h"
#include <fstream>
#include <regex>
#include "../Misc.h"

using namespace std;

#pragma comment (lib, "crypt32.lib")


/////////////////////////////////////////////
// CLoginDataManager

#define AUTOLOGINVERSION	_T("2.0")

// 定義
vector<LoginInfomation>	CLoginDataManager::s_vecLoginInfo;
CString	CLoginDataManager::s_strLoginDataPath;


//------------------------
/// 暗号化したものを読み込む
void	CLoginDataManager::Load()
{
	if (s_strLoginDataPath.IsEmpty()) {
		s_strLoginDataPath = Misc::GetExeDirectory() + _T("AutoLoginData");
	}

	s_vecLoginInfo.reserve(20);

	if (::PathFileExists(s_strLoginDataPath) == FALSE)
		return ;

	/* 暗号化されたファイルの読み込み */
	vector<char>	data;
	if (_ReadData(s_strLoginDataPath, data) == false)
		return;
	
	/* 復号化 */
	DATA_BLOB	DataOut;
	DATA_BLOB	DataIn;
	DataIn.cbData	= data.size();
	DataIn.pbData	= (BYTE*)data.data();
	if (!CryptUnprotectData(&DataIn, NULL, NULL, NULL, NULL, 0, &DataOut)) {
		MessageBox(NULL, _T("データの復号化に失敗"), NULL, 0);
		// 復号化に失敗したファイルをリネームする
		CString strRenamedFilePath = s_strLoginDataPath + _T("_Unprotectfaild");
		MoveFileEx(s_strLoginDataPath, strRenamedFilePath, MOVEFILE_REPLACE_EXISTING);
		return ;
	}
	std::wstring strData = (LPCWSTR)DataOut.pbData;
	LocalFree(DataOut.pbData);

	/* s_vecLoginInfo に設定 */
	_DeSerializeLoginData(strData);
}


//-----------------------
/// 暗号化して保存する
void	CLoginDataManager::Save()
{
	/* s_vecLoginInfo を直列化 */
	std::wstring strData;
	_SerializeLoginData(strData);

	/* 暗号化 */
	DATA_BLOB	DataOut;
	DATA_BLOB	DataIn;
	DataIn.cbData = (strData.length() * sizeof(TCHAR)) + sizeof(TCHAR);
	DataIn.pbData = (BYTE*)strData.c_str();
	if (!CryptProtectData(&DataIn, L"unDonutAutoLoginData", NULL, NULL, NULL, 0, &DataOut)) {
		MessageBox(NULL, _T("データの暗号化に失敗しました"), NULL, 0);
		return ;
	}

	/* 暗号化したデータをファイルに書き込み */
	_WriteData(s_strLoginDataPath, DataOut.pbData, DataOut.cbData);
}

//-----------------------
void	CLoginDataManager::Import(LPCTSTR strPath)
{
	vector<char> data;
	if (_ReadData(strPath, data)) {
		/* s_vecLoginInfo に設定 */
		std::wstring strData = (LPCWSTR)data.data();
		_DeSerializeLoginData(strData);
	}
}

//-----------------------
void	CLoginDataManager::Export(LPCTSTR strPath)
{
	/* s_vecLoginInfo を直列化 */
	std::wstring strData;
	_SerializeLoginData(strData);

	/* 暗号化したデータをファイルに書き込み */
	_WriteData(strPath, (BYTE*)strData.c_str(), (strData.length() * sizeof(WCHAR)) + sizeof(WCHAR));
}


//-------------------------------------
/// マッチするログインURLのインデックスを返す
int		CLoginDataManager::Find(LPCTSTR strUrl)
{
	CString strLoginUrl = strUrl;
	strLoginUrl.MakeLower();

	int nCount = s_vecLoginInfo.size();
	for (int i = 0; i < nCount; ++i) {
		if (strLoginUrl.GetLength() < s_vecLoginInfo[i].strLoginUrl.GetLength())
			continue;
		if (strLoginUrl.Left(s_vecLoginInfo[i].strLoginUrl.GetLength()) == s_vecLoginInfo[i].strLoginUrl)
			return i;
	}
	
	return -1;
}

//----------------------------------
void	CLoginDataManager::GetNameValueMap(int nIndex, NameValueMap*& pmap)
{
	pmap = &s_vecLoginInfo[nIndex].mapNameValue;
}

//---------------------------------
void	CLoginDataManager::GetCheckboxMap(int nIndex, CheckboxMap*& pmapCheckbox)
{
	pmapCheckbox = &s_vecLoginInfo[nIndex].mapCheckbox;
}


bool	CLoginDataManager::_ReadData(LPCTSTR strPath, vector<char>& vecData)
{
	std::ifstream	fstream(strPath, std::ios::in | std::ios::binary);
	if (fstream.fail()) {
		CString strMsg;
		strMsg.Format(_T("ファイルのオープンに失敗しました\n「%s」"), strPath);
		MessageBox(NULL, strMsg, NULL, 0);
		return false;
	}
	fstream.seekg(0, std::ios::end); 
	long filesize = (long)fstream.tellg();
	fstream.seekg(0, std::ios::beg);

	vecData.resize(filesize);
	fstream.read(vecData.data(), filesize);
	if (fstream.fail()) {
		MessageBox(NULL, _T("ファイルの読み込みに失敗"), NULL, 0);
		return false;
	}
	return true;
}

//----------------------------------
/// strDataからログイン情報に追加する
void	CLoginDataManager::_DeSerializeLoginData(const std::wstring& strData)
{
	s_vecLoginInfo.clear();

	/* 改行単位にばらす */
	vector<std::wstring>	vecLine;
	int nStart = 0;
	int nCount = (int)strData.size();
	for (int i = 0; i < nCount; ++i) {
		if (strData[i] == L'\n') {
			int nLength = i - nStart;	// 改行を含めない長さ
			if (nLength == 0)
				continue;
			vecLine.push_back(strData.substr(nStart, nLength));
			nStart = i + 1;	// 改行を飛ばす
			if (nStart == nCount)
				break;	// 終わり
		}
	}
	/* s_vecLoginInfo に追加 */
	for (auto it = vecLine.cbegin(); it != vecLine.cend(); ++it) {
		wregex	rxUrl(L"\\[(.*?)\\]");
		wsmatch	match;
		if (std::regex_match(*it, match, rxUrl)) {
			LoginInfomation	info;
			info.strLoginUrl = match.str(1).c_str();
			++it;
			if (it == vecLine.cend())
				break;	// 終わり

			wregex	rxNameValue(L"([^=]+)=(.*)");
			wsmatch	match2;
			while (std::regex_match(*it, match2, rxNameValue)) {
				CString strName = match2.str(1).c_str();
				if (strName.Left(9) == _T("checkbox:")) {
					info.mapCheckbox.insert(std::make_pair(strName.Mid(9), match2.str(2) == L"true"));
				} else {
					info.mapNameValue.insert(std::make_pair(match2.str(1).c_str(), match2.str(2).c_str()));
				}
				++it;
				if (it == vecLine.cend())
					break;	// 終わり
			}
			--it;	// 進めすぎたので戻しておく
			s_vecLoginInfo.push_back(info);
		}
	
	}
}

//----------------------------------------------
/// ログイン情報をstrDataに追加する
void	CLoginDataManager::_SerializeLoginData(std::wstring& strData)
{
	for (auto it = s_vecLoginInfo.cbegin(); it != s_vecLoginInfo.cend(); ++it) {
		CString temp;
		temp.Format(_T("[%s]\n"), it->strLoginUrl);
		strData += temp;
		for (auto itmap = it->mapNameValue.cbegin(); itmap != it->mapNameValue.cend(); ++itmap) {
			temp.Format(_T("%s=%s\n"), itmap->first, itmap->second);
			strData += temp;
		}
		for (auto itcheck = it->mapCheckbox.cbegin(); itcheck != it->mapCheckbox.cend(); ++itcheck) {
			if (itcheck->first.IsEmpty() == FALSE) {
				temp.Format(_T("checkbox:%s=%s\n"), itcheck->first, itcheck->second ? _T("true") : _T("false"));
				strData += temp;
			}
		}
	}
}

bool	CLoginDataManager::_WriteData(LPCTSTR strPath, BYTE* pData, DWORD size)
{
	std::fstream fstream(strPath, std::ios::out | std::ios::binary);
	if (fstream.fail()) {
		CString strMsg;
		strMsg.Format(_T("ファイルのオープンに失敗しました\n「%s」"), strPath);
		MessageBox(NULL, strMsg, NULL, 0);
		return false;
	}
	fstream.write((LPCSTR)pData, size);
	if (fstream.fail()) {
		MessageBox(NULL, _T("ファイルの書き込みに失敗"), NULL, 0);
		return false;
	}
	return true;
}




////////////////////////////////////////
// CLoginInfoEditDialog : ログイン情報編集ダイアログ


CLoginInfoEditDialog::CLoginInfoEditDialog(const LoginInfomation& info) 
	: m_info(info)
{	}


//-------------------------------
/// ダイアログ初期化
BOOL	CLoginInfoEditDialog::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	/* タイトル変更 */
	CString strtemp;
	int nLen = GetWindowTextLength() + 1;
	GetWindowText(strtemp.GetBuffer(nLen), nLen);
	strtemp.ReleaseBuffer();
	CString strTitle;
	CString strVersion = AUTOLOGINVERSION;
	strTitle.Format(_T("%s - ver %s"), strtemp, strVersion);
	SetWindowText(strTitle);

	_SetLoginInfoData();

	for (auto it = s_vecLoginInfo.cbegin(); it != s_vecLoginInfo.cend(); ++it) {
		m_UrlList.AddString(it->strLoginUrl);
	}
	return 0;
}

//------------------------------
void	CLoginInfoEditDialog::OnApply(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nIndex = m_UrlList.GetCurSel();
	if (nIndex == -1)
		return ;

	_SetCopyLoginInfo();
	LoginInfomation& info = s_vecLoginInfo[nIndex];
	info = m_info;

	m_UrlList.DeleteString(nIndex);
	m_UrlList.InsertString(nIndex, m_Url);
	m_UrlList.SetCurSel(nIndex);
}

//------------------------------
void	CLoginInfoEditDialog::OnAdd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	_SetCopyLoginInfo();
	int nIndex = m_UrlList.AddString(m_Url);
	m_UrlList.SetCurSel(nIndex);
	s_vecLoginInfo.push_back(m_info);
}

//------------------------------
void	CLoginInfoEditDialog::OnDelete(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nIndex = m_UrlList.GetCurSel();
	if (nIndex == -1)
		return;

	s_vecLoginInfo.erase(s_vecLoginInfo.begin() + nIndex);
	m_UrlList.DeleteString(nIndex);
	if (nIndex < m_UrlList.GetCount() || 0 <= --nIndex) {
		m_UrlList.SetCurSel(nIndex);
		OnUrlListChange(0, 0, NULL);
	} else {
		OnNew(0, 0, NULL);
	}
}

//------------------------------
void	CLoginInfoEditDialog::OnNew(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_UrlList.SetCurSel(-1);

	m_info.strLoginUrl.Empty();
	m_info.mapNameValue.clear();
	_SetLoginInfoData();
}

//------------------------------
/// ファイルからログイン情報をインポートする
void	CLoginInfoEditDialog::OnImport(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CFileDialog dlg(TRUE, _T("txt"), _T("AutoLoginDataFile"), OFN_HIDEREADONLY | OFN_CREATEPROMPT,
		_T("テキスト ファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0\0"));
	if (dlg.DoModal() == IDOK) {
		Import(dlg.m_szFileName);
		while (m_UrlList.DeleteString(0) != -1) ;	
		for (auto it = s_vecLoginInfo.cbegin(); it != s_vecLoginInfo.cend(); ++it) {
			m_UrlList.AddString(it->strLoginUrl);
		}
		OnNew(0, 0, NULL);
	}

}

//------------------------------
/// ファイルにログイン情報をエクスポートする
void	CLoginInfoEditDialog::OnExport(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CFileDialog dlg(FALSE, _T("txt"), _T("AutoLoginDataFile"), OFN_OVERWRITEPROMPT,
		_T("テキスト ファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0\0"));
	if (dlg.DoModal() == IDOK) {
		Export(dlg.m_szFileName);
	}

}

//------------------------------
/// 自動ログインテスト
void	CLoginInfoEditDialog::OnTest(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	OnApply(0, 0, NULL);

	m_funcAutoLogin();
}


//------------------------------
void	CLoginInfoEditDialog::OnFinish(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	Save();
	EndDialog(IDOK);
}

//-----------------------------
void CLoginInfoEditDialog::OnUrlListChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nIndex = m_UrlList.GetCurSel();
	if (nIndex == -1)
		return ;

	m_info = s_vecLoginInfo[nIndex];
	_SetLoginInfoData();
}


//-----------------------------
/// 右側にm_infoからデータを設定
void	CLoginInfoEditDialog::_SetLoginInfoData()
{
	m_Url	= m_info.strLoginUrl;
	{		
		for (int i = 0; i < s_cMaxNameValue; ++i) {
			m_Name[i].Empty();
			m_Value[i].Empty();
		}

		auto it = m_info.mapNameValue.begin();
		int nCount = (int)m_info.mapNameValue.size();
		for (int i = 0; i < nCount && i < s_cMaxNameValue; ++i) {
			m_Name[i]	= it->first;
			m_Value[i]	= it->second;
			++it;
		}

	}
	{
		for (int i = 0; i < s_cMaxCheckbox; ++i) {
			m_CheckboxName[i].Empty();
			m_bCheck[i] = false;
		}

		auto it = m_info.mapCheckbox.begin();
		int nCount = (int)m_info.mapCheckbox.size();
		for (int i = 0; i < nCount && i < s_cMaxCheckbox; ++i) {
			m_CheckboxName[i]	= it->first;
			m_bCheck[i]			= it->second;
			++it;
		}
	}
	DoDataExchange(DDX_LOAD);
}

//---------------------------
/// 右側からm_infoにデータを設定
void	CLoginInfoEditDialog::_SetCopyLoginInfo()
{
	DoDataExchange(DDX_SAVE);
	m_info.strLoginUrl	= m_Url;

	m_info.mapNameValue.clear();
	for (int i = 0; i < s_cMaxNameValue; ++i) {
		if (m_Name[i].IsEmpty() == FALSE) {
			m_info.mapNameValue.insert(std::make_pair(m_Name[i], m_Value[i]));
		}
	}

	m_info.mapCheckbox.clear();
	for (int i = 0; i < s_cMaxCheckbox; ++i) {
		if (m_CheckboxName[i].IsEmpty() == FALSE) {
			m_info.mapCheckbox.insert(std::make_pair(m_CheckboxName[i], m_bCheck[i]));
		}
	}
}