#ifndef MAIN_H
#define MAIN_H

#include "m64p_plugin.h"
#include "m64p_common.h"
#include "m64p_config.h"

#ifdef _WIN32
#define DLSYM(a, b) GetProcAddress(a, b)
#else
#include <dlfcn.h>
#define DLSYM(a, b) dlsym(a, b)
#endif

extern void (*debug_callback)(void *, int, const char *);
extern void *debug_callback_context;

extern ptr_ConfigOpenSection ConfigOpenSection;
extern ptr_ConfigSaveSection ConfigSaveSection;
extern ptr_ConfigSetDefaultInt ConfigSetDefaultInt;
extern ptr_ConfigSetDefaultBool ConfigSetDefaultBool;
extern ptr_ConfigGetParamInt ConfigGetParamInt;
extern ptr_ConfigGetParamBool ConfigGetParamBool;
extern ptr_ConfigSetParameter ConfigSetParameter;

#endif // MAIN_H
