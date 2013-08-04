#include <string.h>
#include <stdbool.h>

#include "mydb.h"
#include "frames.h"


int mydb_insert_transport_frame(DB_T *db, char *buf, size_t len) {

	return 0;
}

int mydb_insert_gps_subframe(DB_T *db, char *buf, size_t len) {

	return 0;
}

RES_T * mydb_select_undecoded_transport_frames(DB_T *) {

	// select * from rx_tbl where status = 0;

	return NULL;
}





