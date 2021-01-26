#include "autoconf.h"
#ifdef USE_SDL2

#include <cstddef>
#include <string>
#include <utility>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "main-sdl2/prelude.hpp"
#include "main-sdl2/system.hpp"

Color::Color(const u8 r, const u8 g, const u8 b, const u8 a)
    : color_{ r, g, b, a }
{
}

Color Color::from_sdl_color(const SDL_Color color) { return Color(color.r, color.g, color.b, color.a); }

u8 Color::r() const { return color_.r; }
u8 Color::g() const { return color_.g; }
u8 Color::b() const { return color_.b; }
u8 Color::a() const { return color_.a; }

SDL_Color Color::to_sdl_color() const { return color_; }

Rect::Rect(const int x, const int y, const int w, const int h)
    : x_(x)
    , y_(y)
    , w_(w)
    , h_(h)
{
}

Rect Rect::from_sdl_rect(const SDL_Rect &rect) { return Rect(rect.x, rect.y, rect.w, rect.h); }

int Rect::x() const { return x_; }
int Rect::y() const { return y_; }
int Rect::w() const { return w_; }
int Rect::h() const { return h_; }

int Rect::right() const { return x_ + w_; }
int Rect::bottom() const { return y_ + h_; }

std::pair<int, int> Rect::pos() const { return { x_, y_ }; }
std::pair<int, int> Rect::size() const { return { w_, h_ }; }

bool Rect::contains(const int x, const int y) const { return x_ <= x && x < right() && y_ <= y && y < bottom(); }

SDL_Rect Rect::to_sdl_rect() const { return SDL_Rect{ x_, y_, w_, h_ }; }

System::System()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        PANIC("SDL_Init() failed");

    SDL_StartTextInput();

    {
        const int flags = IMG_INIT_JPG | IMG_INIT_PNG;
        if (IMG_Init(flags) != flags)
            PANIC("IMG_Init() failed");
    }

    if (TTF_Init() != 0)
        PANIC("TTF_Init() failed");

    {
        // TODO: 新しい SDL_Mixer では Midi などにも対応してるようだがどうする?
        const int flags = MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG;
        if (Mix_Init(flags) != flags)
            PANIC("Mix_Init() failed");

        if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) != 0)
            PANIC("Mix_OpenAudio() failed");
    }
}

System::~System()
{
    Mix_CloseAudio();
    Mix_Quit();

    TTF_Quit();

    IMG_Quit();

    SDL_Quit();
}

void Window::drop()
{
    if (win_) {
        SDL_DestroyWindow(win_);
        win_ = nullptr;
    }
}

Window::Window(SDL_Window *const win)
    : win_(win)
{
}

Window Window::create(const std::string &title, const int x, const int y, const int w, const int h, const u32 flags)
{
    auto *win = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
    if (!win)
        PANIC("SDL_CreateWindow() failed");
    return Window(win);
}

Window::Window(Window &&other) noexcept
    : win_(std::exchange(other.win_, nullptr))
{
}

Window &Window::operator=(Window &&rhs) noexcept
{
    drop();
    win_ = std::exchange(rhs.win_, nullptr);
    return *this;
}

Window::~Window() { drop(); }

SDL_Window *Window::get() const { return win_; }

void Renderer::drop()
{
    if (ren_) {
        SDL_DestroyRenderer(ren_);
        ren_ = nullptr;
    }
}

Renderer::Renderer(SDL_Renderer *const ren)
    : ren_(ren)
{
}

Renderer Renderer::with_window(SDL_Window *const win)
{
    auto *ren = SDL_CreateRenderer(win, -1, 0);
    if (!ren)
        PANIC("SDL_CreateRenderer() failed");
    return Renderer(ren);
}

Renderer::Renderer(Renderer &&other) noexcept
    : ren_(std::exchange(other.ren_, nullptr))
{
}

Renderer &Renderer::operator=(Renderer &&rhs) noexcept
{
    drop();
    ren_ = std::exchange(rhs.ren_, nullptr);
    return *this;
}

Renderer::~Renderer() { drop(); }

SDL_Renderer *Renderer::get() const { return ren_; }

void Texture::drop()
{
    if (tex_) {
        SDL_DestroyTexture(tex_);
        tex_ = nullptr;
    }
}

Texture::Texture(SDL_Texture *const tex)
    : tex_(tex)
{
}

Texture Texture::create_target(SDL_Renderer *const ren, const int w, const int h)
{
    // TODO: ピクセルフォーマットはこれが最善なのか?
    auto *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!tex)
        PANIC("SDL_CreateTexture() failed");
    return Texture(tex);
}

Texture Texture::from_surface(SDL_Renderer *const ren, SDL_Surface *const surf)
{
    auto *tex = SDL_CreateTextureFromSurface(ren, surf);
    if (!tex)
        PANIC("SDL_CreateTextureFromSurface() failed");
    return Texture(tex);
}

Texture::Texture(Texture &&other) noexcept
    : tex_(std::exchange(other.tex_, nullptr))
{
}

Texture &Texture::operator=(Texture &&rhs) noexcept
{
    drop();
    tex_ = std::exchange(rhs.tex_, nullptr);
    return *this;
}

Texture::~Texture() { drop(); }

SDL_Texture *Texture::get() const { return tex_; }

void Surface::drop()
{
    if (surf_) {
        SDL_FreeSurface(surf_);
        surf_ = nullptr;
    }
}

Surface::Surface(SDL_Surface *const surf)
    : surf_(surf)
{
}

Surface Surface::create(const int w, const int h)
{
    auto *surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surf)
        PANIC("SDL_CreateRGBSurfaceWithFormat() failed");
    return Surface(surf);
}

Surface Surface::create_tiled(SDL_Surface *const tile, const int w, const int h)
{
    const int w_tile = tile->w;
    const int h_tile = tile->h;

    auto surf = Surface::create(w, h);
    for (int y = 0; y < h; y += h_tile) {
        for (int x = 0; x < w; x += w_tile) {
            SDL_Rect rect{ x, y, w_tile, h_tile };
            if (SDL_BlitSurface(tile, nullptr, surf.get(), &rect) != 0)
                PANIC("SDL_BlitSurface() failed");
        }
    }

    return surf;
}

Surface Surface::from_bytes(const u8 *const buf, const std::size_t len)
{
    auto *rdr = SDL_RWFromConstMem(buf, len);
    if (!rdr)
        PANIC("SDL_RWFromConstMem() failed");

    auto *surf = IMG_Load_RW(rdr, 1);
    if (!surf)
        PANIC("IMG_Load_RW() failed");

    return Surface(surf);
}

Surface::Surface(Surface &&other) noexcept
    : surf_(std::exchange(other.surf_, nullptr))
{
}

Surface &Surface::operator=(Surface &&rhs) noexcept
{
    drop();
    surf_ = std::exchange(rhs.surf_, nullptr);
    return *this;
}

Surface::~Surface() { drop(); }

SDL_Surface *Surface::get() const { return surf_; }

Texture Surface::to_texture(SDL_Renderer *const ren) const { return Texture::from_surface(ren, get()); }

#endif // USE_SDL2
