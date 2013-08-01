#include <mysql/mysql.h>
#include <postgresql/libpq-fe.h>
// includes for unixODBC
//#include <sql.h>
//#include <sqlext.h>



extern MYSQL *my_mysql_connect(const char *server, const char *user, const char *password, const char *database);
extern MYSQL_RES *my_mysql_query(MYSQL *conn, const char *query);

extern PGconn *pgsql_connect(const char *server, const char *user, const char *password, const char *database);
extern PGresult *pgsql_query(PGconn *conn, const char *query);





