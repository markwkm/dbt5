/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 */

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdexcept>

#include "BrokerageHouse2.h"
#include "BrokerageHouseWorker.h"
#include "DBT5Util.h"

// A driver connection, in a worker process
struct CBrokerageHouse2::TConnection
{
	ev_io io; // becomes ready when the driver sends a request
	CSocket socket;
	CBrokerageHouse2 *pBrokerageHouse;
};

// Pass a connection to the worker processes.  The descriptor is sent as
// ancillary data with one byte of data.
static bool
passConnection(int pass, int fd)
{
	char data = 0;
	struct iovec iov;
	struct msghdr msg;

	union
	{
		struct cmsghdr header;
		char buf[CMSG_SPACE(sizeof(int))];
	} control;
	struct cmsghdr *cmsg;
	ssize_t rc;

	iov.iov_base = &data;
	iov.iov_len = sizeof(data);
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control.buf;
	msg.msg_controllen = sizeof(control.buf);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));

	do {
		// MSG_NOSIGNAL: report a socket pair that no worker process holds
		// as EPIPE instead of delivering SIGPIPE
		rc = sendmsg(pass, &msg, MSG_NOSIGNAL);
	} while (rc == -1 && errno == EINTR);

	return rc != -1;
}

// Receive a connection that the listener passed.  Returns what recvmsg()
// returns, and the descriptor in fd, or -1 if the message carried none.
static ssize_t
receiveConnection(int pass, int *fd)
{
	char data;
	struct iovec iov;
	struct msghdr msg;

	union
	{
		struct cmsghdr header;
		char buf[CMSG_SPACE(sizeof(int))];
	} control;
	struct cmsghdr *cmsg;
	ssize_t rc;

	iov.iov_base = &data;
	iov.iov_len = sizeof(data);
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control.buf;
	msg.msg_controllen = sizeof(control.buf);

	do {
		rc = recvmsg(pass, &msg, 0);
	} while (rc == -1 && errno == EINTR);

	*fd = -1;
	if (rc > 0) {
		cmsg = CMSG_FIRSTHDR(&msg);
		if (cmsg != NULL && cmsg->cmsg_level == SOL_SOCKET
				&& cmsg->cmsg_type == SCM_RIGHTS
				&& cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
			memcpy(fd, CMSG_DATA(cmsg), sizeof(int));
		}
	}
	return rc;
}

// Constructor
CBrokerageHouse2::CBrokerageHouse2(const char *szHost, const char *szDBName,
		const char *szDBPort, const char *szMEEHost, const char *szMEEPort,
		const int iListenPort, char *outputDirectory, int iClientSide,
		bool verbose, int iProcessesPerProcessor)
: CBrokerageHouse(szHost, szDBName, szDBPort, szMEEHost, szMEEPort,
		  iListenPort, outputDirectory, iClientSide, verbose),
  m_iProcessesPerProcessor(iProcessesPerProcessor), m_pLoop(NULL),
  m_pWorker(NULL)
{
	m_Pass[0] = -1;
	m_Pass[1] = -1;
}

// Destructor
CBrokerageHouse2::~CBrokerageHouse2()
{
	delete m_pWorker;

	for (int i = 0; i < 2; i++) {
		if (m_Pass[i] != -1) {
			close(m_Pass[i]);
		}
	}
}

// Listener
void
CBrokerageHouse2::startListener(void)
{
	m_Socket.dbt5Listen(m_iListenPort);

	// A connection reset before it is accepted leaves nothing to accept,
	// which must not block the listener.
	if (!setNonBlocking(m_Socket.getListenFd())) {
		throw std::runtime_error(string("cannot make listener non-blocking: ")
								 + strerror(errno));
	}

	// A sequenced packet socket keeps each passed connection in a message
	// of its own.
	if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, m_Pass) == -1) {
		throw std::runtime_error(
				string("cannot create socket pair: ") + strerror(errno));
	}
	// Every worker process wakes up for each passed connection, and the
	// ones that do not receive it must not block.
	if (!setNonBlocking(m_Pass[1])) {
		throw std::runtime_error(
				string("cannot make socket pair non-blocking: ")
				+ strerror(errno));
	}

	vector<int> processors = availableProcessors();
	int iProcesses = (int) processors.size() * m_iProcessesPerProcessor;

	cout << processors.size() << " processor(s) available" << endl
		 << "starting " << iProcesses
		 << " worker process(es), 1 database connection per process" << endl;

	for (int i = 0; i < iProcesses; i++) {
		pid_t pid = forkOnProcessor(processors[i % processors.size()]);
		if (pid == 0) {
			runWorker();
			exit(0);
		} else if (pid == -1) {
			// Exiting kills the worker processes forked so far.
			throw std::runtime_error(
					string("cannot fork worker process: ") + strerror(errno));
		}
	}

	// Only the worker processes receive passed connections.
	close(m_Pass[1]);
	m_Pass[1] = -1;

	m_pLoop = EV_DEFAULT;
	ev_io_init(&m_Accept, acceptCallback, m_Socket.getListenFd(), EV_READ);
	m_Accept.data = this;
	ev_io_start(m_pLoop, &m_Accept);

	ev_run(m_pLoop, 0);
}

void
CBrokerageHouse2::acceptCallback(struct ev_loop *loop, ev_io *w, int revents)
{
	CBrokerageHouse2 *pBrokerageHouse
			= reinterpret_cast<CBrokerageHouse2 *>(w->data);
	int fd;

	do {
		fd = accept(w->fd, NULL, NULL);
	} while (fd == -1 && errno == EINTR);
	if (fd == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK && errno != ECONNABORTED) {
			ostringstream osErr;
			osErr << "Problem accepting socket connection" << endl
				  << "Error: " << strerror(errno) << " at "
				  << "BrokerageHouse2::acceptCallback" << endl;
			pBrokerageHouse->logErrorMessage(osErr.str());
		}
		return;
	}

	if (!passConnection(pBrokerageHouse->m_Pass[0], fd)) {
		int err = errno;
		ostringstream osErr;
		osErr << "Error: cannot pass connection to worker processes: "
			  << strerror(err) << " at BrokerageHouse2::acceptCallback"
			  << endl;
		pBrokerageHouse->logErrorMessage(osErr.str());
		if (err == EPIPE) {
			// Every worker process exited, so no connection can be served.
			pBrokerageHouse->logErrorMessage(
					"no worker processes left, exiting\n");
			exit(1);
		}
	}

	// The worker process received its own copy of the descriptor.
	close(fd);
}

// The loop of a worker process: it receives the connections that the
// listener passes, and executes the transactions requested on them.
void
CBrokerageHouse2::runWorker()
{
	m_Socket.closeListenerSocket();
	close(m_Pass[0]);
	m_Pass[0] = -1;

	m_pLoop = EV_DEFAULT;
	ev_io_init(&m_Receive, receiveCallback, m_Pass[1], EV_READ);
	m_Receive.data = this;
	ev_io_start(m_pLoop, &m_Receive);

	// Returns once the listener exited and every connection closed.
	ev_run(m_pLoop, 0);

	delete m_pWorker;
	m_pWorker = NULL;
}

void
CBrokerageHouse2::receiveCallback(struct ev_loop *loop, ev_io *w, int revents)
{
	CBrokerageHouse2 *pBrokerageHouse
			= reinterpret_cast<CBrokerageHouse2 *>(w->data);
	int fd;
	ssize_t rc = receiveConnection(w->fd, &fd);

	if (rc == 0) {
		// The listener closed the socket pair.  Keep serving the
		// connections already received.
		ev_io_stop(loop, w);
		return;
	} else if (rc == -1) {
		// EAGAIN means another worker process received the connection
		// first.
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			ostringstream osErr;
			osErr << "Error: cannot receive connection from listener: "
				  << strerror(errno) << " at BrokerageHouse2::receiveCallback"
				  << endl;
			pBrokerageHouse->logErrorMessage(osErr.str());
		}
		return;
	} else if (fd == -1) {
		pBrokerageHouse->logErrorMessage(
				"Error: listener passed no connection at "
				"BrokerageHouse2::receiveCallback\n");
		return;
	}

	// No exception may propagate into the event loop, which is C.
	try {
		pBrokerageHouse->addConnection(fd);
	} catch (...) {
		close(fd);
		ostringstream osErr;
		osErr << "Error: " << caughtExceptionText()
			  << " at BrokerageHouse2::receiveCallback" << endl
			  << "accepted socket connection closed" << endl;
		pBrokerageHouse->logErrorMessage(osErr.str());
	}
}

// Start serving a connection that the listener passed
void
CBrokerageHouse2::addConnection(int fd)
{
	// Create the database connection and transactions with the first
	// connection, as the thread based Brokerage House creates them when it
	// accepts a connection.
	if (m_pWorker == NULL) {
		try {
			m_pWorker = new CBrokerageHouseWorker(
					this, m_szMEEHost, m_szMEEPort);
		} catch (CSocketErr *pErr) {
			// Most likely the market exchange could not be reached.  Close
			// the connection to release the driver, and try again with the
			// next one.
			close(fd);

			ostringstream osErr;
			osErr << "Error: " << pErr->ErrorText() << " at "
				  << "BrokerageHouse2::addConnection" << endl
				  << "accepted socket connection closed" << endl;
			logErrorMessage(osErr.str());
			delete pErr;
			return;
		}
	}

	TConnection *pConnection = new TConnection;
	pConnection->socket.setSocketFd(fd);
	pConnection->pBrokerageHouse = this;
	ev_io_init(&pConnection->io, requestCallback, fd, EV_READ);
	pConnection->io.data = pConnection;
	ev_io_start(m_pLoop, &pConnection->io);
}

// Stop serving a connection and close it
void
CBrokerageHouse2::closeConnection(TConnection *pConnection)
{
	ev_io_stop(m_pLoop, &pConnection->io);
	delete pConnection;
}

void
CBrokerageHouse2::requestCallback(struct ev_loop *loop, ev_io *w, int revents)
{
	TConnection *pConnection = reinterpret_cast<TConnection *>(w->data);
	CBrokerageHouse2 *pBrokerageHouse = pConnection->pBrokerageHouse;

	// No exception may propagate into the event loop, which is C.
	try {
		pBrokerageHouse->requestReady(pConnection);
	} catch (...) {
		ostringstream osErr;
		osErr << "Error: " << caughtExceptionText()
			  << " at BrokerageHouse2::requestCallback" << endl;
		pBrokerageHouse->logErrorMessage(osErr.str());
		pBrokerageHouse->closeConnection(pConnection);
	}
}

// Execute the transaction that a driver requested and send its status
void
CBrokerageHouse2::requestReady(TConnection *pConnection)
{
	TMsgDriverBrokerage request;
	TMsgBrokerageDriver Reply; // return message

	try {
		pConnection->socket.dbt5Receive(
				reinterpret_cast<void *>(&request), sizeof(request));
	} catch (CSocketErr *pErr) {
		if (pErr->getAction() != CSocketErr::ERR_SOCKET_CLOSED) {
			ostringstream osErr;
			osErr << "Error on Receive: " << pErr->ErrorText()
				  << " at BrokerageHouse2::requestReady" << endl;
			logErrorMessage(osErr.str());
		}
		delete pErr;

		// The socket has been closed, stop serving it.
		closeConnection(pConnection);
		return;
	}

	Reply.iStatus = m_pWorker->runTransaction(&request);
	try {
		pConnection->socket.dbt5Send(
				reinterpret_cast<void *>(&Reply), sizeof(Reply));
	} catch (CSocketErr *pErr) {
		ostringstream osErr;
		osErr << "Error on Send: " << pErr->ErrorText()
			  << " at BrokerageHouse2::requestReady" << endl;
		logErrorMessage(osErr.str());
		delete pErr;

		// The socket has been closed, stop serving it.
		closeConnection(pConnection);
	}
}
