#ifndef _auth_h_
#define _auth_h_

int get_access_token(
    char *result, int length, char *username, char *identity_token);

#endif
