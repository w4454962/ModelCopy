#pragma once

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <inttypes.h>
#include <string>
#include <vector>

#include <filesystem>

#ifdef _HAS_CXX17
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;

#endif

std::string a2u(std::string const& strGb2312);
std::string u2a(std::string const& strUtf8);
bool CopyFilesToClip(std::vector<std::string>& fileList);