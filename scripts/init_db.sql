CREATE DATABASE IF NOT EXISTS userdb;
use userdb;

CREATE TABLE IF NOT EXISTS users (
  id INTEGER UNSIGNED PRIMARY KEY /*!40101 AUTO_INCREMENT */,
  username VARCHAR(64) NOT NULL UNIQUE,
  password TEXT
);

CREATE TABLE IF NOT EXISTS igdPermission (
  id INTEGER UNSIGNED PRIMARY KEY /*!40101 AUTO_INCREMENT */,
  user_id INTEGER UNSIGNED,
  permission TEXT,
  FOREIGN KEY (user_id) REFERENCES users(id)
);

INSERT INTO users (username, password) VALUES ('user1', '$argon2id$v=19$m=102400,t=2,p=8$wPlCNl16IBJEi06WY6D8Ag$W7S0Qqy1eD0RtHQ43oVm/w');
INSERT INTO igdPermission (user_id, permission) VALUES ((SELECT id from users where username = 'user1'), \
      '{"pub_port_range": ["1024-1030", "30000-30002"], "int_ip_range": ["172.16.0.0/24", "172.30.2.128/25"]}');