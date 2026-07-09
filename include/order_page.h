#ifndef ORDER_PAGE_H
#define ORDER_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QTextEdit>

class OrderPage : public QWidget {
    Q_OBJECT
public:
    explicit OrderPage(int currentUserId, int currentRole, QWidget* parent = nullptr);

public slots:
    void refreshOrderTable();
    void onSearchOrder();
    void onOrderSelect();
    void onUpdateStatus();
    void onAddItem();
    void onRemoveItem();
    void onCreateOrder();
    void onPayOrder();
    void onReview();
    void onViewDetail();
    void onEditOrder();
    void onDeleteOrder();

private:
    void setupUI();
    void loadProducts();
    void clearCart();

    int currentUserId_;
    int currentRole_;
    int selectedOrderId_ = 0;

    // 订单列表
    QTableWidget* orderTable_;
    QLineEdit* orderSearchEdit_;
    QComboBox* statusFilter_;
    QPushButton* updateStatusBtn_;

    // 评价 & 管理
    QPushButton* reviewBtn_;
    QPushButton* detailBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;

    // 新建订单（购物车）
    QComboBox* productCombo_;
    QSpinBox* qtySpin_;
    QPushButton* addItemBtn_;
    QPushButton* removeItemBtn_;
    QTableWidget* cartTable_;
    QDoubleSpinBox* totalAmount_;
    QLineEdit* orderAddressEdit_;
    QTextEdit* orderRemarkEdit_;
    QPushButton* createOrderBtn_;
    QPushButton* payOrderBtn_;
};

#endif
