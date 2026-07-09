#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QEventLoop>
#include "dbmanager.h"
#include "login_dialog.h"
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    app.setOrganizationName("ECommerce");
    app.setApplicationName("ECommerce");

    // 全局样式
    app.setStyleSheet(
        "QMainWindow { background-color: #ecf0f1; }"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { "
        "  border: 1px solid #bdc3c7; border-radius: 3px; padding: 4px 8px; background: white; }"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { "
        "  border-color: #3498db; }"
        "QGroupBox { background: white; border-radius: 4px; padding: 15px; margin-top: 10px; }"
        "QTableWidget { gridline-color: #ecf0f1; }"
        "QHeaderView::section { background: #f8f9fa; padding: 6px; border: none; "
        "  border-bottom: 2px solid #dee2e6; font-weight: bold; }"
    );

    // 从配置文件读取数据库连接信息（保存在 build 目录下的 ecommerce.ini）
    QString configPath = QCoreApplication::applicationDirPath() + "/ecommerce.ini";
    QSettings settings(configPath, QSettings::IniFormat);

    QString dbHost = settings.value("Database/Host", "127.0.0.1").toString();
    int dbPort = settings.value("Database/Port", 3306).toInt();
    QString dbUser = settings.value("Database/User", "root").toString();
    QString dbPass = settings.value("Database/Password", "").toString();
    QString dbName = settings.value("Database/Name", "ecommerce").toString();

    // 循环尝试连接数据库
    auto& db = DBManager::instance();
    while (!db.connect(dbHost.toStdString(), dbPort,
                       dbUser.toStdString(), dbPass.toStdString(),
                       dbName.toStdString())) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("数据库连接失败");
        msgBox.setText("无法连接 MySQL，请检查后重试。\n\n"
                       "请确认：\n"
                       "1. MySQL 已启动\n"
                       "2. 已执行 sql/init.sql 初始化数据库\n"
                       "3. 用户名和密码正确");
        msgBox.setIcon(QMessageBox::Critical);
        auto* retryBtn = msgBox.addButton("重试", QMessageBox::AcceptRole);
        auto* configBtn = msgBox.addButton("修改配置", QMessageBox::ActionRole);
        msgBox.addButton("退出", QMessageBox::RejectRole);
        msgBox.exec();

        if (msgBox.clickedButton() == configBtn) {
            bool ok;
            QString host = QInputDialog::getText(nullptr, "数据库配置", "主机地址:",
                                                  QLineEdit::Normal, dbHost, &ok);
            if (!ok) continue;
            dbHost = host;

            int port = QInputDialog::getInt(nullptr, "数据库配置", "端口:", dbPort, 1, 65535, 1, &ok);
            if (!ok) continue;
            dbPort = port;

            QString user = QInputDialog::getText(nullptr, "数据库配置", "用户名:",
                                                  QLineEdit::Normal, dbUser, &ok);
            if (!ok) continue;
            dbUser = user;

            QString pass = QInputDialog::getText(nullptr, "数据库配置", "密码:",
                                                  QLineEdit::Password, dbPass, &ok);
            if (!ok) continue;
            dbPass = pass;

            QString name = QInputDialog::getText(nullptr, "数据库配置", "数据库名:",
                                                  QLineEdit::Normal, dbName, &ok);
            if (!ok) continue;
            dbName = name;

            // 连接成功后保存配置
            if (db.connect(dbHost.toStdString(), dbPort,
                           dbUser.toStdString(), dbPass.toStdString(),
                           dbName.toStdString())) {
                settings.setValue("Database/Host", dbHost);
                settings.setValue("Database/Port", dbPort);
                settings.setValue("Database/User", dbUser);
                settings.setValue("Database/Password", dbPass);
                settings.setValue("Database/Name", dbName);
                settings.sync();
                break;
            }
        } else if (msgBox.clickedButton() != retryBtn) {
            return 1;
        }
    }

    // 登录/主界面循环（使用局部 QEventLoop 避免多次调用 app.exec()）
    bool needRelogin = true;
    while (needRelogin) {
        LoginDialog loginDialog;
        if (loginDialog.exec() != QDialog::Accepted) {
            break;
        }

        needRelogin = false;
        MainWindow* mainWindow = new MainWindow(loginDialog.userId(), loginDialog.userRole(),
                                                loginDialog.userName());
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);

        QObject::connect(mainWindow, &MainWindow::logoutRequested, [&]() {
            needRelogin = true;
            mainWindow->close();
        });

        mainWindow->show();

        // 局部事件循环 — 主窗口关闭时退出，不再调用 app.exec()
        QEventLoop loop;
        QObject::connect(mainWindow, &QMainWindow::destroyed, &loop, &QEventLoop::quit);
        loop.exec();
    }

    db.disconnect();
    return 0;
}
