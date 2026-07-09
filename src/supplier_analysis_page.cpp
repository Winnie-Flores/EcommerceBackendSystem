#include "supplier_analysis_page.h"
#include "dbmanager.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QScrollArea>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

SupplierAnalysisPage::SupplierAnalysisPage(int supplierId, QWidget* parent)
    : QWidget(parent), supplierId_(supplierId) {
    setupUI();
    refresh();
}

void SupplierAnalysisPage::setupUI() {
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #ecf0f1; }");

    auto* container = new QWidget();
    auto* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // ===== 标题行 =====
    auto* titleRow = new QHBoxLayout();
    auto* titleLabel = new QLabel("📊 数据分析");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50;");

    periodCombo_ = new QComboBox();
    periodCombo_->addItem("全部时间", "all");
    periodCombo_->addItem("本周", "week");
    periodCombo_->addItem("本月", "month");
    periodCombo_->addItem("近3个月", "3months");
    periodCombo_->setMinimumHeight(30);
    periodCombo_->setMinimumWidth(120);
    periodCombo_->setStyleSheet("QComboBox { border: 1px solid #bdc3c7; border-radius: 3px; padding: 4px 8px; background: white; }");

    auto* refreshBtn = new QPushButton("刷新");
    refreshBtn->setStyleSheet("QPushButton { background: #3498db; color: white; padding: 6px 20px; border-radius: 3px; font-weight: bold; } QPushButton:hover { background: #2980b9; }");

    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(periodCombo_);
    titleRow->addWidget(refreshBtn);
    mainLayout->addLayout(titleRow);

    // ===== 数据概览卡片 =====
    auto* summaryGroup = new QGroupBox("数据概览");
    summaryGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 8px; padding-top: 15px; background: white; }");
    auto* summaryLayout = new QHBoxLayout(summaryGroup);
    summaryLayout->setSpacing(12);

    auto makeCard = [](const QString& title, const QString& icon, const QString& color, QLabel*& label) -> QWidget* {
        auto* card = new QWidget();
        card->setStyleSheet("background: white; border-radius: 8px; padding: 12px; border: 1px solid #ecf0f1;");
        card->setMinimumWidth(150);
        auto* layout = new QVBoxLayout(card);
        layout->setSpacing(6);

        auto* iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 22px;");
        iconLabel->setAlignment(Qt::AlignCenter);

        auto* t = new QLabel(title);
        t->setStyleSheet("color: #7f8c8d; font-size: 11px;");
        t->setAlignment(Qt::AlignCenter);

        label = new QLabel("0");
        label->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold;").arg(color));
        label->setAlignment(Qt::AlignCenter);

        layout->addWidget(iconLabel);
        layout->addWidget(t);
        layout->addWidget(label);
        return card;
    };

    summaryLayout->addWidget(makeCard("我的销售额", "💰", "#e67e22", totalSalesLabel_));
    summaryLayout->addWidget(makeCard("涉及订单数", "📋", "#3498db", totalOrdersLabel_));
    summaryLayout->addWidget(makeCard("商品种类", "📦", "#2ecc71", totalProductsLabel_));
    summaryLayout->addWidget(makeCard("缺货商品", "⚠️", "#e74c3c", outOfStockLabel_));
    summaryLayout->addWidget(makeCard("毛利润", "📈", "#9b59b6", grossProfitLabel_));

    mainLayout->addWidget(summaryGroup);

    // ===== 中间行：趋势图 + 商品排行 =====
    auto* midRow = new QHBoxLayout();
    midRow->setSpacing(15);

    // 左侧：销售趋势混合图表
    auto* trendGroup = new QGroupBox("销售趋势（柱状=销售额 / 折线=订单数）");
    trendGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 8px; padding-top: 15px; background: white; }");
    auto* trendLayout = new QVBoxLayout(trendGroup);
    trendChartView_ = new QtCharts::QChartView();
    trendChartView_->setRenderHint(QPainter::Antialiasing);
    trendChartView_->setMinimumSize(450, 300);
    trendLayout->addWidget(trendChartView_);
    midRow->addWidget(trendGroup, 3);

    // 右侧：商品销售排行
    auto* rankGroup = new QGroupBox("我的商品销售排行 TOP5");
    rankGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 8px; padding-top: 15px; background: white; }");
    auto* rankLayout = new QVBoxLayout(rankGroup);
    rankingTable_ = new QTableWidget();
    rankingTable_->setColumnCount(4);
    rankingTable_->setHorizontalHeaderLabels({"商品名称", "销量", "销售额", "毛利润"});
    rankingTable_->horizontalHeader()->setStretchLastSection(true);
    rankingTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rankingTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rankingTable_->setAlternatingRowColors(true);
    rankingTable_->verticalHeader()->setVisible(false);
    rankingTable_->setMaximumHeight(300);
    rankLayout->addWidget(rankingTable_);
    midRow->addWidget(rankGroup, 2);

    mainLayout->addLayout(midRow);

    // ===== 底部行：饼图 =====
    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(15);

    // 左侧：库存状态饼图
    auto* stockPieGroup = new QGroupBox("库存状态分布");
    stockPieGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 8px; padding-top: 15px; background: white; }");
    auto* stockPieLayout = new QVBoxLayout(stockPieGroup);
    stockPieView_ = new QtCharts::QChartView();
    stockPieView_->setRenderHint(QPainter::Antialiasing);
    stockPieView_->setMinimumSize(300, 280);
    stockPieLayout->addWidget(stockPieView_);
    bottomRow->addWidget(stockPieGroup);

    // 右侧：订单状态饼图
    auto* orderPieGroup = new QGroupBox("订单状态分布");
    orderPieGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #bdc3c7; border-radius: 4px; margin-top: 8px; padding-top: 15px; background: white; }");
    auto* orderPieLayout = new QVBoxLayout(orderPieGroup);
    orderPieView_ = new QtCharts::QChartView();
    orderPieView_->setRenderHint(QPainter::Antialiasing);
    orderPieView_->setMinimumSize(300, 280);
    orderPieLayout->addWidget(orderPieView_);
    bottomRow->addWidget(orderPieGroup);

    mainLayout->addLayout(bottomRow);

    mainLayout->addStretch();

    scrollArea->setWidget(container);

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scrollArea);

    connect(refreshBtn, &QPushButton::clicked, this, &SupplierAnalysisPage::refresh);
    connect(periodCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SupplierAnalysisPage::refresh);
}

QString SupplierAnalysisPage::timeCondition() const {
    QString val = periodCombo_->currentData().toString();
    if (val == "week")
        return " AND o.created_at >= DATE_SUB(CURDATE(), INTERVAL WEEKDAY(CURDATE()) DAY) ";
    if (val == "month")
        return " AND DATE_FORMAT(o.created_at, '%Y-%m') = DATE_FORMAT(CURDATE(), '%Y-%m') ";
    if (val == "3months")
        return " AND o.created_at >= DATE_SUB(CURDATE(), INTERVAL 3 MONTH) ";
    return ""; // all
}

void SupplierAnalysisPage::refresh() {
    refreshSummary();
    refreshProductRanking();
    refreshSalesTrend();
    refreshStockPie();
    refreshOrderStatusPie();
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

// ===== 概览卡片 =====
void SupplierAnalysisPage::refreshSummary() {
    auto& db = DBManager::instance();
    std::string sid = std::to_string(supplierId_);
    QString tc = timeCondition();

    // 我的商品总销售额
    auto sales = db.query(
        "SELECT COALESCE(SUM(oi.quantity * oi.price), 0) as total "
        "FROM order_items oi "
        "JOIN products p ON oi.product_id=p.id "
        "JOIN orders o ON oi.order_id=o.id "
        "WHERE p.supplier_id=" + sid + " AND o.status IN (1,2,3)" + tc.toStdString());
    totalSalesLabel_->setText("¥" + QString::number(rowDouble(sales[0], "total"), 'f', 0));

    // 含我商品的订单数（带时间条件）
    std::string orderSql = "SELECT COUNT(DISTINCT o.id) as cnt "
        "FROM orders o "
        "JOIN order_items oi ON o.id=oi.order_id "
        "JOIN products p ON oi.product_id=p.id "
        "WHERE p.supplier_id=" + sid;
    if (!tc.isEmpty())
        orderSql += tc.toStdString();
    auto orderResult = db.query(orderSql);
    totalOrdersLabel_->setText(rowStr(orderResult[0], "cnt"));

    // 我的商品种类
    auto products = db.query("SELECT COUNT(*) as cnt FROM products WHERE supplier_id=" + sid);
    totalProductsLabel_->setText(rowStr(products[0], "cnt"));

    // 缺货商品（stock=0）
    auto outOfStock = db.query("SELECT COUNT(*) as cnt FROM products WHERE supplier_id=" + sid + " AND stock=0");
    outOfStockLabel_->setText(rowStr(outOfStock[0], "cnt"));

    // 毛利润 (售价-成本) * 销量
    auto profit = db.query(
        "SELECT COALESCE(SUM((oi.price - p.cost) * oi.quantity), 0) as total "
        "FROM order_items oi "
        "JOIN products p ON oi.product_id=p.id "
        "JOIN orders o ON oi.order_id=o.id "
        "WHERE p.supplier_id=" + sid + " AND o.status IN (1,2,3)" + tc.toStdString());
    double gp = rowDouble(profit[0], "total");
    grossProfitLabel_->setText("¥" + QString::number(gp, 'f', 0));
}

// ===== 商品排行 =====
void SupplierAnalysisPage::refreshProductRanking() {
    auto& db = DBManager::instance();
    std::string sid = std::to_string(supplierId_);
    QString tc = timeCondition();

    auto result = db.query(
        "SELECT p.name, "
        "COALESCE(SUM(oi.quantity), 0) as sold, "
        "COALESCE(SUM(oi.quantity * oi.price), 0) as revenue, "
        "COALESCE(SUM((oi.price - p.cost) * oi.quantity), 0) as profit "
        "FROM order_items oi "
        "JOIN products p ON oi.product_id=p.id "
        "JOIN orders o ON oi.order_id=o.id "
        "WHERE p.supplier_id=" + sid + " AND o.status IN (1,2,3)" + tc.toStdString() +
        " GROUP BY p.id, p.name ORDER BY revenue DESC LIMIT 5");

    rankingTable_->setRowCount(static_cast<int>(result.size()));
    for (size_t i = 0; i < result.size(); ++i) {
        rankingTable_->setItem(static_cast<int>(i), 0, new QTableWidgetItem(rowStr(result[i], "name")));

        auto* soldItem = new QTableWidgetItem(rowStr(result[i], "sold"));
        soldItem->setTextAlignment(Qt::AlignCenter);
        rankingTable_->setItem(static_cast<int>(i), 1, soldItem);

        auto* revItem = new QTableWidgetItem("¥" + QString::number(rowDouble(result[i], "revenue"), 'f', 2));
        revItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rankingTable_->setItem(static_cast<int>(i), 2, revItem);

        auto* profItem = new QTableWidgetItem("¥" + QString::number(rowDouble(result[i], "profit"), 'f', 2));
        profItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        double p = rowDouble(result[i], "profit");
        if (p > 0) profItem->setForeground(QColor("#27ae60"));
        else if (p < 0) profItem->setForeground(QColor("#e74c3c"));
        rankingTable_->setItem(static_cast<int>(i), 3, profItem);
    }
}

// ===== 销售趋势混合图表（柱状+折线） =====
void SupplierAnalysisPage::refreshSalesTrend() {
    auto& db = DBManager::instance();
    std::string sid = std::to_string(supplierId_);

    auto result = db.query(
        "SELECT DATE_FORMAT(o.created_at, '%Y-%m') as month, "
        "COALESCE(SUM(oi.quantity * oi.price), 0) as amount, "
        "COUNT(DISTINCT o.id) as cnt "
        "FROM orders o "
        "JOIN order_items oi ON o.id=oi.order_id "
        "JOIN products p ON oi.product_id=p.id "
        "WHERE p.supplier_id=" + sid + " AND o.status IN (1,2,3) "
        "GROUP BY month ORDER BY month LIMIT 12");

    auto* chart = new QtCharts::QChart();
    chart->setTitle("月度销售趋势");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QStringList categories;
    auto* barSet = new QtCharts::QBarSet("销售额(元)");
    barSet->setColor(QColor("#3498db"));

    auto* lineSeries = new QtCharts::QLineSeries();
    lineSeries->setName("订单数");
    QPen linePen(QColor("#e74c3c"), 2);
    lineSeries->setPen(linePen);

    double maxAmount = 0;
    int maxCnt = 0;

    if (result.empty()) {
        *barSet << 0;
        categories << "暂无数据";
        lineSeries->append(0, 0);
    } else {
        for (size_t i = 0; i < result.size(); ++i) {
            double amount = rowDouble(result[i], "amount");
            int cnt = rowInt(result[i], "cnt");
            *barSet << amount;
            categories << rowStr(result[i], "month");
            lineSeries->append(static_cast<qreal>(i), cnt);
            if (amount > maxAmount) maxAmount = amount;
            if (cnt > maxCnt) maxCnt = cnt;
        }
    }

    // 柱状图
    auto* barSeries = new QtCharts::QBarSeries();
    barSeries->append(barSet);
    chart->addSeries(barSeries);

    // 折线
    chart->addSeries(lineSeries);
    lineSeries->setPointsVisible(true);

    // X 轴：使用 QBarCategoryAxis（兼容 QBarSeries）
    auto* axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    // 左 Y（销售额）
    auto* axisYLeft = new QtCharts::QValueAxis();
    axisYLeft->setTitleText("销售额 (元)");
    axisYLeft->setLabelFormat("%.0f");
    axisYLeft->setRange(0, maxAmount > 0 ? maxAmount * 1.15 : 1.0);
    chart->addAxis(axisYLeft, Qt::AlignLeft);
    barSeries->attachAxis(axisYLeft);

    // 右 Y（订单数）
    auto* axisYRight = new QtCharts::QValueAxis();
    axisYRight->setTitleText("订单数");
    axisYRight->setLabelFormat("%d");
    axisYRight->setRange(0, maxCnt > 0 ? maxCnt * 1.2 : 1.0);
    chart->addAxis(axisYRight, Qt::AlignRight);
    lineSeries->attachAxis(axisYRight);

    // 替换旧图表
    auto* oldChart = trendChartView_->chart();
    trendChartView_->setChart(chart);
    delete oldChart;
}

// ===== 库存状态饼图 =====
void SupplierAnalysisPage::refreshStockPie() {
    auto& db = DBManager::instance();
    std::string sid = std::to_string(supplierId_);

    auto full = db.query("SELECT COUNT(*) as cnt FROM products WHERE supplier_id=" + sid + " AND stock > alert_stock * 2");
    auto normal = db.query("SELECT COUNT(*) as cnt FROM products WHERE supplier_id=" + sid + " AND stock > alert_stock AND stock <= alert_stock * 2");
    auto low = db.query("SELECT COUNT(*) as cnt FROM products WHERE supplier_id=" + sid + " AND stock > 0 AND stock <= alert_stock");
    auto zero = db.query("SELECT COUNT(*) as cnt FROM products WHERE supplier_id=" + sid + " AND stock = 0");

    int cntFull = rowInt(full[0], "cnt");
    int cntNormal = rowInt(normal[0], "cnt");
    int cntLow = rowInt(low[0], "cnt");
    int cntZero = rowInt(zero[0], "cnt");

    auto* series = new QtCharts::QPieSeries();
    if (cntFull > 0) {
        auto* s = series->append("充足(" + QString::number(cntFull) + ")", cntFull);
        s->setColor(QColor("#27ae60"));
    }
    if (cntNormal > 0) {
        auto* s = series->append("正常(" + QString::number(cntNormal) + ")", cntNormal);
        s->setColor(QColor("#3498db"));
    }
    if (cntLow > 0) {
        auto* s = series->append("低库存(" + QString::number(cntLow) + ")", cntLow);
        s->setColor(QColor("#f39c12"));
    }
    if (cntZero > 0) {
        auto* s = series->append("缺货(" + QString::number(cntZero) + ")", cntZero);
        s->setColor(QColor("#e74c3c"));
    }
    if (cntFull + cntNormal + cntLow + cntZero == 0) {
        series->append("暂无商品", 1)->setColor(QColor("#bdc3c7"));
    }

    series->setLabelsVisible(true);
    series->setLabelsPosition(QtCharts::QPieSlice::LabelOutside);

    auto* chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("库存状态");
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    auto* oldStockChart = stockPieView_->chart();
    stockPieView_->setChart(chart);
    delete oldStockChart;
}

// ===== 订单状态饼图 =====
void SupplierAnalysisPage::refreshOrderStatusPie() {
    auto& db = DBManager::instance();
    std::string sid = std::to_string(supplierId_);
    QString tc = timeCondition();

    std::string orderSql =
        "SELECT o.status, COUNT(DISTINCT o.id) as cnt "
        "FROM orders o "
        "JOIN order_items oi ON o.id=oi.order_id "
        "JOIN products p ON oi.product_id=p.id "
        "WHERE p.supplier_id=" + sid;
    if (!tc.isEmpty())
        orderSql += tc.toStdString();
    orderSql += " GROUP BY o.status";

    auto result = db.query(orderSql);

    auto* series = new QtCharts::QPieSeries();

    // 颜色映射
    QColor colors[] = {
        QColor("#f39c12"),  // 待支付 - 橙
        QColor("#3498db"),  // 已支付 - 蓝
        QColor("#9b59b6"),  // 已发货 - 紫
        QColor("#27ae60"),  // 已完成 - 绿
        QColor("#95a5a6")   // 已取消 - 灰
    };

    for (auto& row : result) {
        int st = rowInt(row, "status");
        int cnt = rowInt(row, "cnt");
        if (cnt > 0) {
            auto* s = series->append(statusText(st) + "(" + QString::number(cnt) + ")", cnt);
            if (st >= 0 && st <= 4) s->setColor(colors[st]);
        }
    }

    if (result.empty()) {
        series->append("暂无订单", 1)->setColor(QColor("#bdc3c7"));
    }

    series->setLabelsVisible(true);
    series->setLabelsPosition(QtCharts::QPieSlice::LabelOutside);

    auto* chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("订单状态分布");
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    auto* oldOrderChart = orderPieView_->chart();
    orderPieView_->setChart(chart);
    delete oldOrderChart;
}
