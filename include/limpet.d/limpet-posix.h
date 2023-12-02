/*
 * Common functions for POSIX-like operating systems.
 */

#ifndef _LIMPET_POSIX_H_
#define _LIMPET_POSIX_H_

#define __LIMPET_DEFINE_SIGNAME(signame) \
    {.number = SIG ## signame, .name = #signame}

static void __limpet_print_signame(int sig) {
    static const struct {
        int         number;
        const char* name;
    } signames[] = {
        __LIMPET_DEFINE_SIGNAME(ABRT),
        __LIMPET_DEFINE_SIGNAME(ALRM),
        __LIMPET_DEFINE_SIGNAME(BUS),
        __LIMPET_DEFINE_SIGNAME(CHLD),
        __LIMPET_DEFINE_SIGNAME(CLD),
        __LIMPET_DEFINE_SIGNAME(CONT),
        __LIMPET_DEFINE_SIGNAME(FPE),
        __LIMPET_DEFINE_SIGNAME(HUP),
        __LIMPET_DEFINE_SIGNAME(ILL),
        __LIMPET_DEFINE_SIGNAME(INT),
        __LIMPET_DEFINE_SIGNAME(IO),
        __LIMPET_DEFINE_SIGNAME(IOT),
        __LIMPET_DEFINE_SIGNAME(KILL),
        __LIMPET_DEFINE_SIGNAME(PIPE),
        __LIMPET_DEFINE_SIGNAME(POLL),
        __LIMPET_DEFINE_SIGNAME(PROF),
        __LIMPET_DEFINE_SIGNAME(PWR),
        __LIMPET_DEFINE_SIGNAME(QUIT),
        __LIMPET_DEFINE_SIGNAME(SEGV),
        __LIMPET_DEFINE_SIGNAME(STKFLT),
        __LIMPET_DEFINE_SIGNAME(STOP),
        __LIMPET_DEFINE_SIGNAME(TSTP),
        __LIMPET_DEFINE_SIGNAME(SYS),
        __LIMPET_DEFINE_SIGNAME(TERM),
        __LIMPET_DEFINE_SIGNAME(TRAP),
        __LIMPET_DEFINE_SIGNAME(TTIN),
        __LIMPET_DEFINE_SIGNAME(TTOU),
        __LIMPET_DEFINE_SIGNAME(URG),
        __LIMPET_DEFINE_SIGNAME(USR1),
        __LIMPET_DEFINE_SIGNAME(USR2),
        __LIMPET_DEFINE_SIGNAME(VTALRM),
        __LIMPET_DEFINE_SIGNAME(XCPU),
        __LIMPET_DEFINE_SIGNAME(XFSZ),
        __LIMPET_DEFINE_SIGNAME(WINCH),
    };
    size_t i;

    for (i = 0; i < __LIMPET_ARRAY_SIZE(signames); i++) {
        if (signames[i].number == sig) {
            break;
        }
    }

    if (i == __LIMPET_ARRAY_SIZE(signames)) {
        __limpet_printf("signal unknown (%d)", sig);
    } else {
        __limpet_printf("signal SIG%s (%d)", signames[i].name, sig);
    }
}

/*
 * We don't need this defined from this point on, so drop it from the
 * namespace
 */
#undef __LIMPET_DEFINE_SIGNAME

static void __limpet_print_status(struct __limpet_test *test) {
    int status = test->sysdep.exit_status;

    if (test->sysdep.timedout) {
        __limpet_printf("timed out after %g seconds: FAILURE",
            test->params->timeout);
    } else if (WIFEXITED(status)) {
        int exit_status;

        exit_status = WEXITSTATUS(status);

        if (exit_status == 0) {
            __limpet_printf("exit code %d: SUCCESS", exit_status);
        } else {
            __limpet_printf("exit code %d: FAILURE", exit_status);
        }
    } else if (WIFSIGNALED(status)) {
        int sig;
        const char *core_dumped;

        sig = WTERMSIG(status);
        __limpet_print_signame(sig);
#ifdef WCOREDUMP
        core_dumped = WCOREDUMP(status) ? " (core dumped)" : "";
#else
        core_dumped = "";
#endif

        __limpet_printf("%s: FAILURE", core_dumped);
    } else if (WIFSTOPPED(status)) {
        int sig;

        sig = WSTOPSIG(status);
        __limpet_printf("stopped, ");
        __limpet_print_signame(sig);
        __limpet_printf(": FAILURE");
    } else if (WIFCONTINUED(status)) {
        __limpet_printf("continued: %s\n", "FAILURE");
    } else {
        __limpet_printf("unknown reason: %s", "FAILURE");
    }
}   

/*
 * Printing and exit functions
 */
#define __limpet_fail_with(err, fmt, ...)    do {        \
        __limpet_fail(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __limpet_fail_errno(fmt, ...)    do {        \
        __limpet_fail_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)

#define __limpet_warn_with(err, fmt, ...)    do {        \
        __limpet_warn(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __limpet_warn_errno(fmt, ...)    do {        \
        __limpet_warn_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)

static void __limpet_fail(const char *fmt, ...) __attribute((noreturn));
static void __limpet_fail(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    __limpet_exit(true);
}

static void __limpet_warn(const char *fmt, ...) __attribute((unused));
static void __limpet_warn(const char *fmt, ...) {
    va_list ap;

    printf("Warning: ");
    va_start(ap, fmt);  
    vprintf(fmt, ap);
    va_end(ap);
}

static void __limpet_printf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
#endif /* _LIMPET_POSIX_H_ */
