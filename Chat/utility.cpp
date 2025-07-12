#include "pch.h"
#include "framework.h"
#include "utility.h"

CString getCurrentDateTimeString()
{
	time_t now = time(nullptr);
	struct tm localTime;
	localtime_s(&localTime, &now);

	CString strTime;
	strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),
		localTime.tm_year + 1900,
		localTime.tm_mon + 1,
		localTime.tm_mday,
		localTime.tm_hour,
		localTime.tm_min,
		localTime.tm_sec);

	return strTime;
}

std::string convertString(const CString& str)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	std::string utf8(len, 0);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, &utf8[0], len, NULL, NULL);
	return utf8;
}

CString convertString(const std::string& str)
{
	wchar_t wbuffer[1024];
	int wlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wbuffer, 1024);
	return CString(wbuffer);
}

void saveToRegistry(const CString& key, const CString& value)
{
	AfxGetApp()->WriteProfileString(_T("ChatUtilSettings"), key, value);
}

CString loadFromRegistry(const CString& key)
{
	return AfxGetApp()->GetProfileString(_T("ChatUtilSettings"), key, _T(""));
}

CString trimFromAffix(const CString& str, const CString& prefix)
{
	if (prefix.IsEmpty())
	{
		return str;
	}
	int pos = str.Find(prefix);
	if (pos != -1)
	{
		CString tmp = str.Mid(pos + prefix.GetLength());
		return tmp;
	}
	return CString();
}

CString trimFromAffix(const CString& str, const CString& prefix, const CString& suffix)
{
	if (suffix.IsEmpty())
	{
		return str;
	}
	int pos = str.Find(suffix);
	if (pos != -1)
	{
		CString tmp = str.Left(pos);
		return trimFromAffix(tmp, prefix);
	}
	return CString();
}