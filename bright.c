/*
 * Copyright (c) 2010, David Kelso <david at kelso dot id dot au>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>

// How far the brightness should move with each step.
#define JUMP 0xa

int main(int argc, char *argv[]) {
  unsigned int value, location[3];
  int success;
  char command[100];
  FILE *fp;

  if (geteuid()) {
    printf("Must be root\n");
    return 1;
  }

  if (argc != 2) {
    printf("One argument required: ++ or --\n");
    return 1;
  }

  // Get the PCI location of the VGA adapter
  fp = popen("lspci | grep VGA", "r");
  success = fscanf(fp, "%20d:%2d.%1d VGA", &location[0], &location[1], &location[2]);
  pclose(fp);

  if (!success) {
    printf("lspci couldn't find your vga adapter\n");
    return 1;
  }


  // Get the existing value of the brightness
  snprintf(command, sizeof command, "setpci -s %02d:%02d.%01d F4.B", 
      location[0], location[1], location[2]);
  fp = popen(command, "r");
  success = fscanf(fp, "%x", &value);
  pclose(fp);

  if (!success) {
    printf("setpci didn't work\n");
    return 1;
  }

  // Increase or decrease the existing brightness value
  if (strcmp(argv[1], "++") == 0) {
    value += JUMP;
    if (value > 0xff) value = 0xff;
  } else if (strcmp(argv[1], "--") == 0) {
    if (value < JUMP) value = 0;
    else value -= JUMP;
  } else {
    printf("++ to increase, -- to decrease\n");
    return 1;
  }

  // Set the new brightness value
  snprintf(command, sizeof command, "setpci -s %02d:%02d.%01d F4.B=%x", 
      location[0], location[1], location[2], value);
  system(command);

  return 0;
}
