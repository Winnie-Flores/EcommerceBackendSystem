#ifndef USER_PAGE_H
#define USER_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class UserPage : public QWidget {
    Q_OBJECT
public:
    explicit UserPage(int currentUserId, int currentRole, QWidget* parent = nullptr);

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

    int currentUserId_;
    int currentRole_;
    int editingId_ = 0;

    QTableWidget* table_;
    QLineEdit* searchEdit_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QLineEdit* realNameEdit_;
    QLineEdit* phoneEdit_;
    QLineEdit* emailEdit_;
    QLineEdit* addressEdit_;
    QComboBox* roleCombo_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
    QLabel* infoLabel_;
};

#endif
