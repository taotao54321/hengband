#include "autoconf.h"
#ifdef USE_SDL2

#include <optional>
#include <string>
#include <tuple>

#include <SDL.h>

#include "main-sdl2/keyboard.hpp"
#include "main-sdl2/prelude.hpp"

namespace {

bool is_lower_ascii(const SDL_Keycode sym) { return 'a' <= sym && sym <= 'z'; }
bool is_upper_ascii(const SDL_Keycode sym) { return 'A' <= sym && sym <= 'Z'; }
bool is_alpha_ascii(const SDL_Keycode sym) { return is_lower_ascii(sym) || is_upper_ascii(sym); }
bool is_print_ascii(const SDL_Keycode sym) { return '\x20' <= sym && sym <= '\x7E'; }

SDL_Keycode to_upper_ascii(const SDL_Keycode sym) { return is_lower_ascii(sym) ? sym - 0x20 : sym; }

enum class KeyboardLayout { JPN, USA };

// とりあえず JP, US のみ対応。
// 一応 pref の shift キー設定と組み合わせることで任意のレイアウトに対応できそ
// うな気もするが、それなりの作業を要しそうなので手抜きした。
KeyboardLayout detect_keyboard_layout()
{
    // US キーボードの [ が JP キーボードでは @ であることを利用して判定
    if (SDL_GetKeyFromScancode(SDL_SCANCODE_LEFTBRACKET) == SDLK_AT)
        return KeyboardLayout::JPN;
    return KeyboardLayout::USA;
}

SDL_Keycode sym_shifted_jp(const SDL_Keycode sym, const SDL_Scancode code)
{
    // clang-format off
    switch(sym) {
        // layout-specific
        case '1': return '!';
        case '2': return '"';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '&';
        case '7': return '\'';
        case '8': return '(';
        case '9': return ')';
        case '-': return '=';
        case '^': return '~';
        case '@': return '`';
        case '[': return '{';
        case ';': return '+';
        case ':': return '*';
        case ']': return '}';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        case '\\':
            switch(code) {
                case SDL_SCANCODE_INTERNATIONAL3: return '|';
                case SDL_SCANCODE_INTERNATIONAL1: return '_';
                default: return sym;
            }

        default:
            if(is_lower_ascii(sym))
                return to_upper_ascii(sym);
            return sym;
    }
    // clang-format on
}

SDL_Keycode sym_shifted_us(const SDL_Keycode sym, const SDL_Scancode)
{
    // clang-format off
    switch(sym) {
        // layout-specific
        case '1':  return '!';
        case '2':  return '@';
        case '3':  return '#';
        case '4':  return '$';
        case '5':  return '%';
        case '6':  return '^';
        case '7':  return '&';
        case '8':  return '*';
        case '9':  return '(';
        case '0':  return ')';
        case '-':  return '_';
        case '=':  return '+';
        case '\\': return '|';
        case '[':  return '{';
        case ']':  return '}';
        case ';':  return ':';
        case '\'': return '"';
        case ',':  return '<';
        case '.':  return '>';
        case '/':  return '?';

        default:
            if(is_lower_ascii(sym))
                return to_upper_ascii(sym);
            return sym;
    }
    // clang-format on
}

SDL_Keycode sym_shifted(const KeyboardLayout layout, const SDL_Keycode sym, const SDL_Scancode code)
{
    switch (layout) {
    case KeyboardLayout::JPN:
        return sym_shifted_jp(sym, code);
    case KeyboardLayout::USA:
        return sym_shifted_us(sym, code);
    default:
        PANIC("unreachable");
    }
}

// layout に従って ev を変換し、(sym, shift, ctrl, alt) を得る。
std::tuple<SDL_Keycode, bool, bool, bool> apply_keyboard_layout(const KeyboardLayout layout, const SDL_KeyboardEvent &ev)
{
    auto sym = ev.keysym.sym;
    const auto code = ev.keysym.scancode;
    auto shift = bool(ev.keysym.mod & KMOD_SHIFT);
    const auto ctrl = bool(ev.keysym.mod & KMOD_CTRL);
    const auto alt = bool(ev.keysym.mod & KMOD_ALT);

    // Shift キーが押されている場合、Shift+a -> A のようにキーコードが変わりうる。
    // しかし、Shift+F1 などキーコードが変わらないこともある。
    // 前者の場合はキーコードを変更し、shift フラグを下ろす。
    // 後者の場合はキーコード、shift フラグともそのままにする。これにより F1 と
    // Shift+F1 に別のマクロを割り当てることができる。
    if (shift) {
        const auto sym_new = sym_shifted(layout, sym, code);
        if (sym != sym_new) {
            sym = sym_new;
            shift = false;
        }
    }

    return { sym, shift, ctrl, alt };
}

std::optional<char> try_into_char_ctrl(const SDL_Keycode sym)
{
    // clang-format off
    switch(sym) {
        case '[': return '\x1B'; // ESC

        default:
            // Shift+Ctrl で大文字になっている可能性がある
            if(is_alpha_ascii(sym))
                return char(sym & 0x1F);
            return std::nullopt;
    }
    // clang-format on
}

std::optional<char> try_into_char_plain(const SDL_Keycode sym)
{
    // clang-format off
    switch(sym) {
        case SDLK_BACKSPACE: return '\x08';
        case SDLK_TAB:       return '\x09';
        case SDLK_RETURN:    return '\x0D';
        case SDLK_ESCAPE:    return '\x1B';
        case SDLK_DELETE:    return '\x7F';

        default:
            if (is_print_ascii(sym))
                return char(sym);
            return std::nullopt;
    }
    // clang-format on
}

// 入力がマクロトリガー化せずそのまま文字として送信できるならその文字を返す。
// さもなくば std::nullopt を返す。
std::optional<char> try_into_char(const SDL_Keycode sym, const bool shift, const bool ctrl, const bool alt)
{
    // 既に Shift 処理済みなので、Shift+何か は文字として送れない。
    // また、Alt+何か は常に文字として送れない。
    if (shift || alt)
        return std::nullopt;

    if (ctrl)
        return try_into_char_ctrl(sym);

    return try_into_char_plain(sym);
}

// キーコード sym のマクロトリガー用文字列を返す。
// 扱えないキーコードに対しては std::nullopt を返す。
//
// 例えば SDLK_LSHIFT などは明らかに弾く必要があり、全キーコードを素通しにする
// わけにはいかない。とりあえず一般的な特殊キーと ASCII 表示可能文字のみ扱うこ
// とにする。
std::optional<std::string> sym_trigger_name(const SDL_Keycode sym)
{
    // clang-format off
    switch(sym) {
        case SDLK_F1:  return "F1";
        case SDLK_F2:  return "F2";
        case SDLK_F3:  return "F3";
        case SDLK_F4:  return "F4";
        case SDLK_F5:  return "F5";
        case SDLK_F6:  return "F6";
        case SDLK_F7:  return "F7";
        case SDLK_F8:  return "F8";
        case SDLK_F9:  return "F9";
        case SDLK_F10: return "F10";
        case SDLK_F11: return "F11";
        case SDLK_F12: return "F12";
        case SDLK_F13: return "F13";
        case SDLK_F14: return "F14";
        case SDLK_F15: return "F15";
        case SDLK_F16: return "F16";
        case SDLK_F17: return "F17";
        case SDLK_F18: return "F18";
        case SDLK_F19: return "F19";
        case SDLK_F20: return "F20";
        case SDLK_F21: return "F21";
        case SDLK_F22: return "F22";
        case SDLK_F23: return "F23";
        case SDLK_F24: return "F24";

        case SDLK_PAUSE: return "Pause";

        case SDLK_INSERT: return "Insert";

        case SDLK_HOME: return "Home";
        case SDLK_END:  return "End";

        case SDLK_PAGEUP:   return "Page_Up";
        case SDLK_PAGEDOWN: return "Page_Down";

        case SDLK_DOWN:  return "Down";
        case SDLK_LEFT:  return "Left";
        case SDLK_RIGHT: return "Right";
        case SDLK_UP:    return "Up";

        case SDLK_KP_0:        return "KP_0";
        case SDLK_KP_1:        return "KP_1";
        case SDLK_KP_2:        return "KP_2";
        case SDLK_KP_3:        return "KP_3";
        case SDLK_KP_4:        return "KP_4";
        case SDLK_KP_5:        return "KP_5";
        case SDLK_KP_6:        return "KP_6";
        case SDLK_KP_7:        return "KP_7";
        case SDLK_KP_8:        return "KP_8";
        case SDLK_KP_9:        return "KP_9";
        case SDLK_KP_COMMA:    return "KP_Comma";
        case SDLK_KP_DIVIDE:   return "KP_Divide";
        case SDLK_KP_ENTER:    return "KP_Enter";
        case SDLK_KP_EQUALS:   return "KP_Equals";
        case SDLK_KP_MINUS:    return "KP_Minus";
        case SDLK_KP_MULTIPLY: return "KP_Multiply";
        case SDLK_KP_PERIOD:   return "KP_Period";
        case SDLK_KP_PLUS:     return "KP_Plus";

        default:
            // ASCII についてはそのコードを16進文字列化
            // ("F1" などと若干紛らわしい意味はあるが...)
            if(is_print_ascii(sym))
                return FORMAT("{:02X}", sym);
            return std::nullopt;
    }
    // clang-format on
}

std::optional<std::string> trigger_string(const SDL_Keycode sym, const bool shift, const bool ctrl, const bool alt)
{
    const auto name = sym_trigger_name(sym);
    if (!name)
        return std::nullopt;

    // clang-format off
    return FORMAT("\x1F{}{}{}x{}\x0D",
        ctrl  ? "C" : "",
        shift ? "S" : "",
        alt   ? "A" : "",
        *name);
    // clang-format on
}

} // anonymous namespace

// SDL はスキャンコードをキーコードに変換する機能はあるが、Shift キーの処理(JP
// キーボードで Shift+[ が { になる、など)までは面倒を見てくれないため、そこは
// 自前実装する。
// (SDL_TextInputEvent に任せられればよいのだが、それだと今度は Ctrl キーなどの
// 処理が難しくなる)
std::string key_sequence(const SDL_KeyboardEvent &ev)
{
    static const auto layout = detect_keyboard_layout();

    const auto [sym, shift, ctrl, alt] = apply_keyboard_layout(layout, ev);

    // マクロトリガー化せずそのまま文字として送信できるならそうする。
    // 通常のアルファベットや、Ctrl+A, Esc などが該当する。
    if (const auto ch = try_into_char(sym, shift, ctrl, alt))
        return std::string(1, *ch);

    // さもなくばマクロトリガー化を試みる。
    // カーソルキー、Alt+A などが該当する。
    if (const auto trigger = trigger_string(sym, shift, ctrl, alt))
        return *trigger;

    return "";
}

#endif // USE_SDL2
