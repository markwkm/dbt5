/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Class representing PostgreSQL database loader.
//

#ifndef PG_LOADER_H
#define PG_LOADER_H

namespace TPCE
{
	const int iDateTimeFmt = 11;
	const int iConnectStrLen = 256;
	const char delimiter = '|';

	//
	// PGSQLLoader class.
	//
	template <typename T> class CPGSQLLoader : public CBaseLoader<T>
	{
	protected:
		FILE *p;

		char m_szConnectStr[iConnectStrLen + 1];
		char m_szTable[iMaxPath + 1]; // name of the table being loaded

	public:
		typedef const T *PT; // pointer to the table row

		CPGSQLLoader(const char *szConnectStr, const char *szTable);
		virtual ~CPGSQLLoader(void);

		// resets to clean state; needed after FinishLoad to continue loading
		virtual void Init();

		virtual void Commit(); // commit rows sent so far
		virtual void FinishLoad(); // finish load
		void Connect(); // connect to PostgreSQL

		// disconnect - should not throw any exceptions (to put into the destructor)
		void Disconnect();

		virtual void WriteNextRecord(PT next_record) = 0; // pure virtual function
	};

	//
	// The constructor.
	//
	template <typename T>
	CPGSQLLoader<T>::CPGSQLLoader(const char *szConnectStr, const char *szTable)
	{
		// FIXME: This may truncate if the szConnectStr is actually close to
		// iConnectStrLen.
		snprintf(m_szConnectStr, iConnectStrLen, "psql %s", szConnectStr);

		strncpy(m_szTable, szTable, iMaxPath);
	}

	//
	// Destructor closes the connection.
	//
	template <typename T>
	CPGSQLLoader<T>::~CPGSQLLoader()
	{
		Disconnect();
	}

	//
	// Reset state e.g. close the connection, bind columns again, and reopen.
	// Needed after Commit() to continue loading.
	//
	template <typename T>
	void CPGSQLLoader<T>::Init()
	{
		Connect();
	}

	template <typename T>
	void CPGSQLLoader<T>::Connect()
	{
		// Open a pipe to psql.
		p = popen(m_szConnectStr, "w");
		if (p == NULL) {
			cout << "error using psql" << endl;
			exit(1);
		}
		// FIXME: Have blind faith that psql connected ok.
		while (fgetc(p) != EOF) ;

		// BEGIN the transaction now to avoid WAL activity.  Don't remember which
		// version of PostgreSQL takes advantage of this, one of the 8.x series.
		fprintf(p, "BEGIN;\n");
		while (fgetc(p) != EOF) ;

		fprintf(p, "COPY %s FROM STDIN DELIMITER '%c' NULL '';\n",
				m_szTable, delimiter);
		// FIXME: Have blind faith that COPY started correctly.
		while (fgetc(p) != EOF) ;
	}

	//
	// Commit sent rows. This needs to be called every so often to avoid row-level
	// lock accumulation.
	//
	template <typename T>
	void CPGSQLLoader<T>::Commit()
	{
		// With COPY, don't COMMIT until we're done.
	}

	//
	// Commit sent rows. This needs to be called after the last row has been sent
	// and before the object is destructed. Otherwise all rows will be discarded
	// since this is in a transaction.
	//
	template <typename T>
	void CPGSQLLoader<T>::FinishLoad()
	{
		// End of the COPY.
		fprintf(p, "\\.\n");
		// FIXME: Have blind faith that COPY was successful.
		while (fgetc(p) != EOF) ;

		// COMMIT the COPY.
		fprintf(p, "COMMIT;\n");
		// FIXME: Have blind faith that COMMIT was successful.
		while (fgetc(p) != EOF) ;
	}

	//
	// Disconnect from the server. Should not throw any exceptions.
	//
	template <typename T>
	void CPGSQLLoader<T>::Disconnect()
	{
		if (p != NULL) {
			pclose(p);
		}
	}

} // namespace TPCE

#endif // PG_LOADER_H
