#ifndef OPTIMIZEDBWORKER_H
#define OPTIMIZEDBWORKER_H

#include <QMap>
#include <QtSql>
#include <QObject>
#include <QVariant>

class OptimizeDBWorker : public QObject
{
    Q_OBJECT
public:
    OptimizeDBWorker(const QMap<QString,QVariant> &connectionData = QMap<QString,QVariant>());
    ~OptimizeDBWorker();
private:
    QMap<QString, QVariant> m_connectionData;
signals:
    void finished();
    void progressValue(int progress);
    void progressValueCount(int count);
    void error(QString errorText);
    void successfully();
public slots:
    void process();
};

#endif // OPTIMIZEDBWORKER_H
