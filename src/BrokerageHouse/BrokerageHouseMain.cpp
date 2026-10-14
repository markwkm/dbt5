/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * executable that opens the Brokerage House to business
 * 25 July 2006
 */

#include <unistd.h>

#ifdef LIBEV
#include "BrokerageHouse2.h"
#else
#include "BrokerageHouse.h"
#endif
#include "DBT5Consts.h"

// Establish defaults for command line option
int iClientSide = 0;
int iListenPort = iBrokerageHousePort;
bool verbose = false;
#ifdef LIBEV
int iProcessesPerProcessor = 1;
#endif

char szHost[iMaxHostname + 1] = "";
char szDBName[iMaxDBName + 1] = "";
char szDBPort[iMaxPort + 1] = "";
char szMEEHost[iMaxHostname + 1] = "localhost";
char szMEEPort[iMaxPort + 1] = "";
char outputDirectory[iMaxPath + 1] = ".";

// shows program usage
void
usage()
{
#ifdef LIBEV
	cout << "Usage: BrokerageHouseMain2 [options]" << endl << endl;
#else
	cout << "Usage: BrokerageHouseMain [options]" << endl << endl;
#endif
	cout << "   Option      Default    Description" << endl;
	cout << "   =========   =========  ===============" << endl;
	cout << "   -1                     Use client-side app logic" << endl;
	cout << "   -d string              Database name" << endl;
#ifdef LIBEV
	printf("   -F integer  %-9d  Processes per processor\n",
			iProcessesPerProcessor);
#endif
	cout << "   -h string   localhost  Database server" << endl;
	printf("   -l integer  %-9d  Socket listen port\n", iListenPort);
	printf("   -m string   %9s  Market Exchange Emulator hostname\n",
			szMEEHost);
	printf("   -M integer  %9s  Market Exchange Emulator port\n", szMEEPort);
	cout << "   -o string   .          Output directory" << endl;
	cout << "   -p integer             Database port" << endl;
	cout << "   -v                     Verbose output" << endl;
	cout << endl;
}

// Parse command line
void
parse_command_line(int argc, char *argv[])
{
	int ch;

	// getopt reports missing option arguments and unknown options,
	// unlike the old hand rolled parser, which silently accepted them.
#ifdef LIBEV
	const char *optstring = "1d:F:h:l:m:M:o:p:v";
#else
	const char *optstring = "1d:h:l:m:M:o:p:v";
#endif
	while ((ch = getopt(argc, argv, optstring)) != -1) {
		switch (ch) {
		case '1':
			iClientSide = 1;
			break;
		case 'd': // Database name.
			strncpy(szDBName, optarg, iMaxDBName);
			szDBName[iMaxDBName] = '\0';
			break;
#ifdef LIBEV
		case 'F':
			iProcessesPerProcessor = atoi(optarg);
			if (iProcessesPerProcessor < 1) {
				cerr << "Error: invalid number of processes for -F: " << optarg
					 << endl;
				exit(1);
			}
			break;
#endif
		case 'h': // Database host name.
			strncpy(szHost, optarg, iMaxHostname);
			szHost[iMaxHostname] = '\0';
			break;
		case 'l':
			iListenPort = atoi(optarg);
			if (iListenPort < 1 || iListenPort > 65535) {
				cerr << "Error: invalid port for -l: " << optarg << endl;
				exit(1);
			}
			break;
		case 'm':
			strncpy(szMEEHost, optarg, iMaxHostname);
			szMEEHost[iMaxHostname] = '\0';
			break;
		case 'M': // Market Exchange port
			strncpy(szMEEPort, optarg, iMaxPort);
			szMEEPort[iMaxPort] = '\0';
			break;
		case 'o': // output directory
			strncpy(outputDirectory, optarg, iMaxPath);
			outputDirectory[iMaxPath] = '\0';
			break;
		case 'p': // Postmaster port
			strncpy(szDBPort, optarg, iMaxPort);
			szDBPort[iMaxPort] = '\0';
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
	snprintf(szMEEPort, iMaxPort, "%d", iMarketExchangePort);

	cout << "dbt5 - Brokerage House" << endl;

	// Parse command line
	parse_command_line(argc, argv);

	cout << "Listening on port: " << iListenPort << endl << endl;

	char *pidFilename = new char[1024];
	snprintf(pidFilename, 1023, "%s/bh.pid", outputDirectory);
	FILE *fpid = fopen(pidFilename, "w");
	if (fpid == NULL) {
		cerr << "ERROR: can't create pid file: " << pidFilename << endl;
		return 1;
	}
	fprintf(fpid, "%d", getpid());
	fclose(fpid);
	delete[] pidFilename;

	// Let the user know what settings will be used.
	cout << "Using the following database settings:" << endl
		 << "  Database hostname: " << szHost << endl
		 << "  Database port: " << szDBPort << endl
		 << "  Database name: " << szDBName << endl;

	cout << "Using the following Market Exchange Emulator settings:" << endl
		 << "  Hostname: " << szMEEHost << endl
		 << "  Port: " << szMEEPort << endl;

#ifdef LIBEV
	CBrokerageHouse2 BrokerageHouse(szHost, szDBName, szDBPort, szMEEHost,
			szMEEPort, iListenPort, outputDirectory, iClientSide, verbose,
			iProcessesPerProcessor);
#else
	CBrokerageHouse BrokerageHouse(szHost, szDBName, szDBPort, szMEEHost,
			szMEEPort, iListenPort, outputDirectory, iClientSide, verbose);
#endif
	cout << "Brokerage House opened for business, waiting for traders..."
		 << endl;
	try {
		BrokerageHouse.startListener();
	} catch (CBaseErr *pErr) {
		cout << "Error " << pErr->ErrorNum() << ": " << pErr->ErrorText();
		if (pErr->ErrorLoc()) {
			cout << " at " << pErr->ErrorLoc();
		}
		cout << endl;
		return 1;
	} catch (std::bad_alloc const &) {
		// operator new will throw std::bad_alloc exception if there is not
		// sufficient memory for the request.
		cout << "*** Out of memory ***" << endl;
		return 2;
	} catch (const exception &e) {
		cout << e.what() << endl;
		return 3;
	}

#ifndef LIBEV
	pthread_exit(NULL);
#endif

	cout << "Brokerage House closed for business" << endl;
	return (0);
}
