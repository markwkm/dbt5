/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * Support for the programs that use event loops instead of threads.
 */

#ifndef DBT5_UTIL_H
#define DBT5_UTIL_H

#include <sys/types.h>

#include <string>
#include <vector>
using namespace std;

// The processors this process may run on, in increasing order.
vector<int> availableProcessors();

// Fork a child process pinned to the processor cpu.  The child is killed when
// the parent exits.  Returns what fork() returns.
pid_t forkOnProcessor(int cpu);

// Describe the exception that the caller is handling, and delete it if it is
// an error thrown by pointer.  Call only from a catch handler.  The callbacks
// of the event loop, which is C, must catch every exception, and use this to
// log it.
string caughtExceptionText();

#endif // DBT5_UTIL_H
