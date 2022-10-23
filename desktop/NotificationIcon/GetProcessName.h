#pragma once

std::wstring GetProgrameNameFromPath(const std::wstring& processPath)  throw (HRESULT);
std::wstring GetProcessNameFromID(DWORD processId) throw (HRESULT);
