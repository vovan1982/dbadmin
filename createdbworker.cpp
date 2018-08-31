#include <QFile>
#include "createdbworker.h"

CreateDBWorker::CreateDBWorker(const QMap<QString, QVariant> &credentials) : m_credentials(credentials)
{

}

CreateDBWorker::~CreateDBWorker()
{

}

void CreateDBWorker::process()
{
    emit logData(tr("Установка связи с сервером"));
    QSqlDatabase::removeDatabase("new_base_connect");
    QSqlDatabase db = QSqlDatabase::addDatabase(m_credentials.value("driver").toString(),"new_base_connect");
    db.setHostName(m_credentials.value("host").toString());
    db.setPort(m_credentials.value("port").toInt());
    db.setUserName(m_credentials.value("login").toString());
    db.setPassword(m_credentials.value("pass").toString());
    if(!db.open()){
        emit error(tr("Не удалось подключится к серверу mysql: %1").arg(db.lastError().text()));
        emit finished();
        return;
    }
    emit logData(tr("Связь установлена"));
    if(m_credentials.value("emptyBase").toBool()){
        if(createEmptyDataBase(db)){
            emit newBaseCreated(m_credentials.value("databaseName").toString());
            emit logData(tr("\nПроцедура создания базы данных %1 успешно завершена").arg(m_credentials.value("databaseName").toString()));
        }
    }
    if(m_credentials.value("emptyBaseWithDirectories").toBool()){
        if(createDataBaseWithReference(db)){
            emit newBaseCreated(m_credentials.value("databaseName").toString());
            emit logData(tr("\nПроцедура создания базы данных %1 успешно завершена").arg(m_credentials.value("databaseName").toString()));
        }
    }
    if(m_credentials.value("demoBase").toBool()){
        if(createDataBaseWithDemoData(db)){
            emit newBaseCreated(m_credentials.value("databaseName").toString());
            emit logData(tr("\nПроцедура создания базы данных %1 успешно завершена").arg(m_credentials.value("databaseName").toString()));
        }
    }
    db.close();
    emit finished();
}

int CreateDBWorker::ParseSqlScriptFile(QSqlDatabase & db, const QString & fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return  0;

    QTextStream in(&file);
    QString sql = in.readAll();
    if (sql.length() == 0)
        return 0;

    QList<int> splitPoints;
    enum { IN_STR, IN_ESC, NORMAL } state = NORMAL;
    int successCount = 0;

    for (int i = 0; i < sql.length(); i++)
    {
        const int character = sql.at(i).unicode();
        switch (state)
        {
        case IN_STR:
            switch (character)
            {
            case '\'':
                state = NORMAL;
                break;
            case '\\':
                state = IN_ESC;
                break;
            }
            break;

        case IN_ESC:
            state = IN_STR;
            break;

        case NORMAL:
            switch (character)
            {
            case ';':
                splitPoints.push_back(i);
                break;

            case '\'':
                state = IN_STR;
                break;
            }
        }
    }

    splitPoints.push_back(sql.length() - 1);

    for (int i = 0, j = 0; i < splitPoints.length(); i++)
    {
        QString statement = sql.mid(j, splitPoints.at(i) - j + 1);
        j = splitPoints.at(i) + 1;

        if (statement.trimmed().length() > 0)
        {
            QSqlQuery query(db);
            if (query.exec(statement))
                successCount++;
            else{
                emit error(tr("При выполнении запроса возникла ошибка: %1").arg(query.lastError().text()));
                return 0;
            }
        }
    }

    return successCount;
}

bool CreateDBWorker::createEmptyDataBase(const QSqlDatabase & db)
{
    QString trigers;
    QString fileName(":/sql/sql/base.sql");
    QFile file(fileName);
    QSqlQuery sql = QSqlQuery(db);

    emit logData(tr("=== Процедура создания чистой базы данных ==="));

    emit logData(tr("Создание базы данных %1").arg(m_credentials.value("databaseName").toString()));
    if(!sql.exec(QString("SET NAMES 'utf8'; CREATE DATABASE %1; CHARACTER SET utf8; COLLATE utf8_general_ci;").arg(m_credentials.value("databaseName").toString())))
    {
        emit error(tr("При создании базы данных возникла ошибка: %1").arg(sql.lastError().text()));
        emit logData(tr("=== Процедура создания чистой базы данных завершена ==="));
        return false;
    }
    emit logData(tr("База данных создана"));

    emit logData(tr("Подключение к созданной базе данных"));
    QSqlDatabase::removeDatabase("new_emptybase_connect");
    QSqlDatabase db_new = QSqlDatabase::addDatabase(db.driverName(),"new_emptybase_connect");
    db_new.setHostName(db.hostName());
    db_new.setPort(db.port());
    db_new.setUserName(db.userName());
    db_new.setPassword(db.password());
    db_new.setDatabaseName(m_credentials.value("databaseName").toString());
    if(!db_new.open()){
        emit error(tr("Не удалось подключится к созданной базе данных: %1").arg(db_new.lastError().text()));
        emit logData(tr("Удаление базы данных %1").arg(m_credentials.value("databaseName").toString()));
        if(!sql.exec(QString("DROP DATABASE %1;").arg(m_credentials.value("databaseName").toString()))){
            emit error(tr("При удалении базы данных возникла ошибка: %1").arg(sql.lastError().text()));
        }else{
            emit logData(tr("База данных %1 удалена").arg(m_credentials.value("databaseName").toString()));
        }
        emit logData(tr("=== Процедура создания чистой базы данных завершена ==="));
        return false;
    }
    emit logData(tr("Подключение установлено"));

    emit logData(tr("Чтение структуры базы данных"));
    if(!file.open(QIODevice::ReadOnly)) {
        emit error(tr("При чтении структуры базы данных возникла ошибка!!!"));
        emit logData(tr("Удаление базы данных %1").arg(m_credentials.value("databaseName").toString()));
        if(!sql.exec(QString("DROP DATABASE %1;").arg(m_credentials.value("databaseName").toString()))){
            emit error(tr("При удалении базы данных возникла ошибка: %1").arg(sql.lastError().text()));
        }else{
            emit logData(tr("База данных %1 удалена").arg(m_credentials.value("databaseName").toString()));
        }
        emit logData(tr("=== Процедура создания чистой базы данных завершена ==="));
        return false;
    }
    file.close();
    emit logData(tr("Структура базы считана"));

    emit logData(tr("Создание структуры базы данных"));
    if(ParseSqlScriptFile(db_new,fileName) == 0){
        emit logData(tr("Не удалось создать структуру базы данных"));
        emit logData(tr("Удаление базы данных %1").arg(m_credentials.value("databaseName").toString()));
        if(!sql.exec(QString("DROP DATABASE %1;").arg(m_credentials.value("databaseName").toString()))){
            emit error(tr("При удалении базы данных возникла ошибка: %1").arg(sql.lastError().text()));
        }else{
            emit logData(tr("База данных %1 удалена").arg(m_credentials.value("databaseName").toString()));
        }
        emit logData(tr("=== Процедура создания чистой базы данных завершена ==="));
        return false;
    }
    emit logData(tr("Структура создана"));

    emit logData(tr("Чтение триггеров"));
    fileName = ":/sql/sql/trigers.sql";
    file.setFileName(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        emit error(tr("При чтении триггеров возникла ошибка!!!"));
        emit logData(tr("Удаление базы данных %1").arg(m_credentials.value("databaseName").toString()));
        if(!sql.exec(QString("DROP DATABASE %1;").arg(m_credentials.value("databaseName").toString()))){
            emit error(tr("При удалении базы данных возникла ошибка: %1").arg(sql.lastError().text()));
        }else{
            emit logData(tr("База данных %1 удалена").arg(m_credentials.value("databaseName").toString()));
        }
        emit logData(tr("=== Процедура создания чистой базы данных завершена ==="));
        return false;
    }
    else
    {
        trigers = file.readAll();
    }
    file.close();
    emit logData(tr("Триггеры считаны"));

    emit logData(tr("Создание триггеров"));
    QStringList listTrigers = trigers.split("$$");
    sql = QSqlQuery(db_new);
    sql.exec("DELIMITER $$");
    for(int i = 0;i<listTrigers.size();++i){
        sql.exec(QString(listTrigers.at(i)));
        sql.exec("$$");
    }
    sql.exec("DELIMITER ;");
    db_new.close();
    emit logData(tr("Триггеры созданы"));
    emit logData(tr("=== Процедура создания чистой базы данных завершена ==="));
    return true;
}

bool CreateDBWorker::createDataBaseWithReference(const QSqlDatabase &db)
{
    if(createEmptyDataBase(db)){
        emit logData(tr("\n=== Процедура заполнения справочников ==="));
        QString fileName(":/sql/sql/reference.sql");
        QFile file(fileName);
        emit logData(tr("Установка подключения к базе данных"));
        QSqlDatabase::removeDatabase("new_emptybase_connect");
        QSqlDatabase::removeDatabase("reference_new_base_connect");
        QSqlDatabase db_new = QSqlDatabase::addDatabase(db.driverName(),"reference_new_base_connect");
        db_new.setHostName(db.hostName());
        db_new.setPort(db.port());
        db_new.setUserName(db.userName());
        db_new.setPassword(db.password());
        db_new.setDatabaseName(m_credentials.value("databaseName").toString());
        if(!db_new.open()){
            emit error(tr("При заполнении справочников не удалось подключится к созданной базе данных: %1").arg(db_new.lastError().text()));
            emit logData(tr("=== Процедура заполнения справочников завершена ==="));
            return false;
        }
        emit logData(tr("Подключение установлено"));
        emit logData(tr("Чтение данных для заполнения"));
        if(!file.open(QIODevice::ReadOnly)) {
            emit error(tr("При чтении данных возникла ошибка!!!"));
            emit logData(tr("=== Процедура заполнения справочников завершена ==="));
            return false;
        }
        file.close();
        emit logData(tr("Данные считаны"));

        emit logData(tr("Заполнение справочников"));
        if(ParseSqlScriptFile(db_new,fileName) == 0){
            emit error(tr("При заполнении справочников возникла ошибка!!!"));
            emit logData(tr("=== Процедура заполнения справочников завершена ==="));
            return false;
        }
        emit logData(tr("Справочники заполнены"));
        db_new.close();
        emit logData(tr("=== Процедура заполнения справочников завершена ==="));
        return true;
    }else
        return false;
}

bool CreateDBWorker::createDataBaseWithDemoData(const QSqlDatabase & db)
{
    if(createDataBaseWithReference(db)){
        emit logData(tr("\n=== Процедура внесения демонстрационных данных ==="));
        QString fileName(":/sql/sql/demoData.sql");
        QFile file(fileName);
        emit logData(tr("Установка подключения к базе данных"));
        QSqlDatabase::removeDatabase("reference_new_base_connect");
        QSqlDatabase::removeDatabase("demoData_new_base_connect");
        QSqlDatabase db_new = QSqlDatabase::addDatabase(db.driverName(),"demoData_new_base_connect");
        db_new.setHostName(db.hostName());
        db_new.setPort(db.port());
        db_new.setUserName(db.userName());
        db_new.setPassword(db.password());
        db_new.setDatabaseName(m_credentials.value("databaseName").toString());
        if(!db_new.open()){
            emit error(tr("При внесении демонстрационных данных не удалось подключится к созданной базе данных: %1").arg(db_new.lastError().text()));
            emit logData(tr("=== Процедура внесения демонстрационных данных завершена ==="));
            return false;
        }
        emit logData(tr("Подключение установлено"));
        emit logData(tr("Чтение демонстрационных данных для внесения"));
        if(!file.open(QIODevice::ReadOnly)) {
            emit error(tr("Не удалось прочитать демонстрационные данные!!!"));
            emit logData(tr("=== Процедура внесения демонстрационных данных завершена ==="));
            return false;
        }
        file.close();
        emit logData(tr("Данные считаны"));

        emit logData(tr("Внесение данных"));
        if(ParseSqlScriptFile(db_new,fileName) == 0){
            emit error(tr("При внесения демонстрационных данных возникла ошибка!!!"));
            emit logData(tr("=== Процедура внесения демонстрационных данных завершена ==="));
            return false;
        }
        db_new.close();
        emit logData(tr("Данные внесены"));
        emit logData(tr("=== Процедура внесения демонстрационных данных завершена ==="));
        return true;
    }else
        return false;
}
