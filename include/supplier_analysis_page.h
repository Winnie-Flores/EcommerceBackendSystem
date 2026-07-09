#ifndef SUPPLIER_ANALYSIS_PAGE_H
#define SUPPLIER_ANALYSIS_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

namespace QtCharts {
    class QChartView;
}

class SupplierAnalysisPage : public QWidget {
    Q_OBJECT
public:
    explicit SupplierAnalysisPage(int supplierId, QWidget* parent = nullptr);

public slots:
    void refresh();

private:
    void setupUI();
    void refreshSummary();
    void refreshProductRanking();
    void refreshSalesTrend();
    void refreshStockPie();
    void refreshOrderStatusPie();
    QString timeCondition() const; // 根据选择返回 SQL 时间条件

    int supplierId_;

    // 时间筛选
    QComboBox* periodCombo_;

    // 概览卡片
    QLabel* totalSalesLabel_;
    QLabel* totalOrdersLabel_;
    QLabel* totalProductsLabel_;
    QLabel* outOfStockLabel_;
    QLabel* grossProfitLabel_;

    // 商品排行
    QTableWidget* rankingTable_;

    // 图表
    QtCharts::QChartView* trendChartView_;
    QtCharts::QChartView* stockPieView_;
    QtCharts::QChartView* orderPieView_;
};

#endif // SUPPLIER_ANALYSIS_PAGE_H
