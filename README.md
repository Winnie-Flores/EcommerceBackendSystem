# 电商管理系统 (E-Commerce Management System)

基于 **Qt5 + MySQL** 的桌面端电商管理系统，支持管理员、普通用户、供应商三种角色。

## 功能模块

### 管理员端
| 模块 | 功能 |
|------|------|
| **用户管理** | 用户增删改查、分页浏览、按用户名/角色搜索、角色分配（管理员/普通用户/供应商） |
| **商品管理** | 商品增删改查、分类筛选、上架/下架切换、库存预警显示、按供应商关联查询 |
| **供应商管理** | 供应商信息管理、关联商品查看 |
| **库存管理** | 库存变动日志（入库/出库/补货/调拨）、操作记录追踪 |
| **订单管理** | 订单列表（分页/搜索/状态筛选）、查看详情（含商品小计与供应商信息）、订单状态流转（待支付→已支付→已发货→已完成）、取消订单 |
| **数据报表** | Qt Charts 图表展示销售趋势、分类销售占比等 |
| **个人中心** | 个人信息查看与修改、退出登录 |

### 供应商端
| 模块 | 功能 |
|------|------|
| **商品管理** | 管理自有商品 |
| **订单管理** | 查看/处理与本供应商相关的订单 |
| **数据分析** | 销售数据统计与分析 |

### 普通用户端
- **个人中心**：查看/修改个人信息、退出登录

## 技术栈

| 技术 | 说明 |
|------|------|
| 语言 | C++17 |
| UI 框架 | Qt 5（Widgets + Sql + Charts） |
| 样式 | Fusion 风格 + 自定义 QSS（Fluent 2 亚克力浅白主题） |
| 数据库 | MySQL 8.0 + MySQL Connector/C |
| 加密 | OpenSSL（SHA-256 密码哈希） |
| 图表 | Qt Charts |

## 项目结构

```
├── CMakeLists.txt              # CMake 构建配置
├── README.md
├── style.qss                   # 全局 Fluent 2 亚克力主题样式（700+ 行）
├── sql/
│   ├── init.sql                # 建表 + 初始化种子数据
│   └── mock_data.sql           # 丰富模拟数据（14 用户 / 30 商品 / 12 订单 / 6 评价）
├── include/
│   ├── dbmanager.h             # 数据库连接管理（单例）
│   ├── login_dialog.h          # 登录对话框
│   ├── main_window.h           # 主窗口（侧边导航 + 页面栈）
│   ├── user_page.h             # 用户管理页
│   ├── product_page.h          # 商品管理页
│   ├── supplier_page.h         # 供应商管理页
│   ├── inventory_page.h        # 库存管理页
│   ├── order_page.h            # 订单管理页
│   ├── report_page.h           # 数据报表页
│   ├── profile_page.h          # 个人中心页
│   ├── supplier_product_page.h # 供应商-商品管理页
│   ├── supplier_order_page.h   # 供应商-订单管理页
│   └── supplier_analysis_page.h# 供应商-数据分析页
└── src/
    ├── main.cpp                # 程序入口（登录循环 + 样式加载）
    ├── dbmanager.cpp           # MySQL 连接管理
    ├── login_dialog.cpp        # 登录界面
    ├── main_window.cpp         # 主界面（侧边导航栏 + 页面路由）
    ├── user_page.cpp           # 用户 CRUD
    ├── product_page.cpp        # 商品 CRUD
    ├── supplier_page.cpp       # 供应商管理
    ├── inventory_page.cpp      # 库存日志
    ├── order_page.cpp          # 订单管理
    ├── report_page.cpp         # 销售报表
    ├── profile_page.cpp        # 个人设置
    ├── supplier_product_page.cpp
    ├── supplier_order_page.cpp
    └── supplier_analysis_page.cpp
```

## 环境要求

- **操作系统**：macOS（Apple Silicon / Intel）
- **编译器**：支持 C++17（Apple Clang）
- **Qt 5**（Widgets / Sql / Charts）
- **MySQL 8.0** + MySQL Connector/C
- **OpenSSL**（通过 Homebrew 安装）
- **CMake** ≥ 3.16

## 快速开始

### 1. 安装依赖

```bash
# Qt 5（Anaconda 版本，默认路径 /opt/anaconda3）
# 如使用其他 Qt 发行版，请修改 CMakeLists.txt 中的 QT_PATH

# MySQL
brew install mysql

# OpenSSL
brew install openssl
```

### 2. 初始化数据库

启动 MySQL 后执行：

```bash
# 创建数据库并导入表结构
mysql -u root -p --default-character-set=utf8mb4 < sql/init.sql

# （可选）导入丰富模拟数据
mysql -u root -p --default-character-set=utf8mb4 < sql/mock_data.sql
```

> **注意**：务必加上 `--default-character-set=utf8mb4`，否则中文会乱码。

### 3. 编译

```bash
mkdir build && cd build
cmake .. && make -j$(sysctl -n hw.logicalcpu)
```

### 4. 配置数据库连接

在 `build/` 目录下创建 `ecommerce.ini`：

```ini
[Database]
Host=127.0.0.1
Port=3306
User=root
Password=你的MySQL密码
Name=ecommerce
```

程序启动时也会提示输入连接信息，并自动保存。

### 5. 运行

```bash
cd build
./ECommerce
```

## 预置账号

| 角色 | 用户名 | 密码 |
|------|--------|------|
| 管理员 | `admin` | `admin123` |
| 普通用户 | `testuser` | `user123` |
| 供应商 | `supplier1` | `supp123` |

> 导入 `mock_data.sql` 后有更多测试账号可用。

## 模拟数据一览（mock_data.sql）

| 数据类别 | 数量 | 说明 |
|---------|------|------|
| 用户 | 14 人 | 管理员 1 + 普通用户 8 + 供应商 5 |
| 商品 | 30 个 | 覆盖 7 个分类（含下架 + 库存预警商品） |
| 订单 | 12 个 | 覆盖全部 5 种状态（待支付/已支付/已发货/已完成/已取消） |
| 订单明细 | 21 条 | 每条订单 1-2 个商品 |
| 商品评价 | 6 条 | 评分 3-5 分 |
| 库存日志 | 36 条 | 覆盖入库/出库/补货/调拨 4 种操作 |

## 数据库配置

程序运行时从 `build/ecommerce.ini` 读取数据库配置，启动失败时会弹出界面让用户修改。

如果更换 MySQL 连接参数，新配置会自动保存到 `ecommerce.ini` 中。
