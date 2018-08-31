#include <QThread>
#include <QMessageBox>
#include "optimizedb.h"
#include "optimizedbworker.h"
#include "ui_optimizedb.h"

OptimizeDB::OptimizeDB(QWidget *parent, const QMap<QString, QVariant> &connectionData) :
    QDialog(parent),
    ui(new Ui::OptimizeDB)
{
    ui->setupUi(this);
    OptimizeDBWorker * worker = new OptimizeDBWorker(connectionData);
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker,&OptimizeDBWorker::progressValueCount,ui->progressBar,&QProgressBar::setMaximum);
    connect(worker,&OptimizeDBWorker::progressValue,ui->progressBar,&QProgressBar::setValue);
    connect(worker,&OptimizeDBWorker::error,this,&OptimizeDB::showError);
    connect(worker,&OptimizeDBWorker::successfully,this,&OptimizeDB::successfullyMessage);
    connect(this, SIGNAL(accepted()), worker, SIGNAL(finished()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

OptimizeDB::~OptimizeDB()
{
    delete ui;
}

void OptimizeDB::showError(QString errorText)
{
    QMessageBox::critical(this,tr("Ошибка"), errorText, tr("Закрыть"));
    accept();
}

void OptimizeDB::successfullyMessage()
{
    QMessageBox::information(this,tr(" "), tr("Оптимизация успешно завершена"), tr("Закрыть"));
    accept();
}
