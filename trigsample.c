/****************************************************************************
 * TrigSample
 *
 * Copyright (C) Jean Zundel <jzu@free.fr> 2015 
 *
 * Doubles a signal with another in a separate WAV file.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 *****************************************************************************/

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
      max,
      insize,
      smpsize,
      fin,
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

  // Use the size of input and sample files to malloc buffers

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

  // Various checks

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

  // Make an empty WAV structure

  memset (bout, 0, insize);
  memcpy (bout, bin, 44);                      // WAV header

  // If we get a powerful enough signal
  // Find the maximum amplitude of that signal
  // Copy the sample 
  // Adjust the copy wrt the original signal amplitude
  // Print a dot

  i = 22;
  while (i < (insize - smpsize - 22) / 2) { 
    if (bin [i] > threshold) {
      max = 0;
      for (j = i; j < i+500; j++)
        max = (abs (bin [j]) > max) 
              ? abs (bin [j]) 
              : max;
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

