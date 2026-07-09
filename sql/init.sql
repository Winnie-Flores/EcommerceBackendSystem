-- ============================================
-- 电商管理后台 - 数据库初始化
-- ============================================

CREATE DATABASE IF NOT EXISTS ecommerce
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_unicode_ci;

USE ecommerce;

DROP TABLE IF EXISTS product_reviews;
DROP TABLE IF EXISTS order_items;
DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS inventory_log;
DROP TABLE IF EXISTS products;
DROP TABLE IF EXISTS suppliers;
DROP TABLE IF EXISTS categories;
DROP TABLE IF EXISTS users;

-- ----------------------------
-- 用户表
-- ----------------------------
CREATE TABLE users (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    username    VARCHAR(50)  NOT NULL UNIQUE,
    password    VARCHAR(64)  NOT NULL COMMENT 'SHA256',
    real_name   VARCHAR(50)  NOT NULL,
    phone       VARCHAR(20)  DEFAULT '',
    email       VARCHAR(100) DEFAULT '',
    address     VARCHAR(255) DEFAULT '',
    role        TINYINT      NOT NULL DEFAULT 0 COMMENT '0-普通用户 1-管理员 2-供应商',
    avatar      MEDIUMTEXT   DEFAULT '' COMMENT '头像base64',
    balance     DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- 商品分类表
-- ----------------------------
CREATE TABLE categories (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    name        VARCHAR(50)  NOT NULL UNIQUE,
    description VARCHAR(255) DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- 供应商表
-- ----------------------------
CREATE TABLE suppliers (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    name        VARCHAR(100) NOT NULL,
    contact     VARCHAR(50)  DEFAULT '',
    phone       VARCHAR(20)  DEFAULT '',
    email       VARCHAR(100) DEFAULT '',
    address     VARCHAR(255) DEFAULT '',
    user_id     INT          DEFAULT NULL,
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- 商品表
-- ----------------------------
CREATE TABLE products (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    name        VARCHAR(100) NOT NULL,
    category_id INT          NOT NULL,
    supplier_id INT          DEFAULT NULL,
    price       DECIMAL(12,2) NOT NULL,
    cost        DECIMAL(12,2) NOT NULL DEFAULT 0.00,
    stock       INT          NOT NULL DEFAULT 0,
    alert_stock INT          NOT NULL DEFAULT 10 COMMENT '库存预警阈值',
    description TEXT,
    image_url   VARCHAR(255) DEFAULT '',
    status      TINYINT      NOT NULL DEFAULT 1 COMMENT '1-上架 0-下架',
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(id),
    FOREIGN KEY (supplier_id) REFERENCES suppliers(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- 库存变动日志
-- ----------------------------
CREATE TABLE inventory_log (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    product_id  INT          NOT NULL,
    type        TINYINT      NOT NULL COMMENT '0-入库 1-出库',
    quantity    INT          NOT NULL,
    before_stock INT         NOT NULL,
    after_stock  INT         NOT NULL,
    remark      VARCHAR(255) DEFAULT '',
    operator_id INT,
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (product_id) REFERENCES products(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- 订单表
-- ----------------------------
CREATE TABLE orders (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    order_no    VARCHAR(32)  NOT NULL UNIQUE COMMENT '订单编号',
    user_id     INT          NOT NULL,
    total_amount DECIMAL(12,2) NOT NULL,
    status      TINYINT      NOT NULL DEFAULT 0 COMMENT '0-待支付 1-已支付 2-已发货 3-已完成 4-已取消',
    address     VARCHAR(255) DEFAULT '',
    remark      VARCHAR(255) DEFAULT '',
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- 订单明细表
-- ----------------------------
CREATE TABLE order_items (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    order_id    INT          NOT NULL,
    product_id  INT          NOT NULL,
    quantity    INT          NOT NULL,
    price       DECIMAL(12,2) NOT NULL COMMENT '下单时单价',
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------
-- 商品评价表
-- ----------------------------
CREATE TABLE product_reviews (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    order_id    INT NOT NULL,
    product_id  INT NOT NULL,
    user_id     INT NOT NULL,
    rating      TINYINT NOT NULL COMMENT '评分 1-5',
    content     TEXT,
    created_at  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    UNIQUE KEY uk_order_product (order_id, product_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================
-- 测试数据
-- ============================================

-- 管理员 (密码: admin123)
INSERT INTO users (username, password, real_name, phone, email, role, balance)
VALUES ('admin', '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9',
        '系统管理员', '13800000000', 'admin@shop.com', 1, 0.00);

-- 普通用户 (密码: user123)
INSERT INTO users (username, password, real_name, phone, email, role, balance)
VALUES ('user1', 'e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446',
        '张三', '13900000001', 'zhangsan@test.com', 0, 10000.00);

-- 供应商账号 (密码: supp123)
INSERT INTO users (username, password, real_name, phone, email, role, balance)
VALUES ('supplier1', '9b5d6c3b1c6dcbcb020a97fef4464786693489f9da335d6dcaa624e31aa836eb',
        '李明', '13810001000', 'lm@huawei-sc.com', 2, 0.00),
       ('supplier2', '9b5d6c3b1c6dcbcb020a97fef4464786693489f9da335d6dcaa624e31aa836eb',
        '王芳', '13810002000', 'wf@uniqlo-sc.com', 2, 0.00),
       ('supplier3', '9b5d6c3b1c6dcbcb020a97fef4464786693489f9da335d6dcaa624e31aa836eb',
        '赵强', '13810003000', 'zq@3songshu.com', 2, 0.00),
       ('supplier4', '9b5d6c3b1c6dcbcb020a97fef4464786693489f9da335d6dcaa624e31aa836eb',
        '孙丽', '13810004000', 'sl@ikea-sc.com', 2, 0.00);

-- 分类
INSERT INTO categories (name, description) VALUES
('电子产品', '手机、电脑、数码配件等'),
('服装鞋帽', '男装、女装、鞋类等'),
('食品饮料', '零食、饮料、生鲜等'),
('家居用品', '家具、厨具、日用品等'),
('图书音像', '书籍、音乐、影视等');

-- 供应商
INSERT INTO suppliers (name, contact, phone, email, address, user_id) VALUES
('华为供应链', '李明', '13810001000', 'lm@huawei-sc.com', '深圳市龙岗区', 3),
('优衣库供应商', '王芳', '13810002000', 'wf@uniqlo-sc.com', '上海市浦东新区', 4),
('三只松鼠', '赵强', '13810003000', 'zq@3songshu.com', '芜湖市弋江区', 5),
('宜家供应链', '孙丽', '13810004000', 'sl@ikea-sc.com', '北京市朝阳区', 6);

-- 商品
INSERT INTO products (name, category_id, supplier_id, price, cost, stock, alert_stock, description, status) VALUES
('华为Mate 60 Pro', 1, 1, 6999.00, 5500.00, 50, 5, '旗舰智能手机', 1),
('AirPods Pro 2', 1, 1, 1899.00, 1400.00, 100, 10, '降噪耳机', 1),
('男士羽绒服', 2, 2, 899.00, 500.00, 200, 20, '冬季保暖羽绒服', 1),
('运动跑鞋', 2, 2, 599.00, 300.00, 150, 15, '轻便透气', 1),
('坚果大礼包', 3, 3, 128.00, 80.00, 500, 30, '每日坚果混合装', 1),
('进口牛奶', 3, 3, 68.00, 45.00, 300, 20, '1L装全脂牛奶', 1),
('北欧书桌', 4, 4, 1299.00, 800.00, 30, 5, '简约现代风格', 1),
('智能台灯', 4, 4, 299.00, 180.00, 80, 8, '护眼LED', 1),
('C++ Primer Plus', 5, NULL, 89.00, 60.00, 200, 20, 'C++经典教材', 1),
('三体全集', 5, NULL, 99.00, 70.00, 300, 30, '刘慈欣科幻巨著', 1);
