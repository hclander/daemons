// Estructuras para meter y sacar datos de las tramas

#include <sys/types.h>

#define PACKED __attribute__((__packed__))

#define MAX_TRANS_BUFF_SIZE 1500   //TODO: Revisar esto

#define TRANS_HEADER_START_SIZE 1
#define TRANS_HEADER_LENGTH_SIZE 2
#define TRANS_HEADER_SIZE (TRANS_HEADER_START_SIZE + TRANS_HEADER_LENGTH_SIZE)

#define TRANS_HEADER_SERIAL_SIZE 4
#define TRANS_PREAMBLE_SIZE   (TRANS_HEADER_SIZE + TRANS_HEADER_SERIAL_SIZE)

#define TRANS_DATA_OFFSET  TRANS_PREAMBLE_SIZE

#define TRANS_FOOTER_XOR_SIZE 1
#define TRANS_FOOTER_CRC_SIZE 2

#define TRANS_FOOTER_SIZE TRANS_FOOTER_XOR_SIZE

#define TRANS_OVERLOAD TRANS_PREAMBLE_SIZE + TRANS_FOOTER_SIZE

typedef union {

	struct  {
		u_int8_t version:4;
		u_int8_t undefined:2;
		u_int8_t crc:1;
		u_int8_t lendian:1;
	} flags ;

	u_int8_t start;

} start_flags_t;

typedef struct {
	start_flags_t start;
	u_int16_t length;
	u_int32_t sn;
} PACKED start_header_t ;


typedef start_header_t *start_header_p;

typedef struct {
	start_header_t header;
	unsigned char data[MAX_TRANS_BUFF_SIZE];
} PACKED transport_buf_t;

typedef transport_buf_t *transport_buf_p;


#define START_LITTLE_ENDIAN 0x80  // (1<<7)
#define START_CRC			0x40  //(1<<6)
#define START_UNDEFINED     0x30
#define START_VERSION		0x0F  //



// FRAME COMMANDS

#define FRAME_CMD_COMPATIBLE 0x04
#define FRAME_CMD_CONECTION 0x06
#define FRAME_CMD_MSG_TXT   0x0A
#define FRAME_CMD_MSG_BIN   0x0B
#define FRAME_CMD_GPS_HUMAN 0x10
#define FRAME_CMD_GPS_EPOCH 0x11
#define FRAME_CMD_RTC_TIME  0x12
#define FRAME_CMD_PROBE		0xA0
#define FRAME_CMD_SENSOR0	0xD0
#define FRAME_CMD_SENSOR1   0xE0
#define FRAME_CMD_PROBE_SENSOR_MASK 0xF0



// GPS COMMAND

/*
0x10 = Gps con hora humana: SEC(4) signo_lat+latitud(3) longitud(3) signo_longitud+rumbo(1) velocidad(1) fix+hdop(1) hora_humana(4)
            - SEC es el número de secuencia del registro de datos del localizador
            - signo_latitud(1 bit): Sur=1
            - latitud: 7 bits de grado + 2 bytes de minutos multiplicados por 1000
            - longitud: 8 bits de grado + 2 bytes de minutos multiplicados por 1000
            - signo_longitud(1 bit): Oeste=1
            - rumbo(7 bits) = (rumbo_real/4 => de 0º a 90º)

            - velocidad en nudos
            - fix: 1 bit
            - hdop: 7 bits
            - hora_humana: año (6 bits) + mes (4 bits) + día (5 bits) + hora (5 bits) + min (6 bits) + seg (6 bits) ,, puede escogerse una ordenación diferente que sea mejor para el microcontrolador

        - 0x11 = Gps con hora epoch unix: SEC(4) signo_lat+latitud(3) longitud(3) signo_longitud+rumbo(1) velocidad(1) fix+hdop(1) hora_epoch(4)
            - es idéntica a 0x10 pero la fecha y hora son los segundos transcurridos desde 1/1/1970 0h UTC.
*/

typedef struct {

	u_int8_t cmd;
	u_int32_t seq;

	u_int8_t lat_deg:7;
	u_int8_t lat_sign:1;

	u_int16_t lat_min;

	u_int8_t lon_deg;
	u_int16_t lon_min;


	u_int8_t bearing:7;
	u_int8_t lon_sign:1;

	u_int8_t knots;

	u_int8_t hdop:7;
	u_int8_t fix:1;


	union {
		struct {
			u_int32_t secs:6;
			u_int32_t mins:6;
			u_int32_t hour:5;
			u_int32_t day:5;
			u_int32_t month:4;
			u_int32_t year:6;
		} parts;
		u_int32_t epoch;
	} time;

} PACKED frm_cmd_gps_t;

typedef frm_cmd_gps_t *frm_cmd_gps_p;


//typedef struct {
//
//
//} frm_cmd_decoded_gps_t;
//
//typedef frm_cmd_decoded_gps_t *frm_cmd_decoded_gps_p;



#define KNOTS_TO_KMPH  1.852
#define MIN_TO_DEC 	  60000.0
#define GPS_DECODE_SPEED(knots) (knots * KNOTS_TO_KMPH)
#define GPS_DECODE_LOC(sign,deg,min) ( (deg+min/MIN_TO_DEC) * (1+sign*-2))
#define GPS_DECODE_LOC_1M(sign,deg,min) ( (deg+min/MIN_TO_DEC) * (1+sign*-2) * 1000000)
#define GPS_DECODE_BEARING(bearing) (bearing*4)

#define GPS_ENCODE_LOCMIN(loc)  ( (loc - (int) loc) * MIN_TO_DEC * (loc<0?-1:1) )
#define GPS_ENCODE_SPEED(kpmh) (kmph / KNOTS_TO_KMPH)
#define GPS_ENCODE_BEARING(bearing) (bearing/4)



int frame_xor_checksum(unsigned char *buffer, size_t offset, size_t len);
int frame_test_transport(unsigned char *buffer, size_t len );
int frame_test_gps(unsigned char *buffer, size_t len);
int frame_decode_gps(unsigned char *buffer, size_t len, void *dst, size_t *gpsLen);
int frame_decode_transport(unsigned char *buffer, size_t len);
int frame_encode_transport(int ns, void *src, size_t srcLen, void *dst, size_t *dstLen );
