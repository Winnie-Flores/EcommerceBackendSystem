#ifndef SUPPLIER_PRODUCT_PAGE_H
#define SUPPLIER_PRODUCT_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>

class SupplierProductPage : public QWidget {
    Q_OBJECT
public:
    explicit SupplierProductPage(int supplierId, QWidget* parent = nullptr);

public slots:
    void refreshTable();
    void checkStockAlert();

signals:
    void stockAlert(const QString& productName, int stock, int alertStock);

private slots:
    void onSearch();
    void onAdd();
    void onEdit();
    void onTableSelect();
    void onToggleStatus();
    void onViewReviews();

private:
    void clearForm();
    void loadCategories();
    void setupUI();

    int supplierId_;
    int editingId_ = 0;

    QTableWidget* table_;
    QLineEdit* searchEdit_;
    QComboBox* categoryFilter_;
    QLineEdit* nameEdit_;
    QComboBox* categoryCombo_;
    QDoubleSpinBox* priceEdit_;
    QDoubleSpinBox* costEdit_;
    QSpinBox* stockEdit_;
    QSpinBox* alertStockEdit_;
    QTextEdit* descEdit_;
    QLineEdit* imageUrlEdit_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* toggleBtn_;
    QPushButton* reviewBtn_;
    QLabel* alertLabel_;
};

#endif
