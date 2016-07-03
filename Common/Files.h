#pragma once
#include <string>
#include <vector>

bool CreateFolder(const std::string& path);
bool FindFiles(const std::string& wildcard, std::vector <std::string> &files);
bool GetConfigString(const std::string& app, const std::string& key, std::string& value); 