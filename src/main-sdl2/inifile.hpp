#pragma once

// music.cfg / sound.cfg 専用。
// 一般的な INI ファイルには対応しない。

#include <map>
#include <string>

// value = ini_map[section][key]
using IniMap = std::map<std::string, std::map<std::string, std::string>>;

// INI ファイル path をパースし、IniMap を返す。
// 失敗時は標準エラーにその旨表示し、空の IniMap を返す。
IniMap inifile_parse(const std::string &path);
