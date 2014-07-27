#pragma once
#include <Windows.h>
#include <string>

// 파일이름으로 프로세스를 찾아 그 프로세스가 가진 윈도우를 프론트로 내세운다
bool ActivateProcess(const std::wstring& imageFilename);