// Defines
#define KVS_BUCKETS  16
#define PADDING     "----"
#define PAD_LEN       4
 
// Data structures
struct authval {
  unsigned char *value;
  // unsigned char *tag; 
};

struct kvpair {
  unsigned char *key;
  struct authval *av;
};

struct kv_list_entry {
  struct kvpair *entry;
  struct kv_list_entry *next;
};

struct kvs {
  struct kv_list_entry *store[KVS_BUCKETS]; 
  unsigned int keysize;
  unsigned int valsize;
  // unsigned int tagsize;
};

// Global key-value store
//extern struct kv_list_entry *Passwds[KVS_BUCKETS]; 
//extern struct kv_list_entry *Objects[KVS_BUCKETS]; 
// extern struct kvs *Passwds;
extern struct kvs *Objects;

// API
extern int kvs_init( struct kvs *kvs, char *filepath, unsigned int keysize, 
		     unsigned int valsize //, unsigned int tagsize
		     // , unsigned int padsize 
		     );
extern int kvs_auth_set( struct kvs *kvs, unsigned char *key, unsigned char *val //, unsigned char *tag 
			 );
extern int kvs_auth_get( struct kvs *kvs, unsigned char *key, unsigned char **val // , unsigned char **tag 
			 );
extern int kvs_dump( struct kvs *kvs, char *path, unsigned int keysize, 
		     unsigned int valsize //, unsigned int tagsize 
		     );


