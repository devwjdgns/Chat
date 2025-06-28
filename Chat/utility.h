#pragma once
#include <nlohmann/json.hpp>
#include <iostream>
#include <codecvt>
#include <locale>


std::string convertString(const CString& str);
CString convertString(const std::string& str);

void saveToRegistry(const CString& key, const CString& value);
CString loadFromRegistry(const CString& key);

CString trimFromAffix(const CString& str, const CString& prefix);
CString trimFromAffix(const CString& str, const CString& prefix, const CString& suffix);