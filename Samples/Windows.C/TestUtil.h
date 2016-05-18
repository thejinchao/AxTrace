#pragma once

std::string convertToMultibyte(const wchar_t* szSource);
std::string convertWideToUtf8(const wchar_t* szSource);
std::wstring convertToWide(const char* szSource);
std::string trimstring(const std::string& strIn);
std::string convertSafeString(const wchar_t* szSource);
