DROP VIEW IF EXISTS invalid;

DROP TABLE IF EXISTS quers;
DROP TABLE IF EXISTS meta;
DROP TABLE IF EXISTS joins;
DROP TABLE IF EXISTS locks;
DROP TABLE IF EXISTS main;
DROP TABLE IF EXISTS users;

CREATE TABLE joins
(
  colindex int(10) NOT NULL,
  srctable varchar(64) NOT NULL,
  srckey varchar(64) NOT NULL,
  srcval varchar(64) NOT NULL,

  PRIMARY KEY (colindex)
);

CREATE TABLE locks
(
  sheet int(10) UNSIGNED NOT NULL,
  user int(10) UNSIGNED NOT NULL,
  PRIMARY KEY (sheet),
  KEY (user)
);

CREATE TABLE main
(
  id int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  path varchar(64) NOT NULL,
  user int(10) UNSIGNED DEFAULT NULL,
  time datetime DEFAULT CURRENT_TIMESTAMP(),
  PRIMARY KEY (id),
  UNIQUE KEY (path),
  KEY (user)
);

CREATE TABLE meta
(
  colindex int(10) NOT NULL,
  name varchar(64) DEFAULT NULL,
  fill tinyint(1) DEFAULT 0,
  hide tinyint(1) DEFAULT 0,
  block tinyint(1) DEFAULT 0,
  PRIMARY KEY (colindex),
  UNIQUE KEY (name)
);

CREATE TABLE quers
(
  name varchar(32) NOT NULL,
  label varchar(256) NOT NULL,
  PRIMARY KEY (name)
);

CREATE TABLE users
(
  id int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  name varchar(64) NOT NULL,
  pass varchar(64) NOT NULL,
  PRIMARY KEY (id),
  UNIQUE KEY (name)
);

CREATE VIEW invalid AS
  SELECT id, user FROM main WHERE 1=1;

ALTER TABLE locks
  ADD CONSTRAINT locks_sheet FOREIGN KEY (sheet) REFERENCES main (id) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT locks_user FOREIGN KEY (user) REFERENCES users (id) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE main
  ADD CONSTRAINT main_user FOREIGN KEY (user) REFERENCES users (id) ON DELETE SET NULL ON UPDATE CASCADE;
