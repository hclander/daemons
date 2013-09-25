// Estructuras y funciones particulares de acceso a base de datos
#ifndef MYDB_H_
#define MYDB_H_

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


int mydb_insert_transport_frame(DB_T *db, long ip, int port,long sn, void *buf, size_t len);
int mydb_insert_trashed_transport_frame(DB_T *db, long ip, int port,long sn, void *buf, size_t len);
int mydb_insert_gps_subframe(DB_T *db, int rx_id, int loc_id,  void *buf, size_t len);
RES_T * mydb_select_undecoded_transport_frames(DB_T *db);
int mydb_update_transport_frame_status(DB_T *db, int rx_id, int status);


#endif /* MYDB_H_ */
