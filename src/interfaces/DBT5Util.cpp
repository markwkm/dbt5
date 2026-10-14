/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // sched_getaffinity() and sched_setaffinity()
#endif

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

#include "DBT5Util.h"
#include "error.h"

using namespace TPCE;

vector<int>
availableProcessors()
{
	vector<int> processors;
	cpu_set_t set;

	// Use the affinity mask, which taskset or a cgroup can restrict to fewer
	// processors than are online.
	CPU_ZERO(&set);
	if (sched_getaffinity(0, sizeof(set), &set) == 0) {
		for (int i = 0; i < CPU_SETSIZE; i++) {
			if (CPU_ISSET(i, &set)) {
				processors.push_back(i);
			}
		}
	}

	if (processors.empty()) {
		long online = sysconf(_SC_NPROCESSORS_ONLN);
		for (long i = 0; i < online; i++) {
			processors.push_back((int) i);
		}
	}
	if (processors.empty()) {
		processors.push_back(0);
	}

	return processors;
}

pid_t
forkOnProcessor(int cpu)
{
	pid_t parent = getpid();

	// The child gets a copy of any buffered output and would write it a
	// second time.
	cout.flush();
	cerr.flush();
	fflush(NULL);

	pid_t pid = fork();
	if (pid != 0) {
		return pid;
	}

	// The kit's scripts stop a program by killing the process recorded in
	// its pid file, which is the parent.  Request the signal before
	// checking the parent, so a parent that exits in between cannot leave
	// this process running.
	if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1 || getppid() != parent) {
		_exit(1);
	}

	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(cpu, &set);
	if (sched_setaffinity(0, sizeof(set), &set) == -1) {
		cerr << "[" << getpid() << "] cannot pin to processor " << cpu << ": "
			 << strerror(errno) << endl;
	} else {
		cout << "[" << getpid() << "] pinned to processor " << cpu << "."
			 << endl;
	}

	return 0;
}

bool
setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL);

	return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

string
caughtExceptionText()
{
	// Rethrow the exception so that the handlers below match its type.  The
	// kit throws EGen errors by pointer and by value, and database errors as
	// strings.
	try {
		throw;
	} catch (CBaseErr *pErr) {
		string text(pErr->ErrorText());
		delete pErr;
		return text;
	} catch (std::exception &e) {
		return e.what();
	} catch (std::string &s) {
		return s;
	} catch (...) {
		return "unknown exception";
	}
}
