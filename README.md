# 电商管理后台 (E-Commerce Backend System)

基于 C++ 和 MySQL 的控制台电商管理系统。

## 功能模块

1. **商品管理** - 商品增删改查、分类管理、库存管理
2. **用户管理** - 用户注册/登录、信息管理
3. **订单管理** - 下单、订单查询、状态更新
4. **库存管理** - 入库/出库、库存预警

## 技术栈

- C++17
- MySQL 8.0
- MySQL Connector/C++ 8.0

## 编译运行

### 依赖安装 (macOS)
```bash
brew install mysql-client mysql-connector-c++
```

### 编译
```bash
mkdir build && cd build
cmake .. && make
```

### 数据库初始化
```bash
mysql -u root -p < ../sql/init.sql
```

### 运行
```bash
./ecommerce
```

## 项目结构

```
├── CMakeLists.txt
├── README.md
├── sql/
│   └── init.sql          # 数据库建表+测试数据
├── include/
│   ├── db_connector.h    # 数据库连接
│   ├── product.h         # 商品模块
│   ├── user.h            # 用户模块
│   ├── order.h           # 订单模块
│   └── inventory.h       # 库存模块
├── src/
│   ├── db_connector.cpp
│   ├── product.cpp
│   ├── user.cpp
│   ├── order.cpp
│   ├── inventory.cpp
│   └── main.cpp          # 主入口+菜单
└── lib/                  # 第三方库（可选）
```
