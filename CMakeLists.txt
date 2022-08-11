CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT(dbt5 CXX)

set (PROJECT_VERSION_MAJOR 0)
set (PROJECT_VERSION_MINOR 1)
set (PROJECT_VERSION_PATCH 0)

set (PROJECT_VERSION
     ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

#
# Defining binaries so to minimized typoes.
#

SET(DBT5_BROKERAGE_HOUSE BrokerageHouseMain)
SET(DBT5_DRIVER DriverMain)
SET(DBT5_MARKET_EXCHANGE MarketExchangeMain)
SET(DBT5_TEST_TXN TestTxn)

#
# Check for large file support by using 'getconf'.
#

FIND_PROGRAM(GETCONF getconf)
IF (GETCONF)
    SET(GETCONF_FOUND TRUE)
ENDIF (GETCONF)

IF (GETCONF_FOUND)
    EXECUTE_PROCESS(COMMAND ${GETCONF} LFS_CFLAGS COMMAND ${GETCONF}
            LFS64_CFLAGS OUTPUT_VARIABLE LFS_CFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF (GETCONF_FOUND)

link_directories("${CMAKE_SOURCE_DIR}/egen/lib")

#
# Defining commonly used paths.
#

SET(EGENINC "${CMAKE_HOME_DIRECTORY}/egen/inc")
SET(UTILITIESINC "${CMAKE_HOME_DIRECTORY}/egen/Utilities/inc")

SET(TXN src/transactions)
SET(INTF src/interfaces)

SET(BH src/BrokerageHouse)
SET(CE src/Driver)
SET(ME src/MarketExchange)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBPQ libpq)
if (LIBPQ_FOUND)
    set (DBCFLAGS "${DBCFLAGS} -DHAVE_LIBPQ ${LIBPQ_CFLAGS}")

    set (DB_SOURCE
         ${DB_SOURCE}
         ${TXN}/pgsql/DBConnection.cpp
    )

    set (DB_SCRIPTS
         ${DB_SCRIPTS}
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-start-db
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-create-indexes
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-create-tables
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-drop-tables
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-restore-db
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-load-stored-procs
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-backup-db
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-db-plans
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-drop-db
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-check-db
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-db-stat
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-stop-db
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-create-db
         ${CMAKE_SOURCE_DIR}/scripts/pgsql/dbt5-pgsql-build-db
    )
endif (LIBPQ_FOUND)

#
# Defining common groups of source files.
#

SET(TXN_SOURCE ${DB_SOURCE}
               ${TXN}/TxnBaseDB.cpp
               ${TXN}/MarketFeedDB.cpp
               ${TXN}/TradeCleanupDB.cpp
               ${TXN}/DataMaintenanceDB.cpp
               ${TXN}/MarketWatchDB.cpp
               ${TXN}/SecurityDetailDB.cpp
               ${TXN}/BrokerVolumeDB.cpp
               ${TXN}/CustomerPositionDB.cpp
               ${TXN}/TradeUpdateDB.cpp
               ${TXN}/TradeLookupDB.cpp
               ${TXN}/TradeResultDB.cpp
               ${TXN}/TradeOrderDB.cpp
               ${TXN}/TradeStatusDB.cpp)

#
# Files to build.
#

ADD_EXECUTABLE(${DBT5_BROKERAGE_HOUSE}
               ${INTF}/CSocket.cpp
               ${INTF}/TxnHarnessSendToMarket.cpp
               ${TXN_SOURCE}
               ${BH}/BrokerageHouseMain.cpp
               ${BH}/BrokerageHouse.cpp
)

ADD_EXECUTABLE(${DBT5_DRIVER}
               ${CE}/Driver.cpp
               ${CE}/DriverMain.cpp
               src/Customer/Customer.cpp
               ${INTF}/CESUT.cpp
               ${INTF}/CSocket.cpp
               ${INTF}/DMSUT.cpp
               ${INTF}/BaseInterface.cpp
)

ADD_EXECUTABLE(${DBT5_MARKET_EXCHANGE}
               ${INTF}/CSocket.cpp
               ${INTF}/MEESUT.cpp
               ${INTF}/BaseInterface.cpp
               ${ME}/MarketExchange.cpp
               ${ME}/MarketExchangeMain.cpp
)

ADD_EXECUTABLE(${DBT5_TEST_TXN}
               ${INTF}/MEESUTtest.cpp
               ${INTF}/DMSUTtest.cpp
               ${TXN_SOURCE}
               ${INTF}/BaseInterface.cpp
               ${INTF}/CESUT.cpp
               ${INTF}/CSocket.cpp
               ${INTF}/TxnHarnessSendToMarketTest.cpp
               ${INTF}/TxnHarnessSendToMarket.cpp
               ${BH}/BrokerageHouse.cpp
               src/TestTransactions/TestTxn.cpp
)

#
# Set linker flags.
#

target_link_libraries (${DBT5_BROKERAGE_HOUSE} -lpthread libegen.a)
target_link_libraries (${DBT5_DRIVER} -lpthread libegen.a)
target_link_libraries (${DBT5_MARKET_EXCHANGE} -lpthread libegen.a)
target_link_libraries (${DBT5_TEST_TXN} -lpthread libegen.a)

if (LIBPQ_FOUND)
    target_link_libraries (${DBT5_BROKERAGE_HOUSE} ${LIBPQ_LDFLAGS})
    target_link_libraries (${DBT5_TEST_TXN} ${LIBPQ_LDFLAGS})
endif (LIBPQ_FOUND)

#
# Set compiler flags.
#

set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LFS_CFLAGS} -std=c++98")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_HOME_DIRECTORY}/src/include")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_FORMAT_MACROS")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

set_source_files_properties(
    ${BH}/BrokerageHouse.cpp
    ${BH}/BrokerageHouseMain.cpp
    ${INTF}/TxnHarnessSendToMarket.cpp
    ${TXN}/pgsql/DBConnection.cpp
    src/TestTransactions/TestTxn.cpp
    src/transactions/BrokerVolumeDB.cpp
    src/transactions/CustomerPositionDB.cpp
    src/transactions/DataMaintenanceDB.cpp
    src/transactions/MarketFeedDB.cpp
    src/transactions/MarketWatchDB.cpp
    src/transactions/SecurityDetailDB.cpp
    src/transactions/TradeCleanupDB.cpp
    src/transactions/TradeLookupDB.cpp
    src/transactions/TradeOrderDB.cpp
    src/transactions/TradeResultDB.cpp
    src/transactions/TradeStatusDB.cpp
    src/transactions/TradeUpdateDB.cpp
    src/transactions/TxnBaseDB.cpp
    COMPILE_FLAGS "-I${EGENINC} -I${UTILITIESINC} ${DBCFLAGS}"
)

set_source_files_properties(
    ${CE}/Driver.cpp
    ${CE}/DriverMain.cpp
    src/Customer/Customer.cpp
    ${INTF}/BaseInterface.cpp
    ${INTF}/CESUT.cpp
    ${INTF}/DMSUT.cpp
    ${INTF}/DMSUTtest.cpp
    ${INTF}/MEESUT.cpp
    ${INTF}/MEESUTtest.cpp
    ${INTF}/TxnHarnessSendToMarket.cpp
    ${INTF}/TxnHarnessSendToMarketTest.cpp
    ${ME}/MarketExchange.cpp
    ${ME}/MarketExchangeMain.cpp
    COMPILE_FLAGS "-I${EGENINC} -I${UTILITIESINC}"
)

set_source_files_properties(
    ${INTF}/CSocket.cpp
    COMPILE_FLAGS "-I${UTILITIESINC}"
)

#
# Install rules.
#

install (PROGRAMS
         ${CMAKE_BINARY_DIR}/${DBT5_BROKERAGE_HOUSE}
         ${CMAKE_BINARY_DIR}/${DBT5_DRIVER}
         ${CMAKE_BINARY_DIR}/${DBT5_MARKET_EXCHANGE}
         ${CMAKE_BINARY_DIR}/${DBT5_TEST_TXN}
         ${CMAKE_SOURCE_DIR}/scripts/dbt5-post-process
         ${CMAKE_SOURCE_DIR}/scripts/dbt5-sysstats
         ${CMAKE_SOURCE_DIR}/scripts/dbt5-get-os-info
         ${CMAKE_SOURCE_DIR}/scripts/dbt5-run-workload
         ${CMAKE_SOURCE_DIR}/scripts/dbt5-plot-transaction-rate
         ${CMAKE_SOURCE_DIR}/scripts/dbt5-generate-report
         ${DB_SCRIPTS}
         DESTINATION "/bin")

SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET(CPACK_GENERATOR "TXZ;TZIP")

SET(CPACK_PACKAGE_NAME "dbt5")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Database Test 5")
SET(CPACK_PACKAGE_VENDOR "osdldbt")

SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

SET(CPACK_INSTALL_CMAKE_PROJECTS "")

SET(CPACK_PACKAGE_DESCRIPTION_FILE "README.rst")
SET(CPACK_RESOURCE_FILE_LICENSE "LICENSE")

set (CPACK_SOURCE_IGNORE_FILES
     "\\\\.bz2\\$;\\\\.gz\\$;\\\\.xz\\$;\\\\.Z\\$;\\\\.zip\\$"
)
set (CPACK_SOURCE_IGNORE_FILES
     "${CPACK_SOURCE_IGNORE_FILES};/CMakeCache\\\\.txt\\$"
)
set (CPACK_SOURCE_IGNORE_FILES
     "${CPACK_SOURCE_IGNORE_FILES};/CMakeFiles/"
)
set (CPACK_SOURCE_IGNORE_FILES
     "${CPACK_SOURCE_IGNORE_FILES};/cmake_install\\\\.cmake\\$"
)
set (CPACK_SOURCE_IGNORE_FILES
     "${CPACK_SOURCE_IGNORE_FILES};/CPackConfig\\\\.cmake\\$"
)
set (CPACK_SOURCE_IGNORE_FILES
     "${CPACK_SOURCE_IGNORE_FILES};/\\\\.gitignore\\$"
)
set (CPACK_SOURCE_IGNORE_FILES
     "${CPACK_SOURCE_IGNORE_FILES};/\\\\_CPack\\\\_Packages/"
)
set (CPACK_SOURCE_IGNORE_FILES
     "${CPACK_SOURCE_IGNORE_FILES};/CPackSourceConfig\\\\.cmake\\$"
)

include (CPack)
