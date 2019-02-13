#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QTableWidget>

#include "p2p_peer_manager.h"
namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
private:
    void FlushTableWithPeerList(QTableWidget* table, const std::list<std::shared_ptr<P2PPeer>>& peer_list);
private slots:
    void OnTimer();
    void on_btnStartStopService_clicked(bool checked);
    void on_btnConnect_clicked();

private:
    std::shared_ptr<P2PPeerManager> p2p_manager_;
    QTimer timer_;
    Ui::MainWidget *ui;
};

#endif // MAIN_WIDGET_H
