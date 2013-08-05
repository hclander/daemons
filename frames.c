#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "frames.h"


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
			if (!frame_xor_checksum(buffer,TRANS_HEADER_SIZE,datalen))   //
				result = true;
		}
	}

	return result;
}



int frame_test_gps(unsigned char *buffer, size_t len) {
	int result = false;

	frm_cmd_gps_p src;

	if (len>=sizeof(frm_cmd_gps_t)) {

		src = (frm_cmd_gps_p) buffer;

		if (src->cmd == FRAME_CMD_GPS_HUMAN
				|| src->cmd == FRAME_CMD_GPS_EPOCH) {

			result = true;
		}

	}

	return result;
}


int frame_decode_gps(unsigned char *buffer, size_t len, void *dst, size_t *gpsLen){

	int result = false;

	frm_cmd_gps_p src,gps;

	if (len>= sizeof(frm_cmd_gps_t))
		if (frame_test_gps(buffer,*gpsLen)) {

			memcpy(dst,buffer,sizeof(frm_cmd_gps_t));

			result = true;
			//FIXME Decode here fields here??
//			gps = (frm_cmd_gps_p) dst;
//			gps->

		}

	*gpsLen = sizeof(frm_cmd_gps_t);

	return result;
}



int frame_encode_transport(int ns, void *src, size_t srcLen, void *dst, size_t *dstLen ) {

	int result = false;
	int chkSum;
	//unsigned char * pchkSum;

	if ( *dstLen >= srcLen+TRANS_OVERLOAD) {

		transport_buf_p trans = (transport_buf_p) dst;

		trans->header.start.start = 0x00;

		trans->header.sn = htonl(ns);

		trans->header.length = htons(TRANS_OVERLOAD - TRANS_HEADER_SIZE + srcLen);

		memcpy(dst+TRANS_PREAMBLE_SIZE,src,srcLen);

		chkSum=frame_xor_checksum(dst,TRANS_HEADER_SIZE,TRANS_OVERLOAD - TRANS_HEADER_SIZE + srcLen-1);

		//pchkSum = dst +
		((char *)dst)[srcLen+TRANS_OVERLOAD-TRANS_FOOTER_SIZE] =chkSum;


		result = true;

	}

	*dstLen=srcLen+TRANS_OVERLOAD;

   return result;;
}

int frame_decode_transport(unsigned char *buffer, size_t len){

	return 0;
}




