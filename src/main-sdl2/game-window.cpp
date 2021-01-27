#include "autoconf.h"
#ifdef USE_SDL2

#include <algorithm>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "main-sdl2/asset.hpp"
#include "main-sdl2/font.hpp"
#include "main-sdl2/game-window.hpp"
#include "main-sdl2/prelude.hpp"
#include "main-sdl2/system.hpp"

namespace {

constexpr int MAIN_WIN_MENU_H = 32;

constexpr int MAIN_WIN_NCOL_MIN = 80;
constexpr int MAIN_WIN_NROW_MIN = 24;

constexpr char FONT_NAME_DEFAULT[] = "monospace";

} // anonymous namespace

GameWindowDesc::GameWindowDesc()
    : title_("Hengband")
    , x_(0)
    , y_(0)
    , w_(400)
    , h_(400)
    , font_path_()
    , font_pt_(16)
    , visible_(true)
{
}

const std::string &GameWindowDesc::title() const { return title_; }

GameWindowDesc &GameWindowDesc::title(std::string title)
{
    title_ = std::move(title);
    return *this;
}

int GameWindowDesc::x() const { return x_; }

GameWindowDesc &GameWindowDesc::x(const int x)
{
    x_ = x;
    return *this;
}

int GameWindowDesc::y() const { return y_; }

GameWindowDesc &GameWindowDesc::y(const int y)
{
    y_ = y;
    return *this;
}

int GameWindowDesc::w() const { return w_; }

GameWindowDesc &GameWindowDesc::w(const int w)
{
    w_ = w;
    return *this;
}

int GameWindowDesc::h() const { return h_; }

GameWindowDesc &GameWindowDesc::h(const int h)
{
    h_ = h;
    return *this;
}

const std::string &GameWindowDesc::font_path() const { return font_path_; }

GameWindowDesc &GameWindowDesc::font_path(std::string path)
{
    font_path_ = std::move(path);
    return *this;
}

int GameWindowDesc::font_pt() const { return font_pt_; }

GameWindowDesc &GameWindowDesc::font_pt(const int pt)
{
    font_pt_ = pt;
    return *this;
}

bool GameWindowDesc::visible() const { return visible_; }

GameWindowDesc &GameWindowDesc::visible(const bool visible)
{
    visible_ = visible;
    return *this;
}

GameWindow GameWindowDesc::build(const bool is_main) const
{
    // フォント生成
    // フォント名が未指定ならデフォルトフォント名を使う。
    auto font_path = font_path_;
    if (font_path.empty()) {
        const auto path_monospace = get_font_path(FONT_NAME_DEFAULT);
        if (!path_monospace)
            PANIC(R"(font "{}" not found)", FONT_NAME_DEFAULT);
        font_path = *path_monospace;
    }
    Font font(font_path, font_pt_);

    // ウィンドウ生成
    // メインウィンドウの場合、最小端末サイズを下回っていたら補正。
    // 起動時に非表示ウィンドウが見切れるのを防ぐため、最初は hidden 状態で生成する。
    auto w = w_;
    auto h = h_;
    auto [ncol, nrow] = font.xy2cr(w, h);
    if (is_main) {
        if (chmax(ncol, MAIN_WIN_NCOL_MIN))
            w = font.c2x(ncol);
        if (chmax(nrow, MAIN_WIN_NROW_MIN))
            h = font.r2y(nrow);
    }
    auto win = Window::create(title_, x_, y_, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

    auto game_win = GameWindow(is_main, std::move(font), std::move(win));

    game_win.set_visible(visible_);

    return game_win;
}

ButtonVisual::ButtonVisual(Rect rect, Texture tex)
    : rect(rect)
    , tex(std::move(tex))
{
}

Texture GameWindow::init_tex_term() const
{
    const auto [w, h] = font_.cr2xy(ncnr_);
    return Texture::create_target(ren_.get(), w, h);
}

Texture GameWindow::init_tex_ascii() const
{
    constexpr int ASCII_MIN = 0x20;
    constexpr int ASCII_MAX = 0x7E;

    const Color COLOR_FG(0xFF, 0xFF, 0xFF, 0xFF);
    const Color COLOR_BG(0, 0, 0, 0xFF);

    const auto w = font_.w() * (ASCII_MAX - ASCII_MIN + 1);
    const auto h = font_.h();
    const auto surf = Surface::create(w, h, COLOR_BG);

    for (const auto ch : IRANGE(ASCII_MIN, ASCII_MAX + 1)) {
        const auto surf_ch = font_.render(std::string(1, char(ch)), COLOR_FG, COLOR_BG);
        const auto x = font_.w() * (ch - ASCII_MIN);
        SDL_Rect rect{ x, 0, 0, 0 }; // w, h は無視される
        if (SDL_BlitSurface(surf_ch.get(), nullptr, surf.get(), &rect) != 0)
            PANIC("SDL_BlitSurface() failed");
    }

    return surf.to_texture(ren_.get());
}

Texture GameWindow::init_tex_wall() const
{
    const auto surf_tile = Surface::from_bytes(WALL_BMP, std::size(WALL_BMP));
    const auto surf = Surface::create_tiled(surf_tile.get(), font_.w(), font_.h());
    return surf.to_texture(ren_.get());
}

std::vector<ButtonVisual> GameWindow::init_buttons_window() const
{
    constexpr std::pair<const u8 *, std::size_t> IMGS[]{
        { nullptr, 0 },
        { WINDOW_1_PNG, std::size(WINDOW_1_PNG) },
        { WINDOW_2_PNG, std::size(WINDOW_2_PNG) },
        { WINDOW_3_PNG, std::size(WINDOW_3_PNG) },
        { WINDOW_4_PNG, std::size(WINDOW_4_PNG) },
        { WINDOW_5_PNG, std::size(WINDOW_5_PNG) },
        { WINDOW_6_PNG, std::size(WINDOW_6_PNG) },
        { WINDOW_7_PNG, std::size(WINDOW_7_PNG) },
    };

    if (!is_main_)
        return {};

    const int right = client_area_size().first;

    std::vector<ButtonVisual> buttons;
    for (const auto i : IRANGE(1, 8)) {
        const int x = right - 32 * (8 - i);
        const Rect rect(x, 0, 32, 32);
        const auto [buf, len] = IMGS[i];
        auto tex = Surface::from_bytes(buf, len).to_texture(ren_.get());
        buttons.emplace_back(rect, std::move(tex));
    }

    return buttons;
}

std::optional<ButtonVisual> GameWindow::init_button_bgm() const
{
    if (!is_main_)
        return std::nullopt;

    const int right = client_area_size().first;

    const Rect rect(right - 32 * 10, 0, 32, 32);
    auto tex = Surface::from_bytes(BGM_PNG, std::size(BGM_PNG)).to_texture(ren_.get());

    return ButtonVisual{ rect, std::move(tex) };
}

std::optional<ButtonVisual> GameWindow::init_button_se() const
{
    if (!is_main_)
        return std::nullopt;

    const int right = client_area_size().first;

    const Rect rect(right - 32 * 9, 0, 32, 32);
    auto tex = Surface::from_bytes(SE_PNG, std::size(SE_PNG)).to_texture(ren_.get());

    return ButtonVisual{ rect, std::move(tex) };
}

GameWindow::GameWindow(const bool is_main, Font font, Window win)
    : is_main_(is_main)
    , font_(std::move(font))
    , win_(std::move(win))
    , ren_(Renderer::with_window(win_.get()))
    , ncnr_(term_size_for(client_area_size()))
    , tex_term_(init_tex_term())
    , tex_ascii_(init_tex_ascii())
    , tex_wall_(init_tex_wall())
    , buttons_window_(init_buttons_window())
    , button_bgm_(init_button_bgm())
    , button_se_(init_button_se())
{
    if (is_main_) {
        const auto [w_min, h_min] = client_area_size_for(MAIN_WIN_NCOL_MIN, MAIN_WIN_NROW_MIN);
        SDL_SetWindowMinimumSize(win_.get(), w_min, h_min);
    }

    term_clear();
}

std::tuple<int, int, int, int> GameWindow::borders_size() const
{
    int top;
    int left;
    int bottom;
    int right;
    // X11 以外ではサポートされていないため失敗する
    if (SDL_GetWindowBordersSize(win_.get(), &top, &left, &bottom, &right) != 0)
        return { 0, 0, 0, 0 };
    return { top, left, bottom, right };
}

std::pair<int, int> GameWindow::pos() const
{
    int x, y;
    SDL_GetWindowPosition(win_.get(), &x, &y);
    return { x, y };
}

std::pair<int, int> GameWindow::client_area_size() const
{
    // TODO:
    //   全ての環境でうまくいくかは未確認。
    //   X11 環境では
    //
    //     * SDL_CreateWindow() に指定したサイズ
    //     * SDL_GetWindowSize() で得られるサイズ
    //     * SDL_RendererOutputSize() で得られるサイズ
    //
    //   は全て一致し、クライアント領域サイズとなるようだ。

    int w, h;
    SDL_GetWindowSize(win_.get(), &w, &h);
    return { w, h };
}

std::pair<int, int> GameWindow::client_area_size_for(const int ncol, const int nrow) const
{
    auto [w, h] = font_.cr2xy(ncol, nrow);

    // メインウィンドウの場合はメニューバーの分を足す
    if (is_main_)
        h += MAIN_WIN_MENU_H;

    return { w, h };
}

std::pair<int, int> GameWindow::term_size_for(int w, int h) const
{
    // メインウィンドウの場合はメニューバーの分を引く
    if (is_main_)
        h = std::max(1, h - MAIN_WIN_MENU_H);

    auto [ncol, nrow] = font_.xy2cr(w, h);

    // メインウィンドウの場合は最小端末サイズを下回らないようにする
    if (is_main_) {
        chmax(ncol, MAIN_WIN_NCOL_MIN);
        chmax(nrow, MAIN_WIN_NROW_MIN);
    }

    return { ncol, nrow };
}

std::pair<int, int> GameWindow::term_size_for(const std::pair<int, int> &wh) const { return term_size_for(wh.first, wh.second); }

Rect GameWindow::term_area_rect() const
{
    const int x = 0;
    const int y = is_main_ ? MAIN_WIN_MENU_H : 0;
    const int w = font_.w() * ncnr_.first;
    const int h = font_.h() * ncnr_.second;

    return Rect(x, y, w, h);
}

void GameWindow::draw_button(const ButtonVisual &button, const bool enabled) const
{
    const auto rect = button.rect.to_sdl_rect();
    const auto &tex = button.tex;

    if (SDL_RenderCopy(ren_.get(), tex.get(), nullptr, &rect) != 0)
        PANIC("SDL_RenderCopy() failed");

    const auto color = enabled ? Color(0xC0, 0xC0, 0x20, 0x60) : Color(0x30, 0x30, 0x30, 0x60);
    if (SDL_SetRenderDrawBlendMode(ren_.get(), SDL_BLENDMODE_BLEND) != 0)
        PANIC("SDL_SetRenderDrawBlendMode() failed");
    if (SDL_SetRenderDrawColor(ren_.get(), color.r(), color.g(), color.b(), color.a()) != 0)
        PANIC("SDL_SetRenderDrawColor() failed");
    if (SDL_RenderFillRect(ren_.get(), &rect) != 0)
        PANIC("SDL_RenderFillRect() failed");
}

u32 GameWindow::id() const
{
    const auto res = SDL_GetWindowID(win_.get());
    if (res == 0)
        PANIC("SDL_GetWindowID() failed");
    return res;
}

bool GameWindow::is_visible() const
{
    // SDL_WINDOW_SHOWN と SDL_WINDOW_HIDDEN を両方指定したウィンドウは hidden
    // になるようなので、後者で判定する
    return !bool(SDL_GetWindowFlags(win_.get()) & SDL_WINDOW_HIDDEN);
}

void GameWindow::set_visible(const bool visible) const
{
    // メインウィンドウは非表示にできない
    if (visible || is_main_)
        SDL_ShowWindow(win_.get());
    else
        SDL_HideWindow(win_.get());
}

void GameWindow::toggle_visible() const { set_visible(!is_visible()); }

void GameWindow::raise() const { SDL_RaiseWindow(win_.get()); }

std::pair<int, int> GameWindow::term_size() const { return ncnr_; }

void GameWindow::term_clear() const
{
    if (SDL_SetRenderTarget(ren_.get(), tex_term_.get()) != 0)
        PANIC("SDL_SetRenderTarget() failed");

    if (SDL_SetRenderDrawColor(ren_.get(), 0, 0, 0, 0xFF) != 0)
        PANIC("SDL_SetRenderDrawColor() failed");
    if (SDL_RenderClear(ren_.get()) != 0)
        PANIC("SDL_RenderClear() failed");
}

void GameWindow::term_fill_rect(const int c, const int r, const int ncol, const int nrow, Color color) const
{
    if (SDL_SetRenderTarget(ren_.get(), tex_term_.get()) != 0)
        PANIC("SDL_SetRenderTarget() failed");

    const auto rect = font_.calc_rect(c, r, ncol, nrow).to_sdl_rect();
    if (SDL_SetRenderDrawColor(ren_.get(), color.r(), color.g(), color.b(), color.a()) != 0)
        PANIC("SDL_SetRenderDrawColor() failed");
    if (SDL_RenderFillRect(ren_.get(), &rect) != 0)
        PANIC("SDL_RenderFillRect() failed");
}

void GameWindow::term_draw_text(const int c, const int r, const std::string &text, Color color) const
{
    constexpr int ASCII_MIN = 0x20;
    constexpr int ASCII_MAX = 0x7E;

    const Color COLOR_BG(0, 0, 0, 0xFF);

    if (SDL_SetRenderTarget(ren_.get(), tex_term_.get()) != 0)
        PANIC("SDL_SetRenderTarget() failed");

    const auto [x_orig, y_orig] = font_.cr2xy(c, r);

    if (ALL(std::all_of, text, [](const char c) { return ASCII_MIN <= c && c <= ASCII_MAX; })) {
        // text が ASCII 表示可能文字のみからなる場合、テクスチャキャッシュを使って
        // 高速に描画する。
        for (const auto i : IRANGE(0, int(std::size(text)))) {
            const auto x_src = font_.w() * (text[i] - ASCII_MIN);
            const auto x_dst = x_orig + font_.w() * i;
            const SDL_Rect rect_src{ x_src, 0, font_.w(), font_.h() };
            const SDL_Rect rect_dst{ x_dst, y_orig, font_.w(), font_.h() };
            if (SDL_SetTextureColorMod(tex_ascii_.get(), color.r(), color.g(), color.b()) != 0)
                PANIC("SDL_SetTextureColorMod() failed");
            if (SDL_RenderCopy(ren_.get(), tex_ascii_.get(), &rect_src, &rect_dst) != 0)
                PANIC("SDL_RenderCopy() failed");
        }
    } else {
        // text が非 ASCII を含む場合はナイーブな方法で描画する。
        const auto surf = font_.render(text, color, COLOR_BG);
        const auto tex = surf.to_texture(ren_.get());

        const SDL_Rect rect{ x_orig, y_orig, surf.get()->w, surf.get()->h };
        if (SDL_RenderCopy(ren_.get(), tex.get(), nullptr, &rect) != 0)
            PANIC("SDL_RenderCopy() failed");
    }
}

void GameWindow::term_draw_wall(const int c, const int r, Color color) const
{
    if (SDL_SetRenderTarget(ren_.get(), tex_term_.get()) != 0)
        PANIC("SDL_SetRenderTarget() failed");

    const auto rect = font_.calc_rect(c, r, 1, 1).to_sdl_rect();
    if (SDL_SetTextureColorMod(tex_wall_.get(), color.r(), color.g(), color.b()) != 0)
        PANIC("SDL_SetTextureColorMod() failed");
    if (SDL_RenderCopy(ren_.get(), tex_wall_.get(), nullptr, &rect) != 0)
        PANIC("SDL_RenderCopy() failed");
}

void GameWindow::present(const PresentParam &param) const
{
    if (SDL_SetRenderTarget(ren_.get(), nullptr) != 0)
        PANIC("SDL_SetRenderTarget() failed");

    // 端末画面表示
    {
        // メインウィンドウの場合、メニューバーの下に表示
        int x = 0;
        int y = 0;
        if (is_main_)
            y += MAIN_WIN_MENU_H;

        int w, h;
        if (SDL_QueryTexture(tex_term_.get(), nullptr, nullptr, &w, &h) != 0)
            PANIC("SDL_QueryTexture() failed");
        const SDL_Rect rect{ x, y, w, h };
        if (SDL_RenderCopy(ren_.get(), tex_term_.get(), nullptr, &rect) != 0)
            PANIC("SDL_RenderCopy() failed");
    }

    // メインウィンドウの場合、メニューバー表示
    if (is_main_) {
        for (const auto i : IRANGE(1, 8))
            draw_button(buttons_window_[i - 1], param.visibles[i]);

        draw_button(*button_bgm_, param.bgm_enabled);
        draw_button(*button_se_, param.se_enabled);
    }

    // 選択中ならその範囲を表示
    if (param.selection) {
        const auto [c, r, ncol, nrow] = *param.selection;
        const auto rect_term = term_area_rect();

        const auto x = rect_term.x() + font_.w() * c;
        const auto y = rect_term.y() + font_.h() * r;
        const auto w = font_.w() * ncol;
        const auto h = font_.h() * nrow;
        SDL_Rect rect{ x, y, w, h };

        if (SDL_SetRenderDrawBlendMode(ren_.get(), SDL_BLENDMODE_BLEND) != 0)
            PANIC("SDL_SetRenderDrawBlendMode() failed");
        if (SDL_SetRenderDrawColor(ren_.get(), 0xFF, 0xFF, 0xFF, 0x40) != 0)
            PANIC("SDL_SetRenderDrawColor() failed");
        if (SDL_RenderFillRect(ren_.get(), &rect) != 0)
            PANIC("SDL_RenderFillRect() failed");
    }

    SDL_RenderPresent(ren_.get());
}

UiElement GameWindow::ui_element_at(const int x, const int y) const
{
    if (is_main_ && y < MAIN_WIN_MENU_H) {
        for (const auto i : IRANGE(1, 8)) {
            if (buttons_window_[i - 1].rect.contains(x, y))
                return WindowButton{ i };
        }

        if (button_bgm_->rect.contains(x, y))
            return BgmButton{};

        if (button_se_->rect.contains(x, y))
            return SeButton{};

        return NullElement{};
    }

    {
        const auto rect = term_area_rect();
        if (rect.contains(x, y)) {
            const auto rel_x = x - rect.x();
            const auto rel_y = y - rect.y();
            const auto col = rel_x / font_.w();
            const auto row = rel_y / font_.h();
            return TermCell{ col, row };
        }
    }

    return NullElement{};
}

std::pair<int, int> GameWindow::on_size_change(const int w, const int h)
{
    // 端末画面サイズが変わる場合、端末画面テクスチャを作り直す
    const auto ncnr_new = term_size_for(w, h);
    if (ncnr_ != ncnr_new) {
        ncnr_ = ncnr_new;
        tex_term_ = init_tex_term();
    }

    return ncnr_new;
}

GameWindowDesc GameWindow::desc() const
{
    // TODO:
    //   X11 環境以外での動作は未検証。
    //   X11 環境では SDL_GetWindowPosition() と SDL_GetWindowBordersSize() を
    //   組み合わせることで外枠も含めたウィンドウ位置が得られる。

    int border_top, border_left;
    std::tie(border_top, border_left, std::ignore, std::ignore) = borders_size();

    const auto [x, y] = pos();
    const auto [w, h] = client_area_size();

    return GameWindowDesc()
        .title(SDL_GetWindowTitle(win_.get()))
        .x(x - border_left)
        .y(y - border_top)
        .w(w)
        .h(h)
        .font_path(font_.path())
        .font_pt(font_.pt())
        .visible(is_visible());
}

#endif // USE_SDL2
