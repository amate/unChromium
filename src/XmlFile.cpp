// XmlFile.cpp

#include "stdafx.h"
#include "XmlFile.h"
#pragma comment(lib, "xmllite.lib")


///////////////////////////////////////////////////////////////////////////////
// CXmlFileWrite

// �R���X�g���N�^
CXmlFileRead::CXmlFileRead(const CString &FilePath)
{
	if (FilePath.IsEmpty()) {
		throw _T("�ۑ���̃p�X������܂���");
	}

    if (FAILED(CreateXmlReader(IID_IXmlReader, reinterpret_cast<void**>(&m_pReader), 0))){
        throw _T("CreateXmlReader���s");
    }
	
	// �t�@�C���X�g���[���쐬
    CComPtr<IStream> pStream;
	HRESULT hr = SHCreateStreamOnFile(FilePath, STGM_READ, &pStream);
    if (FAILED(hr)) {
        throw _T("SHCreateStreamOnFile���s");
    }

    if (FAILED(m_pReader->SetInput(pStream))){
        throw _T("SetInput���s");
    }

}

// �f�X�g���N�^
CXmlFileRead::~CXmlFileRead()
{
	//m_pReader->
}

// node == XmlNodeType_Element�̂Ƃ�element��Ԃ�
// MoveToFirstAttribute()����Α�������Ԃ�
CString	CXmlFileRead::GetLocalName()
{
	LPCWSTR	strLocalName;
	if (FAILED(m_pReader->GetLocalName(&strLocalName, NULL))) {
		throw _T("GetLocalName���s");
	}
	return CString(strLocalName);
}

CString	CXmlFileRead::GetValue()
{
	LPCWSTR	strValue;
	if (FAILED(m_pReader->GetValue(&strValue, NULL))) {
		throw _T("GetValue���s");
	}
	return CString(strValue);
}

int		CXmlFileRead::GetValuei()
{
	return _wtoi(GetValue());
}

// externalElement�̓����̗v�f��strElement�ŕԂ�
bool	CXmlFileRead::GetInternalElement(LPCWSTR externalElement, CString &strElement)
{
	XmlNodeType 	nodeType;
	while (m_pReader->Read(&nodeType) == S_OK) {
		if (nodeType == XmlNodeType_Element) {
			strElement = GetLocalName();
			return true;
		} else if (nodeType == XmlNodeType_EndElement) {
			if (GetLocalName() == externalElement) {
				return false;	// </externalElement>�ɂ����̂�
			}
		}
	}
	throw _T("GetInternalElement���s");
	return false;
}


//////////////////////////////////////////////////////////////////////
// CXmlFileRead2

void	CXmlFileRead2::MoveToFirstAttribute()
{
	if (__super::MoveToFirstAttribute() == false)
		throw _T("MoveToFirstAttribute���s");
}

void	CXmlFileRead2::MoveToNextAttribute()
{
	if (__super::MoveToNextAttribute() == false)
		throw _T("MoveToNextAttribute���s");
}





///////////////////////////////////////////////////////////////////////////////
// CXmlFileWrite

// �R���X�g���N�^
CXmlFileWrite::CXmlFileWrite(const CString &FilePath)
{
	if (FilePath.IsEmpty()) {
		throw _T("�ۑ���̃p�X������܂���");
	}

    if (FAILED(CreateXmlWriter(IID_IXmlWriter, reinterpret_cast<void**>(&m_pWriter), 0))){
		throw _T("CreateXmlWriter���s");
    }
   
	// �t�@�C���X�g���[���쐬
    CComPtr<IStream> pStream;
	HRESULT hr = SHCreateStreamOnFile(FilePath, STGM_CREATE | STGM_WRITE, &pStream);
    if (FAILED(hr)) {
        throw _T("SHCreateStreamOnFile���s\n");
    }

    if (FAILED(m_pWriter->SetOutput(pStream))){
        throw _T("SetOutput���s");
    }

    // �C���f���g�L����
    if (FAILED(m_pWriter->SetProperty(XmlWriterProperty_Indent, TRUE))){
        throw _T("SetProperty���s");
    }

    // <?xml version="1.0" encoding="UTF-8"?>
    if (FAILED(m_pWriter->WriteStartDocument(XmlStandalone_Omit))){
        throw _T("WriteStartDocument���s");
    }
}

// �f�X�g���N�^
CXmlFileWrite::~CXmlFileWrite()
{
	// �J���Ă���^�O�����
    if (FAILED(m_pWriter->WriteEndDocument())){
        throw _T("WriteEndDocument���s");
    }

	// �t�@�C���ɏ�������
    if (FAILED(m_pWriter->Flush())){
        throw _T("Flush���s");
    }
}




//	<Element>
void	CXmlFileWrite::WriteStartElement(LPCWSTR Element)
{
    if ( FAILED(m_pWriter->WriteStartElement(NULL, Element, NULL))){
        throw _T("WriteStartElement���s");
    }
}

//	<LocalName>Value</LocalName>
void	CXmlFileWrite::WriteElementString(LPCWSTR LocalName, LPCWSTR Value)
{
	if (FAILED(m_pWriter->WriteElementString(NULL, LocalName, NULL, Value))){
        throw _T("WriteElementString���s");
    }
}

//	<LocalName>Value</LocalName>
void	CXmlFileWrite::WriteElementValue(LPCWSTR LocalName, DWORD Value)
{
	CString temp;
	temp.Format(_T("%d"), Value);
	if (FAILED(m_pWriter->WriteElementString(NULL, LocalName, NULL, temp))){
        throw _T("WriteElementString���s");
    }
}

//	<element	LocalName="Value">
void	CXmlFileWrite::WriteAttributeString(LPCWSTR LocalName, LPCWSTR Value)
{
	if (FAILED(m_pWriter->WriteAttributeString(NULL, LocalName, NULL, Value))){
        throw _T("WriteAttributeString���s");
    }
}

void	CXmlFileWrite::WriteAttributeValue(LPCWSTR LocalName, DWORD dwValue)
{
	CString strValue;
	strValue.Format(_T("%d"), dwValue);
	WriteAttributeString(LocalName, strValue);
}

//	<element>	Text	</element>
void	CXmlFileWrite::WriteString(LPCWSTR	Text)
{
    if (FAILED(m_pWriter->WriteString(Text))){
        throw _T("WriteString���s");
    }
}

//	</Element>
void	CXmlFileWrite::WriteFullEndElement()
{
    if (FAILED(m_pWriter->WriteFullEndElement())){
        throw _T("WriteFullEndElement���s");
    }
}






