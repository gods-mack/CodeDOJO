#include <iostream>
#include <sql.h>
#include <sqlext.h>

#define MAX_ROWS 5 // Number of rows to insert in a single batch

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

    // Set the row array size for batch insertion
    SQLSetStmtAttr(hStmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)MAX_ROWS, 0);

    // Data to be inserted
    SQLINTEGER pnr_ids[MAX_ROWS] = {13, 14, 15, 16, 17};
    SQLINTEGER principles[MAX_ROWS] = {123, 456, 789, 1011, 1213};
    SQLINTEGER si_totals[MAX_ROWS] = {1000, 2000, 3000, 4000, 5000};
    
    // Indicators (not null)
    SQLLEN pnrIdInd[MAX_ROWS] = {0};
    SQLLEN principleInd[MAX_ROWS] = {0};
    SQLLEN siTotalInd[MAX_ROWS] = {0};

    // Bind the parameter arrays
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, pnr_ids, 0, pnrIdInd);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, principles, 0, principleInd);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, si_totals, 0, siTotalInd);

    // Prepare the SQL insert statement
    const char* insertQuery = "INSERT INTO pnr_info (pnr_id, principle, si_total) VALUES (?, ?, ?)";
    ret = SQLPrepare(hStmt, (SQLCHAR*)insertQuery, SQL_NTS);
    checkSQLReturnCode(ret, "SQL Prepare Failed!");

    // Execute the batch insert
    ret = SQLExecute(hStmt);
    checkSQLReturnCode(ret, "Batch Insert Failed!");

    std::cout << "Batch insertion successful!" << std::endl;

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}
