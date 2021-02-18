#pragma once

#include <string_view>
#include <utility>

// 失敗時に中断する変換関数
//
// (変換後文字列, 変換元文字列内で変換に成功した Byte 数) を返す。

std::pair<std::string, std::size_t> euc_to_utf8(std::string_view euc);
std::pair<std::string, std::size_t> utf8_to_euc(std::string_view utf8);

// 失敗を無視する変換関数
//
// 変換後文字列を返す。
// 変換できない Byte は ch_replace に置換する。

std::string euc_to_utf8_lossy(std::string_view euc, char ch_replace);
std::string utf8_to_euc_lossy(std::string_view utf8, char ch_replace);

// UTF-8 文字列の最初の文字のバイト数を返す。
// 空文字列もしくは非 UTF-8 文字列が渡されたら 0 を返す。
int utf8_char_byte_count(std::string_view utf8);
