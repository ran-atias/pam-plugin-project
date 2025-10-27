#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char** environ;  // Needed for execv()

__attribute__((constructor))
void preload_check() {
    fprintf(stdout, "[HOOK] preload_check triggered\n");
    fflush(stdout);
}

// Hook for execve
int execve(const char *pathname, char *const argv[], char *const envp[]) {
    static int (*real_execve)(const char *, char *const[], char *const[]) = NULL;
    fprintf(stdout, "[HOOK] execve called: %s\n", pathname);
    
    if (!real_execve) {
        real_execve = dlsym(RTLD_NEXT, "execve");
        if (!real_execve) {
            exit(1);
        }
    }

    fprintf(stdout, "[HOOK] execve called: %s\n", pathname);
    for (int i = 0; argv && argv[i]; ++i) {
        fprintf(stdout, "  argv[%d]: %s\n", i, argv[i]);
    }
    
    fflush(stdout);
    
    // TODO here we can print the child pid or do whatever we want
    
	// Invoke the real execve
    return real_execve(pathname, argv, envp);
}

// Hook for execv (uses environ)
int execv(const char *pathname, char *const argv[]) {
    return execve(pathname, argv, environ);
}
