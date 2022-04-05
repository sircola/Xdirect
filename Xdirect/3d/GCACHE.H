

#define MAX_GCACHE_LUMPS 1024


//
// Doubly linked-list lump structure
//
typedef struct gcachelump_s {

	BOOL free;
	BOOL assigned;
	INT start;
	char key[256];
	INT size;
	INT access_num;

	struct gcachelump_s *previous;
	struct gcachelump_s *next;

} gcachelump_t, *gcachelump_ptr;




//
//
//
typedef struct gcache_s {

	gcachelump_t gcache_lumps[ MAX_GCACHE_LUMPS ];

	INT free_space;
	INT total_space;
	INT num_lumps;

	gcachelump_t *first_lump;

} gcache_t, *gcache_ptr;



extern void glide_cache_init( void );
extern void glide_cache_uninit( void );
extern INT glide_cache_item_check( char * );
extern INT glide_cache_item_space( char *, INT );



