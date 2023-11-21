/*
 * Definitions for the Linux-based Leavemein
 */

#ifndef _LEAVEIN_TEST_LINUX_H_
#define _LEAVEIN_TEST_LINUX_H_

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <pty.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/*
 * System-dependent per-test information.
 * log_fd - file descriptor for the log file.
 * raw_pty - the master side of a pseudoterminal
 * tty_pty - the slave side of a pseudoterminal
 * thread - the thread for a test that is responsible for forking and waiting
 *      for the process whose process is in pid
 * timedout - true if the process timed out
 * exit_status - If we fail and have an errno value, it will be stored here.
 * pid - the process ID of the subprocess that actually runs the test
 * mutex - Mutex guarding the pid element of this structure
 * cond - Condition variable guarding the pid element of this structure
 */
struct __leavemein_sysdep {
    int                         log_fd;
    int                         raw_pty;
    int                         tty_pty;
    pthread_t                   thread;
    bool                        timedout;
    int                         exit_status;
    pid_t                       pid;
};

#include "leavemein-sysdep.h"

/*
 * Value to use for __leavemein_sysdep initiatization
 */
#define __LEAVEMEIN_SYSDEP_INIT         {   \
        .log_fd = -1,                       \
        .raw_pty = -1,                      \
        .tty_pty = -1,                      \
        .thread = 0,                        \
        .timedout = false,                  \
        .exit_status = 0,                   \
        .pid = -1,                          \
    }

#define __LEAVEMEIN_ARRAY_SIZE(a)   (sizeof(a) / sizeof ((a)[0]))

static void __leavemein_exit(bool is_error) __attribute((noreturn));
static void __leavemein_exit(bool is_error) {
    close(0);
    close(1);
    close(2);

    exit(is_error ? EXIT_FAILURE : EXIT_SUCCESS);
}

/*
 * print an error message like printf()
 */
static void __leavemein_fail(const char *fmt, ...) __attribute((noreturn));
static void __leavemein_fail(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    __leavemein_exit(true);
}

static void __leavemein_warn(const char *fmt, ...) {
    va_list ap;

    printf("Warning: ");
    va_start(ap, fmt);  
    vprintf(fmt, ap);
    va_end(ap);
}

static void __leavemein_printf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

#define __leavemein_fail_with(err, fmt, ...)    do {        \
        __leavemein_fail(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __leavemein_fail_errno(fmt, ...)    do {        \
        __leavemein_fail_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)

#define __leavemein_warn_with(err, fmt, ...)    do {        \
        __leavemein_warn(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __leavemein_warn_errno(fmt, ...)    do {        \
        __leavemein_warn_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)

/*
 * Define initialization for a structure holding a mutex
 */
#define __LEAVEMEIN_MUTEX_INIT   { .mutex = PTHREAD_MUTEX_INITIALIZER, }

struct __leavemein_mutex {
    pthread_mutex_t mutex;
};

/*
 * Define initialize for a struct with a conditional variable and a mutext
 */
#define __LEAVEMEIN_COND_INIT   { .cond = PTHREAD_COND_INITIALIZER, }

struct __leavemein_cond {
    pthread_cond_t  cond;
};

static void __leavemein_mutex_init(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_init(&mutex->mutex, NULL);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to initialize mutex");
    }
}

static void __leavemein_mutex_lock(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_lock(&mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to lock mutex");
    }
}

static void __leavemein_mutex_unlock(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_unlock(&mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to unlock mutex");
    }
}

static void __leavemein_cond_init(struct __leavemein_cond *cond) {
    int rc;

    rc = pthread_cond_init(&cond->cond, NULL);
    if (rc != 0) {
        __leavemein_fail_with(rc,
            "Failed to initialize conditional variable");
    }
}

static void __leavemein_cond_signal(struct __leavemein_cond *cond) {
    int rc;

    rc = pthread_cond_signal(&cond->cond);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to signal conditional variable");
    }
}

static void __leavemein_cond_wait(struct __leavemein_cond *cond,
    struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_cond_wait(&cond->cond, &mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to wait conditional variable");
    }
}

/*
 * Environment variables available to configure executation are:
 * LEAVEMEIN_MAX_JOBS   Specifies the maximum number of threads running at a
 *      time. If this is not set or is zero, there is no limit
 * LEAVEMEIN_RUNLIST    A list of names of tests to be run, separated by
 *      colons. If this is not set, all tests will be run. Note: If this is
 *      set to an empty string, no tests will be run
 */
#define __LEAVEMEIN_MAX_JOBS  "LEAVEMEIN_MAX_JOBS"
#define __LEAVEMEIN_RUNLIST   "LEAVEMEIN_RUNLIST"
#define __LEAVEMEIN_TIMEOUT   "LEAVEMEIN_TIMEOUT"

/*
 * List of all environment variables to eliminate before running the test
 */
static const char *__leavemein_envvars[] = {
    __LEAVEMEIN_MAX_JOBS,
    __LEAVEMEIN_RUNLIST,
    __LEAVEMEIN_TIMEOUT,
};

static const char *__leavemein_get_maxjobs(void) {
    return getenv(__LEAVEMEIN_MAX_JOBS);
}

static const char *__leavemein_get_runlist(void) {
    return getenv(__LEAVEMEIN_RUNLIST);
}

static const char *__leavemein_get_timeout(void) {
    return getenv(__LEAVEMEIN_TIMEOUT);
}

/*
 * Remove things in the environment specific to leavmein
 */
static void __leavemein_parse_done(void)
{
    size_t i;

    for (i = 0; i < __LEAVEMEIN_ARRAY_SIZE(__leavemein_envvars); i++) {
        const char *envvar;
        envvar = __leavemein_envvars[i];

        if (unsetenv(envvar) == -1) {
            __leavemein_fail("Unable to unset environment variable %s\n",
                envvar);
        }
    }
}

/*
 * Create a pseudoterminal so that the test can play with stdin, stdout,
 * and stderr.
 */
static bool __leavemein_make_pty(struct __leavemein_sysdep *sysdep)
    __LEAVEMEIN_UNUSED;
static bool __leavemein_make_pty(struct __leavemein_sysdep *sysdep) {
    struct termios termios;
    struct winsize winsize;
    int rc;

    rc = tcgetattr(0, &termios);
    if (rc == -1) {
        __leavemein_fail_errno("Unable to get terminal characteristics");
    }

    rc = ioctl(0, TIOCGWINSZ, &winsize);
    if (rc == -1) {
        __leavemein_fail_errno("Unable to get windows size");
    }

    rc = openpty(&sysdep->raw_pty, &sysdep->tty_pty, NULL, &termios,
        &winsize);
    if (rc == -1) {
        __leavemein_fail_errno("Unable to create pty");
    }

    return true;
}

static bool __leavemein_thread_setup(struct __leavemein_test *test) {
    static const char tmpfile_name_template[] = "/tmp/logfileXXXXXX";
    char tmpfile_name[sizeof(tmpfile_name_template)];

    test->sysdep.log_fd = open(tmpfile_name_template,
        O_TMPFILE | O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

    if (test->sysdep.log_fd == -1 && errno == EINVAL) {
        strncpy(tmpfile_name, tmpfile_name_template, sizeof(tmpfile_name));
        test->sysdep.log_fd = mkstemp(tmpfile_name);
    }

    if (test->sysdep.log_fd == -1) {
        __leavemein_fail_errno("Unable to make log file");
    }

    if (!__leavemein_make_pty(&test->sysdep)) {
        return false;
    }

    return true;
}

/*
 * Set up the new test process
 */
static void __leavemein_setup_one(struct __leavemein_sysdep *sysdep)
    __LEAVEMEIN_UNUSED;
static void __leavemein_setup_one(struct __leavemein_sysdep *sysdep) {
    if (dup2(sysdep->tty_pty, 0) == -1) {
        __leavemein_fail_errno("dup2(%d, %d)", sysdep->tty_pty, 0);
    }
    if (dup2(0, 1) == -1) {
        __leavemein_fail_errno("dup2(%d, %d)", 0, 1);
    }
    if (dup2(0, 2) == -1) {
        __leavemein_fail_errno("dup2(%d, %d)", 0, 2);
    }

    if (close(sysdep->tty_pty) == -1) {
        __leavemein_fail_errno("close(tty_pty %d)", sysdep->tty_pty);
    }
    if (close(sysdep->raw_pty) == -1) {
        __leavemein_fail_errno("close(raw_pty %d)", sysdep->raw_pty);
    }
    if (close(sysdep->log_fd) == -1) {
        __leavemein_fail_errno("close(raw_pty %d)", sysdep->log_fd);
    }
}
    
/*
 * Copy from one file descriptor to another
 * in_fd - Input file descriptor
 * out_fd - Output file descriptor
 *
 * Returns: The number of characters written or -1 on error.
 */
static ssize_t __leavemein_copy_file(int in_fd, int out_fd)
    __LEAVEMEIN_UNUSED;
static ssize_t __leavemein_copy_file(int in_fd, int out_fd) {
    char buf[4096];
    ssize_t zrc = 0;
    ssize_t total = 0;

    for (zrc = read(in_fd, buf, sizeof(buf)); zrc > 0;
        zrc = read(in_fd, buf, sizeof(buf))) {
        zrc = write(out_fd, buf, zrc);
        if (zrc < 0) {
            return -1;
        }
        total += zrc;
    }

    if (zrc != 0) {
        return -1;
    }

    return total;
}

/*
 * Copy output into the log and wait for the process to terminate
 */
static void __leavemein_log_and_wait(struct __leavemein_test *test) {
    enum { io_read, io_write, io_eof } io_state;
    enum { proc_running, proc_killed, proc_reaped } proc_state;
    struct timeval timeout;
    struct timeval abs_timeout;
    fd_set rfds;
    fd_set wfds;
    char buf[4096];
    int nfds;
    int pid_fd;
    int rc;

    timeout.tv_sec = (time_t)test->params->timeout;
    timeout.tv_usec = (suseconds_t)((test->params->timeout - timeout.tv_sec) *
        1000000);
    rc = gettimeofday(&abs_timeout, NULL);
    if (rc == -1) {
        __leavemein_fail_errno("gettimeofday failed");
    }
    timeradd(&abs_timeout, &timeout, &abs_timeout);

    /*
     * Get a file descriptor for this pid so we can use select() to wait for
     * it to exit
     */
    pid_fd = syscall(SYS_pidfd_open, test->sysdep.pid, 0);
    if (pid_fd == -1) {
        __leavemein_fail_errno("pidfd_open failed for pid %d",
            test->sysdep.pid);
    }

    io_state = io_read;
    proc_state = proc_running;

    do {
        struct timeval delta_timeout;
        struct timeval now;
        struct timeval *tv;
        ssize_t zrc;

        nfds = 0;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        switch (proc_state) {
        case proc_running:
            FD_SET(pid_fd, &rfds);
            nfds = MAX(nfds, pid_fd + 1);

            rc = gettimeofday(&now, NULL);
            if (rc == -1) {
                __leavemein_fail("gettimeofday failed");
            }
            if (timercmp(&abs_timeout, &now, >)) {
                timersub(&abs_timeout, &now, &delta_timeout);
            } else {
                timerclear(&delta_timeout);
            }
            tv = &delta_timeout;
            break;

        case proc_killed:
            FD_SET(pid_fd, &rfds);
            nfds = MAX(nfds, pid_fd + 1);
            tv = NULL;
            break;

        case proc_reaped:
            tv = NULL;
            break;
        }

        switch (io_state) {
        case io_read:
            FD_SET(test->sysdep.raw_pty, &rfds);
            nfds = MAX(nfds, test->sysdep.raw_pty + 1);
            break;

        case io_write:
            FD_SET(test->sysdep.log_fd, &wfds);
            nfds = MAX(nfds, test->sysdep.log_fd + 1);
            break;

        case io_eof:
            break;
        }

        rc = select(nfds, &rfds, &wfds, NULL, tv);
        if (rc == -1) {
            __leavemein_fail_errno("Select failed");
        }

        switch (proc_state) {
        case proc_running:
            if (FD_ISSET(pid_fd, &rfds)) {
                rc = waitpid(test->sysdep.pid, &test->sysdep.exit_status, 0);
                if (rc == -1) {
                    __leavemein_fail_errno("waitpid failed");
                }
                proc_state = proc_reaped;
            } else if (rc == 0) {
                test->sysdep.timedout = true;
                rc = kill(test->sysdep.pid, SIGKILL);
                if (rc == -1) {
                    __leavemein_warn("Unable to kill PID %d\n",
                        test->sysdep.pid);
                }
                proc_state = proc_killed;
            }
            break;

        case proc_killed:
            if (FD_ISSET(pid_fd, &rfds)) {
                rc = waitpid(test->sysdep.pid, &test->sysdep.exit_status, 0);
                if (rc == -1) {
                    __leavemein_fail_errno("waitpid failed");
                }
                proc_state = proc_reaped;
            }
            break;

        case proc_reaped:
            break;
        }
            
        switch (io_state) {
        case io_read:
            if (FD_ISSET(test->sysdep.raw_pty, &rfds)) {
                zrc = read(test->sysdep.raw_pty, buf, sizeof(buf));
                switch (zrc) {
                case -1:
                    /*
                     * I would have expected a zero return when the other end
                     * of the pseudoterminal was closed, but seem to get
                     * this
                     */
                    if (errno != EIO) {
                        __leavemein_fail_errno("raw pty read failed");
                    }
                    io_state = io_eof;
                    break;

                case 0:
                    io_state = io_eof;
                    break;

                default:
                    io_state = io_write;
                    break;
                }
            }
            break;

        case io_write:
            if (FD_ISSET(test->sysdep.log_fd, &wfds)) {
                zrc = write(test->sysdep.log_fd, buf, zrc);
                if (zrc == -1) {
                    __leavemein_fail_errno("log write failed");
                }
                io_state = io_read;
            }
            break;

        case io_eof:
            break;
        }
    } while (proc_state != proc_reaped || io_state != io_eof);
}

/*
 * This dumps the log file to standard out.
 *
 * Returns true if there was something to print, false otherwise
 */
static ssize_t __leavemein_dump_log(struct __leavemein_test *test) {
    ssize_t total;

    if (lseek(test->sysdep.log_fd, 0, SEEK_SET) == (off_t) -1) {
        __leavemein_fail_errno("lseek failed on fd %d", test->sysdep.log_fd);
    }

    total = __leavemein_copy_file(test->sysdep.log_fd, 1);
    if (total == -1) {
        __leavemein_fail_errno("Log dump failed");
    }

    if (close(test->sysdep.log_fd) != 0) {
        __leavemein_warn_errno("Close of log file failed");
    }

    return total;
}

/*
 * Run the test as a subprocess
 */
static void *__leavemein_run_one(void *arg) __LEAVEMEIN_UNUSED;
static void *__leavemein_run_one(void *arg) {
    struct __leavemein_test *test = (struct __leavemein_test *)arg;
    pid_t pid;

    __leavemein_thread_setup(test);

    if (fflush(stdout) == -1) {
        __leavemein_fail_errno("fflush(stdout) failed");
    }

    pid = fork();
    switch (pid) {
    case -1:
        __leavemein_fail_errno("Fork failed");
        break;

    case 0:
        __leavemein_setup_one(&test->sysdep);
        (*test->func)();
        __leavemein_exit(false);
        break;

    default:

        /*
         * Set the process ID and let the wait begin
         */
        if (close(test->sysdep.tty_pty) == -1) {
            __leavemein_fail_errno("close(test->tty_pty) failed");
        }
        test->sysdep.pid = pid;
        break;
    }

    __leavemein_log_and_wait(test);

    if (test->sysdep.timedout) {
        __leavemein_inc_failed();
        __leavemein_enqueue_done(test);
    } else if (!WIFEXITED(test->sysdep.exit_status) ||
        WEXITSTATUS(test->sysdep.exit_status) != 0) {
            __leavemein_inc_failed();
            __leavemein_enqueue_done(test);
    } else {
            __leavemein_inc_passed();
            __leavemein_enqueue_done(test);
    }

    return test;
}

/*
 * Run one test
 *  test - Pointer to a __leavemein_params for the test to run
 * Returns: true if the test was started, false otherwise
 */
static bool __leavemein_start_one(struct __leavemein_test *test) {
    int rc;

    rc = pthread_create(&test->sysdep.thread, NULL, __leavemein_run_one,
        test);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to create thread");
    }

    return true;
}

/*
 * Wait for a thread and clean things up
 */
static void __leavemein_cleanup_test(struct __leavemein_test *test) {
    int rc;

    rc = pthread_join(test->sysdep.thread, NULL);
    if (rc != 0) {
        __leavemein_fail_with(rc, "pthread_join failed");
    }
}

#define __LEAVEMEIN_DEFINE_SIGNAME(signame) \
    {.number = SIG ## signame, .name = #signame}

static void __leavemein_print_signame(int sig) {
    static const struct {
        int         number;
        const char* name;
    } signames[] = {
        __LEAVEMEIN_DEFINE_SIGNAME(ABRT),
        __LEAVEMEIN_DEFINE_SIGNAME(ALRM),
        __LEAVEMEIN_DEFINE_SIGNAME(BUS),
        __LEAVEMEIN_DEFINE_SIGNAME(CHLD),
        __LEAVEMEIN_DEFINE_SIGNAME(CLD),
        __LEAVEMEIN_DEFINE_SIGNAME(CONT),
        __LEAVEMEIN_DEFINE_SIGNAME(FPE),
        __LEAVEMEIN_DEFINE_SIGNAME(HUP),
        __LEAVEMEIN_DEFINE_SIGNAME(ILL),
        __LEAVEMEIN_DEFINE_SIGNAME(INT),
        __LEAVEMEIN_DEFINE_SIGNAME(IO),
        __LEAVEMEIN_DEFINE_SIGNAME(IOT),
        __LEAVEMEIN_DEFINE_SIGNAME(KILL),
        __LEAVEMEIN_DEFINE_SIGNAME(PIPE),
        __LEAVEMEIN_DEFINE_SIGNAME(POLL),
        __LEAVEMEIN_DEFINE_SIGNAME(PROF),
        __LEAVEMEIN_DEFINE_SIGNAME(PWR),
        __LEAVEMEIN_DEFINE_SIGNAME(QUIT),
        __LEAVEMEIN_DEFINE_SIGNAME(SEGV),
        __LEAVEMEIN_DEFINE_SIGNAME(STKFLT),
        __LEAVEMEIN_DEFINE_SIGNAME(STOP),
        __LEAVEMEIN_DEFINE_SIGNAME(TSTP),
        __LEAVEMEIN_DEFINE_SIGNAME(SYS),
        __LEAVEMEIN_DEFINE_SIGNAME(TERM),
        __LEAVEMEIN_DEFINE_SIGNAME(TRAP),
        __LEAVEMEIN_DEFINE_SIGNAME(TTIN),
        __LEAVEMEIN_DEFINE_SIGNAME(TTOU),
        __LEAVEMEIN_DEFINE_SIGNAME(URG),
        __LEAVEMEIN_DEFINE_SIGNAME(USR1),
        __LEAVEMEIN_DEFINE_SIGNAME(USR2),
        __LEAVEMEIN_DEFINE_SIGNAME(VTALRM),
        __LEAVEMEIN_DEFINE_SIGNAME(XCPU),
        __LEAVEMEIN_DEFINE_SIGNAME(XFSZ),
        __LEAVEMEIN_DEFINE_SIGNAME(WINCH),
    };
    size_t i;

    for (i = 0; i < __LEAVEMEIN_ARRAY_SIZE(signames); i++) {
        if (signames[i].number == sig) {
            break;
        }
    }

    if (i == __LEAVEMEIN_ARRAY_SIZE(signames)) {
        __leavemein_printf("signal unknown (%d)", sig);
    } else {
        __leavemein_printf("signal SIG%s (%d)", signames[i].name, sig);
    }
}

static void __leavemein_print_status(__leavemein_test *test) {
    int status = test->sysdep.exit_status;

    if (test->sysdep.timedout) {
        __leavemein_printf("timed out after %g seconds: FAILURE",
            test->params->timeout);
    } else if (WIFEXITED(status)) {
        int exit_status;

        exit_status = WEXITSTATUS(status);

        if (exit_status == 0) {
            __leavemein_printf("exit code %d: SUCCESS", exit_status);
        } else {
            __leavemein_printf("exit code %d: FAILURE", exit_status);
        }
    } else if (WIFSIGNALED(status)) {
        int sig;
        const char *core_dumped;

        sig = WTERMSIG(status);
        __leavemein_print_signame(sig);
#ifdef WCOREDUMP
        core_dumped = WCOREDUMP(status) ? " (core dumped)" : "";
#else
        core_dumped = "";
#endif

        __leavemein_printf("%s: FAILURE", core_dumped);
    } else if (WIFSTOPPED(status)) {
        int sig;

        sig = WSTOPSIG(status);
        __leavemein_printf("stopped, ");
        __leavemein_print_signame(sig);
        __leavemein_printf(": FAILURE");
    } else if (WIFCONTINUED(status)) {
        __leavemein_printf("continued: %s\n", "FAILURE");
    } else {
        __leavemein_printf("unknown reason: %s", "FAILURE");
    }
}   
#endif /* _LEAVEIN_TEST_LINUX_H_ */
