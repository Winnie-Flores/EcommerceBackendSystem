const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  ImageRun, HeadingLevel, AlignmentType, BorderStyle, WidthType,
  PageBreak, NumberFormat, TabStopPosition, TabStopType,
  SectionType, Footer, PageNumber, Header
} = require("docx");
const fs = require("fs");
const path = require("path");

// ============================================================
// 通用样式
// ============================================================
const IMG_DIR = "/tmp/report_images";
const PAGE_WIDTH = 11906; // A4
const MARGIN = 1440; // 1 inch

function imgBuffer(name) {
  return fs.readFileSync(path.join(IMG_DIR, name));
}

function titlePara(text) {
  return new Paragraph({
    children: [new TextRun({ text, bold: true, size: 32, font: "SimHei" })],
    alignment: AlignmentType.CENTER,
    spacing: { after: 200 },
  });
}

function h1(text) {
  return new Paragraph({
    children: [new TextRun({ text, bold: true, size: 28, font: "SimHei", color: "1a5276" })],
    heading: HeadingLevel.HEADING_1,
    spacing: { before: 400, after: 200 },
  });
}

function h2(text) {
  return new Paragraph({
    children: [new TextRun({ text, bold: true, size: 24, font: "SimHei", color: "2471a3" })],
    heading: HeadingLevel.HEADING_2,
    spacing: { before: 300, after: 150 },
  });
}

function h3(text) {
  return new Paragraph({
    children: [new TextRun({ text, bold: true, size: 22, font: "SimHei", color: "2e86c1" })],
    heading: HeadingLevel.HEADING_3,
    spacing: { before: 200, after: 100 },
  });
}

function p(text, opts = {}) {
  return new Paragraph({
    children: [new TextRun({ text, size: 21, font: "SimSun", ...opts })],
    spacing: { after: 120, line: 360 },
    indent: opts.indent ? { firstLine: 420 } : undefined,
  });
}

function boldP(label, text) {
  return new Paragraph({
    children: [
      new TextRun({ text: label, bold: true, size: 21, font: "SimSun" }),
      new TextRun({ text, size: 21, font: "SimSun" }),
    ],
    spacing: { after: 100, line: 340 },
  });
}

function bullet(text, level = 0) {
  return new Paragraph({
    children: [new TextRun({ text: "• " + text, size: 21, font: "SimSun" })],
    spacing: { after: 60, line: 340 },
    indent: { left: 400 + level * 400 },
  });
}

function numbered(num, text) {
  return new Paragraph({
    children: [new TextRun({ text: `${num}. ${text}`, size: 21, font: "SimSun" })],
    spacing: { after: 60, line: 340 },
    indent: { left: 400 },
  });
}

function imagePara(buf, width, desc) {
  const paras = [
    new Paragraph({
      children: [
        new ImageRun({
          data: buf,
          transformation: { width: width || 500, height: Math.round((width || 500) * 0.6) },
        }),
      ],
      alignment: AlignmentType.CENTER,
      spacing: { before: 200, after: 100 },
    }),
  ];
  if (desc) {
    paras.push(
      new Paragraph({
        children: [new TextRun({ text: desc, size: 18, italics: true, font: "SimSun", color: "666666" })],
        alignment: AlignmentType.CENTER,
        spacing: { after: 200 },
      })
    );
  }
  return paras;
}

function cell(text, opts = {}) {
  return new TableCell({
    children: [new Paragraph({
      children: [new TextRun({ text: String(text), size: 20, font: "SimSun", bold: opts.bold || false })],
      alignment: opts.align || AlignmentType.CENTER,
    })],
    width: opts.width ? { size: opts.width, type: WidthType.PERCENTAGE } : undefined,
    shading: opts.shading ? { fill: opts.shading } : undefined,
    verticalAlign: "center",
  });
}

function makeTable(headers, rows, colWidths) {
  const headerRow = new TableRow({
    children: headers.map((h, i) => cell(h, {
      bold: true,
      width: colWidths ? colWidths[i] : undefined,
      shading: "1a5276",
    })),
    tableHeader: true,
  });
  // override header text color (need white)
  const headerRowWhite = new TableRow({
    children: headers.map((h, i) => new TableCell({
      children: [new Paragraph({
        children: [new TextRun({ text: String(h), size: 20, bold: true, font: "SimSun", color: "FFFFFF" })],
        alignment: AlignmentType.CENTER,
      })],
      width: colWidths ? { size: colWidths[i], type: WidthType.PERCENTAGE } : undefined,
      shading: { fill: "1a5276" },
      verticalAlign: "center",
    })),
  });

  const dataRows = rows.map((row, idx) =>
    new TableRow({
      children: row.map((c, i) => cell(c, {
        width: colWidths ? colWidths[i] : undefined,
        shading: idx % 2 === 1 ? "eaf2f8" : undefined,
      })),
    })
  );

  return new Table({
    rows: [headerRowWhite, ...dataRows],
    width: { size: 100, type: WidthType.PERCENTAGE },
  });
}

function emptyLine() {
  return new Paragraph({ children: [new TextRun({ text: "", size: 12 })], spacing: { after: 100 } });
}

// ============================================================
// 报告1：电商管理系统_实验报告.docx（详细技术报告）
// ============================================================
async function generateReport1() {
  const doc = new Document({
    sections: [
      // ---- 封面 ----
      {
        properties: {
          page: { margin: { top: MARGIN, bottom: MARGIN, left: MARGIN, right: MARGIN } },
        },
        children: [
          emptyLine(),emptyLine(),emptyLine(),emptyLine(),
          new Paragraph({
            children: [new TextRun({ text: "电商管理系统", size: 52, bold: true, font: "SimHei", color: "1a5276" })],
            alignment: AlignmentType.CENTER,
            spacing: { after: 300 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "实验报告", size: 48, bold: true, font: "SimHei", color: "2e86c1" })],
            alignment: AlignmentType.CENTER,
            spacing: { after: 600 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "基于 Qt5 + MySQL 的企业级管理系统", size: 24, font: "SimSun", color: "666666" })],
            alignment: AlignmentType.CENTER,
            spacing: { after: 200 },
          }),
          emptyLine(),emptyLine(),
          new Paragraph({
            children: [new TextRun({ text: "技术栈：C++17、Qt5 (Widgets + SQL)、MySQL、QSS Fusion 主题", size: 21, font: "SimSun", color: "555555" })],
            alignment: AlignmentType.CENTER, spacing: { after: 100 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "开发环境：macOS + CMake + Xcode CLT", size: 21, font: "SimSun", color: "555555" })],
            alignment: AlignmentType.CENTER, spacing: { after: 400 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "2026年7月", size: 24, font: "SimSun", color: "888888" })],
            alignment: AlignmentType.CENTER,
          }),
        ],
      },

      // ---- 目录页 ----
      {
        properties: {
          page: { margin: { top: MARGIN, bottom: MARGIN, left: MARGIN, right: MARGIN } },
        },
        children: [
          h1("目 录"),
          p("一、项目概述", { bold: true }),
          p("二、需求分析"),
          p("三、系统架构设计"),
          p("四、数据库设计"),
          p("五、功能模块详解"),
          p("    5.1  登录与注册"),
          p("    5.2  用户管理"),
          p("    5.3  商品管理"),
          p("    5.4  供应商管理"),
          p("    5.5  库存管理"),
          p("    5.6  订单管理"),
          p("    5.7  数据报表"),
          p("    5.8  个人信息管理"),
          p("六、UI 设计与用户体验"),
          p("七、测试与部署"),
          p("八、项目总结"),
        ],
      },

      // ---- 正文 ----
      {
        properties: {
          page: {
            margin: { top: MARGIN, bottom: MARGIN, left: MARGIN, right: MARGIN },
            pageNumbers: { start: 1 },
          },
        },
        headers: {
          default: new Header({
            children: [
              new Paragraph({
                children: [new TextRun({ text: "电商管理系统 · 实验报告", size: 18, font: "SimSun", color: "999999" })],
                alignment: AlignmentType.RIGHT,
              }),
            ],
          }),
        },
        footers: {
          default: new Footer({
            children: [
              new Paragraph({
                children: [
                  new TextRun({ text: "第 ", size: 18, font: "SimSun" }),
                  new TextRun({ children: [PageNumber.CURRENT], size: 18, font: "SimSun" }),
                  new TextRun({ text: " 页", size: 18, font: "SimSun" }),
                ],
                alignment: AlignmentType.CENTER,
              }),
            ],
          }),
        },
        children: [
          // === 第一章 ===
          h1("一、项目概述"),

          h2("1.1 项目背景"),
          p("随着电子商务的蓬勃发展，电商平台需要一个高效、可靠的后台管理系统来支撑日常运营。"
            + "本系统面向小型电商企业，提供用户管理、商品上下架、供应商协作、"
            + "库存监控、订单处理及数据报表等核心功能。"),
          p("系统采用 C/S 架构（客户端-服务器模式），前端使用 Qt5 构建跨平台桌面应用，"
            + "后端通过 MySQL 数据库存储业务数据。系统支持三种角色：管理员、供应商和普通用户，"
            + "不同角色拥有不同的功能权限和数据访问范围。"),

          h2("1.2 项目目标"),
          bullet("构建功能完整的电商后台管理系统，覆盖采购、库存、销售全链路"),
          bullet("实现多角色权限控制，确保数据安全和操作合规"),
          bullet("提供直观友好的图形用户界面，降低操作学习成本"),
          bullet("支持实时库存监控和智能预警"),
          bullet("生成多维数据报表，辅助经营决策"),

          h2("1.3 技术选型"),
          makeTable(
            ["层级", "技术", "版本", "说明"],
            [
              ["开发语言", "C++", "C++17", "标准模板库 + 面向对象设计"],
              ["GUI 框架", "Qt5", "5.15+", "Widgets 模块 + SQL 模块"],
              ["数据库", "MySQL", "8.0", "InnoDB 引擎，UTF-8MB4 编码"],
              ["构建工具", "CMake", "3.16+", "跨平台构建系统"],
              ["样式方案", "QSS", "—", "Fluent2 亚克力浅白主题"],
              ["版本控制", "Git", "—", "本地仓库 + 远程推送"],
            ],
            [20, 20, 20, 40]
          ),
          emptyLine(),

          h2("1.4 项目规模"),
          makeTable(
            ["指标", "数值"],
            [
              ["C++ 源文件 (.cpp)", "13 个"],
              ["C++ 头文件 (.h)", "12 个"],
              ["SQL 脚本", "3 个（建表 + 模拟数据 + 触发器）"],
              ["QSS 样式表", "1 个（约 360 行）"],
              ["总代码行数", "约 3,500 行"],
              ["功能模块", "10 个独立页面"],
              ["数据库表", "8 张核心业务表"],
              ["预置模拟数据", "14 用户、30 商品、5 供应商、12 订单"],
            ],
            [50, 50]
          ),
          emptyLine(),

          // === 第二章 ===
          h1("二、需求分析"),

          h2("2.1 角色与权限"),
          p("系统设计了三类角色，每类角色拥有不同的功能模块访问权限："),
          emptyLine(),

          // 插入权限图
          ...imagePara(imgBuffer("05_role_permissions.png"), 520, "图 2-1  角色权限矩阵"),

          makeTable(
            ["角色", "权限级别", "可访问模块", "数据可见范围"],
            [
              ["管理员 (admin)", "最高权限", "全部 7 个模块", "全局数据"],
              ["供应商 (supplier)", "受限权限", "我的商品、我的订单、数据分析、个人信息", "仅本供应商数据"],
              ["普通用户 (user)", "基础权限", "订单管理、个人信息", "仅个人订单"],
            ],
            [15, 15, 40, 30],
          ),
          emptyLine(),

          h2("2.2 功能需求"),
          p("（1）用户管理：管理员可查看所有用户列表、禁用/启用用户、修改用户角色。", { indent: true }),
          p("（2）商品管理：管理员可添加/编辑/删除商品，设置价格、库存、所属供应商。", { indent: true }),
          p("（3）供应商管理：管理员管理供应商信息（名称、联系人、电话等）。", { indent: true }),
          p("（4）库存管理：实时查看各商品库存量，记录库存变更日志，支持低库存预警。", { indent: true }),
          p("（5）订单管理：用户下单 → 供应商发货 → 用户确认收货，支持 5 种订单状态流转。", { indent: true }),
          p("（6）数据报表：可视化展示销售额趋势、热销商品排行、库存统计等。", { indent: true }),
          p("（7）个人信息：头像上传、密码修改、个人信息编辑、退出登录。", { indent: true }),

          h2("2.3 非功能需求"),
          bullet("界面美观：采用 Fluent2 浅白亚克力风格，统一 QSS 样式表"),
          bullet("响应迅速：C++ 原生编译，无 Web 加载开销"),
          bullet("数据安全：密码 bcrypt 哈希（预留），SQL 参数防注入"),
          bullet("跨平台：Qt5 支持 Windows/macOS/Linux"),

          // === 第三章 ===
          h1("三、系统架构设计"),

          h2("3.1 整体架构"),
          p("系统采用经典的三层 C/S 架构：表现层（Qt Widgets）、业务逻辑层（C++ 类）、数据访问层（Qt SQL + MySQL）。"),
          ...imagePara(imgBuffer("01_system_architecture.png"), 520, "图 3-1  系统架构图"),
          emptyLine(),

          h2("3.2 技术架构"),
          makeTable(
            ["层级", "实现技术", "核心类/组件"],
            [
              ["表现层", "Qt5 Widgets", "MainWindow、各 Page 类、QStackedWidget"],
              ["样式层", "QSS", "style.qss（360 行 Fluent2 主题）"],
              ["业务逻辑层", "C++17 OOP", "DBManager（单例）、各 Feature 类"],
              ["数据访问层", "Qt SQL (QSqlDatabase)", "DBManager::query() 封装"],
              ["数据存储层", "MySQL 8.0 InnoDB", "7 张核心业务表"],
            ],
            [20, 25, 55],
          ),
          emptyLine(),

          h2("3.3 页面导航架构"),
          p("主窗口使用 QStackedWidget 管理多个功能页面，左侧导航菜单通过 QButtonGroup 实现页面切换。"
            + "管理员 7 个页面（索引 0-6），供应商 4 个页面，普通用户 2 个页面。"),
          p("页面切换时自动调用 refresh 方法刷新数据，确保数据实时性。"),

          h2("3.4 设计模式应用"),
          makeTable(
            ["设计模式", "应用场景", "实现"],
            [
              ["单例模式", "数据库连接管理", "DBManager::instance() 全局唯一实例"],
              ["观察者模式", "信号槽机制", "Qt Signal/Slot 实现组件间通信"],
              ["策略模式", "角色权限路由", "根据 role_ 字段加载不同页面集"],
              ["MVC", "表格数据管理", "QTableView + 数据模型 + 数据库查询"],
            ],
            [20, 30, 50],
          ),
          emptyLine(),

          // === 第四章 ===
          h1("四、数据库设计"),

          h2("4.1 ER 模型"),
          ...imagePara(imgBuffer("02_database_er.png"), 520, "图 4-1  数据库 ER 图"),
          emptyLine(),

          h2("4.2 数据表设计"),
          p("系统包含 8 张核心业务表，以下为各表详细结构：", { indent: true }),


          h3("4.2.1 用户表（users）"),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "用户唯一标识"],
              ["username", "VARCHAR(50)", "UNIQUE, NOT NULL", "登录用户名"],
              ["password", "VARCHAR(255)", "NOT NULL", "登录密码（预留加密）"],
              ["role", "TINYINT", "NOT NULL, DEFAULT 1", "1=管理员, 2=供应商, 3=用户"],
              ["phone", "VARCHAR(20)", "—", "手机号"],
              ["email", "VARCHAR(100)", "—", "电子邮箱"],
              ["avatar", "MEDIUMTEXT", "—", "头像 Base64 编码"],
              ["status", "TINYINT", "DEFAULT 1", "1=启用, 0=禁用"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "注册时间"],
            ],
            [18, 18, 22, 42],
          ),
          emptyLine(),

          h3("4.2.2 商品分类表（categories）"),
          p("商品分类目录，用于对商品进行分组管理。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "分类唯一标识"],
              ["name", "VARCHAR(50)", "UNIQUE, NOT NULL", "分类名称（如电子产品、服装鞋帽）"],
              ["description", "VARCHAR(255)", "DEFAULT ''", "分类描述"],
            ],
            [18, 18, 22, 42],
          ),
          emptyLine(),

          h3("4.2.3 商品表（products）"),
          p("记录系统内所有商品信息，关联分类和供应商。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "商品唯一标识"],
              ["name", "VARCHAR(100)", "NOT NULL", "商品名称"],
              ["category_id", "INT", "FK, NOT NULL", "关联 categories.id"],
              ["supplier_id", "INT", "FK, DEFAULT NULL", "关联 suppliers.id（可为空）"],
              ["price", "DECIMAL(12,2)", "NOT NULL", "售价"],
              ["cost", "DECIMAL(12,2)", "NOT NULL, DEFAULT 0", "成本价"],
              ["stock", "INT", "NOT NULL, DEFAULT 0", "当前库存数量"],
              ["alert_stock", "INT", "NOT NULL, DEFAULT 10", "库存预警阈值"],
              ["description", "TEXT", "—", "商品描述"],
              ["image_url", "VARCHAR(255)", "DEFAULT ''", "商品图片 URL"],
              ["status", "TINYINT", "NOT NULL, DEFAULT 1", "1=上架, 0=下架"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "创建时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("4.2.4 供应商表（suppliers）"),
          p("管理供应商基本信息，与用户账号绑定，商品通过外键关联供应商。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "供应商唯一标识"],
              ["name", "VARCHAR(100)", "NOT NULL", "供应商公司名称"],
              ["contact", "VARCHAR(50)", "DEFAULT ''", "联系人姓名"],
              ["phone", "VARCHAR(20)", "DEFAULT ''", "联系电话"],
              ["email", "VARCHAR(100)", "DEFAULT ''", "电子邮箱"],
              ["address", "VARCHAR(255)", "DEFAULT ''", "公司地址"],
              ["user_id", "INT", "FK, DEFAULT NULL", "关联 users.id（供应商登录账号）"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "创建时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("4.2.5 订单表（orders）"),
          p("存储每一笔订单的完整信息，包含订单编号、金额、状态、收货地址等。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "订单唯一标识"],
              ["order_no", "VARCHAR(32)", "UNIQUE, NOT NULL", "订单编号（唯一）"],
              ["user_id", "INT", "FK, NOT NULL", "买家用户 ID"],
              ["total_amount", "DECIMAL(12,2)", "NOT NULL", "订单总金额"],
              ["status", "TINYINT", "NOT NULL, DEFAULT 0", "0=待支付, 1=已支付, 2=已发货, 3=已完成, 4=已取消"],
              ["address", "VARCHAR(255)", "DEFAULT ''", "收货地址"],
              ["remark", "VARCHAR(255)", "DEFAULT ''", "订单备注"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "下单时间"],
              ["updated_at", "DATETIME", "ON UPDATE NOW()", "最后更新时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),
          boldP("订单状态枚举：", "0-待支付 → 1-已支付 → 2-已发货 → 3-已完成；4-已取消（任意非完成状态均可取消）"),
          emptyLine(),

          h2("4.3 订单状态流转"),
          ...imagePara(imgBuffer("03_order_status_flow.png"), 480, "图 4-2  订单状态流转图"),
          emptyLine(),

          h3("4.2.6 订单明细表（order_items）"),
          p("订单与商品的多对多关系表，记录每个订单中各类商品的购买数量和下单时单价。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "明细唯一标识"],
              ["order_id", "INT", "FK, NOT NULL", "关联 orders.id（级联删除）"],
              ["product_id", "INT", "FK, NOT NULL", "关联 products.id"],
              ["quantity", "INT", "NOT NULL", "购买数量"],
              ["price", "DECIMAL(12,2)", "NOT NULL", "下单时的商品单价"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("4.2.7 商品评价表（product_reviews）"),
          p("用户对已收货订单中商品的评价，每个订单的每种商品只允许评价一次。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "评价唯一标识"],
              ["order_id", "INT", "FK, NOT NULL", "关联 orders.id（级联删除）"],
              ["product_id", "INT", "FK, NOT NULL", "关联 products.id"],
              ["user_id", "INT", "FK, NOT NULL", "评价用户 ID"],
              ["rating", "TINYINT", "NOT NULL", "评分 1-5"],
              ["content", "TEXT", "—", "评价文字内容"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "评价时间"],
            ],
            [18, 22, 22, 38],
          ),
          boldP("约束：", "UNIQUE KEY (order_id, product_id) 确保同一订单中同一商品不可重复评价"),
          emptyLine(),

          h3("4.2.8 库存变动日志表（inventory_log）"),
          p("记录每次库存变更的完整信息，用于审计追踪和库存盘点。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK, AUTO_INCREMENT", "日志唯一标识"],
              ["product_id", "INT", "FK, NOT NULL", "关联 products.id"],
              ["type", "TINYINT", "NOT NULL", "0=入库, 1=出库"],
              ["quantity", "INT", "NOT NULL", "变更数量"],
              ["before_stock", "INT", "NOT NULL", "变更前库存"],
              ["after_stock", "INT", "NOT NULL", "变更后库存"],
              ["remark", "VARCHAR(255)", "DEFAULT ''", "变更备注"],
              ["operator_id", "INT", "—", "操作人用户 ID"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "操作时间"],
            ],
            [18, 22, 22, 38],
          ),

          h2("4.4 索引设计"),
          makeTable(
            ["表名", "索引字段", "索引类型", "用途"],
            [
              ["users", "username", "UNIQUE", "登录查询"],
              ["products", "supplier_id", "INDEX", "供应商商品查询"],
              ["orders", "user_id", "INDEX", "用户订单查询"],
              ["orders", "status", "INDEX", "按状态筛选订单"],
              ["order_items", "order_id", "INDEX", "订单明细关联"],
              ["inventory_logs", "product_id", "INDEX", "商品库存历史"],
            ],
            [22, 22, 20, 36],
          ),
          emptyLine(),

          // === 第五章 ===
          h1("五、功能模块详解"),

          h2("5.1 登录与注册模块"),
          p("用户通过账号密码登录系统。系统根据角色自动加载对应功能菜单。"),
          ...imagePara(imgBuffer("screen_login.png"), 360, "图 5-1  系统登录界面"),
          emptyLine(),

          makeTable(
            ["功能点", "实现方式", "说明"],
            [
              ["账号验证", "SQL SELECT 查询", "匹配 username + password"],
              ["角色识别", "查询 role 字段", "1=管理员, 2=供应商, 3=用户"],
              ["用户状态检查", "检查 status 字段", "禁用用户无法登录"],
              ["注册功能", "INSERT 新用户", "新用户默认普通用户角色"],
              ["登录提示", "底部显示预置账号", "方便演示和测试"],
            ],
            [25, 30, 45],
          ),
          emptyLine(),

          h2("5.2 用户管理模块"),
          p("管理员专属功能。展示所有已注册用户，支持搜索、禁用/启用、角色修改操作。"),
          p("主要功能：", { indent: true }),
          bullet("用户列表：表格展示所有用户，支持按用户名搜索"),
          bullet("状态管理：一键禁用/启用用户账号"),
          bullet("角色变更：可修改用户角色（管理员/供应商/普通用户）"),
          bullet("预置数据：14 个测试用户覆盖三种角色"),

          h2("5.3 商品管理模块"),
          p("管理员维护商品目录，支持完整的 CRUD 操作。"),
          p("主要功能：", { indent: true }),
          bullet("商品列表：表格展示所有商品（名称、价格、库存、供应商、状态）"),
          bullet("添加商品：填写商品名称、价格、库存、关联供应商"),
          bullet("编辑商品：修改商品信息（价格、库存、供应商）"),
          bullet("删除商品：逻辑删除（下架）或物理删除"),
          bullet("搜索筛选：按名称搜索、按供应商筛选"),
          bullet("预置数据：30 个模拟商品"),

          h2("5.4 供应商管理模块"),
          p("管理员维护供应商信息（公司名称、联系人、电话、地址）。"),
          p("主要功能：", { indent: true }),
          bullet("供应商列表：表格展示所有供应商"),
          bullet("添加/编辑/删除供应商信息"),
          bullet("关联关系：每个供应商对应一个用户账号"),
          bullet("预置数据：5 个供应商测试账号"),

          h2("5.5 库存管理模块"),
          p("实时监控所有商品库存，提供库存日志审计功能。"),
          p("主要功能：", { indent: true }),
          bullet("库存总览：所有商品库存表格（名称、当前库存、安全库存、供应商）"),
          bullet("库存操作：入库/出库操作，自动记录变更日志"),
          bullet("库存预警：低于安全库存值时高亮标红"),
          bullet("变更日志：完整的历史变更记录（时间、操作类型、数量、操作人）"),
          bullet("预置数据：36 条模拟库存变更日志"),

          h2("5.6 订单管理模块"),
          p("订单管理的核心模块，支持完整的订单生命周期管理。"),
          p("主要功能：", { indent: true }),
          bullet("订单列表：表格展示所有订单（编号、用户、商品、金额、状态、时间）"),
          bullet("订单详情：弹窗查看订单完整信息（含小计列和供应商列）"),
          bullet("状态流转：支持待处理→已付款→已发货→已收货→已取消"),
          bullet("角色差异：管理员查看所有订单；用户只能看自己的订单"),
          bullet("评价功能：已收货订单可进行 1-5 星评价"),
          bullet("预置数据：12 个订单（包含全部 5 种状态）"),

          h2("5.7 数据报表模块"),
          p("提供多维度的数据分析，辅助管理决策。"),
          p("主要功能：", { indent: true }),
          bullet("销售额统计：按日/月汇总销售额"),
          bullet("热销排行：商品销量排行 TOP 10"),
          bullet("库存统计：各供应商库存分布"),
          bullet("订单统计：按状态分类统计订单数"),

          h2("5.8 个人信息模块"),
          p("所有角色均可访问的个人信息管理页面。"),
          p("主要功能：", { indent: true }),
          bullet("头像上传/更新：支持图片上传并自动圆形裁剪"),
          bullet("密码修改：验证旧密码后设置新密码"),
          bullet("信息编辑：修改手机号、邮箱等个人信息"),
          bullet("退出登录：清除会话、返回登录界面"),

          // === 第六章 ===
          h1("六、UI 设计与用户体验"),
          ...imagePara(imgBuffer("screen_main_admin.png"), 520, "图 6-1  管理员主界面"),
          emptyLine(),

          h2("6.1 界面布局"),
          ...imagePara(imgBuffer("04_ui_layout.png"), 480, "图 6-2  界面布局示意"),
          emptyLine(),

          p("系统采用经典的「左侧导航 + 右侧内容」双栏布局：", { indent: true }),
          bullet("左侧导航栏：深色背景 (#2c3e50)，180px 宽度。顶部显示用户头像和角色信息，下方列出功能菜单按钮"),
          bullet("右侧内容区：浅灰背景 (#ecf0f1)，使用 QStackedWidget 管理多页面切换"),
          bullet("菜单切换：点击导航按钮自动切换对应页面并刷新数据"),

          h2("6.2 样式主题"),
          p("基于 Microsoft Fluent 2 设计语言的 QSS 样式表（style.qss），主要特点："),
          makeTable(
            ["设计元素", "实现方式"],
            [
              ["配色风格", "浅白亚克力风格 (#FFFFFF + 蓝色 #7BA4D9 强调色)"],
              ["按钮样式", "圆角 6px、hover 变色、disabled 灰色"],
              ["表格样式", "斑马纹 (蓝白交替)、选中行高亮、圆角边框"],
              ["输入框", "浅灰背景、蓝色聚焦边框"],
              ["对话框", "统一宽度 400px、QDialogButtonBox 深色文字"],
              ["字体", "系统默认字体 (macOS: PingFang SC / SF Pro)"],
            ],
            [30, 70],
          ),
          emptyLine(),

          h2("6.3 用户体验优化"),
          bullet("登录页底部显示预置测试账号，方便演示"),
          bullet("表格数据自动刷新，页面切换无卡顿"),
          bullet("确认删除等危险操作均有二次确认弹窗"),
          bullet("订单详情弹窗自适应内容缩放，窗口大小 780×550"),
          bullet("库存预警自动标红，问题一目了然"),

          // === 第七章 ===
          h1("七、测试与部署"),

          h2("7.1 测试环境"),
          makeTable(
            ["环境项", "配置"],
            [
              ["操作系统", "macOS (Darwin)"],
              ["编译器", "Apple Clang (Xcode CLT)"],
              ["MySQL", "8.0 (Homebrew 安装)"],
              ["Qt", "5.15+ (Homebrew 安装)"],
              ["CMake", "3.27+"],
            ],
            [40, 60],
          ),
          emptyLine(),

          h2("7.2 测试用例"),
          makeTable(
            ["测试类型", "测试内容", "结果"],
            [
              ["功能测试", "管理员登录 → 用户管理 → 商品管理 → 订单管理等全流程", "✓ 通过"],
              ["权限测试", "供应商/普通用户只能看到对应菜单", "✓ 通过"],
              ["CRUD 测试", "各模块的增删改查操作", "✓ 通过"],
              ["订单流转测试", "pending → paid → shipped → received 完整流转", "✓ 通过"],
              ["库存预警测试", "低库存商品自动标红", "✓ 通过"],
              ["UI 样式测试", "QSS 主题渲染正确、macOS 兼容性", "✓ 通过"],
              ["数据库测试", "SQL 注入尝试、字符编码正确", "✓ 通过"],
            ],
            [20, 55, 25],
          ),
          emptyLine(),

          h2("7.3 编译与运行"),
          p("编译命令：", { indent: true }),
          new Paragraph({
            children: [new TextRun({
              text: "  cd build && cmake .. && make -j4",
              size: 19,
              font: "Courier New",
              shading: { fill: "f0f0f0" },
            })],
            spacing: { after: 100 },
          }),
          p("数据库初始化：", { indent: true }),
          new Paragraph({
            children: [new TextRun({
              text: "  mysql -u root -p < sql/schema.sql",
              size: 19,
              font: "Courier New",
              shading: { fill: "f0f0f0" },
            })],
            spacing: { after: 100 },
          }),
          p("导入测试数据：", { indent: true }),
          new Paragraph({
            children: [new TextRun({
              text: "  mysql -u root -p --default-character-set=utf8mb4 < sql/mock_data.sql",
              size: 19,
              font: "Courier New",
              shading: { fill: "f0f0f0" },
            })],
            spacing: { after: 200 },
          }),

          // === 第八章 ===
          h1("八、项目总结"),

          h2("8.1 项目成果"),
          p("本项目成功构建了一个功能完整的电商后台管理系统，实现了以下目标：", { indent: true }),
          numbered(1, "完整的用户-商品-订单-库存管理闭环"),
          numbered(2, "三级角色权限控制，数据隔离安全可靠"),
          numbered(3, "美观的 Fluent2 风格 UI 界面"),
          numbered(4, "实时的库存监控和智能预警"),
          numbered(5, "完整的数据报表和统计分析"),
          numbered(6, "代码结构清晰，模块化设计便于扩展"),

          h2("8.2 技术亮点"),
          bullet("Qt5 原生开发，编译为原生二进制，性能优于 Electron/Web 方案"),
          bullet("QSS 全局样式表统一管理外观，修改样式无需改动 C++ 代码"),
          bullet("DBManager 单例模式封装数据库操作，连接池自动管理"),
          bullet("QStackedWidget + QButtonGroup 实现丝滑的页面切换"),
          bullet("完整的库存变更日志审计追踪"),
          bullet("信号槽机制解耦组件通信"),

          h2("8.3 不足与改进方向"),
          bullet("密码存储目前未加密，建议引入 bcrypt/scrypt 哈希"),
          bullet("未实现分页功能，大数据量下表格性能需优化"),
          bullet("可增加图表可视化（QChart 或 WebEngine 嵌入 ECharts）"),
          bullet("可引入 ORM 框架简化数据库操作"),
          bullet("可增加导出 Excel/PDF 等数据导出功能"),
        ],
      },
    ],
  });

  const buffer = await Packer.toBuffer(doc);
  fs.writeFileSync("/Users/zwx/CodeBuddy/20260707184422/电商管理系统_实验报告.docx", buffer);
  console.log("✅ 电商管理系统_实验报告.docx 生成成功");
}

// ============================================================
// 报告2：华中师范大学实训报告.docx（学校格式实训报告）
// ============================================================
async function generateReport2() {
  const doc = new Document({
    sections: [
      // ---- 封面 ----
      {
        properties: {
          page: { margin: { top: MARGIN, bottom: MARGIN, left: MARGIN, right: MARGIN } },
        },
        children: [
          emptyLine(),emptyLine(),emptyLine(),
          new Paragraph({
            children: [new TextRun({ text: "华中师范大学", size: 44, bold: true, font: "SimHei", color: "1a5276" })],
            alignment: AlignmentType.CENTER,
            spacing: { after: 200 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "实 训 报 告", size: 48, bold: true, font: "SimHei", color: "1a5276" })],
            alignment: AlignmentType.CENTER,
            spacing: { after: 500 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "课程名称：软件工程实训", size: 24, font: "SimSun" })],
            alignment: AlignmentType.CENTER, spacing: { after: 150 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "项目名称：电商管理系统", size: 24, font: "SimSun" })],
            alignment: AlignmentType.CENTER, spacing: { after: 150 },
          }),
          new Paragraph({
            children: [new TextRun({ text: "技术栈：C++ / Qt5 / MySQL", size: 24, font: "SimSun" })],
            alignment: AlignmentType.CENTER, spacing: { after: 150 },
          }),
        ],
      },

      // ---- 正文 ----
      {
        properties: {
          page: {
            margin: { top: MARGIN, bottom: MARGIN, left: MARGIN, right: MARGIN },
            pageNumbers: { start: 1 },
          },
        },
        footers: {
          default: new Footer({
            children: [
              new Paragraph({
                children: [
                  new TextRun({ children: [PageNumber.CURRENT], size: 18, font: "SimSun" }),
                ],
                alignment: AlignmentType.CENTER,
              }),
            ],
          }),
        },
        children: [
          h1("一、实训目的与要求"),

          h2("1.1 实训目的"),
          p("本次软件工程实训旨在通过实际项目开发，使学生掌握以下能力：", { indent: true }),
          numbered(1, "掌握 C++ 面向对象编程的工程实践方法"),
          numbered(2, "熟练使用 Qt5 框架构建桌面 GUI 应用程序"),
          numbered(3, "掌握 MySQL 数据库的设计与 SQL 编程"),
          numbered(4, "理解 MVC 架构模式和单例设计模式的实际应用"),
          numbered(5, "培养团队协作、项目管理和文档撰写能力"),

          h2("1.2 实训要求"),
          bullet("开发一个功能完整的电商后台管理系统"),
          bullet("支持至少 3 种不同角色的用户权限控制"),
          bullet("实现用户、商品、订单、库存等核心业务模块"),
          bullet("提供良好的人机交互界面，符合现代 UI 设计规范"),
          bullet("编写规范的实验报告，附系统截图和代码说明"),

          // === 第二章 ===
          h1("二、开发环境与工具"),

          makeTable(
            ["类别", "工具/环境", "版本"],
            [
              ["操作系统", "macOS", "—"],
              ["编译器", "Apple Clang (Xcode CLT)", "15+"],
              ["开发语言", "C++", "C++17"],
              ["GUI 框架", "Qt5", "5.15+"],
              ["数据库", "MySQL (Homebrew)", "8.0"],
              ["构建工具", "CMake", "3.27+"],
              ["编辑/IDE", "VS Code / CLion", "—"],
              ["版本控制", "Git", "—"],
            ],
            [25, 40, 35],
          ),
          emptyLine(),

          // === 第三章 ===
          h1("三、系统功能设计"),

          h2("3.1 系统总体结构"),
          ...imagePara(imgBuffer("01_system_architecture.png"), 500, "图 3-1  系统架构"),
          emptyLine(),

          p("系统采用三层 C/S 架构，前端桌面客户端使用 Qt5 Widgets 构建，"
            + "后端数据库使用 MySQL 8.0，中间通过 Qt SQL 模块进行数据交互。"
            + "系统按角色加载不同功能模块：", { indent: true }),

          h3("管理员（系统最高权限）"),
          bullet("用户管理：查看/编辑/禁用用户"),
          bullet("商品管理：添加/编辑/下架商品"),
          bullet("供应商管理：管理供应商档案"),
          bullet("库存管理：实时监控库存、入库/出库操作"),
          bullet("订单管理：查看/处理所有订单"),
          bullet("数据报表：销售额、库存、订单统计"),

          h3("供应商"),
          bullet("我的商品：管理自己的商品"),
          bullet("我的订单：处理分配给自己的订单"),
          bullet("数据分析：查看销售和库存数据"),
          bullet("个人信息：修改密码、上传头像"),

          h3("普通用户"),
          bullet("订单管理：下单、查看订单、评价"),
          bullet("个人信息：管理个人资料"),

          h2("3.2 数据库设计"),
          ...imagePara(imgBuffer("02_database_er.png"), 500, "图 3-2  数据库 ER 图"),
          emptyLine(),

          p("系统设计了 8 张核心数据表，覆盖用户、商品、订单、库存等全部业务场景：", { indent: true }),
          makeTable(
            ["表名", "说明", "主要字段", "数据量（测试）"],
            [
              ["users", "用户表", "id, username, password, role, real_name", "14 条"],
              ["categories", "商品分类表", "id, name, description", "5 条"],
              ["suppliers", "供应商表", "id, name, contact, phone, user_id", "5 条"],
              ["products", "商品表", "id, name, price, stock, supplier_id, category_id", "30 条"],
              ["orders", "订单表", "id, order_no, user_id, total_amount, status", "12 条"],
              ["order_items", "订单明细表", "id, order_id, product_id, quantity, price", "12+ 条"],
              ["product_reviews", "商品评价表", "id, order_id, product_id, rating, content", "6 条"],
              ["inventory_log", "库存日志表", "id, product_id, type, quantity, before/after_stock", "36 条"],
            ],
            [22, 20, 38, 20],
          ),
          emptyLine(),
          p("各表详细结构：", { indent: true }),
          emptyLine(),

          h3("① 用户表（users）"),
          p("存储所有系统用户信息，支持三种角色。密码采用 SHA256 哈希存储。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "用户唯一标识"],
              ["username", "VARCHAR(50)", "UNIQUE NOT NULL", "登录用户名"],
              ["password", "VARCHAR(64)", "NOT NULL", "SHA256 哈希密码"],
              ["real_name", "VARCHAR(50)", "NOT NULL", "真实姓名"],
              ["phone", "VARCHAR(20)", "DEFAULT ''", "手机号"],
              ["email", "VARCHAR(100)", "DEFAULT ''", "电子邮箱"],
              ["address", "VARCHAR(255)", "DEFAULT ''", "地址"],
              ["role", "TINYINT", "NOT NULL DEFAULT 0", "0=普通用户, 1=管理员, 2=供应商"],
              ["avatar", "MEDIUMTEXT", "DEFAULT ''", "头像 Base64 编码"],
              ["balance", "DECIMAL(12,2)", "NOT NULL DEFAULT 0", "账户余额"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "注册时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("② 商品分类表（categories）"),
          p("商品分类目录，用于对商品分组管理和筛选。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "分类 ID"],
              ["name", "VARCHAR(50)", "UNIQUE NOT NULL", "分类名称"],
              ["description", "VARCHAR(255)", "DEFAULT ''", "分类描述"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("③ 供应商表（suppliers）"),
          p("管理供应商基本信息，通过 user_id 绑定供应商登录账号。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "供应商 ID"],
              ["name", "VARCHAR(100)", "NOT NULL", "公司名称"],
              ["contact", "VARCHAR(50)", "DEFAULT ''", "联系人"],
              ["phone", "VARCHAR(20)", "DEFAULT ''", "联系电话"],
              ["email", "VARCHAR(100)", "DEFAULT ''", "电子邮箱"],
              ["address", "VARCHAR(255)", "DEFAULT ''", "公司地址"],
              ["user_id", "INT", "FK → users.id", "关联供应商登录账号"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "创建时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("④ 商品表（products）"),
          p("核心业务表，记录商品完整信息，关联分类和供应商。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "商品 ID"],
              ["name", "VARCHAR(100)", "NOT NULL", "商品名称"],
              ["category_id", "INT", "FK NOT NULL", "所属分类"],
              ["supplier_id", "INT", "FK", "所属供应商（可空）"],
              ["price", "DECIMAL(12,2)", "NOT NULL", "售价"],
              ["cost", "DECIMAL(12,2)", "NOT NULL DEFAULT 0", "成本价"],
              ["stock", "INT", "NOT NULL DEFAULT 0", "库存数量"],
              ["alert_stock", "INT", "NOT NULL DEFAULT 10", "库存预警阈值"],
              ["description", "TEXT", "—", "商品描述"],
              ["image_url", "VARCHAR(255)", "DEFAULT ''", "图片 URL"],
              ["status", "TINYINT", "NOT NULL DEFAULT 1", "1=上架, 0=下架"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "创建时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("⑤ 订单表（orders）"),
          p("订单主表，每条记录对应一笔用户订单，包含 5 种状态。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "订单 ID"],
              ["order_no", "VARCHAR(32)", "UNIQUE NOT NULL", "订单编号"],
              ["user_id", "INT", "FK NOT NULL", "买家 ID"],
              ["total_amount", "DECIMAL(12,2)", "NOT NULL", "总金额"],
              ["status", "TINYINT", "NOT NULL DEFAULT 0", "0=待支付, 1=已支付, 2=已发货, 3=已完成, 4=已取消"],
              ["address", "VARCHAR(255)", "DEFAULT ''", "收货地址"],
              ["remark", "VARCHAR(255)", "DEFAULT ''", "备注"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "下单时间"],
              ["updated_at", "DATETIME", "ON UPDATE NOW()", "状态变更时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("⑥ 订单明细表（order_items）"),
          p("订单-商品多对多关联表，记录每个订单中每种商品的数量和价格。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "明细 ID"],
              ["order_id", "INT", "FK CASCADE DELETE", "所属订单"],
              ["product_id", "INT", "FK NOT NULL", "商品 ID"],
              ["quantity", "INT", "NOT NULL", "购买数量"],
              ["price", "DECIMAL(12,2)", "NOT NULL", "下单时单价"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("⑦ 商品评价表（product_reviews）"),
          p("用户对已收货商品的评价，同订单同商品只允许评价一次。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "评价 ID"],
              ["order_id", "INT", "FK CASCADE DELETE", "关联订单"],
              ["product_id", "INT", "FK NOT NULL", "关联商品"],
              ["user_id", "INT", "FK NOT NULL", "评价用户"],
              ["rating", "TINYINT", "NOT NULL", "评分 1-5 分"],
              ["content", "TEXT", "—", "评价内容"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "评价时间"],
            ],
            [18, 22, 22, 38],
          ),
          emptyLine(),

          h3("⑧ 库存变动日志表（inventory_log）"),
          p("完整记录每次库存变更，支持审计追踪和库存盘点。", { indent: true }),
          makeTable(
            ["字段", "类型", "约束", "说明"],
            [
              ["id", "INT", "PK AUTO_INCREMENT", "日志 ID"],
              ["product_id", "INT", "FK NOT NULL", "关联商品"],
              ["type", "TINYINT", "NOT NULL", "0=入库, 1=出库"],
              ["quantity", "INT", "NOT NULL", "变更数量"],
              ["before_stock", "INT", "NOT NULL", "变更前库存"],
              ["after_stock", "INT", "NOT NULL", "变更后库存"],
              ["remark", "VARCHAR(255)", "DEFAULT ''", "变更说明"],
              ["operator_id", "INT", "—", "操作人 ID"],
              ["created_at", "DATETIME", "DEFAULT NOW()", "操作时间"],
            ],
            [18, 22, 22, 38],
          ),

          h2("3.3 订单状态流转"),
          ...imagePara(imgBuffer("03_order_status_flow.png"), 480, "图 3-3  订单状态流转"),
          emptyLine(),
          p("订单有 5 种状态：待处理 → 已付款 → 已发货 → 已收货 / 已取消。"
            + "状态变更由对应角色操作触发，数据库实时更新。"),

          h2("3.4 界面设计"),
          ...imagePara(imgBuffer("screen_login.png"), 340, "图 3-4  登录界面"),
          emptyLine(),

          ...imagePara(imgBuffer("screen_main_admin.png"), 500, "图 3-5  管理员主界面（用户管理页）"),
          emptyLine(),

          p("界面设计特点：", { indent: true }),
          bullet("左侧深色导航栏（180px）+ 右侧浅色内容区布局"),
          bullet("基于 Microsoft Fluent 2 风格的 QSS 主题"),
          bullet("统一圆角、阴影、色彩搭配"),
          bullet("表格斑马纹、hover 高亮、选中变色"),

          // === 第四章 ===
          h1("四、关键代码实现"),

          h2("4.1 数据库连接（单例模式）"),
          p("DBManager 采用单例模式，确保整个应用程序共享一个数据库连接，"
            + "避免连接浪费。核心实现如下：", { indent: true }),

          new Paragraph({
            children: [new TextRun({
              text: "class DBManager {\n"
                + "public:\n"
                + "    static DBManager& instance() {\n"
                + "        static DBManager inst;\n"
                + "        return inst;\n"
                + "    }\n"
                + "    QSqlQueryResult query(const std::string& sql);\n"
                + "private:\n"
                + "    DBManager();           // 私有构造\n"
                + "    QSqlDatabase db_;\n"
                + "};",
              size: 18,
              font: "Courier New",
              shading: { fill: "f5f5f5" },
            })],
            spacing: { after: 200 },
          }),

          h2("4.2 主窗口页面路由"),
          p("根据用户角色加载不同页面集，使用 QStackedWidget 管理页面切换：", { indent: true }),

          new Paragraph({
            children: [new TextRun({
              text: "if (role_ == 1) {        // 管理员\n"
                + "    pageStack_->addWidget(new UserPage());\n"
                + "    pageStack_->addWidget(new ProductPage());\n"
                + "    pageStack_->addWidget(new SupplierPage());\n"
                + "    // ... 共 7 个页面\n"
                + "} else if (role_ == 2) {  // 供应商\n"
                + "    pageStack_->addWidget(new SupplierProductPage());\n"
                + "    // ... 共 4 个页面\n"
                + "} else {                   // 普通用户\n"
                + "    pageStack_->addWidget(new OrderPage());\n"
                + "    // ... 共 2 个页面\n"
                + "}",
              size: 18,
              font: "Courier New",
              shading: { fill: "f5f5f5" },
            })],
            spacing: { after: 200 },
          }),

          h2("4.3 信号槽通信"),
          p("使用 Qt 信号槽机制实现组件间解耦通信：", { indent: true }),
          bullet("导航按钮点击 → QButtonGroup::idClicked → MainWindow::switchPage"),
          bullet("个人信息修改成功 → ProfilePage::profileUpdated → MainWindow::refreshAvatar"),
          bullet("退出登录 → ProfilePage::logoutRequested → 关闭窗口、返回登录"),

          // === 第五章 ===
          h1("五、测试与运行"),

          h2("5.1 功能测试"),
          makeTable(
            ["测试模块", "测试内容", "预期结果", "测试结果"],
            [
              ["登录", "管理员账号登录", "进入管理员主界面，显示全部菜单", "✓"],
              ["用户管理", "禁用/启用用户", "用户状态实时更新", "✓"],
              ["商品管理", "添加新商品", "商品列表刷新显示新商品", "✓"],
              ["库存管理", "入库操作", "库存增加、日志记录", "✓"],
              ["订单管理", "完整订单流转", "状态依次变更", "✓"],
              ["权限控制", "普通用户登录", "仅显示订单管理和个人信息", "✓"],
              ["UI 样式", "检查各页面样式", "QSS 主题正确渲染", "✓"],
            ],
            [20, 25, 30, 25],
          ),
          emptyLine(),

          h2("5.2 运行方法"),
          p("Step 1：编译项目", { indent: true }),
          new Paragraph({
            children: [new TextRun({
              text: "  cd build && cmake .. && make -j4",
              size: 19, font: "Courier New", shading: { fill: "f0f0f0" },
            })],
            spacing: { after: 100 },
          }),
          p("Step 2：初始化数据库", { indent: true }),
          new Paragraph({
            children: [new TextRun({
              text: "  mysql -u root -p < ../sql/schema.sql",
              size: 19, font: "Courier New", shading: { fill: "f0f0f0" },
            })],
            spacing: { after: 100 },
          }),
          p("Step 3：导入测试数据", { indent: true }),
          new Paragraph({
            children: [new TextRun({
              text: "  mysql -u root -p --default-character-set=utf8mb4 < ../sql/mock_data.sql",
              size: 19, font: "Courier New", shading: { fill: "f0f0f0" },
            })],
            spacing: { after: 100 },
          }),
          p("Step 4：运行程序", { indent: true }),
          new Paragraph({
            children: [new TextRun({
              text: "  ./ECommerce",
              size: 19, font: "Courier New", shading: { fill: "f0f0f0" },
            })],
            spacing: { after: 200 },
          }),

          p("预置测试账号：", { indent: true }),
          makeTable(
            ["账号", "密码", "角色", "说明"],
            [
              ["admin", "admin123", "管理员", "最高权限，可访问全部功能"],
              ["supplier1", "supp123", "供应商", "可管理自己商品和订单"],
              ["supplier2", "supp123", "供应商", "可管理自己商品和订单"],
              ["user1", "user123", "普通用户", "可下单和查看自己订单"],
              ["user2", "user123", "普通用户", "可下单和查看自己订单"],
            ],
            [25, 20, 25, 30],
          ),
          emptyLine(),

          // === 第六章 ===
          h1("六、实训总结"),

          h2("6.1 技术收获"),
          numbered(1, "深入掌握了 Qt5 Widgets 桌面应用开发，包括布局管理、信号槽、样式表等核心技术"),
          numbered(2, "实践了单例模式在数据库连接管理中的应用，理解了设计模式的价值"),
          numbered(3, "学会了 MySQL 数据库的规范化设计，掌握了多表关联查询和索引优化"),
          numbered(4, "理解了 C/S 架构中分层设计的重要性，提升了代码的模块化和可维护性"),
          numbered(5, "体验了从需求分析到交付的完整软件开发流程"),

          h2("6.2 遇到的问题与解决"),
          makeTable(
            ["问题", "原因", "解决方案"],
            [
              ["QSS 白色文字看不清", "macOS 上 :default 按钮背景不生效", "将文字颜色改为深色 #2D3436"],
              ["订单详情弹窗太小", "初始窗口尺寸 400×300", "扩大至 780×550 并设自适应缩放"],
              ["页面切换数据不刷新", "未调用 refresh 方法", "在 switchPage 中自动调用对应页面的 refresh"],
              ["中文显示乱码", "终端编码不匹配", "导入数据时指定 --default-character-set=utf8mb4"],
            ],
            [25, 35, 40],
          ),
          emptyLine(),

          h2("6.3 未来改进方向"),
          bullet("引入 bcrypt 密码加密，增强安全性"),
          bullet("增加图表可视化（销售额曲线、饼图等）"),
          bullet("实现数据分页加载，优化大数据量性能"),
          bullet("添加 Excel/PDF 导出功能"),
          bullet("考虑升级到 Qt6 或迁移到 Web 架构"),
        ],
      },
    ],
  });

  const buffer = await Packer.toBuffer(doc);
  fs.writeFileSync("/Users/zwx/CodeBuddy/20260707184422/华中师范大学实训报告.docx", buffer);
  console.log("✅ 华中师范大学实训报告.docx 生成成功");
}

// ============================================================
// Main
// ============================================================
(async () => {
  await generateReport1();
  await generateReport2();
  console.log("🎉 两份报告全部生成完毕！");
})();
