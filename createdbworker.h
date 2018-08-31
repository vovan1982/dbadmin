#ifndef CREATEDBWORKER_H
#define CREATEDBWORKER_H

#include <QtSql>
#include <QMap>
#include <QObject>
#include <QVariant>

class CreateDBWorker : public QObject
{
    Q_OBJECT
public:
    explicit CreateDBWorker(const QMap<QString,QVariant> &credentials = QMap<QString,QVariant>());
    ~CreateDBWorker();
private:
    QMap<QString,QVariant> m_credentials;
    int ParseSqlScriptFile(QSqlDatabase & db, const QString & fileName);
    bool createEmptyDataBase(const QSqlDatabase & db);
    bool createDataBaseWithReference(const QSqlDatabase & db);
    bool createDataBaseWithDemoData(const QSqlDatabase & db);

signals:
    void finished();
    void logData(QString text);
    void error(QString errorText);
    void newBaseCreated(const QString &baseName);

public slots:
    void process();
};

#endif // CREATEDBWORKER_H
