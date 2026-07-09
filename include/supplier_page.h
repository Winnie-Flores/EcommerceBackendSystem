#ifndef SUPPLIER_PAGE_H
#define SUPPLIER_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>

class SupplierPage : public QWidget {
    Q_OBJECT
public:
    explicit SupplierPage(QWidget* parent = nullptr);

public slots:
    void refreshTable();
    void onSearch();
    void onAdd();
    void onEdit();
    void onDelete();
    void onTableSelect();

private:
    void clearForm();
    void setupUI();

    int editingId_ = 0;

    QTableWidget* table_;
    QLineEdit* searchEdit_;
    QLineEdit* nameEdit_;
    QLineEdit* contactEdit_;
    QLineEdit* phoneEdit_;
    QLineEdit* emailEdit_;
    QLineEdit* addressEdit_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};

#endif
