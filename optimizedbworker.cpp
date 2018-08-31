#include <QMessageBox>
#include "optimizedbworker.h"

OptimizeDBWorker::OptimizeDBWorker(const QMap<QString, QVariant> &connectionData) : m_connectionData(connectionData)
{

}

OptimizeDBWorker::~OptimizeDBWorker()
{

}

void OptimizeDBWorker::process()
{
    if(QSqlDatabase::connectionNames().contains("OptimizeDBWorker_thread")){
        QSqlDatabase::removeDatabase("OptimizeDBWorker_thread");
    }
    QSqlDatabase db = QSqlDatabase::addDatabase(m_connectionData.value("driver").toString(),"OptimizeDBWorker_thread");
    db.setHostName(m_connectionData.value("host").toString());
    db.setPort(m_connectionData.value("port").toInt());
    db.setUserName(m_connectionData.value("login").toString());
    db.setPassword(m_connectionData.value("pass").toString());
    db.setDatabaseName(m_connectionData.value("databaseName").toString());
    if(!db.open()){
        emit error(tr("При связи с базой данных возникла ошибка: %1").arg(db.lastError().text()));
        emit finished();
        return;
    }
    QSqlQuery sql = QSqlQuery(db);
    if(!sql.exec("SHOW TABLES;"))
    {
        emit error(tr("При получении списка таблиц возникла ошибка: %1").arg(sql.lastError().text()));
        db.close();
        emit finished();
        return;
    }
    emit progressValueCount(sql.size());
    int currentProgress = 0;
    while (sql.next())
    {
        emit progressValue(++currentProgress);
        QSqlQuery sql2 = QSqlQuery(db);
        if(!sql2.exec(QString("OPTIMIZE TABLE %1;").arg(sql.value(0).toString())))
        {
            emit progressValue(0);
            emit error(tr("При оптимизации таблици %2 возникла ошибка: %1").arg(sql2.lastError().text()).arg(sql.value(0).toString()));
            db.close();
            emit finished();
            return;
        }
        while (sql2.next())
        {
            if (sql2.value(2).toString().compare("status",Qt::CaseInsensitive) == 0){
                if (sql2.value(3).toString().compare("OK",Qt::CaseInsensitive) != 0)
                {
                    emit progressValue(0);
                    emit error(tr("При оптимизации таблици %2 возникла ошибка: %1").arg(sql2.lastError().text()).arg(sql.value(0).toString()));
                    db.close();
                    emit finished();
                    return;
                }
            }
        }
    }
    if(db.isOpen()) db.close();
    emit progressValue(0);
    emit successfully();
    emit finished();
}
