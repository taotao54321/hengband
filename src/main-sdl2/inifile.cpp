#include "autoconf.h"
#ifdef USE_SDL2

#include <fstream>
#include <map>
#include <string>

#include <boost/algorithm/string/trim.hpp>

#include "main-sdl2/inifile.hpp"
#include "main-sdl2/prelude.hpp"

IniMap inifile_parse(const std::string &path)
{
    std::ifstream in(path, std::ios::in);
    if (!in) {
        EPRINTLN("cannot open '{}'", path);
        return {};
    }

    IniMap map;

    std::string sect_name;
    std::map<std::string, std::string> sect_map;

    int i_line = 0;
    std::string line;
    while (std::getline(in, line)) {
        i_line += 1;

        // '#' 以降はコメントなので除去
        if (const auto i_comment = line.find('#'); i_comment != std::string::npos)
            line.resize(i_comment);

        // 行頭/行末の空白を除去
        boost::trim(line);

        // 空行は無視
        if (line.empty())
            continue;

        // [Section]
        if (line.front() == '[' && line.back() == ']') {
            // セクション名を得る。空の場合はエラー
            const auto sect_name_new = line.substr(1, std::size(line) - 2);
            if (sect_name_new.empty()) {
                EPRINTLN("{}: Line {}: section name is empty", path, i_line);
                return {};
            }
            // 以前のセクションを記録
            if (!sect_name.empty())
                map[sect_name].insert(std::begin(sect_map), std::end(sect_map));
            // 新しいセクションへ移行
            sect_name = sect_name_new;
            sect_map.clear();
            continue;
        }

        // key = value
        if (const auto i_equal = line.find('='); i_equal != std::string::npos) {
            // セクションがなければエラー
            if (sect_name.empty()) {
                EPRINTLN("{}: Line {}: key=value without section: {}", path, i_line, line);
                return {};
            }
            // (key,value) を得る。ともに先頭/末尾の空白は除去
            auto key = line.substr(0, i_equal);
            auto value = line.substr(i_equal + 1);
            boost::trim(key);
            boost::trim(value);
            // key が空ならエラー
            if (key.empty()) {
                EPRINTLN("{}: Line {}: key name is empty: {}", path, i_line, line);
                return {};
            }
            // 現在のセクションに (key,value) を追加
            sect_map[key] = value;
            continue;
        }

        EPRINTLN("{}: Line {}: cannot parse: {}", path, i_line, line);
        return {};
    }

    // 最後のセクションを記録
    if (!sect_name.empty())
        map[sect_name].insert(std::begin(sect_map), std::end(sect_map));

    return map;
}

#endif // USE_SDL2
