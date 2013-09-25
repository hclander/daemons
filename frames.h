// Estructuras para meter y sacar datos de las tramas

#ifndef FRAMES_H_
#define FRAMES_H_

#include <sys/types.h>

#define PACKED __attribute__((__packed__))

#define TRANS_MAX_BUFF_SIZE 1500   //TODO: Revisar esto

#define TRANS_HEADER_START_SIZE 1
#define TRANS_HEADER_LENGTH_SIZE 2
#define TRANS_HEADER_SIZE (TRANS_HEADER_START_SIZE + TRANS_HEADER_LENGTH_SIZE)

#define TRANS_HEADER_SERIAL_SIZE 4
#define TRANS_PREAMBLE_SIZE   (TRANS_HEADER_SIZE + TRANS_HEADER_SERIAL_SIZE)

#define TRANS_DATA_OFFSET  TRANS_PREAMBLE_SIZE

#define TRANS_FOOTER_XOR_SIZE 1
#define TRANS_FOOTER_CRC_SIZE 2

#define TRANS_FOOTER_SIZE TRANS_FOOTER_XOR_SIZE

#define TRANS_OVERLOAD (TRANS_PREAMBLE_SIZE + TRANS_FOOTER_SIZE)

typedef union {

	struct  {
		u_int8_t version:4;
		u_int8_t undefined:2;
		u_int8_t crc:1;
		u_int8_t lendian:1;
	}  ;

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
	unsigned char data[];
} PACKED transport_buf_t;

typedef transport_buf_t *transport_buf_p;


// TRANSPORT START MASKS

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
#define FRAME_CMD_GPS_OLD   0x13
#define FRAME_CMD_RTC_TIME  0x15
#define FRAME_CMD_PROBE		0xA0
#define FRAME_CMD_SENSOR0	0xD0
#define FRAME_CMD_SENSOR1   0xE0
#define FRAME_CMD_SENSOR2   0xF0
#define FRAME_CMD_PROBE_SENSOR_MASK 0xF0


#define SENSOR_TYPE_0 0x0D
#define SENSOR_TYPE_1 0x0E
#define SENSOR_TYPE_2 0x0F

// FRAME SERVER COMMANDS

#define FRAME_SVR_CMD_ACK  0xEF


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

	u_int8_t len;

	u_int16_t seq_l;
	u_int16_t seq_s;

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


#define GPS_PREAMBLE_SIZE  2	// frm_cmd_gps_t.cmd  and frm_cmd_gps_t.len
#define GPS_FRAME_SIZE 	   sizeof(frm_cmd_gps_t)
#define GPS_DATA_LEN	   (GPS_FRAME_SIZE - GPS_PREAMBLE_SIZE )

//trama extendida    : 1 bit
//long. trama    : 2 bits  (siempre 01)
//hora        : 5 bits
//--
//min.        : 6 bits
//seg.        : 6 bits
//rumbo_2        : 1 bit
//fix        : 1 bit
//contacto    : 1 bit
//E/W        : 1 bit
//--
//min. lat.    : 16 bits
//--
//min. long.    : 16 bits
//--
//rumbo_1        : 1 bit
//velocidad    : 7 bits
//--
//rumbo_0        : 1 bit
//hdop: 7 bits
//--
//N/S        : 1 bit
//grados lat.    : 7 bits
//--
//grados long.    : 8 bits
//--
//día        : 5 bits
//año        : 2 bits
//encendido    : 1 bit (solo aplicable en tramas de posiciones, aunque podría aportar información en tramas de monitor)
//--
//mes        : 4 bits
//sensor_movim.    : 1 bit (a partir de la v150)
//solicitud_asent.: 1 bit (solo aplicable en la trama de monitor ; a partir de la v40)
//ultima_trama    : 1 bit (solo aplicable en la trama de monitor ; a partir de la v38)
//ee_fin        : 1 bit    (solo aplicable en la trama de monitor)
//--
//bloque_fin    : 8 bits    (solo aplicable en la trama de monitor)
//-

typedef struct {

	u_int8_t   cmd;
	u_int8_t   len;

	u_int16_t  seq_l;
	u_int16_t  seq_s;

	u_int8_t   hour:5;
	u_int8_t   size :2;
	u_int8_t   ext :1;

	// Una pequeña gran faena que los mins y los secs esten en bytes distintos...

	union {
		struct {
			u_int16_t  lon_sign:1;  //E/W
			u_int16_t  ign :1;      // ignition contacto
			u_int16_t  fix :1;
			u_int16_t  bear2 :1;      //bearing 2
			u_int16_t  secs :6;
			u_int16_t  mins :6;
		} parts;

		struct {
			u_int8_t byte0;
			u_int8_t byte1;

		} asBytes;

		u_int16_t  asWord;
	} data;

	u_int16_t  lat_min;
	u_int16_t  lon_min;

	u_int8_t   knots:7;
	u_int8_t   bear1 :1;

	u_int8_t   hdop  :7;
	u_int8_t   bear0 :1;

	u_int8_t   lat_deg:7;
	u_int8_t   lat_sign:1;   // N/S


	u_int8_t   lon_deg;

	u_int8_t   turnon: 1;
	u_int8_t   year: 2;
	u_int8_t   day : 5;

	u_int8_t   ee_end:1;
	u_int8_t   last_frm: 1;
	u_int8_t   ack_req: 1;
	u_int8_t   mov_sen: 1;
	u_int8_t   month:4;

	u_int8_t   blk_end;


} PACKED frm_cmd_gps_old_t;

typedef frm_cmd_gps_old_t *frm_cmd_gps_old_p;

typedef union {

	frm_cmd_gps_t     cur;   // current
	frm_cmd_gps_old_t old;

} PACKED frm_cmd_gps_all_t;

typedef frm_cmd_gps_all_t *frm_cmd_gps_all_p;


typedef union {


} frm_cmd_hist_t;


// TRAMA DE CONEXION
//- 0x06 = Conexión: Lon(1) IMEI(15) IMEI_XOR(1) MO(1) VS(2) MC(1) [preferiblemente irá acompañando a una trama Gps] [tb. podría usar el formato "viejo" dentro de una trama compatible; en este caso tb. sería preferible que fuese precedida de una trama Gps]
//            - Lon: puede ser 0 (toma valor implícito) o >20 (cuando queramos añadir bytes nuevos)
//            - IMEI: podría ser guardado en Ascii en otra zona de la memoria (o en "hexa" por el método viejo, en el lugar "viejo")
//            - IMEI_XOR: XOR de los dígitos del IMEI (se alamcena con el mismo método que se almacene el IMEI)
//            - MO: Modo operación (0, 4, 5)
//            - VS: Versión Software
//            - MC: Motivo Conexión

typedef struct {
	u_int8_t cmd;
	u_int8_t len;
	u_int8_t imei[15];
	u_int8_t ixor;
	u_int8_t om;  // Operation mode;
	u_int16_t sv; // Software Version
	u_int8_t cr ;  // Conection Reason

} PACKED frm_cmd_cnx_t;

typedef frm_cmd_cnx_t *frm_cmd_cnx_p;


// Sensores

typedef union {
	u_int8_t cmd;
	struct {
		u_int8_t value:1;
		u_int8_t num:3;
		u_int8_t type:4;

	} sensor;

} PACKED frm_cmd_sensor_t;

typedef frm_cmd_sensor_t *frm_cmd_sensor_p;


typedef struct {
	u_int8_t cmd;
	u_int8_t len;
	u_int32_t sn;

} PACKED frm_cmd_ack_t;
typedef frm_cmd_ack_t *frm_cmd_ack_p;

//typedef struct {
//
//} PACKED frm_cmd_ack_E9_t;
//
//typedef frm_cmd_ack_E9_t *frm_cmd_ack_E9_p;



//typedef struct {
//
//
//} frm_cmd_decoded_gps_t;
//
//typedef frm_cmd_decoded_gps_t *frm_cmd_decoded_gps_p;

#define GPS_STRUCT_MIN_SIZE (sizeof(frm_cmd_gps_old_t)<sizeof(frm_cmd_gps_t)?sizeof(frm_cmd_gps_old_t):sizeof(frm_cmd_gps_t))

#define KNOTS_TO_KMPH  1.852
#define MIN_TO_DEC 	  60000.0
#define GPS_DECODE_SPEED(knots) (knots * KNOTS_TO_KMPH)
#define GPS_DECODE_LOC(sign,deg,min) ( (deg+min/MIN_TO_DEC) * (1+sign*-2))
#define GPS_DECODE_LOC_1M(sign,deg,min) ( (deg+min/MIN_TO_DEC) * (1+sign*-2) * 1000000)
#define GPS_DECODE_BEARING(bearing) (bearing*4)
#define GPS_DECODE_OLD_BEARING(br2,br1,br0)  ( ((br2<<2) | (br1<<1) | br0)*45 )

#define SEN_DECODE_CMD(cmd) ( (cmd) & FRAME_CMD_PROBE_SENSOR_MASK)
#define SEN_DECODE_TYPE(cmd) ( (cmd)>>4 )

#define GPS_ENCODE_LOCMIN(loc)  ( (loc - (int) loc) * MIN_TO_DEC * (loc<0?-1:1) )
#define GPS_ENCODE_SPEED(kpmh) (kmph / KNOTS_TO_KMPH)
#define GPS_ENCODE_BEARING(bearing) (bearing/4)



int frame_xor_checksum(unsigned char *buffer, size_t offset, size_t len);
int frame_test_transport(unsigned char *buffer, size_t len );
int frame_test_gps(unsigned char *buffer, size_t *len);
int frame_test_gps_old(unsigned char *buffer, size_t *len);
int frame_decode_gps(unsigned char *buffer, size_t len, void *dst, size_t *gpsLen);
int frame_decode_gps_old(unsigned char *buffer, size_t len, void *dst, size_t *gpsLen);
int frame_decode_transport(unsigned char *buffer, size_t len);
int frame_encode_transport(int ns, void *src, size_t srcLen, void *dst, size_t *dstLen );
int frame_encode_ack(long serialNumber, int cmd, void *dst, size_t *len);
int frame_test_cnx(void *src, size_t *len);
int frame_encode_cnx(char *imei,int om, int sv, int cr, void *dst,size_t *len);
int frame_encode_gps(int seq,int bearing,int knots, float lat, float lon, int fix, int hdop, time_t aTime, void *dst ,size_t *len);

// DECODE MANAGER
int  frame_decodermanager_init();
int  frame_decodermanager_registerAll();
void frame_decodermanager_finish();
int  frame_decodermanager_decode(void *buf, size_t size, int *count);

#endif //FRAMES_H_
