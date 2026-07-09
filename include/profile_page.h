#ifndef PROFILE_PAGE_H
#define PROFILE_PAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ProfilePage : public QWidget {
    Q_OBJECT
public:
    explicit ProfilePage(int userId, int role, const QString& userName,
                         QWidget* parent = nullptr);
    void refresh();

signals:
    void logoutRequested();
    void profileUpdated();

private slots:
    void onUpdateProfile();
    void onLogout();
    void onChangeAvatar();

private:
    void setupUI();
    void showAvatarPreview(const QPixmap& pixmap);
    void showDefaultAvatar();

    int userId_;
    int role_;
    QString userName_;

    QLineEdit* usernameEdit_;
    QLineEdit* oldPasswordEdit_;
    QLineEdit* newPasswordEdit_;
    QLineEdit* confirmPasswordEdit_;
    QLabel* roleLabel_;
    QLabel* statusLabel_;
    QPushButton* saveBtn_;
    QPushButton* logoutBtn_;
    QPushButton* changeAvatarBtn_;
    QLabel* avatarPreview_;
    QString pendingAvatar_;  // 待保存的头像base64
};

#endif
