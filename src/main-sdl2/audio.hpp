#pragma once

// 音声設定や音声ファイルのロードが失敗したらエラーにせず null object を返す。
// ゲーム進行自体には支障がないので。

#include <map>
#include <string>
#include <vector>

#include <boost/core/noncopyable.hpp>

#include <SDL_mixer.h>

// インスタンスが null object の場合もある。
class Music : private boost::noncopyable {
private:
    // null object ならば nullptr
    Mix_Music *music_;

    explicit Music(Mix_Music *music);

    void drop();

public:
    static Music null();

    // ロード失敗時は null object を返す。
    static Music load(const std::string &path);

    // movable
    Music(Music &&other) noexcept;
    Music &operator=(Music &&rhs) noexcept;

    ~Music();

    [[nodiscard]] bool is_null() const;

    // 成功したかどうかを返す。
    // 自身が null object の場合は何もせず true を返す(再生中の音楽は止まる)。
    [[nodiscard]] bool play() const;
};

// インスタンスが null object の場合もある。
class Sound : private boost::noncopyable {
private:
    // null object ならば nullptr
    Mix_Chunk *chunk_;

    explicit Sound(Mix_Chunk *chunk);

    void drop();

public:
    static Sound null();

    // ロード失敗時は null object を返す。
    static Sound load(const std::string &path);

    // movable
    Sound(Sound &&other) noexcept;
    Sound &operator=(Sound &&rhs) noexcept;

    ~Sound();

    [[nodiscard]] bool is_null() const;

    // 成功したかどうかを返す。
    // 自身が null object の場合は何もせず true を返す。
    [[nodiscard]] bool play() const;
};

// インスタンスが null object の場合もある。
class AudioAsset {
private:
    // null object ならば map は空

    std::map<std::string, std::vector<Music>> musics_;
    std::map<std::string, std::vector<Sound>> sounds_;
    std::vector<Music> music_null_;
    std::vector<Sound> sound_null_;

public:
    explicit AudioAsset(const std::string &dir_xtra);

    // noncopyable
    AudioAsset(const AudioAsset &) = delete;
    AudioAsset &operator=(const AudioAsset &) = delete;

    // movable
    AudioAsset(AudioAsset &&other) = default;
    AudioAsset &operator=(AudioAsset &&rhs) = default;

    // カテゴリと名前を指定して Music を得る。
    // 自身が null object の場合は null object を返す。
    [[nodiscard]] const std::vector<Music> &music(const std::string &category, const std::string &name) const;

    // 名前を指定して Sound を得る。
    // 自身が null object の場合は null object を返す。
    [[nodiscard]] const std::vector<Sound> &sound(const std::string &name) const;
};
