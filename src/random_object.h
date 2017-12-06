// macros
#define MAX_RANDOM_ITEM_TYPES = 21

// function definitions
char * random_affect( OBJ_DATA * obj );

// data structures
struct random_item_type {
  int    type;
  int    wear_flag;
  char * prefix;
  char * name;
  bool   plural;
  bool   reprefixable;
  int    value[ 4 ];
};

struct random_item_material {
  char * prefix;
  char * name;
  char * color;
};

struct random_item_affects {
  char * skill;
  int    mlow;
  int    mhigh;
  char * suffix;
};