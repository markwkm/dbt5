Since this kit is derived from a TPC benchmark specification, TPC Policy on
fair-use must be observed:

https://www.tpc.org/information/about/documentation/TPC_Policies_v5.13.htm#_Toc124863463

TPC Source Code
===============

The TPC currently restricts redistribution of their code but it can be freely
downloaded from:

https://www.tpc.org/tpc_documents_current_versions/current_specifications5.asp

After downloading the TPC-E Tools v1.14.0, create the directory `egen` at the
top level directory of this kit such::

    mkdir egen
    cd egen
    unzip *-tpc-e-tool.zip

The TPC supplied source code needs to be patched to be used for this kit.
Apply the supplied patches to egen (in the suggested order)::

    patch -p1 < patches/tpc-e-tool-1.14.0-postgresql.diff

Patch descriptions:

* `tpc-e-tool-1.14.0-postgresql.diff` add a Makefile for building TPC-E Tools
  for PostgreSQL.

Database Specific Notes
=======================

Usage varies depending on the database management system used.  See addition
README-* for specific instructions.

Database Scale
==============

The minimum "valid" configuration is supposed to be 5000 customer with 300
initial trade days and a scale factor of 500.  An example to build this for
PostgreSQL would be:

	dbt5-pgsql-build-db -c 5000 -s 500 -w 300

Although not verified, through testing there appeared to be a significantly
higher number of invalid and warning results when using a database scaled
smaller than this..  While it is possible to use the programs provided and have
the kit build something to the small size of:

	dbt5-pgsql-build-db -c 1000 -s 500 -w 1

This may result is a high level of failed transactions.
