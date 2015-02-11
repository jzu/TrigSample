#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int main (int argc, char **argv) {

  struct stat instat, 
              smpstat;
  int i, 
      j,
      max;
  int insize,
      smpsize;
  int fin,
      fsmp,
      fout;
  short *bin,
        *bsmp,
        *bout;
  short threshold;

  char *riff = "RIFF";
  char *wave = "WAVE";

  if (argc != 5) {
    printf ("Usage: %s input sample output threshold\n", argv [0]);
    return 1;
  }

  if ((threshold = atoi (argv [4])) == 0) {
    printf ("Usage: %s input sample output threshold\n", argv [0]);
    return 1;
  }

  if ((stat (argv [1], &instat)) < 0) {
    printf ("Error: stat %s\n", argv [1]);
    return 2;
  }
  if ((stat ("sample.wav", &smpstat)) < 0) {    
    printf ("Error: stat %s\n", argv [2]);
    return 2;
  }

  insize = instat.st_size;
  smpsize = smpstat.st_size;

  bin  = malloc ((size_t) insize);
  bsmp = malloc ((size_t) smpsize);
  bout = malloc ((size_t) insize);

  fin  = open (argv [1], O_RDONLY);
  fsmp = open (argv [2], O_RDONLY);

  if ((fout = creat (argv [3], S_IWUSR|S_IRUSR)) <= 0) {
    printf ("Error creating %s\n", argv [3]);
    return 3;
  }
  read (fin, bin, insize);
  read (fsmp, bsmp, smpsize);

  if ((memcmp (riff, bin,   4) != 0) || 
      (memcmp (wave, bin+4, 4) != 0) ||
      (bin [10] != 1)) {
    printf ("Not a mono WAV: %s\n", argv [1]);
    return 4;
  }

  if ((memcmp (riff, bsmp,  4) != 0) || 
      (memcmp (wave, bin+4, 4) != 0) ||
      (bsmp [10] != 1)) {
    printf ("Not a mono WAV: %s\n", argv [2]);
    return 4;
  }

  if (memcmp (bin+10, bsmp+10, 16) != 0) {
    printf ("%s and %s should share the same characteristics\n", 
            argv [0], 
            argv [1]);
    return 5;
  }


  memset (bout, 0, insize);
  memcpy (bout, bin, 44);                      // WAV header

  i = 22;
  while (i < (insize - smpsize - 22) / 2) { 
    if (bin [i] > threshold) {
      max = 0;
      for (j = i; j < i+500; j++)
        max = (bin [j] > max) ? bin [j] : max;
      memcpy (bout+i, bsmp + 22, smpsize - 44);
      for (j = i; j < i + (smpsize - 22) / 2; j++)
        bout [j] = (short) (((int) bout [j] * max) / 32768);
      printf (".");
      i += 500;
    }
    i++;
}

  printf ("\n");

  write (fout, bout, insize);

  close (fout);
  close (fsmp);
  close (fin);
 
  return 0;
}

