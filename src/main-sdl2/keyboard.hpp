#pragma once

#include <string>

#include <SDL.h>

// キーボードイベント ev をゲーム側へ送信するキーシーケンスに変換する。
// 認識しないキーボードイベントに対しては空シーケンスを返す。
std::string key_sequence(const SDL_KeyboardEvent &ev);
