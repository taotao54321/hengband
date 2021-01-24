#include <string>
#include <string_view>

#include <SDL.h>

#include "main-sdl2/keyboard.hpp"
#include "main-sdl2/prelude.hpp"

namespace {

bool is_lower_ascii(const SDL_Keycode sym) { return 'a' <= sym && sym <= 'z'; }
bool is_print_ascii(const SDL_Keycode sym) { return '\x20' <= sym && sym <= '\x7E'; }

SDL_Keycode to_upper_ascii(const SDL_Keycode sym) { return is_lower_ascii(sym) ? sym - 0x20 : sym; }

enum class KeyboardLayout { JP, US };

// とりあえず JP, US のみ対応。
// 一応 pref の shift キー設定と組み合わせることで任意のレイアウトに対応できそ
// うな気もするが、それなりの作業を要しそうなので手抜きした。
KeyboardLayout detect_keyboard_layout()
{
    // US キーボードの [ が JP キーボードでは @ であることを利用して判定
    if (SDL_GetKeyFromScancode(SDL_SCANCODE_LEFTBRACKET) == SDLK_AT)
        return KeyboardLayout::JP;
    return KeyboardLayout::US;
}

std::string special_sequence(const SDL_Keycode sym, const bool shift, const bool ctrl, const bool alt) // {{{
{
    // clang-format off
    const auto trigger = [shift, ctrl, alt](const std::string_view name) {
        return FORMAT("\x1F{}{}{}x{}\x0D",
            ctrl  ? "C" : "",
            shift ? "S" : "",
            alt   ? "A" : "",
            name);
    };

    switch (sym) {
        case SDLK_F1:  return trigger("F1");
        case SDLK_F2:  return trigger("F2");
        case SDLK_F3:  return trigger("F3");
        case SDLK_F4:  return trigger("F4");
        case SDLK_F5:  return trigger("F5");
        case SDLK_F6:  return trigger("F6");
        case SDLK_F7:  return trigger("F7");
        case SDLK_F8:  return trigger("F8");
        case SDLK_F9:  return trigger("F9");
        case SDLK_F10: return trigger("F10");
        case SDLK_F11: return trigger("F11");
        case SDLK_F12: return trigger("F12");
        case SDLK_F13: return trigger("F13");
        case SDLK_F14: return trigger("F14");
        case SDLK_F15: return trigger("F15");
        case SDLK_F16: return trigger("F16");
        case SDLK_F17: return trigger("F17");
        case SDLK_F18: return trigger("F18");
        case SDLK_F19: return trigger("F19");
        case SDLK_F20: return trigger("F20");
        case SDLK_F21: return trigger("F21");
        case SDLK_F22: return trigger("F22");
        case SDLK_F23: return trigger("F23");
        case SDLK_F24: return trigger("F24");

        case SDLK_PAUSE: return trigger("Pause");

        case SDLK_INSERT: return trigger("Insert");

        case SDLK_HOME: return trigger("Home");
        case SDLK_END:  return trigger("End");

        case SDLK_PAGEUP:   return trigger("Page_Up");
        case SDLK_PAGEDOWN: return trigger("Page_Down");

        case SDLK_DOWN:  return trigger("Down");
        case SDLK_LEFT:  return trigger("Left");
        case SDLK_RIGHT: return trigger("Right");
        case SDLK_UP:    return trigger("Up");

        case SDLK_KP_0:        return trigger("KP_0");
        case SDLK_KP_1:        return trigger("KP_1");
        case SDLK_KP_2:        return trigger("KP_2");
        case SDLK_KP_3:        return trigger("KP_3");
        case SDLK_KP_4:        return trigger("KP_4");
        case SDLK_KP_5:        return trigger("KP_5");
        case SDLK_KP_6:        return trigger("KP_6");
        case SDLK_KP_7:        return trigger("KP_7");
        case SDLK_KP_8:        return trigger("KP_8");
        case SDLK_KP_9:        return trigger("KP_9");
        case SDLK_KP_COMMA:    return trigger("KP_Comma");
        case SDLK_KP_DIVIDE:   return trigger("KP_Divide");
        case SDLK_KP_ENTER:    return trigger("KP_Enter");
        case SDLK_KP_EQUALS:   return trigger("KP_Equals");
        case SDLK_KP_MINUS:    return trigger("KP_Minus");
        case SDLK_KP_MULTIPLY: return trigger("KP_Multiply");
        case SDLK_KP_PERIOD:   return trigger("KP_Period");
        case SDLK_KP_PLUS:     return trigger("KP_Plus");

        default: return "";
    }
    // clang-format on
} // }}}

std::string plain_sequence(const SDL_Keycode sym) // {{{
{
    // clang-format off
    switch (sym) {
        case SDLK_BACKSPACE: return "\x08";
        case SDLK_TAB:       return "\x09";
        case SDLK_RETURN:    return "\x0D";
        case SDLK_ESCAPE:    return "\x1B";
        case SDLK_DELETE:    return "\x7F";

        default:
            if (is_print_ascii(sym))
                return std::string(1, char(sym));
            return "";
    }
    //clang-format on
} // }}}

std::string shift_sequence_jp(const SDL_Keycode sym, const SDL_Scancode code) // {{{
{
    // clang-format off
    switch(sym) {
        // layout-specific
        case '1': return "!";
        case '2': return "\"";
        case '3': return "#";
        case '4': return "$";
        case '5': return "%";
        case '6': return "&";
        case '7': return "'";
        case '8': return "(";
        case '9': return ")";
        case '-': return "=";
        case '^': return "~";
        case '@': return "`";
        case '[': return "{";
        case ';': return "+";
        case ':': return "*";
        case ']': return "}";
        case ',': return "<";
        case '.': return ">";
        case '/': return "?";
        case '\\':
            switch(code) {
                case SDL_SCANCODE_INTERNATIONAL3: return "|";
                case SDL_SCANCODE_INTERNATIONAL1: return "_";
                default: return "";
            }

        default:
            if(is_lower_ascii(sym))
                return std::string(1, char(to_upper_ascii(sym)));
            return "";
    }
    // clang-format on
} // }}}

std::string shift_sequence_us(const SDL_Keycode sym, SDL_Scancode) // {{{
{
    // clang-format off
    switch(sym) {
        // layout-specific
        case '1': return "!";
        case '2': return "@";
        case '3': return "#";
        case '4': return "$";
        case '5': return "%";
        case '6': return "^";
        case '7': return "&";
        case '8': return "*";
        case '9': return "(";
        case '0': return ")";
        case '-': return "_";
        case '=': return "+";
        case '\\': return "|";
        case '[': return "{";
        case ']': return "}";
        case ';': return ":";
        case '\'': return "\"";
        case ',': return "<";
        case '.': return ">";
        case '/': return "?";

        default:
            if(is_lower_ascii(sym))
                return std::string(1, char(to_upper_ascii(sym)));
            return "";
    }
    // clang-format on
} // }}}

std::string shift_sequence(const KeyboardLayout layout, const SDL_Keycode sym, const SDL_Scancode code) // {{{
{
    switch (layout) {
    case KeyboardLayout::JP:
        return shift_sequence_jp(sym, code);
    case KeyboardLayout::US:
        return shift_sequence_us(sym, code);
    default:
        PANIC("unreachable");
    }
} // }}}

std::string ctrl_sequence(const SDL_Keycode sym) // {{{
{
    // clang-format off
    switch (sym) {
        case '[': return "\x1B"; // ESC

        default:
            if(is_lower_ascii(sym))
                return std::string(1, char(sym & 0x1F));
            return "";
    }
    // clang-format on
} // }}}

} // anonymous namespace

// SDL はスキャンコードをキーコードに変換する機能はあるが、Shift キーの処理(JP
// キーボードで Shift+[ が { になる、など)までは面倒を見てくれないため、そこは
// 自前実装する。
// (SDL_TextInputEvent に任せられればよいのだが、それだと今度は Ctrl キーなどの
// 処理が難しくなる)
std::string key_sequence(const SDL_KeyboardEvent &ev)
{
    static const auto layout = detect_keyboard_layout();

    const auto sym = ev.keysym.sym;
    const auto code = ev.keysym.scancode;
    const auto shift = bool(ev.keysym.mod & KMOD_SHIFT);
    const auto ctrl = bool(ev.keysym.mod & KMOD_CTRL);
    const auto alt = bool(ev.keysym.mod & KMOD_ALT);

    // 特殊キーはマクロトリガーに変換して返す
    if (auto trigger = special_sequence(sym, shift, ctrl, alt); !trigger.empty())
        return trigger;

    // 特殊キー以外は通常の文字として返す

    // 修飾キーなし
    if (!shift && !ctrl && !alt)
        return plain_sequence(sym);

    // Shift のみ
    if (shift && !ctrl && !alt)
        return shift_sequence(layout, sym, code);

    // Ctrl のみ
    if (!shift && ctrl && !alt)
        return ctrl_sequence(sym);

    return "";
}
