#include "profile_page.h"
#include "dbmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QBuffer>
#include <QPainter>
#include <QPainterPath>
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

ProfilePage::ProfilePage(int userId, int role, const QString& userName,
                         QWidget* parent)
    : QWidget(parent), userId_(userId), role_(role), userName_(userName) {
    setupUI();
}

void ProfilePage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 20, 30, 20);

    // 标题
    auto* titleLabel = new QLabel("个人信息管理");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 头像区
    auto* avatarGroup = new QGroupBox("个人头像");
    avatarGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 14px; border: 1px solid #bdc3c7; "
        "border-radius: 4px; margin-top: 12px; padding-top: 18px; background: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
    );
    auto* avatarLayout = new QHBoxLayout(avatarGroup);
    avatarLayout->setContentsMargins(20, 15, 20, 15);
    avatarLayout->setSpacing(20);

    avatarPreview_ = new QLabel();
    avatarPreview_->setFixedSize(100, 100);
    avatarPreview_->setAlignment(Qt::AlignCenter);
    avatarPreview_->setStyleSheet(
        "QLabel { border: 2px dashed #bdc3c7; border-radius: 50px; background: #f5f6fa; }"
    );

    changeAvatarBtn_ = new QPushButton("选择头像");
    changeAvatarBtn_->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; "
        "border-radius: 4px; padding: 8px 20px; font-size: 13px; }"
        "QPushButton:hover { background-color: #2980b9; }"
    );
    changeAvatarBtn_->setFixedHeight(36);

    auto* avatarRightLayout = new QVBoxLayout();
    avatarRightLayout->addWidget(changeAvatarBtn_);
    avatarRightLayout->addStretch();

    avatarLayout->addWidget(avatarPreview_);
    avatarLayout->addLayout(avatarRightLayout);
    avatarLayout->addStretch();

    mainLayout->addWidget(avatarGroup);

    // 账号信息区
    auto* accountGroup = new QGroupBox("账号信息");
    accountGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 14px; border: 1px solid #bdc3c7; "
        "border-radius: 4px; margin-top: 12px; padding-top: 18px; background: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
    );
    auto* accountLayout = new QFormLayout(accountGroup);
    accountLayout->setSpacing(12);
    accountLayout->setContentsMargins(20, 20, 20, 20);

    // 用户名
    usernameEdit_ = new QLineEdit(userName_);
    usernameEdit_->setMinimumWidth(250);
    accountLayout->addRow("用户名:", usernameEdit_);

    // 角色（只读显示）
    QString roleText;
    if (role_ == 1) roleText = "管理员";
    else if (role_ == 2) roleText = "供应商";
    else roleText = "普通用户";
    roleLabel_ = new QLabel(roleText);
    roleLabel_->setStyleSheet("font-weight: bold; color: #3498db;");
    accountLayout->addRow("角色:", roleLabel_);

    mainLayout->addWidget(accountGroup);

    // 修改密码区
    auto* pwdGroup = new QGroupBox("修改密码");
    pwdGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 14px; border: 1px solid #bdc3c7; "
        "border-radius: 4px; margin-top: 12px; padding-top: 18px; background: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
    );
    auto* pwdLayout = new QFormLayout(pwdGroup);
    pwdLayout->setSpacing(12);
    pwdLayout->setContentsMargins(20, 20, 20, 20);

    oldPasswordEdit_ = new QLineEdit();
    oldPasswordEdit_->setEchoMode(QLineEdit::Password);
    oldPasswordEdit_->setPlaceholderText("请输入原密码");
    oldPasswordEdit_->setMinimumWidth(250);
    pwdLayout->addRow("原密码:", oldPasswordEdit_);

    newPasswordEdit_ = new QLineEdit();
    newPasswordEdit_->setEchoMode(QLineEdit::Password);
    newPasswordEdit_->setPlaceholderText("请输入新密码（留空则不修改）");
    newPasswordEdit_->setMinimumWidth(250);
    pwdLayout->addRow("新密码:", newPasswordEdit_);

    confirmPasswordEdit_ = new QLineEdit();
    confirmPasswordEdit_->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit_->setPlaceholderText("再次输入新密码");
    confirmPasswordEdit_->setMinimumWidth(250);
    pwdLayout->addRow("确认密码:", confirmPasswordEdit_);

    mainLayout->addWidget(pwdGroup);

    // 状态标签
    statusLabel_ = new QLabel();
    statusLabel_->setStyleSheet("color: #7f8c8d; font-size: 12px;");
    mainLayout->addWidget(statusLabel_);

    // 按钮区
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    saveBtn_ = new QPushButton("💾 保存修改");
    saveBtn_->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; "
        "border-radius: 4px; padding: 10px 24px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #2471a3; }"
    );
    btnLayout->addWidget(saveBtn_);

    btnLayout->addStretch();

    logoutBtn_ = new QPushButton("🚪 退出登录");
    logoutBtn_->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; border: none; "
        "border-radius: 4px; padding: 10px 24px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #c0392b; }"
        "QPushButton:pressed { background-color: #a93226; }"
    );
    btnLayout->addWidget(logoutBtn_);

    mainLayout->addLayout(btnLayout);
    mainLayout->addStretch();

    // 连接信号
    connect(saveBtn_, &QPushButton::clicked, this, &ProfilePage::onUpdateProfile);
    connect(logoutBtn_, &QPushButton::clicked, this, &ProfilePage::onLogout);
    connect(changeAvatarBtn_, &QPushButton::clicked, this, &ProfilePage::onChangeAvatar);
}

void ProfilePage::refresh() {
    // 重新从数据库加载用户信息
    auto& db = DBManager::instance();
    std::string sql = "SELECT username, avatar FROM users WHERE id = " + std::to_string(userId_);
    auto result = db.query(sql);
    if (!result.empty()) {
        userName_ = rowStr(result[0], "username");
        usernameEdit_->setText(userName_);

        // 加载头像
        QString avatarBase64 = rowStr(result[0], "avatar");
        pendingAvatar_ = "";  // 清除待保存的
        if (!avatarBase64.isEmpty()) {
            QByteArray data = QByteArray::fromBase64(avatarBase64.toLatin1());
            QPixmap pixmap;
            if (pixmap.loadFromData(data)) {
                showAvatarPreview(pixmap);
            } else {
                showDefaultAvatar();
            }
        } else {
            showDefaultAvatar();
        }
    }
    oldPasswordEdit_->clear();
    newPasswordEdit_->clear();
    confirmPasswordEdit_->clear();
    statusLabel_->clear();
}

void ProfilePage::onUpdateProfile() {
    QString newUsername = usernameEdit_->text().trimmed();
    QString oldPwd = oldPasswordEdit_->text();
    QString newPwd = newPasswordEdit_->text();
    QString confirmPwd = confirmPasswordEdit_->text();

    if (newUsername.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名不能为空！");
        return;
    }

    auto& db = DBManager::instance();

    // 如果修改了用户名，检查是否重复
    if (newUsername != userName_) {
        std::string checkSql = "SELECT id FROM users WHERE username = '"
                               + db.escape(newUsername.toStdString()) + "' AND id != " + std::to_string(userId_);
        auto checkResult = db.query(checkSql);
        if (!checkResult.empty()) {
            QMessageBox::warning(this, "提示", "该用户名已被占用，请换一个！");
            return;
        }
    }

    // 如果要修改密码，验证原密码
    if (!newPwd.isEmpty() || !oldPwd.isEmpty()) {
        if (oldPwd.isEmpty()) {
            QMessageBox::warning(this, "提示", "修改密码需要输入原密码！");
            return;
        }
        if (newPwd.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入新密码！");
            return;
        }
        if (newPwd != confirmPwd) {
            QMessageBox::warning(this, "提示", "两次输入的新密码不一致！");
            return;
        }
        if (newPwd.length() < 4) {
            QMessageBox::warning(this, "提示", "新密码长度不能少于4位！");
            return;
        }

        // 验证原密码（数据库存的是 SHA-256 哈希）
        QString oldHash = sha256(oldPwd);
        std::string verifySql = "SELECT id FROM users WHERE id = " + std::to_string(userId_)
                                + " AND password = '" + db.escape(oldHash.toStdString()) + "'";
        auto verifyResult = db.query(verifySql);
        if (verifyResult.empty()) {
            QMessageBox::warning(this, "提示", "原密码错误！");
            return;
        }

        // 更新密码（存储 SHA-256 哈希）
        QString newHash = sha256(newPwd);
        std::string updatePwdSql = "UPDATE users SET password = '"
                                   + db.escape(newHash.toStdString()) + "' WHERE id = " + std::to_string(userId_);
        db.execute(updatePwdSql);
    }

    // 更新用户名
    if (newUsername != userName_) {
        std::string updateNameSql = "UPDATE users SET username = '"
                                    + db.escape(newUsername.toStdString()) + "' WHERE id = " + std::to_string(userId_);
        db.execute(updateNameSql);
        userName_ = newUsername;
    }

    // 更新头像
    if (!pendingAvatar_.isEmpty()) {
        std::string updateAvatarSql = "UPDATE users SET avatar = '"
                                      + db.escape(pendingAvatar_.toStdString()) + "' WHERE id = " + std::to_string(userId_);
        db.execute(updateAvatarSql);
        pendingAvatar_ = "";
    }

    statusLabel_->setStyleSheet("color: #27ae60; font-size: 13px; font-weight: bold;");
    statusLabel_->setText("个人信息修改成功！");

    emit profileUpdated();

    // 清空密码字段
    oldPasswordEdit_->clear();
    newPasswordEdit_->clear();
    confirmPasswordEdit_->clear();
}

void ProfilePage::onLogout() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认退出", "确定要退出登录吗？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        emit logoutRequested();
    }
}

void ProfilePage::onChangeAvatar() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择头像图片", "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (filePath.isEmpty()) return;

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载所选图片！");
        return;
    }

    // 缩放到合适大小
    QPixmap scaled = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 转为 base64
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    scaled.save(&buffer, "PNG");
    pendingAvatar_ = QString::fromLatin1(byteArray.toBase64());

    showAvatarPreview(scaled);
}

void ProfilePage::showAvatarPreview(const QPixmap& pixmap) {
    // 圆形裁剪预览
    int size = qMin(pixmap.width(), pixmap.height());
    QPixmap scaled = pixmap.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap circle(100, 100);
    circle.fill(Qt::transparent);
    QPainter painter(&circle);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(2, 2, 96, 96);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, 100, 100, scaled);
    painter.end();

    avatarPreview_->setPixmap(circle);
    avatarPreview_->setStyleSheet("QLabel { border: none; }");
}

void ProfilePage::showDefaultAvatar() {
    avatarPreview_->setPixmap(QPixmap());
    avatarPreview_->setText("无头像");
    avatarPreview_->setStyleSheet(
        "QLabel { border: 2px dashed #bdc3c7; border-radius: 50px; "
        "background: #f5f6fa; color: #95a5a6; font-size: 13px; }"
    );
}
