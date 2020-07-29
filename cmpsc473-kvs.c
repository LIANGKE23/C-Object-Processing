#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "cmpsc473-kvs.h"
#include "cmpsc473-util.h"

// Internal functions
extern struct kv_list_entry *kvs_match( struct kvs *kvs, unsigned char *key );

int kvs_init( struct kvs *kvs, char *filepath, unsigned int keysize, 
	      unsigned int valsize )
//padsize
{
	int i, err;
	unsigned char *buf, *orig_buf;
	size_t bufsize;
	size_t bytes_left = 0;

	// initialize sizes
	kvs->keysize = keysize;
	kvs->valsize = valsize;  
	//kvs->tagsize = tagsize;
  
	// initialize buckets 
	for (i = 0; i < KVS_BUCKETS; i++) {
		kvs->store[i] = (struct kv_list_entry *) NULL;
	}

	// Get buf for current file contents
	bufsize = buffer_from_file( filepath, &buf );
	assert( bufsize >= 0 ); 

	// add entries from buf into KVS
	if ( bufsize > 0 ) {
		unsigned int entrysz = ( keysize+valsize+//tagsize+
					 PAD_LEN ); // keysize, valsize, othersize
		bytes_left = bufsize; 
		orig_buf = buf;

		while ( bytes_left >= entrysz ) {  
			err = kvs_auth_set( kvs, buf, buf+keysize );  // key at start of buf
			bytes_left -= entrysz;
			buf += entrysz;
		}
		free( orig_buf );
	}
	assert(bytes_left == 0);    

	return 0;
}


int kvs_auth_set( struct kvs *kvs, unsigned char *key, unsigned char *val //, unsigned char *tag 
	)
{
	struct authval *av;
	struct kvpair *kvp;
	struct kv_list_entry *kvle, *head;

	assert( key && val // && tag 
		);

	/* lookup existing key */
	kvle = kvs_match( kvs, key );
	if ( kvle ) {
		// if found, replace value and tag 
		memcpy( kvle->entry->av->value, val, kvs->valsize );
		// memcpy( kvle->entry->av->tag, tag, kvs->tagsize ); 
		return 0;
	}

	/* make KVE */
	kvp = (struct kvpair *)malloc(sizeof(struct kvpair));
	assert( kvp != 0 );
	kvp->key = (unsigned char *)malloc(kvs->keysize);
	memcpy( kvp->key, key, kvs->keysize );

	av = (struct authval *)malloc(sizeof(struct authval));
	assert( av != 0 );
	kvp->av = av;
	av->value = (unsigned char *)malloc(kvs->valsize);
	// av->tag = (unsigned char *)malloc(kvs->tagsize);
	memcpy( av->value, val, kvs->valsize );
	// memcpy( av->tag, tag, kvs->tagsize ); 

	/* make list entry */
	kvle = (struct kv_list_entry *)malloc(sizeof(struct kv_list_entry));
	assert( kvle != 0 );
	kvle->entry = kvp;

	/* put in KVS */
	// use lower 4 bits of last byte in key as bucket id 
	unsigned int bucket = 0xF & (key[kvs->keysize-1]);   // *** matching function
	head = kvs->store[bucket];
  
	if ( head == NULL ) {
		kvs->store[bucket] = kvle;
		kvle->next = (struct kv_list_entry *)NULL;
	}
	else {
		kvs->store[bucket] = kvle;
		kvle->next = head;
	}

	return 0;
}


int kvs_auth_get( struct kvs *kvs, unsigned char *key, unsigned char **val //, unsigned char **tag 
	)
{
	struct kv_list_entry *kvle;

	assert( key != 0 );

	kvle = kvs_match( kvs, key );
	if ( kvle ) {
		*val = kvle->entry->av->value;
		// *tag = kvle->entry->av->tag;
		return 0;
	}
  
	return 1;
}


struct kv_list_entry *kvs_match( struct kvs *kvs, unsigned char *key )
{
	unsigned int bucket;
	struct kv_list_entry *kvle;

	// use lower 4 bits of last byte in key as bucket id 
	bucket = 0xF & (key[kvs->keysize-1]);   // *** matching function
	kvle = kvs->store[bucket];

	while( kvle != NULL ) {
		struct kv_list_entry *next = kvle->next;
		if ( memcmp( key, kvle->entry->key, kvs->keysize ) == 0 ) {
			return kvle;
		}
		kvle = next;
	}

	return (struct kv_list_entry *)NULL;
}



int kvs_dump( struct kvs *kvs, char *path, unsigned int keysize, 
	      unsigned int valsize //, unsigned int tagsize 
	      )
{
	int i;
	struct kv_list_entry *kvle;
	struct authval *av;
	struct kvpair *kvp;
	FILE *fp = fopen( path, "w+" ); 

	assert( fp != NULL );

	for (i = 0; i < KVS_BUCKETS; i++) {
		kvle = kvs->store[i];
      
		while ( kvle != NULL ) {
			kvp = kvle->entry;
			av = kvp->av;

			fwrite((const char *)kvp->key, 1, keysize, fp);
			fwrite((const char *)av->value, 1, valsize, fp);
			// fwrite((const char *)av->tag, 1, tagsize, fp);
			fwrite((const char *)PADDING, 1, PAD_LEN, fp);
	
			// Next entry
			kvle = kvle->next;
		}
	}
	return 0;
}
