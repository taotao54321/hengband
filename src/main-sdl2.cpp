#include "autoconf.h"
#ifdef USE_SDL2

// SDL2 ドライバ
//
// エラー処理は基本的に「何か変なことが起きたら直ちにクラッシュする」方針で書いている。
// 実装が容易なことと、バグらせたとき即座にフィードバックが得られることが理由。
//
// XXX:
//   EUC-JP 文字列を扱う際、バイト数 == 文字幅 と仮定している箇所があるが、これ
//   は厳密には正しくない(3バイト文字や半角カナがあると崩れる)。この件について
//   は開発コミュニティの判断に委ねたい。

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <SDL.h>

extern "C" {
#include "system/angband.h"
#include "term/gameterm.h"
#include "term/term-color-types.h"
#include "term/z-term.h"
#include "term/z-util.h"
}

#include "main-sdl2/config.hpp"
#include "main-sdl2/encoding.hpp"
#include "main-sdl2/game-window.hpp"
#include "main-sdl2/input.hpp"
#include "main-sdl2/prelude.hpp"
#include "main-sdl2/system.hpp"

namespace {

constexpr int TERM_COUNT = 8;

// clang-format off
constexpr std::array<int, TERM_COUNT> TERM_IDS { 0, 1, 2, 3, 4, 5, 6, 7 };
// clang-format on

struct SelectionAnchor {
    int term_id;
    int col;
    int row;
};

Config config{};

System *sys{};
std::vector<GameWindow> wins{};
std::array<term_type, TERM_COUNT> terms{};

std::optional<SelectionAnchor> sel_anchor{};

int current_term_id() { return *static_cast<int *>(Term->data); }

// SDL のウィンドウIDから対応する端末IDを得る。
// 無効なウィンドウIDに対しては std::nullopt を返す。
// (例えばウィンドウ外でマウスボタンを離したときのウィンドウIDは無効値となる)
std::optional<int> window_id_to_term_id(const u32 win_id)
{
    for (const auto i : IRANGE(TERM_COUNT)) {
        if (wins[i].id() == win_id)
            return i;
    }
    return std::nullopt;
}

void send_key(const char key)
{
    // Windows ドライバと同様、自前でキューを操作する。
    // 逆順に term_key_push() する方法だと長い日本語を入力したときにテキストの
    // 順序が入れ替わってしまう。TEXTINPUT は長いテキストを複数のイベントに分割
    // するため。

    const auto next_idx = [](const u16 i) -> u16 { return i + 1 == Term->key_size ? 0 : i + 1; };

    // キーバッファが一杯なら入力を捨てる
    if (next_idx(Term->key_head) == Term->key_tail) {
        EPRINTLN("key buffer overflow, ignoring key {:#02X}", key);
        return;
    }

    Term->key_queue[Term->key_head] = key;
    Term->key_head = next_idx(Term->key_head);
}

void send_keys(const std::string &keys)
{
    for (const auto key : keys)
        send_key(key);
}

errr on_keydown(const SDL_KeyboardEvent &ev)
{
    send_keys(key_sequence(ev));

    return 0;
}

errr on_textinput(const SDL_TextInputEvent &ev)
{
    send_keys(key_sequence(ev));

    return 0;
}

void window_present(const GameWindow &win)
{
    PresentParam param;
    for (const auto i : IRANGE(8))
        param.visibles[i] = wins[i].is_visible();

    win.present(param);
}

void window_redraw(const int term_id)
{
    const auto &win = wins[term_id];
    win.term_clear();

    auto *term = &terms[term_id];
    term_activate(term);
    term_redraw();

    window_present(win);
}

errr on_window_size_change(const SDL_WindowEvent &ev, const int term_id)
{
    auto &win = wins[term_id];

    const auto [ncol, nrow] = win.on_size_change(ev.data1, ev.data2);

    term_activate(&terms[term_id]);
    term_resize(ncol, nrow);

    return 0;
}

errr on_window_close(const SDL_WindowEvent &, const int term_id)
{
    // メインウィンドウは close イベントを無視
    if (term_id == 0)
        return 0;

    const auto &win = wins[term_id];
    win.set_visible(false);

    window_present(wins[0]);

    return 0;
}

errr on_window(const SDL_WindowEvent &ev)
{
    const auto term_id = window_id_to_term_id(ev.windowID);
    // 無効なウィンドウIDは無視
    if (!term_id)
        return 0;

    errr res = 0;

    switch (ev.event) {
    case SDL_WINDOWEVENT_EXPOSED:
        // これを行わないとリサイズ中に真っ黒になることがある
        window_redraw(*term_id);
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
        res = on_window_size_change(ev, *term_id);
        break;
    case SDL_WINDOWEVENT_CLOSE:
        res = on_window_close(ev, *term_id);
        break;
    }
    default:
        break;
    }

    return res;
}

errr on_mousedown(const SDL_MouseButtonEvent &ev)
{
    // 左ボタン以外は無視
    if (ev.button != SDL_BUTTON_LEFT)
        return 0;

    const auto term_id = window_id_to_term_id(ev.windowID);
    // 無効なウィンドウIDは無視
    if (!term_id)
        return 0;

    const auto &win = wins[*term_id];

    // ウィンドウトグルボタンのみ処理する。
    // 選択は左ボタンを押してからマウスを初めて動かしたときに開始する。
    // clang-format off
    std::visit(overload{
        [&](const WindowButton &btn) {
            wins[btn.idx].toggle_visible();
            window_present(win);
            win.raise();
        },
        [](TermCell) {},
        [](NullElement) {},
    }, win.ui_element_at(ev.x, ev.y));
    // clang-format on

    return 0;
}

errr on_mouseup(const SDL_MouseButtonEvent &ev)
{
    // 左ボタン以外は無視
    if (ev.button != SDL_BUTTON_LEFT)
        return 0;

    // 未選択なら無視
    if (!sel_anchor)
        return 0;

    const auto term_id = window_id_to_term_id(ev.windowID);
    // 無効なウィンドウIDなら無視し、選択解除
    // (ウィンドウ内でボタン押下してからウィンドウ外へドラッグしてボタンを離す
    // ことで、実際に無効なウィンドウIDが生成されうる)
    if (!term_id) {
        sel_anchor = std::nullopt;
        return 0;
    }

    const auto &win = wins[*term_id];

    // 選択開始時と端末IDが一致しなければ無視し、選択解除
    if (term_id != sel_anchor->term_id) {
        sel_anchor = std::nullopt;
        return 0;
    }

    // カーソルが端末画面内にあればコピー処理
    // clang-format off
    std::visit(overload {
        [&](const TermCell& cell) {
            const auto [cmin, cmax] = std::minmax(cell.col, sel_anchor->col);
            const auto [rmin, rmax] = std::minmax(cell.row, sel_anchor->row);
            // TODO: コピー
            EPRINTLN("copy {} {} {} {}", cmin, cmax, rmin, rmax);
        },
        [](WindowButton) {},
        [](NullElement) {},
    }, win.ui_element_at(ev.x, ev.y));
    // clang-format on

    // 選択解除
    sel_anchor = std::nullopt;

    return 0;
}

errr on_mousemotion(const SDL_MouseMotionEvent &ev)
{
    // 左ボタンを押していないなら無視
    if (!bool(ev.state & SDL_BUTTON_LMASK))
        return 0;

    const auto term_id = window_id_to_term_id(ev.windowID);
    // 無効なウィンドウIDは無視
    if (!term_id)
        return 0;

    const auto &win = wins[*term_id];

    // 選択開始時と端末IDが一致しなければいったん選択解除
    if (sel_anchor && term_id != sel_anchor->term_id)
        sel_anchor = std::nullopt;

    // カーソルが端末画面内にあれば選択処理
    // clang-format off
    std::visit(overload {
        [&](const TermCell& cell) {
            // 未選択なら選択開始
            if(!sel_anchor)
                sel_anchor = SelectionAnchor { *term_id, cell.col, cell.row };

            const auto [cmin, cmax] = std::minmax(cell.col, sel_anchor->col);
            const auto [rmin, rmax] = std::minmax(cell.row, sel_anchor->row);
            // TODO: 選択表示
            EPRINTLN("select {} {} {} {}", cmin, cmax, rmin, rmax);
        },
        [](WindowButton) {},
        [](NullElement) {},
    }, win.ui_element_at(ev.x, ev.y));
    // clang-format on

    return 0;
}

errr handle_event(const SDL_Event &ev)
{
    const auto term_id_orig = current_term_id();

    errr res = 0;

    switch (ev.type) {
    case SDL_TEXTINPUT:
        res = on_textinput(ev.text);
        break;
    case SDL_KEYDOWN:
        res = on_keydown(ev.key);
        break;
    case SDL_MOUSEBUTTONDOWN:
        res = on_mousedown(ev.button);
        break;
    case SDL_MOUSEBUTTONUP:
        res = on_mouseup(ev.button);
        break;
    case SDL_MOUSEMOTION:
        res = on_mousemotion(ev.motion);
        break;
    case SDL_WINDOWEVENT:
        res = on_window(ev.window);
        break;
    // SDL_QUIT は無視する。これにより端末からの Ctrl+C も無視される
    default:
        break;
    }

    // イベント処理中に Term が切り替わることがあるので元に戻す
    term_activate(&terms[term_id_orig]);

    return res;
}

errr poll_event()
{
    SDL_Event ev;
    if (SDL_PollEvent(&ev) == 0)
        return 1;
    return handle_event(ev);
}

errr wait_event()
{
    SDL_Event ev;
    if (SDL_WaitEvent(&ev) == 0)
        PANIC("SDL_WaitEvent() failed");
    return handle_event(ev);
}

errr flush_events()
{
    while (poll_event() == 0)
        ;
    return 0;
}

extern "C" errr term_xtra_sdl2(const int name, const int value)
{
    errr res = 0;

    switch (name) {
    case TERM_XTRA_EVENT:
        // UI 側イベントを1つ処理
        // 引数が 0 なら poll, 非0 なら wait
        res = value == 0 ? poll_event() : wait_event();
        break;
    case TERM_XTRA_BORED:
        // UI 側イベントを1つ処理 (poll)
        res = poll_event();
        break;
    case TERM_XTRA_FLUSH:
        // UI 側イベントを全て処理
        res = flush_events();
        break;
    case TERM_XTRA_CLEAR: {
        // 現在のウィンドウの内容をクリア
        const auto &win = wins[current_term_id()];
        win.term_clear();
        break;
    }
    case TERM_XTRA_FRESH: {
        // 現在のウィンドウの描画内容を反映
        const auto &win = wins[current_term_id()];
        window_present(win);
        break;
    }
    case TERM_XTRA_DELAY:
        // ディレイ (引数: ミリ秒)
        SDL_Delay(value);
        break;
    default:
        break;
    }

    return res;
}

extern "C" errr term_curs_sdl2(const int c, const int r)
{
    const auto &win = wins[current_term_id()];
    win.term_fill_rect(c, r, 1, 1, Color(0xFF, 0xFF, 0xFF, 0xFF));

    return 0;
}

extern "C" errr term_bigcurs_sdl2(const int c, const int r)
{
    const auto &win = wins[current_term_id()];
    win.term_fill_rect(c, r, 2, 1, Color(0xFF, 0xFF, 0xFF, 0xFF));

    return 0;
}

extern "C" errr term_wipe_sdl2(const int c, const int r, const int n)
{
    const auto &win = wins[current_term_id()];
    win.term_fill_rect(c, r, n, 1, Color(0, 0, 0, 0xFF));

    return 0;
}

extern "C" errr term_text_sdl2(const TERM_LEN c, const TERM_LEN r, const int n, const TERM_COLOR attr, const char *euc_arg)
{
    // 壁の内部コード。lib/pref/font-sdl.prf で指定されている。
    // この値は EUC-JP と干渉しない。
    constexpr char CH_WALL = 0x7F;

    const auto &win = wins[current_term_id()];

    // 入力文字列内の CH_WALL を '#' に置換し、そのインデックスを記録していく。
    // このインデックスたちを壁描画位置として使う。
    // CH_WALL を含む入力ではバイト数と文字幅が等しいと仮定している。
    // (EUC-JP の3バイト文字や半角カナを含まないということ)

    std::string euc(euc_arg, n);
    std::vector<int> offs_wall;
    for (const auto i : IRANGE(n)) {
        if (euc[i] == CH_WALL) {
            offs_wall.emplace_back(i);
            euc[i] = '#';
        }
    }

    const auto utf8 = euc_to_utf8(euc);

    const Color fg(angband_color_table[attr][1], angband_color_table[attr][2], angband_color_table[attr][3], 0xFF);
    const Color bg(0, 0, 0, 0xFF);

    // 最初に draw_blanks() を行わないと画面にゴミが残ることがある。
    // draw_text() の描画範囲はテキスト内の文字によって変動するため。
    // (等幅フォントであっても高さは文字ごとに異なる)
    // XXX: バイト数と文字幅が一致すると仮定しているが、これは厳密には正しくない
    win.term_fill_rect(c, r, n, 1, Color(0, 0, 0, 0xFF));

    // 先にテキストを描画
    win.term_draw_text(c, r, utf8, fg, bg);

    // 後から壁を描画
    for (const auto off : offs_wall)
        win.term_draw_wall(c + off, r, fg);

    return 0;
}

// 終了時に設定を保存
extern "C" void quit_hook(concptr)
{
    for (const auto i : IRANGE(TERM_COUNT))
        config.win_descs[i] = wins[i].desc();

    if (!config.save())
        EPRINTLN("failed to save config");
}

} // anonymous namespace

extern "C" void init_sdl2(int /*argc*/, char ** /*argv*/)
{
    // ファイルローカルで確保したリソースの解放については考えない。
    // どうせプログラム終了時に解放されるので。

    if (const auto config_loaded = Config::load())
        config = *config_loaded;

    sys = new System;

    for (const auto i : IRANGE(TERM_COUNT)) {
        const auto is_main = i == 0;
        auto win = config.win_descs[i].build(is_main);
        const auto [ncol, nrow] = win.term_size();
        wins.emplace_back(std::move(win));

        auto *term = &terms[i];
        term_init(term, ncol, nrow, 4096);
        term->soft_cursor = true;
        term->attr_blank = TERM_WHITE;
        term->char_blank = ' ';
        term->xtra_hook = term_xtra_sdl2;
        term->curs_hook = term_curs_sdl2;
        term->bigcurs_hook = term_bigcurs_sdl2;
        term->wipe_hook = term_wipe_sdl2;
        term->text_hook = term_text_sdl2;
        term->data = const_cast<void *>(static_cast<const void *>(&TERM_IDS[i]));
        angband_term[i] = term;
    }

    for (const auto &win : wins)
        window_present(win);

    quit_aux = quit_hook;

    // メインウィンドウをアクティブ化
    wins[0].raise();

    // これを行わないとクラッシュする
    // Term の初期化はドライバ側の責任らしい
    term_activate(angband_term[0]);
}

#endif // USE_SDL2
