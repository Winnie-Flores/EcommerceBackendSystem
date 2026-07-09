#include "product_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>

ProductPage::ProductPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    loadCategories();
    loadSuppliers();
    refreshTable();
}

void ProductPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("商品管理");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

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
    table_->setColumnCount(11);
    table_->setHorizontalHeaderLabels({"ID", "名称", "分类", "供应商", "售价", "成本", "库存", "预警", "状态", "描述", "创建时间"});
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
    supplierCombo_ = new QComboBox(); supplierCombo_->setMinimumHeight(28);
    supplierCombo_->addItem("无供应商", 0);

    priceEdit_ = new QDoubleSpinBox(); priceEdit_->setRange(0, 99999999); priceEdit_->setDecimals(2); priceEdit_->setMinimumHeight(28);
    costEdit_ = new QDoubleSpinBox(); costEdit_->setRange(0, 99999999); costEdit_->setDecimals(2); costEdit_->setMinimumHeight(28);
    stockEdit_ = new QSpinBox(); stockEdit_->setRange(0, 999999); stockEdit_->setMinimumHeight(28);
    alertStockEdit_ = new QSpinBox(); alertStockEdit_->setRange(0, 999999); alertStockEdit_->setValue(10); alertStockEdit_->setMinimumHeight(28);
    descEdit_ = new QTextEdit(); descEdit_->setMaximumHeight(60);
    imageUrlEdit_ = new QLineEdit(); imageUrlEdit_->setMinimumHeight(28);

    formLayout->addRow("商品名称:", nameEdit_);
    formLayout->addRow("分类:", categoryCombo_);
    formLayout->addRow("供应商:", supplierCombo_);
    formLayout->addRow("售价:", priceEdit_);
    formLayout->addRow("成本:", costEdit_);
    formLayout->addRow("库存:", stockEdit_);
    formLayout->addRow("预警阈值:", alertStockEdit_);
    formLayout->addRow("描述:", descEdit_);
    formLayout->addRow("图片URL:", imageUrlEdit_);

    mainLayout->addWidget(formGroup);

    // 按钮
    auto* btnLayout = new QHBoxLayout();
    addBtn_ = new QPushButton("新增");
    editBtn_ = new QPushButton("修改");
    deleteBtn_ = new QPushButton("删除");
    toggleBtn_ = new QPushButton("上架/下架");
    QString btnStyle = "QPushButton { padding: 8px 20px; border-radius: 3px; font-weight: bold; }";
    addBtn_->setStyleSheet(btnStyle + "QPushButton { background: #2ecc71; color: white; } QPushButton:hover { background: #27ae60; }");
    editBtn_->setStyleSheet(btnStyle + "QPushButton { background: #f39c12; color: white; } QPushButton:hover { background: #e67e22; }");
    deleteBtn_->setStyleSheet(btnStyle + "QPushButton { background: #e74c3c; color: white; } QPushButton:hover { background: #c0392b; }");
    toggleBtn_->setStyleSheet(btnStyle + "QPushButton { background: #9b59b6; color: white; } QPushButton:hover { background: #8e44ad; }");

    reviewBtn_ = new QPushButton("查看评价");
    reviewBtn_->setStyleSheet(btnStyle + "QPushButton { background: #e67e22; color: white; } QPushButton:hover { background: #d35400; }");

    btnLayout->addWidget(addBtn_);
    btnLayout->addWidget(editBtn_);
    btnLayout->addWidget(deleteBtn_);
    btnLayout->addWidget(toggleBtn_);
    btnLayout->addWidget(reviewBtn_);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(searchBtn, &QPushButton::clicked, this, &ProductPage::onSearch);
    connect(addBtn_, &QPushButton::clicked, this, &ProductPage::onAdd);
    connect(editBtn_, &QPushButton::clicked, this, &ProductPage::onEdit);
    connect(deleteBtn_, &QPushButton::clicked, this, &ProductPage::onDelete);
    connect(toggleBtn_, &QPushButton::clicked, this, &ProductPage::onToggleStatus);
    connect(reviewBtn_, &QPushButton::clicked, this, &ProductPage::onViewReviews);
    connect(table_, &QTableWidget::itemSelectionChanged, this, &ProductPage::onTableSelect);
}

void ProductPage::loadCategories() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT id, name FROM categories ORDER BY id");
    for (auto& row : result) {
        int id = rowInt(row, "id");
        QString name = rowStr(row, "name");
        categoryCombo_->addItem(name, id);
        categoryFilter_->addItem(name, id);
    }
}

void ProductPage::loadSuppliers() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT id, name FROM suppliers ORDER BY id");
    for (auto& row : result) {
        supplierCombo_->addItem(rowStr(row, "name"), rowInt(row, "id"));
    }
}

void ProductPage::refreshTable() {
    auto& db = DBManager::instance();
    std::string sql = "SELECT p.id, p.name, c.name as cat_name, COALESCE(s.name, '-') as sup_name, "
                      "p.price, p.cost, p.stock, p.alert_stock, p.status, p.description, "
                      "DATE_FORMAT(p.created_at, '%Y-%m-%d') as created_at "
                      "FROM products p "
                      "LEFT JOIN categories c ON p.category_id=c.id "
                      "LEFT JOIN suppliers s ON p.supplier_id=s.id ORDER BY p.id";
    auto result = db.query(sql);

    table_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        table_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        table_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "name")));
        table_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "cat_name")));
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(rowStr(result[i], "sup_name")));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(rowDouble(result[i], "price"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::number(rowDouble(result[i], "cost"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(rowInt(result[i], "stock"))));
        table_->setItem(static_cast<int>(i), 7, new QTableWidgetItem(QString::number(rowInt(result[i], "alert_stock"))));

        int status = rowInt(result[i], "status");
        auto* statusItem = new QTableWidgetItem(status == 1 ? "上架" : "下架");
        statusItem->setForeground(status == 1 ? QColor("#27ae60") : QColor("#e74c3c"));
        table_->setItem(static_cast<int>(i), 8, statusItem);

        table_->setItem(static_cast<int>(i), 9, new QTableWidgetItem(rowStr(result[i], "description")));
        table_->setItem(static_cast<int>(i), 10, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void ProductPage::onSearch() {
    QString keyword = searchEdit_->text().trimmed();
    int catId = categoryFilter_->currentData().toInt();
    auto& db = DBManager::instance();

    std::string sql = "SELECT p.id, p.name, c.name as cat_name, COALESCE(s.name, '-') as sup_name, "
                      "p.price, p.cost, p.stock, p.alert_stock, p.status, p.description, "
                      "DATE_FORMAT(p.created_at, '%Y-%m-%d') as created_at "
                      "FROM products p "
                      "LEFT JOIN categories c ON p.category_id=c.id "
                      "LEFT JOIN suppliers s ON p.supplier_id=s.id WHERE 1=1";

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
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(rowStr(result[i], "sup_name")));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::number(rowDouble(result[i], "price"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::number(rowDouble(result[i], "cost"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::number(rowInt(result[i], "stock"))));
        table_->setItem(static_cast<int>(i), 7, new QTableWidgetItem(QString::number(rowInt(result[i], "alert_stock"))));
        int status = rowInt(result[i], "status");
        auto* statusItem = new QTableWidgetItem(status == 1 ? "上架" : "下架");
        statusItem->setForeground(status == 1 ? QColor("#27ae60") : QColor("#e74c3c"));
        table_->setItem(static_cast<int>(i), 8, statusItem);
        table_->setItem(static_cast<int>(i), 9, new QTableWidgetItem(rowStr(result[i], "description")));
        table_->setItem(static_cast<int>(i), 10, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void ProductPage::onTableSelect() {
    auto items = table_->selectedItems();
    if (items.isEmpty()) return;
    int row = items.first()->row();

    editingId_ = table_->item(row, 0)->text().toInt();
    nameEdit_->setText(table_->item(row, 1)->text());

    // 匹配分类
    QString catName = table_->item(row, 2)->text();
    for (int i = 0; i < categoryCombo_->count(); ++i)
        if (categoryCombo_->itemText(i) == catName) { categoryCombo_->setCurrentIndex(i); break; }

    // 匹配供应商
    QString supName = table_->item(row, 3)->text();
    for (int i = 0; i < supplierCombo_->count(); ++i)
        if (supplierCombo_->itemText(i) == supName) { supplierCombo_->setCurrentIndex(i); break; }

    priceEdit_->setValue(table_->item(row, 4)->text().toDouble());
    costEdit_->setValue(table_->item(row, 5)->text().toDouble());
    stockEdit_->setValue(table_->item(row, 6)->text().toInt());
    alertStockEdit_->setValue(table_->item(row, 7)->text().toInt());
    descEdit_->setText(table_->item(row, 9)->text());
}

void ProductPage::onAdd() {
    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "商品名称为必填项");
        return;
    }

    auto& db = DBManager::instance();
    std::string sql = "INSERT INTO products (name, category_id, supplier_id, price, cost, stock, alert_stock, description, image_url) VALUES ('"
                      + db.escape(nameEdit_->text().trimmed().toStdString()) + "', "
                      + std::to_string(categoryCombo_->currentData().toInt()) + ", "
                      + (supplierCombo_->currentData().toInt() > 0 ? std::to_string(supplierCombo_->currentData().toInt()) : "NULL") + ", "
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

void ProductPage::onEdit() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要修改的商品");
        return;
    }

    auto& db = DBManager::instance();
    std::string sql = "UPDATE products SET name='"
                      + db.escape(nameEdit_->text().trimmed().toStdString()) + "', category_id="
                      + std::to_string(categoryCombo_->currentData().toInt()) + ", supplier_id="
                      + (supplierCombo_->currentData().toInt() > 0 ? std::to_string(supplierCombo_->currentData().toInt()) : "NULL")
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
        QMessageBox::information(this, "成功", "商品修改成功");
    } else {
        QMessageBox::critical(this, "错误", "修改失败");
    }
}

void ProductPage::onDelete() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的商品");
        return;
    }
    auto reply = QMessageBox::question(this, "确认", "确定要删除该商品吗？",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    auto& db = DBManager::instance();
    if (db.execute("DELETE FROM products WHERE id=" + std::to_string(editingId_)) >= 0) {
        clearForm();
        refreshTable();
        QMessageBox::information(this, "成功", "商品删除成功");
    } else {
        QMessageBox::critical(this, "错误", "删除失败（该商品可能存在关联订单）");
    }
}

void ProductPage::onToggleStatus() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择商品");
        return;
    }
    auto& db = DBManager::instance();
    auto result = db.query("SELECT status FROM products WHERE id=" + std::to_string(editingId_));
    if (result.empty()) return;

    int curStatus = rowInt(result[0], "status");
    int newStatus = curStatus == 1 ? 0 : 1;
    db.execute("UPDATE products SET status=" + std::to_string(newStatus) + " WHERE id=" + std::to_string(editingId_));
    refreshTable();
}

void ProductPage::clearForm() {
    editingId_ = 0;
    nameEdit_->clear();
    categoryCombo_->setCurrentIndex(0);
    supplierCombo_->setCurrentIndex(0);
    priceEdit_->setValue(0);
    costEdit_->setValue(0);
    stockEdit_->setValue(0);
    alertStockEdit_->setValue(10);
    descEdit_->clear();
    imageUrlEdit_->clear();
}

void ProductPage::onViewReviews() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先在表格中选择一个商品");
        return;
    }

    auto& db = DBManager::instance();

    // 查询商品名称
    auto prod = db.query("SELECT name FROM products WHERE id=" + std::to_string(editingId_));
    if (prod.empty()) return;
    QString productName = rowStr(prod[0], "name");

    // 查询评价
    auto reviews = db.query(
        "SELECT pr.id, pr.rating, pr.content, u.username, "
        "DATE_FORMAT(pr.created_at, '%Y-%m-%d %H:%i') as created_at "
        "FROM product_reviews pr JOIN users u ON pr.user_id=u.id "
        "WHERE pr.product_id=" + std::to_string(editingId_) + " ORDER BY pr.created_at DESC");

    // 计算平均分
    double avgRating = 0;
    if (!reviews.empty()) {
        auto avgResult = db.query(
            "SELECT AVG(rating) as avg_rating, COUNT(*) as cnt "
            "FROM product_reviews WHERE product_id=" + std::to_string(editingId_));
        if (!avgResult.empty())
            avgRating = rowDouble(avgResult[0], "avg_rating");
    }

    // 构建弹窗
    QDialog dlg(this);
    dlg.setWindowTitle("商品评价 - " + productName);
    dlg.resize(550, 420);

    auto* dlgLayout = new QVBoxLayout(&dlg);

    // 评分汇总
    auto* summaryLabel = new QLabel(
        QString("平均评分：%1 / 5.0   （共 %2 条评价）")
            .arg(avgRating, 0, 'f', 1)
            .arg(reviews.size()));
    summaryLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #e67e22; margin-bottom: 8px;");
    dlgLayout->addWidget(summaryLabel);

    // 评分条
    auto* barLayout = new QHBoxLayout();
    for (int s = 5; s >= 1; --s) {
        auto countResult = db.query(
            "SELECT COUNT(*) as cnt FROM product_reviews "
            "WHERE product_id=" + std::to_string(editingId_) + " AND rating=" + std::to_string(s));
        int cnt = countResult.empty() ? 0 : rowInt(countResult[0], "cnt");
        int total = static_cast<int>(reviews.size());

        auto* label = new QLabel(QString("%1星").arg(s));
        label->setStyleSheet("font-size: 12px; color: #555;");
        barLayout->addWidget(label);

        auto* bar = new QLabel();
        int barWidth = total > 0 ? static_cast<int>(60.0 * cnt / total) : 0;
        bar->setFixedSize(barWidth, 14);
        bar->setStyleSheet("background: #f39c12; border-radius: 2px;");
        barLayout->addWidget(bar);

        auto* cntLabel = new QLabel(QString::number(cnt));
        cntLabel->setStyleSheet("font-size: 12px; color: #999;");
        barLayout->addWidget(cntLabel);
        barLayout->addSpacing(10);
    }
    barLayout->addStretch();
    dlgLayout->addLayout(barLayout);

    // 评价列表
    auto* reviewTable = new QTableWidget();
    reviewTable->setColumnCount(4);
    reviewTable->setHorizontalHeaderLabels({"用户", "评分", "内容", "时间"});
    reviewTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    reviewTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    reviewTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    reviewTable->verticalHeader()->setVisible(false);
    reviewTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    reviewTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    reviewTable->setAlternatingRowColors(true);

    reviewTable->setRowCount(static_cast<int>(reviews.size()));
    for (size_t i = 0; i < reviews.size(); ++i) {
        int reviewId = rowInt(reviews[i], "id");
        int rating = rowInt(reviews[i], "rating");
        QString user = rowStr(reviews[i], "username");
        QString content = rowStr(reviews[i], "content");
        QString time = rowStr(reviews[i], "created_at");

        reviewTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(user));

        // 星星
        QString stars;
        for (int s = 0; s < rating; ++s) stars += "★";
        for (int s = rating; s < 5; ++s) stars += "☆";
        auto* starItem = new QTableWidgetItem(stars);
        starItem->setForeground(QColor("#f39c12"));
        starItem->setFont(QFont(starItem->font().family(), -1, QFont::Bold));
        reviewTable->setItem(static_cast<int>(i), 1, starItem);

        reviewTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(content));
        reviewTable->setItem(static_cast<int>(i), 3, new QTableWidgetItem(time));
    }

    dlgLayout->addWidget(reviewTable);

    // 底部按钮
    auto* bottomLayout = new QHBoxLayout();

    auto* deleteBtn = new QPushButton("删除选中评价");
    deleteBtn->setStyleSheet("QPushButton { background: #e74c3c; color: white; padding: 6px 16px; border-radius: 3px; } "
                              "QPushButton:hover { background: #c0392b; }");
    bottomLayout->addWidget(deleteBtn);

    bottomLayout->addStretch();

    auto* closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet("QPushButton { background: #95a5a6; color: white; padding: 6px 20px; border-radius: 3px; } "
                             "QPushButton:hover { background: #7f8c8d; }");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    bottomLayout->addWidget(closeBtn);

    dlgLayout->addLayout(bottomLayout);

    // 删除评价功能
    bool needRefresh = false;
    connect(deleteBtn, &QPushButton::clicked, [&]() {
        auto sel = reviewTable->selectedItems();
        if (sel.isEmpty()) {
            QMessageBox::warning(&dlg, "提示", "请先选择一条评价");
            return;
        }
        int row = sel.first()->row();
        int rid = rowInt(reviews[static_cast<size_t>(row)], "id");
        auto reply = QMessageBox::question(&dlg, "确认", "确定要删除该评价吗？",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
        db.execute("DELETE FROM product_reviews WHERE id=" + std::to_string(rid));
        needRefresh = true;
        dlg.accept();
    });

    dlg.exec();

    if (needRefresh) {
        onViewReviews();  // 对话框已完全关闭后再刷新，避免递归崩溃
    }
}
