// function definitions
void generate_quest( CHAR_DATA * ch, CHAR_DATA * questman );
void quest_update( void );

// data structures
struct quest_data {
  int    vnum;
  int    qp;
};

struct quest_item_type {
  char * name;
  char * short_desc;
  char * long_desc;
};
