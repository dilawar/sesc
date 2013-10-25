/*
 * Common definitions used by other header files
 */

#ifndef __common_h
#define __common_h

typedef int32_t (*PFI)(void *,...);
typedef void (*PFV)(void *,...);

void fatal(const char *s, ...);
void error(const char *fmt, ...);
void warning(const char *fmt, ...);

#endif /* !__common_h */
