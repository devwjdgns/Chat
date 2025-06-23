#include "pch.h"
#include "framework.h"
#include "utility.h"

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
