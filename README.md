# DB-project
## 编译运行
```sh
mkdir build
cd build
cmake ../src
make -j6
./main.out
```
## 记录管理
使用CS346中的页式文件系统与系统设计，实现了新建文件、删除文件、打开文件、关闭文件、插入记录、删除记录、更新记录、获取属性值满足特定条件的记录，进行了简单的单元测试。
### 系统设计
* 第一页记录文件中的的记录大小，每页的记录条数
* 之后每一页，最前面使用一个01串表示对应位置是否存在记录

## 索引管理
基于B树实现了新建索引，删除索引，打开索引，关闭索引，插入节点，删除节点，获取属性值满足特定条件的节点，进行了简单的单元测试。

### B树

使用B树而非B+树的原因是，在CS346的架构中，数据的值可能存在大量重复，因而需要将RID也当做树排序的关键字，具体分析见中期报告。

将记录管理模块作为B树的底层文件系统，B树的每个节点是记录管理模块的一条记录。

B树的实现参考了数据结构课的代码，使用模板，并对与底层文件系统的交互进行封装。因而，可以将该部分代码方便地迁移到其他工程中美。

B树每个节点的孩子数是一个可调变量，后期可通过测试调节其最优值。

### 扫描
扫描时，会去掉明显不可能包含合法记录的节点，并维护这个B树节点及其子树是否全是合法记录。通过这些减小比较次数、遍历节点数，加快扫描。

## SQL命令解析

模仿编译原理课decaf大作业的框架，基于lex/yacc实现了所有文法规则的解析及抽象语法树的构造

## 系统管理 

### 功能

实现了数据库的创建、删除、切换，表的创建删除，主键、外键、unique键的添加删除，列的添加删除，索引的添加删除。在操作前都进行了大量合法性检查，以保证数据库的完整性约束。

### 性能优化

自动为主键建立索引，以加快插入删除速度。

## 查询解析

### 功能

实现了增删改查，SELECT支持多表连接、聚集查询，SET支持四则运算，WHERE子句支持常见条件表达式、NULL的判断、整数和字符串比较、正则匹配。在对数据库进行操作前都先进行检查，主要包括命名检查、类型检查、约束检查等。

### 性能优化

多表连接实现为表从小到大依次连接，所有运算子句都先从名字编译至偏移量，利用索引优化完整性约束的检查过程。

