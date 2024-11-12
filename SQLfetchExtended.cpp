#include <stdio.h>
#include <sql.h>
#include <sqlext.h>
#include <stdlib.h>

#define MAX_ROWS 5  // Number of rows to fetch in each batch

void check_error(SQLRETURN retcode, SQLHANDLE handle, SQLSMALLINT type, const char *message) {
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlState[6], errorMsg[256];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        SQLGetDiagRec(type, handle, 1, sqlState, &nativeError, errorMsg, sizeof(errorMsg), &textLength);
        fprintf(stderr, "Error %s: %s\n", message, errorMsg);
        exit(-1);
    }
}

int main() {
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLHSTMT hStmt;
    SQLRETURN retcode;

    // Step 1: Allocate environment handle
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    check_error(retcode, hEnv, SQL_HANDLE_ENV, "Allocating Environment");

    // Step 2: Set the ODBC version to 3.x
    retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
    check_error(retcode, hEnv, SQL_HANDLE_ENV, "Setting ODBC Version");

    // Step 3: Allocate connection handle
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    check_error(retcode, hDbc, SQL_HANDLE_DBC, "Allocating Connection");

    // Step 4: Connect to the database
    retcode = SQLConnect(hDbc, (SQLCHAR *)"YourDataSource", SQL_NTS,
                         (SQLCHAR *)"YourUsername", SQL_NTS, (SQLCHAR *)"YourPassword", SQL_NTS);
    check_error(retcode, hDbc, SQL_HANDLE_DBC, "Connecting to Database");

    // Step 5: Allocate statement handle
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    check_error(retcode, hStmt, SQL_HANDLE_STMT, "Allocating Statement");

    // Step 6: Execute the query
    const char *query = "SELECT pnr_id, principle, si_total FROM pnr_info WHERE pnr_id > 100 AND pnr_id < 1600";
    retcode = SQLExecDirect(hStmt, (SQLCHAR *)query, SQL_NTS);
    check_error(retcode, hStmt, SQL_HANDLE_STMT, "Executing Query");

    // Step 7: Bind columns for fetching data
    SQLINTEGER pnr_id[MAX_ROWS];
    SQLINTEGER principle[MAX_ROWS];
    SQLINTEGER si_total[MAX_ROWS];
    SQLLEN pnr_idInd[MAX_ROWS], principleInd[MAX_ROWS], si_totalInd[MAX_ROWS];

    retcode = SQLBindCol(hStmt, 1, SQL_C_SLONG, pnr_id, sizeof(SQLINTEGER), pnr_idInd);
    check_error(retcode, hStmt, SQL_HANDLE_STMT, "Binding Column 1");
    retcode = SQLBindCol(hStmt, 2, SQL_C_SLONG, principle, sizeof(SQLINTEGER), principleInd);
    check_error(retcode, hStmt, SQL_HANDLE_STMT, "Binding Column 2");
    retcode = SQLBindCol(hStmt, 3, SQL_C_SLONG, si_total, sizeof(SQLINTEGER), si_totalInd);
    check_error(retcode, hStmt, SQL_HANDLE_STMT, "Binding Column 3");

    // Step 8: Fetch rows using SQLExtendedFetch
    UWORD rowStatus[MAX_ROWS];
    SDWORD fetchedRows;

    // Fetch rows in batches of MAX_ROWS
    retcode = SQLExtendedFetch(hStmt, SQL_FETCH_NEXT, 0, &fetchedRows, rowStatus);
    while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
        printf("Fetched %d rows:\n", fetchedRows);
        for (SDWORD i = 0; i < fetchedRows; i++) {
            if (rowStatus[i] == SQL_ROW_SUCCESS || rowStatus[i] == SQL_ROW_SUCCESS_WITH_INFO) {
                printf("PNR_ID: %d, Principle: %d, SI_Total: %d\n", pnr_id[i], principle[i], si_total[i]);
            }
        }
        // Fetch the next batch
        retcode = SQLExtendedFetch(hStmt, SQL_FETCH_NEXT, 0, &fetchedRows, rowStatus);
    }

    // Step 9: Clean up and disconnect
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}
