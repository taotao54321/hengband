﻿#include "main/sound-of-music.h"
#include "dungeon/quest.h"
#include "game-option/disturbance-options.h"
#include "game-option/special-options.h"
#include "main/music-definitions-table.h"
#include "system/floor-type-definition.h"
#include "term/screen-processor.h"

/*
 * Flush the screen, make a noise
 */
void bell(void)
{
    term_fresh();
    if (ring_bell)
        term_xtra(TERM_XTRA_NOISE, 0);

    flush();
}

/*
 * todo intをsound_typeに差し替える
 * @brief 音を鳴らす
 */
void sound(int val)
{
    if (!use_sound)
        return;

    term_xtra(TERM_XTRA_SOUND, val);
}

/*
 * Hack -- Play a music
 */
errr play_music(int type, int val)
{
    if (!use_music)
        return 1;

    return term_xtra(type, val);
}

/*
 * @brief ダンジョン用の通常BGMまたはクエスト用BGM
 * @param player_ptr プレーヤーへの参照ポインタ
 * @return BGMを鳴らすか後続処理で鳴らすBGMを決めるならばTRUE、鳴らさないならばFALSE
 * @details
 */
bool dungeon_quest_music(player_type *player_ptr)
{
    QUEST_IDX quest_id = player_ptr->current_floor_ptr->inside_quest;
    if (quest_id == 0)
        quest_id = quest_number(player_ptr, player_ptr->current_floor_ptr->dun_level);

    if (quest_id == 0)
        return TRUE;

    if (!play_music(TERM_XTRA_MUSIC_QUEST, quest_id))
        return FALSE;

    return play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_QUEST) != 0;
}

/*
 * Hack -- Select floor music.
 */
void select_floor_music(player_type *player_ptr)
{
    if (!use_music)
        return;

    if (player_ptr->ambush_flag) {
        if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_AMBUSH))
            return;
    }

    if (player_ptr->wild_mode) {
        if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_WILD))
            return;
    }

    if (player_ptr->current_floor_ptr->inside_arena) {
        if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_ARENA))
            return;
    }

    if (player_ptr->phase_out) {
        if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_BATTLE))
            return;
    }

    if (!dungeon_quest_music(player_ptr))
        return;

    if (player_ptr->dungeon_idx) {
        if (player_ptr->feeling == 2) {
            if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_DUN_FEEL2))
                return;
        } else if (player_ptr->feeling >= 3 && player_ptr->feeling <= 5) {
            if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_DUN_FEEL1))
                return;
        } else {
            if (!play_music(TERM_XTRA_MUSIC_DUNGEON, player_ptr->dungeon_idx))
                return;

            if (player_ptr->current_floor_ptr->dun_level < 40) {
                if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_DUN_LOW))
                    return;
            } else if (player_ptr->current_floor_ptr->dun_level < 80) {
                if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_DUN_MED))
                    return;
            } else {
                if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_DUN_HIGH))
                    return;
            }
        }
    }

    if (player_ptr->town_num) {
        if (!play_music(TERM_XTRA_MUSIC_TOWN, player_ptr->town_num))
            return;
        if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_TOWN))
            return;
        return;
    }

    if (!player_ptr->current_floor_ptr->dun_level) {
        if (player_ptr->lev >= 45) {
            if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_FIELD3))
                return;
        } else if (player_ptr->lev >= 25) {
            if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_FIELD2))
                return;
        } else {
            if (!play_music(TERM_XTRA_MUSIC_BASIC, MUSIC_BASIC_FIELD1))
                return;
        }
    }

    play_music(TERM_XTRA_MUSIC_MUTE, 0);
}
