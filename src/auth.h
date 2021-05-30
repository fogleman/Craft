#ifndef _auth_h_
#define _auth_h_

#ifdef _MSC_VER
	#define snprintf _snprintf
#endif

int get_access_token(
    char *result, int length, char *username, char *identity_token);

#endif
