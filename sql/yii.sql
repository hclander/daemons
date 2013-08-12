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
