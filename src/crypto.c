#include <string.h>
#include <stdio.h>
#include "crypto.h"

/*
static void binary_to_hex(char *in, char *out) {
  for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
    snprintf(out+i*2, 3, "%.2X", (unsigned char)in[i]);
  }
}
*/

void hash_password(char *data, char *out) {
  // I removed OpenSSL because I'm not sure what lib we are
  // ending up with in the end. We will run with plain text
  // for now :)
  // TODO: Add a hash function and call this function in main.c
}
