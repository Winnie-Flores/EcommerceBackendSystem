#include "inventory_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>

InventoryPage::InventoryPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    loadProducts();
    refreshProductTable();
    refreshLogTable();
}

void InventoryPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("库存管理");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 预警标签
    alertLabel_ = new QLabel();
    alertLabel_->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 5px;");
    mainLayout->addWidget(alertLabel_);

    // 搜索
    auto* searchLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索商品名称...");
    searchEdit_->setMinimumHeight(30);
    auto* searchBtn = new QPushButton("搜索");
    searchBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 6px 16px; border-radius: 3px; } QPushButton:hover { background: #2980b9; }");
    searchLayout->addWidget(searchEdit_);
    searchLayout->addWidget(searchBtn);
    mainLayout->addLayout(searchLayout);

    // 商品库存表
    productTable_ = new QTableWidget();
    productTable_->setColumnCount(7);
    productTable_->setHorizontalHeaderLabels({"ID", "商品名称", "分类", "当前库存", "预警阈值", "状态", "售价"});
    productTable_->horizontalHeader()->setStretchLastSection(true);
    productTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    productTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    productTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    productTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productTable_->setAlternatingRowColors(true);
    productTable_->setStyleSheet("QTableWidget { background: white; gridline-color: #ddd; } "
                                 "QTableWidget::item:selected { background: #3498db; color: white; }");
    mainLayout->addWidget(productTable_);

    // 出入库操作区
    auto* opGroup = new QGroupBox("出入库操作");
    opGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 10px; padding-top: 15px; }");
    auto* opLayout = new QHBoxLayout(opGroup);

    productCombo_ = new QComboBox(); productCombo_->setMinimumHeight(30);
    quantitySpin_ = new QSpinBox(); quantitySpin_->setRange(1, 99999); quantitySpin_->setValue(1); quantitySpin_->setMinimumHeight(30);
    remarkEdit_ = new QLineEdit(); remarkEdit_->setPlaceholderText("备注..."); remarkEdit_->setMinimumHeight(30);

    stockInBtn_ = new QPushButton("入库 +");
    stockOutBtn_ = new QPushButton("出库 -");
    stockInBtn_->setStyleSheet("QPushButton { background: #2ecc71; color: white; padding: 8px 20px; border-radius: 3px; font-weight: bold; } QPushButton:hover { background: #27ae60; }");
    stockOutBtn_->setStyleSheet("QPushButton { background: #e74c3c; color: white; padding: 8px 20px; border-radius: 3px; font-weight: bold; } QPushButton:hover { background: #c0392b; }");

    opLayout->addWidget(new QLabel("商品:"));
    opLayout->addWidget(productCombo_);
    opLayout->addWidget(new QLabel("数量:"));
    opLayout->addWidget(quantitySpin_);
    opLayout->addWidget(new QLabel("备注:"));
    opLayout->addWidget(remarkEdit_);
    opLayout->addWidget(stockInBtn_);
    opLayout->addWidget(stockOutBtn_);

    mainLayout->addWidget(opGroup);

    // 变动日志
    auto* logLabel = new QLabel("库存变动日志");
    logLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; margin-top: 10px;");
    mainLayout->addWidget(logLabel);

    logTable_ = new QTableWidget();
    logTable_->setColumnCount(6);
    logTable_->setHorizontalHeaderLabels({"ID", "商品", "类型", "数量", "变动前", "变动后"});
    logTable_->horizontalHeader()->setStretchLastSection(true);
    logTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    logTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable_->setAlternatingRowColors(true);
    logTable_->setMaximumHeight(200);
    logTable_->setStyleSheet("QTableWidget { background: white; gridline-color: #ddd; }");
    mainLayout->addWidget(logTable_);

    connect(searchBtn, &QPushButton::clicked, this, &InventoryPage::onSearch);
    connect(stockInBtn_, &QPushButton::clicked, this, &InventoryPage::onStockIn);
    connect(stockOutBtn_, &QPushButton::clicked, this, &InventoryPage::onStockOut);
}

void InventoryPage::loadProducts() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT id, name FROM products ORDER BY id");
    productCombo_->clear();
    for (auto& row : result) {
        productCombo_->addItem(rowStr(row, "name"), rowInt(row, "id"));
    }
}

void InventoryPage::refreshProductTable() {
    auto& db = DBManager::instance();
    auto result = db.query(
        "SELECT p.id, p.name, c.name as cat_name, p.stock, p.alert_stock, p.status, p.price "
        "FROM products p LEFT JOIN categories c ON p.category_id=c.id ORDER BY p.id");

    QStringList alerts;
    productTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        productTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        productTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "name")));
        productTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "cat_name")));

        int stock = rowInt(result[i], "stock");
        int alert = rowInt(result[i], "alert_stock");
        auto* stockItem = new QTableWidgetItem(QString::number(stock));
        if (stock <= alert) {
            stockItem->setForeground(QColor("#e74c3c"));
            stockItem->setFont(QFont(stockItem->font().family(), -1, QFont::Bold));
            alerts.append(rowStr(result[i], "name") + "(" + QString::number(stock) + ")");
        }
        productTable_->setItem(static_cast<int>(i), 3, stockItem);
        productTable_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(alert)));

        int status = rowInt(result[i], "status");
        auto* statusItem = new QTableWidgetItem(status == 1 ? "上架" : "下架");
        statusItem->setForeground(status == 1 ? QColor("#27ae60") : QColor("#e74c3c"));
        productTable_->setItem(static_cast<int>(i), 5, statusItem);
        productTable_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(rowDouble(result[i], "price"), 'f', 2)));
    }

    if (alerts.isEmpty())
        alertLabel_->setText("");
    else
        alertLabel_->setText("⚠ 库存预警: " + alerts.join(", "));
}

void InventoryPage::refreshLogTable() {
    auto& db = DBManager::instance();
    auto result = db.query(
        "SELECT l.id, p.name as product_name, l.type, l.quantity, l.before_stock, l.after_stock "
        "FROM inventory_log l LEFT JOIN products p ON l.product_id=p.id ORDER BY l.id DESC LIMIT 50");

    logTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        logTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        logTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "product_name")));
        QString typeStr = rowInt(result[i], "type") == 0 ? "入库" : "出库";
        auto* typeItem = new QTableWidgetItem(typeStr);
        typeItem->setForeground(rowInt(result[i], "type") == 0 ? QColor("#27ae60") : QColor("#e74c3c"));
        logTable_->setItem(static_cast<int>(i), 2, typeItem);
        logTable_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(rowInt(result[i], "quantity"))));
        logTable_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(rowInt(result[i], "before_stock"))));
        logTable_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::number(rowInt(result[i], "after_stock"))));
    }
}

void InventoryPage::onStockIn() {
    int productId = productCombo_->currentData().toInt();
    int qty = quantitySpin_->value();
    QString remark = remarkEdit_->text().trimmed();

    auto& db = DBManager::instance();

    // 获取当前库存（用于记录日志）
    auto result = db.query("SELECT stock FROM products WHERE id=" + std::to_string(productId));
    if (result.empty()) return;
    int curStock = rowInt(result[0], "stock");

    db.begin();
    // 原子入库
    db.execute("UPDATE products SET stock = stock + " + std::to_string(qty)
               + " WHERE id = " + std::to_string(productId));
    db.execute("INSERT INTO inventory_log (product_id, type, quantity, before_stock, after_stock, remark) VALUES ("
               + std::to_string(productId) + ", 0, " + std::to_string(qty) + ", "
               + std::to_string(curStock) + ", " + std::to_string(curStock + qty) + ", '"
               + db.escape(remark.toStdString()) + "')");
    db.commit();

    refreshProductTable();
    refreshLogTable();
    loadProducts();
    remarkEdit_->clear();
}

void InventoryPage::onStockOut() {
    int productId = productCombo_->currentData().toInt();
    int qty = quantitySpin_->value();
    QString remark = remarkEdit_->text().trimmed();

    auto& db = DBManager::instance();

    // 获取当前库存（用于日志 + 初步检查）
    auto result = db.query("SELECT stock FROM products WHERE id=" + std::to_string(productId));
    if (result.empty()) return;
    int curStock = rowInt(result[0], "stock");

    if (curStock < qty) {
        QMessageBox::warning(this, "提示", "库存不足！当前库存: " + QString::number(curStock));
        return;
    }

    db.begin();
    // 原子出库（并发安全）
    std::string outSql = "UPDATE products SET stock = stock - " + std::to_string(qty)
        + " WHERE id = " + std::to_string(productId) + " AND stock >= " + std::to_string(qty);
    int affected = db.execute(outSql);
    if (affected == 0) {
        db.rollback();
        QMessageBox::warning(this, "提示", "库存不足（可能被其他操作修改），请重试");
        refreshProductTable();
        return;
    }

    db.execute("INSERT INTO inventory_log (product_id, type, quantity, before_stock, after_stock, remark) VALUES ("
               + std::to_string(productId) + ", 1, " + std::to_string(qty) + ", "
               + std::to_string(curStock) + ", " + std::to_string(curStock - qty) + ", '"
               + db.escape(remark.toStdString()) + "')");
    db.commit();

    refreshProductTable();
    refreshLogTable();
    loadProducts();
    remarkEdit_->clear();
}

void InventoryPage::onSearch() {
    QString kw = searchEdit_->text().trimmed();
    if (kw.isEmpty()) { refreshProductTable(); return; }

    auto& db = DBManager::instance();
    auto result = db.query(
        "SELECT p.id, p.name, c.name as cat_name, p.stock, p.alert_stock, p.status, p.price "
        "FROM products p LEFT JOIN categories c ON p.category_id=c.id "
        "WHERE p.name LIKE '%" + db.escape(kw.toStdString()) + "%' ORDER BY p.id");

    productTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        productTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        productTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "name")));
        productTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "cat_name")));
        int stock = rowInt(result[i], "stock");
        int alert = rowInt(result[i], "alert_stock");
        auto* stockItem = new QTableWidgetItem(QString::number(stock));
        if (stock <= alert) stockItem->setForeground(QColor("#e74c3c"));
        productTable_->setItem(static_cast<int>(i), 3, stockItem);
        productTable_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(alert)));
        int status = rowInt(result[i], "status");
        auto* statusItem = new QTableWidgetItem(status == 1 ? "上架" : "下架");
        statusItem->setForeground(status == 1 ? QColor("#27ae60") : QColor("#e74c3c"));
        productTable_->setItem(static_cast<int>(i), 5, statusItem);
        productTable_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(rowDouble(result[i], "price"), 'f', 2)));
    }
}
