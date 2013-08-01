// Estructuras y funciones particulares de acceso a base de datos

#include "lib/db.h"

// Estructuras para meter y sacar datdos de la BD

typedef struct tblRx {
	long id;
	long ts;
	long ip;
	int port;
	long ns;
	int status;
	int len;
	char *data;
}TBL_RX;

typedef struct tblGps {

	long id;
	long rx_id;
	int cmd;
	long lat;
	long lon;
	int bearing;
	int speed;
	int fix;
	int hdop;
	long time;

}TBL_GPS;


