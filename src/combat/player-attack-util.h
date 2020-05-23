﻿#pragma once

#include "system/angband.h"
#include "combat/combat-options-type.h"

typedef enum chaotic_effect {
    CE_NONE = 0,
    CE_VAMPIRIC = 1,
    CE_QUAKE = 2,
    CE_CONFUSION = 3,
    CE_TELE_AWAY = 4,
    CE_POLYMORPH = 5,
} chaotic_effect;

typedef struct player_attack_type {
    s16b hand;
    combat_options mode;
    monster_type *m_ptr;
    bool backstab;
    bool suprise_attack;
    bool stab_fleeing;
    bool monk_attack;
    int num_blow;
    HIT_POINT attack_damage;
    GAME_TEXT m_name[MAX_NLEN];
    BIT_FLAGS flags[TR_FLAG_SIZE];
    chaotic_effect chaos_effect;
} player_attack_type;
