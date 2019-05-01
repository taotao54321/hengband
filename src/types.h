﻿#pragma once

/*!
 * @file types.h
 * @brief グローバルな構造体の定義 / global type declarations
 * @date 2014/08/10
 * @author
 * <pre>
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 * </pre>
 * @details
 * <pre>
 * このファイルはangband.hでのみインクルードすること。
 * This file should ONLY be included by "angband.h"
 *
 * Note that "char" may or may not be signed, and that "signed char"
 * may or may not work on all machines.  So always use "s16b" or "s32b"
 * for signed values.  Also, note that unsigned values cause math problems
 * in many cases, so try to only use "u16b" and "u32b" for "bit flags",
 * unless you really need the extra bit of information, or you really
 * need to restrict yourself to a single byte for storage reasons.
 *
 * Also, if possible, attempt to restrict yourself to sub-fields of
 * known size (use "s16b" or "s32b" instead of "int", and "byte" instead
 * of "bool"), and attempt to align all fields along four-byte words, to
 * optimize storage issues on 32-bit machines.  Also, avoid "bit flags"
 * since these increase the code size and slow down execution.  When
 * you need to store bit flags, use one byte per flag, or, where space
 * is an issue, use a "byte" or "u16b" or "u32b", and add special code
 * to access the various bit flags.
 *
 * Many of these structures were developed to reduce the number of global
 * variables, facilitate structured program design, allow the use of ascii
 * template files, simplify access to indexed data, or facilitate efficient
 * clearing of many variables at once.
 *
 * Certain data is saved in multiple places for efficient access, currently,
 * this includes the tval/sval/weight fields in "object_type", various fields
 * in "header_type", and the "m_idx" and "o_idx" fields in "grid_type".  All
 * of these could be removed, but this would, in general, slow down the game
 * and increase the complexity of the code.
 * </pre>
 */

#include "h-type.h"
#include "defines.h"
#include "object.h"

//#include "player-skill.h"


/*
 * Information about "ego-items".
 */

typedef struct ego_item_type ego_item_type;

struct ego_item_type
{
	STR_OFFSET name;			/* Name (offset) */
	STR_OFFSET text;			/* Text (offset) */

	INVENTORY_IDX slot;		/*!< 装備部位 / Standard slot value */
	PRICE rating;		/*!< ベースアイテムからの価値加速 / Rating boost */

	DEPTH level;			/* Minimum level */
	RARITY rarity;		/* Object rarity */

	HIT_PROB max_to_h;		/* Maximum to-hit bonus */
	HIT_POINT max_to_d;		/* Maximum to-dam bonus */
	ARMOUR_CLASS max_to_a;		/* Maximum to-ac bonus */

	PARAMETER_VALUE max_pval;		/* Maximum pval */

	PRICE cost;			/* Ego-item "cost" */

	BIT_FLAGS flags[TR_FLAG_SIZE];	/* Ego-Item Flags */
	BIT_FLAGS gen_flags;		/* flags for generate */

	IDX act_idx;		/* Activative ability index */
};




/*
 * Monster blow structure
 *
 *	- Method (RBM_*)
 *	- Effect (RBE_*)
 *	- Damage Dice
 *	- Damage Sides
 */

typedef struct monster_blow monster_blow;

struct monster_blow
{
	BLOW_METHOD method;
	BLOW_EFFECT effect;
	DICE_NUMBER d_dice;
	DICE_SID d_side;
};


typedef struct mbe_info_type mbe_info_type;

struct mbe_info_type
{
	int power;        /* The attack "power" */
	int explode_type; /* Explosion effect */
};


/*
 * An entry for the object/monster allocation functions
 *
 * Pass 1 is determined from allocation information
 * Pass 2 is determined from allocation restriction
 * Pass 3 is determined from allocation calculation
 */

typedef struct alloc_entry alloc_entry;

struct alloc_entry
{
	KIND_OBJECT_IDX index;		/* The actual index */

	DEPTH level;		/* Base dungeon level */
	PROB prob1;		/* Probability, pass 1 */
	PROB prob2;		/* Probability, pass 2 */
	PROB prob3;		/* Probability, pass 3 */

	u16b total;		/* Unused for now */
};



/*
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */
typedef struct store_type store_type;

struct store_type
{
	byte type;				/* Store type */

	byte owner;				/* Owner index */
	byte extra;				/* Unused for now */

	s16b insult_cur;		/* Insult counter */

	s16b good_buy;			/* Number of "good" buys */
	s16b bad_buy;			/* Number of "bad" buys */

	s32b store_open;		/* Closed until this current_world_ptr->game_turn */

	s32b last_visit;		/* Last visited on this current_world_ptr->game_turn */

	s16b table_num;			/* Table -- Number of entries */
	s16b table_size;		/* Table -- Total Size of Array */
	s16b *table;			/* Table -- Legal item kinds */

	s16b stock_num;			/* Stock -- Number of entries */
	s16b stock_size;		/* Stock -- Total Size of Array */
	object_type *stock;		/* Stock -- Actual stock items */
};


/*
 * The "name" of spell 'N' is stored as spell_names[X][N],
 * where X is 0 for mage-spells and 1 for priest-spells.
 */
typedef struct magic_type magic_type;

struct magic_type
{
	PLAYER_LEVEL slevel;	/* Required level (to learn) */
	MANA_POINT smana;		/* Required mana (to cast) */
	PERCENTAGE sfail;		/* Minimum chance of failure */
	EXP sexp;				/* Encoded experience bonus */
};


/*
 * Player sex info
 */

typedef struct player_sex player_sex;

struct player_sex
{
	concptr title;			/* Type of sex */
	concptr winner;		/* Name of winner */
#ifdef JP
	concptr E_title;		/* 英語性別 */
	concptr E_winner;		/* 英語性別 */
#endif
};


/*
 * Player racial info
 */

typedef struct player_race player_race;

struct player_race
{
	concptr title;			/* Type of race */

#ifdef JP
	concptr E_title;		/* 英語種族 */
#endif
	s16b r_adj[6];		/* Racial stat bonuses */

	s16b r_dis;			/* disarming */
	s16b r_dev;			/* magic devices */
	s16b r_sav;			/* saving throw */
	s16b r_stl;			/* stealth */
	s16b r_srh;			/* search ability */
	s16b r_fos;			/* search frequency */
	s16b r_thn;			/* combat (normal) */
	s16b r_thb;			/* combat (shooting) */

	byte r_mhp;			/* Race hit-dice modifier */
	byte r_exp;			/* Race experience factor */

	byte b_age;			/* base age */
	byte m_age;			/* mod age */

	byte m_b_ht;		/* base height (males) */
	byte m_m_ht;		/* mod height (males) */
	byte m_b_wt;		/* base weight (males) */
	byte m_m_wt;		/* mod weight (males) */

	byte f_b_ht;		/* base height (females) */
	byte f_m_ht;		/* mod height (females)	  */
	byte f_b_wt;		/* base weight (females) */
	byte f_m_wt;		/* mod weight (females) */

	byte infra;			/* Infra-vision	range */

	u32b choice;        /* Legal class choices */
/*    byte choice_xtra;   */
};


typedef struct player_seikaku player_seikaku;
struct player_seikaku
{
	concptr title;			/* Type of seikaku */

#ifdef JP
	concptr E_title;		/* 英語性格 */
#endif

	s16b a_adj[6];		/* seikaku stat bonuses */

	s16b a_dis;			/* seikaku disarming */
	s16b a_dev;			/* seikaku magic devices */
	s16b a_sav;			/* seikaku saving throw */
	s16b a_stl;			/* seikaku stealth */
	s16b a_srh;			/* seikaku search ability */
	s16b a_fos;			/* seikaku search frequency */
	s16b a_thn;			/* seikaku combat (normal) */
	s16b a_thb;			/* seikaku combat (shooting) */

	s16b a_mhp;			/* Race hit-dice modifier */

	byte no;			/* の */
	byte sex;			/* seibetu seigen */
};



/*
 * A structure to hold "rolled" information
 */
typedef struct birther birther;

struct birther
{
	SEX_IDX psex;		/* Sex index */
	RACE_IDX prace;		/* Race index */
	CLASS_IDX pclass;	/* Class index */
	CHARACTER_IDX pseikaku;	/* Seikaku index */
	REALM_IDX realm1;	/* First magic realm */
	REALM_IDX realm2;	/* Second magic realm */

	s16b age;
	s16b ht;
	s16b wt;
	s16b sc;

	PRICE au; /*!< 初期の所持金 */

	BASE_STATUS stat_max[6];	/* Current "maximal" stat values */
	BASE_STATUS stat_max_max[6];	/* Maximal "maximal" stat values */
	HIT_POINT player_hp[PY_MAX_LEVEL];

	PATRON_IDX chaos_patron;

	s16b vir_types[8];

	char history[4][60];

	bool quick_ok;
};


/* For Monk martial arts */

typedef struct martial_arts martial_arts;

struct martial_arts
{
	concptr desc;       /* A verbose attack description */
	PLAYER_LEVEL min_level;  /* Minimum level to use */
	int chance;     /* Chance of 'success' */
	int dd;         /* Damage dice */
	int ds;         /* Damage sides */
	int effect;     /* Special effects */
};

typedef struct kamae kamae;

struct kamae
{
	concptr desc;       /* A verbose kamae description */
	PLAYER_LEVEL min_level;  /* Minimum level to use */
	concptr info;
};


/* Imitator */

typedef struct monster_power monster_power;
struct monster_power
{
	PLAYER_LEVEL level;
	MANA_POINT smana;
	PERCENTAGE fail;
	int     manedam;
	int     manefail;
	int     use_stat;
	concptr    name;
};


/*
 * A structure describing a town with
 * stores and buildings
 */
typedef struct town_type town_type;
struct town_type
{
	GAME_TEXT name[32];
	u32b seed;      /* Seed for RNG */
	store_type *store;    /* The stores [MAX_STORES] */
	byte numstores;
};

/*
 * Sort-array element
 */
typedef struct tag_type tag_type;

struct tag_type
{
	int tag;
	int index;
};

typedef bool (*monsterrace_hook_type)(MONRACE_IDX r_idx);


/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(object_type *);


typedef struct
{
	FEAT_IDX feat;    /* Feature tile */
	PERCENTAGE percent; /* Chance of type */
}
feat_prob;


/*!
 * @struct autopick_type
 * @brief 自動拾い/破壊設定データの構造体 / A structure type for entry of auto-picker/destroyer
 */
typedef struct {
	concptr name;          /*!< 自動拾い/破壊定義の名称一致基準 / Items which have 'name' as part of its name match */
	concptr insc;          /*!< 対象となったアイテムに自動で刻む内容 / Items will be auto-inscribed as 'insc' */
	BIT_FLAGS flag[2];       /*!< キーワードに関する汎用的な条件フラグ / Misc. keyword to be matched */
	byte action;        /*!< 対象のアイテムを拾う/破壊/放置するかの指定フラグ / Auto-pickup or Destroy or Leave items */
	byte dice;          /*!< 武器のダイス値基準値 / Weapons which have more than 'dice' dice match */
	byte bonus;         /*!< アイテムのボーナス基準値 / Items which have more than 'bonus' magical bonus match */
} autopick_type;


/*
 *  A structure type for terrain template of saving dungeon floor
 */
typedef struct
{
	BIT_FLAGS info;
	FEAT_IDX feat;
	FEAT_IDX mimic;
	s16b special;
	u16b occurrence;
} cave_template_type;


#ifdef TRAVEL
/*
 *  A structure type for travel command
 */
typedef struct {
	int run; /* Remaining grid number */
	int cost[MAX_HGT][MAX_WID];
	POSITION x; /* Target X */
	POSITION y; /* Target Y */
	DIRECTION dir; /* Running direction */
} travel_type;
#endif

typedef struct {
	int flag;
	int type;
	concptr name;
} dragonbreath_type;
