#ifndef INVENTORY_PAGE_H
#define INVENTORY_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

class InventoryPage : public QWidget {
    Q_OBJECT
public:
    explicit InventoryPage(QWidget* parent = nullptr);

public slots:
    void refreshProductTable();
    void refreshLogTable();
    void onStockIn();
    void onStockOut();
    void onSearch();

private:
    void setupUI();
    void loadProducts();

    QTableWidget* productTable_;
    QTableWidget* logTable_;
    QLineEdit* searchEdit_;
    QComboBox* productCombo_;
    QSpinBox* quantitySpin_;
    QLineEdit* remarkEdit_;
    QPushButton* stockInBtn_;
    QPushButton* stockOutBtn_;
    QLabel* alertLabel_;
};

#endif
