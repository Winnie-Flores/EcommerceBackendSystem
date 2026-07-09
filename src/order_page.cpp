#include "order_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QDateTime>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <sstream>
#include <iomanip>
#include <set>

OrderPage::OrderPage(int currentUserId, int currentRole, QWidget* parent)
    : QWidget(parent), currentUserId_(currentUserId), currentRole_(currentRole) {
    setupUI();
    loadProducts();
    refreshOrderTable();
}

void OrderPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("订单管理");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 搜索栏
    auto* searchLayout = new QHBoxLayout();
    orderSearchEdit_ = new QLineEdit();
    orderSearchEdit_->setPlaceholderText("搜索订单号...");
    orderSearchEdit_->setMinimumHeight(30);

    statusFilter_ = new QComboBox();
    statusFilter_->addItem("全部状态", -1);
    statusFilter_->addItem("待支付", 0);
    statusFilter_->addItem("已支付", 1);
    statusFilter_->addItem("已发货", 2);
    statusFilter_->addItem("已完成", 3);
    statusFilter_->addItem("已取消", 4);
    statusFilter_->setMinimumHeight(30);

    auto* searchBtn = new QPushButton("搜索");
    searchBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 6px 16px; border-radius: 3px; } QPushButton:hover { background: #2980b9; }");

    updateStatusBtn_ = new QPushButton("更新状态");
    updateStatusBtn_->setStyleSheet("QPushButton { background: #f39c12; color: white; padding: 6px 16px; border-radius: 3px; font-weight: bold; } QPushButton:hover { background: #e67e22; }");

    payOrderBtn_ = new QPushButton("💳 模拟支付");
    payOrderBtn_->setStyleSheet("QPushButton { background: #27ae60; color: white; padding: 6px 16px; border-radius: 3px; font-weight: bold; } QPushButton:hover { background: #219a52; }");

    reviewBtn_ = new QPushButton("⭐ 评价");
    reviewBtn_->setStyleSheet("QPushButton { background: #e67e22; color: white; padding: 6px 16px; border-radius: 3px; font-weight: bold; } "
                               "QPushButton:hover { background: #d35400; } "
                               "QPushButton:disabled { background: #bdc3c7; color: #7f8c8d; }");
    reviewBtn_->setEnabled(false);

    // 管理员专属按钮
    QString adminBtnStyle = "QPushButton { background: #8e44ad; color: white; padding: 6px 16px; border-radius: 3px; font-weight: bold; } "
                            "QPushButton:hover { background: #7d3c98; } "
                            "QPushButton:disabled { background: #bdc3c7; color: #7f8c8d; }";

    detailBtn_ = new QPushButton("📋 查看详情");
    detailBtn_->setStyleSheet(adminBtnStyle);
    detailBtn_->setEnabled(false);

    editBtn_ = new QPushButton("✏️ 编辑");
    editBtn_->setStyleSheet(adminBtnStyle.replace("#8e44ad", "#2980b9").replace("#7d3c98", "#1f6ea0"));
    editBtn_->setEnabled(false);

    deleteBtn_ = new QPushButton("🗑 删除");
    deleteBtn_->setStyleSheet("QPushButton { background: #e74c3c; color: white; padding: 6px 16px; border-radius: 3px; font-weight: bold; } "
                               "QPushButton:hover { background: #c0392b; } "
                               "QPushButton:disabled { background: #bdc3c7; color: #7f8c8d; }");
    deleteBtn_->setEnabled(false);

    searchLayout->addWidget(orderSearchEdit_);
    searchLayout->addWidget(statusFilter_);
    searchLayout->addWidget(searchBtn);
    searchLayout->addWidget(payOrderBtn_);
    searchLayout->addWidget(reviewBtn_);
    if (currentRole_ == 1) {
        searchLayout->addWidget(detailBtn_);
        searchLayout->addWidget(editBtn_);
        searchLayout->addWidget(deleteBtn_);
        searchLayout->addWidget(updateStatusBtn_);
    }
    mainLayout->addLayout(searchLayout);

    // 订单表格
    orderTable_ = new QTableWidget();
    orderTable_->setColumnCount(7);
    orderTable_->setHorizontalHeaderLabels({"ID", "订单号", "用户", "金额", "状态", "地址", "时间"});
    orderTable_->horizontalHeader()->setStretchLastSection(true);
    orderTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    orderTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    orderTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderTable_->setAlternatingRowColors(true);
    orderTable_->setStyleSheet("QTableWidget { background: white; gridline-color: #ddd; } "
                               "QTableWidget::item:selected { background: #3498db; color: white; }");
    mainLayout->addWidget(orderTable_);

    // 新建订单区
    auto* createGroup = new QGroupBox("创建新订单");
    createGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 10px; padding-top: 15px; }");
    auto* createLayout = new QVBoxLayout(createGroup);

    // 商品选择行
    auto* addItemLayout = new QHBoxLayout();
    productCombo_ = new QComboBox(); productCombo_->setMinimumHeight(30);
    qtySpin_ = new QSpinBox(); qtySpin_->setRange(1, 999); qtySpin_->setValue(1); qtySpin_->setMinimumHeight(30);
    addItemBtn_ = new QPushButton("添加到购物车");
    addItemBtn_->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 8px 16px; border-radius: 3px; } QPushButton:hover { background: #2980b9; }");
    removeItemBtn_ = new QPushButton("移除选中");
    removeItemBtn_->setStyleSheet("QPushButton { background: #e74c3c; color: white; padding: 8px 16px; border-radius: 3px; } QPushButton:hover { background: #c0392b; }");

    addItemLayout->addWidget(new QLabel("商品:"));
    addItemLayout->addWidget(productCombo_);
    addItemLayout->addWidget(new QLabel("数量:"));
    addItemLayout->addWidget(qtySpin_);
    addItemLayout->addWidget(addItemBtn_);
    addItemLayout->addWidget(removeItemBtn_);
    createLayout->addLayout(addItemLayout);

    // 购物车表格
    cartTable_ = new QTableWidget();
    cartTable_->setColumnCount(4);
    cartTable_->setHorizontalHeaderLabels({"商品ID", "商品名称", "数量", "单价"});
    cartTable_->horizontalHeader()->setStretchLastSection(true);
    cartTable_->setMaximumHeight(150);
    cartTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable_->setStyleSheet("QTableWidget { background: white; gridline-color: #ddd; }");
    createLayout->addWidget(cartTable_);

    // 订单信息
    auto* orderInfoLayout = new QHBoxLayout();
    orderInfoLayout->addWidget(new QLabel("收货地址:"));
    orderAddressEdit_ = new QLineEdit(); orderAddressEdit_->setMinimumHeight(28);
    orderInfoLayout->addWidget(orderAddressEdit_);
    orderInfoLayout->addWidget(new QLabel("备注:"));
    orderRemarkEdit_ = new QTextEdit(); orderRemarkEdit_->setMaximumHeight(40);
    orderInfoLayout->addWidget(orderRemarkEdit_);
    orderInfoLayout->addWidget(new QLabel("总金额:"));
    totalAmount_ = new QDoubleSpinBox(); totalAmount_->setReadOnly(true);
    totalAmount_->setDecimals(2); totalAmount_->setRange(0, 99999999); totalAmount_->setMinimumHeight(28);
    totalAmount_->setStyleSheet("font-weight: bold; font-size: 14px; color: #e74c3c;");
    orderInfoLayout->addWidget(totalAmount_);

    createOrderBtn_ = new QPushButton("提交订单");
    createOrderBtn_->setStyleSheet("QPushButton { background: #2ecc71; color: white; padding: 10px 30px; border-radius: 3px; font-size: 15px; font-weight: bold; } QPushButton:hover { background: #27ae60; }");
    orderInfoLayout->addWidget(createOrderBtn_);

    createLayout->addLayout(orderInfoLayout);
    mainLayout->addWidget(createGroup);

    connect(searchBtn, &QPushButton::clicked, this, &OrderPage::onSearchOrder);
    connect(updateStatusBtn_, &QPushButton::clicked, this, &OrderPage::onUpdateStatus);
    connect(payOrderBtn_, &QPushButton::clicked, this, &OrderPage::onPayOrder);
    connect(reviewBtn_, &QPushButton::clicked, this, &OrderPage::onReview);
    connect(detailBtn_, &QPushButton::clicked, this, &OrderPage::onViewDetail);
    connect(editBtn_, &QPushButton::clicked, this, &OrderPage::onEditOrder);
    connect(deleteBtn_, &QPushButton::clicked, this, &OrderPage::onDeleteOrder);
    connect(addItemBtn_, &QPushButton::clicked, this, &OrderPage::onAddItem);
    connect(removeItemBtn_, &QPushButton::clicked, this, &OrderPage::onRemoveItem);
    connect(createOrderBtn_, &QPushButton::clicked, this, &OrderPage::onCreateOrder);
    connect(orderTable_, &QTableWidget::itemSelectionChanged, this, &OrderPage::onOrderSelect);
}

void OrderPage::loadProducts() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT id, name, price, stock FROM products WHERE status=1 ORDER BY id");
    productCombo_->clear();
    for (auto& row : result) {
        productCombo_->addItem(
            rowStr(row, "name") + " (¥" + QString::number(rowDouble(row, "price"), 'f', 2)
            + " 库存:" + QString::number(rowInt(row, "stock")) + ")",
            rowInt(row, "id"));
    }
}

static QString statusText(int status) {
    switch (status) {
        case 0: return "待支付";
        case 1: return "已支付";
        case 2: return "已发货";
        case 3: return "已完成";
        case 4: return "已取消";
        default: return "未知";
    }
}

static QColor statusColor(int status) {
    switch (status) {
        case 0: return QColor("#f39c12");
        case 1: return QColor("#3498db");
        case 2: return QColor("#9b59b6");
        case 3: return QColor("#27ae60");
        case 4: return QColor("#95a5a6");
        default: return QColor("#000000");
    }
}

void OrderPage::refreshOrderTable() {
    auto& db = DBManager::instance();
    std::string sql = "SELECT o.id, o.order_no, u.username, o.total_amount, o.status, o.address, "
                      "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i') as created_at "
                      "FROM orders o LEFT JOIN users u ON o.user_id=u.id ";

    if (currentRole_ == 0)  // 普通用户只看自己的
        sql += "WHERE o.user_id=" + std::to_string(currentUserId_) + " ";
    sql += "ORDER BY o.id DESC";

    auto result = db.query(sql);

    orderTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        orderTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        orderTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "order_no")));
        orderTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "username")));
        orderTable_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(rowDouble(result[i], "total_amount"), 'f', 2)));

        int st = rowInt(result[i], "status");
        auto* stItem = new QTableWidgetItem(statusText(st));
        stItem->setForeground(statusColor(st));
        stItem->setFont(QFont(stItem->font().family(), -1, QFont::Bold));
        orderTable_->setItem(static_cast<int>(i), 4, stItem);

        orderTable_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "address")));
        orderTable_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void OrderPage::onSearchOrder() {
    QString kw = orderSearchEdit_->text().trimmed();
    int filterStatus = statusFilter_->currentData().toInt();
    auto& db = DBManager::instance();

    std::string sql = "SELECT o.id, o.order_no, u.username, o.total_amount, o.status, o.address, "
                      "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i') as created_at "
                      "FROM orders o LEFT JOIN users u ON o.user_id=u.id WHERE 1=1";

    if (currentRole_ == 0)
        sql += " AND o.user_id=" + std::to_string(currentUserId_);

    if (!kw.isEmpty())
        sql += " AND o.order_no LIKE '%" + db.escape(kw.toStdString()) + "%'";
    if (filterStatus >= 0)
        sql += " AND o.status=" + std::to_string(filterStatus);

    sql += " ORDER BY o.id DESC";

    auto result = db.query(sql);
    orderTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        orderTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        orderTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "order_no")));
        orderTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "username")));
        orderTable_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(rowDouble(result[i], "total_amount"), 'f', 2)));
        int st = rowInt(result[i], "status");
        auto* stItem = new QTableWidgetItem(statusText(st));
        stItem->setForeground(statusColor(st));
        stItem->setFont(QFont(stItem->font().family(), -1, QFont::Bold));
        orderTable_->setItem(static_cast<int>(i), 4, stItem);
        orderTable_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "address")));
        orderTable_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void OrderPage::onOrderSelect() {
    auto items = orderTable_->selectedItems();
    if (items.isEmpty()) {
        selectedOrderId_ = 0;
        reviewBtn_->setEnabled(false);
        if (currentRole_ == 1) {
            detailBtn_->setEnabled(false);
            editBtn_->setEnabled(false);
            deleteBtn_->setEnabled(false);
        }
        return;
    }
    selectedOrderId_ = orderTable_->item(items.first()->row(), 0)->text().toInt();

    // 只有"已完成"状态才能评价
    QString st = orderTable_->item(items.first()->row(), 4)->text();
    reviewBtn_->setEnabled(st == "已完成");

    // 管理员按钮：选中订单即可用
    if (currentRole_ == 1) {
        detailBtn_->setEnabled(true);
        editBtn_->setEnabled(true);
        deleteBtn_->setEnabled(true);
    }
}

void OrderPage::onUpdateStatus() {
    if (selectedOrderId_ == 0 || currentRole_ != 1) {
        QMessageBox::warning(this, "提示", "请先选择订单");
        return;
    }

    auto& db = DBManager::instance();
    auto result = db.query("SELECT status FROM orders WHERE id=" + std::to_string(selectedOrderId_));
    if (result.empty()) return;

    int curStatus = rowInt(result[0], "status");
    if (curStatus == 3 || curStatus == 4) {
        QMessageBox::information(this, "提示", "该订单已是最终状态，无需更新");
        return;
    }

    QStringList options;
    if (curStatus == 0) options << "标记为已支付";
    if (curStatus == 1) options << "标记为已发货";
    if (curStatus == 2) options << "标记为已完成";
    options << "取消订单";

    bool ok;
    QString choice = QInputDialog::getItem(this, "更新订单状态", "选择新状态:", options, 0, false, &ok);
    if (!ok) return;

    int newStatus = curStatus;
    if (choice == "标记为已支付") newStatus = 1;
    else if (choice == "标记为已发货") newStatus = 2;
    else if (choice == "标记为已完成") newStatus = 3;
    else if (choice == "取消订单") newStatus = 4;

    // 如果取消订单，恢复库存
    if (newStatus == 4 && curStatus != 4) {
        auto items = db.query("SELECT product_id, quantity FROM order_items WHERE order_id="
                              + std::to_string(selectedOrderId_));
        db.begin();
        for (auto& item : items) {
            db.execute("UPDATE products SET stock=stock+" + std::to_string(rowInt(item, "quantity"))
                       + " WHERE id=" + std::to_string(rowInt(item, "product_id")));
        }
        db.execute("UPDATE orders SET status=4 WHERE id=" + std::to_string(selectedOrderId_));
        db.commit();
    } else {
        db.execute("UPDATE orders SET status=" + std::to_string(newStatus)
                   + " WHERE id=" + std::to_string(selectedOrderId_));
    }

    refreshOrderTable();
    QMessageBox::information(this, "成功", "订单状态已更新");
}

void OrderPage::onAddItem() {
    int productId = productCombo_->currentData().toInt();
    int qty = qtySpin_->value();

    auto& db = DBManager::instance();
    auto result = db.query("SELECT name, price, stock FROM products WHERE id=" + std::to_string(productId));
    if (result.empty()) return;

    QString name = rowStr(result[0], "name");
    double price = rowDouble(result[0], "price");
    int stock = rowInt(result[0], "stock");

    if (qty > stock) {
        QMessageBox::warning(this, "提示", "库存不足！当前库存: " + QString::number(stock));
        return;
    }

    // 检查是否已在购物车
    for (int i = 0; i < cartTable_->rowCount(); ++i) {
        if (cartTable_->item(i, 0)->text().toInt() == productId) {
            int curQty = cartTable_->item(i, 2)->text().toInt();
            cartTable_->item(i, 2)->setText(QString::number(curQty + qty));
            totalAmount_->setValue(totalAmount_->value() + price * qty);
            return;
        }
    }

    int row = cartTable_->rowCount();
    cartTable_->insertRow(row);
    cartTable_->setItem(row, 0, new QTableWidgetItem(QString::number(productId)));
    cartTable_->setItem(row, 1, new QTableWidgetItem(name));
    cartTable_->setItem(row, 2, new QTableWidgetItem(QString::number(qty)));
    cartTable_->setItem(row, 3, new QTableWidgetItem(QString::number(price, 'f', 2)));

    totalAmount_->setValue(totalAmount_->value() + price * qty);
}

void OrderPage::onRemoveItem() {
    auto items = cartTable_->selectedItems();
    if (items.isEmpty()) return;

    int row = items.first()->row();
    double price = cartTable_->item(row, 3)->text().toDouble();
    int qty = cartTable_->item(row, 2)->text().toInt();
    totalAmount_->setValue(totalAmount_->value() - price * qty);

    cartTable_->removeRow(row);
}

void OrderPage::onCreateOrder() {
    if (cartTable_->rowCount() == 0) {
        QMessageBox::warning(this, "提示", "购物车为空，请先添加商品");
        return;
    }

    QString address = orderAddressEdit_->text().trimmed();
    if (address.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入收货地址");
        return;
    }

    // 生成订单号
    auto now = QDateTime::currentDateTime();
    QString orderNo = now.toString("yyyyMMddHHmmss") + QString::number(currentUserId_);

    auto& db = DBManager::instance();
    db.begin();

    // 检查库存
    for (int i = 0; i < cartTable_->rowCount(); ++i) {
        int pid = cartTable_->item(i, 0)->text().toInt();
        int qty = cartTable_->item(i, 2)->text().toInt();
        auto result = db.query("SELECT stock FROM products WHERE id=" + std::to_string(pid));
        if (result.empty() || rowInt(result[0], "stock") < qty) {
            db.rollback();
            QMessageBox::warning(this, "提示", "商品库存不足，请调整数量");
            return;
        }
    }

    // 创建订单
    double total = totalAmount_->value();
    std::string remark = orderRemarkEdit_->toPlainText().toStdString();
    std::string sql = "INSERT INTO orders (order_no, user_id, total_amount, address, remark) VALUES ('"
                      + orderNo.toStdString() + "', " + std::to_string(currentUserId_) + ", "
                      + std::to_string(total) + ", '"
                      + db.escape(address.toStdString()) + "', '"
                      + db.escape(remark) + "')";

    long long orderId = db.insert(sql);
    if (orderId <= 0) {
        db.rollback();
        QMessageBox::critical(this, "错误", "创建订单失败");
        return;
    }

    // 插入订单明细并扣库存（原子操作：UPDATE SET stock = stock - qty WHERE stock >= qty）
    for (int i = 0; i < cartTable_->rowCount(); ++i) {
        int pid = cartTable_->item(i, 0)->text().toInt();
        int qty = cartTable_->item(i, 2)->text().toInt();
        double price = cartTable_->item(i, 3)->text().toDouble();

        db.execute("INSERT INTO order_items (order_id, product_id, quantity, price) VALUES ("
                   + std::to_string(orderId) + ", " + std::to_string(pid) + ", "
                   + std::to_string(qty) + ", " + std::to_string(price) + ")");

        // 原子扣库存（并发安全）
        std::string deductSql = "UPDATE products SET stock = stock - " + std::to_string(qty)
            + " WHERE id = " + std::to_string(pid) + " AND stock >= " + std::to_string(qty);
        int affected = db.execute(deductSql);
        if (affected == 0) {
            db.rollback();
            QMessageBox::critical(this, "错误", "商品库存不足，订单创建失败");
            return;
        }

        // 记录出库日志
        auto stockResult = db.query("SELECT stock FROM products WHERE id=" + std::to_string(pid));
        int curStock = rowInt(stockResult[0], "stock");
        int beforeStock = curStock + qty;
        db.execute("INSERT INTO inventory_log (product_id, type, quantity, before_stock, after_stock, remark) VALUES ("
                   + std::to_string(pid) + ", 1, " + std::to_string(qty) + ", "
                   + std::to_string(beforeStock) + ", " + std::to_string(curStock) + ", '订单出库')");
    }

    db.commit();
    clearCart();
    refreshOrderTable();
    loadProducts();
    QMessageBox::information(this, "成功", "订单创建成功！\n订单号: " + orderNo);
}

void OrderPage::clearCart() {
    cartTable_->setRowCount(0);
    totalAmount_->setValue(0);
    orderAddressEdit_->clear();
    orderRemarkEdit_->clear();
}

void OrderPage::onPayOrder() {
    // 获取当前选中行
    auto items = orderTable_->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在订单列表中选中一个待支付的订单");
        return;
    }

    int row = items.first()->row();
    int orderId = orderTable_->item(row, 0)->text().toInt();
    QString orderNo = orderTable_->item(row, 1)->text();
    QString statusText = orderTable_->item(row, 4)->text();
    double amount = orderTable_->item(row, 3)->text().toDouble();

    if (statusText != "待支付") {
        QMessageBox::information(this, "提示", "只有「待支付」状态的订单才能支付！");
        return;
    }

    // 权限检查：普通用户只能支付自己的订单
    auto& db = DBManager::instance();
    if (currentRole_ == 0) {
        auto check = db.query("SELECT user_id FROM orders WHERE id=" + std::to_string(orderId));
        if (!check.empty() && rowInt(check[0], "user_id") != currentUserId_) {
            QMessageBox::warning(this, "提示", "只能支付自己的订单！");
            return;
        }
    }

    // 模拟支付确认
    QString msg = QString("模拟支付确认\n\n"
                          "订单号：%1\n"
                          "支付金额：¥%2\n\n"
                          "选择支付方式：").arg(orderNo).arg(amount, 0, 'f', 2);

    QStringList methods = {"微信支付", "支付宝", "银行卡", "余额支付"};
    bool ok;
    QString method = QInputDialog::getItem(this, "模拟支付", msg, methods, 0, false, &ok);
    if (!ok || method.isEmpty()) return;

    // 模拟支付处理
    QMessageBox processingBox(this);
    processingBox.setWindowTitle("支付处理中");
    processingBox.setText("正在连接支付网关...\n支付方式：" + method + "\n金额：¥" + QString::number(amount, 'f', 2));
    processingBox.setIcon(QMessageBox::Information);
    processingBox.setStandardButtons(QMessageBox::NoButton);
    processingBox.show();

    // 模拟 1 秒处理延迟
    QTimer::singleShot(800, &processingBox, &QMessageBox::accept);
    processingBox.exec();

    // 更新订单状态为已支付
    db.execute("UPDATE orders SET status=1 WHERE id=" + std::to_string(orderId) + " AND status=0");

    refreshOrderTable();

    QMessageBox::information(this, "支付成功",
                             "✅ 支付成功！\n\n"
                             "订单号：" + orderNo + "\n"
                             "支付方式：" + method + "\n"
                             "支付金额：¥" + QString::number(amount, 'f', 2) + "\n"
                             "支付时间：" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void OrderPage::onReview() {
    if (selectedOrderId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择订单");
        return;
    }

    auto& db = DBManager::instance();

    // 查询订单商品
    auto items = db.query(
        "SELECT oi.product_id, p.name, oi.quantity, oi.price "
        "FROM order_items oi JOIN products p ON oi.product_id=p.id "
        "WHERE oi.order_id=" + std::to_string(selectedOrderId_));
    if (items.empty()) return;

    // 检查哪些已经评价过
    auto reviewed = db.query(
        "SELECT product_id FROM product_reviews WHERE order_id=" + std::to_string(selectedOrderId_));
    std::set<int> reviewedIds;
    for (auto& r : reviewed) reviewedIds.insert(rowInt(r, "product_id"));

    // 构建评价对话框
    QDialog dlg(this);
    dlg.setWindowTitle("商品评价");
    dlg.resize(500, 400);

    auto* dlgLayout = new QVBoxLayout(&dlg);

    auto* title = new QLabel("请对以下商品进行评价 (1-5星)");
    title->setStyleSheet("font-size: 14px; font-weight: bold; margin-bottom: 8px;");
    dlgLayout->addWidget(title);

    auto* scrollArea = new QScrollArea();
    auto* scrollWidget = new QWidget();
    auto* scrollLayout = new QVBoxLayout(scrollWidget);

    // 每个商品一个评价组
    struct ReviewWidget {
        QComboBox* rating;
        QTextEdit* content;
    };
    std::vector<std::pair<int, ReviewWidget>> reviewWidgets;

    for (auto& item : items) {
        int pid = rowInt(item, "product_id");
        QString name = rowStr(item, "name");
        double price = rowDouble(item, "price");

        if (reviewedIds.count(pid)) continue;  // 已评价则跳过

        auto* group = new QGroupBox(name + QString(" (¥%1)").arg(price, 0, 'f', 2));
        group->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 8px; }");
        auto* gLayout = new QVBoxLayout(group);

        auto* ratingCombo = new QComboBox();
        ratingCombo->addItem("★★★★★ 非常好", 5);
        ratingCombo->addItem("★★★★☆ 好", 4);
        ratingCombo->addItem("★★★☆☆ 一般", 3);
        ratingCombo->addItem("★★☆☆☆ 差", 2);
        ratingCombo->addItem("★☆☆☆☆ 很差", 1);
        ratingCombo->setCurrentIndex(0);
        gLayout->addWidget(ratingCombo);

        auto* contentEdit = new QTextEdit();
        contentEdit->setPlaceholderText("说说你的使用感受...（可选）");
        contentEdit->setMaximumHeight(80);
        gLayout->addWidget(contentEdit);

        scrollLayout->addWidget(group);
        reviewWidgets.push_back({pid, {ratingCombo, contentEdit}});
    }

    if (reviewWidgets.empty()) {
        QMessageBox::information(this, "提示", "该订单的所有商品已评价完毕！");
        return;
    }

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    dlgLayout->addWidget(scrollArea);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btnBox->button(QDialogButtonBox::Ok)->setText("提交评价");
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlgLayout->addWidget(btnBox);

    if (dlg.exec() != QDialog::Accepted) return;

    // 保存评价
    for (auto& [pid, rw] : reviewWidgets) {
        int rating = rw.rating->currentData().toInt();
        QString content = rw.content->toPlainText().trimmed();

        std::string sql = "INSERT INTO product_reviews (order_id, product_id, user_id, rating, content) VALUES ("
                          + std::to_string(selectedOrderId_) + ", "
                          + std::to_string(pid) + ", "
                          + std::to_string(currentUserId_) + ", "
                          + std::to_string(rating) + ", '"
                          + db.escape(content.toStdString()) + "')";
        db.execute(sql);
    }

    refreshOrderTable();
    QMessageBox::information(this, "成功", "商品评价提交成功！感谢你的反馈。");
}

// ========== 管理员：查看订单详情 ==========
void OrderPage::onViewDetail() {
    if (selectedOrderId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择订单");
        return;
    }

    auto& db = DBManager::instance();

    // 查询订单基本信息
    auto orderResult = db.query(
        "SELECT o.order_no, u.username, o.total_amount, o.status, o.address, o.remark, "
        "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i:%s') as created_at, "
        "DATE_FORMAT(o.updated_at, '%Y-%m-%d %H:%i:%s') as updated_at "
        "FROM orders o LEFT JOIN users u ON o.user_id=u.id "
        "WHERE o.id=" + std::to_string(selectedOrderId_));
    if (orderResult.empty()) return;

    auto& row = orderResult[0];
    QString orderNo = rowStr(row, "order_no");
    QString username = rowStr(row, "username");
    double total = rowDouble(row, "total_amount");
    int st = rowInt(row, "status");
    QString address = rowStr(row, "address");
    QString remark = rowStr(row, "remark");
    QString createdAt = rowStr(row, "created_at");
    QString updatedAt = rowStr(row, "updated_at");

    // 查询订单商品明细
    auto items = db.query(
        "SELECT oi.product_id, p.name, oi.quantity, oi.price "
        "FROM order_items oi JOIN products p ON oi.product_id=p.id "
        "WHERE oi.order_id=" + std::to_string(selectedOrderId_));

    // 构建详情对话框
    QDialog dlg(this);
    dlg.setWindowTitle("订单详情 - " + orderNo);
    dlg.resize(550, 450);

    auto* dlgLayout = new QVBoxLayout(&dlg);

    // 基本信息
    auto* infoGroup = new QGroupBox("基本信息");
    infoGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; padding-top: 15px; margin-top: 8px; }");
    auto* infoLayout = new QFormLayout(infoGroup);
    infoLayout->setSpacing(6);

    auto addInfo = [&](const QString& label, const QString& value) {
        auto* lbl = new QLabel(value);
        lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
        lbl->setStyleSheet("font-size: 13px;");
        infoLayout->addRow(label + ":", lbl);
    };

    addInfo("订单编号", orderNo);
    addInfo("下单用户", username);
    addInfo("订单金额", QString("¥%1").arg(total, 0, 'f', 2));
    addInfo("订单状态", "<span style='color:" + statusColor(st).name() + "; font-weight:bold;'>"
            + statusText(st) + "</span>");
    addInfo("收货地址", address.isEmpty() ? "（未填写）" : address);
    addInfo("备注", remark.isEmpty() ? "（无）" : remark);
    addInfo("创建时间", createdAt);
    addInfo("更新时间", updatedAt);

    // 状态标签需要富文本
    auto* stLbl = qobject_cast<QLabel*>(infoLayout->itemAt(infoLayout->rowCount() - 4)->widget());
    if (stLbl) stLbl->setTextFormat(Qt::RichText);

    dlgLayout->addWidget(infoGroup);

    // 商品明细
    auto* itemsGroup = new QGroupBox("商品明细");
    itemsGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; padding-top: 15px; margin-top: 8px; }");
    auto* itemsLayout = new QVBoxLayout(itemsGroup);

    auto* itemsTable = new QTableWidget();
    itemsTable->setColumnCount(4);
    itemsTable->setHorizontalHeaderLabels({"商品ID", "商品名称", "数量", "单价"});
    itemsTable->horizontalHeader()->setStretchLastSection(true);
    itemsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    itemsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemsTable->setSelectionMode(QAbstractItemView::NoSelection);
    itemsTable->setAlternatingRowColors(true);

    itemsTable->setRowCount(static_cast<int>(items.size()));
    for (size_t i = 0; i < items.size(); ++i) {
        itemsTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(items[i], "product_id"))));
        itemsTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(items[i], "name")));
        itemsTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::number(rowInt(items[i], "quantity"))));
        itemsTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString("¥%1").arg(rowDouble(items[i], "price"), 0, 'f', 2)));
    }
    itemsTable->setMaximumHeight(static_cast<int>(items.size() + 1) * 30 + 5);
    itemsLayout->addWidget(itemsTable);

    // 合计
    auto* totalLabel = new QLabel(QString("合计：¥%1 （共 %2 件商品）")
                                  .arg(total, 0, 'f', 2).arg(items.size()));
    totalLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c; margin-top: 5px;");
    totalLabel->setAlignment(Qt::AlignRight);
    itemsLayout->addWidget(totalLabel);

    dlgLayout->addWidget(itemsGroup);

    // 关闭按钮
    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlgLayout->addWidget(btnBox);

    dlg.exec();
}

// ========== 管理员：编辑订单 ==========
void OrderPage::onEditOrder() {
    if (selectedOrderId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择订单");
        return;
    }

    auto& db = DBManager::instance();

    // 查询当前订单信息
    auto orderResult = db.query(
        "SELECT order_no, address, remark, status FROM orders WHERE id=" + std::to_string(selectedOrderId_));
    if (orderResult.empty()) return;

    QString orderNo = rowStr(orderResult[0], "order_no");
    QString curAddress = rowStr(orderResult[0], "address");
    QString curRemark = rowStr(orderResult[0], "remark");
    int curStatus = rowInt(orderResult[0], "status");

    // 编辑对话框
    QDialog dlg(this);
    dlg.setWindowTitle("编辑订单 - " + orderNo);
    dlg.resize(480, 280);

    auto* dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setSpacing(12);

    auto* title = new QLabel("编辑订单信息（商品明细不可修改）");
    title->setStyleSheet("font-size: 13px; color: #7f8c8d; margin-bottom: 5px;");
    dlgLayout->addWidget(title);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    // 订单号（只读）
    auto* orderNoLabel = new QLabel(orderNo);
    orderNoLabel->setStyleSheet("color: #7f8c8d;");
    formLayout->addRow("订单编号:", orderNoLabel);

    // 状态（只读）
    auto* statusLabel = new QLabel(statusText(curStatus));
    statusLabel->setStyleSheet("color: " + statusColor(curStatus).name() + "; font-weight: bold;");
    formLayout->addRow("当前状态:", statusLabel);

    // 收货地址
    auto* addressEdit = new QLineEdit(curAddress);
    addressEdit->setMinimumHeight(28);
    addressEdit->setPlaceholderText("收货地址");
    formLayout->addRow("收货地址:", addressEdit);

    // 备注
    auto* remarkEdit = new QTextEdit();
    remarkEdit->setPlainText(curRemark);
    remarkEdit->setMaximumHeight(60);
    remarkEdit->setPlaceholderText("备注信息");
    formLayout->addRow("备注:", remarkEdit);

    dlgLayout->addLayout(formLayout);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btnBox->button(QDialogButtonBox::Ok)->setText("保存修改");
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlgLayout->addWidget(btnBox);

    if (dlg.exec() != QDialog::Accepted) return;

    QString newAddress = addressEdit->text().trimmed();
    QString newRemark = remarkEdit->toPlainText().trimmed();

    // 更新数据库
    std::string sql = "UPDATE orders SET address='" + db.escape(newAddress.toStdString())
                      + "', remark='" + db.escape(newRemark.toStdString())
                      + "' WHERE id=" + std::to_string(selectedOrderId_);
    db.execute(sql);

    refreshOrderTable();
    QMessageBox::information(this, "成功", "订单信息已更新！");
}

// ========== 管理员：删除订单 ==========
void OrderPage::onDeleteOrder() {
    if (selectedOrderId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择订单");
        return;
    }

    auto& db = DBManager::instance();

    // 查询订单信息供确认
    auto orderResult = db.query(
        "SELECT o.order_no, o.status, o.total_amount, "
        "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i') as created_at "
        "FROM orders o WHERE o.id=" + std::to_string(selectedOrderId_));
    if (orderResult.empty()) return;

    QString orderNo = rowStr(orderResult[0], "order_no");
    int st = rowInt(orderResult[0], "status");
    double total = rowDouble(orderResult[0], "total_amount");
    QString createdAt = rowStr(orderResult[0], "created_at");

    // 确认对话框
    QString warningMsg = QString(
        "⚠️ 确认删除订单？\n\n"
        "订单号：%1\n"
        "状态：%2\n"
        "金额：¥%3\n"
        "时间：%4\n\n"
        "此操作不可恢复！\n"
        "删除后将自动恢复商品库存。")
        .arg(orderNo).arg(statusText(st))
        .arg(total, 0, 'f', 2).arg(createdAt);

    auto reply = QMessageBox::question(this, "确认删除", warningMsg,
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    db.begin();

    // 如果订单不是已取消状态，先恢复库存
    if (st != 4) {
        auto items = db.query(
            "SELECT product_id, quantity FROM order_items WHERE order_id="
            + std::to_string(selectedOrderId_));
        for (auto& item : items) {
            int pid = rowInt(item, "product_id");
            int qty = rowInt(item, "quantity");
            db.execute("UPDATE products SET stock = stock + " + std::to_string(qty)
                       + " WHERE id = " + std::to_string(pid));

            // 记录库存恢复日志
            auto stockResult = db.query("SELECT stock FROM products WHERE id=" + std::to_string(pid));
            int curStock = rowInt(stockResult[0], "stock");
            int beforeStock = curStock - qty;
            db.execute("INSERT INTO inventory_log (product_id, type, quantity, before_stock, after_stock, remark) VALUES ("
                       + std::to_string(pid) + ", 0, " + std::to_string(qty) + ", "
                       + std::to_string(beforeStock) + ", " + std::to_string(curStock) + ", '管理员删除订单恢复库存')");
        }
    }

    // 删除订单（order_items 会级联删除）
    db.execute("DELETE FROM orders WHERE id=" + std::to_string(selectedOrderId_));

    db.commit();

    selectedOrderId_ = 0;
    refreshOrderTable();
    QMessageBox::information(this, "成功", "订单已删除，库存已恢复。");
}
