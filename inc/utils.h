#ifndef _UTILS__H
#define _UTILS__H

#include <stdlib.h>
#include <unistd.h>
#include <cstdio>


static void logError(const char* ctxt, const char* msg) {
   fprintf( stderr, "*** CptnHookLite ERROR in %s: %s\n", ctxt, msg);
}

static void logInfo(const char* ctxt, const char* msg) {
   fprintf( stderr, "*** CptnHookLite INFO in %s: %s\n", ctxt, msg);
}

#endif
