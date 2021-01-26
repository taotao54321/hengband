#pragma once

#include <optional>
#include <string>

std::optional<std::string> euc_to_utf8(const std::string &euc);
std::optional<std::string> utf8_to_euc(const std::string &utf8);

// TODO:
//   lossy な変換関数も欲しいが実装が面倒。
//   iconvctl() は環境によっては使えない模様。
