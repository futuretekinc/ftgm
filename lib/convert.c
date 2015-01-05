/**
* UTF8Toascii:
* @out:  a pointer to an array of bytes to store the result
* @outlen:  the length of @out
* @in:  a pointer to an array of UTF-8 chars
* @inlen:  the length of @in
*
* Take a block of UTF-8 chars in and try to convert it to an ASCII
* block of chars out.
*
* Returns 0 if success, -2 if the transcoding fails, or -1 otherwise
* The value of @inlen after return is the number of octets consumed
*     if the return value is positive, else unpredictable.
* The value of @outlen after return is the number of octets consumed.
*/
#include <stdlib.h>
int
UTF8Toascii
(
	unsigned char* out, int *outlen,
	const unsigned char* in, int *inlen
) 
{
		const unsigned char* processed = in;
		const unsigned char* outend;
		const unsigned char* outstart = out;
		const unsigned char* instart = in;
		const unsigned char* inend;
		unsigned int c, d;
		int trailing;

		if (in == NULL) {
				/*
				 * initialization nothing to do
				 */
				*outlen = 0;
				*inlen = 0;
				return(0);
		}
		inend = in + (*inlen);
		outend = out + (*outlen);
		while (in < inend) {
				d = *in++;
				if      (d < 0x80)  { c= d; trailing= 0; }
				else if (d < 0xC0) {
						/* trailing byte in leading position */
						*outlen = out - outstart;
						*inlen = processed - instart;
						return(-2);
				} else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
				else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
				else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
				else {
						/* no chance for this in Ascii */
						*outlen = out - outstart;
						*inlen = processed - instart;
						return(-2);
				}

				if (inend - in < trailing) {
						break;
				} 

				for ( ; trailing; trailing--) {
						if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
								break;
						c <<= 6;
						c |= d & 0x3F;
				}

				/* assertion: c is a single UTF-4 value */
				if (c < 0x80) {
						if (out >= outend)
								break;
						*out++ = c;
				} else {
						/* no chance for this in Ascii */
						*outlen = out - outstart;
						*inlen = processed - instart;
						return(-2);
				}
				processed = in;
		}
		*outlen = out - outstart;
		*inlen = processed - instart;
		return(0);
}

