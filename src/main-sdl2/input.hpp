#pragma once

// SDL2 ドライバ 入力システム
//
// 日本語入力に対応する場合、SDL_KeyboardEvent と SDL_TextInputEvent を適宜使い
// 分ける必要がある。

#include <string>

#include <SDL.h>

// キーボードイベント ev をゲーム側へ送信するキーシーケンスに変換する。
// 認識しないイベントに対しては空シーケンスを返す。
std::string key_sequence(const SDL_KeyboardEvent &ev);

// テキスト入力イベント ev をゲーム側へ送信するキーシーケンスに変換する。
// 認識しないイベントに対しては空シーケンスを返す。
std::string key_sequence(const SDL_TextInputEvent &ev);
