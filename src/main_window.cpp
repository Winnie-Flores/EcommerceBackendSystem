#include "main_window.h"
#include "user_page.h"
#include "product_page.h"
#include "supplier_page.h"
#include "inventory_page.h"
#include "order_page.h"
#include "report_page.h"
#include "profile_page.h"
#include "supplier_product_page.h"
#include "supplier_order_page.h"
#include "supplier_analysis_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QBuffer>
#include <QPushButton>
#include <QButtonGroup>

MainWindow::MainWindow(int userId, int role, const QString& userName,
                       QWidget* parent)
    : QMainWindow(parent), userId_(userId), role_(role), userName_(userName) {

    // 如果是供应商，查询 supplier_id
    if (role_ == 2) {
        auto& db = DBManager::instance();
        auto result = db.query("SELECT id FROM suppliers WHERE user_id=" + std::to_string(userId_));
        if (!result.empty()) {
            supplierId_ = rowInt(result[0], "id");
        }
    }

    setupUI();
}

void MainWindow::setupUI() {
    setWindowTitle("电商管理后台");
    resize(1200, 750);

    auto* centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ===== 左侧导航 =====
    auto* leftPanel = new QWidget();
    leftPanel->setFixedWidth(180);
    leftPanel->setStyleSheet("background-color: #2c3e50;");
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 用户信息区
    auto* userInfoWidget = new QWidget();
    userInfoWidget->setStyleSheet("background-color: #1a252f; padding: 15px;");
    auto* userInfoLayout = new QVBoxLayout(userInfoWidget);

    QString roleText;
    if (role_ == 1) roleText = "管理员";
    else if (role_ == 2) roleText = "供应商";
    else roleText = "普通用户";

    avatarLabel_ = new QLabel();
    avatarLabel_->setFixedSize(64, 64);
    avatarLabel_->setAlignment(Qt::AlignCenter);

    auto* nameLabel = new QLabel(userName_);
    nameLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    nameLabel->setAlignment(Qt::AlignCenter);

    auto* roleLabel = new QLabel(roleText);
    roleLabel->setStyleSheet("color: #95a5a6; font-size: 12px;");
    roleLabel->setAlignment(Qt::AlignCenter);

    userInfoLayout->addWidget(avatarLabel_, 0, Qt::AlignCenter);
    userInfoLayout->addWidget(nameLabel);
    userInfoLayout->addWidget(roleLabel);
    leftLayout->addWidget(userInfoWidget);

    // 导航菜单 — 使用按钮组，无滚动，全部可见
    navContainer_ = new QWidget();
    navContainer_->setStyleSheet("background-color: #2c3e50;");
    auto* navLayout = new QVBoxLayout(navContainer_);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(0);

    navButtonGroup_ = new QButtonGroup(this);
    navButtonGroup_->setExclusive(true);

    QStringList menuItems;
    if (role_ == 1) {
        menuItems << "👥 用户管理" << "📦 商品管理" << "🏭 供应商管理"
                  << "📊 库存管理" << "📈 数据报表" << "🛒 订单管理";
    } else if (role_ == 2) {
        menuItems << "🏪 我的商品" << "📋 我的订单" << "📊 数据分析";
    } else {
        menuItems << "🛒 订单管理";
    }
    menuItems << "👤 个人信息";

    QString btnStyle =
        "QPushButton { background: transparent; color: white; font-size: 14px; "
        "border: none; border-bottom: 1px solid #34495e; "
        "padding: 12px 10px; text-align: center; }"
        "QPushButton:hover { background-color: #34495e; }"
        "QPushButton:checked { background-color: #3498db; }";

    for (int i = 0; i < menuItems.size(); ++i) {
        auto* btn = new QPushButton(menuItems[i]);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(btnStyle);
        navButtonGroup_->addButton(btn, i);
        navLayout->addWidget(btn);
    }

    navLayout->addStretch();
    leftLayout->addWidget(navContainer_);

    // 底部
    leftLayout->addStretch();
    auto* verLabel = new QLabel("v1.0");
    verLabel->setStyleSheet("color: #7f8c8d; font-size: 11px; padding: 10px;");
    verLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(verLabel);

    mainLayout->addWidget(leftPanel);

    // ===== 右侧页面区 =====
    pageStack_ = new QStackedWidget();
    pageStack_->setStyleSheet("background-color: #ecf0f1;");

    if (role_ == 1) {
        // 管理员
        userPage_ = new UserPage(userId_, role_);
        pageStack_->addWidget(userPage_);    // index 0

        productPage_ = new ProductPage();
        pageStack_->addWidget(productPage_);  // index 1

        supplierPage_ = new SupplierPage();
        pageStack_->addWidget(supplierPage_); // index 2

        inventoryPage_ = new InventoryPage();
        pageStack_->addWidget(inventoryPage_); // index 3

        reportPage_ = new ReportPage();
        pageStack_->addWidget(reportPage_);    // index 4

        orderPage_ = new OrderPage(userId_, role_);
        pageStack_->addWidget(orderPage_);     // index 5

        profilePage_ = new ProfilePage(userId_, role_, userName_);
        pageStack_->addWidget(profilePage_);   // index 6
    } else if (role_ == 2) {
        // 供应商
        supplierProductPage_ = new SupplierProductPage(supplierId_);
        pageStack_->addWidget(supplierProductPage_);  // index 0

        supplierOrderPage_ = new SupplierOrderPage(supplierId_);
        pageStack_->addWidget(supplierOrderPage_);     // index 1

        supplierAnalysisPage_ = new SupplierAnalysisPage(supplierId_);
        pageStack_->addWidget(supplierAnalysisPage_);   // index 2

        profilePage_ = new ProfilePage(userId_, role_, userName_);
        pageStack_->addWidget(profilePage_);            // index 3
    } else {
        // 普通用户
        orderPage_ = new OrderPage(userId_, role_);
        pageStack_->addWidget(orderPage_);     // index 0

        profilePage_ = new ProfilePage(userId_, role_, userName_);
        pageStack_->addWidget(profilePage_);   // index 1
    }

    mainLayout->addWidget(pageStack_);

    // 导航点击切换
    connect(navButtonGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &MainWindow::switchPage);
    navButtonGroup_->button(0)->setChecked(true);

    // 个人信息页退出登录信号
    if (profilePage_) {
        connect(profilePage_, &ProfilePage::logoutRequested, this, &MainWindow::onLogout);
        connect(profilePage_, &ProfilePage::profileUpdated, this, &MainWindow::refreshAvatar);
    }

    pageStack_->setCurrentIndex(0);

    // 加载头像
    refreshAvatar();
}

void MainWindow::switchPage(int index) {
    pageStack_->setCurrentIndex(index);
    // 切换时刷新对应页面
    QWidget* w = pageStack_->currentWidget();
    if (auto* up = qobject_cast<UserPage*>(w)) up->refreshTable();
    else if (auto* pp = qobject_cast<ProductPage*>(w)) pp->refreshTable();
    else if (auto* sp = qobject_cast<SupplierPage*>(w)) sp->refreshTable();
    else if (auto* ip = qobject_cast<InventoryPage*>(w)) { ip->refreshProductTable(); ip->refreshLogTable(); }
    else if (auto* rp = qobject_cast<ReportPage*>(w)) rp->refresh();
    else if (auto* op = qobject_cast<OrderPage*>(w)) op->refreshOrderTable();
    else if (auto* prp = qobject_cast<ProfilePage*>(w)) { prp->refresh(); refreshAvatar(); }
    else if (auto* spp = qobject_cast<SupplierProductPage*>(w)) { spp->refreshTable(); spp->checkStockAlert(); }
    else if (auto* sop = qobject_cast<SupplierOrderPage*>(w)) sop->refreshOrderTable();
    else if (auto* sap = qobject_cast<SupplierAnalysisPage*>(w)) sap->refresh();
}

void MainWindow::onLogout() {
    emit logoutRequested();
    close();
}

void MainWindow::refreshAvatar() {
    auto& db = DBManager::instance();
    auto result = db.query("SELECT avatar FROM users WHERE id=" + std::to_string(userId_));
    if (result.empty() || rowStr(result[0], "avatar").isEmpty()) {
        // 无头像，显示默认图标
        avatarLabel_->setPixmap(QPixmap());
        avatarLabel_->setText("👤");
        avatarLabel_->setStyleSheet("font-size: 32px; color: white;");
        return;
    }

    QString base64 = rowStr(result[0], "avatar");
    QByteArray data = QByteArray::fromBase64(base64.toLatin1());
    QPixmap pixmap;
    if (!pixmap.loadFromData(data)) {
        avatarLabel_->setText("👤");
        avatarLabel_->setStyleSheet("font-size: 32px; color: white;");
        return;
    }

    // 圆形裁剪
    int size = qMin(pixmap.width(), pixmap.height());
    QPixmap scaled = pixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap circle(64, 64);
    circle.fill(Qt::transparent);
    QPainter painter(&circle);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(1, 1, 62, 62);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, 64, 64, scaled);
    painter.end();

    avatarLabel_->setPixmap(circle);
    avatarLabel_->setStyleSheet("border: none;");
}
