use orderDB;
-- 索引
alter table supplier add index idx1(S_NATIONKEY);
desc supplier;
alter table supplier drop index idx1;
-- 多表连接
select supplier.S_NAME, nation.N_NAME, region.R_NAME from supplier, nation, region where supplier.S_NATIONKEY=nation.N_NATIONKEY and nation.N_REGIONKEY=region.R_REGIONKEY;
-- 聚集查询
select sum(S_ACCTBAL) from supplier where supplier.S_SUPPKEY=supplier.S_SUPPKEY;
select avg(S_ACCTBAL) from supplier where supplier.S_SUPPKEY=supplier.S_SUPPKEY;
select max(S_ACCTBAL) from supplier where supplier.S_SUPPKEY=supplier.S_SUPPKEY;
select min(S_ACCTBAL) from supplier where supplier.S_SUPPKEY=supplier.S_SUPPKEY;
-- 模糊查询
select * from supplier where S_COMMENT like '.*even.*';
-- 复杂表达式作为更新内容
update lineitem set L_TAX = (L_QUANTITY + L_EXTENDEDPRICE * L_DISCOUNT) * 3.0 where L_ORDERKEY = L_ORDERKEY;
select L_ORDERKEY, L_PARTKEY, L_QUANTITY, L_EXTENDEDPRICE, L_DISCOUNT, L_TAX from lineitem where L_ORDERKEY < 5;
-- unique key
create table REGION2 (R_REGIONKEY INT, R_NAME CHAR(25) NOT NULL, R_COMMENT VARCHAR(152), PRIMARY KEY(R_REGIONKEY));
copy region2 from '../../dataset/region.tbl';
desc region2;
alter table region2 add constraint name_con unique key (R_NAME);
insert into region2 values(5, 'MARS', 'mars');
insert into region2 values(6, 'MARS', 'mars mars');
alter table region2 drop unique key name_con;