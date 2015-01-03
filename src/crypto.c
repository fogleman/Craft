#include <openssl/sha.h>
#include <string.h>
#include <stdio.h>
#include "crypto.h"

static void binary_to_hex(char *in, char *out) {
  for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
    snprintf(out+i*2, 3, "%.2X", (unsigned char)in[i]);
  }
}

void hash_password(char *data, char *out) {
  int length = strlen(data);
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1(data, length, hash);
  binary_to_hex(hash, out);
}
