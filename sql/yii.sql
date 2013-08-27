CHARSET utf8;
DROP DATABASE IF EXISTS Yii;
CREATE DATABASE Yii DEFAULT CHARACTER SET utf8;
USE Yii;

CREATE TABLE empresas_tbl (
	empresa_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	nombre VARCHAR(128) NOT NULL,
	direccion VARCHAR(256) NOT NULL DEFAULT '',
	PRIMARY KEY (empresa_id)
) ENGINE=INNODB DEFAULT CHARSET=utf8;


CREATE TABLE usuarios_tbl (
	usuario_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	login VARCHAR(128) NOT NULL,
	clave VARCHAR(128) NOT NULL,
	nombre VARCHAR(256) NOT NULL DEFAULT '',
	email VARCHAR(128) NOT NULL DEFAULT '',
	empresa_id INT UNSIGNED NOT NULL,
	PRIMARY KEY (usuario_id),
	KEY empresa_id__IDX (empresa_id),
	FOREIGN KEY (empresa_id) REFERENCES empresas_tbl(empresa_id) ON DELETE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;


CREATE TABLE localizables_tbl (
	localizable_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	nombre VARCHAR(256) NOT NULL DEFAULT '',
	empresa_id INT UNSIGNED NOT NULL,
	PRIMARY KEY (localizable_id),
	KEY empresa_id__IDX (empresa_id),
	FOREIGN KEY (empresa_id) REFERENCES empresas_tbl(empresa_id) ON DELETE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8;


CREATE TABLE `rx_tbl` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `ts` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `ip` int unsigned NOT NULL,
  `port` smallint unsigned NOT NULL,
  `ns` int unsigned NOT NULL,
  `status` tinyint NOT NULL DEFAULT '0',
  `len` smallint unsigned NOT NULL,
  `data` blob NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

CREATE TABLE gps_tbl (
	gps_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	cmd TINYINT UNSIGNED NOT NULL,
	len SMALLINT unsigned not null DEFAULT 0,
	seq_l SMALLINT unsigned not null,
	seq_s SMALLINT unsigned not null,
	lat INT NOT NULL,
	lon INT NOT NULL,
	bearing SMALLINT UNSIGNED NOT NULL,
	speed SMALLINT UNSIGNED NOT NULL,
	knots SMALLINT UNSIGNED NOT NULL,
	fix TINYINT UNSIGNED NOT NULL,
	hdop TINYINT UNSIGNED NOT NULL,
	time BIGINT NOT NULL,
	rx_id INT UNSIGNED NOT NULL,
	localizable_id INT UNSIGNED NOT NULL,
	`ts` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY (gps_id),
	KEY localizable_id__IDX (localizable_id),
	FOREIGN KEY (localizable_id) REFERENCES localizables_tbl(localizable_id) ON DELETE CASCADE
) ENGINE=InnoDB;

DROP TABLE IF EXISTS sensor_tbl;
create table sensor_tbl (
    sensor_id   INT UNSIGNED NOT NULL,
    sensor_name  VARCHAR(32) NOT NULL,
    PRIMARY KEY (sensor_id)
) ENGINE=InnoDB ;

DROP TABLE IF EXISTS sensor_localizable_tbl;
create table sensor_localizable_tbl (
    sensor_localizable_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    localizable_id INT UNSIGNED NOT NULL,
    sensor_type TINYINT UNSIGNED NOT NULL,
    sensor_idx  TINYINT UNSIGNED NOT NULL,
    sensor_id   INT UNSIGNED NOT NULL,
    PRIMARY KEY (sensor_localizable_id),
    KEY (localizable_id),
    KEY (sensor_id),
    KEY (sensor_type,sensor_idx),    
    CONSTRAINT FOREIGN KEY (sensor_id) REFERENCES sensor_tbl (sensor_id),
    CONSTRAINT FOREIGN KEY (localizable_id) REFERENCES localizables_tbl(localizable_id)
) ENGINE=InnoDB;


DROP TABLE IF EXISTS sensor_value_tbl;
create table sensor_value_tbl (
    sensor_value_id INT UNSIGNED NOT NULL AUTO_INCREMENT,    
    sensor_type TINYINT UNSIGNED NOT NULL,
    sensor_idx  TINYINT UNSIGNED NOT NULL,
    sensor_id   INT UNSIGNED,
    sensor_value SMALLINT NOT NULL,
    time BIGINT UNSIGNED NOT NULL,
    rx_id INT UNSIGNED NOT NULL,
    localizable_id INT UNSIGNED NOT NULL,
    ts TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (sensor_value_id),
    KEY (sensor_type,sensor_idx),
    KEY (sensor_id),
    KEY (localizable_id),
    KEY (rx_id),
    CONSTRAINT  FOREIGN KEY (rx_id) REFERENCES rx_tbl (id)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS cnx_tbl;
CREATE TABLE cnx_tbl (
    cnx_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
    imei VARCHAR(15) NOT NULL,
    mode SMALLINT UNSIGNED NOT NULL,
    version SMALLINT UNSIGNED NOT NULL,
    reason SMALLINT UNSIGNED NOT NULL,
    rx_id INT UNSIGNED NOT NULL,
    localizable_id INT UNSIGNED NOT NULL,
    time BIGINT UNSIGNED NOT NULL,
    ts TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (cnx_id),
    KEY (localizable_id),
    KEY (rx_id),
    CONSTRAINT FOREIGN KEY (rx_id) REFERENCES rx_tbl (id)    
) ENGINE=InnoDB;



INSERT INTO empresas_tbl (empresa_id, nombre) VALUES 
(1, 'Empresa 1'),
(2, 'Empresa 2');

INSERT INTO usuarios_tbl (usuario_id, login, clave, nombre, empresa_id) VALUES 
(1, 'admin1', sha1('admin1'), 'Alex1', 1),
(2, 'admin2', sha1('admin2'), 'Alex2', 2);

INSERT INTO localizables_tbl (localizable_id, nombre, empresa_id) VALUES
(1, 'Localizable 1', 1),
(2, 'Localizable 2', 1),
(3, 'Localizable 3', 2),
(4, 'Localizable 4', 2);

INSERT INTO gps_tbl (gps_id, cmd, seq_l,seq_s, lat, lon, bearing, speed, knots, fix, hdop, time, rx_id, localizable_id) VALUES
(1, 0, 0,0, 42212030, -8650317, 0, 10, 0, 0, 0, UNIX_TIMESTAMP(), 0, 1),
(2, 0, 0,0, 42212030, -8650317, 0, 20, 0, 0, 0, UNIX_TIMESTAMP(), 0, 1),
(3, 0, 0,0, 42212030, -8650317, 0, 30, 0, 0, 0, UNIX_TIMESTAMP(), 0, 1);

INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(0,"Contacto");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(1,"Sensor 1");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(2,"Sensor 2");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(3,"Sensor 3");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(4,"Sensor 4");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(5,"Sensor 5");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(6,"Sensor 6");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(7,"Sensor 7");

INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(8,"Sensor 8");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(9,"Sensor 9");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(10,"Sensor 10");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(11,"Sensor 11");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(12,"Sensor 12");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(13,"Sensor 13");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(14,"Sensor 14");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(15,"Sensor 15");

INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(16,"Sensor 16");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(17,"Sensor 17");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(18,"Sensor 18");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(19,"Sensor 19");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(20,"Sensor 20");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(21,"Sensor 21");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(22,"Sensor 22");
INSERT INTO sensor_tbl(sensor_id,sensor_name) VALUE(23,"Sensor 23");

INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,0,0x0D,0x00);
INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,1,0x0D,0x01);
INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,2,0x0D,0x02);
INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,3,0x0D,0x03);
INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,4,0x0D,0x04);
INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,5,0x0D,0x05);
INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,6,0x0D,0x06);
INSERT INTO sensor_localizable_tbl (localizable_id, sensor_id, sensor_type, sensor_idx) VALUES (1,7,0x0D,0x07);

/* FUNCIONES */

delimiter $$
DROP PROCEDURE IF EXISTS proc_gps_getAllRows$$
CREATE PROCEDURE proc_gps_getAllRows()
BEGIN
    SELECT
        gps_id, 
        seq_l,
        lat / 1000000.0 as lat, 
        lon / 1000000.0 as lon, 
        bearing, 
        speed, 
        fix, 
        hdop, 
        from_unixtime(time) as time, 
        time as epoch,
        localizable_id
     FROM
        gps_tbl
     ORDER BY gps_id DESC;
END
$$

DROP PROCEDURE IF EXISTS proc_gps_getLast5Rows$$
CREATE PROCEDURE proc_gps_getLast5Rows()
BEGIN  
    SELECT
        gps_id, 
        seq_l,
        lat / 1000000.0 as lat, 
        lon / 1000000.0 as lon, 
        bearing, 
        speed, 
        fix, 
        hdop, 
        from_unixtime(time) as time, 
        time as epoch,
        localizable_id
     FROM
        gps_tbl
     ORDER BY gps_id DESC
     LIMIT 5;
END
$$

DROP PROCEDURE IF EXISTS proc_gps_getLastRows$$
CREATE PROCEDURE proc_gps_getLastRows(numRows int)
BEGIN
/*
  Parece que MySql no soporta parametros en la clausula LIMIT por lo que hay que usar
  una consulta dinámica usando PREPARE STMT
*/
    PREPARE STMT FROM 
      " SELECT  
         gps_id, 
         seq_l,
         lat / 1000000.0 as lat, 
         lon / 1000000.0 as lon, 
         bearing, 
         speed, 
         fix, 
         hdop, 
         from_unixtime(time) as time, 
         time as epoch,
         localizable_id 
       FROM 
        gps_tbl g
       ORDER BY gps_id DESC
       LIMIT ? "; 

    SET @LIMIT = numRows;
    EXECUTE STMT USING @LIMIT;
    DEALLOCATE PREPARE STMT;
END
$$

DROP PROCEDURE IF EXISTS proc_gps_findLatLonWithPrec$$
CREATE PROCEDURE proc_gps_findLatLonWithPrec(aLat float, aLon float, aPrec int)
BEGIN
    SELECT
        gps_id, 
        seq_l,
        lat / 1000000.0 as lat, 
        lon / 1000000.0 as lon, 
        bearing, 
        speed, 
        fix, 
        hdop, 
        from_unixtime(time) as time, 
        time as epoch,
        localizable_id
     FROM
        gps_tbl g
     WHERE
        ABS(g.lat - TRUNCATE(aLat * 1000000,0))<=aPrec
        and ABS(g.lon -TRUNCATE(aLon * 1000000,0))<=aPrec
     ORDER BY gps_id DESC
     LIMIT 100;
END
$$

DROP PROCEDURE IF EXISTS proc_gps_findLatLon$$
CREATE PROCEDURE proc_gps_findLatLon(aLat float, aLon float)
BEGIN
  call proc_gps_findLatLonWithPrec(aLat, aLon, 50);
END
$$

delimiter ;


