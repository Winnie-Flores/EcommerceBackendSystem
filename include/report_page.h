#ifndef REPORT_PAGE_H
#define REPORT_PAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>

namespace QtCharts {
    class QChartView;
    class QChart;
}

class ReportPage : public QWidget {
    Q_OBJECT
public:
    explicit ReportPage(QWidget* parent = nullptr);

public slots:
    void refresh();

private:
    void setupUI();
    void refreshSummary();
    void refreshTopProducts();
    void refreshStockAlert();
    void refreshSalesTrend();

    QLabel* totalSalesLabel_;
    QLabel* totalOrdersLabel_;
    QLabel* totalProductsLabel_;
    QLabel* totalUsersLabel_;
    QTableWidget* topProductsTable_;
    QTableWidget* stockAlertTable_;
    QComboBox* periodCombo_;
    QtCharts::QChartView* chartView_;
};

#endif
