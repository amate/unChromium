/**
 *	@file FileNotification.h
 *	@brief	�t�@�C���̍X�V�̊Ď�. (�����N�o�[��p?)
 *	@note	�ύX�̒ʒm�͕ʃX���b�h������s�����̂Œ���
 */

#pragma once

#include <boost/thread.hpp>


class CFileNotification 
{
public:
	// Ctor/Dtor
	CFileNotification()
		: m_hNotification(INVALID_HANDLE_VALUE)
		, m_hExitEvent(INVALID_HANDLE_VALUE)
	{	}


	~CFileNotification()
	{
		_CleanUpNotification();
	}

	void SetFileNotifyFunc(function<void ()> func) { m_funcNotify = func; }

	bool SetUpFileNotification(const CString& strDirPath, bool bWatchSubTree = false)
	{
		if (m_hExitEvent != INVALID_HANDLE_VALUE)
			return false;	// �Ď������s���ł���
		ATLASSERT(m_funcNotify);

		m_hExitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		ATLASSERT(m_hExitEvent != INVALID_HANDLE_VALUE);

		m_hNotification = ::FindFirstChangeNotification(  strDirPath,
														bWatchSubTree,													// flag for monitoring
														FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);	// fixed by INUYA, thank you.
		ATLASSERT(m_hNotification != INVALID_HANDLE_VALUE);
		if (m_hNotification == INVALID_HANDLE_VALUE) 			// can't find the Link directory
			return false;

		m_thread_Notification = boost::thread( boost::bind(&CFileNotification::_threadFileNotification, this) );

		return true;
	}

	void StopFileNotification() { _CleanUpNotification(); }

private:
	void _CleanUpNotification()
	{
		if (m_hExitEvent == INVALID_HANDLE_VALUE)
			return;

		ATLVERIFY( ::SetEvent(m_hExitEvent) );

		if (!m_thread_Notification.timed_join(boost::posix_time::milliseconds(DONUT_THREADWAIT)))
			ATLASSERT(FALSE);

		::CloseHandle(m_hExitEvent);
		m_hExitEvent = INVALID_HANDLE_VALUE;
		::FindCloseChangeNotification(m_hNotification);
		m_hNotification = INVALID_HANDLE_VALUE;
	}

	void _threadFileNotification()
	{
		HANDLE handles[] = { m_hExitEvent, m_hNotification };
		for (;;) {
			DWORD dwResult = ::WaitForMultipleObjects(_countof(handles), handles, FALSE, INFINITE);
			if (dwResult == WAIT_OBJECT_0)
				break;	// �X���b�h���I��������
			else if (dwResult == WAIT_OBJECT_0 + 1) {
				ATLASSERT(m_funcNotify);
				m_funcNotify();	// �ύX�ʒm
				::FindNextChangeNotification(m_hNotification);
			} else {
				ATLASSERT(FALSE);
				break;
			}
		}
	}

	// Data members
	function<void ()>	m_funcNotify;
	HANDLE	m_hNotification;
	HANDLE	m_hExitEvent;
	boost::thread m_thread_Notification;
};
