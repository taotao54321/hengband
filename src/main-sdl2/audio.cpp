#include "autoconf.h"
#ifdef USE_SDL2

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <SDL_mixer.h>

#include "main-sdl2/audio.hpp"
#include "main-sdl2/inifile.hpp"
#include "main-sdl2/prelude.hpp"

namespace {

std::string musics_key(const std::string &sect_name, const std::string &key) { return FORMAT("{}/{}", sect_name, key); }

// ロードできない音声ファイルは無視する
std::map<std::string, std::vector<Music>> load_musics(const std::string &dir_xtra)
{
    const auto dir = FORMAT("{}/music", dir_xtra);

    const auto ini = inifile_parse(FORMAT("{}/music.cfg", dir));

    std::map<std::string, std::vector<Music>> musics;
    for (const auto &[sect_name, sect_map] : ini) {
        for (const auto &[key, value] : sect_map) {
            std::vector<std::string> filenames;
            boost::split(filenames, value, boost::is_any_of("\t "));

            std::vector<Music> muss;
            for (const auto &filename : filenames) {
                const auto path = FORMAT("{}/{}", dir, filename);
                auto mus = Music::load(path);
                if (!mus.is_null())
                    muss.emplace_back(std::move(mus));
            }

            const auto k = musics_key(sect_name, key);
            musics.emplace(k, std::move(muss));
        }
    }

    return musics;
}

// ロードできない音声ファイルは無視する
std::map<std::string, std::vector<Sound>> load_sounds(const std::string &dir_xtra)
{
    constexpr char SECT_NAME[] = "Sound";

    const auto dir = FORMAT("{}/sound", dir_xtra);

    const auto ini = inifile_parse(FORMAT("{}/sound.cfg", dir));

    const auto it = ini.find(SECT_NAME);
    if (it == std::end(ini))
        return {};
    const auto &sect_map = it->second;

    std::map<std::string, std::vector<Sound>> sounds;
    for (const auto &[key, value] : sect_map) {
        std::vector<std::string> filenames;
        boost::split(filenames, value, boost::is_any_of("\t "));

        std::vector<Sound> snds;
        for (const auto &filename : filenames) {
            const auto path = FORMAT("{}/{}", dir, filename);
            auto snd = Sound::load(path);
            if (!snd.is_null())
                snds.emplace_back(std::move(snd));
        }

        const auto k = FORMAT("{}", key);
        sounds.emplace(k, std::move(snds));
    }

    return sounds;
}

} // anonymous namespace

Music::Music(Mix_Music *const music)
    : music_(music)
{
}

void Music::drop()
{
    if (music_) {
        Mix_HaltMusic();
        Mix_FreeMusic(music_);
        music_ = nullptr;
    }
}

Music Music::null() { return Music(nullptr); }

Music Music::load(const std::string &path)
{
    // ロード失敗したら null object になる
    return Music(Mix_LoadMUS(path.c_str()));
}

Music::Music(Music &&other) noexcept
    : music_(std::exchange(other.music_, nullptr))
{
}

Music &Music::operator=(Music &&rhs) noexcept
{
    drop();
    music_ = std::exchange(rhs.music_, nullptr);
    return *this;
}

Music::~Music() { drop(); }

bool Music::is_null() const { return !bool(music_); }

bool Music::play() const
{
    Mix_HaltMusic();
    if (!music_)
        return true;

    return Mix_PlayMusic(music_, -1) == 0;
}

Sound::Sound(Mix_Chunk *const chunk)
    : chunk_(chunk)
{
}

void Sound::drop()
{
    if (chunk_) {
        // 全サウンドを一括ロード/アンロードするので全チャンネルを止めてよい
        Mix_HaltChannel(-1);
        Mix_FreeChunk(chunk_);
        chunk_ = nullptr;
    }
}

Sound Sound::null() { return Sound(nullptr); }

Sound Sound::load(const std::string &path)
{
    // ロード失敗したら null object になる
    return Sound(Mix_LoadWAV(path.c_str()));
}

Sound::Sound(Sound &&other) noexcept
    : chunk_(std::exchange(other.chunk_, nullptr))
{
}

Sound &Sound::operator=(Sound &&rhs) noexcept
{
    drop();
    chunk_ = std::exchange(rhs.chunk_, nullptr);
    return *this;
}

Sound::~Sound() { drop(); }

bool Sound::is_null() const { return !bool(chunk_); }

bool Sound::play() const
{
    if (!chunk_)
        return true;

    return Mix_PlayChannel(-1, chunk_, 0) >= 0;
}

AudioAsset::AudioAsset(const std::string &dir_xtra)
{
    musics_ = load_musics(dir_xtra);
    sounds_ = load_sounds(dir_xtra);
    music_null_.emplace_back(Music::null());
    sound_null_.emplace_back(Sound::null());
}

const std::vector<Music> &AudioAsset::music(const std::string &category, const std::string &name) const
{
    const auto k = musics_key(category, name);
    const auto it = musics_.find(k);
    if (it == std::end(musics_) || it->second.empty())
        return music_null_;

    return it->second;
}

const std::vector<Sound> &AudioAsset::sound(const std::string &name) const
{
    const auto it = sounds_.find(name);
    if (it == std::end(sounds_) || it->second.empty())
        return sound_null_;

    return it->second;
}

void AudioAsset::stop_music() const
{
    (void)this;
    (void)Music::null().play();
}

#endif // USE_SDL2
