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

    [[nodiscard]] Mix_Music *get() const;
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

    [[nodiscard]] Mix_Chunk *get() const;
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

enum class MixerMusicPlayResult {
    OK,
    NULL_MUSIC, // ゲーム側で「この音楽がなければ他の音楽を試す」処理があるので必要
    FAIL, // 何らかの理由で失敗
};

enum class MixerSoundPlayResult {
    OK,
    NULL_SOUND, // 一応 Music 側に合わせた
    CHANNEL_FULL, // 空きチャンネルがない
    SAME_SOUND_FULL, // 同一サウンドの最大同時再生数に達している
};

// 同一サウンドの同時再生制限付きミキサー。
// 例えば大量の敵を同時に倒したときに敵の死亡音が大量に同時再生されると、
// SDL_mixer のチャンネルが占有されて他のサウンドが再生できなくなるし、単純に音
// 量が増幅されすぎてビックリする。よって制限を設ける。
//
// 事前に Mix_Init(), Mix_OpenAudio() を呼んでいることを仮定。
class Mixer {
private:
    // チャンネル i で最後に再生したサウンド
    std::vector<Mix_Chunk *> chunk_of_channel_;

    // 同一サウンドの最大同時再生数
    int max_same_sound_;

public:
    Mixer(int n_channel, int max_same_sound);

    // music を無限ループ設定で再生する。
    // nullptr を渡すと単に現在の音楽を停止し、NULL_MUSIC を返す。
    [[nodiscard]] MixerMusicPlayResult play_music(Mix_Music *music) const;

    // chunk を1回だけ再生する。MixerSoundPlayResult を返す。
    // nullptr を渡すと何もせず NULL_SOUND を返す。
    MixerSoundPlayResult play_sound(Mix_Chunk *chunk);

    void stop_music() const;
    void stop_sound() const;
};
