#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <time.h>

#include "lib/log.h"
#include "mydb.h"
#include "frames.h"


int mydb_insert_transport_frame(DB_T *db, long ip, int port,long sn, void *buf, size_t len) {

	 char *tpl= " INSERT INTO rx_tbl ( ip, port, ns, len, data ) "
					 " VALUES ( %d, %d, %d, %d, '%s' ) ";


	if (db_isConnected(db)) {

		char chunk[2*len+1];

		mysql_real_escape_string(db->con,chunk,buf,len);

		char sql[3000];


		sprintf(sql,tpl,ip,port,sn,len,chunk);

		RES_T *res=db_query(db,sql);

		res_destroy(res);

		return db_getAffectedRows(db);

	}


	return 0;
}


int mydb_insert_trashed_transport_frame(DB_T *db, long ip, int port,long sn, void *buf, size_t len) {

	 char *tpl= " INSERT INTO trash_rx_tbl ( ip, port, ns, len, data ) "
					 " VALUES ( %d, %d, %d, %d, '%s' ) ";


	if (db_isConnected(db)) {

		char chunk[2*len+1];

		mysql_real_escape_string(db->con,chunk,buf,len);

		char sql[3000];


		sprintf(sql,tpl,ip,port,sn,len,chunk);

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


/*
 * Inserta subframes Old Gps de 13 bytes
 */

int mydb_insert_old_gps_subframe(DB_T *db, int rx_id, int loc_id, int cmd, int len, int seq_l, int seq_s, void *buf, size_t bufLen) {

	char *tpl = " INSERT INTO gps_tbl ( rx_id, localizable_id, cmd, len, seq_l, seq_s, lat, lon, bearing, speed, knots, fix, hdop,time) "
				          " VALUES (%d, %d, %d,%d,%d, %d, %f, %f, %d, %f, %d, %d, %d , %d)";

	struct tm tp;
	struct tm *currTime;
	time_t epoch = time(NULL);
	currTime = gmtime(&epoch);


	if (db_isConnected(db)) {

		frm_gps_old_p gps = (frm_cmd_gps_old_p) buf;

		char sql[1500];

		time_t time;

		memset(&tp,0,sizeof(tp));

		tp.tm_year = (currTime->tm_year & ~0x03) | gps->year;
		tp.tm_mon  = gps->month;
		tp.tm_mday = gps->day;
		tp.tm_hour = gps->hour;
		tp.tm_min  = gps->data.parts.mins;
		tp.tm_sec  = gps->data.parts.secs;
		time = timegm(&tp);

		// Debido a como esta codificado esto hay que darlo la vuelta antes de poder usarlo
	   	gps->data.asWord = ntohs(gps->data.asWord);

		sprintf(sql,tpl,
				rx_id,
				loc_id,
				cmd,
				len,
				seq_l,
				seq_s,
				GPS_DECODE_LOC_1M(gps->lat_sign,gps->lat_deg,ntohs(gps->lat_min)),
				GPS_DECODE_LOC_1M(gps->data.parts.lon_sign ,gps->lon_deg,ntohs(gps->lon_min)),
				GPS_DECODE_OLD_BEARING(gps->data.parts.bear2,gps->bear1,gps->bear0),
				GPS_DECODE_SPEED(gps->knots),
				gps->knots,
				gps->data.parts.fix,
				gps->hdop,
				time
			);

		RES_T *res=db_query(db,sql);

		res_destroy(res);

		return db_getAffectedRows(db);


	}

	return 0;
}


int mydb_insert_rally_old_gps_frame(DB_T *db, int rx_id, int loc_id,  void *buf, size_t bufLen) {
	int recCount = 0;


	if (bufLen > sizeof(frm_cmd_gps_old_t) ) {   //TODO Verificar los tamaños

			frm_cmd_rally_gps_old_p pCmd = (frm_cmd_rally_gps_old_p) buf;

			if( (pCmd->cmd == 0x15) && (ntohs(pCmd->len)>250)) {

				frm_gps_old_t cache;
				frm_gps_old_p pGps = &pCmd->info;
				bool init = false;
				bool extended = false;
				int offset = 0;

				while (pGps->hour<23) {

					extended = pGps -> ext;

					    /* "00" => 9 bytes
					       "01" => 13 bytes
					        "10" => 2 bytes y contacto=0
					        "11" => 2 bytes y contacto=1 y fix=0
					     */

					switch (pGps->size) {

					case 0: // 00

						memcpy(&cache,pGps,sizeof(cache)-4); // Todos los campos menos los 4 ultimos

						offset = sizeof(cache)-4;  // En teoria 9;

						break;

					case 1:  // 11
						init = true;

						cache = *pGps; // Copiamos todos los datos

						offset = sizeof(cache); // En teoria 13;

						break;

					case 2:

						cache.hour = pGps->hour;
						cache.data.parts.mins = pGps->data.parts.mins;
						cache.data.parts.ign = 0;

						offset = 2;
						break;

					case 3:
						cache.hour = pGps->hour;
						cache.data.parts.mins = pGps->data.parts.mins;
						cache.data.parts.ign = 1;
						cache.data.parts.fix = 0;

						offset = 2;

							break;

						}

						// Hacer algo con datos Gps

					if (!init) {
						//Sale en caso de que no haya venido una trama de 13 bytes primero
						LOG_F_E("Error (loc_id=%d,rx_id=%d): Formato de trama incorrecto",loc_id,rx_id);
						break;
					}

					//testPrintOldGpsInfo(&cache);

					if ( mydb_insert_old_gps_subframe(db, rx_id, loc_id,
							pCmd->cmd,  ntohs(pCmd->len), ntohs(pCmd->seq_l), ntohs(pCmd->seq_s),
							&cache, sizeof(cache))
							)
					  recCount++;


					if (extended) {
						// Hacer cositas con los datos extendidos

						// FIXME De momento no soportamos tramas extendidas.

						LOG_F_E("Error (loc_id=%d,rx_id=%d): Formato con trama extendida no soportado",loc_id,rx_id);
						break; //While
						//

					}

					// Se continua...
				   pGps =  ((char *) pGps) + offset;
					//((char *)pGps)+=offset;

				}


			}

		}

	return recCount;
}



int mydb_insert_gps_subframe(DB_T *db, int rx_id, int loc_id,  void *buf, size_t len) {

	 char *tpl = " INSERT INTO gps_tbl ( rx_id, localizable_id, cmd, len, seq_l, seq_s, lat, lon, bearing, speed, knots, fix, hdop,time) "
			          " VALUES (%d, %d, %d,%d,%d, %d, %f, %f, %d, %f, %d, %d, %d , %d)";

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
				gps->len,
				ntohs(gps->seq_l),
				ntohs(gps->seq_s),
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

int mydb_insert_sensor_subframe(DB_T *db, int rx_id, int loc_id, time_t time, void *buf, size_t len) {

	char *tpl = " INSERT INTO sensor_value_tbl(rx_id, loc_id, sensor_type, sensor_idx, sensor_id, sensor_value, time ) "
				" VALUES (%d, %d, %d, %d, (%s), %d, %d ) ";

	char *stpl0 = " SELECT sensor_id from sensor_localizable_tbl where localizable_id = %d and sensor_type = %d and sensor_idx= %d";


	if (db_isConnected(db)) {

		char sql[1500];
		char sub[500];
		RES_T *res;

		frm_cmd_sensor_p sen = (frm_cmd_sensor_p) buf;

		snprintf(sub,sizeof(sub),stpl0,loc_id,sen->sensor.type,sen->sensor.num);

		snprintf(sql,sizeof(sql),tpl,rx_id, loc_id, sen->sensor.type, sen->sensor.num, sub,sen->sensor.value, time);

		res = db_query(db, sql);
		res_destroy(res);

		return db_getAffectedRows(db);

	}


	return 0;
}


int mydb_insert_cnx_subframe(DB_T *db, int rx_id, int loc_id, time_t time, void *buf, size_t len) {


	char *tpl = " INSERT INTO cnx_tbl(rx_id, localizable_id, imei, mode, version, reason, time ) "
				" VALUES ( %d, %d, %s, %d,%d,%d, %d ) ";


	if (db_isConnected(db)) {

		char sql[1500];
		frm_cmd_cnx_p cnx = (frm_cmd_cnx_p) buf;
		RES_T *res;

		snprintf(sql,sizeof(sql),tpl,
			rx_id,
			loc_id,
			cnx->imei,
			cnx->om,
			cnx->sv,
			cnx->cr,
			time
		);

		res=db_query(db,sql);
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





