
#include <stdlib.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
	int b64_decode_ex(u_char *bufplain, const char *bufcoded, int buflen);
	int b64_encode(u_char *buf,char *out,int inLen) ;
#ifdef __cplusplus
}
#endif
