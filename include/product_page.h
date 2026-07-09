#ifndef PRODUCT_PAGE_H
#define PRODUCT_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>

class ProductPage : public QWidget {
    Q_OBJECT
public:
    explicit ProductPage(QWidget* parent = nullptr);

public slots:
    void refreshTable();
    void onSearch();
    void onAdd();
    void onEdit();
    void onDelete();
    void onTableSelect();
    void onToggleStatus();
    void onViewReviews();

private:
    void clearForm();
    void loadCategories();
    void loadSuppliers();
    void setupUI();

    int editingId_ = 0;

    QTableWidget* table_;
    QLineEdit* searchEdit_;
    QComboBox* categoryFilter_;
    QLineEdit* nameEdit_;
    QComboBox* categoryCombo_;
    QComboBox* supplierCombo_;
    QDoubleSpinBox* priceEdit_;
    QDoubleSpinBox* costEdit_;
    QSpinBox* stockEdit_;
    QSpinBox* alertStockEdit_;
    QTextEdit* descEdit_;
    QLineEdit* imageUrlEdit_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
    QPushButton* toggleBtn_;
    QPushButton* reviewBtn_;
};

#endif
