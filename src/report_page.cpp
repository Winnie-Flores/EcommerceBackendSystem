#include "report_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

ReportPage::ReportPage(QWidget* parent) : QWidget(parent) {
    setupUI();
    refresh();
}

void ReportPage::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* titleLabel = new QLabel("数据统计报表");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 概览卡片
    auto* summaryGroup = new QGroupBox("数据概览");
    summaryGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 10px; padding-top: 15px; }");
    auto* summaryLayout = new QHBoxLayout(summaryGroup);

    auto makeCard = [](const QString& title, QLabel*& label) -> QWidget* {
        auto* card = new QWidget();
        card->setStyleSheet("background: white; border-radius: 8px; padding: 15px; border: 1px solid #ecf0f1;");
        auto* layout = new QVBoxLayout(card);
        auto* t = new QLabel(title);
        t->setStyleSheet("color: #7f8c8d; font-size: 12px;");
        t->setAlignment(Qt::AlignCenter);
        layout->addWidget(t);
        label = new QLabel("0");
        label->setStyleSheet("color: #2c3e50; font-size: 28px; font-weight: bold;");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        return card;
    };

    summaryLayout->addWidget(makeCard("总销售额 (¥)", totalSalesLabel_));
    summaryLayout->addWidget(makeCard("总订单数", totalOrdersLabel_));
    summaryLayout->addWidget(makeCard("商品种类", totalProductsLabel_));
    summaryLayout->addWidget(makeCard("用户数", totalUsersLabel_));

    mainLayout->addWidget(summaryGroup);

    // 图表 + 表格
    auto* contentLayout = new QHBoxLayout();

    // 左侧：柱状图
    chartView_ = new QtCharts::QChartView();
    chartView_->setRenderHint(QPainter::Antialiasing);
    chartView_->setMinimumSize(500, 300);
    contentLayout->addWidget(chartView_, 3);

    // 右侧：热销排行 + 库存预警
    auto* rightLayout = new QVBoxLayout();

    auto* topGroup = new QGroupBox("热销商品 TOP5");
    topGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    auto* topLayout = new QVBoxLayout(topGroup);
    topProductsTable_ = new QTableWidget();
    topProductsTable_->setColumnCount(3);
    topProductsTable_->setHorizontalHeaderLabels({"商品", "销量", "销售额"});
    topProductsTable_->horizontalHeader()->setStretchLastSection(true);
    topProductsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    topProductsTable_->setMaximumHeight(200);
    topLayout->addWidget(topProductsTable_);
    rightLayout->addWidget(topGroup);

    auto* alertGroup = new QGroupBox("库存预警");
    alertGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    auto* alertLayout = new QVBoxLayout(alertGroup);
    stockAlertTable_ = new QTableWidget();
    stockAlertTable_->setColumnCount(3);
    stockAlertTable_->setHorizontalHeaderLabels({"商品", "当前库存", "预警阈值"});
    stockAlertTable_->horizontalHeader()->setStretchLastSection(true);
    stockAlertTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stockAlertTable_->setMaximumHeight(200);
    alertLayout->addWidget(stockAlertTable_);
    rightLayout->addWidget(alertGroup);

    contentLayout->addLayout(rightLayout, 2);
    mainLayout->addLayout(contentLayout);

    // 刷新按钮
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto* refreshBtn = new QPushButton("刷新数据");
    refreshBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 8px 24px; border-radius: 3px; font-weight: bold; } QPushButton:hover { background: #2980b9; }");
    btnLayout->addWidget(refreshBtn);
    mainLayout->addLayout(btnLayout);

    connect(refreshBtn, &QPushButton::clicked, this, &ReportPage::refresh);
}

void ReportPage::refresh() {
    refreshSummary();
    refreshTopProducts();
    refreshStockAlert();
    refreshSalesTrend();
}

void ReportPage::refreshSummary() {
    auto& db = DBManager::instance();

    auto sales = db.query("SELECT COALESCE(SUM(total_amount), 0) as total FROM orders WHERE status IN (1,2,3)");
    totalSalesLabel_->setText(QString::number(rowDouble(sales[0], "total"), 'f', 0));

    auto orders = db.query("SELECT COUNT(*) as cnt FROM orders");
    totalOrdersLabel_->setText(rowStr(orders[0], "cnt"));

    auto products = db.query("SELECT COUNT(*) as cnt FROM products");
    totalProductsLabel_->setText(rowStr(products[0], "cnt"));

    auto users = db.query("SELECT COUNT(*) as cnt FROM users");
    totalUsersLabel_->setText(rowStr(users[0], "cnt"));
}

void ReportPage::refreshTopProducts() {
    auto& db = DBManager::instance();
    auto result = db.query(
        "SELECT p.name, SUM(oi.quantity) as sold, SUM(oi.quantity * oi.price) as revenue "
        "FROM order_items oi "
        "JOIN products p ON oi.product_id=p.id "
        "JOIN orders o ON oi.order_id=o.id "
        "WHERE o.status IN (1,2,3) "
        "GROUP BY p.id, p.name ORDER BY sold DESC LIMIT 5");

    topProductsTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        topProductsTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(rowStr(result[i], "name")));
        topProductsTable_->setItem(static_cast<int>(i), 1, new QTableWidgetItem(rowStr(result[i], "sold")));
        topProductsTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(
            "¥" + QString::number(rowDouble(result[i], "revenue"), 'f', 2)));
    }
}

void ReportPage::refreshStockAlert() {
    auto& db = DBManager::instance();
    auto result = db.query(
        "SELECT name, stock, alert_stock FROM products WHERE stock <= alert_stock ORDER BY stock ASC");

    stockAlertTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        auto* nameItem = new QTableWidgetItem(rowStr(result[i], "name"));
        nameItem->setForeground(QColor("#e74c3c"));
        stockAlertTable_->setItem(static_cast<int>(i), 0, nameItem);

        auto* stockItem = new QTableWidgetItem(rowStr(result[i], "stock"));
        stockItem->setForeground(QColor("#e74c3c"));
        stockAlertTable_->setItem(static_cast<int>(i), 1, stockItem);

        stockAlertTable_->setItem(static_cast<int>(i), 2, new QTableWidgetItem(rowStr(result[i], "alert_stock")));
    }
}

void ReportPage::refreshSalesTrend() {
    auto& db = DBManager::instance();
    auto result = db.query(
        "SELECT DATE_FORMAT(created_at, '%Y-%m') as month, "
        "SUM(CASE WHEN status IN (1,2,3) THEN total_amount ELSE 0 END) as amount, "
        "COUNT(CASE WHEN status IN (1,2,3) THEN 1 END) as cnt "
        "FROM orders GROUP BY month ORDER BY month LIMIT 12");

    auto* chart = new QtCharts::QChart();
    chart->setTitle("月度销售趋势");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    auto* barSet = new QtCharts::QBarSet("销售额(元)");
    QStringList categories;

    for (auto& row : result) {
        *barSet << rowDouble(row, "amount");
        categories << rowStr(row, "month");
    }

    if (result.empty()) {
        *barSet << 0;
        categories << "暂无数据";
    }

    auto* series = new QtCharts::QBarSeries();
    series->append(barSet);
    chart->addSeries(series);

    auto* axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto* axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("销售额 (元)");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 替换旧图表
    auto* oldChart = chartView_->chart();
    chartView_->setChart(chart);
    delete oldChart;
}
