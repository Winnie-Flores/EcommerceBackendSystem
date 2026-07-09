#include "supplier_product_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>

SupplierProductPage::SupplierProductPage(int supplierId, QWidget* parent)
    : QWidget(parent), supplierId_(supplierId) {
    setupUI();
    loadCategories();
    refreshTable();
    checkStockAlert();
}

void SupplierProductPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("我的商品");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 库存预警标签
    alertLabel_ = new QLabel();
    alertLabel_->setStyleSheet("background: #ffeaa7; color: #d63031; padding: 8px 12px; "
                                "border-radius: 4px; font-weight: bold; font-size: 13px;");
    alertLabel_->setVisible(false);
    mainLayout->addWidget(alertLabel_);

    // 搜索栏
    auto* searchLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索商品名称...");
    searchEdit_->setMinimumHeight(30);

    categoryFilter_ = new QComboBox();
    categoryFilter_->addItem("全部分类", 0);
    categoryFilter_->setMinimumHeight(30);

    auto* searchBtn = new QPushButton("搜索");
    searchBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 6px 16px; border-radius: 3px; } QPushButton:hover { background: #2980b9; }");

    searchLayout->addWidget(searchEdit_);
    searchLayout->addWidget(categoryFilter_);
    searchLayout->addWidget(searchBtn);
    mainLayout->addLayout(searchLayout);

    // 表格
    table_ = new QTableWidget();
    table_->setColumnCount(10);
    table_->setHorizontalHeaderLabels({"ID", "名称", "分类", "售价", "成本", "库存", "预警阈值", "状态", "描述", "创建时间"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->setStyleSheet("QTableWidget { background: white; gridline-color: #ddd; } "
                          "QTableWidget::item:selected { background: #3498db; color: white; }");
    mainLayout->addWidget(table_);

    // 表单
    auto* formGroup = new QGroupBox("商品信息");
    formGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 10px; padding-top: 15px; }");
    auto* formLayout = new QFormLayout(formGroup);

    nameEdit_ = new QLineEdit(); nameEdit_->setMinimumHeight(28);
    categoryCombo_ = new QComboBox(); categoryCombo_->setMinimumHeight(28);

    priceEdit_ = new QDoubleSpinBox(); priceEdit_->setRange(0, 99999999); priceEdit_->setDecimals(2); priceEdit_->setMinimumHeight(28);
    costEdit_ = new QDoubleSpinBox(); costEdit_->setRange(0, 99999999); costEdit_->setDecimals(2); costEdit_->setMinimumHeight(28);
    stockEdit_ = new QSpinBox(); stockEdit_->setRange(0, 999999); stockEdit_->setMinimumHeight(28);
    alertStockEdit_ = new QSpinBox(); alertStockEdit_->setRange(0, 999999); alertStockEdit_->setValue(10); alertStockEdit_->setMinimumHeight(28);
    descEdit_ = new QTextEdit(); descEdit_->setMaximumHeight(60);
    imageUrlEdit_ = new QLineEdit(); imageUrlEdit_->setMinimumHeight(28);

    formLayout->addRow("商品名称:", nameEdit_);
    formLayout->addRow("分类:", categoryCombo_);
    formLayout->addRow("售价:", priceEdit_);
    formLayout->addRow("成本:", costEdit_);
    formLayout->addRow("库存:", stockEdit_);
    formLayout->addRow("预警阈值:", alertStockEdit_);
    formLayout->addRow("描述:", descEdit_);
    formLayout->addRow("图片URL:", imageUrlEdit_);

    mainLayout->addWidget(formGroup);

    // 按钮
    auto* btnLayout = new QHBoxLayout();
    addBtn_ = new QPushButton("新增商品");
    editBtn_ = new QPushButton("修改商品");
    toggleBtn_ = new QPushButton("上架/下架");
    QString btnStyle = "QPushButton { padding: 8px 20px; border-radius: 3px; font-weight: bold; }";
    addBtn_->setStyleSheet(btnStyle + "QPushButton { background: #2ecc71; color: white; } QPushButton:hover { background: #27ae60; }");
    editBtn_->setStyleSheet(btnStyle + "QPushButton { background: #f39c12; color: white; } QPushButton:hover { background: #e67e22; }");
    toggleBtn_->setStyleSheet(btnStyle + "QPushButton { background: #9b59b6; color: white; } QPushButton:hover { background: #8e44ad; }");

    reviewBtn_ = new QPushButton("查看评价");
    reviewBtn_->setStyleSheet(btnStyle + "QPushButton { background: #e67e22; color: white; } QPushButton:hover { background: #d35400; }");

    btnLayout->addWidget(addBtn_);
    btnLayout->addWidget(editBtn_);
    btnLayout->addWidget(toggleBtn_);
    btnLayout->addWidget(reviewBtn_);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(searchBtn, &QPushButton::clicked, this, &SupplierProductPage::onSearch);
    connect(addBtn_, &QPushButton::clicked, this, &SupplierProductPage::onAdd);
    connect(editBtn_, &QPushButton::clicked, this, &SupplierProductPage::onEdit);
    connect(toggleBtn_, &QPushButton::clicked, this, &SupplierProductPage::onToggleStatus);
    connect(reviewBtn_, &QPushButton::clicked, this, &SupplierProductPage::onViewReviews);
    connect(table_, &QTableWidget::itemSelectionChanged, this, &SupplierProductPage::onTableSelect);
}

void SupplierProductPage::loadCategories() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT id, name FROM categories ORDER BY id");
    for (auto& row : result) {
        int id = rowInt(row, "id");
        QString name = rowStr(row, "name");
        categoryCombo_->addItem(name, id);
        categoryFilter_->addItem(name, id);
    }
}

void SupplierProductPage::refreshTable() {
    auto& db = DBManager::instance();
    std::string sql = "SELECT p.id, p.name, c.name as cat_name, "
                      "p.price, p.cost, p.stock, p.alert_stock, p.status, p.description, "
                      "DATE_FORMAT(p.created_at, '%Y-%m-%d') as created_at "
                      "FROM products p "
                      "LEFT JOIN categories c ON p.category_id=c.id "
                      "WHERE p.supplier_id=" + std::to_string(supplierId_) + " ORDER BY p.id";
    auto result = db.query(sql);

    table_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        table_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        table_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "name")));
        table_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "cat_name")));
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(rowDouble(result[i], "price"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(rowDouble(result[i], "cost"), 'f', 2)));

        int stock = rowInt(result[i], "stock");
        int alert = rowInt(result[i], "alert_stock");
        auto* stockItem = new QTableWidgetItem(QString::number(stock));
        if (stock <= alert) {
            stockItem->setForeground(QColor("#e74c3c"));
            stockItem->setFont(QFont(stockItem->font().family(), -1, QFont::Bold));
        }
        table_->setItem(static_cast<int>(i), 5, stockItem);

        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(alert)));

        int status = rowInt(result[i], "status");
        auto* statusItem = new QTableWidgetItem(status == 1 ? "上架" : "下架");
        statusItem->setForeground(status == 1 ? QColor("#27ae60") : QColor("#e74c3c"));
        table_->setItem(static_cast<int>(i), 7, statusItem);

        table_->setItem(static_cast<int>(i), 8, new QTableWidgetItem(rowStr(result[i], "description")));
        table_->setItem(static_cast<int>(i), 9, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void SupplierProductPage::checkStockAlert() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT name, stock, alert_stock FROM products "
                           "WHERE supplier_id=" + std::to_string(supplierId_) + " AND stock <= alert_stock AND status=1");

    if (result.empty()) {
        alertLabel_->setVisible(false);
        return;
    }

    QStringList warnings;
    for (auto& row : result) {
        QString name = rowStr(row, "name");
        int stock = rowInt(row, "stock");
        int alert = rowInt(row, "alert_stock");
        warnings << (name + "(库存:" + QString::number(stock) + "/预警:" + QString::number(alert) + ")");
    }

    alertLabel_->setText("⚠ 库存预警: " + warnings.join(" | "));
    alertLabel_->setVisible(true);

    // 弹窗提醒
    QTimer::singleShot(500, this, [this, warnings]() {
        QMessageBox::warning(this, "库存预警",
                             "以下商品库存不足，请及时补货：\n\n" + warnings.join("\n"));
    });
}

void SupplierProductPage::onSearch() {
    QString keyword = searchEdit_->text().trimmed();
    int catId = categoryFilter_->currentData().toInt();
    auto& db = DBManager::instance();

    std::string sql = "SELECT p.id, p.name, c.name as cat_name, "
                      "p.price, p.cost, p.stock, p.alert_stock, p.status, p.description, "
                      "DATE_FORMAT(p.created_at, '%Y-%m-%d') as created_at "
                      "FROM products p "
                      "LEFT JOIN categories c ON p.category_id=c.id "
                      "WHERE p.supplier_id=" + std::to_string(supplierId_);

    if (!keyword.isEmpty())
        sql += " AND p.name LIKE '%" + db.escape(keyword.toStdString()) + "%'";
    if (catId > 0)
        sql += " AND p.category_id=" + std::to_string(catId);
    sql += " ORDER BY p.id";

    auto result = db.query(sql);
    table_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        table_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        table_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "name")));
        table_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "cat_name")));
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(rowDouble(result[i], "price"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(rowDouble(result[i], "cost"), 'f', 2)));

        int stock = rowInt(result[i], "stock");
        int alert = rowInt(result[i], "alert_stock");
        auto* stockItem = new QTableWidgetItem(QString::number(stock));
        if (stock <= alert) {
            stockItem->setForeground(QColor("#e74c3c"));
            stockItem->setFont(QFont(stockItem->font().family(), -1, QFont::Bold));
        }
        table_->setItem(static_cast<int>(i), 5, stockItem);

        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(alert)));
        int status = rowInt(result[i], "status");
        auto* statusItem = new QTableWidgetItem(status == 1 ? "上架" : "下架");
        statusItem->setForeground(status == 1 ? QColor("#27ae60") : QColor("#e74c3c"));
        table_->setItem(static_cast<int>(i), 7, statusItem);
        table_->setItem(static_cast<int>(i), 8, new QTableWidgetItem(rowStr(result[i], "description")));
        table_->setItem(static_cast<int>(i), 9, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void SupplierProductPage::onTableSelect() {
    auto items = table_->selectedItems();
    if (items.isEmpty()) return;
    int row = items.first()->row();

    editingId_ = table_->item(row, 0)->text().toInt();
    nameEdit_->setText(table_->item(row, 1)->text());

    // 匹配分类
    QString catName = table_->item(row, 2)->text();
    for (int i = 0; i < categoryCombo_->count(); ++i)
        if (categoryCombo_->itemText(i) == catName) { categoryCombo_->setCurrentIndex(i); break; }

    priceEdit_->setValue(table_->item(row, 3)->text().toDouble());
    costEdit_->setValue(table_->item(row, 4)->text().toDouble());
    stockEdit_->setValue(table_->item(row, 5)->text().toInt());
    alertStockEdit_->setValue(table_->item(row, 6)->text().toInt());
    descEdit_->setText(table_->item(row, 8)->text());
}

void SupplierProductPage::onAdd() {
    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "商品名称为必填项");
        return;
    }

    auto& db = DBManager::instance();
    std::string sql = "INSERT INTO products (name, category_id, supplier_id, price, cost, stock, alert_stock, description, image_url) VALUES ('"
                      + db.escape(nameEdit_->text().trimmed().toStdString()) + "', "
                      + std::to_string(categoryCombo_->currentData().toInt()) + ", "
                      + std::to_string(supplierId_) + ", "
                      + std::to_string(priceEdit_->value()) + ", "
                      + std::to_string(costEdit_->value()) + ", "
                      + std::to_string(stockEdit_->value()) + ", "
                      + std::to_string(alertStockEdit_->value()) + ", '"
                      + db.escape(descEdit_->toPlainText().toStdString()) + "', '"
                      + db.escape(imageUrlEdit_->text().trimmed().toStdString()) + "')";

    if (db.execute(sql) >= 0) {
        clearForm();
        refreshTable();
        QMessageBox::information(this, "成功", "商品添加成功");
    } else {
        QMessageBox::critical(this, "错误", "添加失败");
    }
}

void SupplierProductPage::onEdit() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要修改的商品");
        return;
    }

    // 验证商品是否属于当前供应商
    auto& db = DBManager::instance();
    auto check = db.query("SELECT supplier_id FROM products WHERE id=" + std::to_string(editingId_));
    if (check.empty() || rowInt(check[0], "supplier_id") != supplierId_) {
        QMessageBox::warning(this, "提示", "只能修改自己的商品");
        return;
    }

    std::string sql = "UPDATE products SET name='"
                      + db.escape(nameEdit_->text().trimmed().toStdString()) + "', category_id="
                      + std::to_string(categoryCombo_->currentData().toInt())
                      + ", price=" + std::to_string(priceEdit_->value())
                      + ", cost=" + std::to_string(costEdit_->value())
                      + ", stock=" + std::to_string(stockEdit_->value())
                      + ", alert_stock=" + std::to_string(alertStockEdit_->value())
                      + ", description='" + db.escape(descEdit_->toPlainText().toStdString()) + "'"
                      + ", image_url='" + db.escape(imageUrlEdit_->text().trimmed().toStdString()) + "'"
                      + " WHERE id=" + std::to_string(editingId_);

    if (db.execute(sql) >= 0) {
        clearForm();
        refreshTable();
        checkStockAlert();
        QMessageBox::information(this, "成功", "商品修改成功");
    } else {
        QMessageBox::critical(this, "错误", "修改失败");
    }
}

void SupplierProductPage::onToggleStatus() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择商品");
        return;
    }

    auto& db = DBManager::instance();
    auto check = db.query("SELECT supplier_id, status FROM products WHERE id=" + std::to_string(editingId_));
    if (check.empty() || rowInt(check[0], "supplier_id") != supplierId_) {
        QMessageBox::warning(this, "提示", "只能操作自己的商品");
        return;
    }

    int curStatus = rowInt(check[0], "status");
    int newStatus = curStatus == 1 ? 0 : 1;
    db.execute("UPDATE products SET status=" + std::to_string(newStatus) + " WHERE id=" + std::to_string(editingId_));
    refreshTable();
}

void SupplierProductPage::clearForm() {
    editingId_ = 0;
    nameEdit_->clear();
    categoryCombo_->setCurrentIndex(0);
    priceEdit_->setValue(0);
    costEdit_->setValue(0);
    stockEdit_->setValue(0);
    alertStockEdit_->setValue(10);
    descEdit_->clear();
    imageUrlEdit_->clear();
}

void SupplierProductPage::onViewReviews() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先在表格中选择一个商品");
        return;
    }

    auto& db = DBManager::instance();

    // 验证商品属于当前供应商
    auto check = db.query("SELECT id, name FROM products WHERE id="
                          + std::to_string(editingId_) + " AND supplier_id=" + std::to_string(supplierId_));
    if (check.empty()) {
        QMessageBox::warning(this, "提示", "只能查看自己商品的评价");
        return;
    }
    QString productName = rowStr(check[0], "name");

    // 查询评价
    auto reviews = db.query(
        "SELECT pr.rating, pr.content, u.username, "
        "DATE_FORMAT(pr.created_at, '%Y-%m-%d %H:%i') as created_at "
        "FROM product_reviews pr JOIN users u ON pr.user_id=u.id "
        "WHERE pr.product_id=" + std::to_string(editingId_) + " ORDER BY pr.created_at DESC");

    double avgRating = 0;
    if (!reviews.empty()) {
        auto avgResult = db.query(
            "SELECT AVG(rating) as avg_rating FROM product_reviews WHERE product_id="
            + std::to_string(editingId_));
        if (!avgResult.empty())
            avgRating = rowDouble(avgResult[0], "avg_rating");
    }

    QDialog dlg(this);
    dlg.setWindowTitle("商品评价 - " + productName);
    dlg.resize(500, 400);

    auto* dlgLayout = new QVBoxLayout(&dlg);

    auto* summaryLabel = new QLabel(
        QString("平均评分：%1 / 5.0   （共 %2 条评价）")
            .arg(avgRating, 0, 'f', 1)
            .arg(reviews.size()));
    summaryLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #e67e22; margin-bottom: 8px;");
    dlgLayout->addWidget(summaryLabel);

    auto* reviewTable = new QTableWidget();
    reviewTable->setColumnCount(4);
    reviewTable->setHorizontalHeaderLabels({"用户", "评分", "内容", "时间"});
    reviewTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    reviewTable->verticalHeader()->setVisible(false);
    reviewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reviewTable->setAlternatingRowColors(true);

    reviewTable->setRowCount(static_cast<int>(reviews.size()));
    for (size_t i = 0; i < reviews.size(); ++i) {
        int rating = rowInt(reviews[i], "rating");
        reviewTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(rowStr(reviews[i], "username")));

        QString stars;
        for (int s = 0; s < rating; ++s) stars += "★";
        for (int s = rating; s < 5; ++s) stars += "☆";
        auto* starItem = new QTableWidgetItem(stars);
        starItem->setForeground(QColor("#f39c12"));
        starItem->setFont(QFont(starItem->font().family(), -1, QFont::Bold));
        reviewTable->setItem(static_cast<int>(i), 1, starItem);

        reviewTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(reviews[i], "content")));
        reviewTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(rowStr(reviews[i], "created_at")));
    }

    dlgLayout->addWidget(reviewTable);

    auto* closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet("QPushButton { background: #95a5a6; color: white; padding: 6px 20px; border-radius: 3px; } "
                             "QPushButton:hover { background: #7f8c8d; }");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    dlgLayout->addLayout(btnLayout);

    dlg.exec();
}
