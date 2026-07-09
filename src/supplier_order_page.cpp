#include "supplier_order_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>

SupplierOrderPage::SupplierOrderPage(int supplierId, QWidget* parent)
    : QWidget(parent), supplierId_(supplierId) {
    setupUI();
    refreshOrderTable();
}

void SupplierOrderPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("我的订单");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 搜索栏
    auto* searchLayout = new QHBoxLayout();
    orderSearchEdit_ = new QLineEdit();
    orderSearchEdit_->setPlaceholderText("搜索订单号...");
    orderSearchEdit_->setMinimumHeight(30);

    statusFilter_ = new QComboBox();
    statusFilter_->addItem("全部状态", -1);
    statusFilter_->addItem("待支付", 0);
    statusFilter_->addItem("已支付", 1);
    statusFilter_->addItem("已发货", 2);
    statusFilter_->addItem("已完成", 3);
    statusFilter_->addItem("已取消", 4);
    statusFilter_->setMinimumHeight(30);

    auto* searchBtn = new QPushButton("搜索");
    searchBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 6px 16px; border-radius: 3px; } QPushButton:hover { background: #2980b9; }");

    detailBtn_ = new QPushButton("查看详情");
    detailBtn_->setStyleSheet("QPushButton { background: #9b59b6; color: white; padding: 6px 16px; border-radius: 3px; font-weight: bold; } QPushButton:hover { background: #8e44ad; }");

    searchLayout->addWidget(orderSearchEdit_);
    searchLayout->addWidget(statusFilter_);
    searchLayout->addWidget(searchBtn);
    searchLayout->addWidget(detailBtn_);
    mainLayout->addLayout(searchLayout);

    // 订单表格
    orderTable_ = new QTableWidget();
    orderTable_->setColumnCount(6);
    orderTable_->setHorizontalHeaderLabels({"ID", "订单号", "下单用户", "金额", "状态", "下单时间"});
    orderTable_->horizontalHeader()->setStretchLastSection(true);
    orderTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    orderTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    orderTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderTable_->setAlternatingRowColors(true);
    orderTable_->setStyleSheet("QTableWidget { background: white; gridline-color: #ddd; } "
                               "QTableWidget::item:selected { background: #3498db; color: white; }");
    mainLayout->addWidget(orderTable_);

    auto* hintLabel = new QLabel("提示：此列表仅显示包含您供应商品的订单，点击「查看详情」可查看订单明细（您的商品会以绿色标注）");
    hintLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; margin-top: 8px;");
    mainLayout->addWidget(hintLabel);

    connect(searchBtn, &QPushButton::clicked, this, &SupplierOrderPage::onSearchOrder);
    connect(detailBtn_, &QPushButton::clicked, this, &SupplierOrderPage::onViewOrderDetail);
    connect(orderTable_, &QTableWidget::itemSelectionChanged, [this]() {
        auto items = orderTable_->selectedItems();
        if (!items.isEmpty())
            selectedOrderId_ = orderTable_->item(items.first()->row(), 0)->text().toInt();
    });
}

static QString statusText(int status) {
    switch (status) {
        case 0: return "待支付";
        case 1: return "已支付";
        case 2: return "已发货";
        case 3: return "已完成";
        case 4: return "已取消";
        default: return "未知";
    }
}

static QColor statusColor(int status) {
    switch (status) {
        case 0: return QColor("#f39c12");
        case 1: return QColor("#3498db");
        case 2: return QColor("#9b59b6");
        case 3: return QColor("#27ae60");
        case 4: return QColor("#95a5a6");
        default: return QColor("#000000");
    }
}

void SupplierOrderPage::refreshOrderTable() {
    auto& db = DBManager::instance();
    // 查询包含该供应商商品的所有订单（去重）
    std::string sql = "SELECT DISTINCT o.id, o.order_no, u.username, o.total_amount, o.status, "
                      "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i') as created_at "
                      "FROM orders o "
                      "JOIN order_items oi ON o.id=oi.order_id "
                      "JOIN products p ON oi.product_id=p.id "
                      "LEFT JOIN users u ON o.user_id=u.id "
                      "WHERE p.supplier_id=" + std::to_string(supplierId_) + " "
                      "ORDER BY o.id DESC";

    auto result = db.query(sql);

    orderTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        orderTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        orderTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "order_no")));
        orderTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "username")));
        orderTable_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(rowDouble(result[i], "total_amount"), 'f', 2)));

        int st = rowInt(result[i], "status");
        auto* stItem = new QTableWidgetItem(statusText(st));
        stItem->setForeground(statusColor(st));
        stItem->setFont(QFont(stItem->font().family(), -1, QFont::Bold));
        orderTable_->setItem(static_cast<int>(i), 4, stItem);

        orderTable_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void SupplierOrderPage::onSearchOrder() {
    QString kw = orderSearchEdit_->text().trimmed();
    int filterStatus = statusFilter_->currentData().toInt();
    auto& db = DBManager::instance();

    std::string sql = "SELECT DISTINCT o.id, o.order_no, u.username, o.total_amount, o.status, "
                      "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i') as created_at "
                      "FROM orders o "
                      "JOIN order_items oi ON o.id=oi.order_id "
                      "JOIN products p ON oi.product_id=p.id "
                      "LEFT JOIN users u ON o.user_id=u.id "
                      "WHERE p.supplier_id=" + std::to_string(supplierId_);

    if (!kw.isEmpty())
        sql += " AND o.order_no LIKE '%" + db.escape(kw.toStdString()) + "%'";
    if (filterStatus >= 0)
        sql += " AND o.status=" + std::to_string(filterStatus);

    sql += " ORDER BY o.id DESC";

    auto result = db.query(sql);
    orderTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        orderTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::number(rowInt(result[i], "id"))));
        orderTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "order_no")));
        orderTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "username")));
        orderTable_->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(rowDouble(result[i], "total_amount"), 'f', 2)));
        int st = rowInt(result[i], "status");
        auto* stItem = new QTableWidgetItem(statusText(st));
        stItem->setForeground(statusColor(st));
        stItem->setFont(QFont(stItem->font().family(), -1, QFont::Bold));
        orderTable_->setItem(static_cast<int>(i), 4, stItem);
        orderTable_->setItem(static_cast<int>(i), 5, new QTableWidgetItem(rowStr(result[i], "created_at")));
    }
}

void SupplierOrderPage::onViewOrderDetail() {
    if (selectedOrderId_ == 0) {
        QMessageBox::warning(this, "提示", "请先选择要查看的订单");
        return;
    }

    auto& db = DBManager::instance();

    // 查询订单基本信息
    auto orderResult = db.query("SELECT o.id, o.order_no, u.username, o.total_amount, o.status, "
                                "o.address, o.remark, "
                                "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i') as created_at "
                                "FROM orders o LEFT JOIN users u ON o.user_id=u.id "
                                "WHERE o.id=" + std::to_string(selectedOrderId_));
    if (orderResult.empty()) return;

    // 查询订单明细
    auto items = db.query("SELECT oi.product_id, p.name, oi.quantity, oi.price, "
                          "p.supplier_id, s.name as sup_name "
                          "FROM order_items oi "
                          "JOIN products p ON oi.product_id=p.id "
                          "LEFT JOIN suppliers s ON p.supplier_id=s.id "
                          "WHERE oi.order_id=" + std::to_string(selectedOrderId_));

    // 构建弹窗内容
    QString detailHtml = "<div style='font-family: Microsoft YaHei; font-size: 14px;'>";
    detailHtml += "<h3 style='color: #2c3e50; margin-bottom: 10px;'>订单详情</h3>";

    detailHtml += "<table style='width:100%; border-collapse:collapse; margin-bottom:15px;'>";
    detailHtml += "<tr><td style='padding:4px 10px; color:#7f8c8d;'>订单号:</td>"
                  "<td style='padding:4px 10px; font-weight:bold;'>" + rowStr(orderResult[0], "order_no") + "</td></tr>";
    detailHtml += "<tr><td style='padding:4px 10px; color:#7f8c8d;'>下单用户:</td>"
                  "<td style='padding:4px 10px;'>" + rowStr(orderResult[0], "username") + "</td></tr>";
    detailHtml += "<tr><td style='padding:4px 10px; color:#7f8c8d;'>订单金额:</td>"
                  "<td style='padding:4px 10px; color:#e74c3c; font-weight:bold;'>¥" +
                  QString::number(rowDouble(orderResult[0], "total_amount"), 'f', 2) + "</td></tr>";

    int st = rowInt(orderResult[0], "status");
    QString stStr = statusText(st);
    QString stColor = (st == 0) ? "#f39c12" : (st == 1) ? "#3498db" : (st == 2) ? "#9b59b6" : (st == 3) ? "#27ae60" : "#95a5a6";
    detailHtml += "<tr><td style='padding:4px 10px; color:#7f8c8d;'>状态:</td>"
                  "<td style='padding:4px 10px; color:" + stColor + "; font-weight:bold;'>" + stStr + "</td></tr>";
    detailHtml += "<tr><td style='padding:4px 10px; color:#7f8c8d;'>收货地址:</td>"
                  "<td style='padding:4px 10px;'>" + rowStr(orderResult[0], "address") + "</td></tr>";
    detailHtml += "<tr><td style='padding:4px 10px; color:#7f8c8d;'>下单时间:</td>"
                  "<td style='padding:4px 10px;'>" + rowStr(orderResult[0], "created_at") + "</td></tr>";
    detailHtml += "</table>";

    // 订单明细表格
    detailHtml += "<h4 style='color: #2c3e50; margin: 10px 0;'>商品明细</h4>";
    detailHtml += "<table style='width:100%; border-collapse:collapse; border:1px solid #ddd;'>";
    detailHtml += "<tr style='background:#f8f9fa;'>"
                  "<th style='padding:8px; border:1px solid #ddd; text-align:left;'>商品名称</th>"
                  "<th style='padding:8px; border:1px solid #ddd; text-align:center;'>数量</th>"
                  "<th style='padding:8px; border:1px solid #ddd; text-align:right;'>单价</th>"
                  "<th style='padding:8px; border:1px solid #ddd; text-align:right;'>小计</th>"
                  "<th style='padding:8px; border:1px solid #ddd; text-align:center;'>供应商</th>"
                  "</tr>";

    for (auto& item : items) {
        int itemSupplierId = rowInt(item, "supplier_id");
        bool isMine = (itemSupplierId == supplierId_);

        QString rowStyle = isMine ? "background:#d4efdf; font-weight:bold;" : "";
        QString badge = isMine ? " <span style='background:#27ae60; color:white; padding:1px 6px; border-radius:3px; font-size:11px;'>我的商品</span>" : "";

        double subtotal = rowInt(item, "quantity") * rowDouble(item, "price");

        detailHtml += "<tr style='" + rowStyle + "'>";
        detailHtml += "<td style='padding:8px; border:1px solid #ddd;'>" + rowStr(item, "name") + badge + "</td>";
        detailHtml += "<td style='padding:8px; border:1px solid #ddd; text-align:center;'>" + QString::number(rowInt(item, "quantity")) + "</td>";
        detailHtml += "<td style='padding:8px; border:1px solid #ddd; text-align:right;'>¥" + QString::number(rowDouble(item, "price"), 'f', 2) + "</td>";
        detailHtml += "<td style='padding:8px; border:1px solid #ddd; text-align:right;'>¥" + QString::number(subtotal, 'f', 2) + "</td>";
        detailHtml += "<td style='padding:8px; border:1px solid #ddd; text-align:center;'>" + rowStr(item, "sup_name") + "</td>";
        detailHtml += "</tr>";
    }

    detailHtml += "</table>";
    detailHtml += "<p style='color:#7f8c8d; font-size:12px; margin-top:10px;'>"
                  "<span style='background:#d4efdf; padding:2px 8px; border-radius:3px;'>绿色行</span> = 您供应的商品</p>";
    detailHtml += "</div>";

    // 弹窗显示
    QDialog dlg(this);
    dlg.setWindowTitle("订单详情 - " + rowStr(orderResult[0], "order_no"));
    dlg.setMinimumSize(600, 400);
    auto* dlgLayout = new QVBoxLayout(&dlg);
    auto* label = new QLabel(detailHtml);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    dlgLayout->addWidget(label);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    dlgLayout->addWidget(btnBox);

    dlg.exec();
}
