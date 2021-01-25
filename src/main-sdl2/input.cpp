#include "autoconf.h"
#ifdef USE_SDL2

// SDL2 ドライバ 入力システム
//
// SDL2 はユーザー入力を認識したとき、SDL_KEYDOWN, SDL_TEXTINPUT のいずれか、も
// しくは両方を発生する。この挙動の詳細を記す。
// (SDL2 2.0.14, X11 + fcitx 環境で確認)
//
// 日本語入力が on の場合、SDL_KEYDOWN は発生せず、入力確定時に SDL_TEXTINPUT
// が発生する。このとき SDL_TextInputEvent 内には UTF-8 テキストが入っている。
// 入力がバッファに収まらない場合、複数回の SDL_TEXTINPUT イベントに分割される。
//
// 日本語入力が off の場合、キー押下時は常に SDL_KEYDOWN が発生する。
// そのキーが文字として解釈できる場合(例: アルファベット)、SDL_TEXTINPUT も発生
// する。このとき SDL_TextInputEvent 内にはキーボードレイアウトに基づく Shift
// キー処理がなされたテキストが入っている。
// 文字として解釈できないキーは以下の通り:
//
//   * ファンクションキー、カーソルキーなどの特殊キー
//   * ASCII 制御文字 (BS, TAB, CR, ESC, DEL, ...)
//   * 修飾キー Ctrl を含む入力 (Alt については非修飾キーが文字として解釈できる
//     なら普通に SDL_TEXTINPUT が発生する)
//
// 以上を踏まえると、入力処理は基本的に SDL_TEXTINPUT ハンドラで行い、これでカ
// バーしきれない入力を SDL_KEYDOWN ハンドラで拾う形が妥当だろう。

// Shift+Ctrl は ASCII 表示可能文字については Shift を無視して扱う(扱いが自明で
// ないため。例えば US キーボードの Shift+Ctrl+; は JP キーボードの Ctrl+: と等
// 価なのか?)

// 今のところ、日本語入力システムから半角ASCIIが1文字送られた場合は通常のコマン
// ドとして取り扱う。この判定は難しいため妥協した。
// (やるとすれば SDL_TEXTINPUT と同時に SDL_KEYDOWN が来たかどうかを見るのだろ
// うが、SDL2 の内部実装に依存するし、負荷状況によってはタイムラグが生じる可能
// 性もあり、安定性のある方法とは言い難い)

#include <optional>
#include <string>
#include <tuple>

#include <SDL.h>

#include "main-sdl2/encoding.hpp"
#include "main-sdl2/input.hpp"
#include "main-sdl2/prelude.hpp"

namespace {

bool is_lower_ascii(const SDL_Keycode sym) { return 'a' <= sym && sym <= 'z'; }
bool is_print_ascii(const SDL_Keycode sym) { return '\x20' <= sym && sym <= '\x7E'; }

std::tuple<bool, bool, bool> mod_extract(const u32 mod)
{
    const auto shift = bool(mod & KMOD_SHIFT);
    const auto ctrl = bool(mod & KMOD_CTRL);
    const auto alt = bool(mod & KMOD_ALT);

    return { shift, ctrl, alt };
}

// キーコード sym のマクロトリガー用名称を返す。
// 認識できないキーコードに対しては std::nullopt を返す。
std::optional<std::string> trigger_name(const SDL_Keycode sym)
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

        // Shift+BS なども扱いたいので...
        case SDLK_BACKSPACE: return "Backspace";
        case SDLK_TAB:       return "Tab";
        case SDLK_RETURN:    return "Enter";
        case SDLK_ESCAPE:    return "Escape";
        case SDLK_DELETE:    return "Delete";

        default:
            // ASCII 表示可能文字についてはそのコードを16進文字列化
            // ("F1" などと若干紛らわしい意味はあるが...)
            if(is_print_ascii(sym))
                return FORMAT("{:02X}", sym);
            return std::nullopt;
    }
    // clang-format on
}

// ゲーム内コマンド入力
class CommandInput {
private:
    SDL_Keycode sym_;
    bool shift_;
    bool ctrl_;
    bool alt_;

    CommandInput(const SDL_Keycode sym, const bool shift, const bool ctrl, const bool alt)
        : sym_(sym)
        , shift_(shift)
        , ctrl_(ctrl)
        , alt_(alt)
    {
    }

    // マクロトリガー化せずそのまま文字として送信できるならその文字を返す。
    // さもなくば std::nullopt を返す。
    [[nodiscard]] std::optional<char> try_to_char() const
    {
        // Alt+何か は文字として送れない
        if (alt_)
            return std::nullopt;

        // Ctrl+アルファベット、および Ctrl+[ は文字として送る
        // このとき Shift は無視する
        // それ以外の Ctrl+何か は文字として送れない
        if (ctrl_) {
            switch (sym_) {
            case '[':
                return '\x1B'; // ESC
            default:
                if (is_lower_ascii(sym_))
                    return char(sym_ & 0x1F);
                return std::nullopt;
            }
        }

        // Shift が含まれない場合、ASCII 表示可能文字および一部の制御文字はそのまま送る。
        // 表示可能文字については TEXTINPUT がソースなので Shift は含まれないはず。
        // 制御文字については Shift+BS なども許したいのでこのようにした。
        if (!shift_) {
            // clang-format off
            switch (sym_) {
                case SDLK_BACKSPACE: return '\x08';
                case SDLK_TAB:       return '\x09';
                case SDLK_RETURN:    return '\x0D';
                case SDLK_ESCAPE:    return '\x1B';
                case SDLK_DELETE:    return '\x7F';
                default:
                    if(is_print_ascii(sym_))
                        return char(sym_);
                    return std::nullopt;
            }
            // clang-format on
        }

        return std::nullopt;
    }

    // マクロトリガー文字列を返す。
    // コマンドが無効な場合、空文字列を返す。
    [[nodiscard]] std::string to_trigger() const
    {
        const auto name = trigger_name(sym_);
        if (!name)
            return "";

        return FORMAT("\x1F{}{}{}x{}\x0D", ctrl_ ? "C" : "", shift_ ? "S" : "", alt_ ? "A" : "", *name);
    }

public:
    // ゲーム側へ送信するキーシーケンスを返す。
    // コマンドが無効な場合、空シーケンスを返す。
    [[nodiscard]] std::string to_sequence() const
    {
        if (const auto ch = try_to_char())
            return std::string(1, *ch);

        return to_trigger();
    }

    // キーボードイベント ev が処理対象なら CommandInput に変換して返す。
    // 処理対象でなければ std::nullopt を返す。
    static std::optional<CommandInput> from(const SDL_KeyboardEvent &ev)
    {
        const auto sym = ev.keysym.sym;
        const auto [shift, ctrl, alt] = mod_extract(ev.keysym.mod);

        // Ctrl を含むものは全て対象
        if (ctrl)
            return CommandInput(sym, shift, ctrl, alt);

        // Ctrl を含まない場合、ASCII 表示可能文字以外を対象とする
        if (!is_print_ascii(sym))
            return CommandInput(sym, shift, ctrl, alt);

        return std::nullopt;
    }

    // テキスト入力イベント ev が処理対象なら CommandInput に変換して返す。
    // 処理対象でなければ std::nullopt を返す。
    static std::optional<CommandInput> from(const SDL_TextInputEvent &ev)
    {
        // 1Byte の入力のみを対象とする
        if (ev.text[0] == '\0' || ev.text[1] != '\0')
            return std::nullopt;

        const auto sym = u8(ev.text[0]);
        const auto [shift, ctrl, alt] = mod_extract(SDL_GetModState());

        // Ctrl を含むものは対象としない
        if (ctrl)
            return std::nullopt;

        // ASCII 表示可能文字のみを対象とする
        if (!is_print_ascii(sym))
            return std::nullopt;

        // Shift 処理は済んでいるので Shift フラグは下ろす
        return CommandInput(sym, false, ctrl, alt);
    }
};

} // anonymous namespace

// 特殊キー、ASCII 制御文字、および Ctrl を含む入力のみ処理する。
std::string key_sequence(const SDL_KeyboardEvent &ev)
{
    const auto input = CommandInput::from(ev);
    if (!input)
        return "";

    return input->to_sequence();
}

std::string key_sequence(const SDL_TextInputEvent &ev)
{
    // 一応空入力チェック
    if (ev.text[0] == '\0')
        return "";

    if (const auto input = CommandInput::from(ev))
        return input->to_sequence();

    // コマンド入力でないなら日本語入力と解釈し、システムエンコーディングに変換
    // してそのまま垂れ流す。
    return utf8_to_euc(ev.text);
}

#endif // USE_SDL2
