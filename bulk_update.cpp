#include <iostream>
#include <sql.h>
#include <sqlext.h>

#define MAX_ROWS 5 // Number of rows to update in a single batch

void checkSQLReturnCode(SQLRETURN ret, const char* message) {
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error: " << message << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main() {
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLHSTMT hStmt;
    SQLRETURN ret;

    // Allocate environment handle
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    // Allocate connection handle and connect to the database
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    ret = SQLConnect(hDbc, (SQLCHAR*)"your_database", SQL_NTS,
                     (SQLCHAR*)"your_username", SQL_NTS,
                     (SQLCHAR*)"your_password", SQL_NTS);
    checkSQLReturnCode(ret, "Connection Failed!");

    // Allocate statement handle
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // Set the parameter set size for batch update
    SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)MAX_ROWS, 0);

    // Data arrays to be updated
    SQLINTEGER pnr_ids[MAX_ROWS] = {101, 102, 103, 104, 105};
    SQLINTEGER new_principles[MAX_ROWS] = {1500, 2500, 3500, 4500, 5500};
    SQLINTEGER new_si_totals[MAX_ROWS] = {15, 25, 35, 45, 55};
    SQLLEN indicators[MAX_ROWS] = {0, 0, 0, 0, 0};

    // Bind the parameters using arrays
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, new_principles, 0, indicators);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, new_si_totals, 0, indicators);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, pnr_ids, 0, indicators);

    // Prepare the update statement
    const char* updateQuery = "UPDATE pnr_info SET principle = ?, si_total = ? WHERE pnr_id = ?";
    ret = SQLPrepare(hStmt, (SQLCHAR*)updateQuery, SQL_NTS);
    checkSQLReturnCode(ret, "Prepare Failed!");

    // Execute the batch update
    ret = SQLExecute(hStmt);
    checkSQLReturnCode(ret, "Batch Update Failed!");

    // Check how many rows were updated
    SQLLEN rowsUpdated;
    ret = SQLGetStmtAttr(hStmt, SQL_ATTR_ROW_COUNT, &rowsUpdated, 0, NULL);
    std::cout << "Rows updated in a single batch: " << rowsUpdated << std::endl;

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}
