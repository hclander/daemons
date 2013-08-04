#include <string.h>
#include <stdbool.h>
#include "db.h"


/*
 * Methods for DB_T
 */

DB_T *db_create(char *host, char *database, char *user, char *password) {
	DB_T *db = calloc(1,sizeof(DB_T));

	db->host = strdup(host);
	db->database = strdup(database);
	db->user = strdup(user);
	db->password = strdup(password);
	mysql_init(&db->mysql);

	return db;
}

int db_connect(DB_T *db) {

	if (!db_isConnected(db)) {

		db->con=mysql_real_connect(&db->mysql, db->host,db->user, db->password, db->database,0,NULL,0);
	}

	return db_isConnected(db);

}

RES_T *db_query(DB_T *db, char *sql) {

	RES_T *res;

	if (db_isConnected(db)) {
		if (!mysql_query(db,sql)) {

			res = res_create(db);

			return res;
		}
	}

	return NULL;
}

int db_isConnected(DB_T *db) {

	return (db->con)?true:false;
}

char *db_getError(DB_T *db) {

	if (db_isConnected(db))
		return mysql_error(db->con);
	else
		return mysql_errno(&db->mysql);
}


void db_disconnect(DB_T *db){
	if (db_isConnected(db)) {
		mysql_close(db->con);
		db->con = NULL;
	}
}


void db_destroy(DB_T *db) {

	if (db) {

		free(db->host);
		free(db->database);
		free(db->user);
		free(db->password);

		db_disconnect(db);

		free(db);
	}
}

/*
 * Methods for ROW
 */

ROW_T *row_create(RES_T *res) {
	ROW_T * row = calloc(1,sizeof(ROW_T));

	row->numFields = mysql_num_fields(res->res);

	row->fields = mysql_fetch_fields(res->res);

	row->row = NULL;

	return row;
}

void row_next(RES_T *res, ROW_T *row) {
	row->row = mysql_fetch_row(res->res);
}

int row_getNumFields(ROW_T *row) {
	if (!row_isClosed(row))
		return row->numFields;
	return -1;
}

 char *row_getFieldName(ROW_T *row,int i) {

	 if (!row_isClosed(row))
		 if (i>=0 && i< (row->numFields))
			 return row->fields[i].name;

	 return NULL;
 }

 int *row_getFieldIndex(ROW_T *row,char *fieldName) {

	 if (!row_isClosed(row)) {

		 for (int i=0; i<row->numFields; i++) {
			 if (!strcmp(row->fields[i].name,fieldName))
				 return i;
		 }
	 }

	 return -1;
 }

char *row_getFieldValue(ROW_T *row, int i) {
	if (!row_isClosed(row))
		if ((i>=0) && i<(row->numFields))
			return row->row[i];
	return NULL;
}

int row_isClosed(ROW_T *row) {
	return row->row?false:true;
}

void row_close(ROW_T *row) {
	if (!row_isClosed(row))
		row->row = NULL;
}

void row_destroy(ROW_T *row) {
	if (row) {
		row_close(row);
		free(row);
	}
}

/*
 * Methods for RES
 */

RES_T *res_create(DB_T *db) {

	RES_T *res;

	if (db_isConnected(db)) {
		res = calloc(1,sizeof(RES_T));

		res->res = mysql_use_result(db->con);
		res->rowCount =-1;

		res->currentRow = NULL;

		return res;
	}

	return NULL;
}

int res_isClosed(RES_T *res) {
	return res->res?false:true;
}

void res_close(RES_T *res){
	if (!res_isClosed(res)) {
		mysql_free_result(res->res);
		res->res = NULL;
		//res->currentRow
	}
}


int res_getRowCount(RES_T *res) {
	if (!res_isClosed(res)) {
		if (res->rowCount<0)
			res->rowCount = mysql_num_rows(res->res);

		return res->rowCount;
	}

     return -1;
}

ROW_T *res_next(RES_T *res) {

	if (!res_isClosed(res)) {

		if (!res->currentRow)
			res->currentRow = row_create(res);

		row_next(res,res->currentRow);

		return res->currentRow;
	}

	return NULL;
}


ROW_T *res_getCurrentRow(RES_T *res) {

	if (!res_isClosed(res))
		return res->currentRow;

	return NULL;
}

void res_destroy(RES_T *res) {
   if (res) {
	res_close(res);
	free(res);
   }
}

