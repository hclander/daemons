#include "db.h"
#include "log.h"

// Ejemplo de conexion con mysql

MYSQL *my_mysql_connect(const char *server, const char *user, const char *password, const char *database) {

	MYSQL *conn = mysql_init(NULL);

	if (!mysql_real_connect(conn,server,user,password,database,0, NULL,0)) {

		LOG_E(mysql_error(conn));
		return NULL;

	}

	return conn;
}

MYSQL_RES *my_mysql_query(MYSQL *conn, const char *query) {
	MYSQL_RES *res = NULL;

	if (conn!=NULL) {

		if (mysql_query(conn, query)) {
			LOG_E(mysql_error(conn));
			return NULL;
		}

		res = mysql_use_result(conn);

	}

	return res;
}


// Funciones de conexion Postgres
PGconn *pgsql_connect(const char *server, const char *user, const char *password, const char *database) {
	char strConn [255];
	PGconn *conn = NULL;

	snprintf(strConn, sizeof(strConn), "dbname=%s host=%s user=%s password=%s", database, server, user, password);

	conn = PQconnectdb(strConn);

	if (PQstatus(conn) == CONNECTION_BAD) {

		LOG_F_E("Error al conectar a la base de datos: %s\n",database);
		PQfinish(conn);
		return NULL;
	}

	return conn;
}


PGresult *pgsql_query(PGconn *conn, const char *query) {
	PGresult *res = NULL;

    if (conn!=NULL) {

    	res = PQexec(conn,query);
    	int status = PQresultStatus(res);

    	if ((PQresultStatus(res) != PGRES_COMMAND_OK)
    			&& (PQresultStatus(res)!=PGRES_TUPLES_OK)) {
    		LOG_E(PQerrorMessage(conn));
    		PQclear(res);
    		return  NULL;
    	}
    }

	return res;
}
