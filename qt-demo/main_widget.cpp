#include "main_widget.h"
#include "ui_main_widget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget){

    connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.start(100);
    ui->setupUi(this);
}

MainWidget::~MainWidget(){
    delete ui;
}

void MainWidget::FlushTableWithPeerList(QTableWidget *table, const std::list<std::shared_ptr<P2PPeer> > &peer_list){
    while(table->rowCount()){
        table->removeRow(0);
    }
    std::for_each(peer_list.begin(), peer_list.end(), [=](std::shared_ptr<P2PPeer> peer){
        table->insertRow(0);
        table->setItem(0,0,new QTableWidgetItem(peer->address_local().address().to_string().c_str()));
        table->setItem(0,1,new QTableWidgetItem(QString().sprintf("%d", peer->address_local().port())));
        table->setItem(0,2,new QTableWidgetItem(peer->address_remote().address().to_string().c_str()));
        table->setItem(0,3,new QTableWidgetItem(QString().sprintf("%d", peer->address_remote().port())));
    });
}

void MainWidget::OnTimer(){
    if(!p2p_manager_)return;
    std::list<std::shared_ptr<P2PPeer> > peer_list;

    peer_list = p2p_manager_->PeerListInTmp();
    FlushTableWithPeerList(ui->tb_peer_in_tmp, peer_list);

    peer_list = p2p_manager_->PeerListOutTmp();
    FlushTableWithPeerList(ui->tb_peer_out_tmp, peer_list);
}

void MainWidget::on_btnStartStopService_clicked(bool checked){
    if(checked){
        p2p_manager_ = std::make_shared<P2PPeerManager>(ui->edtServerPort->text().toInt());
        p2p_manager_->Start();
    }else{
        p2p_manager_.reset();
    }
}

void MainWidget::on_btnConnect_clicked(){
    boost::asio::ip::tcp::endpoint ep;
    ep.address(boost::asio::ip::address::from_string(ui->edtConnectOutIP->text().toStdString()));
    ep.port(ui->edtConnectOutPort->text().toInt());
    p2p_manager_->ConnectTo(boost::asio::ip::tcp::endpoint(ep));
}
