-- ============================================
-- 电商管理后台 - 模拟测试数据
-- 运行方式: mysql -u root -p < sql/mock_data.sql
-- 前提: 已通过 init.sql 完成数据库初始化
-- ============================================

USE ecommerce;

-- ============================================
-- 清理旧数据（保留 init.sql 创建的基线数据）
-- ============================================

-- 删除模拟数据产生的记录（反向按外键依赖顺序）
DELETE FROM product_reviews WHERE id > 0;
DELETE FROM order_items WHERE id > 0;
DELETE FROM orders WHERE id > 0;
DELETE FROM inventory_log WHERE id > 0;
DELETE FROM products WHERE id > 10;
DELETE FROM suppliers WHERE id > 4;
DELETE FROM categories WHERE id > 5;
DELETE FROM users WHERE id > 6;

-- 重置自增 ID，确保后续插入获得可预测的 ID
ALTER TABLE users           AUTO_INCREMENT = 7;
ALTER TABLE categories      AUTO_INCREMENT = 6;
ALTER TABLE suppliers       AUTO_INCREMENT = 5;
ALTER TABLE products        AUTO_INCREMENT = 11;
ALTER TABLE inventory_log   AUTO_INCREMENT = 1;
ALTER TABLE orders          AUTO_INCREMENT = 1;
ALTER TABLE order_items     AUTO_INCREMENT = 1;
ALTER TABLE product_reviews AUTO_INCREMENT = 1;

-- ============================================
-- Part 1: 更多普通用户 (密码均为 user123)
-- ID 起: 7
-- ============================================
INSERT INTO users (username, password, real_name, phone, email, role, balance, address) VALUES
('zhangwei',  'e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446', '张伟',   '13900010001', 'zhangwei@qq.com',      0, 5000.00, '北京市海淀区中关村大街1号'),
('liuxia',   'e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446', '刘霞',   '13900010002', 'liuxia@163.com',        0, 8000.00, '上海市徐汇区漕溪北路888号'),
('chenjie',  'e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446', '陈杰',   '13900010003', 'chenjie@gmail.com',     0, 3000.00, '广州市天河区体育西路100号'),
('wangmei',  'e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446', '王梅',   '13900010004', 'wangmei@126.com',       0, 15000.00, '深圳市南山区科技园南路2号'),
('huanglei', 'e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446', '黄磊',   '13900010005', 'huanglei@outlook.com',  0, 2000.00, '杭州市西湖区文三路456号'),
('zhoujing', 'e606e38b0d8c19b24cf0ee3808183162ea7cd63ff7912dbb22b5e803286b4446', '周静',   '13900010006', 'zhoujing@qq.com',       0, 12000.00, '成都市武侯区天府大道789号');

-- ============================================
-- Part 2: 新增供应商账号
-- ID 起: 13 (users), 5 (suppliers)
-- ============================================
INSERT INTO users (username, password, real_name, phone, email, role, balance) VALUES
('supplier5', '9b5d6c3b1c6dcbcb020a97fef4464786693489f9da335d6dcaa624e31aa836eb', '钱进',  '13810005000', 'qj@xiaomi-sc.com', 2, 0.00),
('supplier6', '9b5d6c3b1c6dcbcb020a97fef4464786693489f9da335d6dcaa624e31aa836eb', '吴婷',  '13810006000', 'wt@jd-sc.com',     2, 0.00);

INSERT INTO suppliers (name, contact, phone, email, address, user_id) VALUES
('小米供应链', '钱进', '13810005000', 'qj@xiaomi-sc.com', '北京市海淀区清河中街',   13),
('京东供应链', '吴婷', '13810006000', 'wt@jd-sc.com',     '北京市亦庄经济开发区',  14);

-- ============================================
-- Part 3: 新增商品分类
-- ID 起: 6
-- ============================================
INSERT INTO categories (name, description) VALUES
('运动户外', '健身器材、户外装备、运动服饰等'),
('美妆个护', '护肤品、彩妆、个人护理等');

-- ============================================
-- Part 4: 新增商品 (20个，ID 从 11 开始)
-- 覆盖全部 7 个分类，含上下架/库存预警场景
-- ============================================
INSERT INTO products (name, category_id, supplier_id, price, cost, stock, alert_stock, description, status) VALUES
-- 电子产品 (category_id=1)
('iPad Air M2',           1, 5, 4799.00, 3800.00,  60,  5, '11英寸 M2芯片 平板电脑',          1),
('小米14 Ultra',          1, 5, 5999.00, 4800.00,  35,  5, '徕卡光学 旗舰影像',               1),
('罗技MX Master 3S',      1, 6,  699.00,  480.00, 120, 10, '无线蓝牙鼠标 8K DPI',            1),
('Keychron K8 Pro',       1, 6,  599.00,  420.00,  80,  8, '机械键盘 热插拔 蓝牙双模',        1),
('Anker USB-C扩展坞',     1, 6,  299.00,  200.00, 200, 15, '7合1 Type-C Hub 4K@60Hz',       1),

-- 服装鞋帽 (category_id=2)
('女士碎花连衣裙',         2, 2,  359.00,  200.00, 180, 15, '夏季新款 法式复古风',            1),
('男士商务衬衫',           2, 2,  259.00,  140.00, 220, 20, '免烫修身 长袖正装衬衫',          1),
('经典帆布鞋',             2, 2,  199.00,  100.00, 350, 25, '低帮百搭 潮流休闲鞋',            1),
('羊毛围巾',               2, 2,  129.00,   70.00, 150, 15, '100%纯羊毛 冬季保暖 格纹款',     1),

-- 食品饮料 (category_id=3)
('明前龙井茶礼盒',         3, 3,  388.00,  260.00, 120, 10, '2026年新茶 特级西湖龙井',        1),
('费列罗榛果巧克力礼盒',   3, 3,  168.00,  110.00, 280, 20, '24粒装 进口巧克力',              1),
('即食冰糖燕窝',           3, 3,  498.00,  360.00,  80, 10, '6瓶装 印尼进口 即食滋补',         1),
('法国进口干红葡萄酒',     3, 3,  268.00,  180.00, 160, 15, 'AOC级别 750ml 波尔多产区',       1),

-- 家居用品 (category_id=4)
('记忆棉护颈枕头',         4, 4,  199.00,  120.00, 300, 20, '慢回弹 颈椎支撑 60x40cm',        1),
('不锈钢炒锅套装',         4, 4,  599.00,  380.00,  90, 10, '316不锈钢 不粘锅 三件套',         1),

-- 图书音像 (category_id=5)
('Python编程从入门到实践', 5, NULL, 89.00,  58.00, 250, 20, '第3版 全彩印刷 Python 3.x',     1),
('人类简史',               5, NULL, 68.00,  42.00, 400, 30, '尤瓦尔·赫拉利 从动物到上帝',      1),

-- 运动户外 (category_id=6)
('瑜伽垫 TPE加厚',         6, 5,  159.00,   90.00, 200, 20, '6mm加厚 防滑 环保TPE材质',        1),
('户外折叠椅',             6, 6,  129.00,   75.00, 150, 15, '铝合金轻量 便携露营椅',            1),

-- 美妆个护 (category_id=7)
('氨基酸洁面乳',           7, 6,  89.00,   50.00, 500, 30, '温和清洁 敏感肌可用 150ml',        1);

-- 个别商品设为下架 (status=0)，用于测试上下架切换
UPDATE products SET status = 0 WHERE name = '户外折叠椅';

-- 个别商品库存接近或低于预警线，用于测试库存预警
UPDATE products SET stock = 2  WHERE name = '小米14 Ultra';
UPDATE products SET stock = 5  WHERE name = '法国进口干红葡萄酒';
UPDATE products SET stock = 8  WHERE name = '即食冰糖燕窝';

-- ============================================
-- Part 5: 库存变动日志
-- ID 起: 1
-- ============================================

-- 原有商品 (id 1-10) 初始入库
INSERT INTO inventory_log (product_id, type, quantity, before_stock, after_stock, remark, operator_id) VALUES
(1,  0, 50,  0,  50,  '初始入库', 1),
(2,  0, 100, 0,  100, '初始入库', 1),
(3,  0, 200, 0,  200, '初始入库', 1),
(4,  0, 150, 0,  150, '初始入库', 1),
(5,  0, 500, 0,  500, '初始入库', 1),
(6,  0, 300, 0,  300, '初始入库', 1),
(7,  0, 30,  0,  30,  '初始入库', 1),
(8,  0, 80,  0,  80,  '初始入库', 1),
(9,  0, 200, 0,  200, '初始入库', 1),
(10, 0, 300, 0,  300, '初始入库', 1);

-- 新增商品 (id 11-30) 新品入库
INSERT INTO inventory_log (product_id, type, quantity, before_stock, after_stock, remark, operator_id) VALUES
(11, 0, 60,  0,  60,  '新品入库', 1),
(12, 0, 37,  0,  37,  '新品入库', 1),
(13, 0, 120, 0,  120, '新品入库', 1),
(14, 0, 80,  0,  80,  '新品入库', 1),
(15, 0, 200, 0,  200, '新品入库', 1),
(16, 0, 180, 0,  180, '新品入库', 1),
(17, 0, 220, 0,  220, '新品入库', 1),
(18, 0, 350, 0,  350, '新品入库', 1),
(19, 0, 150, 0,  150, '新品入库', 1),
(20, 0, 120, 0,  120, '新品入库', 1),
(21, 0, 280, 0,  280, '新品入库', 1),
(22, 0, 84,  0,  84,  '新品入库', 1),
(23, 0, 163, 0,  163, '新品入库', 1),
(24, 0, 300, 0,  300, '新品入库', 1),
(25, 0, 90,  0,  90,  '新品入库', 1),
(26, 0, 250, 0,  250, '新品入库', 1),
(27, 0, 400, 0,  400, '新品入库', 1),
(28, 0, 200, 0,  200, '新品入库', 1),
(29, 0, 150, 0,  150, '新品入库', 1),
(30, 0, 500, 0,  500, '新品入库', 1);

-- 补货/调拨/出库调整
INSERT INTO inventory_log (product_id, type, quantity, before_stock, after_stock, remark, operator_id) VALUES
(1,  0, 10, 50,  60,  '补货入库', 1),
(4,  0, 50, 150, 200, '调拨入库', 1),
(7,  0, 15, 30,  45,  '补货入库', 1),
(12, 0, 5,  37,  42,  '紧急补货', 1),
(9,  1, 10, 200, 190, '促销活动出库', 1),
(6,  1, 20, 300, 280, '批发出库', 1);

-- 更新商品库存以匹配入库日志
UPDATE products SET stock = 60  WHERE id = 1;
UPDATE products SET stock = 200 WHERE id = 4;
UPDATE products SET stock = 45  WHERE id = 7;
UPDATE products SET stock = 42  WHERE id = 12;
UPDATE products SET stock = 190 WHERE id = 9;
UPDATE products SET stock = 280 WHERE id = 6;

-- ============================================
-- Part 6: 订单 (ID 从 1 开始，覆盖 0-4 全部状态)
-- ============================================

-- === 已完成订单 (status=3) ===
INSERT INTO orders (id, order_no, user_id, total_amount, status, address, remark, created_at) VALUES
(1, '20260625103000007', 7, 8898.00, 3, '北京市海淀区中关村大街1号',          '请发顺丰',            '2026-06-25 10:30:00'),
(2, '20260628141500008', 8, 1498.00, 3, '上海市徐汇区漕溪北路888号',          '',                    '2026-06-28 14:15:00'),
(3, '20260701092000009', 9,  392.00, 3, '广州市天河区体育西路100号',          '公司团购',            '2026-07-01 09:20:00');

INSERT INTO order_items (order_id, product_id, quantity, price) VALUES
(1, 1, 1, 6999.00), (1, 2, 1, 1899.00),
(2, 3, 1,  899.00), (2, 4, 1,  599.00),
(3, 5, 2,  128.00), (3, 6, 2,   68.00);

-- === 已发货订单 (status=2) ===
INSERT INTO orders (id, order_no, user_id, total_amount, status, address, remark, created_at) VALUES
(4, '202607051600000010', 10, 1598.00, 2, '深圳市南山区科技园南路2号',          '送货上门',            '2026-07-05 16:00:00'),
(5, '20260706113000007',  7,  188.00, 2, '北京市海淀区中关村大街1号',          '',                    '2026-07-06 11:30:00'),
(6, '20260707144500008',  8, 4799.00, 2, '上海市徐汇区漕溪北路888号',          '礼品包装',            '2026-07-07 14:45:00');

INSERT INTO order_items (order_id, product_id, quantity, price) VALUES
(4, 7, 1, 1299.00), (4, 8, 1,  299.00),
(5, 9, 1,   89.00), (5, 10, 1,  99.00),
(6, 11, 1, 4799.00);

-- === 已支付订单 (status=1) ===
INSERT INTO orders (id, order_no, user_id, total_amount, status, address, remark, created_at) VALUES
(7, '20260708090000009',   9, 5999.00, 1, '广州市天河区体育西路100号',          '急用 请尽快发货',     '2026-07-08 09:00:00'),
(8, '202607081015000011', 11,  898.00, 1, '杭州市西湖区文三路456号',            '',                    '2026-07-08 10:15:00'),
(9, '202607090930000012', 12,  556.00, 1, '成都市武侯区天府大道789号',          '',                    '2026-07-09 09:30:00');

INSERT INTO order_items (order_id, product_id, quantity, price) VALUES
(7, 12, 1, 5999.00),
(8, 13, 1,  699.00), (8, 18, 1, 199.00),
(9, 20, 1,  388.00), (9, 21, 1, 168.00);

-- === 待支付订单 (status=0) ===
INSERT INTO orders (id, order_no, user_id, total_amount, status, address, remark, created_at) VALUES
(10, '20260709100000007',   7, 898.00, 0, '北京市海淀区中关村大街1号',          '办公使用',            '2026-07-09 10:00:00'),
(11, '202607091130000010', 10, 398.00, 0, '深圳市南山区科技园南路2号',          '',                    '2026-07-09 11:30:00');

INSERT INTO order_items (order_id, product_id, quantity, price) VALUES
(10, 14, 1, 599.00), (10, 15, 1, 299.00),
(11, 24, 2, 199.00);

-- === 已取消订单 (status=4) ===
INSERT INTO orders (id, order_no, user_id, total_amount, status, address, remark, created_at) VALUES
(12, '20260703160000008', 8,  899.00, 4, '上海市徐汇区漕溪北路888号',          '尺码选错,重新下单',   '2026-07-03 16:00:00');

INSERT INTO order_items (order_id, product_id, quantity, price) VALUES
(12, 3, 1, 899.00);

-- 重置 orders AUTO_INCREMENT 到 13
ALTER TABLE orders AUTO_INCREMENT = 13;

-- ============================================
-- Part 7: 商品评价 (仅已完成订单可评价)
-- ============================================
INSERT INTO product_reviews (order_id, product_id, user_id, rating, content, created_at) VALUES
(1, 1, 7, 5, '华为Mate 60 Pro各方面体验都很棒，拍照效果惊艳，系统流畅不卡顿，续航优秀！',  '2026-07-02 09:00:00'),
(1, 2, 7, 4, 'AirPods降噪效果不错，就是戴久了耳朵有点不舒服。',                              '2026-07-02 09:15:00'),
(2, 3, 8, 5, '羽绒服很保暖，做工精细，尺码标准，这个冬天就靠它了！',                          '2026-07-05 14:00:00'),
(2, 4, 8, 4, '跑鞋透气性好，轻便舒适，就是鞋底偏软，适合日常慢跑。',                          '2026-07-05 14:10:00'),
(3, 5, 9, 5, '坚果大礼包分量足，种类丰富，日期新鲜，同事们都说好吃！',                        '2026-07-08 18:00:00'),
(3, 6, 9, 3, '牛奶还可以，不过包装有点简单，运输过程中有一盒轻微变形。',                      '2026-07-08 18:20:00');

-- ============================================
-- Part 8: 更新用户余额（扣除已完成和已发货订单的金额）
-- ============================================
UPDATE users SET balance = balance - 8898.00 WHERE id = 7;
UPDATE users SET balance = balance - 1498.00 WHERE id = 8;
UPDATE users SET balance = balance -  392.00 WHERE id = 9;
UPDATE users SET balance = balance - 1598.00 WHERE id = 10;
UPDATE users SET balance = balance -  188.00 WHERE id = 7;
UPDATE users SET balance = balance - 4799.00 WHERE id = 8;
UPDATE users SET balance = balance -  898.00 WHERE id = 11;

-- ============================================
-- 导入结果汇总
-- ============================================
SELECT '=== 用户 ===' AS '';
SELECT id, username, real_name, role, balance FROM users ORDER BY id;

SELECT '=== 分类 ===' AS '';
SELECT id, name FROM categories ORDER BY id;

SELECT '=== 供应商 ===' AS '';
SELECT s.id, s.name, s.contact FROM suppliers s ORDER BY s.id;

SELECT '=== 商品 (库存预警 + 上下架状态) ===' AS '';
SELECT id, name, price, stock, alert_stock,
       CASE WHEN stock <= alert_stock THEN '⚠ 预警' ELSE '✓ 正常' END AS 库存,
       CASE WHEN status=1 THEN '上架' ELSE '下架' END AS 状态
FROM products ORDER BY id;

SELECT '=== 订单 ===' AS '';
SELECT id, order_no, user_id, total_amount,
       CASE status WHEN 0 THEN '待支付' WHEN 1 THEN '已支付' WHEN 2 THEN '已发货'
                    WHEN 3 THEN '已完成' WHEN 4 THEN '已取消' END AS 状态,
       DATE(created_at) AS 日期
FROM orders ORDER BY id;

SELECT '=== 评价 ===' AS '';
SELECT pr.id, u.real_name AS 用户, p.name AS 商品, pr.rating AS 评分,
       LEFT(pr.content, 25) AS 内容
FROM product_reviews pr
JOIN users u ON pr.user_id = u.id
JOIN products p ON pr.product_id = p.id
ORDER BY pr.id;
