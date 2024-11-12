# SQL_ATTR_ROW_ARRAY_SIZE and SQL_ATTR_PARAMSET_SIZE

The attribute SQL_ATTR_PARAMSET_SIZE in ODBC is crucial when performing batch operations (like bulk inserts) because it defines how many sets of parameters will be processed in a single execution of SQLExecute.

## What Is ```SQL_ATTR_PARAMSET_SIZE?```
Purpose: It specifies the number of parameter sets in your arrays that will be executed as a batch.
Effect: When you set this attribute, ODBC understands that the bound arrays contain multiple rows of data, and SQLExecute should process all these rows in one call.

## How It Works Together with SQL_ATTR_ROW_ARRAY_SIZE
```SQL_ATTR_ROW_ARRAY_SIZE```: Specifies the number of rows fetched in a single call to SQLFetch.
```SQL_ATTR_PARAMSET_SIZE```: Specifies the number of parameter sets (i.e., rows) to insert in a single call to SQLExecute.

## When performing bulk inserts:

Parameter Binding: 
You bind your data using arrays, where each element in the array corresponds to a value for a row.

Setting Attributes:
- ```SQL_ATTR_PARAMSET_SIZE``` tells the driver how many rows of data are in those arrays.
- ```SQL_ATTR_ROW_ARRAY_SIZE``` is used primarily for fetching data, not inserting.

##Example:
Understanding ```SQL_ATTR_PARAMSET_SIZE```
Let's say you have the following data:


| pnr_id  | principle | si_total  |
|----------|----------|----------|
|  13    | 123  | 1000  |
|  14    |  456   |2000  |
| 15      | 345   | 3000 |
You can insert these rows using batch execution by:

Binding arrays for each column.
Setting `SQL_ATTR_PARAMSET_SIZE` to 3 since you have three rows to insert.


## Benefits of Using SQL_ATTR_PARAMSET_SIZE
- Reduced Round-Trips: Instead of inserting each row one at a time (i.e., calling SQLExecute for each row), you can insert multiple rows in one call, reducing network latency.
- Improved Performance: Batch inserts are much faster, especially when dealing with large datasets.
- Efficient Resource Usage: Minimizes the overhead of repeated SQL parsing and execution.



```C++
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

    // Set the parameter set size
    SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)MAX_ROWS, 0);

    // Data to be inserted
    SQLINTEGER pnr_ids[MAX_ROWS] = {13, 14, 15, 16, 17};
    SQLINTEGER principles[MAX_ROWS] = {123, 456, 789, 1011, 1213};
    SQLINTEGER si_totals[MAX_ROWS] = {1000, 2000, 3000, 4000, 5000};
    
    // Indicators (not null)
    SQLLEN pnrIdInd[MAX_ROWS] = {0, 0, 0, 0, 0};
    SQLLEN principleInd[MAX_ROWS] = {0, 0, 0, 0, 0};
    SQLLEN siTotalInd[MAX_ROWS] = {0, 0, 0, 0, 0};

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

    // Check how many rows were inserted
    SQLLEN rowsProcessed;
    ret = SQLGetStmtAttr(hStmt, SQL_ATTR_ROW_COUNT, &rowsProcessed, 0, NULL);
    std::cout << "Rows inserted: " << rowsProcessed << std::endl;

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    return 0;
}
```
