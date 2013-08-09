#ifndef DB_H_
#define DB_H_

#include <mysql/mysql.h>

typedef struct {

	MYSQL mysql;
	MYSQL *con ;

	char *host;
	char *database ;
	char *user;
	char *password;

	int affectedRows;

	//

} DB_T;

typedef DB_T *DB_P;

typedef struct {
	MYSQL_ROW row;

	MYSQL_FIELD *fields;
	int numFields;

} ROW_T;

typedef ROW_T * ROW_P;

typedef struct {
	MYSQL_RES *res;

	ROW_T *currentRow;

	long rowCount;
	//

} RES_T;

typedef RES_T *RES_P;


/*
 * Methods for DB_T
 */

DB_T *db_create(char *host, char *database, char *user, char *password);
int db_connect(DB_T *db);
RES_T *db_query(DB_T *db, char *sql) ;
int db_isConnected(DB_T *db);

char *db_getError(DB_T *db);

void db_disconnect(DB_T *db);


void db_destroy(DB_T *db) ;

/*
 * Methods for ROW
 */

ROW_T *row_create(RES_T *res) ;

void row_next(RES_T *res, ROW_T *row);

int row_getNumFields(ROW_T *row) ;

char *row_getFieldName(ROW_T *row,int i) ;
int row_getFieldIndex(ROW_T *row,char *fieldName) ;
char *row_getFieldValue(ROW_T *row, int i) ;
int db_getAffectedRows(DB_T *db);

int row_isClosed(ROW_T *row);

void row_close(ROW_T *row) ;

void row_destroy(ROW_T *row) ;

/*
 * Methods for RES
 */

RES_T *res_create(DB_T *db) ;

int res_isClosed(RES_T *res);

void res_close(RES_T *res);

int res_getRowCount(RES_T *res);

ROW_T *res_next(RES_T *res) ;

ROW_T *res_getCurrentRow(RES_T *res) ;

void res_destroy(RES_T *res);


#endif /* DB_H_ */
