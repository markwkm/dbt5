/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * executable that opens the Market Exchange to business
 * 30 July 2006
 */

#include <unistd.h>

#ifdef LIBEV
#include "DBT5Util.h"
#include "MarketExchange2.h"
#else
#include "MarketExchange.h"
#endif
#include "DBT5Consts.h"

// Establish defaults for command line options
char szBHaddr[iMaxHostname + 1] = "localhost"; // Brokerage House address
int iListenPort = iMarketExchangePort; // socket port to listen
int iBHlistenPort = iBrokerageHousePort;
// # of customers for this instance
TIdent iConfiguredCustomerCount = iDefaultCustomerCount;
// total number of customers in the database
TIdent iActiveCustomerCount = iDefaultCustomerCount;

bool verbose = false;
#ifdef LIBEV
// Connections to the Brokerage House, set to the number of processors
// available by default.  A Brokerage House with worker processes starts that
// many by default, and a Trade-Result or Market-Feed transaction waits while
// the worker process that serves its connection executes other transactions.
int iBHConnections;
#endif

// EGen flat_ing directory location
char szFileLoc[iMaxPath + 1];
// path to output files
char outputDirectory[iMaxPath + 1] = ".";

// shows program usage
void
usage()
{
#ifdef LIBEV
	cout << "Usage: MarketExchangeMain2 [options]" << endl << endl;
#else
	cout << "Usage: MarketExchangeMain [options]" << endl << endl;
#endif
	cout << "   Option      Default     Description" << endl;
	cout << "   ==========  ==========  =============================" << endl;
	printf("   -c integer  %-10ld  Active customer count\n",
			iActiveCustomerCount);
#ifdef LIBEV
	printf("   -C integer  %-10d  Brokerage House connections\n",
			iBHConnections);
#endif
	cout << "   -i string               Location of EGen flat_in directory"
		 << endl;
	printf("   -l integer  %-10d  Socket listen port\n", iListenPort);
	printf("   -h string   %-10s  Brokerage House address\n", szBHaddr);
	printf("   -o string   %-10s  directory for output files\n",
			outputDirectory);
	printf("   -t integer  %-10ld  Configured customer count\n",
			iConfiguredCustomerCount);
	printf("   -p integer  %-10d  Brokerage House listen port\n",
			iBHlistenPort);
	cout << "   -v                      Verbose output" << endl;
}

// Parse command line
void
parse_command_line(int argc, char *argv[])
{
	int ch;

	// getopt reports missing option arguments and unknown options,
	// unlike the old hand rolled parser, which silently accepted them.
#ifdef LIBEV
	const char *optstring = "c:C:h:i:l:o:p:t:v";
#else
	const char *optstring = "c:h:i:l:o:p:t:v";
#endif
	while ((ch = getopt(argc, argv, optstring)) != -1) {
		switch (ch) {
		case 'c':
			iActiveCustomerCount = atol(optarg);
			break;
#ifdef LIBEV
		case 'C':
			iBHConnections = atoi(optarg);
			if (iBHConnections < 1) {
				cerr << "Error: invalid number of connections for -C: "
					 << optarg << endl;
				exit(1);
			}
			break;
#endif
		case 'h':
			strncpy(szBHaddr, optarg, iMaxHostname);
			szBHaddr[iMaxHostname] = '\0';
			break;
		case 'i':
			strncpy(szFileLoc, optarg, iMaxPath);
			szFileLoc[iMaxPath] = '\0';
			break;
		case 'l':
			iListenPort = atoi(optarg);
			if (iListenPort < 1 || iListenPort > 65535) {
				cerr << "Error: invalid port for -l: " << optarg << endl;
				exit(1);
			}
			break;
		case 'o':
			strncpy(outputDirectory, optarg, iMaxPath);
			outputDirectory[iMaxPath] = '\0';
			break;
		case 'p':
			iBHlistenPort = atoi(optarg);
			if (iBHlistenPort < 1 || iBHlistenPort > 65535) {
				cerr << "Error: invalid port for -p: " << optarg << endl;
				exit(1);
			}
			break;
		case 't':
			iConfiguredCustomerCount = atol(optarg);
			break;
		case 'v':
			verbose = true;
			break;
		default:
			usage();
			exit(1);
		}
	}

	if (optind < argc) {
		usage();
		cout << endl << "Error: unexpected argument: " << argv[optind]
			 << endl;
		exit(1);
	}
}

int
main(int argc, char *argv[])
{
	// Establish defaults for command line options
	strncpy(szFileLoc, "flat_in", iMaxPath);
#ifdef LIBEV
	iBHConnections = (int) availableProcessors().size();
#endif

	cout << "dbt5 - Market Exchange Main" << endl;

	// Parse command line
	parse_command_line(argc, argv);

	cout << "Listener port: " << iListenPort << endl << endl;

	char *pidFilename = new char[1024];
	snprintf(pidFilename, 1023, "%s/mee.pid", outputDirectory);
	FILE *fpid = fopen(pidFilename, "w");
	if (fpid == NULL) {
		cerr << "ERROR: can't create pid file: " << pidFilename << endl;
		return 1;
	}
	fprintf(fpid, "%d", getpid());
	fclose(fpid);
	delete[] pidFilename;

	// Let the user know what settings will be used.
	cout << "Using the following settings:" << endl << endl;
	cout << "EGen flat_in directory location: " << szFileLoc << endl;
	cout << "Configured customer count: " << iConfiguredCustomerCount << endl;
	cout << "Active customer count: " << iActiveCustomerCount << endl;
	cout << "Brokerage House address: " << szBHaddr << endl;
	cout << "Brokerage House port: " << iBHlistenPort << endl;
#ifdef LIBEV
	cout << "Brokerage House connections: " << iBHConnections << endl;
#endif

	try {
		const DataFileManager inputFiles(szFileLoc, iConfiguredCustomerCount,
				iActiveCustomerCount, TPCE::DataFileManager::IMMEDIATE_LOAD);
#ifdef LIBEV
		CMarketExchange2 MarketExchange(inputFiles, szFileLoc, 1,
				iConfiguredCustomerCount, iActiveCustomerCount, iListenPort,
				szBHaddr, iBHlistenPort, outputDirectory, verbose,
				iBHConnections);
#else
		CMarketExchange MarketExchange(inputFiles, szFileLoc, 1,
				iConfiguredCustomerCount, iActiveCustomerCount, iListenPort,
				szBHaddr, iBHlistenPort, outputDirectory, verbose);
#endif
		cout << "Market Exchange started, waiting for trade requests..."
			 << endl;

		MarketExchange.startListener();
	} catch (CBaseErr *pErr) {
		cout << "Error " << pErr->ErrorNum() << ": " << pErr->ErrorText();
		if (pErr->ErrorLoc()) {
			cout << " at " << pErr->ErrorLoc();
		}
		cout << endl;
		return 1;
	} catch (CBaseErr &err) {
		cout << "Error " << err.ErrorNum() << ": " << err.ErrorText();
		if (err.ErrorLoc()) {
			cout << " at " << err.ErrorLoc();
		}
		cout << endl;
		return 1;
	} catch (std::bad_alloc const &) {
		// operator new will throw std::bad_alloc exception if there is no
		// sufficient memory for the request.
		cout << "*** Out of memory ***" << endl;
		return 2;
	} catch (std::exception &e) {
		cout << "Error: " << e.what() << endl;
		return 1;
	}

#ifndef LIBEV
	pthread_exit(NULL);
#endif

	cout << "Market Exchange closed for business." << endl;
	return (0);
}
