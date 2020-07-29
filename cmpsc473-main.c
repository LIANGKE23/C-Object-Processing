// #if defined(_GNU_SOURCE) || _POSIX_C_SOURCE >= 200809L //for getline()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include "cmpsc473-kvs.h"
#include "cmpsc473-util.h"
#include "cmpsc473-format-102.h" // CHANGE: student-specific

/* Defines */
#define OBJ_LEN     168  // CHANGE: size of object tree for this project
#define KEY_LEN     8
#define LINE_SIZE   100

#define OBJECTS_PATH "./objects-file"

struct kvs *Objects;


/* Project APIs */
// public 
extern int set_object( char *objname,  char *filename );
extern int get_object( char *objname );

// internal
extern struct A *upload_A( FILE *fp );
extern struct B *upload_B( FILE *fp );
extern struct C *upload_C( FILE *fp );
extern unsigned char *marshall( struct A *objA );
extern struct A *unmarshall( unsigned char *obj );
extern int output_obj( struct A *objA );
extern int kvs_dump( struct kvs *kvs, char *filepath, unsigned int keysize, 
		     unsigned int valsize //, unsigned int tagsize
		     );


#include <errno.h>// safe_atoi
#include <limits.h>
 
static inline int safe_atoi( char *str, int *tgt )
{
        char *end_ptr;
        int base = 10;
        // set errno = 0 before the call                                                                                         
        errno = 0;
        // same size in gcc - compile with -Wconversion                                                                          
        long num = strtol( str, &end_ptr, base );
        // error for converting to long                                                                                          
        if (((ERANGE == errno) && (( num == LONG_MAX ) || ( num == LONG_MIN )))
            || (( errno != 0 ) && ( num == 0 ))) {
                perror("strtol");
                return -1;
        }
        else if (end_ptr == str) {
                if (puts("not valid numeric input\n") == EOF) {
                        /* Handle error */
                }
                return -1;
        }
        else if ('\n' != *end_ptr && '\0' != *end_ptr) {
                if (puts("extra characters on input line\n") == EOF) {
                        /* Handle error */
                }
                return -1;
        }
 
        // bigger than int?                                                                                                      
        else if (( num >= INT_MAX ) || ( num <= INT_MIN )) {
                if (puts("too big for int\n") == EOF) {
                        /* Handle error */
                }
                return -1;
        }
        *tgt = num;    // long to int here                                                                                       
        return 0;  // means OK, tgt has value                                                                                    
}


/*****************************

Invoke:
cmpsc473-p1 cmd obj-name obj-file

Commands:
<set_object> obj-name obj-file
<get_object> obj-name 

1 - set object - add object of <obj-name> from <obj-file> to object store

2 - get-object - retrieve object of <obj-name> from object store

******************************/


int main( int argc, char *argv[] )
{
	int rtn;

	assert( argc >= 3 );

	/* initialize KVS from file */
	Objects = (struct kvs *)malloc(sizeof(struct kvs));
	kvs_init( Objects, OBJECTS_PATH, KEY_LEN, OBJ_LEN //, PAD_LEN 
		  );  // OBJ_LEN - size of the object tree for this project

	if ( strncmp( argv[1], "set", 1 ) == 0 ) {
		assert( argc == 4 );
		rtn = set_object( argv[2], argv[3] );
	}
	else if ( strncmp( argv[1], "get", 2 ) == 0 ) {
		assert( argc == 3 );
		rtn = get_object( argv[2] );
	}
	else {
		printf( "Unknown command: %s\nExiting...\n", argv[1] );
		exit(-1);
	}

	kvs_dump( Objects, OBJECTS_PATH, KEY_LEN, OBJ_LEN ); 

	exit(0);
}


int get_object( char *objname )
{
	unsigned char *key = (unsigned char *)malloc(KEY_LEN);
	unsigned char *obj;
	int rc;

	struct A *objA;

	assert( strlen(objname) <= KEY_LEN );  

	memset( key, 0, KEY_LEN );
	memcpy( key, objname, strlen(objname) );

	rc = kvs_auth_get( Objects, key, &obj );
 
	if ( rc == 0 ) {  // found object, run op0 (output)
		objA = unmarshall( obj );
//	printf("Object Retrieved: %s\n", key );
		objA->op0( objA );
	}
	else {
		fprintf(stderr, "get_object failed to return object for name: %s\n", objname );
		return -1;
	}

	return 0;
}


int set_object( char *objname, char *filename )
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0, size;
	unsigned char *key = (unsigned char *)malloc(KEY_LEN);
	struct A *objA;
	struct B *objB;
	struct C *objC;

	fp = fopen( filename, "r" );  // read input
	assert( fp != NULL ); 

	memset( key, 0, KEY_LEN );
	memcpy( key, objname, strlen(objname) );

	fp = fopen( filename, "r" );  // read input
	assert( fp != NULL ); 

	while(1) {
		size = getline( &line, &len, fp );
		if ( size == -1 ) break;

		if ( strcmp( line, "struct A\n" ) == 0 ) {
			objA = upload_A( fp );
			if (!objA) return -1;
		}

		else if ( strcmp( line, "struct B\n" ) == 0 ) {
			objB = upload_B( fp );
			if (!objB) return -1;
		}

		else if ( strcmp( line, "struct C\n" ) == 0 ) {
			objC = upload_C( fp );
			if (!objC) return -1;
		}
	}

	// TASK 2: 
	// update pointers from objA to other objects 
	// assign objA function pointers
	//int (*op0)(struct A *objA);
	objA->ptr_c = objB;
        objA->ptr_d = objC;
	objA->op0 = &output_obj;
	objA->op1 = &marshall;
	  //unsigned char *(*op1)(struct A *objA);
	/* upload object into key-value store */
	// Run op1 (marshall)
	kvs_auth_set( Objects, key, objA->op1(objA) );

	return 0;
}


// TASK 1: Upload object specific data into fields of objects
// See example below for objD 
struct A *upload_A( FILE *fp )
{

	char *line = NULL;
	size_t len = 0, size;
	// other declarations
	struct A *objA = (struct A *)malloc(sizeof(struct A));
	memset( objA, 0, sizeof(struct A) );
	int *k;
    char *ref;
    int p;

	while(1) {
		size = getline( &line, &len, fp );
		if ( strncmp( line, "\n", 1 ) == 0 )
			break;
                // replace \n at end with null-terminator for value
		line[strlen(line)-1] = '\0';   

		if (( ref = strstr( line, "string_a" ))) {
			if (strlen(line)<=29){
				memcpy( &(objA->string_a), ref+9, strlen(ref+9) );
			}
			else if(strlen(line)>15){
				line[30] = '\0';
				memcpy( &(objA->string_a), ref+9, strlen(ref+9) );
			}
		
		}

		if (( ref = strstr( line, "num_b" ))) {
			safe_atoi( ref+6, &k);
		       
		  
		        objA->num_b = k;
		   

		    }
		}
	

	/* parse and set objA field values */

	return objA;
}

struct B *upload_B( FILE *fp )
{
	char *line = NULL;
    size_t len = 0, size;
	// other declarations
	struct B *objB = (struct B *)malloc(sizeof(struct B));
	memset( objB, 0, sizeof(struct B) );
    char *ref;
	int *k;	
	int p;

	while(1) {
		size = getline( &line, &len, fp );
		if ( strncmp( line, "\n", 1 ) == 0 )
			break;
       
                // replace \n at end with null-terminator for value
		line[strlen(line)-1] = '\0';   

		if (( ref = strstr( line, "num_a" ))) {
			safe_atoi( ref+6, &k);
		      
			objB->num_a = k;
		   
		}

		if (( ref = strstr( line, "string_d" ))) {

			if (strlen(line)<=29){
			  memcpy( &(objB->string_d), ref+9, strlen(ref+9) );
			}
			else if(strlen(line)>15){
				line[30] = '\0';
				memcpy( &(objB->string_d), ref+9, strlen(ref+9) );
			}

		}

		if (( ref = strstr( line, "string_c" ))) {
			if (strlen(line)<=29){
			memcpy( &(objB->string_c), ref+9, strlen(ref+9) );
			}
			else if(strlen(line)>15){
				line[30] = '\0';
			memcpy( &(objB->string_c), ref+9, strlen(ref+9) );
			}
		}

		if (( ref = strstr( line, "string_e" ))) {
			if (strlen(line)<=29){
			memcpy( &(objB->string_e), ref+9, strlen(ref+9) );
			}
			else if(strlen(line)>15){
				line[30] = '\0';
			memcpy( &(objB->string_e), ref+9, strlen(ref+9) );
			}

		}

		if (( ref = strstr( line, "num_b" ))) {
          safe_atoi( ref+6, &k);
		       
		 
			objB->num_b = k;
		   

		}
	}

	/* parse and set objB field values */

	return objB;
}


struct C *upload_C( FILE *fp )
{
	char *line = NULL;
    size_t len = 0, size;
	struct C *objC = (struct C *)malloc(sizeof(struct C));
	memset( objC, 0, sizeof(struct C) );
	int *k;
    char *ref;
    int p;
    
	while(1) {

		size = getline( &line, &len, fp );
		if ( strncmp( line, "\n", 1 ) == 0 )
			break;
       
                // replace \n at end with null-terminator for value
		line[strlen(line)-1] = '\0';   

		if (( ref = strstr( line, "num_a" ))) {
			safe_atoi( ref+6, &k);
		 
			objC->num_a = k;
		
		}

		if (( ref = strstr( line, "string_c" ))) {
			if (strlen(line)<=29){
			memcpy( &(objC->string_c), ref+9, strlen(ref+9) );
			}
			else if(strlen(line)>15){
				line[30] = '\0';
			memcpy( &(objC->string_c), ref+9, strlen(ref+9) );;
			}
		}

		if (( ref = strstr( line, "string_d" ))) {
			if (strlen(line)<=29){
			memcpy( &(objC->string_d), ref+9, strlen(ref+9) );
			}
			else if(strlen(line)>15){
				line[30] = '\0';
			memcpy( &(objC->string_d), ref+9, strlen(ref+9) );
			}

		}

		if (( ref = strstr( line, "string_b" ))) {
			if (strlen(line)<=29){
			memcpy( &(objC->string_b), ref+9, strlen(ref+9) );
			}
			else if(strlen(line)>15){
				line[30] = '\0';
			memcpy( &(objC->string_b), ref+9, strlen(ref+9) );
			}

		}
	}

	/* parse and set objC field values */

	return objC;
}


/*  Example 
struct D *upload_D( FILE *fp )
{
    char *line = NULL;
	// other declarations
	struct D *objD = (struct D *)malloc(sizeof(struct D));
	memset( objD, 0, sizeof(struct D) );

	while(1) {
		size = getline( &line, &len, fp );
		if ( strncmp( line, "\n", 1 ) == 0 )
			break;
       
                // replace \n at end with null-terminator for value
		line[strlen(line)-1] = '\0';   

		if (( ref = strstr( line, "num_a" ))) {
			objD->num_a = atoi( ref+6 );
		}

		if (( ref = strstr( line, "string_b" ))) {
			memcpy( &(objD->string_b), ref+9, strlen(ref+9) );
		}
	}


	return objD;
}
*/


// TASK 3: Linearize data for objects A, B, and C
// suitable for storing the objects in the key-value store and on-disk
unsigned char *marshall( struct A *objA )
{
	unsigned char *obj = (unsigned char *)malloc(OBJ_LEN);

	// Append each field value in order at end of buffer
	// Extend on example below

	memcpy( obj, &(objA->string_a), STRLEN );
	memcpy( obj+STRLEN, &(objA->num_b), sizeof(int) );
	
        memcpy( obj+STRLEN+sizeof(int), &(objA->ptr_c-> num_a), sizeof(int) );
        memcpy( obj+STRLEN+2*sizeof(int), &(objA->ptr_c-> num_b), sizeof(int));
        memcpy( obj+STRLEN+3*sizeof(int), &(objA->ptr_c-> string_c), STRLEN );    	
    memcpy( obj+2*STRLEN+3*sizeof(int), &(objA->ptr_c-> string_d), STRLEN );
    memcpy( obj+3*STRLEN+3*sizeof(int), &(objA->ptr_c-> string_e) , STRLEN );

    memcpy( obj+4*STRLEN+3*sizeof(int), &(objA->ptr_d-> num_a), sizeof(int) );    
    memcpy( obj+4*STRLEN+4*sizeof(int), &(objA->ptr_d-> string_b), STRLEN );
    memcpy( obj+5*STRLEN+4*sizeof(int), &(objA->ptr_d-> string_c), STRLEN );
    memcpy( obj+6*STRLEN+4*sizeof(int), &(objA->ptr_d-> string_d), STRLEN );     

	printf("Size of object = %lu\n", 
	       STRLEN +sizeof(int)
	       // sizeof A's fields that need to be stored - non-pointers
	       +sizeof(struct B)+sizeof(struct C));
 
	return obj;
}

/* 

   Example:

	memcpy( obj, &(objA->num_a), sizeof(int) );
	memcpy( obj+sizeof(int), &(objA->num_b), sizeof(int) );
	memcpy( obj+(2*sizeof(int)), &(objA->string_c), STRLEN ); 
	memcpy( obj+(2*sizeof(int))+STRLEN, &(objA->string_d), STRLEN ); 
	...

*/



// TASK 4: Convert linear layout of object data to structured layout
// Assign each element in the buffer to its field
struct A *unmarshall( unsigned char *obj )
{
	struct A *objA = (struct A *)malloc(sizeof(struct A));
	struct B *objB = (struct B *)malloc(sizeof(struct B));
	struct C *objC = (struct C *)malloc(sizeof(struct C));

	// find and assign field values for A, B, and C

	objA->ptr_c = objB;
	objA->ptr_d = objC; 

    memcpy(&(objC->string_d) , obj+6*STRLEN+4*sizeof(int), STRLEN );
    memcpy(&(objC->string_c) , obj+5*STRLEN+4*sizeof(int), STRLEN );
    memcpy(&(objC->string_b) , obj+4*STRLEN+4*sizeof(int), STRLEN );
    memcpy(&(objC->num_a) , obj+4*STRLEN+3*sizeof(int), sizeof(int) );

    memcpy(&(objB-> string_e), obj+3*STRLEN+3*sizeof(int), STRLEN );
    memcpy(&(objB-> string_d), obj+2*STRLEN+3*sizeof(int), STRLEN );
    memcpy(&(objB-> string_c), obj+STRLEN+3*sizeof(int), STRLEN );
    memcpy(&(objB-> num_b), obj+STRLEN+2*sizeof(int), sizeof(int) );
    memcpy(&(objB-> num_a), obj+STRLEN+sizeof(int), sizeof(int) );

    memcpy( &(objA->num_b),obj+STRLEN, sizeof(int) );	
    memcpy( &(objA->string_a),obj, STRLEN );
    

	// assign function pointers
	objA->ptr_c = objB;
        objA->ptr_d = objC;
	objA->op0 = &output_obj;
	objA->op1 = &marshall;


	return objA;
}


// TASK 5: Output first 4 fields of A, B, and C as output
// Fix to match your structures
// Example below
int output_obj( struct A *objA )
{
  
        printf("ObjA \n");
	// Base object fields
	printf("ObjA -> num_b: %d\n", objA->num_b );

        printf("ObjB \n");
	// First sub-object fields
	printf("ObjB -> num_a: %d\n", objA->ptr_c->num_a );
	printf("ObjB -> num_b: %d\n", objA->ptr_c->num_b );
	printf("ObjB -> string_c: %s\n", objA->ptr_c->string_c );
	printf("ObjB -> string_d: %s\n", objA->ptr_c->string_d );
	printf("ObjB -> string_e: %s\n", objA->ptr_c->string_e );


        printf("ObjC \n");
	// Last sub-object fields
	printf("ObjC -> num_a: %d\n", objA->ptr_d->num_a );
	printf("ObjC -> string_b: %s\n", objA->ptr_d->string_b );
	printf("ObjC -> string_c: %s\n", objA->ptr_d->string_c );
	printf("ObjC -> string_d: %s\n", objA->ptr_d->string_d );

	return 0;
}

