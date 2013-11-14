#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <time.h>

#include "lib/log.h"
#include "frames.h"
#include "framedecoder_list.h"

framedecoder_list_t decodersList;


int frame_xor_checksum(unsigned char *buffer, size_t offset, size_t len) {
	int chkSum =0;

	for (int i=0; i<len; i++) {
		chkSum ^= buffer[offset+i];
	}

	return chkSum;
}

//Test the integrity of transport frame
int frame_test_transport(unsigned char *buffer, size_t len ) {

	int result = false;

	if (len > TRANS_OVERLOAD) { // checks minimun length

		transport_buf_p trans  = (transport_buf_p) buffer;
		int datalen = ntohs(trans->header.length);

		if (len >=( datalen + TRANS_HEADER_SIZE) ) {  // checks enough data

			//TODO: Verify checksum method. Now assuming XOR
			// if ( trans->header.start.flags.crc)

			//Trick: frame_xor_checksum returns 0 if checksum is ok
			if (!frame_xor_checksum(buffer,0,datalen+TRANS_HEADER_SIZE))   //
				result = true;
		}
	}

	return result;
}


int frame_test_gps_old(unsigned char *buffer, size_t *len) {
	int result = false;

	frm_cmd_gps_old_p src;
	int minLen = sizeof(frm_cmd_gps_old_t);

	if (*len>=minLen){
		src = (frm_cmd_gps_old_p) buffer;

		if (src->cmd ==FRAME_CMD_GPS_OLD) {
			if( src->len>=minLen) {
				result = true;
				if (src->len > minLen)
					minLen = src->len;
			}
		}
	}


	*len = minLen;

	return result;
}

int frame_test_gps(unsigned char *buffer, size_t *len) {
	int result = false;

	frm_cmd_gps_p src;
	int minLen = sizeof(frm_cmd_gps_t);

	if (*len>=minLen) {

		src = (frm_cmd_gps_p) buffer;


		if (src->cmd == FRAME_CMD_GPS_HUMAN
				|| src->cmd == FRAME_CMD_GPS_EPOCH ) {

			if ((src->len==0) || (src->len+GPS_PREAMBLE_SIZE>=minLen)) {

				result = true;

				if (src->len>minLen)
					minLen = src->len;
			}
		}

	}

	*len=minLen;

	return result;

}



int frame_test_gps_all(unsigned char *buffer, size_t *len) {

	return frame_test_gps(buffer,len) || frame_test_gps_old(buffer,len);
}

int frame_decode_gps_old(unsigned char *buffer, size_t len, void *dst, size_t *dstLen) {
	int result = false;

	int minLen=sizeof(frm_cmd_gps_old_t);

	if (len>=minLen) {
		if (frame_test_gps_old(buffer,dstLen)) {

			memcpy(dst,buffer,minLen);

			result = true;
		}
	}

	*dstLen = minLen;
	return result;
}


int frame_decode_rally_gps_old(unsigned char *buffer, size_t len, void *dst, size_t *dstLen) {
	int result = false;
	int minLen = GPS_RALLY_FRAME_MIN_SIZE;

	if (len>=minLen) {

		frm_cmd_rally_gps_old_p pCmd = (frm_cmd_rally_gps_old_p) buffer;

		if( (pCmd->cmd == 0x15) && (ntohs(pCmd->len)>250)) {

			result = true;
		}

	}

	return result;
}

int frame_decode_gps(unsigned char *buffer, size_t len, void *dst, size_t *dstLen){

	int result = false;

	frm_cmd_gps_p src,gps;

	int minLen=sizeof(frm_cmd_gps_t);

	if (len>= minLen) {
		if (frame_test_gps(buffer,dstLen)) {

			memcpy(dst,buffer,*dstLen);

			result = true;
			//FIXME Decode here fields here??
//			gps = (frm_cmd_gps_p) dst;
//			gps->

		}
	} else {

		*dstLen = minLen;
	}

	return result;
}

int frame_decode_gps_all(unsigned char *buffer, size_t len, void *dst, size_t *dstLen) {

	return frame_decode_gps(buffer,len,dst,dstLen)
			|| frame_decode_gps_old(buffer,len,dst,dstLen);
}





int frame_encode_transport(int ns, void *src, size_t srcLen, void *dst, size_t *dstLen ) {

	int result = false;
	int chkSum;
	//unsigned char * pchkSum;

	if ( *dstLen >= srcLen+TRANS_OVERLOAD) {

		transport_buf_p trans = (transport_buf_p) dst;

		if (src ==NULL ) {
			src = dst;
		}
		//memcpy(dst+TRANS_PREAMBLE_SIZE,src,srcLen);
		memmove(dst+TRANS_PREAMBLE_SIZE,src,srcLen);  // To supports overlaps

		trans->header.start.start = 0x00;

		trans->header.sn = htonl(ns);

		trans->header.length = htons(TRANS_OVERLOAD - TRANS_HEADER_SIZE + srcLen);


		chkSum=frame_xor_checksum(dst,0,TRANS_OVERLOAD + srcLen-1);

		//pchkSum = dst +
		((char *)dst)[srcLen+TRANS_OVERLOAD-TRANS_FOOTER_SIZE] =chkSum;

		result = true;

	}

	*dstLen=srcLen+TRANS_OVERLOAD;

   return result;
}


//int frame_envelop_transport(int ns, size_t srcLen, void *buf, size_t *bufLen ) {
//
//	int result = false;
//
//
//	return result;
//}


int frame_decode_transport(unsigned char *buffer, size_t len){

	return 0;
}

int frame_encode_ack(long serialNumber, int cmd, void *dst, size_t *len) {
	int result = false;

	if (*len>=sizeof(frm_cmd_ack_t)) {
		frm_cmd_ack_p ack = (frm_cmd_ack_p) dst;

		ack->cmd = FRAME_SVR_CMD_ACK;

		ack->len = 0;

		ack->sn  = htonl(serialNumber);

		result = true;

	}

	*len=sizeof(frm_cmd_ack_t);

	return result;
}

int frame_test_cnx(void *src, size_t *len) {
	int result = false;

	if (*len >= sizeof(frm_cmd_cnx_t)) {

		frm_cmd_cnx_p cnx = (frm_cmd_cnx_p) src;

		if (cnx -> cmd == FRAME_CMD_CONECTION) {

			if (!frame_xor_checksum(cnx->imei,0,sizeof(cnx->imei)+1))
				result = true;
		}
	}

	*len = sizeof(frm_cmd_cnx_t);

	return result;
}

int frame_decode_cnx(unsigned char *src, size_t srcLen, void *dst, size_t *dstLen) {
	int result = false;

	if (srcLen>=sizeof(frm_cmd_cnx_t)) {
		if (frame_test_cnx(src,dstLen)) {

			memcpy(dst,src,*dstLen);

			result = true;

		}
	}

	return result;
}

int frame_test_sensor(void *src, size_t *len) {
	int result = false;

	if (*len>=sizeof(frm_cmd_sensor_t)) {
		frm_cmd_sensor_p sen = (frm_cmd_sensor_p) src;


		switch(sen->sensor.type) {
			case SENSOR_TYPE_0:
			case SENSOR_TYPE_1:
			case SENSOR_TYPE_2:
				result = true;
		}

	}

	*len = sizeof(frm_cmd_sensor_t);

	return result;
}

int frame_decode_sensor(unsigned char *src, size_t srcLen, void *dst, size_t *dstLen) {
	int result = false;

	if (srcLen>=sizeof(frm_cmd_sensor_t)) {
		if (frame_test_sensor(src,dstLen)){
			memcpy(dst,src,*dstLen);
			result = true;
		}
	}

	return result;
}

int frame_encode_gps(int seq,int bearing,int knots, float lat, float lon, int fix, int hdop, time_t aTime, void *dst ,size_t *len) {
	int result = false;

	if (*len>=sizeof(frm_cmd_gps_t)) {
		frm_cmd_gps_p gps = (frm_cmd_gps_p) dst;

		gps->cmd = 0x11;
		gps->len =0;
		gps->seq_l= seq;
		gps->seq_s=0;
		gps->bearing = GPS_ENCODE_BEARING(bearing);
		gps->knots = knots;
		gps->lat_sign = lat<0?1:0;
		gps->lat_deg = gps->lat_sign?-lat:lat;
		gps->lat_min =htons(GPS_ENCODE_LOCMIN(lat));
		gps->lon_sign = lon<0?1:0;
		gps->lon_deg = gps->lon_sign?-lon:lon;
		gps->lon_min = htons(GPS_ENCODE_LOCMIN(lon));
		gps->fix = fix;
		gps->hdop = hdop;
		gps->time.epoch = aTime?htonl(aTime):htonl(time(NULL));

	}

	*len=sizeof(frm_cmd_gps_t);

	return result;
}

int frame_encode_cnx(char *imei,int om, int sv, int cr, void *dst,size_t *len) {
	int result = false;

	if (*len>=sizeof(frm_cmd_cnx_t)) {
		frm_cmd_cnx_p cnx = (frm_cmd_cnx_p) dst;
		cnx->cmd = FRAME_CMD_CONECTION;
		cnx->len = 0;
		strncpy(cnx->imei,imei,sizeof(cnx->imei));
		cnx->ixor = frame_xor_checksum(cnx->imei,0,sizeof(cnx->imei));

		cnx->om = om;

		cnx->sv = htons(sv);
		cnx->cr = cr;

		result = true;

	}

	*len = sizeof(frm_cmd_cnx_t);

	return result;
}

// TODO ACK Varios EF XX XX 00 CS  para 0x11

// TODO Ack nserie E9 00 00 XX XX CS  para 0x06

// En ambos casos XX XX  = numero serie

int frame_decodermanager_init() {

	decodersList = fdl_create();
	frame_decodermanager_registerAll();

	return 0;
}

void frame_decodermanager_finish() {
	fdl_destroy(&decodersList);
}

int frame_decodermanager_registerAll() {

	fdl_register_func(decodersList,FRAME_CMD_GPS_EPOCH,frame_decode_gps);
	fdl_register_func(decodersList,FRAME_CMD_GPS_HUMAN,frame_decode_gps);
	fdl_register_func(decodersList,FRAME_CMD_GPS_OLD,frame_decode_gps_old);
	fdl_register_func(decodersList,FRAME_CMD_CONECTION,frame_decode_cnx);
	fdl_register_func(decodersList,FRAME_CMD_GPS_RALLY_OLD,frame_decode_rally_gps_old);

	//TODO Simplificar el registro de comandos con valores en el propio comando

	for (int i=0;i<0x10;i++) {
		fdl_register_func(decodersList,FRAME_CMD_SENSOR0 | i,frame_decode_sensor);
	}

	for (int i=0;i<0x10;i++) {
			fdl_register_func(decodersList,FRAME_CMD_SENSOR1 | i,frame_decode_sensor);
	}

	return fdl_get_count(decodersList);
}

int frame_decodermanager_decode(void *buf, size_t size, int *count) {
	int result = false;
	unsigned char *src;
	int cmd;
	size_t len;
	size_t dstLen;
	char dst[512];

	//FIXME: Esto todavía no hace nada ... de hecho sólo mueve datos de un sitio a otro no graba a base de datos
	//       hay que hacer varios cambios y crear la funcionalidad de gestor de decodificacion (decodemanager)

	if (buf) {
		src = buf;
		*count=0;
		len = dstLen = 0;
		framedecoder_func_t decode;

		LOG_D("Begin decoding buffer.");

		while (true) {

			if (len>=size) {
				result = true;
				break;
			}

			cmd = *src;

			LOG_F_D("Found CMD: 0x%02X",cmd);

			decode=fdl_get_func(decodersList,cmd);

			if(!decode) {
				LOG_F_W("Decode function not found to cmd=0x%02X",cmd);
				break;
			}

			dstLen=sizeof(dst);
			if (!decode(src,size-len,dst,&dstLen)) {
				LOG_F_W("Decode function fails processing cmd=0x%02X",cmd);
				break;
			}

			len+=dstLen;
			src += dstLen;
			(*count)++;
		}

		LOG_D("End decoding buffer.");

	}
	return result;
}

