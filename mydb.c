#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <time.h>

#include "mydb.h"
#include "frames.h"


int mydb_insert_transport_frame(DB_T *db, long ip, int port, void *buf, size_t len) {

	 char *tpl= " INSERT INTO rx_tbl ( ip, port, ns, len, data ) "
					 " VALUES ( %d, %d, %d, %d, '%s' ) ";


	if (db_isConnected(db)) {
		transport_buf_p trans = (transport_buf_p) buf;

		char chunk[2*len+1];

		mysql_real_escape_string(db->con,chunk,buf,len);

		char sql[3000];


		sprintf(sql,tpl,ip,port,ntohl(trans->header.sn),len,chunk);

		RES_T *res=db_query(db,sql);

		res_destroy(res);

		return db_getAffectedRows(db);

	}


	return 0;
}


int mydb_update_transport_frame_status(DB_T *db, int rx_id, int status) {
	char *tpl = " UPDATE rx_tbl SET status = %d WHERE id = %d ";

	if (db_isConnected(db)) {
		char sql[1500];

		sprintf(sql,tpl,status,rx_id);

		RES_T *res=db_query(db,sql);

		res_destroy(res);
		return db_getAffectedRows(db);

	}

	return 0;
}

int mydb_insert_gps_subframe(DB_T *db, int rx_id, int loc_id,  void *buf, size_t len) {

	 char *tpl = " INSERT INTO gps_tbl ( rx_id, localizable_id, cmd, sec, lat, lon, bearing, speed, knots, fix, hdop,time) "
			          " VALUES (%d, %d, %d,%d, %f, %f, %d, %f, %d, %d, %d , %d)";

	struct tm tp;


	if (db_isConnected(db)) {

		frm_cmd_gps_p gps = (frm_cmd_gps_p) buf;

		char sql[1500];

		time_t time;

		switch(gps->cmd) {

		case 0x10: // human;

				memset(&tp,0,sizeof(tp));

				tp.tm_year = gps->time.parts.year;
				tp.tm_mon  = gps->time.parts.month;
				tp.tm_mday = gps->time.parts.day;
				tp.tm_hour = gps->time.parts.hour;
				tp.tm_min  = gps->time.parts.mins;
				tp.tm_sec  = gps->time.parts.secs;

				time = timegm(&tp);
				//time = mktime(&tp);

			break;

		case 0x11:  //epoch;

				time = ntohl(gps->time.epoch);
			break;


		}

		sprintf(sql,tpl,
				rx_id,
				loc_id,
				gps->cmd,
				ntohl(gps->seq),
				GPS_DECODE_LOC_1M(gps->lat_sign,gps->lat_deg,ntohs(gps->lat_min)),
				GPS_DECODE_LOC_1M(gps->lon_sign,gps->lon_deg,ntohs(gps->lon_min)),
				GPS_DECODE_BEARING(gps->bearing),
				GPS_DECODE_SPEED(gps->knots),
				gps->knots,
				gps->fix,
				gps->hdop,
				time
			);

		RES_T *res=db_query(db,sql);

		res_destroy(res);

		return db_getAffectedRows(db);
	}

	return 0;
}

RES_T * mydb_select_undecoded_transport_frames(DB_T *db) {

	 char *sql="SELECT * FROM rx_tbl where status=0 order by ts";

	if (db_isConnected(db)) {
		RES_T *res = db_query(db,sql);

		return res;
	}

	return NULL;
}





