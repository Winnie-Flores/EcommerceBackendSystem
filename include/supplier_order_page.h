#ifndef SUPPLIER_ORDER_PAGE_H
#define SUPPLIER_ORDER_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class SupplierOrderPage : public QWidget {
    Q_OBJECT
public:
    explicit SupplierOrderPage(int supplierId, QWidget* parent = nullptr);

public slots:
    void refreshOrderTable();
    void onSearchOrder();
    void onViewOrderDetail();

private:
    void setupUI();

    int supplierId_;
    int selectedOrderId_ = 0;

    QTableWidget* orderTable_;
    QLineEdit* orderSearchEdit_;
    QComboBox* statusFilter_;
    QPushButton* detailBtn_;
};

#endif
