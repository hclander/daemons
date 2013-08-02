#include <string.h>
#include <stdbool.h>
#include "mydb.h"


typedef struct {

	char *host;
	char *database;
	char *user;
	char *password;

	MYSQL *con;



} DB_T;

typedef DB_T *DB_P;


DB_T *db_create(char *host, char *database, char *user, char *password) {
	DB_T *db = calloc(1,sizeof(DB_T));

	db->host = strdup(host);
	db->database = strdup(database);
	db->user = strdup(user);
	db->password = strdup(password);

	return db;
}

int db_connect(DB_T *db) {

	db->con=mysql_init(NULL);

}

int db_isConnected(DB_T *db) {

	if (db->con)
		return true;

	return false;
}

void db_destroy(DB_T *db) {

	if (db) {

		free(db->host);
		free(db->database);
		free(db->user);
		free(db->password);

		if (db_isConnected(db)) {
			mysql_close(db->con);
		}

	}
}
