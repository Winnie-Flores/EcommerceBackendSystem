#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>

class UserPage;
class ProductPage;
class SupplierPage;
class InventoryPage;
class OrderPage;
class ReportPage;
class ProfilePage;
class SupplierProductPage;
class SupplierOrderPage;
class SupplierAnalysisPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(int userId, int role, const QString& userName,
                        QWidget* parent = nullptr);

signals:
    void logoutRequested();

private slots:
    void onLogout();
    void refreshAvatar();

private:
    void setupUI();
    void switchPage(int index);

    int userId_;
    int role_;
    QString userName_;
    int supplierId_ = 0;  // 供应商角色时的 supplier id

    QWidget* navContainer_ = nullptr;
    QButtonGroup* navButtonGroup_ = nullptr;
    QStackedWidget* pageStack_;
    QLabel* avatarLabel_ = nullptr;

    // 管理员页面
    UserPage* userPage_ = nullptr;
    ProductPage* productPage_ = nullptr;
    SupplierPage* supplierPage_ = nullptr;
    InventoryPage* inventoryPage_ = nullptr;
    ReportPage* reportPage_ = nullptr;
    OrderPage* orderPage_ = nullptr;

    // 供应商页面
    SupplierProductPage* supplierProductPage_ = nullptr;
    SupplierOrderPage* supplierOrderPage_ = nullptr;
    SupplierAnalysisPage* supplierAnalysisPage_ = nullptr;

    // 通用页面
    ProfilePage* profilePage_ = nullptr;
};

#endif
