#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);

    int userId() const { return userId_; }
    int userRole() const { return userRole_; }
    QString userName() const { return userName_; }

private slots:
    void onLogin();
    void onRegister();

private:
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QLabel* statusLabel_;
    int userId_ = 0;
    int userRole_ = 0;
    QString userName_;
};

#endif
