#include "login_dialog.h"
#include "dbmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
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

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("电商管理后台 - 登录");
    setFixedSize(400, 320);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(40, 30, 40, 30);

    // 标题
    auto* titleLabel = new QLabel("🛒 电商管理后台");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 表单
    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    usernameEdit_ = new QLineEdit();
    usernameEdit_->setPlaceholderText("请输入用户名");
    usernameEdit_->setMinimumHeight(32);
    formLayout->addRow("用户名:", usernameEdit_);

    passwordEdit_ = new QLineEdit();
    passwordEdit_->setPlaceholderText("请输入密码");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setMinimumHeight(32);
    formLayout->addRow("密  码:", passwordEdit_);

    mainLayout->addLayout(formLayout);

    // 状态标签
    statusLabel_ = new QLabel();
    statusLabel_->setStyleSheet("color: red; font-size: 12px;");
    statusLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel_);

    // 按钮
    auto* btnLayout = new QHBoxLayout();
    auto* loginBtn = new QPushButton("登 录");
    loginBtn->setMinimumHeight(36);
    loginBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; "
        "border-radius: 4px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #2471a3; }"
    );

    auto* registerBtn = new QPushButton("注 册");
    registerBtn->setMinimumHeight(36);
    registerBtn->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border: none; "
        "border-radius: 4px; font-size: 15px; font-weight: bold; }"
        "QPushButton:hover { background-color: #27ae60; }"
    );

    btnLayout->addWidget(loginBtn);
    btnLayout->addWidget(registerBtn);
    mainLayout->addLayout(btnLayout);

    // 提示
    auto* hintLabel = new QLabel("管理员 admin/admin123 | 用户 user1/user123 | 供应商 supplier1/supp123");
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet("color: #95a5a6; font-size: 11px; margin-top: 5px;");
    mainLayout->addWidget(hintLabel);

    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    connect(passwordEdit_, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);
}

void LoginDialog::onLogin() {
    QString username = usernameEdit_->text().trimmed();
    QString password = passwordEdit_->text();

    if (username.isEmpty() || password.isEmpty()) {
        statusLabel_->setText("请输入用户名和密码");
        return;
    }

    QString hash = sha256(password);
    auto& db = DBManager::instance();
    std::string sql = "SELECT id, username, real_name, role FROM users WHERE username='"
                      + db.escape(username.toStdString()) + "' AND password='"
                      + hash.toStdString() + "'";

    auto result = db.query(sql);
    if (result.empty()) {
        statusLabel_->setText("用户名或密码错误");
        return;
    }

    userId_ = rowInt(result[0], "id");
    userName_ = rowStr(result[0], "real_name");
    userRole_ = rowInt(result[0], "role");
    accept();
}

void LoginDialog::onRegister() {
    QString username = usernameEdit_->text().trimmed();
    QString password = passwordEdit_->text();

    if (username.isEmpty() || password.isEmpty()) {
        statusLabel_->setText("请输入用户名和密码");
        return;
    }
    if (password.length() < 4) {
        statusLabel_->setText("密码至少4位");
        return;
    }

    auto& db = DBManager::instance();

    // 检查用户是否存在
    auto check = db.query("SELECT id FROM users WHERE username='"
                          + db.escape(username.toStdString()) + "'");
    if (!check.empty()) {
        statusLabel_->setText("用户名已存在");
        return;
    }

    QString hash = sha256(password);
    std::string sql = "INSERT INTO users (username, password, real_name, role) VALUES ('"
                      + db.escape(username.toStdString()) + "', '"
                      + hash.toStdString() + "', '"
                      + db.escape(username.toStdString()) + "', 0)";

    long long id = db.insert(sql);
    if (id > 0) {
        QMessageBox::information(this, "注册成功", "注册成功，请登录！");
        statusLabel_->setText("");
    } else {
        statusLabel_->setText("注册失败，请重试");
    }
}
