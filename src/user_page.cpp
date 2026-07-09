#include "user_page.h"
#include "dbmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <openssl/sha.h>

static QString sha256(const QString& str) {
    QByteArray data = str.toUtf8();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.constData()), data.size(), hash);
    QString result;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        result.append(QString::asprintf("%02x", hash[i]));
    return result;
}

UserPage::UserPage(int currentUserId, int currentRole, QWidget* parent)
    : QWidget(parent), currentUserId_(currentUserId), currentRole_(currentRole) {
    setupUI();
    refreshTable();
}

void UserPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("用户管理");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 搜索栏
    auto* searchLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("搜索用户名或真实姓名...");
    searchEdit_->setMinimumHeight(30);
    auto* searchBtn = new QPushButton("搜索");
    searchBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 6px 16px; border-radius: 3px; } QPushButton:hover { background: #2980b9; }");
    searchLayout->addWidget(searchEdit_);
    searchLayout->addWidget(searchBtn);
    mainLayout->addLayout(searchLayout);

    // 表格
    table_ = new QTableWidget();
    table_->setColumnCount(9);
    table_->setHorizontalHeaderLabels({"ID", "用户名", "真实姓名", "电话", "邮箱", "地址", "角色", "余额", "注册时间"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->setStyleSheet("QTableWidget { background: white; gridline-color: #ddd; } "
                          "QTableWidget::item:selected { background: #3498db; color: white; }");
    mainLayout->addWidget(table_);

    // 编辑表单
    auto* formGroup = new QGroupBox("用户信息");
    formGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 10px; padding-top: 15px; }");
    auto* formLayout = new QFormLayout(formGroup);

    usernameEdit_ = new QLineEdit(); usernameEdit_->setMinimumHeight(28);
    passwordEdit_ = new QLineEdit(); passwordEdit_->setPlaceholderText("留空则不修改密码");
    passwordEdit_->setEchoMode(QLineEdit::Password); passwordEdit_->setMinimumHeight(28);
    realNameEdit_ = new QLineEdit(); realNameEdit_->setMinimumHeight(28);
    phoneEdit_ = new QLineEdit(); phoneEdit_->setMinimumHeight(28);
    emailEdit_ = new QLineEdit(); emailEdit_->setMinimumHeight(28);
    addressEdit_ = new QLineEdit(); addressEdit_->setMinimumHeight(28);
    roleCombo_ = new QComboBox();
    roleCombo_->addItem("普通用户", 0);
    roleCombo_->addItem("管理员", 1);
    roleCombo_->addItem("供应商", 2);

    formLayout->addRow("用户名:", usernameEdit_);
    formLayout->addRow("密码:", passwordEdit_);
    formLayout->addRow("真实姓名:", realNameEdit_);
    formLayout->addRow("电话:", phoneEdit_);
    formLayout->addRow("邮箱:", emailEdit_);
    formLayout->addRow("地址:", addressEdit_);
    formLayout->addRow("角色:", roleCombo_);

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

    infoLabel_ = new QLabel();
    infoLabel_->setStyleSheet("color: #7f8c8d;");
    btnLayout->addWidget(infoLabel_);

    mainLayout->addLayout(btnLayout);

    connect(searchBtn, &QPushButton::clicked, this, &UserPage::onSearch);
    connect(addBtn_, &QPushButton::clicked, this, &UserPage::onAdd);
    connect(editBtn_, &QPushButton::clicked, this, &UserPage::onEdit);
    connect(deleteBtn_, &QPushButton::clicked, this, &UserPage::onDelete);
    connect(table_, &QTableWidget::itemSelectionChanged, this, &UserPage::onTableSelect);
}

void UserPage::refreshTable() {
    auto& db = DBManager::instance();
    std::string sql = "SELECT id, username, real_name, phone, email, address, role, balance, "
                      "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i') as created_at FROM users ORDER BY id";
    auto result = db.query(sql);

    table_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        table_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        table_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "username")));
        table_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "real_name")));
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(rowStr(result[i], "phone")));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(rowStr(result[i], "email")));
        table_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "address")));
        int r = rowInt(result[i], "role");
        QString role = (r == 1) ? "管理员" : (r == 2) ? "供应商" : "普通用户";
        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(role));
        table_->setItem(static_cast<int>(i), 7, new QTableWidgetItem(QString::number(rowDouble(result[i], "balance"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 8, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
    infoLabel_->setText(QString("共 %1 条记录").arg(result.size()));
}

void UserPage::onSearch() {
    QString keyword = searchEdit_->text().trimmed();
    if (keyword.isEmpty()) { refreshTable(); return; }

    auto& db = DBManager::instance();
    std::string kw = db.escape(keyword.toStdString());
    std::string sql = "SELECT id, username, real_name, phone, email, address, role, balance, "
                      "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i') as created_at FROM users "
                      "WHERE username LIKE '%" + kw + "%' OR real_name LIKE '%" + kw + "%' ORDER BY id";
    auto result = db.query(sql);

    table_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        table_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        table_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "username")));
        table_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "real_name")));
        table_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(rowStr(result[i], "phone")));
        table_->setItem(static_cast<int>(i), 4, new QTableWidgetItem(rowStr(result[i], "email")));
        table_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "address")));
        int r = rowInt(result[i], "role");
        QString role = (r == 1) ? "管理员" : (r == 2) ? "供应商" : "普通用户";
        table_->setItem(static_cast<int>(i), 6, new QTableWidgetItem(role));
        table_->setItem(static_cast<int>(i), 7, new QTableWidgetItem(QString::number(rowDouble(result[i], "balance"), 'f', 2)));
        table_->setItem(static_cast<int>(i), 8, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
    infoLabel_->setText(QString("搜索到 %1 条记录").arg(result.size()));
}

void UserPage::onTableSelect() {
    auto items = table_->selectedItems();
    if (items.isEmpty()) return;
    int row = items.first()->row();

    editingId_ = table_->item(row, 0)->text().toInt();
    usernameEdit_->setText(table_->item(row, 1)->text());
    passwordEdit_->clear();
    realNameEdit_->setText(table_->item(row, 2)->text());
    phoneEdit_->setText(table_->item(row, 3)->text());
    emailEdit_->setText(table_->item(row, 4)->text());
    addressEdit_->setText(table_->item(row, 5)->text());
    QString roleText = table_->item(row, 6)->text();
    roleCombo_->setCurrentIndex(roleText == "管理员" ? 1 : (roleText == "供应商" ? 2 : 0));
}

void UserPage::onAdd() {
    QString username = usernameEdit_->text().trimmed();
    QString password = passwordEdit_->text();
    QString realName = realNameEdit_->text().trimmed();

    if (username.isEmpty() || password.isEmpty() || realName.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名、密码、真实姓名为必填项");
        return;
    }

    auto& db = DBManager::instance();
    auto check = db.query("SELECT id FROM users WHERE username='"
                          + db.escape(username.toStdString()) + "'");
    if (!check.empty()) {
        QMessageBox::warning(this, "提示", "用户名已存在");
        return;
    }

    std::string sql = "INSERT INTO users (username, password, real_name, phone, email, address, role) VALUES ('"
                      + db.escape(username.toStdString()) + "', '"
                      + sha256(password).toStdString() + "', '"
                      + db.escape(realName.toStdString()) + "', '"
                      + db.escape(phoneEdit_->text().trimmed().toStdString()) + "', '"
                      + db.escape(emailEdit_->text().trimmed().toStdString()) + "', '"
                      + db.escape(addressEdit_->text().trimmed().toStdString()) + "', "
                      + std::to_string(roleCombo_->currentData().toInt()) + ")";

    if (db.execute(sql) >= 0) {
        clearForm();
        refreshTable();
        QMessageBox::information(this, "成功", "用户添加成功");
    } else {
        QMessageBox::critical(this, "错误", "添加失败");
    }
}

void UserPage::onEdit() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要修改的用户");
        return;
    }

    auto& db = DBManager::instance();
    std::string sql = "UPDATE users SET real_name='"
                      + db.escape(realNameEdit_->text().trimmed().toStdString()) + "', phone='"
                      + db.escape(phoneEdit_->text().trimmed().toStdString()) + "', email='"
                      + db.escape(emailEdit_->text().trimmed().toStdString()) + "', address='"
                      + db.escape(addressEdit_->text().trimmed().toStdString()) + "', role="
                      + std::to_string(roleCombo_->currentData().toInt());

    QString newPass = passwordEdit_->text();
    if (!newPass.isEmpty()) {
        sql += ", password='" + sha256(newPass).toStdString() + "'";
    }
    sql += " WHERE id=" + std::to_string(editingId_);

    if (db.execute(sql) >= 0) {
        clearForm();
        refreshTable();
        QMessageBox::information(this, "成功", "用户修改成功");
    } else {
        QMessageBox::critical(this, "错误", "修改失败");
    }
}

void UserPage::onDelete() {
    if (editingId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的用户");
        return;
    }
    if (editingId_ == currentUserId_) {
        QMessageBox::warning(this, "提示", "不能删除自己");
        return;
    }

    auto reply = QMessageBox::question(this, "确认", "确定要删除该用户吗？",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    auto& db = DBManager::instance();
    if (db.execute("DELETE FROM users WHERE id=" + std::to_string(editingId_)) >= 0) {
        clearForm();
        refreshTable();
        QMessageBox::information(this, "成功", "用户删除成功");
    } else {
        QMessageBox::critical(this, "错误", "删除失败（该用户可能存在关联订单）");
    }
}

void UserPage::clearForm() {
    editingId_ = 0;
    usernameEdit_->clear();
    passwordEdit_->clear();
    realNameEdit_->clear();
    phoneEdit_->clear();
    emailEdit_->clear();
    addressEdit_->clear();
    roleCombo_->setCurrentIndex(0);
}
