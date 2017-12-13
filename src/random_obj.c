/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

/***************************************************************************
*	 EotS 1.3 is copyright 1993-1996 Eric Orvedal and Nathan Axtman         *
*                                                                         *
*	 EotS has been brought to you by us, the merry drunk mudders	          *
*	    Kjodo                                                               *
*	    Torann                                                              *
*	    Sledge                                                              *
*	    Nicodemous                                                          *
*	    Tom                                                                 *
*                                                                         *
*	 By using this code, you have agreed to follow the terms of the         *
*	 blood oath of the carungatang                                          *
*                                                                         *
*              EotS Random object functions                               *
*       I think you will enjoy :)   (Ugh...935 lines [was 3000+ :)] )     *
***************************************************************************/

#define unix 1
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "random_obj.h"

const struct random_item_type random_item_table[ ] = {
//  type,        wear_flag,         prefix,      name,           plural, reprefixable, object_values
  { ITEM_ARMOR,  ITEM_WEAR_FINGER,  "a",         "ring",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FINGER,  "a",         "signet",       FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FINGER,  "a",         "band",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_NECK,    "a",         "necklace",     FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_NECK,    "a",         "collar",       FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_NECK,    "a",         "pendant",      FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_NECK,    "a",         "chain",        FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_BODY,    "a suit of", "armor",        FALSE,  FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_BODY,    "a",         "breastplate",  FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_BODY,    "a",         "vest",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a",         "hat",          FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a",         "cap",          FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a",         "hood",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a",         "crown",        FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a",         "head piece",   FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a",         "helm",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a",         "helmet",       FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HEAD,    "a pair of", "horns",        TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_LEGS,    "a pair of", "leggings",     TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_LEGS,    "a pair of", "pants",        TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_LEGS,    "a pair of", "breeches",     TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_LEGS,    "a",         "skirt",        FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FEET,    "a pair of", "shoes",        TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FEET,    "a pair of", "boots",        TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FEET,    "a pair of", "slippers",     TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FEET,    "a pair of", "sandles",      TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HANDS,   "a pair of", "gloves",       TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_HANDS,   "a pair of", "gauntlets",    TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_ARMS,    "a pair of", "sleeves",      TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_WRIST,   "a",         "bracer",       FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_WRIST,   "a",         "bracelet",     FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_WRIST,   "a pair of", "handcuffs",    TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_SHIELD,  "a",         "shield",       FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_SHIELD,  "a",         "buckler",      FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_HOLD,         "a",         "sceptre",      FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_HOLD,         "a",         "staff",        FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_EARS,    "an",        "earring",      FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_EARS,    "an",        "ear clasp",    FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_ABOUT,   "a",         "robe",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_ABOUT,   "a",         "cloak",        FALSE,  TRUE,        { 0, 1, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_ABOUT,   "a",         "cape",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_WAIST,   "a",         "belt",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_WAIST,   "a",         "sash",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_CONTACT, "some",      "tears",        TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_CONTACT, "a pair of", "glasses",      TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_CONTACT, "a pair of", "eyes",         TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_CONTACT, "a pair of", "shades",       TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_CONTACT, "a pair of", "goggles",      TRUE,   FALSE,       { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_ORBIT,   "an",        "orb",          FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FACE,    "a",         "mask",         FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_FACE,    "a",         "faceplate",    FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_ARMOR,  ITEM_WEAR_ANKLE,   "an",        "anklet",       FALSE,  TRUE,        { 0, 0, 0, 0                  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "quarterstaff", FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_HIT    } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "club",         FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CRUSH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "greatclub",    FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CRUSH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "hammer",       FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CRUSH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "warhammer",    FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CRUSH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "flail",        FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CRUSH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "mace",         FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CRUSH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "morningstar",  FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CRUSH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "dagger",       FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_PIERCE } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "knife",        FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_PIERCE } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "rapier",       FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_PIERCE } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "spear",        FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_PIERCE } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "trident",      FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_PIERCE } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "pike",         FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_PIERCE } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "sword",        FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_SLASH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "shortsword",   FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_SLASH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "longsword",    FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_SLASH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "broadsword",   FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_SLASH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "greatsword",   FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_SLASH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "scimitar",     FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_SLASH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "sickle",       FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_SLASH  } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "axe",          FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CHOP   } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "handaxe",      FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CHOP   } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "battleaxe",    FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_CHOP   } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "whip",         FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_WHIP   } },
  { ITEM_WEAPON, ITEM_WIELD,        "a",         "bullwhip",     FALSE,  TRUE,        { 0, 0, 0, WEAPON_TYPE_WHIP   } }
};

const struct random_item_material random_material_table[ ] = {
//  prefix, name,           color
  { "a",    "dragonscale",  "&r"  },
  { "a",    "dragonskin",   "&g"  },
  { "a",    "bone",         "&w"  },
  { "an",   "obsidian",     "&z"  },
  { "an",   "ebony",        "&z"  },
  { "an",   "ivory",        "&W"  },
  { "a",    "crystal",      "&W"  },
  { "a",    "silver",       "&W"  },
  { "a",    "golden",       "&Y"  },
  { "a",    "copper",       "&O"  },
  { "a",    "wooden",       "&O"  },
  { "a",    "leather",      "&O"  },
  { "a",    "stone",        "&w"  },
  { "a",    "glass",        "&C"  },
  { "an",   "ice",          "&C"  },
  { "an",   "iron",         "&w"  },
  { "a",    "steel",        "&w"  },
  { "a",    "ruby",         "&R"  },
  { "a",    "sapphire",     "&B"  },
  { "an",   "emerald",      "&G"  }
};

const char * random_color_table[ ] = {
  "&r", "&R", "&g", "&G",
  "&b", "&B", "&C", "&W",
  "&Y", "&O", "&p", "&P",
  "&c", "&w", "&z"
};

const struct random_item_affects random_affect_table[ ] = {
//  skill,              mlow, mhigh, suffix
  { "invis",            -1,   100,   "of invisibility"   },
  { "hide",             -1,   100,   "of hiding"         },
  { "sneak",            -1,   100,   "of sneaking"       },
  { "scry",             -1,   100,   "of sight"          },
  { "protect",          -1,   100,   "of protection"     },
  { "fly",              -1,   100,   "of flight"         },
  { "infrared",         -1,   100,   "of infravision"    },
  { "pass-door",        -1,   100,   "of entry"          },
  { "giant-str",        -1,   100,   "of the giant"      },
  { "strength",         -10,  10,    "of strength"       },
  { "dexterity",        -10,  10,    "of dexterity"      },
  { "intelligence",     -10,  10,    "of intelligence"   },
  { "wisdom",           -10,  10,    "of wisdom"         },
  { "constitution",     -10,  10,    "of constitution"   },
  { "sex",              -1,   1,     "of empathy"        },
  { "age",              -10,  100,   "of aging"          },
  { "height",           -3,   10,    "of the ant"        },
  { "weight",           -1,   100,   "of encumbrance"    },
  { "mana",             -200, 200,   "of energy"         },
  { "anti-disarm",      -100, 100,   "of gripping"       },
  { "hp",               -200, 200,   "of life"           },
  { "move",             -200, 200,   "of endurance"      },
  { "ac",               -200, 200,   "of armoring"       },
  { "hitroll",          -200, 200,   "of accuracy"       },
  { "damroll",          -200, 200,   "of the brute"      },
  { "combat-mind",      -1,   100,   "of combat"         }
};

OBJ_DATA * random_object( int level ) {
  OBJ_DATA       * obj;
  OBJ_INDEX_DATA * pObjIndex;
  char             name[ MAX_STRING_LENGTH ];
  char             short_desc[ MAX_STRING_LENGTH ];
  char             long_desc[ MAX_STRING_LENGTH ];
  char             color[ 10 ];

  pObjIndex = get_obj_index( OBJ_VNUM_DUMMY );
  obj       = create_object( pObjIndex, level );

  // pick a random item from the table
  struct random_item_type item = random_item_table[ RANDOM( random_item_table ) ];

  // set item type
  obj->item_type   = item.type;

  // set wear flags
  obj->wear_flags ^= ITEM_TAKE;
  obj->wear_flags ^= item.wear_flag;

  // set default values
  obj->value[ 0 ] = item.value[ 0 ];
  obj->value[ 1 ] = item.value[ 1 ];
  obj->value[ 2 ] = item.value[ 2 ];
  obj->value[ 3 ] = item.value[ 3 ];

  // take special considerations for weapons
  if ( item.type == ITEM_WEAPON ) {
    int lowdam  = number_range( 1, level );
    int highdam = number_range( lowdam, level + 5 );

    obj->value[ 1 ] = lowdam;
    obj->value[ 2 ] = highdam;
  }

  // pick a random color
  strcpy(color, random_color_table[ RANDOM( random_color_table ) ]);

  // pick a random material from the table
  struct random_item_material material = random_material_table[ RANDOM( random_material_table ) ];

  // put it all together
  if ( (( number_percent() + level ) > 90) && (dice(1, 2) > 1) ) {
    sprintf( short_desc, "%s%s%s %s %s", material.color, material.name, color, item.name, random_affect( obj ));
  } else {
    sprintf( short_desc, "%s%s %s%s%s %s", color, item.reprefixable ? material.prefix : item.prefix, material.color, material.name, color, item.name );
  }

  sprintf( long_desc, "%s %s %s been left here.", capitalize(item.prefix), item.name, item.plural ? "have" : "has");

  sprintf( name, "%s %s", material.name, item.name);

  obj->name        = str_dup( name );
  obj->short_descr = str_dup( short_desc );
  obj->description = str_dup( long_desc );
  obj->level = level;

  return obj;
}

char * random_affect( OBJ_DATA * obj ) {
  AFFECT_DATA * paf;

  int rKey = RANDOM( random_affect_table );

  paf           = new_affect();
  paf->location = flag_value( apply_flags, random_affect_table[ rKey ].skill );
  paf->modifier = number_range( random_affect_table[ rKey ].mlow, random_affect_table[ rKey ].mhigh );
  paf->type     = skill_lookup( random_affect_table[ rKey ].skill );

  if ( paf->type < 0 ) {
    paf->type = 0;
  }

  paf->duration  = -1;
  paf->bitvector = 0;
  paf->next      = obj->affected;
  obj->affected  = paf;

  return random_affect_table[ rKey ].suffix;
}
