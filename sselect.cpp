#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

#define ARRAY_SIZE 10  // Number of rows to fetch at once

void checkError(SQLRETURN ret, SQLHANDLE handle, SQLSMALLINT type, const char* msg) {
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlState[6], errorMsg[256];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        SQLGetDiagRec(type, handle, 1, sqlState, &nativeError, errorMsg, sizeof(errorMsg), &textLength);
        printf("Error: %s\nSQL State: %s\nMessage: %s\n", msg, sqlState, errorMsg);
    }
}

int main() {
    SQLHENV hEnv = NULL;
    SQLHDBC hDbc = NULL;
    SQLHSTMT hStmt = NULL;
    SQLRETURN ret;

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    checkError(ret, hEnv, SQL_HANDLE_ENV, "Allocating Environment Handle");
    
    // Set the ODBC version
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    checkError(ret, hEnv, SQL_HANDLE_ENV, "Setting ODBC Version");

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Allocating Connection Handle");

    // Connect to the database (Adjust DSN, UserID, Password as needed)
    ret = SQLConnect(hDbc, (SQLCHAR*)"YourDSN", SQL_NTS, (SQLCHAR*)"UserID", SQL_NTS, (SQLCHAR*)"Password", SQL_NTS);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Connecting to Database");

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    checkError(ret, hStmt, SQL_HANDLE_STMT, "Allocating Statement Handle");

    // Set row array size for bulk fetch
    SQLSetStmtAttr(hStmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)ARRAY_SIZE, 0);

    // Define arrays to hold data
    SQLLEN pnrIds[ARRAY_SIZE], principles[ARRAY_SIZE], siTotals[ARRAY_SIZE];
    SQLLEN indPnrId[ARRAY_SIZE], indPrinciple[ARRAY_SIZE], indSiTotal[ARRAY_SIZE];

    // Bind columns to arrays
    SQLBindCol(hStmt, 1, SQL_C_SLONG, pnrIds, sizeof(SQLINTEGER), indPnrId);
    SQLBindCol(hStmt, 2, SQL_C_SLONG, principles, sizeof(SQLINTEGER), indPrinciple);
    SQLBindCol(hStmt, 3, SQL_C_SLONG, siTotals, sizeof(SQLINTEGER), indSiTotal);

    // Prepare the query
    const char* query = "SELECT pnr_id, priciple, si_total FROM pnr_info WHERE pnr_id > 1 AND pnr_id < 50";
    ret = SQLExecDirect(hStmt, (SQLCHAR*)query, SQL_NTS);
    checkError(ret, hStmt, SQL_HANDLE_STMT, "Executing Query");

    // Fetch data in bulk
    SQLULEN rowsFetched;
    SQLSetStmtAttr(hStmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);

    while ((ret = SQLFetch(hStmt)) != SQL_NO_DATA) {
        for (SQLULEN i = 0; i < rowsFetched; i++) {
            printf("Row %lu: PNR_ID = %ld, Principle = %ld, SI_Total = %ld\n",
                   i + 1, pnrIds[i], principles[i], siTotals[i]);
        }
    }

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}
