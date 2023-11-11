/*
 * Things needed to provode system-dependent leavemein header file
 * definitions.
 * 
 */

#ifndef __LEAVEMEIN_SYSDEP_H_
#define __LEAVEMEIN_SYSDEP_H_

#define __LEAVEMEIN_DEFAULT_TIMEOUT     3.0

/*
 * Parameters of the test run.
 * max_jobs - Maximum number of simultaneous threads. A zero value means
 *  there is no limit.
 * n_runlist - Number of items in runlist. If this is zero and runlist is NULL,
 * run all tests. If this is zero and runlist is not NULL, run nothing.
 * runlist - List of tests to run
 * timeout - Number of seconds to allow each test to run
 */
struct __leavemein_params {
    unsigned max_jobs;
    size_t n_runlist;
    char **runlist;
    float timeout;
};

/*
 * Per-test information. This part is common to all implementations
 *  name - Name of the test
 *  func - Function to execute the test
 *  next - Next item on the list of tests, or NULL at the end.
 *  sysdep - System-dependent information
 */
struct __leavemein_test {
    struct __leavemein_test     *next;
    const char *                name;
    void                        (*func)(void);
    struct __leavemein_sysdep   sysdep;
};
#endif /* __LEAVEMEIN_SYSDEP_H_ */
