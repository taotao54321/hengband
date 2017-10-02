﻿/*!
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
 * in "header_type", and the "m_idx" and "o_idx" fields in "cave_type".  All
 * of these could be removed, but this would, in general, slow down the game
 * and increase the complexity of the code.
 * </pre>
 */


/*!
 * @struct feature_state
 * @brief 地形状態変化指定構造体 / Feature state structure
 */
typedef struct feature_state feature_state;

struct feature_state
{
	byte action; /*!< 変化条件をFF_*のIDで指定 / Action (FF_*) */
	s16b result; /*!< 変化先ID / Result (f_info ID) */
};


/*!
 * @struct feature_type
 * @brief 地形情報の構造体 / Information about terrain "features"
 */

typedef struct feature_type feature_type;

struct feature_type
{
	STR_OFFSET name;                /*!< 地形名参照のためのネームバッファオフセット値 / Name (offset) */
	STR_OFFSET text;                /*!< 地形説明参照のためのネームバッファオフセット値 /  Text (offset) */
	STR_OFFSET tag;                 /*!< 地形特性タグ参照のためのネームバッファオフセット値 /  Tag (offset) */

	IDX mimic;               /*!< 未確定時の外形地形ID / Feature to mimic */

	BIT_FLAGS flags[FF_FLAG_SIZE]; /*!< 地形の基本特性ビット配列 / Flags */

	u16b priority;            /*!< 縮小表示で省略する際の表示優先度 / Map priority */
	IDX destroyed;           /*!< *破壊*に巻き込まれた時の地形移行先(未実装？) / Default destroyed state */

	feature_state state[MAX_FEAT_STATES]; /*!< feature_state テーブル */

	byte subtype;  /*!< 副特性値 */
	byte power;    /*!< 地形強度 */

	byte d_attr[F_LIT_MAX];   /*!< デフォルトの地形シンボルカラー / Default feature attribute */
	byte d_char[F_LIT_MAX];   /*!< デフォルトの地形シンボルアルファベット / Default feature character */

	byte x_attr[F_LIT_MAX];   /*!< 設定変更後の地形シンボルカラー / Desired feature attribute */
	byte x_char[F_LIT_MAX];   /*!< 設定変更後の地形シンボルアルファベット / Desired feature character */
};


/*!
 * @struct object_kind
 * @brief ベースアイテム情報の構造体 / Information about object "kinds", including player knowledge.
 * @details
 * ゲーム進行用のセーブファイル上では aware と tried のみ保存対象とすること。と英文ではあるが実際はもっとある様子である。 /
 * Only "aware" and "tried" are saved in the savefile
 */

typedef struct object_kind object_kind;

struct object_kind
{
	STR_OFFSET name;			/*!< ベースアイテム名参照のためのネームバッファオフセット値 / Name (offset) */
	STR_OFFSET text;			/*!< 解説テキスト参照のためのネームバッファオフセット値 / Text (offset) */
	STR_OFFSET flavor_name;	/*!< 未確定名参照のためのネームバッファオフセット値 / Flavor name (offset) */

	byte tval;			/*!< ベースアイテム種別の大項目値 Object type */
	byte sval;			/*!< ベースアイテム種別の小項目値 Object sub type */

	PARAMETER_VALUE pval;	/*!< ベースアイテムのpval（能力修正共通値） Object extra info */

	HIT_PROB to_h;			/*!< ベースアイテムの命中修正値 / Bonus to hit */
	HIT_POINT to_d;			/*!< ベースアイテムのダメージ修正値 / Bonus to damage */
	ARMOUR_CLASS to_a;			/*!< ベースアイテムのAC修正値 / Bonus to armor */

	ARMOUR_CLASS ac;			/*!< ベースアイテムのAC基本値 /  Base armor */

	DICE_NUMBER dd;
	DICE_SID ds;		/*!< ダメージダイスの数と大きさ / Damage dice/sides */

	WEIGHT weight;		/*!< ベースアイテムの重量 / Weight */

	PRICE cost;			/*!< ベースアイテムの基本価値 / Object "base cost" */

	u32b flags[TR_FLAG_SIZE];	/*!< ベースアイテムの基本特性ビット配列 / Flags */

	u32b gen_flags;		/*!< ベースアイテムの生成特性ビット配列 / flags for generate */

	byte locale[4];		/*!< ベースアイテムの生成階テーブル / Allocation level(s) */
	byte chance[4];		/*!< ベースアイテムの生成確率テーブル / Allocation chance(s) */

	byte level;			/*!< ベースアイテムの基本生成階 / Level */
	byte extra;			/*!< その他色々のビットフラグ配列 / Something */

	SYMBOL_COLOR d_attr;		/*!< デフォルトのアイテムシンボルカラー / Default object attribute */
	SYMBOL_CODE d_char;		/*!< デフォルトのアイテムシンボルアルファベット / Default object character */

	SYMBOL_COLOR x_attr;		/*!< 設定変更後のアイテムシンボルカラー /  Desired object attribute */
	SYMBOL_CODE x_char;		/*!< 設定変更後のアイテムシンボルアルファベット /  Desired object character */

	s16b flavor;		/*!< 調査中(TODO) / Special object flavor (or zero) */

	bool easy_know;		/*!< ベースアイテムが初期からベース名を判断可能かどうか / This object is always known (if aware) */

	bool aware;			/*!< ベースアイテムが鑑定済かどうか /  The player is "aware" of the item's effects */

	bool tried;			/*!< ベースアイテムを未鑑定のまま試したことがあるか /  The player has "tried" one of the items */

	byte act_idx;		/*!< 発動能力のID /  Activative ability index */
};



typedef struct artifact_type artifact_type;

/*!
 * @struct artifact_type
 * @brief 固定アーティファクト情報の構造体 / Artifact structure.
 * @details
 * @note
 * the save-file only writes "cur_num" to the savefile.
 * "max_num" is always "1" (if that artifact "exists")
 */
struct artifact_type
{
	STR_OFFSET name;			/*!< アーティファクト名(headerオフセット参照) / Name (offset) */
	STR_OFFSET text;			/*!< アーティファクト解説(headerオフセット参照) / Text (offset) */

	byte tval;			/*!< ベースアイテム大項目ID / Artifact type */
	byte sval;			/*!< ベースアイテム小項目ID / Artifact sub type */

	s16b pval;			/*!< pval修正値 / Artifact extra info */

	s16b to_h;			/*!< 命中ボーナス値 /  Bonus to hit */
	s16b to_d;			/*!< ダメージボーナス値 / Bonus to damage */
	s16b to_a;			/*!< ACボーナス値 / Bonus to armor */

	s16b ac;			/*!< 上書きベースAC値 / Base armor */

	byte dd, ds;		/*!< ダイス値 / Damage when hits */

	s16b weight;		/*!< 重量 / Weight */

	s32b cost;			/*!< 基本価格 / Artifact "cost" */

	u32b flags[TR_FLAG_SIZE];       /*! アイテムフラグ / Artifact Flags */

	u32b gen_flags;		/*! アイテム生成フラグ / flags for generate */

	byte level;			/*! 基本生成階 / Artifact level */
	byte rarity;		/*! レアリティ / Artifact rarity */

	byte cur_num;		/*! 現在の生成数 / Number created (0 or 1) */
	byte max_num;		/*! (未使用)最大生成数 / Unused (should be "1") */

	s16b floor_id;      /*! アイテムを落としたフロアのID / Leaved on this location last time */

	byte act_idx;		/*! 発動能力ID / Activative ability index */
};


/*
 * Information about "ego-items".
 */

typedef struct ego_item_type ego_item_type;

struct ego_item_type
{
	STR_OFFSET name;			/* Name (offset) */
	STR_OFFSET text;			/* Text (offset) */

	byte slot;			/* Standard slot value */
	byte rating;		/* Rating boost */

	DEPTH level;			/* Minimum level */
	RARITY rarity;		/* Object rarity */

	byte max_to_h;		/* Maximum to-hit bonus */
	byte max_to_d;		/* Maximum to-dam bonus */
	byte max_to_a;		/* Maximum to-ac bonus */

	PARAMETER_VALUE max_pval;		/* Maximum pval */

	s32b cost;			/* Ego-item "cost" */

	u32b flags[TR_FLAG_SIZE];	/* Ego-Item Flags */

	u32b gen_flags;		/* flags for generate */

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
	byte method;
	byte effect;
	byte d_dice;
	byte d_side;
};


typedef struct mbe_info_type mbe_info_type;

struct mbe_info_type
{
	int power;        /* The attack "power" */
	int explode_type; /* Explosion effect */
};


/*
 * Monster "race" information, including racial memories
 *
 * Note that "d_attr" and "d_char" are used for MORE than "visual" stuff.
 *
 * Note that "x_attr" and "x_char" are used ONLY for "visual" stuff.
 *
 * Note that "cur_num" (and "max_num") represent the number of monsters
 * of the given race currently on (and allowed on) the current level.
 * This information yields the "dead" flag for Unique monsters.
 *
 * Note that "max_num" is reset when a new player is created.
 * Note that "cur_num" is reset when a new level is created.
 *
 * Note that several of these fields, related to "recall", can be
 * scrapped if space becomes an issue, resulting in less "complete"
 * monster recall (no knowledge of spells, etc).  All of the "recall"
 * fields have a special prefix to aid in searching for them.
 */


typedef struct monster_race monster_race;

struct monster_race
{
	STR_OFFSET name;	/*!< 名前データのオフセット(日本語) /  Name offset(Japanese) */
#ifdef JP
	STR_OFFSET E_name;		/*!< 名前データのオフセット(英語) /  Name offset(English) */
#endif
	STR_OFFSET text;				/*!< 思い出テキストのオフセット / Lore text offset */

	byte hdice;				/*!< HPのダイス数 / Creatures hit dice count */
	byte hside;				/*!< HPのダイス面数 / Creatures hit dice sides */

	s16b ac;				/*!< アーマークラス / Armour Class */

	s16b sleep;				/*!< 睡眠値 / Inactive counter (base) */
	byte aaf;				/*!< 感知範囲(1-100スクエア) / Area affect radius (1-100) */
	byte speed;				/*!< 加速(110で+0) / Speed (normally 110) */

	s32b mexp;				/*!< 殺害時基本経験値 / Exp value for kill */

	s16b extra;				/*!< 未使用 /  Unused (for now) */

	byte freq_spell;		/*!< 魔法＆特殊能力仕様頻度(1/n) /  Spell frequency */

	u32b flags1;			/* Flags 1 (general) */
	u32b flags2;			/* Flags 2 (abilities) */
	u32b flags3;			/* Flags 3 (race/resist) */
	u32b flags4;			/* Flags 4 (inate/breath) */
	u32b flags7;			/* Flags 7 (movement related abilities) */
	u32b flags8;			/* Flags 8 (wilderness info) */
	u32b flags9;			/* Flags 9 (drops info) */
	u32b flagsr;			/* Flags R (resistances info) */

	u32b a_ability_flags1;	/* Activate Ability Flags 5 (normal spells) */
	u32b a_ability_flags2;	/* Activate Ability Flags 6 (special spells) */
	u32b a_ability_flags3;	/* Activate Ability Flags 7 (implementing) */
	u32b a_ability_flags4;	/* Activate Ability Flags 8 (implementing) */

	monster_blow blow[4];	/* Up to four blows per round */
	u16b reinforce_id[6];
	u16b reinforce_dd[6];
	u16b reinforce_ds[6];

	u16b artifact_id[4];	/* 特定アーティファクトドロップID */
	RARITY artifact_rarity[4];	/* 特定アーティファクトレア度 */
	PERCENTAGE artifact_percent[4]; /* 特定アーティファクトドロップ率 */

	PERCENTAGE arena_ratio;		/* アリーナの評価修正値(%基準 / 0=100%) / Arena */

	IDX next_r_idx;
	u32b next_exp;

	DEPTH level;			/* Level of creature */
	RARITY rarity;			/* Rarity of creature */


	SYMBOL_COLOR d_attr;		/* Default monster attribute */
	SYMBOL_CODE d_char;			/* Default monster character */


	SYMBOL_COLOR x_attr;		/* Desired monster attribute */
	SYMBOL_CODE x_char;			/* Desired monster character */


	byte max_num;			/* Maximum population allowed per level */

	byte cur_num;			/* Monster population on current level */

	s16b floor_id;          /* Location of unique monster */


	s16b r_sights;			/* Count sightings of this monster */
	s16b r_deaths;			/* Count deaths from this monster */

	s16b r_pkills;			/* Count visible monsters killed in this life */
	s16b r_akills;			/* Count all monsters killed in this life */
	s16b r_tkills;			/* Count monsters killed in all lives */

	byte r_wake;			/* Number of times woken up (?) */
	byte r_ignore;			/* Number of times ignored (?) */

	byte r_xtra1;			/* Something (unused) */
	byte r_xtra2;			/* Something (unused) */

	byte r_drop_gold;		/* Max number of gold dropped at once */
	byte r_drop_item;		/* Max number of item dropped at once */

	byte r_cast_spell;		/* Max number of other spells seen */

	byte r_blows[4];		/* Number of times each blow type was seen */

	u32b r_flags1;			/* Observed racial flags */
	u32b r_flags2;			/* Observed racial flags */
	u32b r_flags3;			/* Observed racial flags */
	u32b r_flags4;			/* Observed racial flags */
	u32b r_flags5;			/* Observed racial flags */
	u32b r_flags6;			/* Observed racial flags */
	/* u32b r_flags7; */	/* Observed racial flags */
	u32b r_flagsr;			/* Observed racial resistance flags */
};



/*
 * Information about "vault generation"
 */

typedef struct vault_type vault_type;

struct vault_type
{
	STR_OFFSET name;	/* Name (offset) */
	STR_OFFSET text;	/* Text (offset) */

	byte typ;			/* Vault type */
	byte rat;			/* Vault rating */
	byte hgt;			/* Vault height */
	byte wid;			/* Vault width */
};


/*
 * Information about "skill"
 */

typedef struct skill_table skill_table;

struct skill_table
{
	SUB_EXP w_start[5][64];	  /* start weapon exp */
	SUB_EXP w_max[5][64];        /* max weapon exp */
	SUB_EXP s_start[10];	  /* start skill */
	SUB_EXP s_max[10];           /* max skill */
};


/*
 * A single "grid" in a Cave
 *
 * Note that several aspects of the code restrict the actual cave
 * to a max size of 256 by 256.  In partcular, locations are often
 * saved as bytes, limiting each coordinate to the 0-255 range.
 *
 * The "o_idx" and "m_idx" fields are very interesting.  There are
 * many places in the code where we need quick access to the actual
 * monster or object(s) in a given cave grid.  The easiest way to
 * do this is to simply keep the index of the monster and object
 * (if any) with the grid, but this takes 198*66*4 bytes of memory.
 * Several other methods come to mind, which require only half this
 * amound of memory, but they all seem rather complicated, and would
 * probably add enough code that the savings would be lost.  So for
 * these reasons, we simply store an index into the "o_list" and
 * "m_list" arrays, using "zero" when no monster/object is present.
 *
 * Note that "o_idx" is the index of the top object in a stack of
 * objects, using the "next_o_idx" field of objects (see below) to
 * create the singly linked list of objects.  If "o_idx" is zero
 * then there are no objects in the grid.
 *
 * Note the special fields for the "MONSTER_FLOW" code.
 */

typedef struct cave_type cave_type;

struct cave_type
{
	u16b info;		/* Hack -- cave flags */

	IDX feat;		/* Hack -- feature type */
	IDX o_idx;		/* Object in this grid */
	IDX m_idx;		/* Monster in this grid */

	s16b special;	/* Special cave info */

	s16b mimic;		/* Feature to mimic */

	byte cost;		/* Hack -- cost of flowing */
	byte dist;		/* Hack -- distance from player */
	byte when;		/* Hack -- when cost was computed */
};



/*
 * Simple structure to hold a map location
 */
typedef struct coord coord;

struct coord
{
	POSITION y;
	POSITION x;
};



/*
 * Object information, for a specific object.
 *
 * Note that a "discount" on an item is permanent and never goes away.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Note that "object" records are "copied" on a fairly regular basis,
 * and care must be taken when handling such objects.
 *
 * Note that "object flags" must now be derived from the object kind,
 * the artifact and ego-item indexes, and the two "xtra" fields.
 *
 * Each cave grid points to one (or zero) objects via the "o_idx"
 * field (above).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a "stack" of objects in the same grid.
 *
 * Each monster points to one (or zero) objects via the "hold_o_idx"
 * field (below).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a pile of objects held by the monster.
 *
 * The "held_m_idx" field is used to indicate which monster, if any,
 * is holding the object.  Objects being held have "ix=0" and "iy=0".
 */

typedef struct object_type object_type;

struct object_type
{
	IDX k_idx;			/* Kind index (zero if "dead") */

	POSITION iy;			/* Y-position on map, or zero */
	POSITION ix;			/* X-position on map, or zero */

	OBJECT_TYPE_VALUE tval;			/* Item type (from kind) */
	OBJECT_SUBTYPE_VALUE sval;			/* Item sub-type (from kind) */

	PARAMETER_VALUE pval;			/* Item extra-parameter */

	DISCOUNT_RATE discount;		/* Discount (if any) */

	ITEM_NUMBER number;	/* Number of items */

	WEIGHT weight;		/* Item weight */

	IDX name1;			/* Artifact type, if any */
	IDX name2;			/* Ego-Item type, if any */

	byte xtra1;			/* Extra info type (now unused) */
	byte xtra2;			/* Extra info activation index */
	byte xtra3;			/* Extra info for weaponsmith */
	s16b xtra4;			/* Extra info fuel or captured monster's current HP */
	s16b xtra5;			/* Extra info captured monster's max HP */

	HIT_PROB to_h;			/* Plusses to hit */
	HIT_POINT to_d;			/* Plusses to damage */
	ARMOUR_CLASS to_a;			/* Plusses to AC */

	ARMOUR_CLASS ac;			/* Normal AC */

	DICE_NUMBER dd;
	DICE_SID ds;		/* Damage dice/sides */

	TIME_EFFECT timeout;	/* Timeout Counter */

	byte ident;			/* Special flags  */

	byte marked;		/* Object is marked */

	u16b inscription;	/* Inscription index */
	u16b art_name;      /* Artifact name (random artifacts) */

	byte feeling;          /* Game generated inscription number (eg, pseudo-id) */

	BIT_FLAGS art_flags[TR_FLAG_SIZE];        /* Extra Flags for ego and artifacts */
	BIT_FLAGS curse_flags;        /* Flags for curse */

	IDX next_o_idx;	/* Next object in stack (if any) */
	IDX held_m_idx;	/* Monster holding us (if any) */
};



/*
 * Monster information, for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 *
 * The "hold_o_idx" field points to the first object of a stack
 * of objects (if any) being carried by the monster (see above).
 */

typedef struct monster_type monster_type;

struct monster_type
{
	IDX r_idx;		/* Monster race index */
	IDX ap_r_idx;		/* Monster race appearance index */
	byte sub_align;		/* Sub-alignment for a neutral monster */

	POSITION fy;		/* Y location on map */
	POSITION fx;		/* X location on map */

	HIT_POINT hp;		/* Current Hit points */
	HIT_POINT maxhp;		/* Max Hit points */
	HIT_POINT max_maxhp;		/* Max Max Hit points */
	u32b dealt_damage;		/* Sum of damages dealt by player */

	TIME_EFFECT mtimed[MAX_MTIMED];	/* Timed status counter */

	byte mspeed;	        /* Monster "speed" */
	ACTION_ENERGY energy_need;	/* Monster "energy" */

	POSITION cdis;		/* Current dis from player */

	BIT_FLAGS8 mflag;	/* Extra monster flags */
	BIT_FLAGS8 mflag2;	/* Extra monster flags */

	bool ml;		/* Monster is "visible" */

	IDX hold_o_idx;	/* Object being held (if any) */

	POSITION target_y;		/* Can attack !los player */
	POSITION target_x;		/* Can attack !los player */

	u16b nickname;		/* Monster's Nickname */

	u32b exp;

	u32b smart;			/* Field for "smart_learn" */

	s16b parent_m_idx;
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
	s16b index;		/* The actual index */

	byte level;		/* Base dungeon level */
	byte prob1;		/* Probability, pass 1 */
	byte prob2;		/* Probability, pass 2 */
	byte prob3;		/* Probability, pass 3 */

	u16b total;		/* Unused for now */
};



/*
 * Available "options"
 *
 *	- Address of actual option variable (or NULL)
 *
 *	- Normal Value (TRUE or FALSE)
 *
 *	- Option Page Number (or zero)
 *
 *	- Savefile Set (or zero)
 *	- Savefile Bit in that set
 *
 *	- Textual name (or NULL)
 *	- Textual description
 */

typedef struct option_type option_type;

struct option_type
{
	bool	*o_var;

	byte	o_norm;

	byte	o_page;

	byte	o_set;
	byte	o_bit;

	cptr	o_text;
	cptr	o_desc;
};


typedef struct quest_type quest_type;

/*!
 * @struct quest_type
 * @brief クエスト情報の構造体 / Structure for the "quests".
 */

struct quest_type
{
	s16b status;            /*!< クエストの進行ステータス / Is the quest taken, completed, finished? */

	s16b type;              /*!< クエストの種別 / The quest type */

	char name[60];          /*!< クエスト名 / Quest name */
	DEPTH level;             /*!< 処理階層 / Dungeon level */
	IDX r_idx;             /*!< クエスト対象のモンスターID / Monster race */
	   
	s16b cur_num;           /*!< 撃破したモンスターの数 / Number killed */
	s16b max_num;           /*!< 求められるモンスターの撃破数 / Number required */

	IDX k_idx;             /*!< クエスト対象のアイテムID / object index */
	s16b num_mon;           /*!< QUEST_TYPE_KILL_NUMBER時の目標撃破数 number of monsters on level */

	byte flags;             /*!< クエストに関するフラグビット / quest flags */
	byte dungeon;           /*!< クエスト対象のダンジョンID / quest dungeon */

	byte complev;           /*!< クリア時プレイヤーレベル / player level (complete) */
	u32b comptime;          /*!< クリア時ゲーム時間 /  quest clear time*/
};


/*
 * A store owner
 */
typedef struct owner_type owner_type;

struct owner_type
{
	cptr owner_name;	/* Name */

	s32b max_cost;		/* Purse limit */

	byte max_inflate;	/* Inflation (max) */
	byte min_inflate;	/* Inflation (min) */

	byte haggle_per;	/* Haggle unit */

	byte insult_max;	/* Insult limit */

	byte owner_race;	/* Owner race */
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

	s32b store_open;		/* Closed until this turn */

	s32b last_visit;		/* Last visited on this turn */

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
	byte slevel;		/* Required level (to learn) */
	byte smana;			/* Required mana (to cast) */
	byte sfail;			/* Minimum chance of failure */
	byte sexp;			/* Encoded experience bonus */
};


/*
 * Information about the player's "magic"
 *
 * Note that a player with a "spell_book" of "zero" is illiterate.
 */

typedef struct player_magic player_magic;

struct player_magic
{
	int spell_book;		/* Tval of spell books (if any) */
	int spell_xtra;		/* Something for later */

	int spell_stat;		/* Stat for spells (if any)  */
	int spell_type;		/* Spell type (mage/priest) */

	int spell_first;		/* Level of first spell */
	int spell_weight;		/* Weight that hurts spells */

	magic_type info[MAX_MAGIC][32];    /* The available spells */
};



/*
 * Player sex info
 */

typedef struct player_sex player_sex;

struct player_sex
{
	cptr title;			/* Type of sex */
	cptr winner;		/* Name of winner */
#ifdef JP
	cptr E_title;		/* 英語性別 */
	cptr E_winner;		/* 英語性別 */
#endif
};


/*
 * Player racial info
 */

typedef struct player_race player_race;

struct player_race
{
	cptr title;			/* Type of race */

#ifdef JP
	cptr E_title;		/* 英語種族 */
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


/*
 * Player class info
 */

typedef struct player_class player_class;

struct player_class
{
	cptr title;			/* Type of class */

#ifdef JP
	cptr E_title;		/* 英語職業 */
#endif
	s16b c_adj[6];		/* Class stat modifier */

	s16b c_dis;			/* class disarming */
	s16b c_dev;			/* class magic devices */
	s16b c_sav;			/* class saving throws */
	s16b c_stl;			/* class stealth */
	s16b c_srh;			/* class searching ability */
	s16b c_fos;			/* class searching frequency */
	s16b c_thn;			/* class to hit (normal) */
	s16b c_thb;			/* class to hit (bows) */

	s16b x_dis;			/* extra disarming */
	s16b x_dev;			/* extra magic devices */
	s16b x_sav;			/* extra saving throws */
	s16b x_stl;			/* extra stealth */
	s16b x_srh;			/* extra searching ability */
	s16b x_fos;			/* extra searching frequency */
	s16b x_thn;			/* extra to hit (normal) */
	s16b x_thb;			/* extra to hit (bows) */

	s16b c_mhp;			/* Class hit-dice adjustment */
	s16b c_exp;			/* Class experience factor */

	byte pet_upkeep_div; /* Pet upkeep divider */
};


typedef struct player_seikaku player_seikaku;
struct player_seikaku
{
	cptr title;			/* Type of seikaku */

#ifdef JP
	cptr E_title;		/* 英語性格 */
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
 * Most of the "player" information goes here.
 *
 * This stucture gives us a large collection of player variables.
 *
 * This structure contains several "blocks" of information.
 *   (1) the "permanent" info
 *   (2) the "variable" info
 *   (3) the "transient" info
 *
 * All of the "permanent" info, and most of the "variable" info,
 * is saved in the savefile.  The "transient" info is recomputed
 * whenever anything important changes.
 */

typedef struct player_type player_type;

struct player_type
{
	POSITION oldpy;		/* Previous player location -KMW- */
	POSITION oldpx;		/* Previous player location -KMW- */

	CHARACTER_IDX psex;			/* Sex index */
	CHARACTER_IDX prace;			/* Race index */
	CHARACTER_IDX pclass;		/* Class index */
	CHARACTER_IDX pseikaku;		/* Seikaku index */
	CHARACTER_IDX realm1;        /* First magic realm */
	CHARACTER_IDX realm2;        /* Second magic realm */
	CHARACTER_IDX oops;			/* Unused */

	DICE_SID hitdie;		/* Hit dice (sides) */
	u16b expfact;       /* Experience factor
			     * Note: was byte, causing overflow for Amberite
			     * characters (such as Amberite Paladins)
			     */

	s16b age;			/* Characters age */
	s16b ht;			/* Height */
	s16b wt;			/* Weight */
	s16b sc;			/* Social Class */

	PRICE au;			/* Current Gold */

	EXP max_max_exp;	/* Max max experience (only to calculate score) */
	EXP max_exp;		/* Max experience */
	EXP exp;			/* Cur experience */
	EXP exp_frac;		/* Cur exp frac (times 2^16) */

	PLAYER_LEVEL lev;			/* Level */

	s16b town_num;			/* Current town number */
	s16b arena_number;		/* monster number in arena -KMW- */
	bool inside_arena;		/* Is character inside arena? */
	IDX inside_quest;		/* Inside quest level */
	bool inside_battle;		/* Is character inside tougijou? */

	POSITION wilderness_x;	/* Coordinates in the wilderness */
	POSITION wilderness_y;
	bool wild_mode;

	HIT_POINT mhp;			/* Max hit pts */
	HIT_POINT chp;			/* Cur hit pts */
	u32b chp_frac;		/* Cur hit frac (times 2^16) */

	MANA_POINT msp;			/* Max mana pts */
	MANA_POINT csp;			/* Cur mana pts */
	u32b csp_frac;		/* Cur mana frac (times 2^16) */

	s16b max_plv;		/* Max Player Level */

	BASE_STATUS stat_max[6];	/* Current "maximal" stat values */
	BASE_STATUS stat_max_max[6];	/* Maximal "maximal" stat values */
	BASE_STATUS stat_cur[6];	/* Current "natural" stat values */

	s16b learned_spells;
	s16b add_spells;

	u32b count;

	TIME_EFFECT fast;		/* Timed -- Fast */
	TIME_EFFECT slow;		/* Timed -- Slow */
	TIME_EFFECT blind;		/* Timed -- Blindness */
	TIME_EFFECT paralyzed;		/* Timed -- Paralysis */
	TIME_EFFECT confused;		/* Timed -- Confusion */
	TIME_EFFECT afraid;		/* Timed -- Fear */
	TIME_EFFECT image;		/* Timed -- Hallucination */
	TIME_EFFECT poisoned;		/* Timed -- Poisoned */
	TIME_EFFECT cut;		/* Timed -- Cut */
	TIME_EFFECT stun;		/* Timed -- Stun */

	TIME_EFFECT protevil;		/* Timed -- Protection */
	TIME_EFFECT invuln;		/* Timed -- Invulnerable */
	TIME_EFFECT ult_res;		/* Timed -- Ultimate Resistance */
	TIME_EFFECT hero;		/* Timed -- Heroism */
	TIME_EFFECT shero;		/* Timed -- Super Heroism */
	TIME_EFFECT shield;		/* Timed -- Shield Spell */
	TIME_EFFECT blessed;		/* Timed -- Blessed */
	TIME_EFFECT tim_invis;		/* Timed -- See Invisible */
	TIME_EFFECT tim_infra;		/* Timed -- Infra Vision */
	TIME_EFFECT tsuyoshi;		/* Timed -- Tsuyoshi Special */
	TIME_EFFECT ele_attack;	/* Timed -- Elemental Attack */
	TIME_EFFECT ele_immune;	/* Timed -- Elemental Immune */

	TIME_EFFECT oppose_acid;	/* Timed -- oppose acid */
	TIME_EFFECT oppose_elec;	/* Timed -- oppose lightning */
	TIME_EFFECT oppose_fire;	/* Timed -- oppose heat */
	TIME_EFFECT oppose_cold;	/* Timed -- oppose cold */
	TIME_EFFECT oppose_pois;	/* Timed -- oppose poison */

	TIME_EFFECT tim_esp;       /* Timed ESP */
	TIME_EFFECT wraith_form;   /* Timed wraithform */

	TIME_EFFECT resist_magic;  /* Timed Resist Magic (later) */
	TIME_EFFECT tim_regen;
	TIME_EFFECT kabenuke;
	TIME_EFFECT tim_stealth;
	TIME_EFFECT tim_levitation;
	TIME_EFFECT tim_sh_touki;
	TIME_EFFECT lightspeed;
	TIME_EFFECT tsubureru;
	TIME_EFFECT magicdef;
	TIME_EFFECT tim_res_nether;	/* Timed -- Nether resistance */
	TIME_EFFECT tim_res_time;	/* Timed -- Time resistance */
	IDX mimic_form;
	TIME_EFFECT tim_mimic;
	TIME_EFFECT tim_sh_fire;
	TIME_EFFECT tim_sh_holy;
	TIME_EFFECT tim_eyeeye;

	/* for mirror master */
	TIME_EFFECT tim_reflect;       /* Timed -- Reflect */
	TIME_EFFECT multishadow;       /* Timed -- Multi-shadow */
	TIME_EFFECT dustrobe;          /* Timed -- Robe of dust */

	s16b chaos_patron;
	u32b muta1;
	u32b muta2;
	u32b muta3;

	s16b virtues[8];
	s16b vir_types[8];

	s16b word_recall;	  /* Word of recall counter */
	s16b alter_reality;	  /* Alter reality counter */
	IDX recall_dungeon;      /* Dungeon set to be recalled */

	s16b energy_need;	  /* Energy needed for next move */
	s16b enchant_energy_need;	  /* Energy needed for next upkeep effect	 */

	s16b food;		  /* Current nutrition */

	u32b special_attack;	  /* Special attack capacity -LM- */
	u32b special_defense;	  /* Special block capacity -LM- */
	byte action;		  /* Currently action */

	u32b spell_learned1;	  /* bit mask of spells learned */
	u32b spell_learned2;	  /* bit mask of spells learned */
	u32b spell_worked1;	  /* bit mask of spells tried and worked */
	u32b spell_worked2;	  /* bit mask of spells tried and worked */
	u32b spell_forgotten1;	  /* bit mask of spells learned but forgotten */
	u32b spell_forgotten2;	  /* bit mask of spells learned but forgotten */
	byte spell_order[64];	  /* order spells learned/remembered/forgotten */

	SUB_EXP spell_exp[64];        /* Proficiency of spells */
	SUB_EXP weapon_exp[5][64];    /* Proficiency of weapons */
	SUB_EXP skill_exp[GINOU_MAX]; /* Proficiency of misc. skill */

	s32b magic_num1[108];     /* Array for non-spellbook type magic */
	byte magic_num2[108];     /* Flags for non-spellbook type magics */

	s16b mane_spell[MAX_MANE];
	s16b mane_dam[MAX_MANE];
	s16b mane_num;

	s16b concent;      /* Sniper's concentration level */

	s16b player_hp[PY_MAX_LEVEL];
	char died_from[80];   	  /* What killed the player */
	cptr last_message;        /* Last message on death or retirement */
	char history[4][60];  	  /* Textual "history" for the Player */

	u16b total_winner;	  /* Total winner */
	u16b panic_save;	  /* Panic save */

	u16b noscore;		  /* Cheating flags */

	bool wait_report_score;   /* Waiting to report score */
	bool is_dead;		  /* Player is dead */

	bool wizard;		  /* Player is in wizard mode */

	s16b riding;              /* Riding on a monster of this index */
	byte knowledge;           /* Knowledge about yourself */
	s32b visit;               /* Visited towns */

	byte start_race;          /* Race at birth */
	s32b old_race1;           /* Record of race changes */
	s32b old_race2;           /* Record of race changes */
	s16b old_realm;           /* Record of realm changes */

	s16b pet_follow_distance; /* Length of the imaginary "leash" for pets */
	s16b pet_extra_flags;     /* Various flags for controling pets */

	s16b today_mon;           /* Wanted monster */

	bool dtrap;               /* Whether you are on trap-safe grids */
	s16b floor_id;            /* Current floor location */ 

	bool autopick_autoregister; /* auto register is in-use or not */

	byte feeling;		/* Most recent dungeon feeling */
	s32b feeling_turn;	/* The turn of the last dungeon feeling */


	/*** Temporary fields ***/

	bool playing;			/* True if player is playing */
	bool leaving;			/* True if player is leaving */

	byte exit_bldg;			/* Goal obtained in arena? -KMW- */

	bool leaving_dungeon;	/* True if player is leaving the dungeon */
	bool teleport_town;
	bool enter_dungeon;     /* Just enter the dungeon */

	IDX health_who;	/* Health bar trackee */

	IDX monster_race_idx;	/* Monster race trackee */

	IDX object_kind_idx;	/* Object kind trackee */

	s16b new_spells;	/* Number of spells available */
	s16b old_spells;

	s16b old_food_aux;	/* Old value of food */

	bool old_cumber_armor;
	bool old_cumber_glove;
	bool old_heavy_wield[2];
	bool old_heavy_shoot;
	bool old_icky_wield[2];
	bool old_riding_wield[2];
	bool old_riding_ryoute;
	bool old_monlite;

	s16b old_lite;		/* Old radius of lite (if any) */

	bool cumber_armor;	/* Mana draining armor */
	bool cumber_glove;	/* Mana draining gloves */
	bool heavy_wield[2];	/* Heavy weapon */
	bool heavy_shoot;	/* Heavy shooter */
	bool icky_wield[2];	/* Icky weapon */
	bool riding_wield[2];	/* Riding weapon */
	bool riding_ryoute;	/* Riding weapon */
	bool monlite;

	s16b cur_lite;		/* Radius of lite (if any) */


	u32b notice;		/* Special Updates (bit flags) */
	u32b update;		/* Pending Updates (bit flags) */
	u32b redraw;		/* Normal Redraws (bit flags) */
	u32b window;		/* Window Redraws (bit flags) */

	s16b stat_use[6];	/* Current modified stats */
	s16b stat_top[6];	/* Maximal modified stats */

	bool sutemi;
	bool counter;

	s32b align;				/* Good/evil/neutral */
	POSITION run_py;
	POSITION run_px;


	/*** Extracted fields ***/

	u32b total_weight;	/* Total weight being carried */

	s16b stat_add[6];	/* Modifiers to stat values */
	s16b stat_ind[6];	/* Indexes into stat tables */

	bool immune_acid;	/* Immunity to acid */
	bool immune_elec;	/* Immunity to lightning */
	bool immune_fire;	/* Immunity to fire */
	bool immune_cold;	/* Immunity to cold */

	bool resist_acid;	/* Resist acid */
	bool resist_elec;	/* Resist lightning */
	bool resist_fire;	/* Resist fire */
	bool resist_cold;	/* Resist cold */
	bool resist_pois;	/* Resist poison */

	bool resist_conf;	/* Resist confusion */
	bool resist_sound;	/* Resist sound */
	bool resist_lite;	/* Resist light */
	bool resist_dark;	/* Resist darkness */
	bool resist_chaos;	/* Resist chaos */
	bool resist_disen;	/* Resist disenchant */
	bool resist_shard;	/* Resist shards */
	bool resist_nexus;	/* Resist nexus */
	bool resist_blind;	/* Resist blindness */
	bool resist_neth;	/* Resist nether */
	bool resist_fear;	/* Resist fear */
	bool resist_time;	/* Resist time */

	bool reflect;       /* Reflect 'bolt' attacks */
	bool sh_fire;       /* Fiery 'immolation' effect */
	bool sh_elec;       /* Electric 'immolation' effect */
	bool sh_cold;       /* Cold 'immolation' effect */

	bool anti_magic;    /* Anti-magic */
	bool anti_tele;     /* Prevent teleportation */

	bool sustain_str;	/* Keep strength */
	bool sustain_int;	/* Keep intelligence */
	bool sustain_wis;	/* Keep wisdom */
	bool sustain_dex;	/* Keep dexterity */
	bool sustain_con;	/* Keep constitution */
	bool sustain_chr;	/* Keep charisma */

	u32b cursed;            /* Player is cursed */

	bool can_swim;		/* No damage falling */
	bool levitation;		/* No damage falling */
	bool lite;		/* Permanent light */
	bool free_act;		/* Never paralyzed */
	bool see_inv;		/* Can see invisible */
	bool regenerate;	/* Regenerate hit pts */
	bool hold_exp;		/* Resist exp draining */

	bool telepathy;		/* Telepathy */
	bool esp_animal;
	bool esp_undead;
	bool esp_demon;
	bool esp_orc;
	bool esp_troll;
	bool esp_giant;
	bool esp_dragon;
	bool esp_human;
	bool esp_evil;
	bool esp_good;
	bool esp_nonliving;
	bool esp_unique;

	bool slow_digest;	/* Slower digestion */
	bool bless_blade;	/* Blessed blade */
	bool xtra_might;	/* Extra might bow */
	bool impact[2];		/* Earthquake blows */
	bool pass_wall;     /* Permanent wraithform */
	bool kill_wall;
	bool dec_mana;
	bool easy_spell;
	bool heavy_spell;
	bool warning;
	bool mighty_throw;
	bool see_nocto;		/* Noctovision */

	s16b to_dd[2]; /* Extra dice/sides */
	s16b to_ds[2];

	s16b dis_to_h[2];	/* Known bonus to hit (wield) */
	s16b dis_to_h_b;	/* Known bonus to hit (bow) */
	s16b dis_to_d[2];	/* Known bonus to dam (wield) */
	s16b dis_to_a;		/* Known bonus to ac */

	s16b dis_ac;		/* Known base ac */

	s16b to_h[2];			/* Bonus to hit (wield) */
	s16b to_h_b;			/* Bonus to hit (bow) */
	s16b to_h_m;			/* Bonus to hit (misc) */
	s16b to_d[2];			/* Bonus to dam (wield) */
	s16b to_d_m;			/* Bonus to dam (misc) */
	s16b to_a;			/* Bonus to ac */

	s16b to_m_chance;		/* Minusses to cast chance */

	bool ryoute;
	bool migite;
	bool hidarite;
	bool no_flowed;

	s16b ac;			/* Base ac */

	s16b see_infra;		/* Infravision range */

	s16b skill_dis;		/* Skill: Disarming */
	s16b skill_dev;		/* Skill: Magic Devices */
	s16b skill_sav;		/* Skill: Saving throw */
	s16b skill_stl;		/* Skill: Stealth factor */
	s16b skill_srh;		/* Skill: Searching ability */
	s16b skill_fos;		/* Skill: Searching frequency */
	s16b skill_thn;		/* Skill: To hit (normal) */
	s16b skill_thb;		/* Skill: To hit (shooting) */
	s16b skill_tht;		/* Skill: To hit (throwing) */
	s16b skill_dig;		/* Skill: Digging */

	s16b num_blow[2];	/* Number of blows */
	s16b num_fire;		/* Number of shots */

	byte tval_xtra;		/* Correct xtra tval */

	byte tval_ammo;		/* Correct ammo tval */

	byte pspeed;		/* Current speed */

	s16b energy_use;	/* Energy use this turn */

	POSITION y;	/* Player location in dungeon */
	POSITION x;	/* Player location in dungeon */
	char name[32]; /* Current player's character name */
};


/*
 * A structure to hold "rolled" information
 */
typedef struct birther birther;

struct birther
{
	byte psex;         /* Sex index */
	byte prace;        /* Race index */
	byte pclass;       /* Class index */
	byte pseikaku;     /* Seikaku index */
	byte realm1;       /* First magic realm */
	byte realm2;       /* Second magic realm */

	s16b age;
	s16b ht;
	s16b wt;
	s16b sc;

	s32b au;

	s16b stat_max[6];	/* Current "maximal" stat values */
	s16b stat_max_max[6];	/* Maximal "maximal" stat values */
	s16b player_hp[PY_MAX_LEVEL];

	s16b chaos_patron;

	s16b vir_types[8];

	char history[4][60];

	bool quick_ok;
};


/* For Monk martial arts */

typedef struct martial_arts martial_arts;

struct martial_arts
{
	cptr    desc;       /* A verbose attack description */
	int     min_level;  /* Minimum level to use */
	int     chance;     /* Chance of 'success' */
	int     dd;         /* Damage dice */
	int     ds;         /* Damage sides */
	int     effect;     /* Special effects */
};

typedef struct kamae kamae;

struct kamae
{
	cptr    desc;       /* A verbose kamae description */
	int     min_level;  /* Minimum level to use */
	cptr    info;
};

/* Mindcrafters */
typedef struct mind_type mind_type;
struct mind_type
{
	int     min_lev;
	int     mana_cost;
	int     fail;
	cptr    name;
};

typedef struct mind_power mind_power;
struct mind_power
{
	mind_type info[MAX_MIND_POWERS];
};

/* Imitator */

typedef struct monster_power monster_power;
struct monster_power
{
	int     level;
	int     smana;
	int     fail;
	int     manedam;
	int     manefail;
	int     use_stat;
	cptr    name;
};


/*
 * A structure to describe a building.
 * From Kamband
 */
typedef struct building_type building_type;

struct building_type
{
	char name[20];                  /* proprietor name */
	char owner_name[20];            /* proprietor name */
	char owner_race[20];            /* proprietor race */

	char act_names[8][30];          /* action names */
	s32b member_costs[8];           /* Costs for class members of building */
	s32b other_costs[8];		    /* Costs for nonguild members */
	char letters[8];                /* action letters */
	s16b actions[8];                /* action codes */
	s16b action_restr[8];           /* action restrictions */

	s16b member_class[MAX_CLASS];   /* which classes are part of guild */
	s16b member_race[MAX_RACES];    /* which classes are part of guild */
	s16b member_realm[MAX_MAGIC+1]; /* which realms are part of guild */
};


/* Border */
typedef struct border_type border_type;
struct border_type
{
	s16b north[MAX_WID];
	s16b south[MAX_WID];
	s16b east[MAX_HGT];
	s16b west[MAX_HGT];
	s16b north_west;
	s16b north_east;
	s16b south_west;
	s16b south_east;
};


/*
 * A structure describing a wilderness area
 * with a terrain or a town
 */
typedef struct wilderness_type wilderness_type;
struct wilderness_type
{
	int         terrain;
	int         town;
	int         road;
	u32b        seed;
	s16b        level;
	byte        entrance;
};


/*
 * A structure describing a town with
 * stores and buildings
 */
typedef struct town_type town_type;
struct town_type
{
	char        name[32];
	u32b        seed;      /* Seed for RNG */
	store_type	*store;    /* The stores [MAX_STORES] */
	byte        numstores;
};

/* Dungeons */
typedef struct dun_type dun_type;
struct dun_type
{
	byte min_level; /* Minimum level in the dungeon */
	byte max_level; /* Maximum dungeon level allowed */

	cptr name;      /* The name of the dungeon */
};

/*
 * Sort-array element
 */
typedef struct tag_type tag_type;

struct tag_type
{
	int     tag;
	int     index;
};

typedef bool (*monster_hook_type)(int r_idx);


/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(object_type *);


/*
 * Semi-Portable High Score List Entry (128 bytes) -- BEN
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */

typedef struct high_score high_score;

struct high_score
{
	char what[8];		/* Version info (string) */

	char pts[10];		/* Total Score (number) */

	char gold[10];		/* Total Gold (number) */

	char turns[10];		/* Turns Taken (number) */

	char day[10];		/* Time stamp (string) */

	char who[16];		/* Player Name (string) */

	char uid[8];		/* Player UID (number) */

	char sex[2];		/* Player Sex (string) */
	char p_r[3];		/* Player Race (number) */
	char p_c[3];		/* Player Class (number) */
	char p_a[3];		/* Player Seikaku (number) */

	char cur_lev[4];		/* Current Player Level (number) */
	char cur_dun[4];		/* Current Dungeon Level (number) */
	char max_lev[4];		/* Max Player Level (number) */
	char max_dun[4];		/* Max Dungeon Level (number) */

	char how[40];		/* Method of death (string) */
};


typedef struct
{
	s16b feat;    /* Feature tile */
	byte percent; /* Chance of type */
}
feat_prob;


/* A structure for the != dungeon types */
typedef struct dungeon_info_type dungeon_info_type;
struct dungeon_info_type {
	STR_OFFSET name;		/* Name */
	STR_OFFSET text;		/* Description */

	byte dy;
	byte dx;

	feat_prob floor[DUNGEON_FEAT_PROB_NUM]; /* Floor probability */
	feat_prob fill[DUNGEON_FEAT_PROB_NUM];  /* Cave wall probability */
	s16b outer_wall;                        /* Outer wall tile */
	s16b inner_wall;                        /* Inner wall tile */
	s16b stream1;                           /* stream tile */
	s16b stream2;                           /* stream tile */

	s16b mindepth;         /* Minimal depth */
	s16b maxdepth;         /* Maximal depth */
	byte min_plev;         /* Minimal plev needed to enter -- it's an anti-cheating mesure */
	s16b pit;
	s16b nest;
	byte mode;		/* Mode of combinaison of the monster flags */

	int min_m_alloc_level;	/* Minimal number of monsters per level */
	int max_m_alloc_chance;	/* There is a 1/max_m_alloc_chance chance per round of creating a new monster */

	u32b flags1;		/* Flags 1 */

	u32b mflags1;		/* The monster flags that are allowed */
	u32b mflags2;
	u32b mflags3;
	u32b mflags4;
	u32b mflags7;
	u32b mflags8;
	u32b mflags9;
	u32b mflagsr;

	u32b m_a_ability_flags1;
	u32b m_a_ability_flags2;
	u32b m_a_ability_flags3;
	u32b m_a_ability_flags4;

	char r_char[5];		/* Monster race allowed */
	int final_object;	/* The object you'll find at the bottom */
	int final_artifact;	/* The artifact you'll find at the bottom */
	int final_guardian;	/* The artifact's guardian. If an artifact is specified, then it's NEEDED */

	byte special_div;	/* % of monsters affected by the flags/races allowed, to add some variety */
	int tunnel_percent;
	int obj_great;
	int obj_good;
};


/*!
 * @struct autopick_type
 * @brief 自動拾い/破壊設定データの構造体 / A structure type for entry of auto-picker/destroyer
 */
typedef struct {
	cptr name;          /*!< 自動拾い/破壊定義の名称一致基準 / Items which have 'name' as part of its name match */
	cptr insc;          /*!< 対象となったアイテムに自動で刻む内容 / Items will be auto-inscribed as 'insc' */
	u32b flag[2];       /*!< キーワードに関する汎用的な条件フラグ / Misc. keyword to be matched */
	byte action;        /*!< 対象のアイテムを拾う/破壊/放置するかの指定フラグ / Auto-pickup or Destroy or Leave items */
	byte dice;          /*!< 武器のダイス値基準値 / Weapons which have more than 'dice' dice match */
	byte bonus;         /*!< アイテムのボーナス基準値 / Items which have more than 'bonus' magical bonus match */
} autopick_type;


/*
 *  A structure type for the saved floor
 */
typedef struct 
{
	s16b floor_id;        /* No recycle until 65536 IDs are all used */
	byte savefile_id;     /* ID for savefile (from 0 to MAX_SAVED_FLOOR) */
	DEPTH dun_level;
	s32b last_visit;      /* Time count of last visit. 0 for new floor. */
	u32b visit_mark;      /* Older has always smaller mark. */
	s16b upper_floor_id;  /* a floor connected with level teleportation */
	s16b lower_floor_id;  /* a floor connected with level tel. and trap door */
} saved_floor_type;


/*
 *  A structure type for terrain template of saving dungeon floor
 */
typedef struct
{
	u16b info;
	s16b feat;
	s16b mimic;
	s16b special;
	u16b occurrence;
} cave_template_type;


/*!
 * @struct arena_type
 * @brief 闘技場のモンスターエントリー構造体 / A structure type for arena entry
 */
typedef struct
{
	s16b r_idx; /*!< 闘技場のモンスター種族ID(0ならば表彰式) / Monster (0 means victory prizing) */
	byte tval;  /*!< モンスター打倒後に得られるアイテムの大カテゴリID / tval of prize (0 means no prize) */
	byte sval;  /*!< モンスター打倒後に得られるアイテムの小カテゴリID / sval of prize */
} arena_type;


/*
 * A structure type for doors
 */
typedef struct
{
	s16b open;
	s16b broken;
	s16b closed;
	s16b locked[MAX_LJ_DOORS];
	s16b num_locked;
	s16b jammed[MAX_LJ_DOORS];
	s16b num_jammed;
} door_type;


#ifdef TRAVEL
/*
 *  A structure type for travel command
 */
typedef struct {
	int run; /* Remaining grid number */
	int cost[MAX_HGT][MAX_WID];
	int x; /* Target X */
	int y; /* Target Y */
	int dir; /* Running direction */
} travel_type;
#endif

typedef struct {
	cptr flag;
	byte index;
	byte level;
	s32b value;
	struct {
		int constant;
		int dice;
	} timeout;
	cptr desc;
} activation_type;

typedef struct {
	int flag;
	int type;
	cptr name;
} dragonbreath_type;
