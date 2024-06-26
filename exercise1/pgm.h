#include <stdio.h>
#include <time.h>

#include <stdlib.h>


void generate_pgm_image_old(unsigned char *playground, int maxval, int k, const char *image_name);
void generate_pgm_image(unsigned char *playground, int maxval, int k, const char *image_name);
void write_pgm_image(void *image, int maxval, int xsize, int ysize, const char *image_name);
void read_pgm_image(void **image, int *maxval, int *xsize, int *ysize, const char *image_name);

// create a function that generates a pgm image from a given matrix of just 2 values 0 and 1 and saves it to a file
void generate_pgm_image_old(unsigned char *playground, int maxval, int k, const char *image_name) {
    unsigned char new_playground[k][k];
    unsigned char *p = &new_playground[0][0];
    for (int i = 0; i < k; i++){
        for (int j = 0; j < k; j++){
            if (playground[i*k + j] == 1) {
                new_playground[i][j] = 255;
            } else {
                new_playground[i][j] = 0;
            }
        }
    }
    write_pgm_image(p, maxval, k, k, image_name);
    return;
}

void generate_pgm_image(unsigned char *playground, int maxval, int k, const char *image_name) {
    char *cImage; 
    cImage = (char*)calloc( k * k, sizeof(char) );
    unsigned char _maxval = (char)maxval;
    int idx = 0;
    void *ptr;
    ptr = (void*)cImage;
    for (int i = 0; i < k; i++){
        for (int j = 0; j < k; j++){
            if (playground[i*k + j] == 1) {
                cImage[idx++]  = -1;
            } else {
                cImage[idx++]  = 0;
            }
        }
    }
    write_pgm_image(ptr, maxval, k, k, image_name);
    return;
}

void read_generated_pgm_image(unsigned char **playground, int *k, const char *filename) {
    unsigned char *image_data;
    int maxval;
    int xsize, ysize;
    read_pgm_image((void**)&image_data, &maxval, &xsize, &ysize, filename);

    if (image_data == NULL) {
        return;
    }

    *k = xsize;
    *playground = (unsigned char *)malloc(xsize * ysize * sizeof(unsigned char));
    if (!*playground) {
        perror("Unable to allocate memory for the playground");
        free(image_data);
        return;
    }

    int threshold = maxval / 2;

    for (int i = 0; i < xsize * ysize; i++) {
        if (image_data[i] > threshold) {
            (*playground)[i] = 1;
        } else {
            (*playground)[i] = 0;
        }
    }

    free(image_data);
}



void write_pgm_image( void *image, int maxval, int xsize, int ysize, const char *image_name)
/*
 * image        : a pointer to the memory region that contains the image
 * maxval       : either 255 or 65536
 * xsize, ysize : x and y dimensions of the image
 * image_name   : the name of the file to be written
 *
 */
{
  FILE* image_file; 
  image_file = fopen(image_name, "w"); 
  
  // Writing header
  // The header's format is as follows, all in ASCII.
  // "whitespace" is either a blank or a TAB or a CF or a LF
  // - The Magic Number (see below the magic numbers)
  // - the image's width
  // - the height
  // - a white space
  // - the image's height
  // - a whitespace
  // - the maximum color value, which must be between 0 and 65535
  //
  // if he maximum color value is in the range [0-255], then
  // a pixel will be expressed by a single byte; if the maximum is
  // larger than 255, then 2 bytes will be needed for each pixel
  //

  int color_depth = 1 + ( maxval > 255 );

  fprintf(image_file, "P5\n# generated by\n# put here your name\n%d %d\n%d\n", xsize, ysize, maxval);
  
  // Writing file
  fwrite( image, 1, xsize*ysize*color_depth, image_file);  

  fclose(image_file); 
  return ;

  /* ---------------------------------------------------------------

     TYPE    MAGIC NUM     EXTENSION   COLOR RANGE
           ASCII  BINARY

     PBM   P1     P4       .pbm        [0-1]
     PGM   P2     P5       .pgm        [0-255]
     PPM   P3     P6       .ppm        [0-2^16[
  
  ------------------------------------------------------------------ */
}


void read_pgm_image( void **image, int *maxval, int *xsize, int *ysize, const char *image_name)
/*
 * image        : a pointer to the pointer that will contain the image
 * maxval       : a pointer to the int that will store the maximum intensity in the image
 * xsize, ysize : pointers to the x and y sizes
 * image_name   : the name of the file to be read
 *
 */
{
  FILE* image_file; 
  image_file = fopen(image_name, "r"); 

  *image = NULL;
  *xsize = *ysize = *maxval = 0;
  
  char    MagicN[2];
  char   *line = NULL;
  size_t  k, n = 0;
  
  // get the Magic Number
  k = fscanf(image_file, "%2s%*c", MagicN );

  // skip all the comments
  k = getline( &line, &n, image_file);
  while ( (k > 0) && (line[0]=='#') )
    k = getline( &line, &n, image_file);

  if (k > 0)
    {
      k = sscanf(line, "%d%*c%d%*c%d%*c", xsize, ysize, maxval);
      if ( k < 3 )
	fscanf(image_file, "%d%*c", maxval);
    }
  else
    {
      *maxval = -1;         // this is the signal that there was an I/O error
			    // while reading the image header
      free( line );
      return;
    }
  free( line );
  
  int color_depth = 1 + ( *maxval > 255 );
  unsigned int size = *xsize * *ysize * color_depth;
  
  if ( (*image = (char*)malloc( size )) == NULL )
    {
      fclose(image_file);
      *maxval = -2;         // this is the signal that memory was insufficient
      *xsize  = 0;
      *ysize  = 0;
      return;
    }
  
  if ( fread( *image, 1, size, image_file) != size )
    {
      free( image );
      image   = NULL;
      *maxval = -3;         // this is the signal that there was an i/o error
      *xsize  = 0;
      *ysize  = 0;
    }  

  fclose(image_file);
  return;
}

