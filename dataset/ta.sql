CREATE DATABASE testDB;

DROP DATABASE testDB;

USE tpch;

CREATE TABLE nationBack(n_nationkey INT NOT NULL, n_name CHAR(25) NOT NULL, n_regionkey INT NOT NULL, n_comment VARCHAR(152));

SHOW TABLES;

DROP TABLE nationBack;

DESC nation;

INSERT INTO nation VALUES (0,'America',0, 'nothing left');

INSERT INTO nation VALUES ('id86','America',0, 'nothing left');

INSERT INTO orders VALUES (127664,315000,'F',6.5,'2018-02-29',0,'Clerk101',1,'nice service');
DELETE FROM customer WHERE c_custkey=5;

UPDATE partsupp SET ps_availqty=8774 WHERE ps_partkey=12;

SELECT * FROM customer WHERE c_nationkey<10;

SELECT o_orderdate,o_totalprice FROM orders WHERE o_orderdate=1996-01-02;

SELECT customer.c_phone, orders.o_orderstatus FROM customer,orders where customer.c_custkey=orders.o_custkey AND customer.c_name='Customer#000000001';

UPDATE nation SET n_regionkey = 316001 WHERE n_nationkey= 15;

CREATE TABLE NATION_EMPTY (N_NATIONKEY INT, N_NAME CHAR(25), N_REGIONKEY INT NOT NULL, N_COMMENT VARCHAR(152), PRIMARY KEY(N_NATIONKEY));

ALTER TABLE nation_empty ADD n_comment_2 varchar(32); -- should be nation

ALTER TABLE nation_empty drop n_comment_2; -- should be nation

ALTER TABLE nation RENAME TO province;

ALTER TABLE nation ADD PRIMARY KEY (n_nationkey);

ALTER TABLE NATION ADD CONSTRAINT NATION_FK1 FOREIGN KEY (N_REGIONKEY) references REGION(R_REGIONKEY);

ALTER TABLE nation DROP FOREIGN KEY NATION_FK1;

ALTER TABLE customer ADD INDEX Idx_residual(c_acctbal);

ALTER TABLE customer DROP INDEX Idx_residual;

SELECT MAX(p_size) FROM part where P_PARTKEY=P_PARTKEY;

SELECT AVG(p_size) FROM part where P_PARTKEY=P_PARTKEY;

SELECT MIN(p_size) FROM part where P_PARTKEY=P_PARTKEY;

SELECT SUM(p_size) FROM part where P_PARTKEY=P_PARTKEY;

SELECT o_totalprice FROM orders WHERE o_clerk like 'Clerk.*';

SELECT customer.c_name, orders.o_orderstatus, nation.n_nationkey FROM customer,orders,nation WHERE customer.c_custkey=orders.o_custkey AND customer.c_nationkey=nation.n_nationkey AND nation.n_name='CHINA';

SELECT customer.c_name,orders.o_orderstatus,nation.n_nationkey FROM customer,orders,nation,region WHERE customer.c_custkey=orders.o_custkey AND customer.c_nationkey=nation.n_nationkey AND nation.n_regionkey=region.r_regionkey AND nation.n_name='CHINA';

SELECT customer.c_name,orders.o_orderstatus,nation.n_nationkey FROM customer,orders,nation,region,lineitem WHERE customer.c_custkey=orders.o_custkey AND customer.c_nationkey=nation.n_nationkey AND nation.n_regionkey=region.r_regionkey AND orders.o_orderkey=lineitem.l_orderkey AND nation.n_name='CHINA';

SELECT MIN(o_totalprice), o_orderkey FROM orders GROUP BY o_orderkey LIMIT 5; -- not implement

SELECT * FROM customer WHERE c_custkey IN (SELECT o_custkey FROM orders WHERE o_totalprice > 10); -- not implement

ALTER TABLE customer ADD CONSTRAINT uidx_id UNIQUE KEY (c_custkey);

ALTER TABLE nation ADD CONSTRAINT fk_nation_region FOREIGN KEY(n_regionkey) REFERENCES region(r_regionkey) ON UPDATE CASCADE; -- not implement


