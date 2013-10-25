#ifndef _EMULINIT_H_
#define _EMULINIT_H_
void fail(const char *fmt, ...)  __attribute__ ((noreturn));
void emulInit(int32_t argc, char **argv, char **envp);
#endif // _EMULINIT_H_
