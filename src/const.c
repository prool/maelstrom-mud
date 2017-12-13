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

#define unix 1
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

const struct direction_type direction_table[ MAX_DIR ] = {
//{ "name",      "noun",          "navigation",       "descriptive",       "abbr", "blood",     reverse       }
  { "north",     "the north",     "to the north",     "northern",          "n",    "north",     DIR_SOUTH     },
  { "east",      "the east",      "to the east",      "eastern",           "e",    "east",      DIR_WEST      },
  { "south",     "the south",     "to the south",     "southern",          "s",    "south",     DIR_NORTH     },
  { "west",      "the west",      "to the west",      "western",           "w",    "west",      DIR_EAST      },
  { "up",        "above",         "above",            "roof",              "u",    "above",     DIR_DOWN      },
  { "down",      "below",         "below",            "ground",            "d",    "below",     DIR_UP        },
  { "northwest", "the northwest", "to the northwest", "northwestern",      "nw",   "northwest", DIR_SOUTHEAST },
  { "northeast", "the northeast", "to the northeast", "northeastern",      "ne",   "northeast", DIR_SOUTHWEST },
  { "southwest", "the southwest", "to the southwest", "southwestern",      "sw",   "southwest", DIR_NORTHEAST },
  { "southeast", "the southeast", "to the southeast", "southeastern",      "se",   "southeast", DIR_NORTHWEST }
};

const struct race_type race_table[ MAX_RACE ] = {
//{ "Sml", "Full Name", mstr, mint, mwis, mdex, mcon, mcha, age, size        },
  { "Hum", "Human",     0,    0,    0,    0,    0,    0,    15,  SIZE_MEDIUM },
  { "Elf", "Elf",       0,    2,    0,    2,    -2,   0,    110, SIZE_MEDIUM },
  { "Dwa", "Dwarf",     0,    0,    2,    0,    2,    -2,   40,  SIZE_MEDIUM },
  { "Gno", "Gnome",     -2,   0,    0,    0,    2,    2,    40,  SIZE_SMALL  },
  { "Hlf", "Halfling",  -2,   0,    0,    2,    0,    2,    20,  SIZE_SMALL  }
};

const struct class_type class_table[ MAX_CLASS ] = {
//{ "Who", "Long",    attr_prime, skill_adept,  mbab, hitdice, d6gold, spellcaster, "whotype"      }
  { "Cas", "Caster",  APPLY_INT,  95,           0.5,  6,       2,      TRUE,        "&BARCH MAGUS" },
  { "Rog", "Rogue",   APPLY_DEX,  85,           0.75, 8,       4,      FALSE,       "&zBLACKGUARD" },
  { "Fig", "Fighter", APPLY_STR,  85,           1,    10,      5,      FALSE,       "&wKNIGHT"     },
};

const struct size_type size_table[ MAX_SIZE ] = {
//{        "size", mac, mstealth,  mcarry }
  {        "Fine", -8, 16,         0.125  },
  {  "Dimunitive", -4, 12,         0.25   },
  {        "Tiny", -2, 8,          0.5    },
  {       "Small", -1, 4,          0.75   },
  {      "Medium", 0,  0,          1      },
  {       "Large", 1,  -4,         2      },
  {        "Huge", 2,  -8,         4      },
  {  "Gargantuan", 4,  -12,        8      },
  {    "Colossal", 8,  -16,        16     }
};

/*
 * Wiznet table and prototype for future flag setting
 */
const struct wiznet_type wiznet_table[] = {
  { "on",        WIZ_ON,        LEVEL_HERO     },
  { "prefix",    WIZ_PREFIX,    LEVEL_HERO     },
  { "ticks",     WIZ_TICKS,     LEVEL_IMMORTAL },
  { "general",   WIZ_GENERAL,   LEVEL_HERO     },
  { "logins",    WIZ_LOGINS,    LEVEL_HERO     },
  { "sites",     WIZ_SITES,     LEVEL_IMMORTAL },
  { "links",     WIZ_LINKS,     LEVEL_IMMORTAL },
  { "newbies",   WIZ_NEWBIE,    LEVEL_HERO     },
  { "spam",      WIZ_SPAM,      LEVEL_IMMORTAL },
  { "deaths",    WIZ_DEATHS,    LEVEL_HERO     },
  { "resets",    WIZ_RESETS,    LEVEL_IMMORTAL },
  { "mobdeaths", WIZ_MOBDEATHS, LEVEL_IMMORTAL },
  { "flags",     WIZ_FLAGS,     LEVEL_IMMORTAL },
  { "penalties", WIZ_PENALTIES, L_GOD          },
  { "saccing",   WIZ_SACCING,   L_CON          },
  { "levels",    WIZ_LEVELS,    LEVEL_HERO     },
  { "load",      WIZ_LOAD,      L_CON          },
  { "restore",   WIZ_RESTORE,   L_CON          },
  { "snoops",    WIZ_SNOOPS,    L_CON          },
  { "switches",  WIZ_SWITCHES,  L_SEN          },
  { "secure",    WIZ_SECURE,    L_CON          },
  { "oldlog",    WIZ_OLDLOG,    L_DIR          },
  { NULL,        0,             0              }
};

/*
 * Attribute bonus tables.
 */
const struct  str_app_type str_app[ 31 ] = {
  { -5, -4, 0,    0  }, /* 0  */
  { -5, -4, 3,    1  }, /* 1  */
  { -3, -2, 3,    2  },
  { -3, -1, 10,   3  }, /* 3  */
  { -2, -1, 25,   4  },
  { -2, -1, 55,   5  }, /* 5  */
  { -1, 0,  80,   6  },
  { -1, 0,  90,   7  },
  {  0, 0,  100,  8  },
  {  0, 0,  100,  9  },
  {  0, 0,  115,  10 }, /* 10  */
  {  0, 0,  115,  11 },
  {  0, 0,  140,  12 },
  {  0, 0,  140,  13 }, /* 13  */
  {  0, 1,  170,  14 },
  {  1, 1,  170,  15 }, /* 15  */
  {  1, 2,  195,  16 },
  {  2, 3,  220,  22 },
  {  2, 4,  250,  25 }, /* 18  */
  {  3, 5,  400,  30 },
  {  3, 6,  500,  35 }, /* 20  */
  {  4, 7,  600,  40 },
  {  5, 7,  700,  45 },
  {  6, 8,  800,  50 },
  {  8, 10, 900,  55 },
  { 10, 12, 999,  60 }, /* 25   */
  { 12, 14, 999,  60 },
  { 14, 16, 999,  60 },
  { 16, 18, 999,  60 }, /* 28 */
  { 18, 20, 999,  60 },
  { 20, 22, 999,  60 } /* 30   */

};

const struct  int_app_type int_app[ 31 ] = {
  {  3 }, /*  0 */
  {  5 }, /*  1 */
  {  7 },
  {  8 }, /*  3 */
  {  9 },
  { 10 }, /*  5 */
  { 11 },
  { 12 },
  { 13 },
  { 15 },
  { 17 }, /* 10 */
  { 19 },
  { 22 },
  { 25 },
  { 28 },
  { 31 }, /* 15 */
  { 34 },
  { 37 },
  { 40 }, /* 18 */
  { 44 },
  { 49 }, /* 20 */
  { 55 },
  { 60 },
  { 70 },
  { 85 },
  { 99 }, /* 25 */
  { 99 },
  { 99 },
  { 99 },
  { 99 },
  { 99 }  /* 30 */
};

const struct  wis_app_type wis_app[ 31 ] = {
  { 0 },  /*  0 */
  { 0 },  /*  1 */
  { 0 },
  { 0 },  /*  3 */
  { 0 },
  { 1 },  /*  5 */
  { 1 },
  { 1 },
  { 1 },
  { 2 },
  { 2 },  /* 10 */
  { 2 },
  { 2 },
  { 2 },
  { 2 },
  { 3 },  /* 15 */
  { 3 },
  { 4 },
  { 4 },  /* 18 */
  { 5 },
  { 5 },  /* 20 */
  { 6 },
  { 7 },
  { 7 },
  { 7 },
  { 8 },  /* 25 */
  { 8 },
  { 8 },
  { 9 },
  { 9 },
  { 10}   /* 30 */
};

const struct  dex_app_type dex_app[ 31 ] = {
  { 60     }, /* 0 */
  { 50     }, /* 1 */
  { 50     },
  { 40     },
  { 30     },
  { 20     }, /* 5 */
  { 10     },
  { 0      },
  { 0      },
  { 0      },
  { 0      }, /* 10 */
  { 0      },
  { 0      },
  { 0      },
  { 0      },
  { -10    }, /* 15 */
  { -15    },
  { -20    },
  { -30    },
  { -40    },
  { -50    }, /* 20 */
  { -65    },
  { -75    },
  { -90    },
  { -100   },
  { -120   },  /* 25 */
  { -140   },
  { -160   },
  { -180   },
  { -200   },
  { -220   }  /* 30 */
};

const struct  con_app_type con_app[ 31 ] = {
  { -4, 20 }, /*  0 */
  { -3, 25 }, /*  1 */
  { -2, 30 },
  { -2, 35 }, /*  3 */
  { -1, 40 },
  { -1, 45 }, /*  5 */
  { -1, 50 },
  {  0, 55 },
  {  0, 60 },
  {  0, 65 },
  {  0, 70 }, /* 10 */
  {  0, 75 },
  {  0, 80 },
  {  0, 85 },
  {  0, 88 },
  {  1, 90 }, /* 15 */
  {  2, 95 },
  {  2, 97 },
  {  3, 99 }, /* 18 */
  {  3, 99 },
  {  4, 99 }, /* 20 */
  {  4, 99 },
  {  5, 99 },
  {  6, 99 },
  {  7, 99 },
  {  8, 99 }, /* 25 */
  {  8, 99 },
  {  9, 99 },
  {  9, 99 },
  {  10, 99},
  {  10, 99}  /* 30 */
};

/*
 * Liquid properties.
 * Used in world.obj.
 */
const struct  liq_type liq_table[ LIQ_MAX ] = {
  { "water",           "clear",     {  0, 0, 10 } }, /*  0 */
  { "beer",            "amber",     {  3, 2, 5  } },
  { "wine",            "rose",      {  4, 2, 5  } },
  { "ale",             "brown",     {  2, 2, 5  } },
  { "dark ale",        "dark",      {  1, 2, 5  } },
  { "whisky",          "golden",    {  8, 1, 4  } }, /*  5 */
  { "lemonade",        "pink",      {  0, 1, 8  } },
  { "firebreather",    "boiling",   { 10, 0, 0  } },
  { "local specialty", "everclear", {  3, 3, 3  } },
  { "slime mold juice", "green",    {  0, 4, -8 } },
  { "milk",            "white",     {  0, 3, 6  } }, /* 10 */
  { "tea",             "tan",       {  0, 1, 6  } },
  { "coffee",          "black",     {  0, 1, 6  } },
  { "blood",           "red",       {  0, 2, -1 } },
  { "salt water",      "clear",     {  0, 1, -2 } },
  { "cola",            "cherry",    {  0, 1, 5  } } /* 15 */
};

#define SLOT(s) s
const struct  skill_type skill_table[ MAX_SKILL ] = {

  /*
   * Magic spells.
   */

  /*
     {
     "Name",
     {CAS,ROG,FIG}
     function,    target type,    position,
     gsn,     min mana, wait,
     "damage name",   "off name", "room see spell ends",
     dispelable, SLOT(slot)
     },
   */

  {
    "reserved",
    { 999,999,999   },
    0,TAR_IGNORE,POS_STANDING,
    NULL,0,0,
    "","","",FALSE,SLOT(0)
  },

  {
    "animate dead",
    { L_APP,L_APP,L_APP },
    spell_animate,TAR_OBJ_INV,POS_STANDING,
    NULL,100,24,
    "","!Animate Dead!","",FALSE,SLOT(0)
  },

  {
    "armor",
    { L_APP,L_APP,L_APP },
    spell_armor,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,5,12,
    "",
    "You feel less protected.",
    "$n looks less protected.",
    TRUE,SLOT(1)
  },

  {
    "astral walk",
    { L_APP,L_APP,L_APP },
    spell_astral,TAR_IGNORE,POS_STANDING,
    NULL,50,12,
    "","!Astral Walk!","",FALSE,SLOT(0)
  },

  {
    "blindness",
    { L_APP,L_APP,L_APP },
    spell_blindness,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_blindness,5,12,
    "",
    "You can see again.",
    "$n is no longer blinded",
    TRUE,SLOT(3)
  },

  {
    "change sex",
    { L_APP,L_APP,L_APP },
    spell_change_sex,TAR_CHAR_DEFENSIVE,POS_FIGHTING,
    NULL,15,12,
    "",
    "Your body feels familiar again.",
    "$n looks more like $mself again.",
    TRUE,SLOT(4)
  },

  {
    "charm person",
    { L_APP,L_APP,L_APP },
    spell_charm_person,TAR_CHAR_OFFENSIVE,POS_STANDING,
    &gsn_charm_person,5,12,
    "",
    "You feel more self-confident.",
    "$n regains $s free will.",
    TRUE,SLOT(5)
  },

  {
    "continual light",
    { L_APP,L_APP,L_APP },
    spell_continual_light,TAR_IGNORE,POS_STANDING,
    NULL,7,12,
    "","!Continual Light!","",FALSE,SLOT(0)
  },

  {
    "control weather",
    { L_APP,L_APP,L_APP },
    spell_control_weather,TAR_IGNORE,POS_STANDING,
    NULL,25,12,
    "","!Control Weather!","",FALSE,SLOT(0)
  },

  {
    "create food",
    { L_APP,L_APP,L_APP },
    spell_create_food,TAR_IGNORE,POS_STANDING,
    NULL,5,12,
    "","!Create Food!","",FALSE,SLOT(0)
  },

  {
    "create water",
    { L_APP,L_APP,L_APP },
    spell_create_water,TAR_OBJ_INV,POS_STANDING,
    NULL,5,12,
    "","!Create Water!","",FALSE,SLOT(0)
  },

  {
    "cure blindness",
    { L_APP,L_APP,L_APP },
    spell_cure_blindness,TAR_CHAR_DEFENSIVE,POS_FIGHTING,
    NULL,5,12,
    "","!Cure Blindness!","",FALSE,SLOT(0)
  },

  {
    "cure poison",
    { L_APP,L_APP,L_APP },
    spell_cure_poison,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,5,12,
    "","!Cure Poison!","",FALSE,SLOT(0)
  },

  {
    "dispel magic",
    { L_APP,L_APP,L_APP },
    spell_dispel_magic,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,15,16,
    "","!Dispel Magic!","",FALSE,SLOT(0)
  },

  {
    "enchant weapon",
    { L_APP,L_APP,L_APP },
    spell_enchant_weapon,TAR_OBJ_INV,POS_STANDING,
    NULL,100,24,
    "","!Enchant Weapon!","",FALSE,SLOT(0)
  },

  {
    "energy drain",
    { L_APP,L_APP,L_APP },
    spell_energy_drain,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,35,12,
    "&zene&wrgy d&Wrain&X","!Energy Drain!","",FALSE,SLOT(0)
  },

  {
    "flamestrike",
    { L_APP,L_APP,L_APP },
    spell_flamestrike,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,20,12,
    "&rflame&Rstrike","!Flamestrike!","",FALSE,SLOT(0)
  },

  {
    "fly",
    { L_APP,L_APP,L_APP },
    spell_fly,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,10,18,
    "",
    "You slowly float to the ground.",
    "$n falls to the ground!",
    TRUE,SLOT(15)
  },

  {
    "gate",
    { L_APP,L_APP,L_APP },
    spell_gate,TAR_CHAR_DEFENSIVE,POS_FIGHTING,
    NULL,50,12,
    "","!Gate!","",FALSE,SLOT(0)
  },

  {
    "giant strength",
    { L_APP,L_APP,L_APP },
    spell_giant_strength,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,20,12,
    "",
    "You feel weaker.",
    "$n no longer looks so mighty.",TRUE,SLOT(16)
  },

  {
    "eternal intellect",
    { L_APP,L_APP,L_APP },
    spell_eternal_intellect,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,20,12,
    "",
    "You mind surges with a superior prowess.",
    "$n looks a bit duller.",TRUE,SLOT(16)
  },

  {
    "goodberry",
    { L_APP,L_APP,L_APP },
    spell_goodberry,TAR_OBJ_INV,POS_STANDING,
    NULL,25,8,
    "","!GOODBERRY!","",FALSE,SLOT(0)
  },

  {
    "harm",
    { L_APP,L_APP,L_APP },
    spell_harm,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,35,12,
    "&zharm &wspell&X","!Harm!","",FALSE,SLOT(0)
  },

  {
    "heal",
    { L_APP,L_APP,L_APP },
    spell_heal,TAR_CHAR_DEFENSIVE,POS_FIGHTING,
    NULL,50,12,
    "","!Heal!","",FALSE,SLOT(0)
  },

  {
    "icestorm",
    { L_APP,L_APP,L_APP },
    spell_icestorm,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,25,8,
    "&Cicy blast&X","!ICESTORM!","",FALSE,SLOT(0)
  },

  {
    "infravision",
    { L_APP,L_APP,L_APP },
    spell_infravision,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,5,18,
    "","You no longer see in the dark.","",
    TRUE,SLOT(19)
  },

  {
    "incinerate",
    { L_APP,L_APP,L_APP },
    spell_incinerate,TAR_CHAR_OFFENSIVE,POS_STANDING,
    &gsn_incinerate,30,12,
    "&rflames&X","The flames have been extinguished.",
    "The flames around $n have been extinguished.",TRUE,SLOT(20)
  },

  {
    "invis",
    { L_APP,L_APP,L_APP },
    spell_invis,TAR_CHAR_DEFENSIVE,POS_STANDING,
    &gsn_invis,5,12,
    "",
    "You are no longer invisible.",
    "$n fades into existance.",
    TRUE,SLOT(21)
  },

  {
    "know alignment",
    { L_APP,L_APP,L_APP },
    spell_know_alignment,TAR_CHAR_DEFENSIVE,POS_FIGHTING,
    NULL,9,12,
    "","!Know Alignment!","",FALSE,SLOT(0)
  },

  {
    "lightning bolt",
    { L_APP,L_APP,L_APP },
    spell_lightning_bolt,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,15,12,
    "&Blightning bolt&X","!Lightning Bolt!","",FALSE,SLOT(0)
  },

  {
    "locate object",
    { L_APP,L_APP,L_APP },
    spell_locate_object,TAR_IGNORE,POS_STANDING,
    NULL,20,18,
    "","!Locate Object!","",FALSE,SLOT(0)
  },

  {
    "magic missile",
    { L_APP,L_APP,L_APP },
    spell_magic_missile,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,15,12,
    "&Pmag&pic mis&Psile","!Magic Missile!","",FALSE,SLOT(0)
  },

  {
    "mana",
    { L_APP,L_APP,L_APP },
    spell_mana,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,0,0,
    "","!Mana!","",FALSE,SLOT(0)
  },

  {
    "mass invis",
    { L_APP,L_APP,L_APP },
    spell_mass_invis,TAR_IGNORE,POS_STANDING,
    &gsn_mass_invis,20,24,
    "",
    "!Mass Invis!",
    "$n fades into existance.",TRUE,SLOT(22)
  },

  {
    "pass door",
    { L_APP,L_APP,L_APP },
    spell_pass_door,TAR_CHAR_SELF,POS_STANDING,
    NULL,20,12,
    "",
    "You feel solid again.",
    "$n becomes soild.",TRUE,SLOT(23)
  },

  {
    "aura of peace",
    { L_APP,L_APP,L_APP },
    spell_aura,TAR_CHAR_SELF,POS_STANDING,
    NULL,100,12,
    "",
    "The peace aura around your body fades.",
    "The peace aura around $n fades.",FALSE,SLOT(24)
  },

  {
    "permenancy",
    { L_APP,L_APP,L_APP },
    spell_permenancy,TAR_OBJ_INV,POS_STANDING,
    NULL,200,24,
    "","!Permenancy!","",FALSE,SLOT(0)
  },

  {
    "poison",
    { L_APP,L_APP,L_APP },
    spell_poison,TAR_CHAR_OFFENSIVE,POS_STANDING,
    &gsn_poison,10,12,
    "&Gp&go&Gi&gs&Go&gn",
    "You feel less sick.",
    "$n looks better.",TRUE,SLOT(25)
  },

  {
    "portal",
    { L_APP,L_APP,L_APP },
    spell_portal,TAR_IGNORE,POS_STANDING,
    NULL,100,12,
    "","","",FALSE,SLOT(0)
  },

  {
    "protection evil",
    { L_APP,L_APP,L_APP },
    spell_protection,TAR_CHAR_SELF,POS_STANDING,
    NULL,5,12,
    "",
    "You feel less protected.",
    "$n looks less protected.",
    TRUE,SLOT(26)
  },

  {
    "refresh",
    { L_APP,L_APP,L_APP },
    spell_refresh,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,12,18,
    "refresh","!Refresh!","",FALSE,SLOT(0)
  },

  {
    "scry",
    { L_APP,L_APP,L_APP },
    spell_scry,TAR_CHAR_SELF,POS_STANDING,
    NULL,35,20,
    "","Your vision returns to normal.",
    "$n's vision returns to normal.",TRUE,SLOT(0)
  },

  {
    "shield",
    { L_APP,L_APP,L_APP },
    spell_shield,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,12,18,
    "",
    "Your force shield shimmers then fades away.",
    "The shield protecting $n shimmers then fades away.",
    TRUE,SLOT(28)
  },

  {
    "sleep",
    { L_APP,L_APP,L_APP },
    spell_sleep,TAR_CHAR_OFFENSIVE,POS_STANDING,
    &gsn_sleep,15,12,
    "","You feel less tired.",
    "$n looks much more refreshed.",TRUE,SLOT(30)
  },

  {
    "spell bind",
    { L_APP,L_APP,L_APP },
    spell_spell_bind,TAR_OBJ_INV,POS_STANDING,
    NULL,100,24,
    "","!Spell Bind!","",FALSE,SLOT(0)
  },

  {
    "summon",
    { L_APP,L_APP,L_APP },
    spell_summon,TAR_IGNORE,POS_STANDING,
    NULL,50,12,
    "","!Summon!","",FALSE,SLOT(0)
  },

  {
    "teleport",
    { L_APP,L_APP,L_APP },
    spell_teleport,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,35,12,
    "","!Teleport!","",FALSE,SLOT(0)
  },

  {
    "word of recall",
    { 25, L_APP, L_APP },
    spell_word_of_recall,TAR_CHAR_SELF,POS_RESTING,
    NULL,5,12,
    "","!Word of Recall!","",FALSE,SLOT(0)
  },

  {
    "summon swarm",
    { L_APP,L_APP,L_APP },
    spell_summon_swarm,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,50,12,
    "","!Summon Insect!","",FALSE,SLOT(0)
  },

  {
    "summon pack",
    { L_APP,L_APP,L_APP },
    spell_summon_pack,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,50,12,
    "","!Summon Pack!","",FALSE,SLOT(0)
  },

  {
    "summon demon",
    { L_APP,L_APP,L_APP },
    spell_summon_demon,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,50,12,
    "","!Summon Demon!","",FALSE,SLOT(0)
  },

  {
    "backstab",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_backstab,0,24,
    "backstab","!Backstab!","",FALSE,SLOT(0)
  },

  {
    "palm",
    { L_APP, 10, L_APP },
    spell_null,TAR_IGNORE,POS_RESTING,
    &gsn_palm,0,0,
    "","!palm","",FALSE,SLOT(0)
  },

  {
    "disarm",
    { L_APP, L_APP, 30 },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_disarm,0,24,
    "","!Disarm!","",FALSE,SLOT(0)
  },

  {
    "dodge",
    { L_APP, 5, 5 },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_dodge,0,0,
    "","!Dodge!","",FALSE,SLOT(0)
  },

  {
    "dual",
    { L_APP, 5, 5 },
    spell_null,TAR_IGNORE,POS_RESTING,
    &gsn_dual,0,0,
    "","!DUAL!","",FALSE,SLOT(0)
  },

  {
    "hide",
    { L_APP, 5, L_APP },
    spell_null,TAR_IGNORE,POS_RESTING,
    &gsn_hide,0,12,
    "",
    "You are no longer so difficult to see.",
    "$n has stopped hiding.",FALSE,SLOT(33)
  },

  {
    "iron grip",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_grip,0,8,
    "",
    "You loosen your hold on your weapon.",
    "$n has loosened $s hold on $s weapon.",FALSE,SLOT(34)
  },

  {
    "kick",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_kick,0,8,
    "kick","!Kick!","",FALSE,SLOT(0)
  },

  {
    "parry",
    { L_APP, L_APP, 10 },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_parry,0,0,
    "","!Parry!","",FALSE,SLOT(0)
  },

  {
    "peek",
    { L_APP, 20, L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_peek,0,0,
    "","!Peek!","",FALSE,SLOT(0)
  },

  {
    "pick lock",
    { L_APP, 15, L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_pick_lock,0,12,
    "","!Pick!","",FALSE,SLOT(0)
  },

  {
    "poison weapon",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_OBJ_INV,POS_STANDING,
    &gsn_poison_weapon,0,12,
    "poisonous concoction","!Poison Weapon!","",FALSE,SLOT(0)
  },

  {
    "punch",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_punch,0,10,
    "punch","!punch!","",FALSE,SLOT(0)
  },

  {
    "rescue",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_rescue,0,12,
    "","!Rescue!","",FALSE,SLOT(0)
  },

  {
    "sneak",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_sneak,0,12,
    "",NULL,"",FALSE,SLOT(35)
  },

  {
    "steal",
    { L_APP, 30, L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_steal,0,24,
    "","!Steal!","",FALSE,SLOT(0)
  },

  {
    "aura sight",
    { L_APP,L_APP,L_APP },
    spell_aura_sight,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,9,12,
    "","!Aura Sight!","",FALSE,SLOT(0)
  },

  {
    "combat mind",
    { L_APP,L_APP,L_APP },
    spell_combat_mind,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,15,12,
    "","Your battle sense has faded.",
    "$n's sense of battle has diminished.",FALSE,SLOT(40)
  },

  {
    "complete healing",
    { L_APP,L_APP,L_APP },
    spell_complete_healing,TAR_CHAR_SELF,POS_STANDING,
    NULL,200,12,
    "","!Complete Healing!","",FALSE,SLOT(0)
  },

  {
    "control flames",
    { L_APP,L_APP,L_APP },
    spell_control_flames,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,15,12,
    "tongue of &Rflame&X","!Control Flames!","",FALSE,SLOT(0)
  },

  {
    "create sound",
    { L_APP,L_APP,L_APP },
    spell_create_sound,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,5,12,
    "","!Create Sound!","",FALSE,SLOT(0)
  },

  {
    "detonate",
    { L_APP,L_APP,L_APP },
    spell_detonate,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,35,24,
    "&zdet&Rona&rtion","!Detonate!","",FALSE,SLOT(0)
  },

  {
    "disintegrate",
    { L_APP,L_APP,L_APP },
    spell_disintegrate,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,150,18,
    "disintegration","!Disintegrate!","",FALSE,SLOT(0)
  },

  {
    "displacement",
    { L_APP,L_APP,L_APP },
    spell_displacement,TAR_CHAR_SELF,POS_STANDING,
    NULL,10,12,
    "",
    "You are no longer displaced.",
    "$n's image fades as he appears in another spot.",FALSE,SLOT(41)
  },

  {
    "domination",
    { L_APP,L_APP,L_APP },
    spell_domination,TAR_CHAR_OFFENSIVE,POS_STANDING,
    &gsn_domination,5,12,
    "",
    "You regain control of your body.",
    "$n regains control of $m body.",FALSE,SLOT(42)
  },

  {
    "ectoplasmic form",
    { L_APP,L_APP,L_APP },
    spell_ectoplasmic_form,TAR_CHAR_SELF,POS_STANDING,
    NULL,20,12,
    "",
    "You feel solid again.",
    "$n looks solid again.",
    FALSE,SLOT(43)
  },

  {
    "ego whip",
    { L_APP,L_APP,L_APP },
    spell_ego_whip,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,20,12,
    "",
    "You feel more confident.",
    "$n looks more confident.",FALSE,SLOT(44)
  },

  {
    "flesh armor",
    { L_APP,L_APP,L_APP },
    spell_flesh_armor,TAR_CHAR_SELF,POS_STANDING,
    NULL,15,12,
    "",
    "Your skin returns to normal.",
    "$n's skin returns to its normal texture.",FALSE,SLOT(47)
  },

  {
    "inflict pain",
    { L_APP,L_APP,L_APP },
    spell_inflict_pain,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,10,12,
    "&wmind&Wpower&X","!Inflict Pain!","",FALSE,SLOT(0)
  },

  {
    "intellect fortress",
    { L_APP,L_APP,L_APP },
    spell_intellect_fortress,TAR_IGNORE,POS_STANDING,
    NULL,25,24,
    "",
    "Your intellectual fortress crumbles.",
    "$n's intellectual fortress crumbles.",
    FALSE,SLOT(50)
  },

  {
    "lend health",
    { L_APP,L_APP,L_APP },
    spell_lend_health,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,10,12,
    "","!Lend Health!","",FALSE,SLOT(0)
  },

  {
    "levitation",
    { L_APP,L_APP,L_APP },
    spell_levitation,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,10,18,
    "",
    "You slowly float to the ground.",
    "$n slowly floats to the ground.",
    FALSE,SLOT(51)
  },

  {
    "mind thrust",
    { L_APP,L_APP,L_APP },
    spell_mind_thrust,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,8,12,
    "&wmind &Wthrust&X","!Mind Thrust!","",FALSE,SLOT(0)
  },

  {
    "shadow form",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_shadow,0,12,
    "","You no longer move in the shadows.",
    "$n returns from the shadows.",FALSE,SLOT(54)
  },

  {
    "thought shield",
    { L_APP,L_APP,L_APP },
    spell_thought_shield,TAR_CHAR_SELF,POS_STANDING,
    NULL,5,12,
    "","You no longer feel so protected.",
    "$n doesn't feel very protected.",FALSE,SLOT(56)
  },

  {
    "ultrablast",
    { L_APP,L_APP,L_APP },
    spell_ultrablast,TAR_IGNORE,POS_FIGHTING,
    NULL,75,24,
    "&zultr&wabl&Wast&X","!Ultrablast!","",FALSE,
    SLOT(0)
  },

  {
    "cancellation",
    { L_APP,L_APP,L_APP },
    spell_cancellation,TAR_CHAR_DEFENSIVE,POS_FIGHTING,
    NULL,15,16,
    "","!Cancellation!","",FALSE,SLOT(0)
  },

  {
    "throw",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_STANDING,
    &gsn_throw,0,0,
    "throw","!Throw!","",FALSE,SLOT(0)
  },

  /*
   * Turn undead and mental block by Altrag
   */
  {
    "turn undead",
    { L_APP,L_APP,L_APP },
    spell_turn_undead,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,100,12,
    "","!Turn Undead!","",FALSE,SLOT(0)
  },

  {
    "mental block",
    { L_APP,L_APP,L_APP },
    spell_mental_block,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,25,8,
    "","Your mind feels more vulnerable.",
    "$n's mind becomes more vulnerable.",FALSE,SLOT(57)
  },

  /*
   * Patch by Altrag
   */
  {
    "patch",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_patch,0,0,
    "","!Patch!","",FALSE,SLOT(0)
  },

  {
    "flamesphere",
    { L_APP,L_APP,L_APP },
    gspell_flamesphere,TAR_GROUP_OFFENSIVE,POS_STANDING,
    NULL,14,20,
    "flamesphere","!Flamesphere!","",TRUE,SLOT(58)
  },

  {
    "bash door",
    { L_APP, L_APP, 15 },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_bash_door,0,24,
    "bash","!Bash Door!","",FALSE,SLOT(0)
  },

  {
    "mental drain",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_mental_drain,0,12,
    "mental drain","!Mental Drain!","",FALSE,SLOT(0)
  },

  {
    "drain life",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_drain_life,0,12,
    "&zlife &wdrain","!Drain Life!","",FALSE,SLOT(0)
  },

  {
    "holy strength",
    { L_APP,L_APP,L_APP },
    spell_holy_strength,TAR_CHAR_SELF,POS_STANDING,
    NULL,100,12,
    "","Your divine strength fades.",
    "The divine strength surrounding $n fades.",FALSE,SLOT(59)
  },

  {
    "protection good",
    { L_APP,L_APP,L_APP },
    spell_protection_good,TAR_CHAR_SELF,POS_STANDING,
    NULL,5,12,
    "","You feel less protected.",
    "$n looks less protected.",TRUE,SLOT(62)
  },

  {
    "shield block",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_shield_block,0,0,
    "","!Shield Block!","",FALSE,SLOT(0)
  },

  {
    "turn evil",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_turn_evil,0,12,
    "&Wdivine power&X","!Turn Evil!","",FALSE,SLOT(0)
  },

  {
    "gouge",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_gouge,0,12,
    "gouge","!Gouge!","",FALSE,SLOT(0)
  },

  {
    "summon angel",
    { L_APP,L_APP,L_APP },
    spell_summon_angel,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,100,12,
    "","!Summon Angel!","",FALSE,SLOT(0)
  },

  {
    "holy fires",
    { L_APP,L_APP,L_APP },
    spell_holy_fires,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,50,12,
    "&Who&Rly fl&rames&X","!Holy Fires!","",FALSE,SLOT(0)
  },

  {
    "true sight",
    { L_APP,L_APP,L_APP },
    spell_truesight,TAR_CHAR_SELF,POS_STANDING,
    NULL,59,12,
    "","Your vision is no longer so true.",
    "$n's vision returns to normal.",TRUE,SLOT(64)
  },

  {
    "alchemy",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_alchemy,0,50,
    "","!ALCHEMY!","",FALSE,SLOT(0)
  },

  {
    "scribe",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_scribe,0,50,
    "","!SCRIBE!","",FALSE,SLOT(0)
  },

  {
    "stun",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_stun,0,0,
    "","!Stun!","",FALSE,SLOT(0)
  },

  {
    "entangle",
    { L_APP,L_APP,L_APP },
    spell_entangle,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,25,14,
    "","The vines release their grasp.",
    "The vines unwrap themselves from $n.",TRUE,SLOT(67)
  },

  {
    "confusion",
    { L_APP,L_APP,L_APP },
    spell_confusion,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,50,24,
    "","You become more sure of your surroundings.",
    "$n looks less confused.",TRUE,SLOT(70)
  },

  {
    "mind probe",
    { L_APP,L_APP,L_APP },
    spell_mind_probe,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,50,24,
    "","!MIND PROBE!","",FALSE,SLOT(0)
  },

  {
    "trip",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_trip,0,12,
    "trip","!Trip!","",FALSE,SLOT(0)
  },

  {
    "fumble",
    { L_APP,L_APP,L_APP },
    spell_fumble,TAR_CHAR_OFFENSIVE,POS_STANDING,
    NULL,50,24,
    "","You feel more sure of hand.",
    "$n looks more confident.",TRUE,SLOT(73)
  },

  {
    "circle",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_circle,0,12,
    "sneak attack","!CIRCLE!","",FALSE,SLOT(0)
  },

  {
    "summon shadow",
    { L_APP,L_APP,L_APP },
    spell_summon_shadow,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,100,12,
    "","!Summon Shadow!","",FALSE,SLOT(0)
  },

  {
    "summon beast",
    { L_APP,L_APP,L_APP },
    spell_summon_beast,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,100,12,
    "","!Summon Beast!","",FALSE,SLOT(0)
  },

  {
    "summon treant",
    { L_APP,L_APP,L_APP },
    spell_summon_trent,TAR_CHAR_SELF,POS_FIGHTING,
    NULL,100,12,
    "","!Summon Trent!","",FALSE,SLOT(0)
  },

  {
    "shatter",
    { L_APP,L_APP,L_APP },
    spell_shatter,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,150,32,
    "","!shatter!","",FALSE,SLOT(0)
  },

  {
    "molecular unbinding",
    { L_APP,L_APP,L_APP },
    spell_molecular_unbind,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,100,32,
    "","!shatter!","",FALSE,SLOT(0)
  },

  {
    "phase shift",
    { L_APP,L_APP,L_APP },
    spell_phase_shift,TAR_CHAR_SELF,POS_STANDING,
    NULL,100,12,
    "","You shift back into the physical plane.",
    "$n's body shifts back into the existing world.",FALSE,SLOT(75)
  },

  {
    "disrupt",
    { L_APP,L_APP,L_APP },
    spell_disrupt,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,125,12,
    "disruption","You feel less disrupted.",
    "$n looks more stable.",TRUE,SLOT(76)
  },

  {
    "healing hands",
    { L_APP,L_APP,L_APP },
    spell_healing_hands,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,40,12,
    "","!Healing Hands!","",FALSE,SLOT(0)
  },

  {
    "pray",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_SELF,POS_RESTING,
    &gsn_prayer,0,12,
    "","Thalador's blessing leaves your soul.",
    "Thalador's blessing rises from $n's soul.",FALSE,SLOT(77)
  },
  {
    "spellcraft",
    { 50, L_APP, L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_spellcraft,0,0,
    "","!Spellcraft!","",FALSE,SLOT(0)
  },
  {
    "multiburst",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_multiburst,0,8,
    "","!Multiburst!","",FALSE,SLOT(0)
  },
  {
    "dark ritual",
    { L_APP,L_APP,L_APP },
    spell_dark_ritual,TAR_IGNORE,POS_STANDING,
    NULL,5,12,
    "","!Dark Ritual!","",FALSE,SLOT(0)
  },
  {
    "gravebind",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_OBJ_INV,POS_RESTING,
    &gsn_gravebind,0,0,
    "","!Gravebind!","",FALSE,SLOT(0)
  },

  {
    "stench of decay",
    { L_APP,L_APP,L_APP },
    spell_stench_of_decay,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    NULL,15,12,
    "&zstench of decay&X","!Stench of Decay!","",FALSE,SLOT(0)
  },
  /* Werewolf starts here -- Hannibal */
  {
    "fast healing",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_SLEEPING,
    &gsn_fastheal,0,0,
    "","!Fast Healing!","",FALSE,SLOT(0)
  },
  {
    "rage",
    { L_APP, L_APP, 25 },
    spell_null,TAR_IGNORE,POS_RESTING,
    &gsn_rage,0,24,
    "","You calm down as the rage leaves you.",
    "$n calms down as the rage leaves $m.",FALSE,SLOT(80)
  },

  {
    "track",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_track,0,0,
    "","!Track!","",FALSE,SLOT(0)
  },

  {
    "headbutt",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_headbutt,0,0,
    "headbutt","!Headbutt!","",FALSE,SLOT(0)
  },

  {
    "retreat",
    { L_APP, L_APP, 25 },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_retreat,0,0,
    "","!Run Away:P!","",FALSE,SLOT(0)
  },

  {
    "escape",
    { L_APP, 25, L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_escape,0,0,
    "","!Run Away:P!","",FALSE,SLOT(0)
  },

  {
    "antidote",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_antidote,0,0,
    "","","",FALSE,SLOT(0)
  },
  {
    "haggle",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_IGNORE,POS_FIGHTING,
    &gsn_haggle,0,0,
    "","","",FALSE,SLOT(0)
  },

  {
    "scrolls",
    { 1, 1, 1 },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_scrolls,0,0,
    "&Wfailure","!Scrolls!","",FALSE,SLOT(0)
  },

  {
    "wands",
    { 1, L_APP, L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_wands,0,0,
    "&Wfailure","!Wands!","",FALSE,SLOT(0)
  },

  {
    "staves",
    { 5, L_APP, L_APP },
    spell_null,TAR_IGNORE,POS_STANDING,
    &gsn_staves,0,0,
    "&Wfailure","!Staves!","",FALSE,SLOT(0)
  },

  {
    "purify",
    { L_APP,L_APP,L_APP },
    spell_purify,TAR_CHAR_DEFENSIVE,POS_STANDING,
    NULL,70,12,"","!purify!",FALSE,SLOT(0)
  },
  {
    "lure",
    { L_APP, 25, L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_lure,0,32,"","!lure!",FALSE,SLOT(0)
  },
  {
    "silence",
    { L_APP,L_APP,L_APP },
    spell_silence,TAR_CHAR_OFFENSIVE,POS_STANDING,
    NULL,60,12,"","You may cast spells again.",
    "$n is somatic once again.",TRUE,SLOT(24)
  },

  {
    "flip",
    { L_APP,L_APP,L_APP },
    spell_null,TAR_CHAR_OFFENSIVE,POS_FIGHTING,
    &gsn_flip,0,32,"flip","!flip!",FALSE,SLOT(0)
  },
  {
    "hallucinate",
    { L_APP,L_APP,L_APP },
    spell_hallucinate,TAR_CHAR_OFFENSIVE,POS_STANDING,
    NULL,160,12,"","You regain your sanity.",
    "$n looks better.",TRUE,SLOT(205)
  },

  /*
   * Place all new spells/skills BEFORE this one.  It is used as an index marker
   * in the same way that theres a blank entry at the end of the command table.
   * (in interp.c)
   * -- Altrag
   */
  {
    "",
    {999,999,999   },
    spell_null,0,0,
    NULL,0,0,
    "","","",SLOT(0)
  }
};

const struct gskill_type gskill_table[ MAX_GSPELL ] = {
  /*
   * The globals for group spells..
   * -- Altrag
   */
  /*{wait,SLOT(slot),{CAS,ROG,FIG},*/
  { 3,SLOT(221),{0,0,0} },
  { 2,SLOT(227),{2,0,0} },
};
