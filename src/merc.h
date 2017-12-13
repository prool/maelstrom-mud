/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
*  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
*                                                                         *
*  In order to use any part of this Envy Diku Mud, you must comply with   *
*  the original Diku license in 'license.doc', the Merc license in        *
*  'license.txt', as well as the Envy license in 'license.nvy'.           *
*  In particular, you may not remove either of these copyright notices.   *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

// the current version of maelstrom
#define VERSION "2.0.0"

#include <jansson.h>
#include "devops.h" /* Include devops attributes */
#include "colors.h" /* Include the ansi color routines */

#define DECLARE_DO_FUN( fun )    DO_FUN fun
#define DECLARE_SPEC_FUN( fun )  SPEC_FUN fun
#define DECLARE_SPELL_FUN( fun ) SPELL_FUN fun

/*
 * Boolean Definitions
 */
#if !defined( FALSE )
#define FALSE 0
#endif

#if !defined( TRUE )
#define TRUE 1
#endif

typedef unsigned char bool;

/*
 * Structure types.
 */
typedef struct  affect_data       AFFECT_DATA;
typedef struct  area_data         AREA_DATA;
typedef struct  arena_data        ARENA_DATA;
typedef struct  new_clan_data     CLAN_DATA;
typedef struct  ban_data          BAN_DATA;
typedef struct  gskill_data       GSPELL_DATA;
typedef struct  char_data         CHAR_DATA;
typedef struct  social_data       SOCIAL_DATA;
typedef struct  descriptor_data   DESCRIPTOR_DATA;
typedef struct  exit_data         EXIT_DATA;
typedef struct  extra_descr_data  EXTRA_DESCR_DATA;
typedef struct  help_data         HELP_DATA;
typedef struct  kill_data         KILL_DATA;
typedef struct  mob_index_data    MOB_INDEX_DATA;
typedef struct  note_data         NOTE_DATA;
typedef struct  obj_data          OBJ_DATA;
typedef struct  obj_index_data    OBJ_INDEX_DATA;
typedef struct  pc_data           PC_DATA;
typedef struct  reset_data        RESET_DATA;
typedef struct  room_affect_data  ROOM_AFFECT_DATA;
typedef struct  powered_data      POWERED_DATA;
typedef struct  room_index_data   ROOM_INDEX_DATA;
typedef struct  shop_data         SHOP_DATA;
typedef struct  time_info_data    TIME_INFO_DATA;
typedef struct  weather_data      WEATHER_DATA;
typedef struct  mob_prog_data     MPROG_DATA;
typedef struct  mob_prog_act_list MPROG_ACT_LIST;
typedef struct  alias_data        ALIAS_DATA;
typedef struct  trap_data         TRAP_DATA;
typedef struct  playerlist_data   PLAYERLIST_DATA;
typedef struct  skill_type        SKILL_TYPE;
typedef struct  newbie_data       NEWBIE_DATA;
typedef struct  war_data          WAR_DATA;
typedef struct  money_data        MONEY_DATA;

#define S_PER_G 10
#define C_PER_S 10
#define C_PER_G ( S_PER_G * C_PER_S )

#define SILVER_PER_GOLD   10
#define COPPER_PER_SILVER 10
#define COPPER_PER_GOLD   ( SILVER_PER_GOLD * COPPER_PER_SILVER )

/*
 * Function types.
 */
typedef void DO_FUN    ( CHAR_DATA * ch, char * argument );
typedef bool SPEC_FUN  ( CHAR_DATA * ch );
typedef void SPELL_FUN ( int sn, int level, CHAR_DATA * ch, void * vo );

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH      8192 // 1024
#define MAX_STRING_LENGTH 8192
#define MAX_INPUT_LENGTH  256

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_SKILL         386
#define MAX_GSPELL        2
#define MAX_CLASS         3
#define MAX_RACE          5
#define MAX_CLAN          21   // max 20 clans + 1 for clan 0
#define MAX_LEVEL         113
#define MAX_SIZE          9

#define STUN_MAX 5

#define L_IMP      MAX_LEVEL
#define L_CON      ( L_IMP - 1 )
#define L_DIR      ( L_CON - 1 )
#define L_SEN      ( L_DIR - 1 )
#define L_GOD      ( L_SEN - 1 )
#define L_DEM      ( L_GOD - 1 )
#define L_JUN      ( L_DEM - 1 )
#define L_APP      ( L_JUN - 1 )
#define L_CHAMP5   ( L_APP - 1 )
#define L_CHAMP4   ( L_CHAMP5 - 1 )
#define L_CHAMP3   ( L_CHAMP4 - 1 )
#define L_CHAMP2   ( L_CHAMP3 - 1 )
#define L_CHAMP1   ( L_CHAMP2 - 1 )
#define LEVEL_HERO 100

#define LEVEL_IMMORTAL 106
#define LEVEL_MORTAL   105

#define PULSE_PER_SECOND 6
#define PULSE_VIOLENCE   (  3 * PULSE_PER_SECOND )
#define PULSE_MOBILE     (  4 * PULSE_PER_SECOND )
#define PULSE_TICK       ( 40 * PULSE_PER_SECOND )
#define PULSE_AREA       ( 80 * PULSE_PER_SECOND )

/* Save the database - OLC 1.1b */
#define PULSE_DB_DUMP ( 1800 * PULSE_PER_SECOND )     /* 30 minutes  */

struct money_data {
  int gold;
  int silver;
  int copper;
};

struct arena_data {
  AREA_DATA * area;  // arena area
  CHAR_DATA * cch;   // challenger char
  CHAR_DATA * och;   // optional challengee char
  CHAR_DATA * fch;   // first char in arena
  CHAR_DATA * sch;   // second char in arena
  int         award; // money in the pot
  int         count; // update ticker
};

struct war_data {
  AREA_DATA * area;    // battlefield
  CHAR_DATA * fch;
  CHAR_DATA * sch;
  bool        iswar;
  int         wartype;
  int         max_level;
  int         min_level;
  int         inwar;
  int         team_red;
  int         team_blue;
  int         clan_chlng;
  int         clan_accpt;
  int         timeleft;
  int         count;
  int         ticker;
};

struct wiznet_type {
  char * name;
  long   flag;
  int    level;
};

/*
 * Site ban structure.
 */
struct  ban_data {
  BAN_DATA * next;
  char       type;
  char     * name;
  char     * user;
};

bool MOBtrigger;

#define ERROR_PROG     -1
#define IN_FILE_PROG   0
#define ACT_PROG       1
#define SPEECH_PROG    2
#define RAND_PROG      4
#define FIGHT_PROG     8
#define DEATH_PROG     16
#define HITPRCNT_PROG  32
#define ENTRY_PROG     64
#define GREET_PROG     128
#define ALL_GREET_PROG 256
#define GIVE_PROG      512
#define BRIBE_PROG     1024

/*
 * Time and weather stuff.
 */
#define SUN_EVENING   0
#define SUN_MIDNIGHT  1
#define SUN_DARK      2
#define SUN_DAWN      3
#define SUN_MORNING   4
#define SUN_NOON      5
#define SUN_AFTERNOON 6
#define SUN_DUSK      7

#define SKY_CLOUDLESS 0
#define SKY_CLOUDY    1
#define SKY_RAINING   2
#define SKY_LIGHTNING 3

struct  time_info_data {
  int hour;
  int day;
  int month;
  int year;
};

struct  weather_data {
  int mmhg;
  int change;
  int sky;
  int sunlight;
};

/*
 * WIZnet flags
 */
#define WIZ_ON        1
#define WIZ_TICKS     2
#define WIZ_LOGINS    4
#define WIZ_SITES     8
#define WIZ_LINKS     16
#define WIZ_DEATHS    32
#define WIZ_RESETS    64
#define WIZ_MOBDEATHS 128
#define WIZ_FLAGS     256
#define WIZ_PENALTIES 512
#define WIZ_SACCING   1024
#define WIZ_LEVELS    2048
#define WIZ_SECURE    4096
#define WIZ_SWITCHES  8192
#define WIZ_SNOOPS    16384
#define WIZ_RESTORE   32768
#define WIZ_LOAD      65536
#define WIZ_NEWBIE    131072
#define WIZ_PREFIX    262144
#define WIZ_SPAM      524288
#define WIZ_GENERAL   1048576
#define WIZ_OLDLOG    2097152

/*
 * Connected state for a channel.
 */
#define CON_HOTREBOOT_RECOVER    -15
#define CON_PLAYING              0
#define CON_GET_NAME             1
#define CON_GET_OLD_PASSWORD     2
#define CON_CONFIRM_NEW_NAME     3
#define CON_GET_NEW_PASSWORD     4
#define CON_CONFIRM_NEW_PASSWORD 5
#define CON_GET_NEW_SEX          6
#define CON_GET_NEW_CLASS        7
#define CON_READ_MOTD            8
#define CON_GET_NEW_RACE         9
#define CON_CONFIRM_RACE         10
#define CON_CONFIRM_CLASS        11
#define CON_CHECK_AUTHORIZE      12
#define CON_GET_2ND_CLASS        13
#define CON_CONFIRM_2ND_CLASS    14
#define CON_WANT_MULTI           15
#define CON_GET_3RD_CLASS        16
#define CON_CONFIRM_3RD_CLASS    17
#define CON_WANT_MULTI_2         18
#define CON_BEGIN_REMORT         19
#define CON_GET_ANSI             105
#define CON_AUTHORIZE_NAME       100
#define CON_AUTHORIZE_NAME1      101
#define CON_AUTHORIZE_NAME2      102
#define CON_AUTHORIZE_NAME3      103
#define CON_AUTHORIZE_LOGOUT     104

/*
 * Descriptor (channel) structure.
 */
struct  descriptor_data {
  DESCRIPTOR_DATA * next;
  DESCRIPTOR_DATA * snoop_by;
  CHAR_DATA       * character;
  CHAR_DATA       * original;
  char            * host;
  char            * user;
  int               descriptor;
  int               connected;
  bool              fcommand;
  char              inbuf[ MAX_INPUT_LENGTH * 4 ];
  char              incomm[ MAX_INPUT_LENGTH   ];
  char              inlast[ MAX_INPUT_LENGTH   ];
  int               repeat;
  char            * showstr_head;
  char            * showstr_point;
  char            * outbuf;
  int               outsize;
  int               outtop;
  void            * pEdit;
  void            * inEdit;
  char           ** pString;
  int               editor;
  int               editin;
  bool              ansi;
};

/*
 * Attribute bonus structures.
 */
struct  str_app_type {
  int tohit;
  int todam;
  int carry;
  int wield;
};

struct  int_app_type {
  int learn;
};

struct  wis_app_type {
  int practice;
};

struct  dex_app_type {
  int defensive;
};

struct  con_app_type {
  int hitp;
  int shock;
};

/*
 * TO types for act.
 */
#define TO_ROOM    0
#define TO_NOTVICT 1
#define TO_VICT    2
#define TO_CHAR    3
#define TO_COMBAT  4

/*
 * Help table types.
 */
struct  help_data {
  HELP_DATA * next;
  /*AREA_DATA *   area;*/   /* OLC 1.1b */
  int    level;
  char * keyword;
  char * text;
};

/*
 * Structure for social in the socials list.
 */
struct  social_data {
  SOCIAL_DATA * next;
  char        * name;
  char        * char_no_arg;
  char        * others_no_arg;
  char        * char_found;
  char        * others_found;
  char        * vict_found;
  char        * char_auto;
  char        * others_auto;
};

struct  newbie_data {
  NEWBIE_DATA * next;
  char        * keyword;
  char        * answer1;
  char        * answer2;
};

/*
 * Shop types.
 */
#define MAX_TRADE 5

struct  shop_data {
  SHOP_DATA * next;                  // Next shop in list
  int         keeper;                // Vnum of shop keeper mob
  int         buy_type[ MAX_TRADE ]; // Item types shop will buy
  int         profit_buy;            // Cost multiplier for buying
  int         profit_sell;           // Cost multiplier for selling
  int         open_hour;             // First opening hour
  int         close_hour;            // First closing hour
};

/*
 *   Player list structer.
 */
struct playerlist_data {
  PLAYERLIST_DATA * next;
  char            * name;
  unsigned char     level;
  char            * clan_name;
  unsigned char     clan_rank;
};

/**
 * Classes
 */
#define CLASS_CASTER  0
#define CLASS_ROGUE   1
#define CLASS_FIGHTER 2

/**
 * Races
 */
#define RACE_HUMAN    0
#define RACE_ELF      1
#define RACE_DWARF    2
#define RACE_GNOME    3
#define RACE_HALFLING 4

/**
 * Sizes
 */
#define SIZE_FINE       0
#define SIZE_DIMUNITIVE 1
#define SIZE_TINY       2
#define SIZE_SMALL      3
#define SIZE_MEDIUM     4
#define SIZE_LARGE      5
#define SIZE_HUGE       6
#define SIZE_GARGANTUAN 7
#define SIZE_COLOSSAL   8

/*
 * Class Structure
 */
struct  class_type {
  char   who_name[ 4 ];      // Three-letter name for 'who'
  char   who_long[ 15 ];     // Long name of Class
  int    attr_prime;         // Prime attribute
  int    skill_adept;        // Maximum skill level
  double mbab;               // Base Attack Bonus Modifier (thac0 = 20 - (level * mbab))
  int    hitdice;            // Hit Dice
  int    d6gold;             // Number of starting d6 to roll for starting gold
  bool   spellcaster;        // Class is a spellcaster
  char   whotype[ 15 ];      // Class whotype
};

/*
 * Race Structure
 */
struct  race_type {
  char race_name[ 4 ];      // Three-letter name for 'who'
  char race_full[ 20 ];     // Long name of Race
  int  mstr;                // Strength modifier
  int  mint;                // Intelligence modifier
  int  mwis;                // Wisdom modifier
  int  mdex;                // Dexterity modifier
  int  mcon;                // Constitution modifier
  int  mcha;                // Charisma modifier
  int  age;                 // Age at adulthood
  int  size;                // Size
};

/**
 * Size Structure
 */
struct size_type {
  char   name[ 20 ];           // Size name
  int    mac;                  // AC modifier
  int    mstealth;             // Stealth modifier
  double mcarry;               // Carrying capacity modifier
};

/**
 * Direction Structure
 */
struct direction_type {
  char * name;
  char * noun;
  char * navigation;
  char * descriptive;
  char * abbreviation;
  char * blood;
  int    reverse;
};

/*
 * Data structure for notes.
 */
struct  note_data {
  NOTE_DATA * next;
  char      * sender;
  char      * date;
  char      * to_list;
  char      * subject;
  char      * text;
  time_t      date_stamp;
  bool protected;
  int on_board;
};

/*
 * An affect.
 */
struct  affect_data {
  AFFECT_DATA * next;
  int           type;
  int           level;
  int           duration;
  int           location;
  int           modifier;
  int           bitvector;
  bool          deleted;
};

/*
 * A kill structure (indexed by level).
 */
struct  kill_data {
  int number;
  int killed;
};

/* Bitvector values.. replaces the old A,B,C system. */
#define BV00 0x00000001
#define BV01 0x00000002
#define BV02 0x00000004
#define BV03 0x00000008
#define BV04 0x00000010
#define BV05 0x00000020
#define BV06 0x00000040
#define BV07 0x00000080
#define BV08 0x00000100
#define BV09 0x00000200
#define BV10 0x00000400
#define BV11 0x00000800
#define BV12 0x00001000
#define BV13 0x00002000
#define BV14 0x00004000
#define BV15 0x00008000
#define BV16 0x00010000
#define BV17 0x00020000
#define BV18 0x00040000
#define BV19 0x00080000
#define BV20 0x00100000
#define BV21 0x00200000
#define BV22 0x00400000
#define BV23 0x00800000
#define BV24 0x01000000
#define BV25 0x02000000
#define BV26 0x04000000
#define BV27 0x08000000
#define BV28 0x10000000
#define BV29 0x20000000
#define BV30 0x40000000
#define BV31 0x80000000

/* RT ASCII conversions -- used so we can have letters in this file */
#define A  1
#define B  2
#define C  4
#define D  8
#define E  16
#define F  32
#define G  64
#define H  128
#define I  256
#define J  512
#define K  1024
#define L  2048
#define M  4096
#define N  8192
#define O  16384
#define P  32768
#define Q  65536
#define R  131072
#define S  262144
#define T  524288
#define U  1048576
#define V  2097152
#define W  4194304
#define X  8388608
#define Y  16777216
#define Z  33554432
#define aa 67108864     // doubled due to conflicts
#define bb 134217728
#define cc 268435456
#define dd 536870912
#define ee 1073741824

/***************************************************************************
*                                                                         *
*                   VALUES OF INTEREST TO AREA BUILDERS                   *
*                   (Start of section ... start here)                     *
*                                                                         *
***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD       127
#define MOB_VNUM_DEMON1          4
#define MOB_VNUM_DEMON2          4
#define MOB_VNUM_SUPERMOB        7
#define MOB_VNUM_ULT             3160
#define MOB_VNUM_AIR_ELEMENTAL   8914
#define MOB_VNUM_EARTH_ELEMENTAL 8915
#define MOB_VNUM_WATER_ELEMENTAL 8916
#define MOB_VNUM_FIRE_ELEMENTAL  8917
#define MOB_VNUM_DUST_ELEMENTAL  8918
#define MOB_VNUM_DEMON           80
#define MOB_VNUM_INSECTS         81
#define MOB_VNUM_WOLFS           82
#define MOB_VNUM_ANGEL           83
#define MOB_VNUM_SHADOW          84
#define MOB_VNUM_BEAST           85
#define MOB_VNUM_TRENT           86

/* CLANS */
#define CLAN_PKILL         BV00
#define CLAN_CIVIL_PKILL   BV01
#define CLAN_CHAMP_INDUCT  BV02
#define CLAN_LEADER_INDUCT BV03
#define CLAN_FIRST_INDUCT  BV04
#define CLAN_SECOND_INDUCT BV05

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC      BV00 // Auto set for mobs
#define ACT_SENTINEL    BV01 // Stays in one room
#define ACT_SCAVENGER   BV02 // Picks up objects
#define ACT_AGGRESSIVE  BV05 // Attacks PC's
#define ACT_STAY_AREA   BV06 // Won't leave area
#define ACT_WIMPY       BV07 // Flees when hurt
#define ACT_PET         BV08 // Auto set for pets
#define ACT_TRAIN       BV09 // Can train PC's
#define ACT_PRACTICE    BV10 // Can practice PC's
#define ACT_GAMBLE      BV11 // Runs a gambling game
#define ACT_PROTOTYPE   BV12 // Prototype flag
#define ACT_UNDEAD      BV13 // Can be turned
#define ACT_TRACK       BV14 // Track players
#define ACT_QUESTMASTER BV15 // Autoquest giver
#define ACT_POSTMAN     BV16 // Hello Mr. Postman!
#define ACT_NODRAG      BV17 // No drag mob
#define ACT_NOPUSH      BV18 // No push mob
#define ACT_NOSHADOW    BV19 // No shadow mob
#define ACT_NOASTRAL    BV20 // No astral mob
#define ACT_NEWBIE      BV21 // Newbie Helper

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND         BV00
#define AFF_INVISIBLE     BV01
#define AFF_INFRARED      BV09
#define AFF_FLAMING       BV11
#define AFF_POISON        BV12
#define AFF_PROTECT       BV13
#define AFF_SNEAK         BV15
#define AFF_HIDE          BV16
#define AFF_SLEEP         BV17
#define AFF_CHARM         BV18
#define AFF_FLYING        BV19
#define AFF_PASS_DOOR     BV20
#define AFF_STUN          BV21
#define AFF_SUMMONED      BV22
#define AFF_MUTE          BV23
#define AFF_PEACE         BV24
#define AFF_SCRY          BV29
#define AFF_ANTI_FLEE     BV30
#define AFF_DISJUNCTION   BV31

#define AFF_POLYMORPH       1
#define CODER               2
#define AFF_NOASTRAL        4
#define AFF_TRUESIGHT       16
#define AFF_PROTECTION_GOOD 128
#define AFF_CONFUSED        1024
#define AFF_FUMBLE          2048
#define AFF_HALLUCINATING   8192
#define AFF_PHASED          16384
#define AFF_RAGE            65536
#define AFF_INERTIAL        262144
#define AFF_PLOADED         1073741824

/* damage classes */
#define DAM_NONE      0
#define DAM_BASH      1
#define DAM_PIERCE    2
#define DAM_SLASH     3
#define DAM_FIRE      4
#define DAM_COLD      5
#define DAM_LIGHTNING 6
#define DAM_ACID      7
#define DAM_POISON    8
#define DAM_NEGATIVE  9
#define DAM_HOLY      10
#define DAM_ENERGY    11
#define DAM_MENTAL    12
#define DAM_DISEASE   13
#define DAM_DROWNING  14
#define DAM_LIGHT     15
#define DAM_OTHER     16
#define DAM_HARM      17
#define DAM_CHARM     18
#define DAM_SOUND     19

/* return values for check_imm */
#define IS_NORMAL     0
#define IS_IMMUNE     1
#define IS_RESISTANT  2
#define IS_VULNERABLE 3

/* IMM bits for mobs */
#define IMM_SUMMON    ( A )
#define IMM_CHARM     ( B )
#define IMM_MAGIC     ( C )
#define IMM_WEAPON    ( D )
#define IMM_BASH      ( E )
#define IMM_PIERCE    ( F )
#define IMM_SLASH     ( G )
#define IMM_FIRE      ( H )
#define IMM_COLD      ( I )
#define IMM_LIGHTNING ( J )
#define IMM_ACID      ( K )
#define IMM_POISON    ( L )
#define IMM_NEGATIVE  ( M )
#define IMM_HOLY      ( N )
#define IMM_ENERGY    ( O )
#define IMM_MENTAL    ( P )
#define IMM_DISEASE   ( Q )
#define IMM_DROWNING  ( R )
#define IMM_LIGHT     ( S )
#define IMM_SOUND     ( T )
#define IMM_WOOD      ( X )
#define IMM_SILVER    ( Y )
#define IMM_IRON      ( Z )

/* RES bits for mobs */
#define RES_SUMMON    ( A )
#define RES_CHARM     ( B )
#define RES_MAGIC     ( C )
#define RES_WEAPON    ( D )
#define RES_BASH      ( E )
#define RES_PIERCE    ( F )
#define RES_SLASH     ( G )
#define RES_FIRE      ( H )
#define RES_COLD      ( I )
#define RES_LIGHTNING ( J )
#define RES_ACID      ( K )
#define RES_POISON    ( L )
#define RES_NEGATIVE  ( M )
#define RES_HOLY      ( N )
#define RES_ENERGY    ( O )
#define RES_MENTAL    ( P )
#define RES_DISEASE   ( Q )
#define RES_DROWNING  ( R )
#define RES_LIGHT     ( S )
#define RES_SOUND     ( T )
#define RES_WOOD      ( X )
#define RES_SILVER    ( Y )
#define RES_IRON      ( Z )

/* VULN bits for mobs */
#define VULN_SUMMON    ( A )
#define VULN_CHARM     ( B )
#define VULN_MAGIC     ( C )
#define VULN_WEAPON    ( D )
#define VULN_BASH      ( E )
#define VULN_PIERCE    ( F )
#define VULN_SLASH     ( G )
#define VULN_FIRE      ( H )
#define VULN_COLD      ( I )
#define VULN_LIGHTNING ( J )
#define VULN_ACID      ( K )
#define VULN_POISON    ( L )
#define VULN_NEGATIVE  ( M )
#define VULN_HOLY      ( N )
#define VULN_ENERGY    ( O )
#define VULN_MENTAL    ( P )
#define VULN_DISEASE   ( Q )
#define VULN_DROWNING  ( R )
#define VULN_LIGHT     ( S )
#define VULN_SOUND     ( T )
#define VULN_WOOD      ( X )
#define VULN_SILVER    ( Y )
#define VULN_IRON      ( Z )

// weapon class
#define WEAPON_EXOTIC  0
#define WEAPON_SWORD   1
#define WEAPON_DAGGER  2
#define WEAPON_SPEAR   3
#define WEAPON_MACE    4
#define WEAPON_AXE     5
#define WEAPON_FLAIL   6
#define WEAPON_WHIP    7
#define WEAPON_POLEARM 8

// weapon damage types
#define WEAPON_TYPE_HIT     0
#define WEAPON_TYPE_SLICE   1
#define WEAPON_TYPE_STAB    2
#define WEAPON_TYPE_SLASH   3
#define WEAPON_TYPE_WHIP    4
#define WEAPON_TYPE_CLAW    5
#define WEAPON_TYPE_BLAST   6
#define WEAPON_TYPE_POUND   7
#define WEAPON_TYPE_CRUSH   8
#define WEAPON_TYPE_GREP    9
#define WEAPON_TYPE_BITE    10
#define WEAPON_TYPE_PIERCE  11
#define WEAPON_TYPE_SUCTION 12
#define WEAPON_TYPE_CHOP    13

// gate flags
#define GATE_NORMAL_EXIT ( A )
#define GATE_GOWITH      ( C )
#define GATE_BUGGY       ( D )
#define GATE_RANDOM      ( E )

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL 0
#define SEX_MALE    1
#define SEX_FEMALE  2

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE     2
#define OBJ_VNUM_MONEY_SOME    3
#define OBJ_VNUM_BLOOD         9
#define OBJ_VNUM_CORPSE_NPC    10
#define OBJ_VNUM_CORPSE_PC     11
#define OBJ_VNUM_SEVERED_HEAD  12
#define OBJ_VNUM_TORN_HEART    13
#define OBJ_VNUM_SLICED_ARM    14
#define OBJ_VNUM_SLICED_LEG    15
#define OBJ_VNUM_FINAL_TURD    16
#define OBJ_VNUM_PORTAL        17
#define OBJ_VNUM_DOLL          18
#define OBJ_VNUM_BERRY         19
#define OBJ_VNUM_MUSHROOM      20
#define OBJ_VNUM_LIGHT_BALL    21
#define OBJ_VNUM_PARCHMENT     25050
#define OBJ_VNUM_QUILL         25051
#define OBJ_VNUM_FLASK         25052
#define OBJ_VNUM_CAULDRON      25053
#define OBJ_VNUM_MFIRE         25054
#define OBJ_VNUM_MINK          25055
#define OBJ_VNUM_RWPARCHMENT   1339
#define OBJ_VNUM_RWQUILL       1341
#define OBJ_VNUM_RWFLASK       1343
#define OBJ_VNUM_RWCAULDRON    1344
#define OBJ_VNUM_RWFIRE        1345
#define OBJ_VNUM_RWINK         1346
#define OBJ_VNUM_PAPER         25062
#define OBJ_VNUM_LETTER        25063
#define OBJ_VNUM_SMOKEBOMB     25064
#define OBJ_VNUM_TO_FORGE_A    25065 // armor
#define OBJ_VNUM_TO_FORGE_W    25066 // weapon
#define OBJ_VNUM_SMITHY_HAMMER 713
#define OBJ_VNUM_SCHOOL_MACE   3001
#define OBJ_VNUM_SCHOOL_DAGGER 138
#define OBJ_VNUM_SCHOOL_SWORD  3003
#define OBJ_VNUM_SCHOOL_VEST   136
#define OBJ_VNUM_SCHOOL_SHIELD 3005
#define OBJ_VNUM_SCHOOL_BANNER 144
#define OBJ_VNUM_SCHOOL_CLUB   3007
#define OBJ_VNUM_BLACK_POWDER  8903
#define OBJ_VNUM_FLAMEBLADE    8920

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION     10
#define ITEM_NOTEBOARD  11
#define ITEM_FURNITURE  12
#define ITEM_TRASH      13
#define ITEM_CONTAINER  15
#define ITEM_DRINK_CON  17
#define ITEM_KEY        18
#define ITEM_FOOD       19
#define ITEM_MONEY      20
#define ITEM_BOAT       22
#define ITEM_CORPSE_NPC 23
#define ITEM_CORPSE_PC  24
#define ITEM_FOUNTAIN   25
#define ITEM_PILL       26
#define ITEM_LENSE      27
#define ITEM_LIQUID     28
#define ITEM_PORTAL     29
#define ITEM_VODOO      30
#define ITEM_BERRY      31
//#define ITEM_POTION_BAG        32

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW         1
#define ITEM_LOCK         8
#define ITEM_INVIS        32
#define ITEM_MAGIC        64
#define ITEM_NODROP       128
#define ITEM_ANTI_GOOD    512
#define ITEM_ANTI_EVIL    1024
#define ITEM_ANTI_NEUTRAL 2048
#define ITEM_NOREMOVE     4096
#define ITEM_INVENTORY    8192
#define ITEM_POISONED     16384
#define ITEM_NO_LOCATE    ( cc )
#define ITEM_NO_DAMAGE    ( dd )
#define ITEM_PATCHED      ( ee )

#define ITEM_ANTI_CASTER  1
#define ITEM_ANTI_ROGUE   4
#define ITEM_ANTI_FIGHTER 8

#define ITEM_ANTI_HUMAN    1
#define ITEM_ANTI_ELF      2
#define ITEM_ANTI_DWARF    4
#define ITEM_ANTI_GNOME    8
#define ITEM_ANTI_HALFLING 16

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE         1
#define ITEM_WEAR_FINGER  2
#define ITEM_WEAR_NECK    4
#define ITEM_WEAR_BODY    8
#define ITEM_WEAR_HEAD    16
#define ITEM_WEAR_LEGS    32
#define ITEM_WEAR_FEET    64
#define ITEM_WEAR_HANDS   128
#define ITEM_WEAR_ARMS    256
#define ITEM_WEAR_SHIELD  512
#define ITEM_WEAR_ABOUT   1024
#define ITEM_WEAR_WAIST   2048
#define ITEM_WEAR_WRIST   4096
#define ITEM_WIELD        8192
#define ITEM_HOLD         16384
#define ITEM_WEAR_ORBIT   32768
#define ITEM_WEAR_FACE    65536
#define ITEM_WEAR_CONTACT 131072
#define ITEM_PROTOTYPE    262144
#define ITEM_WEAR_EARS    524288
#define ITEM_WEAR_ANKLE   1048576
#define ITEM_HOOD_ON      2097152

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE          0
#define APPLY_STR           1
#define APPLY_DEX           2
#define APPLY_INT           3
#define APPLY_WIS           4
#define APPLY_CON           5
#define APPLY_SEX           6
#define APPLY_CLASS         7
#define APPLY_LEVEL         8
#define APPLY_AGE           9
#define APPLY_HEIGHT        10
#define APPLY_WEIGHT        11
#define APPLY_MANA          12
#define APPLY_HIT           13
#define APPLY_MOVE          14
#define APPLY_GOLD          15
#define APPLY_EXP           16
#define APPLY_AC            17
#define APPLY_HITROLL       18
#define APPLY_DAMROLL       19
#define APPLY_ANTI_DIS      26
#define APPLY_CHA           27

#define PERM_SPELL_BEGIN 100

#define APPLY_INVISIBLE        ( PERM_SPELL_BEGIN + 0 )
#define APPLY_INFRARED         ( PERM_SPELL_BEGIN + 6 )
#define APPLY_PROTECT          ( PERM_SPELL_BEGIN + 7 )
#define APPLY_SNEAK            ( PERM_SPELL_BEGIN + 8 )
#define APPLY_HIDE             ( PERM_SPELL_BEGIN + 9 )
#define APPLY_FLYING           ( PERM_SPELL_BEGIN + 10 )
#define APPLY_PASS_DOOR        ( PERM_SPELL_BEGIN + 11 )
#define APPLY_SCRY             ( PERM_SPELL_BEGIN + 17 )
#define APPLY_POISON           ( PERM_SPELL_BEGIN + 20 )
#define APPLY_GIANT_STRENGTH   ( PERM_SPELL_BEGIN + 22 )
#define APPLY_COMBAT_MIND      ( PERM_SPELL_BEGIN + 23 )

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE 1
#define CONT_PICKPROOF 2
#define CONT_CLOSED    4
#define CONT_LOCKED    8

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO       2
#define ROOM_VNUM_CHAT        1
#define ROOM_VNUM_HELL        8
#define ROOM_VNUM_ARTIFACTOR  25097
#define ROOM_VNUM_SMITHY      713
#define ROOM_ARENA_VNUM       7350
#define ROOM_ARENA_ENTER_F    7368
#define ROOM_ARENA_ENTER_S    7369
#define ROOM_ARENA_HALL_SHAME 7370

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_NONE            0
#define ROOM_DARK            1
#define ROOM_NO_MOB          4
#define ROOM_INDOORS         8
#define ROOM_NO_SHADOW       16
#define ROOM_PRIVATE         512
#define ROOM_SAFE            1024
#define ROOM_SOLITARY        2048
#define ROOM_PET_SHOP        4096
#define ROOM_NO_RECALL       8192
#define ROOM_CONE_OF_SILENCE 16384
#define ROOM_NO_MAGIC        32768
#define ROOM_NO_PKILL        65536
#define ROOM_NO_ASTRAL_IN    131072
#define ROOM_NO_ASTRAL_OUT   262144
#define ROOM_TELEPORT_AREA   524288
#define ROOM_TELEPORT_WORLD  1048576
#define ROOM_NO_OFFENSIVE    2097152
#define ROOM_NO_FLEE         4194304
#define ROOM_SILENT          8388608
#define ROOM_BANK            16777216
#define ROOM_NOFLOOR         33554432
#define ROOM_SMITHY          67108864
#define ROOM_NOSCRY          134217728
#define ROOM_DAMAGE          268435456
#define ROOM_PKILL           536870912
#define ROOM_MARK            1073741824

/*
 * Directions.
 * Used in #ROOMS.
 */
#define MAX_DIR 10

#define DIR_NORTH     0
#define DIR_EAST      1
#define DIR_SOUTH     2
#define DIR_WEST      3
#define DIR_UP        4
#define DIR_DOWN      5
#define DIR_NORTHWEST 6
#define DIR_NORTHEAST 7
#define DIR_SOUTHWEST 8
#define DIR_SOUTHEAST 9

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR    1
#define EX_CLOSED    2
#define EX_LOCKED    4
#define EX_BASHED    8
#define EX_BASHPROOF 16
#define EX_PICKPROOF 32
#define EX_PASSPROOF 64
#define EX_RANDOM    128
#define EX_MAGICLOCK 256

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE       0
#define SECT_CITY         1
#define SECT_FIELD        2
#define SECT_FOREST       3
#define SECT_HILLS        4
#define SECT_MOUNTAIN     5
#define SECT_WATER_SWIM   6
#define SECT_WATER_NOSWIM 7
#define SECT_UNDERWATER   8
#define SECT_AIR          9
#define SECT_DESERT       10
#define SECT_BADLAND      11
#define SECT_MAX          12

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE     -1
#define WEAR_LIGHT    0
#define WEAR_FINGER_L 1
#define WEAR_FINGER_R 2
#define WEAR_NECK_1   3
#define WEAR_NECK_2   4
#define WEAR_BODY     5
#define WEAR_HEAD     6
#define WEAR_IN_EYES  7
#define WEAR_ON_FACE  8
#define WEAR_ORBIT    9
#define WEAR_ORBIT_2  10
#define WEAR_LEGS     11
#define WEAR_FEET     12
#define WEAR_HANDS    13
#define WEAR_ARMS     14
#define WEAR_SHIELD   15
#define WEAR_ABOUT    16
#define WEAR_WAIST    17
#define WEAR_WRIST_L  18
#define WEAR_WRIST_R  19
#define WEAR_WIELD    20
#define WEAR_WIELD_2  21
#define WEAR_HOLD     22
#define WEAR_EAR_R    23
#define WEAR_EAR_L    24
#define WEAR_ANKLE_L  25
#define WEAR_ANKLE_R  26
#define MAX_WEAR      27

/***************************************************************************
*                                                                         *
*                   VALUES OF INTEREST TO AREA BUILDERS                   *
*                   (End of this section ... stop here)                   *
*                                                                         *
***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK  0
#define COND_FULL   1
#define COND_THIRST 2
/*
 *  Maxes for conditions.
 */
#define MAX_FULL   50
#define MAX_THIRST 50
#define MAX_DRUNK  50

/*
 * Positions.
 */
#define POS_DEAD       0
#define POS_MORTAL     1
#define POS_INCAP      2
#define POS_STUNNED    3
#define POS_SLEEPING   4
#define POS_RESTING    5
#define POS_FIGHTING   6
#define POS_STANDING   7
#define POS_MEDITATING 8

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC     BV00 // Don't EVER set
#define PLR_BOUGHT_PET BV01
#define PLR_AFK        BV02
#define PLR_AUTOEXIT   BV03
#define PLR_AUTOLOOT   BV04
#define PLR_AUTOSAC    BV05
#define PLR_BLANK      BV06
#define PLR_BRIEF      BV07
#define PLR_FULLNAME   BV08
#define PLR_COMBINE    BV09
#define PLR_PROMPT     BV10
#define PLR_TELNET_GA  BV11
#define PLR_HOLYLIGHT  BV12
#define PLR_WIZINVIS   BV13
#define PLR_QUEST      BV14
#define PLR_SILENCE    BV15
#define PLR_NO_EMOTE   BV16
#define PLR_CLOAKED    BV17
#define PLR_NO_TELL    BV18
#define PLR_LOG        BV19
#define PLR_DENY       BV20
#define PLR_FREEZE     BV21
#define PLR_THIEF      BV22
#define PLR_KILLER     BV23
#define PLR_ANSI       BV24
#define PLR_AUTOCOINS  BV25
#define PLR_AUTOSPLIT  BV26
#define PLR_UNDEAD     BV27
#define PLR_QUESTOR    BV28
#define PLR_COMBAT     BV29
#define PLR_OUTCAST    BV30
#define PLR_PKILLER    BV31
#define PLR_WAR        BV00
#define TEAM_RED       BV01
#define TEAM_BLUE      BV02
#define PLR_REMORT     BV03

#define STUN_TOTAL     0 // Commands and combat halted. Normal stun
#define STUN_COMMAND   1 // Commands halted. Combat goes through
#define STUN_MAGIC     2 // Can't cast spells
#define STUN_NON_MAGIC 3 // No weapon attacks
#define STUN_TO_STUN   4 // Requested. Stop continuous stunning

/*
 * Channel bits.
 */
#define CHANNEL_NONE     0
#define CHANNEL_AUCTION  1
#define CHANNEL_CHAT     2
#define CHANNEL_OOC      4
#define CHANNEL_IMMTALK  8
#define CHANNEL_MUSIC    16
#define CHANNEL_QUESTION 32
#define CHANNEL_SHOUT    64
#define CHANNEL_YELL     128
#define CHANNEL_CLAN     256
#define CHANNEL_CLASS    512
#define CHANNEL_HERO     1024

/*
 * Log Channels
 */
#define CHANNEL_LOG       2048
#define CHANNEL_BUILD     4096
#define CHANNEL_GOD       8192
#define CHANNEL_GUARDIAN  16384
#define CHANNEL_CODER     65536
#define CHANNEL_INFO      131072
#define CHANNEL_CHALLENGE 262144

#define CHANNEL_CLASS_MASTER  1048576
#define CHANNEL_CLAN_MASTER   2097152

#define CHANNEL_IMC   4194304
#define CHANNEL_ARENA 16777216

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct  mob_index_data {
  MOB_INDEX_DATA * next;
  SPEC_FUN       * spec_fun;
  SHOP_DATA      * pShop;
  AREA_DATA      * area;
  MPROG_DATA     * mobprogs;
  char           * player_name;
  char           * short_descr;
  char           * long_descr;
  char           * description;
  int              vnum;
  int              progtypes;
  int              count;
  int              killed;
  short            sex;
  short            class;
  int              level;
  int              act;
  long             affected_by;
  long             affected_by2;
  long             imm_flags;
  long             res_flags;
  long             vul_flags;
  int              alignment;
  int              hitroll;
  int              ac;
  int              hitnodice;
  int              hitsizedice;
  int              hitplus;
  int              damnodice;
  int              damsizedice;
  int              damplus;
  MONEY_DATA       money;
};

struct  gskill_data {
  int    sn;
  void * victim;
  int    level;
  int    timer;
};

/*
 * One character (PC or NPC).
 */
struct  char_data {
  CHAR_DATA       * next;
  CHAR_DATA       * next_in_room;
  CHAR_DATA       * master;
  CHAR_DATA       * leader;
  CHAR_DATA       * fighting;
  CHAR_DATA       * hunting;
  CHAR_DATA       * reply;
  CHAR_DATA       * questgiver;
  CHAR_DATA       * questmob;
  SPEC_FUN        * spec_fun;
  MOB_INDEX_DATA  * pIndexData;
  DESCRIPTOR_DATA * desc;
  AFFECT_DATA     * affected;
  AFFECT_DATA     * affected2;
  NOTE_DATA       * pnote;
  OBJ_DATA        * carrying;
  OBJ_DATA        * questobj;
  ROOM_INDEX_DATA * in_room;
  ROOM_INDEX_DATA * was_in_room;
  POWERED_DATA    * powered;
  PC_DATA         * pcdata;
  MPROG_ACT_LIST  * mpact;
  GSPELL_DATA     * gspell;
  char            * name;
  char            * short_descr;
  char            * long_descr;
  char            * description;
  char            * prompt;
  int               cquestpnts;
  int               questpoints;
  int               nextquest;
  int               countdown;
  short             sex;
  short             class[ MAX_CLASS ];
  int               mpactnum;
  short             race;
  short             clan;
  char              clev;
  int               ctimer;
  int               wizinvis;
  int               cloaked;
  int               level;
  int               antidisarm;
  int               trust;
  bool              wizbit;
  int               played;
  time_t            logon;
  time_t            save_time;
  time_t            last_note;
  int               timer;
  int               wait;
  int               race_wait;
  int               hit;
  int               perm_hit;
  int               mod_hit;
  int               mana;
  int               perm_mana;
  int               mod_mana;
  int               move;
  int               perm_move;
  int               mod_move;
  MONEY_DATA        money;
  int               exp;
  int               act;
  int               act2;
  long              affected_by;
  long              affected_by2;
  long              imm_flags;
  long              res_flags;
  long              vul_flags;
  int               position;
  int               size;
  int               practice;
  int               carry_weight;
  int               carry_number;
  int               saving_throw;
  short             alignment;
  char              start_align;
  int               hitroll;
  int               damroll;
  int               armor;
  int               wimpy;
  int               deaf;
  bool              deleted;
  int               combat_timer;
  int               summon_timer;
  int               stunned[ STUN_MAX ];
  int               wiznet;
  int               warpts;
  int               warkills;
  int               wardeaths;
  int               pkills;
  int               pkilled;
  int               arenawon;
  int               arenalost;
  int               incarnations;
  int               raisepts;

};

struct  mob_prog_act_list {
  MPROG_ACT_LIST * next;
  char           * buf;
  CHAR_DATA      * ch;
  OBJ_DATA       * obj;
  void           * vo;
};

struct  mob_prog_data {
  MPROG_DATA * next;
  int          type;
  char       * arglist;
  char       * comlist;
};

/*
 * Data which only PC's have.
 */
struct  pc_data {
  PC_DATA    * next;
  ALIAS_DATA * alias_list;
  char       * pwd;
  char       * afkchar;
  char       * bamfin;
  char       * bamfout;
  char       * bamfusee;
  char       * transto;
  char       * transfrom;
  char       * transvict;
  char       * slayusee;
  char       * slayroom;
  char       * slayvict;
  char       * whotype;
  char       * title;
  char       * lname;
  char       * empowerments;
  char       * detractments;
  int          perm_str;
  int          perm_int;
  int          perm_wis;
  int          perm_dex;
  int          perm_con;
  int          perm_cha;
  int          mod_str;
  int          mod_int;
  int          mod_wis;
  int          mod_dex;
  int          mod_con;
  int          mod_cha;
  int          condition[ 3 ];
  int          pagelen;
  int          learned[ MAX_SKILL ];
  bool         switched;
  int          security;
  MONEY_DATA   bankaccount;
  OBJ_DATA   * storage;
  int          storcount;
  int          corpses;
  char       * plan;
  char       * email;
  bool         confirm_remort;
};

struct  alias_data {
  ALIAS_DATA * next;
  char       * old;
  char       * new;
};

#define TRAP_ERROR 0

/*
 * The object triggers.. quite a few of em.. :)
 */
#define OBJ_TRAP_ERROR    0         // error!
#define OBJ_TRAP_GET      A         // obj is picked up
#define OBJ_TRAP_DROP     B         // obj is dropped
#define OBJ_TRAP_PUT      C         // obj is put into something
#define OBJ_TRAP_WEAR     D         // obj is worn
#define OBJ_TRAP_LOOK     E         // obj is looked at/examined
#define OBJ_TRAP_LOOK_IN  F         // obj is looked inside (containers)
#define OBJ_TRAP_INVOKE   G         // obj is invoked
#define OBJ_TRAP_USE      H         // obj is used (recited, zapped, ect)
#define OBJ_TRAP_CAST     I         // spell is cast on obj - percent
#define OBJ_TRAP_CAST_SN  J         // spell is cast on obj - by slot
#define OBJ_TRAP_JOIN     K         // obj is joined with another
#define OBJ_TRAP_SEPARATE L         // obj is separated into two
#define OBJ_TRAP_BUY      M         // obj is bought from store
#define OBJ_TRAP_SELL     N         // obj is sold to store
#define OBJ_TRAP_STORE    O         // obj is stored in storage boxes
#define OBJ_TRAP_RETRIEVE P         // obj is retrieved from storage
#define OBJ_TRAP_OPEN     Q         // obj is opened (containers)
#define OBJ_TRAP_CLOSE    R         // obj is closed (containers)
#define OBJ_TRAP_LOCK     S         // obj is locked (containers)
#define OBJ_TRAP_UNLOCK   T         // obj is unlocked (containers)
#define OBJ_TRAP_PICK     U         // obj is picked (containers)
#define OBJ_TRAP_RANDOM   V         // random trigger
#define OBJ_TRAP_THROW    W         // obj is thrown
#define OBJ_TRAP_GET_FROM X         // to allow secondary obj's in get
#define OBJ_TRAP_GIVE     Y         // give an obj away
#define OBJ_TRAP_FILL     Z         // obj is filled (drink_cons)

/*
 * Note that entry/exit/pass are only called if the equivilant exit
 * trap for the exit the person went through failed.
 * Pass is only called if the respective enter or exit trap failed.
 */
#define ROOM_TRAP_ERROR   0         // error!
#define ROOM_TRAP_ENTER   A         // someone enters the room
#define ROOM_TRAP_EXIT    B         // someone leaves the room
#define ROOM_TRAP_PASS    C         // someone enters or leaves
#define ROOM_TRAP_CAST    D         // a spell was cast in room - percent
#define ROOM_TRAP_CAST_SN E         // a spell was cast in room - by slot
#define ROOM_TRAP_SLEEP   F         // someone sleeps in the room
#define ROOM_TRAP_WAKE    G         // someone wakes up in the room
#define ROOM_TRAP_REST    H         // someone rests in the room
#define ROOM_TRAP_DEATH   I         // someone dies in the room
#define ROOM_TRAP_TIME    J         // depends on the time of day
#define ROOM_TRAP_RANDOM  K         // random trigger

/*
 * enter/exit/pass rules are the same as those for room traps.
 * note that look trap is only called if scry trap fails.
 */
#define EXIT_TRAP_ERROR  0          // error!
#define EXIT_TRAP_ENTER  A          // someone enters through the exit
#define EXIT_TRAP_EXIT   B          // someone leaves through the exit
#define EXIT_TRAP_PASS   C          // someone enters/leaves through exit
#define EXIT_TRAP_LOOK   D          // someone looks through exit
#define EXIT_TRAP_SCRY   E          // someone scrys through the exit
#define EXIT_TRAP_OPEN   F          // someone opens the exit (door)
#define EXIT_TRAP_CLOSE  G          // someone closes the exit (door)
#define EXIT_TRAP_LOCK   H          // someone locks the exit (door)
#define EXIT_TRAP_UNLOCK I          // someone unlocks the exit (door)
#define EXIT_TRAP_PICK   J          // someone picks the exit (locked door)

struct trap_data {
  TRAP_DATA       * next;
  TRAP_DATA       * next_here;
  OBJ_INDEX_DATA  * on_obj;
  ROOM_INDEX_DATA * in_room;
  EXIT_DATA       * on_exit;
  int               type;
  char            * arglist;
  char            * comlist;
  bool              disarmable;
  bool              disarmed;
  int               disarm_dur;
};

/*
 * Liquids.
 */
#define LIQ_WATER 0
#define LIQ_BLOOD 13
#define LIQ_MAX   16

struct  liq_type {
  char * liq_name;
  char * liq_color;
  int    liq_affect[ 3 ];
};

/*
 * Extra description data for a room or object.
 */
struct  extra_descr_data {
  EXTRA_DESCR_DATA * next;        // Next in list
  char             * keyword;     // Keyword in look/examine
  char             * description; // What to see
  bool               deleted;
};

/*
 * Prototype for an object.
 */
struct  obj_index_data {
  OBJ_INDEX_DATA   * next;
  EXTRA_DESCR_DATA * extra_descr;
  AFFECT_DATA      * affected;
  AREA_DATA        * area;
  TRAP_DATA        * traps;
  int                traptypes;
  char             * name;
  char             * short_descr;
  char             * description;
  int                vnum;
  int                item_type;
  int                extra_flags;
  int                anti_race_flags;
  int                anti_class_flags;
  int                wear_flags;
  int                count;
  int                weight;
  MONEY_DATA         cost;
  int                level;
  int                value[ 4 ];
  int                ac_type;
  int                ac_vnum;
  char             * ac_spell;
  int                ac_charge[ 2 ];
  int                join;
  int                sep_one;
  int                sep_two;
};

/*
 * One object.
 */
struct  obj_data {
  OBJ_DATA         * next;
  OBJ_DATA         * next_content;
  OBJ_DATA         * contains;
  OBJ_DATA         * in_obj;
  CHAR_DATA        * carried_by;
  CHAR_DATA        * stored_by;
  EXTRA_DESCR_DATA * extra_descr;
  AFFECT_DATA      * affected;
  OBJ_INDEX_DATA   * pIndexData;
  ROOM_INDEX_DATA  * in_room;
  char             * name;
  char             * short_descr;
  char             * description;
  int                item_type;
  int                extra_flags;
  int                anti_race_flags;
  int                anti_class_flags;
  int                wear_flags;
  int                wear_loc;
  int                weight;
  MONEY_DATA         cost;
  int                level;
  int                timer;
  int                value[ 4 ];
  int                ac_type;
  int                ac_vnum;
  char             * ac_spell;
  int                ac_charge[ 2 ];
  bool               deleted;
};

/*
 * Exit data.
 */
struct  exit_data {
  ROOM_INDEX_DATA * to_room;
  EXIT_DATA       * next;
  TRAP_DATA       * traps;
  int               traptypes;
  int               rs_flags;
  int               vnum;
  int               exit_info;
  int               key;
  char            * keyword;
  char            * description;
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct  reset_data {
  RESET_DATA * next;
  char         command;
  int          arg1;
  int          arg2;
  int          arg3;
};

/*
 * Area definition.
 */
struct  area_data {
  AREA_DATA * next;
  char      * name;
  int         recall;
  int         age;
  int         nplayer;
  char      * filename;
  char      * builders;
  int         security;
  int         lvnum;
  int         uvnum;
  int         vnum;
  int         area_flags;
  char      * reset_sound;
  int         windstr;
  int         winddir;
  short       llevel;
  short       ulevel;
  int         def_color;
};

struct  new_clan_data {
  CLAN_DATA * next;
  MONEY_DATA  bankaccount;
  char      * name;
  char      * diety;
  char      * description;
  char      * leader;
  char      * first;
  char      * second;
  char      * champ;
  bool        isleader;
  bool        isfirst;
  bool        issecond;
  bool        ischamp;
  int         vnum;
  int         recall;
  int         pkills;
  int         mkills;
  int         members;
  int         pdeaths;
  int         mdeaths;
  int         obj_vnum_1;
  int         obj_vnum_2;
  int         obj_vnum_3;
  int         settings;
};

/*
 * ROOM AFFECT type
 */
struct  room_affect_data {
  ROOM_AFFECT_DATA * next;
  ROOM_INDEX_DATA  * room;
  CHAR_DATA        * powered_by;
  OBJ_DATA         * material;
  int                type;
  int                location;
};

struct  powered_data {
  POWERED_DATA     * next;
  ROOM_INDEX_DATA  * room;
  ROOM_AFFECT_DATA * raf;
  int                type;
  int                cost;
};

/*
 * Room type.
 */
struct  room_index_data {
  ROOM_INDEX_DATA  * next;
  ROOM_AFFECT_DATA * rAffect;
  CHAR_DATA        * people;
  OBJ_DATA         * contents;
  EXTRA_DESCR_DATA * extra_descr;
  AREA_DATA        * area;
  EXIT_DATA        * exit[ MAX_DIR ];
  RESET_DATA       * reset_first;
  RESET_DATA       * reset_last;
  TRAP_DATA        * traps;
  int                traptypes;
  char             * name;
  char             * description;
  int                vnum;
  int                room_flags;
  int                light;
  char               sector_type;
  int                rd; // room damage
};

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED -1
#define TYPE_HIT       1000

/*
 *  Target types.
 */
#define TAR_IGNORE          0
#define TAR_CHAR_OFFENSIVE  1
#define TAR_CHAR_DEFENSIVE  2
#define TAR_CHAR_SELF       3
#define TAR_OBJ_INV         4
#define TAR_GROUP_OFFENSIVE 5
#define TAR_GROUP_DEFENSIVE 6
#define TAR_GROUP_ALL       7
#define TAR_GROUP_OBJ       8
#define TAR_GROUP_IGNORE    9

/*
 * Skills include spells as a particular case.
 */
struct  skill_type {
  char      * name;                         // Name of skill
  int         skill_level[ MAX_CLASS + 3 ]; // Level needed by class
  SPELL_FUN * spell_fun;                    // Spell pointer (for spells)
  int         target;                       // Legal targets
  int         minimum_position;             // Position for caster / user
  int       * pgsn;                         // Pointer to associated gsn
  int         min_mana;                     // Minimum mana used
  int         beats;                        // Waiting time after use
  char      * noun_damage;                  // Damage message
  char      * msg_off;                      // Wear off message
  char      * room_msg_off;                 // Room Wear off message
  bool        dispelable;
  int         slot;                         // For object loading
};

struct gskill_type {
  int wait;                              // Wait for casters in ticks
  int slot;                              // Matching skill_table sn
  int casters[ MAX_CLASS ];              // Casters needed by class
};

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern int gsn_trip;
extern int gsn_track;
extern int gsn_shield_block;
extern int gsn_drain_life;
extern int gsn_mental_drain;
extern int gsn_turn_evil;
extern int gsn_bash_door;
extern int gsn_stun;
extern int gsn_backstab;
extern int gsn_palm;
extern int gsn_dodge;
extern int gsn_dual;
extern int gsn_hide;
extern int gsn_peek;
extern int gsn_pick_lock;
extern int gsn_punch;
extern int gsn_sneak;
extern int gsn_steal;
extern int gsn_disarm;
extern int gsn_poison_weapon;
extern int gsn_kick;
extern int gsn_circle;
extern int gsn_throw;
extern int gsn_parry;
extern int gsn_rescue;
extern int gsn_patch;
extern int gsn_gouge;
extern int gsn_alchemy;
extern int gsn_scribe;
extern int gsn_blindness;
extern int gsn_charm_person;
extern int gsn_invis;
extern int gsn_mass_invis;
extern int gsn_poison;
extern int gsn_sleep;
extern int gsn_prayer;
extern int gsn_scrolls;
extern int gsn_staves;
extern int gsn_wands;
extern int gsn_spellcraft;
extern int gsn_gravebind;
extern int gsn_multiburst;
extern int gsn_fastheal;
extern int gsn_rage;
extern int gsn_headbutt;
extern int gsn_retreat;
extern int gsn_escape;
extern int gsn_antidote;
extern int gsn_haggle;
extern int gsn_incinerate;
extern int gsn_grip;
extern int gsn_lure;
extern int gsn_domination;
extern int gsn_shadow;
extern int gsn_flip;
extern int gsn_hallucinate;

/*
 * Utility macros.
 */
#define UMIN( a, b )           ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define UMAX( a, b )           ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define URANGE( a, b, c )      ( ( b ) < ( a ) ? ( a ) : ( ( b ) > ( c ) ? ( c ) : ( b ) ) )
#define LOWER( c )             ( ( c ) >= 'A' && ( c ) <= 'Z' ? ( c ) + 'a' - 'A' : ( c ) )
#define UPPER( c )             ( ( c ) >= 'a' && ( c ) <= 'z' ? ( c ) + 'A' - 'a' : ( c ) )
#define IS_SET( flag, bit )    ( ( flag ) &   ( bit ) )
#define SET_BIT( var, bit )    ( ( var )  |=  ( bit ) )
#define REMOVE_BIT( var, bit ) ( ( var )  &= ~( bit ) )
#define TOGGLE_BIT( var, bit ) ( ( var )  ^=  ( bit ) )
#define COUNT( a )             ( ( sizeof( a ) ) / ( sizeof( a[0] ) ) )
#define RANDOM( a )            ( number_range( 0, ( COUNT( a ) - 1 ) ) )

/*
 * Character macros.
 */
#define IS_CODER( ch )   ( IS_SET( ch->affected_by2, CODER ) )
#define IS_NPC( ch )     ( IS_SET( ch->act, ACT_IS_NPC ) )
#define IS_QUESTOR( ch ) ( IS_SET( ch->act, PLR_QUESTOR ) )

#define IS_IMMORTAL( ch ) ( get_trust( ch ) >= LEVEL_IMMORTAL )
#define IS_HERO( ch )     ( get_trust( ch ) >= LEVEL_HERO     )

#define IS_AFFECTED( ch, sn )  ( IS_SET( ch->affected_by, ( sn ) ) )
#define IS_AFFECTED2( ch, sn ) ( IS_SET( ch->affected_by2, ( sn ) ) )
#define IS_SIMM( ch, sn )      ( IS_SET( ch->imm_flags, ( sn ) ) )
#define IS_SRES( ch, sn )      ( IS_SET( ch->res_flags, ( sn ) ) )
#define IS_SVUL( ch, sn )      ( IS_SET( ch->vul_flags, ( sn ) ) )

#define IS_GOOD( ch )    ( ch->alignment >=  350 )
#define IS_EVIL( ch )    ( ch->alignment <= -350 )
#define IS_NEUTRAL( ch ) ( !IS_GOOD( ch ) && !IS_EVIL( ch ) )

#define IS_AWAKE( ch ) ( ch->position > POS_SLEEPING )

#define GET_AC( ch )      ( ch->armor + ( IS_AWAKE( ch ) ? dex_app[ get_curr_dex( ch ) ].defensive : 0 ) + size_table[ ch->size ].mac )
#define GET_HITROLL( ch ) ( ch->hitroll + str_app[ get_curr_str( ch ) ].tohit )
#define GET_DAMROLL( ch ) ( ch->damroll + str_app[ get_curr_str( ch ) ].todam )
#define GET_BAB( ch )     ( (int)( ch->level * ( IS_NPC( ch ) ? 1 : class_table[ prime_class( ch ) ].mbab ) ) )
#define GET_THAC0( ch )   ( 20 - GET_BAB( ch ) - GET_HITROLL( ch ) )
#define GET_MOD( stat )   ( (int)( ( stat / 2 ) - 5 ) )

#define IS_OUTSIDE( ch ) ( !IS_SET( ( ch )->in_room->room_flags, ROOM_INDOORS ) )

#define WAIT_STATE( ch, pulse ) ( ( ch )->wait = UMAX( ( ch )->wait, ( pulse ) ) )

#define STUN_CHAR( ch, pulse, type ) ( ( ch )->stunned[ ( type ) ] = UMAX( ( ch )->stunned[ ( type ) ], ( pulse ) ) )

#define IS_STUNNED( ch, type ) ( ( ch )->stunned[ ( type ) ] > 0 )

int mmlvl_mana( CHAR_DATA * ch, int sn );

#define MANA_COST( ch, sn ) ( IS_NPC( ch ) ? 0 : UMAX( skill_table[ sn ].min_mana, 100 / ( 2 + ch->level - mmlvl_mana( ch, sn ) ) ) )
#define IS_SWITCHED( ch )   ( ch->pcdata->switched )

#define UNDEAD_TYPE( ch ) ( IS_NPC( ch ) ? ACT_UNDEAD : PLR_UNDEAD )

#define MAX_HIT( ch )  ( ( ch )->perm_hit + ( ch )->mod_hit )
#define MAX_MANA( ch ) ( ( ch )->perm_mana + ( ch )->mod_mana )
#define MAX_MOVE( ch ) ( ( ch )->perm_move + ( ch )->mod_move )

#define CAN_FLY( ch ) ( IS_AFFECTED( ch, AFF_FLYING ) )
/*
 * Object macros.
 */
#define CAN_WEAR( obj, part )   ( IS_SET( ( obj )->wear_flags,  ( part ) ) )
#define IS_OBJ_STAT( obj, stat )( IS_SET( ( obj )->extra_flags, ( stat ) ) )
#define IS_ANTI_CLASS( obj, stat )( IS_SET( ( obj )->anti_class_flags, ( stat ) ) )
#define IS_ANTI_RACE( obj, stat ) ( IS_SET( ( obj )->anti_race_flags, ( stat ) ) )

/*
 * Arena macro.
 */
#define IS_ARENA( ch ) ( !IS_NPC( ( ch ) ) && ( ch )->in_room && \
                         ( ch )->in_room->area == arena.area &&  \
                         ( ( ch ) == arena.fch || ( ch ) == arena.sch ) )

/*
 * Structure for a command in the command lookup table.
 */
struct  cmd_type {
  char * const name;
  DO_FUN     * do_fun;
  int          position;
  int          level;
  int          log;
  bool         npc;
};

/*
 * Global constants.
 */
extern const struct  str_app_type str_app[ 31 ];
extern const struct  int_app_type int_app[ 31 ];
extern const struct  wis_app_type wis_app[ 31 ];
extern const struct  dex_app_type dex_app[ 31 ];
extern const struct  con_app_type con_app[ 31 ];

extern const struct  class_type     class_table[ MAX_CLASS ];
extern const struct  race_type      race_table[ MAX_RACE  ];
extern const struct  size_type      size_table[ MAX_SIZE  ];
extern const struct  direction_type direction_table[ MAX_DIR   ];

extern const struct  wiznet_type wiznet_table    [];

extern struct  cmd_type cmd_table[];

extern const struct  liq_type    liq_table[ LIQ_MAX     ];
extern const struct  skill_type  skill_table[ MAX_SKILL ];
extern const struct  gskill_type gskill_table[ MAX_GSPELL  ];

extern char *  const title_table[ MAX_CLASS + 3 ][ MAX_LEVEL + 1 ][ 2 ];

/*
 * Global variables.
 */

extern NEWBIE_DATA      * newbie_first;
extern NEWBIE_DATA      * newbie_last;
extern PLAYERLIST_DATA  * playerlist;
extern SOCIAL_DATA      * social_first;
extern SOCIAL_DATA      * social_last;
extern HELP_DATA        * help_first;
extern HELP_DATA        * help_last;
extern HELP_DATA        * help_free;
extern SHOP_DATA        * shop_first;
extern BAN_DATA         * ban_list;
extern CHAR_DATA        * char_list;
extern DESCRIPTOR_DATA  * descriptor_list;
extern NOTE_DATA        * note_list;
extern OBJ_DATA         * object_list;
extern TRAP_DATA        * trap_list;
extern AFFECT_DATA      * affect_free;
extern BAN_DATA         * ban_free;
extern CHAR_DATA        * char_free;
extern DESCRIPTOR_DATA  * descriptor_free;
extern EXTRA_DESCR_DATA * extra_descr_free;
extern NOTE_DATA        * note_free;
extern OBJ_DATA         * obj_free;
extern PC_DATA          * pcdata_free;

extern char           bug_buf   [];
extern time_t         current_time;
extern bool           fLogAll;
extern FILE         * fpReserve;
extern KILL_DATA      kill_table  [];
extern char           log_buf   [];
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA   weather_info;
extern ARENA_DATA     arena;
extern WAR_DATA       war;
extern char         * down_time;
extern char         * warning1;
extern char         * warning2;
extern int            stype;
extern int            prtv;
extern bool           quest;
extern int            qmin;
extern int            qmax;
extern int            port;

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_doubleexp );
DECLARE_DO_FUN( do_shieldify );
DECLARE_DO_FUN( do_clanquest );
DECLARE_DO_FUN( do_quest );
DECLARE_DO_FUN( do_qecho );
DECLARE_DO_FUN( do_qset );
DECLARE_DO_FUN( do_mpasound );
DECLARE_DO_FUN( do_mpat );
DECLARE_DO_FUN( do_mpecho );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat );
DECLARE_DO_FUN( do_mpforce );
DECLARE_DO_FUN( do_mpgoto );
DECLARE_DO_FUN( do_mpjunk );
DECLARE_DO_FUN( do_mpkill );
DECLARE_DO_FUN( do_mpmload );
DECLARE_DO_FUN( do_mpoload );
DECLARE_DO_FUN( do_mppurge );
DECLARE_DO_FUN( do_mpstat );
DECLARE_DO_FUN( do_mpcommands );
DECLARE_DO_FUN( do_mpteleport );
DECLARE_DO_FUN( do_mptransfer );
DECLARE_DO_FUN( do_opstat );
DECLARE_DO_FUN( do_rpstat );
DECLARE_DO_FUN( do_addlag );
DECLARE_DO_FUN( do_accept );
DECLARE_DO_FUN( do_account );
DECLARE_DO_FUN( do_advance );
DECLARE_DO_FUN( do_affectedby );
DECLARE_DO_FUN( do_raffect );
DECLARE_DO_FUN( do_afk );
DECLARE_DO_FUN( do_afkmes );
DECLARE_DO_FUN( do_allow );
DECLARE_DO_FUN( do_ansi );
DECLARE_DO_FUN( do_answer );
DECLARE_DO_FUN( do_antidote );
DECLARE_DO_FUN( do_areas );
DECLARE_DO_FUN( do_astat );
DECLARE_DO_FUN( do_astrip );
DECLARE_DO_FUN( do_at );
DECLARE_DO_FUN( do_auction );
DECLARE_DO_FUN( do_auto );
DECLARE_DO_FUN( do_autoexit );
DECLARE_DO_FUN( do_autocoins );
DECLARE_DO_FUN( do_autoloot );
DECLARE_DO_FUN( do_autosac );
DECLARE_DO_FUN( do_autosplit );
DECLARE_DO_FUN( do_backstab );
DECLARE_DO_FUN( do_bamf );
DECLARE_DO_FUN( do_bash );
DECLARE_DO_FUN( do_bid );
DECLARE_DO_FUN( do_blank );
DECLARE_DO_FUN( do_bodybag );
DECLARE_DO_FUN( do_brandish );
DECLARE_DO_FUN( do_brief );
DECLARE_DO_FUN( do_bug );
DECLARE_DO_FUN( do_bugs );
DECLARE_DO_FUN( do_buy );
DECLARE_DO_FUN( do_challenge );
DECLARE_DO_FUN( do_champlist );
DECLARE_DO_FUN( do_cast );
DECLARE_DO_FUN( do_changes );
DECLARE_DO_FUN( do_channels );
DECLARE_DO_FUN( do_chat );
DECLARE_DO_FUN( do_clan );
DECLARE_DO_FUN( do_clone );
DECLARE_DO_FUN( do_cinfo );
DECLARE_DO_FUN( do_circle );
DECLARE_DO_FUN( do_clans );
DECLARE_DO_FUN( do_clandesc );
DECLARE_DO_FUN( do_class );
DECLARE_DO_FUN( do_cloak );
DECLARE_DO_FUN( do_close );
DECLARE_DO_FUN( do_combat );
DECLARE_DO_FUN( do_combine );
DECLARE_DO_FUN( do_commands );
DECLARE_DO_FUN( do_compare );
DECLARE_DO_FUN( do_config );
DECLARE_DO_FUN( do_consider );
DECLARE_DO_FUN( do_countcommands );
DECLARE_DO_FUN( do_credits );
DECLARE_DO_FUN( do_cset );
DECLARE_DO_FUN( do_darkinvis );
DECLARE_DO_FUN( do_delet );
DECLARE_DO_FUN( do_delete );
DECLARE_DO_FUN( do_deny );
DECLARE_DO_FUN( do_deposit );
DECLARE_DO_FUN( do_desc_check );
DECLARE_DO_FUN( do_descript_clean );
DECLARE_DO_FUN( do_description );
DECLARE_DO_FUN( do_detract );
DECLARE_DO_FUN( do_disarm );
DECLARE_DO_FUN( do_disconnect );
DECLARE_DO_FUN( do_donate );
DECLARE_DO_FUN( do_down );
DECLARE_DO_FUN( do_drink );
DECLARE_DO_FUN( do_drop );
DECLARE_DO_FUN( do_dual );
DECLARE_DO_FUN( do_east );
DECLARE_DO_FUN( do_eat );
DECLARE_DO_FUN( do_echo );
DECLARE_DO_FUN( do_email );
DECLARE_DO_FUN( do_emote );
DECLARE_DO_FUN( do_empower );
DECLARE_DO_FUN( do_enter );
DECLARE_DO_FUN( do_equipment );
DECLARE_DO_FUN( do_examine );
DECLARE_DO_FUN( do_exits );
DECLARE_DO_FUN( do_fill );
DECLARE_DO_FUN( do_finger );
DECLARE_DO_FUN( do_flee );
DECLARE_DO_FUN( do_follow );
DECLARE_DO_FUN( do_force );
DECLARE_DO_FUN( do_freeze );
DECLARE_DO_FUN( do_fullname );
DECLARE_DO_FUN( do_get );
DECLARE_DO_FUN( do_give );
DECLARE_DO_FUN( do_goto );
DECLARE_DO_FUN( do_group );
DECLARE_DO_FUN( do_gtell );
DECLARE_DO_FUN( do_guard );
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_hero );
DECLARE_DO_FUN( do_hide );
DECLARE_DO_FUN( do_hlist );
DECLARE_DO_FUN( do_holylight );
DECLARE_DO_FUN( do_hood );
DECLARE_DO_FUN( do_hotreboo );
DECLARE_DO_FUN( do_hotreboot );
DECLARE_DO_FUN( do_identify );
DECLARE_DO_FUN( do_idea );
DECLARE_DO_FUN( do_ideas );
DECLARE_DO_FUN( do_immlist );
DECLARE_DO_FUN( do_immtalk );
DECLARE_DO_FUN( do_indestructable );
DECLARE_DO_FUN( do_induct );
DECLARE_DO_FUN( do_info );
DECLARE_DO_FUN( do_inventory );
DECLARE_DO_FUN( do_invoke );
DECLARE_DO_FUN( do_invis );
DECLARE_DO_FUN( do_irongrip );
DECLARE_DO_FUN( do_join );
DECLARE_DO_FUN( do_kick );
DECLARE_DO_FUN( do_kill );
DECLARE_DO_FUN( do_list );
DECLARE_DO_FUN( do_lock );
DECLARE_DO_FUN( do_log );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_lowrecall );
DECLARE_DO_FUN( do_memory );
DECLARE_DO_FUN( do_mental_drain );
DECLARE_DO_FUN( do_mfind );
DECLARE_DO_FUN( do_mload );
DECLARE_DO_FUN( do_mset );
DECLARE_DO_FUN( do_mstat );
DECLARE_DO_FUN( do_mwhere );
DECLARE_DO_FUN( do_murde );
DECLARE_DO_FUN( do_murder );
DECLARE_DO_FUN( do_music );
DECLARE_DO_FUN( do_newbie );
DECLARE_DO_FUN( do_newcorpse );
DECLARE_DO_FUN( do_newlock );
DECLARE_DO_FUN( do_noemote );
DECLARE_DO_FUN( do_north );
DECLARE_DO_FUN( do_northwest );
DECLARE_DO_FUN( do_northeast );
DECLARE_DO_FUN( do_note );
DECLARE_DO_FUN( do_notell );
DECLARE_DO_FUN( do_nukerep );
DECLARE_DO_FUN( do_numlock );
DECLARE_DO_FUN( do_ofind );
DECLARE_DO_FUN( do_olist );
DECLARE_DO_FUN( do_oload );
DECLARE_DO_FUN( do_ooc );
DECLARE_DO_FUN( do_open );
DECLARE_DO_FUN( do_order );
DECLARE_DO_FUN( do_oset );
DECLARE_DO_FUN( do_ostat );
DECLARE_DO_FUN( do_outcast );
DECLARE_DO_FUN( do_owhere );
DECLARE_DO_FUN( do_pagelen );
DECLARE_DO_FUN( do_pardon );
DECLARE_DO_FUN( do_password );
DECLARE_DO_FUN( do_patch );
DECLARE_DO_FUN( do_peace );
DECLARE_DO_FUN( do_pick );
DECLARE_DO_FUN( do_pkill );
DECLARE_DO_FUN( do_plan );
DECLARE_DO_FUN( do_playerlist );
DECLARE_DO_FUN( do_pload );
DECLARE_DO_FUN( do_poison_weapon );
DECLARE_DO_FUN( do_practice );
DECLARE_DO_FUN( do_pray );
DECLARE_DO_FUN( do_prompt );
DECLARE_DO_FUN( do_punch );
DECLARE_DO_FUN( do_purge );
DECLARE_DO_FUN( do_put );
DECLARE_DO_FUN( do_quaff );
DECLARE_DO_FUN( do_question );
DECLARE_DO_FUN( do_qui );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_reboo );
DECLARE_DO_FUN( do_reboot );
DECLARE_DO_FUN( do_rebuild );
DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_recho );
DECLARE_DO_FUN( do_recite );
DECLARE_DO_FUN( do_remake );
DECLARE_DO_FUN( do_remote );
DECLARE_DO_FUN( do_remove );
DECLARE_DO_FUN( do_repair );
DECLARE_DO_FUN( do_reply );
DECLARE_DO_FUN( do_report );
DECLARE_DO_FUN( do_rescue );
DECLARE_DO_FUN( do_rest );
DECLARE_DO_FUN( do_restrict );
DECLARE_DO_FUN( do_restore );
DECLARE_DO_FUN( do_retreat );
DECLARE_DO_FUN( do_escape );
DECLARE_DO_FUN( do_retrieve );
DECLARE_DO_FUN( do_return );
DECLARE_DO_FUN( do_rset );
DECLARE_DO_FUN( do_rstat );
DECLARE_DO_FUN( do_sacrifice );
DECLARE_DO_FUN( do_save );
DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_score );
DECLARE_DO_FUN( do_sell );
DECLARE_DO_FUN( do_seize );
DECLARE_DO_FUN( do_separate );
DECLARE_DO_FUN( do_setlev );
DECLARE_DO_FUN( do_shadow );
DECLARE_DO_FUN( do_shout );
DECLARE_DO_FUN( do_shutdow );
DECLARE_DO_FUN( do_shutdown );
DECLARE_DO_FUN( do_silence );
DECLARE_DO_FUN( do_sla );
DECLARE_DO_FUN( do_slay );
DECLARE_DO_FUN( do_slaymes );
DECLARE_DO_FUN( do_sleep );
DECLARE_DO_FUN( do_slist );
DECLARE_DO_FUN( do_slookup );
DECLARE_DO_FUN( do_smash );
DECLARE_DO_FUN( do_sneak );
DECLARE_DO_FUN( do_snoop );
DECLARE_DO_FUN( do_socials );
DECLARE_DO_FUN( do_south );
DECLARE_DO_FUN( do_southwest );
DECLARE_DO_FUN( do_southeast );
DECLARE_DO_FUN( do_spells );
DECLARE_DO_FUN( do_split );
DECLARE_DO_FUN( do_sset );
DECLARE_DO_FUN( do_sstat );
DECLARE_DO_FUN( do_sstime );
DECLARE_DO_FUN( do_stand );
DECLARE_DO_FUN( do_steal );
DECLARE_DO_FUN( do_store );
DECLARE_DO_FUN( do_stun );
DECLARE_DO_FUN( do_switch );
DECLARE_DO_FUN( do_tell );
DECLARE_DO_FUN( do_telnetga );
DECLARE_DO_FUN( do_throw );
DECLARE_DO_FUN( do_trip );
DECLARE_DO_FUN( do_time );
DECLARE_DO_FUN( do_title );
DECLARE_DO_FUN( do_todo );
DECLARE_DO_FUN( do_track );
DECLARE_DO_FUN( do_train );
DECLARE_DO_FUN( do_transfer );
DECLARE_DO_FUN( do_transport );
DECLARE_DO_FUN( do_trmes );
DECLARE_DO_FUN( do_trust );
DECLARE_DO_FUN( do_typo );
DECLARE_DO_FUN( do_typos );
DECLARE_DO_FUN( do_unlock );
DECLARE_DO_FUN( do_up );
DECLARE_DO_FUN( do_users );
DECLARE_DO_FUN( do_value );
DECLARE_DO_FUN( do_visible );
DECLARE_DO_FUN( do_voodo );
DECLARE_DO_FUN( do_vused );
DECLARE_DO_FUN( do_wake );
DECLARE_DO_FUN( do_wear );
DECLARE_DO_FUN( do_weather );
DECLARE_DO_FUN( do_west );
DECLARE_DO_FUN( do_where );
DECLARE_DO_FUN( do_who );
DECLARE_DO_FUN( do_whotype );
DECLARE_DO_FUN( do_wimpy );
DECLARE_DO_FUN( do_withdraw );
DECLARE_DO_FUN( do_wizhelp );
DECLARE_DO_FUN( do_wizify );
DECLARE_DO_FUN( do_wizlock );
DECLARE_DO_FUN( do_wiznet );
DECLARE_DO_FUN( do_worth );
DECLARE_DO_FUN( do_wrlist );
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_zap );
DECLARE_DO_FUN( do_stare );
DECLARE_DO_FUN( do_push );
DECLARE_DO_FUN( do_drag );
DECLARE_DO_FUN( do_authorize );
DECLARE_DO_FUN( do_drain_life );
DECLARE_DO_FUN( do_gouge );
DECLARE_DO_FUN( do_alchemy );
DECLARE_DO_FUN( do_scribe );
DECLARE_DO_FUN( do_multiburst );
DECLARE_DO_FUN( do_gravebind );
DECLARE_DO_FUN( do_rage );
DECLARE_DO_FUN( do_palm );
DECLARE_DO_FUN( do_forge );
DECLARE_DO_FUN( do_permban );
DECLARE_DO_FUN( do_tempban );
DECLARE_DO_FUN( do_newban );
DECLARE_DO_FUN( do_banlist );
DECLARE_DO_FUN( do_scan );
DECLARE_DO_FUN( do_war );
DECLARE_DO_FUN( do_declare );
DECLARE_DO_FUN( do_remor );
DECLARE_DO_FUN( do_remort );
DECLARE_DO_FUN( do_raise );
DECLARE_DO_FUN( do_lure );
DECLARE_DO_FUN( do_flip );

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN( spell_null );
DECLARE_SPELL_FUN( spell_animate );
DECLARE_SPELL_FUN( spell_armor );
DECLARE_SPELL_FUN( spell_astral );
DECLARE_SPELL_FUN( spell_aura );
DECLARE_SPELL_FUN( spell_blindness );
DECLARE_SPELL_FUN( spell_change_sex );
DECLARE_SPELL_FUN( spell_charm_person );
DECLARE_SPELL_FUN( spell_continual_light );
DECLARE_SPELL_FUN( spell_control_weather );
DECLARE_SPELL_FUN( spell_create_food );
DECLARE_SPELL_FUN( spell_create_water );
DECLARE_SPELL_FUN( spell_cure_blindness );
DECLARE_SPELL_FUN( spell_cure_poison );
DECLARE_SPELL_FUN( spell_dispel_magic );
DECLARE_SPELL_FUN( spell_enchant_weapon );
DECLARE_SPELL_FUN( spell_energy_drain );
DECLARE_SPELL_FUN( spell_flamestrike );
DECLARE_SPELL_FUN( spell_fly );
DECLARE_SPELL_FUN( spell_gate );
DECLARE_SPELL_FUN( spell_giant_strength );
DECLARE_SPELL_FUN( spell_eternal_intellect );
DECLARE_SPELL_FUN( spell_goodberry );
DECLARE_SPELL_FUN( spell_harm );
DECLARE_SPELL_FUN( spell_heal );
DECLARE_SPELL_FUN( spell_icestorm );
DECLARE_SPELL_FUN( spell_incinerate );
DECLARE_SPELL_FUN( spell_infravision );
DECLARE_SPELL_FUN( spell_invis );
DECLARE_SPELL_FUN( spell_know_alignment );
DECLARE_SPELL_FUN( spell_lightning_bolt );
DECLARE_SPELL_FUN( spell_locate_object );
DECLARE_SPELL_FUN( spell_magic_missile );
DECLARE_SPELL_FUN( spell_mana );
DECLARE_SPELL_FUN( spell_mass_invis );
DECLARE_SPELL_FUN( spell_mental_block );
DECLARE_SPELL_FUN( spell_pass_door );
DECLARE_SPELL_FUN( spell_permenancy );
DECLARE_SPELL_FUN( spell_poison );
DECLARE_SPELL_FUN( spell_portal );
DECLARE_SPELL_FUN( spell_protection );
DECLARE_SPELL_FUN( spell_refresh );
DECLARE_SPELL_FUN( spell_scry );
DECLARE_SPELL_FUN( spell_shield );
DECLARE_SPELL_FUN( spell_sleep );
DECLARE_SPELL_FUN( spell_spell_bind );
DECLARE_SPELL_FUN( spell_summon );
DECLARE_SPELL_FUN( spell_teleport );
DECLARE_SPELL_FUN( spell_turn_undead );
DECLARE_SPELL_FUN( spell_word_of_recall );
DECLARE_SPELL_FUN( spell_summon_swarm );
DECLARE_SPELL_FUN( spell_summon_pack );
DECLARE_SPELL_FUN( spell_summon_demon );
DECLARE_SPELL_FUN( spell_cancellation );
DECLARE_SPELL_FUN( spell_protection_good );
DECLARE_SPELL_FUN( spell_holy_strength );
DECLARE_SPELL_FUN( spell_summon_angel );
DECLARE_SPELL_FUN( spell_holy_fires );
DECLARE_SPELL_FUN( spell_truesight );
DECLARE_SPELL_FUN( spell_entangle );
DECLARE_SPELL_FUN( spell_confusion );
DECLARE_SPELL_FUN( spell_mind_probe );
DECLARE_SPELL_FUN( spell_fumble );
DECLARE_SPELL_FUN( spell_summon_shadow );
DECLARE_SPELL_FUN( spell_summon_beast );
DECLARE_SPELL_FUN( spell_summon_trent );
DECLARE_SPELL_FUN( spell_shatter );
DECLARE_SPELL_FUN( spell_molecular_unbind );
DECLARE_SPELL_FUN( spell_phase_shift );
DECLARE_SPELL_FUN( spell_healing_hands );
DECLARE_SPELL_FUN( spell_purify );
DECLARE_SPELL_FUN( spell_silence );
DECLARE_SPELL_FUN( spell_hallucinate );
DECLARE_SPELL_FUN( spell_aura_sight );
DECLARE_SPELL_FUN( spell_combat_mind );
DECLARE_SPELL_FUN( spell_complete_healing );
DECLARE_SPELL_FUN( spell_control_flames );
DECLARE_SPELL_FUN( spell_create_sound );
DECLARE_SPELL_FUN( spell_detonate );
DECLARE_SPELL_FUN( spell_disintegrate );
DECLARE_SPELL_FUN( spell_displacement );
DECLARE_SPELL_FUN( spell_disrupt );
DECLARE_SPELL_FUN( spell_domination );
DECLARE_SPELL_FUN( spell_ectoplasmic_form );
DECLARE_SPELL_FUN( spell_ego_whip );
DECLARE_SPELL_FUN( spell_flesh_armor );
DECLARE_SPELL_FUN( spell_inflict_pain );
DECLARE_SPELL_FUN( spell_intellect_fortress );
DECLARE_SPELL_FUN( spell_lend_health );
DECLARE_SPELL_FUN( spell_levitation );
DECLARE_SPELL_FUN( spell_mind_thrust );
DECLARE_SPELL_FUN( spell_thought_shield );
DECLARE_SPELL_FUN( spell_ultrablast );
DECLARE_SPELL_FUN( gspell_flamesphere );
DECLARE_SPELL_FUN( spell_dark_ritual );
DECLARE_SPELL_FUN( spell_stench_of_decay );

char *  crypt( const char * key, const char * salt );

/*
 * The crypt(3) function is not available on some operating systems.
 *
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 *
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if defined( NOCRYPT )
#define crypt( s1, s2 ) ( s1 )
#endif

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
// areas
#define AREA_DIR        "area/"              // Area files
#define AREA_LIST       "area/area.lst"      // List of areas

// binaries
#define EXE_FILE        "bin/envy"           // EXE file

// game data
#define HELP_FILE       "data/help.dat"      // Help file
#define DOWN_TIME_FILE  "data/time.txt"      // For automatic shutdown
#define BAN_LIST        "data/banned.lst"    // List of banned sites & users
#define CLAN_FILE       "data/clans.json"    // For 'clans'
#define SHUTDOWN_FILE   "data/shutdown.txt"  // For 'shutdown'
#define HOTREBOOT_FILE  "data/hotreboot.dat" // temporary data file used
#define NEWBIE_FILE     "data/newbie.json"   // Newbie help file
#define NOTE_FILE       "data/notes.dat"     // For 'notes'
#define SOCIAL_FILE     "data/social.json"   // For 'socials'

// logs
#define LOG_DIR         "log/"               // Log files
#define AUTH_LOG        "log/auth.log"       // List of who authed who

// players
#define PLAYER_DIR      "player/"            // Player files
#define PLAYERLIST_FILE "player/player.lst"  // Player List

// misc
#define NULL_FILE       "/dev/null"          // To reserve one stream

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
/* act_comm.c */
void add_follower( CHAR_DATA * ch, CHAR_DATA * master );
void stop_follower( CHAR_DATA * ch );
void die_follower( CHAR_DATA * ch, char * name );
bool is_same_group( CHAR_DATA * ach, CHAR_DATA * bch );
bool is_note_to( CHAR_DATA * ch, NOTE_DATA * pnote );

/* act_info.c */
void set_title( CHAR_DATA * ch, char * title );
bool check_blind( CHAR_DATA * ch );
MONEY_DATA * add_money( MONEY_DATA * a, MONEY_DATA * b );
MONEY_DATA * sub_money( MONEY_DATA * a, MONEY_DATA * b );
MONEY_DATA * take_money( CHAR_DATA * ch, int amt, char * type, char * verb );
MONEY_DATA * spend_money( MONEY_DATA * a, MONEY_DATA * b );
char       * money_string( MONEY_DATA * money );

/* act_move.c */
void move_char( CHAR_DATA * ch, int door, bool Fall );

/* act_obj.c */
OBJ_DATA  * random_object( int level );

/* act_wiz.c */
ROOM_INDEX_DATA * find_location( CHAR_DATA * ch, char * arg );
void wiznet( char * string, CHAR_DATA * ch, OBJ_DATA * obj, long flag, long flag_skip, int min_level );
bool doubleexp();

/* comm.c */
void close_socket( DESCRIPTOR_DATA * dclose );
void write_to_buffer( DESCRIPTOR_DATA * d, const char * txt, int length );
void send_to_all_char( const char * text );
void send_to_al( int clr, int level, char * text );
void send_to_char( int AType, const char * txt, CHAR_DATA * ch );
void set_char_color( int AType, CHAR_DATA * ch );
void show_string( DESCRIPTOR_DATA * d, char * input );
void act( int AType, const char * format, CHAR_DATA * ch, const void * arg1, const void * arg2, int type );
int game_main( int argc, char ** argv );

/* db.c */
void boot_db( void );
void area_update( void );
CHAR_DATA *  create_mobile( MOB_INDEX_DATA * pMobIndex );
OBJ_DATA *  create_object( OBJ_INDEX_DATA * pObjIndex, int level );
void clear_char( CHAR_DATA * ch );
void free_char( CHAR_DATA * ch );
char *  get_extra_descr( CHAR_DATA * CH, char * name, EXTRA_DESCR_DATA * ed );
MOB_INDEX_DATA * get_mob_index( int vnum );
OBJ_INDEX_DATA * get_obj_index( int vnum );
ROOM_INDEX_DATA * get_room_index( int vnum );
CLAN_DATA *   get_clan_index( int vnum );
char fread_letter( FILE * fp );
int fread_number( FILE * fp );
char *  fread_string( FILE * fp );
void fread_to_eol( FILE * fp );
char *  fread_word( FILE * fp );
void *  alloc_mem( int sMem );
void *  alloc_perm( int sMem );
void free_mem( void * pMem, int sMem );
char *  str_dup( const char * str );
void free_string( char * pstr );
int number_fuzzy( int number );
int number_range( int from, int to );
int number_percent( void );
int number_door( void );
int number_bits( int width );
int number_mm( void );
int dice( int number, int size );
int interpolate( int level, int value_00, int value_32 );
void smash_tilde( char * str );
bool str_cmp( const char * astr, const char * bstr );
bool str_cmp_ast( const char * astr, const char * bstr );
bool str_prefix( const char * astr, const char * bstr );
bool str_infix( const char * astr, const char * bstr );
bool str_suffix( const char * astr, const char * bstr );
char * a_an( const char * str );
char * capitalize( const char * str );
void append_file( CHAR_DATA * ch, char * file, char * str );
void info( const char * str, int param1, int param2 );
void challenge( const char * str, int param1, int param2 );
void bug( const char * str, int param );
void logch( char * l_str, int l_type, int lvl );
void log_string( char * str, int l_type, int level );
void tail_chain( void );
void clone_mobile( CHAR_DATA * parent, CHAR_DATA * clone );
void clone_object( OBJ_DATA * parent, OBJ_DATA * clone );
void parse_ban( char * argument, BAN_DATA * banned );
void arena_chann( const char * str, int param1, int param2 );

/* devops.c */
void report_issue( const char * title, const char * description, const char * label );
void close_issue( int number );
json_t *  get_issues( const char * label );
size_t curl_callback( void * ptr, size_t size, size_t nmemb, struct curl_data * data );
char *    curl_get( const char * url );

/* fight.c */
void violence_update( void );
void multi_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt );
void damage( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt );
void update_pos( CHAR_DATA * victim );
void stop_fighting( CHAR_DATA * ch, bool fBoth );
void raw_kill( CHAR_DATA * ch, CHAR_DATA * victim );
void death_cry( CHAR_DATA * ch );
bool is_safe( CHAR_DATA * ch, CHAR_DATA * victim );

/* handler.c */
int get_trust( CHAR_DATA * ch );
int get_age( CHAR_DATA * ch );
int get_curr_str( CHAR_DATA * ch );
int get_curr_int( CHAR_DATA * ch );
int get_curr_wis( CHAR_DATA * ch );
int get_curr_dex( CHAR_DATA * ch );
int get_curr_con( CHAR_DATA * ch );
int get_curr_cha( CHAR_DATA * ch );
int can_carry_n( CHAR_DATA * ch );
int can_carry_w( CHAR_DATA * ch );
int xp_tolvl( CHAR_DATA * ch );
bool is_direction( char * str );
int get_direction( char * str );
bool is_name( CHAR_DATA * ch, char * str, char * namelist );
bool is_exact_name( char * str, char * namelist );
void affect_to_char( CHAR_DATA * ch, AFFECT_DATA * paf );
void affect_to_char2( CHAR_DATA * ch, AFFECT_DATA * paf );
void affect_remove( CHAR_DATA * ch, AFFECT_DATA * paf );
void affect_remove2( CHAR_DATA * ch, AFFECT_DATA * paf );
void affect_strip( CHAR_DATA * ch, int sn );
bool is_affected( CHAR_DATA * ch, int sn );
void affect_join( CHAR_DATA * ch, AFFECT_DATA * paf );
void affect_join2( CHAR_DATA * ch, AFFECT_DATA * paf );
void char_from_room( CHAR_DATA * ch );
void char_to_room( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex );
void obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch );
void obj_to_storage( OBJ_DATA * obj, CHAR_DATA * ch );
void obj_from_char( OBJ_DATA * obj );
void obj_from_storage( OBJ_DATA * obj );
int apply_ac( OBJ_DATA * obj, int iWear );
OBJ_DATA *  get_eq_char( CHAR_DATA * ch, int iWear );
void equip_char( CHAR_DATA * ch, OBJ_DATA * obj, int iWear );
void unequip_char( CHAR_DATA * ch, OBJ_DATA * obj );
int count_obj_list( OBJ_INDEX_DATA * obj, OBJ_DATA * list );
void obj_from_room( OBJ_DATA * obj );
void obj_to_room( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex );
void obj_to_obj( OBJ_DATA * obj, OBJ_DATA * obj_to );
void obj_from_obj( OBJ_DATA * obj );
void extract_obj( OBJ_DATA * obj );
void extract_char( CHAR_DATA * ch, bool fPull );
CHAR_DATA * get_char_room( CHAR_DATA * ch, char * argument );
CHAR_DATA * get_char_world( CHAR_DATA * ch, char * argument );
CHAR_DATA * get_pc_world( CHAR_DATA * ch, char * argument );
OBJ_DATA * get_obj_type( OBJ_INDEX_DATA * pObjIndexData );
OBJ_DATA * get_obj_list( CHAR_DATA * ch, char * argument, OBJ_DATA * list );
OBJ_DATA * get_obj_carry( CHAR_DATA * ch, char * argument );
OBJ_DATA * get_obj_storage( CHAR_DATA * ch, char * argument );
OBJ_DATA * get_obj_wear( CHAR_DATA * ch, char * argument );
OBJ_DATA * get_obj_here( CHAR_DATA * ch, char * argument );
OBJ_DATA * get_obj_world( CHAR_DATA * ch, char * argument );
OBJ_DATA * create_money( MONEY_DATA * amount );
int get_obj_number( OBJ_DATA * obj );
int get_obj_weight( OBJ_DATA * obj );
bool room_is_dark( ROOM_INDEX_DATA * pRoomIndex );
bool room_is_private( ROOM_INDEX_DATA * pRoomIndex );
bool can_see( CHAR_DATA * ch, CHAR_DATA * victim );
bool can_see_obj( CHAR_DATA * ch, OBJ_DATA * obj );
bool can_drop_obj( CHAR_DATA * ch, OBJ_DATA * obj );
char * item_type_name( OBJ_DATA * obj );
char * affect_loc_name( int location );
char * affect_bit_name( int vector );
char * affect_bit_name2( int vector );
char * extra_bit_name( int extra_flags );
char * anticlass_bit_name( int anti_class_flags );
char * antirace_bit_name( int anti_race_flags );
char * act_bit_name( int act );
char * imm_bit_name( int );
CHAR_DATA * get_char( CHAR_DATA * ch );
bool longstring( CHAR_DATA * ch, char * argument );
void end_of_game( void );
long wiznet_lookup( const char * name );
int strlen_wo_col( char * argument );
char * strip_color( char * argument );
bool can_use_skpell( CHAR_DATA * ch, int sn );
bool can_practice_skpell( CHAR_DATA * ch, int sn );
bool is_class( CHAR_DATA * ch, int class );
bool has_spells( CHAR_DATA * ch );
int prime_class( CHAR_DATA * ch );
int number_classes( CHAR_DATA * ch );
char * class_long( CHAR_DATA * ch );
char * class_numbers( CHAR_DATA * ch, bool pSave );
char * class_short( CHAR_DATA * ch );
bool gets_zapped( CHAR_DATA * ch, OBJ_DATA * obj );
void raffect_to_room( ROOM_INDEX_DATA * room, CHAR_DATA * ch, ROOM_AFFECT_DATA * raf );
void raffect_remove( ROOM_INDEX_DATA * room, CHAR_DATA * ch, ROOM_AFFECT_DATA * raf );
void raffect_remall( CHAR_DATA * ch );
bool is_raffected( ROOM_INDEX_DATA * room, int sn );
void toggle_raffects( ROOM_INDEX_DATA * room );
void loc_off_raf( CHAR_DATA * ch, int type, bool rOff );
void interpret( CHAR_DATA * ch, char * argument );
bool is_number( char * arg );
int number_argument( char * argument, char * arg );
char * one_argument( char * argument, char * arg_first );
char * visible_name( CHAR_DATA * ch, CHAR_DATA * looker, bool show_hooded );

/* magic.c */
int slot_lookup( int slot );
bool is_sn( int sn );
int skill_lookup( const char * name );
bool saves_spell( int level, CHAR_DATA * victim );
void obj_cast_spell( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj );
void update_skpell( CHAR_DATA * ch, int sn );
void do_acspell( CHAR_DATA * ch, OBJ_DATA * pObj, char * argument );

/* save.c */
bool pstat( char * name );
void save_char_obj( CHAR_DATA * ch );
bool load_char_obj( DESCRIPTOR_DATA * d, char * name );
void corpse_back( CHAR_DATA * ch, OBJ_DATA * corpse );
void read_finger( CHAR_DATA * ch, char * argument );
void save_finger( CHAR_DATA * ch );
void fwrite_finger( CHAR_DATA * ch, FILE * fp );
void fread_finger( CHAR_DATA * ch, FILE * fp, char * name );
void save_banlist( BAN_DATA * ban_list );

/* special.c */
SPEC_FUN *  spec_lookup( const char * name );

/* update.c */
void advance_level( CHAR_DATA * ch );
void gain_exp( CHAR_DATA * ch, int gain );
void gain_condition( CHAR_DATA * ch, int iCond, int value );
void update_handler( void );
CHAR_DATA * rand_figment( CHAR_DATA * ch );
MOB_INDEX_DATA * rand_figment_mob( CHAR_DATA * ch );
OBJ_INDEX_DATA * rand_figment_obj( CHAR_DATA * ch );

/* mob_prog.c */
void mprog_wordlist_check( char * arg, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void * vo, int type );
void mprog_percent_check( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void * vo, int type );
void mprog_act_trigger( char * buf, CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj, void * vo );
void mprog_bribe_trigger( CHAR_DATA * mob, CHAR_DATA * ch, MONEY_DATA * amount );
void mprog_entry_trigger( CHAR_DATA * mob );
void mprog_give_trigger( CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj );
void mprog_greet_trigger( CHAR_DATA * mob );
void mprog_fight_trigger( CHAR_DATA * mob, CHAR_DATA * ch );
void mprog_hitprcnt_trigger( CHAR_DATA * mob, CHAR_DATA * ch );
void mprog_death_trigger( CHAR_DATA * mob, CHAR_DATA * ch );
void mprog_random_trigger( CHAR_DATA * mob );
void mprog_speech_trigger( char * txt, CHAR_DATA * mob );

/*
 * Lotsa triggers for ore_progs.. (ore_prog.c)
 */
/*
 * Object triggers
 */
void oprog_get_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_get_from_trigger( OBJ_DATA * obj, CHAR_DATA * ch, OBJ_DATA * secondary );
void oprog_give_trigger( OBJ_DATA * obj, CHAR_DATA * ch, CHAR_DATA * victim );
void oprog_drop_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_put_trigger( OBJ_DATA * obj, CHAR_DATA * ch, OBJ_DATA * secondary );
void oprog_fill_trigger( OBJ_DATA * obj, CHAR_DATA * ch, OBJ_DATA * spring );
void oprog_wear_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_look_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_look_in_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_invoke_trigger( OBJ_DATA * obj, CHAR_DATA * ch, void * vo );
void oprog_use_trigger( OBJ_DATA * obj, CHAR_DATA * ch, void * vo );
void oprog_cast_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_cast_sn_trigger( OBJ_DATA * obj, CHAR_DATA * ch, int sn, void * vo );
void oprog_join_trigger( OBJ_DATA * obj, CHAR_DATA * ch, OBJ_DATA * secondary );
void oprog_separate_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_buy_trigger( OBJ_DATA * obj, CHAR_DATA * ch, CHAR_DATA * vendor );
void oprog_sell_trigger( OBJ_DATA * obj, CHAR_DATA * ch, CHAR_DATA * vendor );
void oprog_store_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_retrieve_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_open_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_close_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_lock_trigger( OBJ_DATA * obj, CHAR_DATA * ch, OBJ_DATA * key );
void oprog_unlock_trigger( OBJ_DATA * obj, CHAR_DATA * ch, OBJ_DATA * key );
void oprog_pick_trigger( OBJ_DATA * obj, CHAR_DATA * ch );
void oprog_random_trigger( OBJ_DATA * obj );
void oprog_throw_trigger( OBJ_DATA * obj, CHAR_DATA * ch );

/*
 * Room triggers
 */
void rprog_enter_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_exit_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_pass_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_cast_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_cast_sn_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch, int sn, void * vo );
void rprog_sleep_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_wake_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_rest_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_death_trigger( ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void rprog_time_trigger( ROOM_INDEX_DATA * room, int hour );
void rprog_random_trigger( ROOM_INDEX_DATA * room );

/*
 * Exit triggers
 */
void eprog_enter_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void eprog_exit_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void eprog_pass_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch, bool fEnter );
void eprog_look_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void eprog_scry_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void eprog_open_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void eprog_close_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch );
void eprog_lock_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch, OBJ_DATA * obj );
void eprog_unlock_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch, OBJ_DATA * obj );
void eprog_pick_trigger( EXIT_DATA * pExit, ROOM_INDEX_DATA * room, CHAR_DATA * ch );

/*
 * gr_magic.c
 */
void check_gcast( CHAR_DATA * ch );
void group_cast( int sn, int level, CHAR_DATA * ch, char * argument );
void set_gspell( CHAR_DATA * ch, GSPELL_DATA * gsp );
void end_gspell( CHAR_DATA * ch );

/*
 * quest.c
 */
bool chance( int num );

/*
 * track.c
 */
void hunt_victim( CHAR_DATA * ch );
bool can_go( CHAR_DATA * ch, int dir );

/*
 * chatmode.c
 */
void start_chat_mode( DESCRIPTOR_DATA * d );
void chat_interp( CHAR_DATA * ch, char * argument );


/*****************************************************************************
*                                    OLC                                    *
*****************************************************************************/

/*
 * This structure is used in special.c to lookup spec funcs and
 * also in olc_act.c to display listings of spec funcs.
 */
struct spec_type {
  char     * spec_name;
  SPEC_FUN * spec_fun;
};

/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type {
  char * name;
  int    bit;
  bool   settable;
};

/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY 1

/*
 * Area flags.
 */
#define AREA_NONE      0
#define AREA_CHANGED   1   // Area has been modified.
#define AREA_ADDED     2   // Area has been added to.
#define AREA_LOADING   4   // Used for counting in db.c
#define AREA_VERBOSE   8   // Used for saving in save.c
#define AREA_PROTOTYPE 16  // Prototype area(no mortals)
#define AREA_CLAN_HQ   32  // Area is a CLAN HQ
#define AREA_NO_QUEST  64  // No quests allowed
#define AREA_MUDSCHOOL 128 // Used for mudschool only

#define NO_FLAG -99        // Must not be used in flags or stats.

/*
 * Interp.c
 */
DECLARE_DO_FUN( do_aedit );
DECLARE_DO_FUN( do_redit );
DECLARE_DO_FUN( do_oedit );
DECLARE_DO_FUN( do_medit );
DECLARE_DO_FUN( do_cedit );
DECLARE_DO_FUN( do_hedit );
DECLARE_DO_FUN( do_sedit );
DECLARE_DO_FUN( do_rename_obj );
DECLARE_DO_FUN( do_mreset );
DECLARE_DO_FUN( do_nedit );
DECLARE_DO_FUN( do_asave );
DECLARE_DO_FUN( do_alist );
DECLARE_DO_FUN( do_resets );
DECLARE_DO_FUN( do_alias );
DECLARE_DO_FUN( do_clear );

/*
 * Global Constants
 */
extern const struct  spec_type spec_table  [];

/*
 * Global variables
 */
extern AREA_DATA * area_first;
extern AREA_DATA * area_last;
extern CLAN_DATA * clan_first;
extern SHOP_DATA * shop_last;

extern int top_affect;
extern int top_area;
extern int top_ed;
extern int top_exit;
extern int top_help;
extern int top_mob_index;
extern int mobs_in_game;
extern int top_obj_index;
extern int top_reset;
extern int top_room;
extern int top_shop;
extern int top_clan;
extern int top_social;
extern int top_race;
extern int top_newbie;
extern int top_trap;
extern int top_mob_prog;

extern char str_empty[ 1 ];

extern MOB_INDEX_DATA  * mob_index_hash[ MAX_KEY_HASH ];
extern OBJ_INDEX_DATA  * obj_index_hash[ MAX_KEY_HASH ];
extern ROOM_INDEX_DATA * room_index_hash[ MAX_KEY_HASH ];

/* db.c */
void reset_area( AREA_DATA * pArea );
void reset_room( ROOM_INDEX_DATA * pRoom );
void newbie_sort( NEWBIE_DATA * pNewbie );

/* string.c */
void string_edit( CHAR_DATA * ch, char ** pString );
void string_append( CHAR_DATA * ch, char ** pString );
char * string_replace( char * orig, char * old, char * new );
void string_add( CHAR_DATA * ch, char * argument );
char * format_string( char * oldstring );
char * first_arg( char * argument, char * arg_first, bool fCase );
char * string_unpad( char * argument );
char * string_proper( char * argument );
char * all_capitalize( const char * str );
char * string_delline( CHAR_DATA * ch, char * argument, char * old );
char * string_insline( CHAR_DATA * ch, char * argument, char * old );

/* mem.c */
RESET_DATA * new_reset_data( void );
void free_reset_data( RESET_DATA * pReset );
AREA_DATA * new_area( void );
EXIT_DATA * new_exit( void );
void free_exit( EXIT_DATA * pExit );
EXTRA_DESCR_DATA * new_extra_descr( void );
void free_extra_descr( EXTRA_DESCR_DATA * pExtra );
ROOM_INDEX_DATA * new_room_index( void );
void free_room_index( ROOM_INDEX_DATA * pRoom );
AFFECT_DATA * new_affect( void );
void free_affect( AFFECT_DATA * pAf );
SHOP_DATA * new_shop( void );
void free_shop( SHOP_DATA * pShop );
OBJ_INDEX_DATA * new_obj_index( void );
void free_obj_index( OBJ_INDEX_DATA * pObj );
MOB_INDEX_DATA * new_mob_index( void );
void free_mob_index( MOB_INDEX_DATA * pMob );
CLAN_DATA * new_clan_index( void );
void free_clan_index( CLAN_DATA * pClan );
MPROG_DATA * new_mprog_data( void );
void free_mprog_data( MPROG_DATA * pMProg );
TRAP_DATA * new_trap_data( void );
void free_trap_data( TRAP_DATA * pTrap );
SOCIAL_DATA * new_social_index( void );
void free_social_index( SOCIAL_DATA * pSocial );
NEWBIE_DATA * new_newbie_index( void );

/* olc.c */
bool run_olc_editor( DESCRIPTOR_DATA * d );
char * olc_ed_name( CHAR_DATA * ch );
char * olc_ed_vnum( CHAR_DATA * ch );
AREA_DATA * get_area_data( int vnum );
void purge_area( AREA_DATA * pArea );

/* special.c */
char * spec_string( SPEC_FUN * fun );

/* bit.c */
bool is_stat( const struct flag_type * flag_table );
int flag_lookup( const char * name, const struct flag_type * flag_table );
int flag_value( const struct flag_type * flag_table, char * argument );
char * flag_string( const struct flag_type * flag_table, int bits );

extern const struct flag_type area_flags[];
extern const struct flag_type sex_flags[];
extern const struct flag_type exit_flags[];
extern const struct flag_type door_resets[];
extern const struct flag_type room_flags[];
extern const struct flag_type sector_flags[];
extern const struct flag_type type_flags[];
extern const struct flag_type extra_flags[];
extern const struct flag_type anti_race_flags[];
extern const struct flag_type anti_class_flags[];
extern const struct flag_type wear_flags[];
extern const struct flag_type act_flags[];
extern const struct flag_type affect_flags[];
extern const struct flag_type affect2_flags[];
extern const struct flag_type apply_flags[];
extern const struct flag_type wear_loc_strings[];
extern const struct flag_type wear_loc_flags[];
extern const struct flag_type weapon_flags[];
extern const struct flag_type container_flags[];
extern const struct flag_type liquid_flags[];
extern const struct flag_type immune_flags[];
extern const struct flag_type mprog_types[];
extern const struct flag_type oprog_types[];
extern const struct flag_type rprog_types[];
extern const struct flag_type eprog_types[];

/* olc_act.c */
extern AFFECT_DATA * new_affect( void );
extern OBJ_INDEX_DATA * new_obj_index( void );
extern void free_affect( AFFECT_DATA * pAf );
extern EXTRA_DESCR_DATA * new_extra_descr( void );
extern void free_extra_descr( EXTRA_DESCR_DATA * pExtra );
extern void free_obj_data( OBJ_DATA * pObj );
extern OBJ_DATA * new_obj_data( void );
extern RESET_DATA * new_reset_data( void );
extern void check_nofloor( CHAR_DATA * ch );
extern void save_clans();
extern void save_social();
extern void save_helps();
extern void wind_update( AREA_DATA * pArea );
extern void send_to_area( AREA_DATA * pArea, char * txt );
extern void free_alias( ALIAS_DATA * pAl );
extern void save_player_list();
extern void save_newbie();
