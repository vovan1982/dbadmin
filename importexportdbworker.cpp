#include <QtSql>
#include <QFile>
#include <QProcess>
#include "importexportdbworker.h"

ImportExportDBWorker::ImportExportDBWorker(const QMap<QString, QVariant> &credentials, const QStringList databases, Enums::ImportExportMode mode) :
    m_credentials(credentials),
    m_databases(databases),
    m_mode(mode)
{

}

ImportExportDBWorker::~ImportExportDBWorker()
{

}

void ImportExportDBWorker::process()
{
    if(m_mode == Enums::Export){
        emit logData(tr("Выполняется экспорт базы..."));
        QString cmd = QString("\"%1/mysqldump.exe\" -h %2 --verbose --single-transaction --skip-triggers -u %3 -p%4 %5")
                .arg(m_credentials.value("pathToMySqlTools").toString())
                .arg(m_credentials.value("host").toString())
                .arg(m_credentials.value("login").toString())
                .arg(m_credentials.value("pass").toString())
                .arg(m_credentials.value("databaseName").toString());
        emit logData(tr("Выполнение команды %1").arg(cmd));
        QProcess *runExport = new QProcess(this);
        connect(runExport,&QProcess::readyReadStandardError,this,&ImportExportDBWorker::readyReadStandardOutput);
        runExport->setStandardOutputFile(m_credentials.value("pathToFile").toString());
        runExport->start(cmd);
        runExport->waitForFinished();
        if(runExport->exitStatus() != QProcess::NormalExit){
            emit logData(tr("ОШИБКА: %1").arg(runExport->errorString()));
            emit finished();
            return;
        }
        QFile file(m_credentials.value("pathToFile").toString());
        if(!file.exists()){
            emit logData(tr("ОШИБКА: файл базы данных не создан!!!"));
            emit finished();
            return;
        }
        cmd = QString("\"%1/mysqldump.exe\" -h %2 --verbose --no-create-info --no-data --triggers --add-drop-trigger --routines --events -u %3 -p%4 %5")
                .arg(m_credentials.value("pathToMySqlTools").toString())
                .arg(m_credentials.value("host").toString())
                .arg(m_credentials.value("login").toString())
                .arg(m_credentials.value("pass").toString())
                .arg(m_credentials.value("databaseName").toString());
        emit logData(tr("Выполнение команды %1").arg(cmd));
        runExport->setStandardOutputFile(m_credentials.value("pathToFile").toString(),QIODevice::Append);
        runExport->start(cmd);
        runExport->waitForFinished();
        delete runExport;
        emit logData(tr("Экспорт базы завершен."));
    }
    if(m_mode == Enums::Import){
        emit logData(tr("Установка связи с сервером %1").arg(m_credentials.value("host").toString()));
        QSqlDatabase::removeDatabase("importexportDBworker_thread");
        QSqlDatabase db = QSqlDatabase::addDatabase(m_credentials.value("driver").toString(),"importexportDBworker_thread");
        db.setHostName(m_credentials.value("host").toString());
        db.setPort(m_credentials.value("port").toInt());
        db.setUserName(m_credentials.value("login").toString());
        db.setPassword(m_credentials.value("pass").toString());
        if(!db.open()){
            emit logData(tr("При связи с сервером mysql возникла ошибка: %1").arg(db.lastError().text()));
            emit finished();
            return;
        }
        emit logData(tr("Связь с сервером установлена"));
        QSqlQuery sql = QSqlQuery(db);
        if(!m_databases.contains(m_credentials.value("databaseName").toString())){
            emit logData(tr("Создание базы данных %1").arg(m_credentials.value("databaseName").toString()));
            if(!sql.exec(QString("CREATE DATABASE %1;").arg(m_credentials.value("databaseName").toString())))
            {
                emit logData(tr("При создании базы данных возникла ошибка: %1").arg(sql.lastError().text()));
                emit finished();
                return;
            }
            emit newBaseCreated(m_credentials.value("databaseName").toString());
            emit logData(tr("База данных создана"));
        }else{
            emit logData(tr("Очистка базы данных %1").arg(m_credentials.value("databaseName").toString()));
            if(!sql.exec(QString("DROP DATABASE %1;").arg(m_credentials.value("databaseName").toString())))
            {
                emit logData(tr("При очистке базы данных возникла ошибка: %1").arg(sql.lastError().text()));
                emit finished();
                return;
            }
            if(!sql.exec(QString("CREATE DATABASE %1;").arg(m_credentials.value("databaseName").toString())))
            {
                emit logData(tr("При очистке базы данных возникла ошибка: %1").arg(sql.lastError().text()));
                emit finished();
                return;
            }
            emit logData(tr("База данных очищена"));
        }
        emit logData(tr("Загрузка структуры и данных"));
        QString cmd = QString("\"%1/mysql.exe\" -h %2 -u %3 -p%4 %5")
                .arg(m_credentials.value("pathToMySqlTools").toString())
                .arg(m_credentials.value("host").toString())
                .arg(m_credentials.value("login").toString())
                .arg(m_credentials.value("pass").toString())
                .arg(m_credentials.value("databaseName").toString());
        emit logData(tr("Выполнение команды %1").arg(cmd));
        QProcess *runExport = new QProcess(this);
        connect(runExport,&QProcess::readyReadStandardError,this,&ImportExportDBWorker::readyReadStandardOutput);
        runExport->setStandardInputFile(m_credentials.value("pathToFile").toString());
        runExport->start(cmd);
        runExport->waitForFinished();
        if(runExport->exitStatus() != QProcess::NormalExit){
            emit logData(tr("ОШИБКА: %1").arg(runExport->errorString()));
            emit finished();
            return;
        }
        db.close();
        delete runExport;
        emit logData(tr("Загрузка структуры и данных завершена"));
    }
    emit finished();
}

void ImportExportDBWorker::readyReadStandardOutput()
{
    QProcess *p = (QProcess *)sender();
    QString buf = QString(p->readAllStandardError());
    emit logData(buf);
}
