#include <iostream>
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

static bool GetGameDataDir(_Out_ std::wstring& path)
{
	WCHAR* buf = (WCHAR*)alloca(MAX_PATH);
	GetEnvironmentVariable(L"USERPROFILE", buf, MAX_PATH);

	if (DWORD ec = GetLastError()) {
		std::wcout << L"Error code: " << ec << std::endl;
		return false;
	}

	std::wstring logDirCN = std::wstring(buf) + L"\\AppData\\LocalLow\\miHoYo\\Ô­Éñ\\output_log.txt";
	std::wstring logDirGLB = std::wstring(buf) + L"\\AppData\\LocalLow\\miHoYo\\Genshi Impact\\output_log.txt";

	std::wifstream fs(logDirCN);

	if (fs.fail()) {
		fs.close();
		fs.open(logDirGLB);
		if(fs.fail()) {
			fs.close();
			std::wcout << L"Unable to open log file" << std::endl;
			return false;
		}
	}

	std::wstring line;
	std::wstringstream ss;
	while (std::getline(fs, line))
		ss << line << std::endl;

	fs.close();

	std::wregex pattern(L"[[:space:]].:/.+(GenshinImpact_Data|YuanShen_Data)");
	std::wsmatch res;
	auto s = ss.str();
	if (!std::regex_search(s, res, pattern))
	{
		std::wcout << L"Unable to find game path" << std::endl;
		return false;
	}
	s = res[0].str();
	pattern = L".:/.+(GenshinImpact_Data|YuanShen_Data)";
	std::regex_search(s, res, pattern);
	path = res[0];
	return true;
}

static bool GetPullURL(_In_ std::wstring gameDataPath, _Out_ std::wstring& pullURL)
{
	std::wstring cacheFile = gameDataPath + L"\\webCaches\\Cache\\Cache_Data\\data_2";
	
	WCHAR* tempPath = (WCHAR*)alloca(MAX_PATH);

	GetEnvironmentVariable(L"TEMP", tempPath, MAX_PATH);

	std::wstring tempCacheFile = tempPath + std::wstring(L"\\genshi_data_tmp");
	if (!CopyFile(cacheFile.c_str(), tempCacheFile.c_str(), false))
	{
		if (DWORD ec = GetLastError()) {
			std::wcout << L"Error code: " << ec << std::endl;
			return false;
		}
	}

	std::wifstream fs(tempCacheFile, std::ios::binary);

	if (fs.fail())
	{
		fs.close();
		std::wcout << L"Unable to open cache file" << std::endl;
		return false;
	}

	std::wstring buf;
	WCHAR byte;
	WCHAR byteArray[4] = {0, 0, 0, 0};
	int idx = 0;
	bool inUrl = false;
	while (fs.good())
	{
		fs.read(&byte, 1);
		if (byteArray[0] == '1' && byteArray[1] == '/' && byteArray[2] == '0' && byteArray[3] == '/')
			inUrl = true;
		else if (byte == 0)
			inUrl = false;
		
		if(inUrl)
			buf += byte;

		byteArray[idx] = byte;
		idx = (idx + 1) % 4;
	}
	fs.close();

	std::wregex pattern(L"https://webstatic.mihoyo.com/.+?game_biz=hk4e_(global|cn)");
	std::wsregex_iterator end;
	std::wsregex_iterator it(buf.begin(), buf.end(), pattern);
	if (it == end) {
		std::wcout << L"Unable to find pull url" << std::endl;
		return false;
	}

	for(it; it != end;it++)
		pullURL = it->str(0) + L"#/log";

	return true;
}

int main()
{
	std::wcout << L"Please wait..." << std::endl;
	std::wstring gameDataDir, pullUrl;
	if (!GetGameDataDir(gameDataDir)) return 0;
	if (!GetPullURL(gameDataDir, pullUrl)) return 0;

	std::wcout << L"Your latest pull record url: \n" << pullUrl << std::endl;

	getchar();
}