#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "cmpsc473-util.h"

int buffer_from_file(char *filepath, unsigned char **buf)
{
	int err;
	struct stat *statbuf;
	FILE *fptr;
	size_t filesize;

	statbuf = (struct stat *)malloc(sizeof(struct stat));
	assert( statbuf != NULL );

	err = stat( filepath, statbuf );

	/* if file does not exist ... */
	if ( err != 0 ) {
		filesize = 0;
	}
	/* if file is there but empty */
	else if (!( filesize = statbuf->st_size ));
	/* else if file exists */
	else {
		/* Get file size */
		assert( filesize > 0 );

		/* Read file data into buf */
		*buf = (unsigned char *)malloc(filesize); 
		assert( *buf != NULL );
  
		fptr = fopen( filepath, "r" );
		if ( fptr != NULL ) {
			err = fread( *buf, 1, filesize, fptr );
			assert( err == filesize ); 
		}
		fclose( fptr );
	}

	free( statbuf );
  
	return filesize;
}
