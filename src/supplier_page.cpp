#include "supplier_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <openssl/sha.h>

// SHA256 辅助函数
static QString sha256(const QString& str) {
    QByteArray data = str.toUtf8();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.constData()), data.size(), hash);

    QString result;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        result.append(QString::asprintf("%02x", hash[i]));
    }
    return result;
}

SupplierPage::SupplierPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    refreshTable();
}

void SupplierPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("供应商管理");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 搜索
    auto* searchLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索供应商名称...");
    searchEdit_->setMinimumHeight(30);
    auto* searchBtn = new QPushButton("搜索");
    searchBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 6px 16px; border-radius: 3px; } QPushButton:hover { background: #2980b9; }");
    searchLayout->addWidget(searchEdit_);
    searchLayout->addWidget(searchBtn);
    mainLayout->addLayout(searchLayout);

    // 表格
    table_ = new QTableWidget();
    table_->setColumnCount(8);
    table_->setHorizontalHeaderLabels({"ID", "名称", "联系人", "电话", "邮箱", "地址", "登录账号", "创建时间"});
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
    auto* formGroup = new QGroupBox("供应商信息");
    formGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 10px; padding-top: 15px; }");
    auto* formLayout = new QFormLayout(formGroup);

    nameEdit_ = new QLineEdit(); nameEdit_->setMinimumHeight(28);
    contactEdit_ = new QLineEdit(); contactEdit_->setMinimumHeight(28);
    phoneEdit_ = new QLineEdit(); phoneEdit_->setMinimumHeight(28);
    emailEdit_ = new QLineEdit(); emailEdit_->setMinimumHeight(28);
    addressEdit_ = new QLineEdit(); addressEdit_->setMinimumHeight(28);
    usernameEdit_ = new QLineEdit(); usernameEdit_->setMinimumHeight(28);
    usernameEdit_->setPlaceholderText("供应商登录用户名（新增时必填）");
    passwordEdit_ = new QLineEdit(); passwordEdit_->setMinimumHeight(28);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText("新增时填写，修改时留空表示不修改");

    formLayout->addRow("名称:", nameEdit_);
    formLayout->addRow("联系人:", contactEdit_);
    formLayout->addRow("电话:", phoneEdit_);
    formLayout->addRow("邮箱:", emailEdit_);
    formLayout->addRow("地址:", addressEdit_);
    formLayout->addRow("登录账号:", usernameEdit_);
    formLayout->addRow("登录密码:", passwordEdit_);

    mainLayout->addWidget(formGroup);

    // 按钮
    auto* btnLayout = new QHBoxLayout();
    addBtn_ = new QPushButton("新增");
    editBtn_ = new QPushButton("修改");
    deleteBtn_ = new QPushButton("删除");
    QString btnStyle = "QPushButton { padding: 8px 20px; border-radius: 3px; font-weight: bold; }";
    addBtn_->setStyleSheet(btnStyle + "QPushButton { background: #2ecc71; color: white; } QPushButton:hover { background: #27ae60; }");
    editBtn_->setStyleSheet(btnStyle + "QPushButton { background: #f39c12; color: white; } QPushButton:hover { background: #e67e22; }");
    deleteBtn_->setStyleSheet(btnStyle + "QPushButton { background: #e74c3c; color: white; } QPushButton:hover { background: #c0392b; }");

    btnLayout->addWidget(addBtn_);
    btnLayout->addWidget(editBtn_);
    btnLayout->addWidget(deleteBtn_);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(searchBtn, &QPushButton::clicked, this, &SupplierPage::onSearch);
    connect(addBtn_, &QPushButton::clicked, this, &SupplierPage::onAdd);
    connect(editBtn_, &QPushButton::clicked, this, &SupplierPage::onEdit);
    connect(deleteBtn_, &QPushButton::clicked, this, &SupplierPage::onDelete);
    connect(table_, &QTableWidget::itemSelectionChanged, this, &SupplierPage::onTableSelect);
}

void SupplierPage::refreshTable() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT s.id, s.name, s.contact, s.phone, s.email, s.address, "
                           "COALESCE(u.username, '-') as username, "
                           "DATE_FORMAT(s.created_at, '%Y-%m-%d') as created_at "
                           "FROM suppliers s LEFT JOIN users u ON s.user_id=u.id ORDER BY s.id");

    table_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        table_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        table_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "name")));
        table_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "contact")));
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(rowStr(result[i], "phone")));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(rowStr(result[i], "email")));
        table_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "address")));
        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(rowStr(result[i], "username")));
        table_->setItem(static_cast<int>(i), 7, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void SupplierPage::onSearch() {
    QString kw = searchEdit_->text().trimmed();
    if (kw.isEmpty()) { refreshTable(); return; }

    auto& db = DBManager::instance();
    std::string sql = "SELECT s.id, s.name, s.contact, s.phone, s.email, s.address, "
                      "COALESCE(u.username, '-') as username, "
                      "DATE_FORMAT(s.created_at, '%Y-%m-%d') as created_at "
                      "FROM suppliers s LEFT JOIN users u ON s.user_id=u.id "
                      "WHERE s.name LIKE '%" + db.escape(kw.toStdString()) + "%' ORDER BY s.id";
    auto result = db.query(sql);

    table_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        table_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        table_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "name")));
        table_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "contact")));
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(rowStr(result[i], "phone")));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(rowStr(result[i], "email")));
        table_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "address")));
        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(rowStr(result[i], "username")));
        table_->setItem(static_cast<int>(i), 7, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void SupplierPage::onTableSelect() {
    auto items = table_->selectedItems();
    if (items.isEmpty()) return;
    int row = items.first()->row();

    editingId_ = table_->item(row, 0)->text().toInt();
    nameEdit_->setText(table_->item(row, 1)->text());
    contactEdit_->setText(table_->item(row, 2)->text());
    phoneEdit_->setText(table_->item(row, 3)->text());
    emailEdit_->setText(table_->item(row, 4)->text());
    addressEdit_->setText(table_->item(row, 5)->text());
    usernameEdit_->setText(table_->item(row, 6)->text());
    passwordEdit_->clear();
}

void SupplierPage::onAdd() {
    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "供应商名称为必填项");
        return;
    }

    QString username = usernameEdit_->text().trimmed();
    QString password = passwordEdit_->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "必须填写登录账号和密码");
        return;
    }
    if (password.length() < 4) {
        QMessageBox::warning(this, "提示", "密码至少4位");
        return;
    }

    auto& db = DBManager::instance();

    // 检查用户名是否已存在
    auto check = db.query("SELECT id FROM users WHERE username='"
                          + db.escape(username.toStdString()) + "'");
    if (!check.empty()) {
        QMessageBox::warning(this, "提示", "登录账号已存在，请更换");
        return;
    }

    db.begin();

    // 创建用户账号
    QString hash = sha256(password);
    std::string userSql = "INSERT INTO users (username, password, real_name, phone, email, role) VALUES ('"
                          + db.escape(username.toStdString()) + "', '"
                          + hash.toStdString() + "', '"
                          + db.escape(contactEdit_->text().trimmed().toStdString()) + "', '"
                          + db.escape(phoneEdit_->text().trimmed().toStdString()) + "', '"
                          + db.escape(emailEdit_->text().trimmed().toStdString()) + "', 2)";

    long long userId = db.insert(userSql);
    if (userId <= 0) {
        db.rollback();
        QMessageBox::critical(this, "错误", "创建登录账号失败");
        return;
    }

    // 创建供应商
    std::string sql = "INSERT INTO suppliers (name, contact, phone, email, address, user_id) VALUES ('"
                      + db.escape(nameEdit_->text().trimmed().toStdString()) + "', '"
                      + db.escape(contactEdit_->text().trimmed().toStdString()) + "', '"
                      + db.escape(phoneEdit_->text().trimmed().toStdString()) + "', '"
                      + db.escape(emailEdit_->text().trimmed().toStdString()) + "', '"
                      + db.escape(addressEdit_->text().trimmed().toStdString()) + "', "
                      + std::to_string(userId) + ")";

    if (db.execute(sql) >= 0) {
        db.commit();
        clearForm();
        refreshTable();
        QMessageBox::information(this, "成功", "供应商添加成功\n登录账号: " + username);
    } else {
        db.rollback();
        QMessageBox::critical(this, "错误", "添加失败");
    }
}

void SupplierPage::onEdit() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要修改的供应商");
        return;
    }

    auto& db = DBManager::instance();

    // 如果有新密码，更新密码
    QString password = passwordEdit_->text();
    if (!password.isEmpty()) {
        if (password.length() < 4) {
            QMessageBox::warning(this, "提示", "密码至少4位");
            return;
        }

        // 查找关联的 user_id
        auto supResult = db.query("SELECT user_id FROM suppliers WHERE id=" + std::to_string(editingId_));
        if (!supResult.empty()) {
            int userId = rowInt(supResult[0], "user_id");
            if (userId > 0) {
                QString hash = sha256(password);
                db.execute("UPDATE users SET password='" + hash.toStdString()
                           + "' WHERE id=" + std::to_string(userId));
            }
        }
    }

    // 如果有新用户名，更新用户名
    QString username = usernameEdit_->text().trimmed();
    if (!username.isEmpty()) {
        auto supResult = db.query("SELECT user_id FROM suppliers WHERE id=" + std::to_string(editingId_));
        if (!supResult.empty()) {
            int userId = rowInt(supResult[0], "user_id");
            if (userId > 0) {
                // 检查重复
                auto check = db.query("SELECT id FROM users WHERE username='"
                                      + db.escape(username.toStdString()) + "' AND id!=" + std::to_string(userId));
                if (!check.empty()) {
                    QMessageBox::warning(this, "提示", "登录账号已存在，请更换");
                    return;
                }
                db.execute("UPDATE users SET username='" + db.escape(username.toStdString())
                           + "' WHERE id=" + std::to_string(userId));
            }
        }
    }

    std::string sql = "UPDATE suppliers SET name='"
                      + db.escape(nameEdit_->text().trimmed().toStdString()) + "', contact='"
                      + db.escape(contactEdit_->text().trimmed().toStdString()) + "', phone='"
                      + db.escape(phoneEdit_->text().trimmed().toStdString()) + "', email='"
                      + db.escape(emailEdit_->text().trimmed().toStdString()) + "', address='"
                      + db.escape(addressEdit_->text().trimmed().toStdString()) + "' WHERE id="
                      + std::to_string(editingId_);

    if (db.execute(sql) >= 0) {
        clearForm();
        refreshTable();
        QMessageBox::information(this, "成功", "供应商修改成功");
    } else {
        QMessageBox::critical(this, "错误", "修改失败");
    }
}

void SupplierPage::onDelete() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的供应商");
        return;
    }

    auto reply = QMessageBox::question(this, "确认", "确定要删除该供应商吗？关联商品的供应商将置空，登录账号也将删除。",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    auto& db = DBManager::instance();
    db.begin();

    // 先获取关联的 user_id，删除用户账号
    auto supResult = db.query("SELECT user_id FROM suppliers WHERE id=" + std::to_string(editingId_));
    if (!supResult.empty()) {
        int userId = rowInt(supResult[0], "user_id");
        if (userId > 0) {
            db.execute("DELETE FROM users WHERE id=" + std::to_string(userId));
        }
    }

    db.execute("UPDATE products SET supplier_id=NULL WHERE supplier_id=" + std::to_string(editingId_));
    db.execute("DELETE FROM suppliers WHERE id=" + std::to_string(editingId_));
    db.commit();

    clearForm();
    refreshTable();
    QMessageBox::information(this, "成功", "供应商删除成功");
}

void SupplierPage::clearForm() {
    editingId_ = 0;
    nameEdit_->clear();
    contactEdit_->clear();
    phoneEdit_->clear();
    emailEdit_->clear();
    addressEdit_->clear();
    usernameEdit_->clear();
    passwordEdit_->clear();
}
