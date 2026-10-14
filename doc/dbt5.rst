=====================================
Database Test 5 (DBT-5) Documentation
=====================================

.. contents:: Table of Contents

------------
Introduction
------------

This document provides instructions on how to set up and use the Open Source
Development Lab's Database Test 5 (DBT-5) kit.  This kit is an open source
fair-use implementation of the TPC Benchmark(TM) E (TPC-E) specification, which
is an on-line transaction processing benchmark.

An introduction and complete details on the TPC-E can be found at:
https://tpc.org/tpce/

------
Design
------

DBT-5 implements the workload with three binaries: **BrokerageHouse**,
**MarketExchange**, and **Driver**.

BrokerageHouse
==============

**BrokerageHouse** is responsible for executing all of the database
transactions.  It implements the **EGenTxnHarness** and **EGenDriverDM**.

MarketExchange
==============

**MarketExchange** emulates stock exchanges and sends messages to the
**BrokerageHouse** for initial Trade Request and Mark Feed transactions.  It
implements the **EGenDriverMEE**.

Driver
======

**Driver** emulates customers and drives the workload.  It implements
**EGenDriverCE**.

Event Driven Programs
=====================

Each of the three programs is also built a second time to use libev event loops
in place of threads.  The event driven programs accept the same options as the
threaded programs, plus the options that this section describes.  The workload
runner, **dbt5 run**, starts the threaded programs by default.  With
``--mode=event-driven``, it starts the event driven programs instead.  In that
mode, the runner passes its ``--driver-processes`` and
``--brokerage-processes`` options to the ``-F`` options of the driver and the
Brokerage House, and its ``--market-connections`` option to the ``-C`` option
of the Market Exchange.

The event driven driver, **DriverMain2**, forks one process for each
processor that it may run on, pins each process to its processor, and divides
the users evenly among the processes.  Each process emulates its users with
one event loop, and writes its own mix log.  The users start one at a time at
the interval that the ``-y`` option sets, as they do with threads, so the
ramp-up time does not change.  The parent process runs the Data-Maintenance
transaction once per minute.  The ``-F`` option sets the number of processes
per processor, 1 by default.

The event driven Brokerage House, **BrokerageHouseMain2**, forks worker
processes the same way.  Each worker process opens one database connection,
and executes the transactions requested on all of the driver connections
that it serves, one transaction at a time.  The parent process accepts each
driver connection and passes it to a worker process through a Unix domain
socket.  The ``-F`` option sets the number of worker processes per processor,
1 by default.  A request waits while its worker process executes a
transaction for another connection, so when there are more connections than
worker processes, more worker processes reduce response times.

The event driven Market Exchange, **MarketExchangeMain2**, runs in a single
process, because the Market Exchange Emulator keeps the state of every
pending trade.  It queues the Trade-Result and Market-Feed transactions that
the emulator generates, and sends each one over a connection to the
Brokerage House that is not waiting for a reply.  The ``-C`` option sets the
number of connections, by default the number of processors that the program
may run on.  The default matches the number of worker processes that the
event driven Brokerage House starts by default.  With a single connection,
these transactions run one at a time, and each one also waits while the
worker process that serves the connection executes other transactions.

Killing the parent process of the event driven driver or Brokerage House
kills its other processes too.  The kit stops a program by killing the process
whose process identifier the program writes to a file, so it stops these
programs the same way that it stops the threaded programs.

----------------
Installing DBT-5
----------------

The latest stable and development version of the kit can be found on GitHub:
https://github.com/osdldbt/dbt5

The TPC's TPC-E Tools cannot be redistributed with DBT-5 and must be downloaded
separately from:
https://www.tpc.org/tpc_documents_current_versions/current_specifications5.asp

Required Software
=================

In addition to the database management system software, the following is also
required:

* C++ compiler
* patch
* Database development libraries and header files
    * PostgreSQL https://www.postgresql.org
        * psql
* libev development library and header files
  https://software.schmorp.de/pkg/libev.html
* SQLite https://www.sqlite.org/index.html is required for processing test
  results

Optional software:

* **sar**, **pidstat** http://pagesperso-orange.fr/sebastien.godard/ (While the
  scripts assume this particular version of **sar** and **pidstat**, it is
  possible to run on non-Linux based operating systems with some modifications
  to the kit.)
* **jq** https://jqlang.github.io/jq/ and **toml-cli**
  https://github.com/gnprice/toml-cli are required to use TOML configuration
  files for executing tests.

.. include:: user-guide.rst

--------------------------------
Database Management System Notes
--------------------------------

.. include:: postgresql.rst

.. include:: testing.rst

.. include:: dev-guide.rst
