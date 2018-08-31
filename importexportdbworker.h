#ifndef IMPORTEXPORTDBWORKER_H
#define IMPORTEXPORTDBWORKER_H

#include <QMap>
#include <QObject>
#include <QVariant>
#include "enums.h"

class ImportExportDBWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImportExportDBWorker(const QMap<QString,QVariant> &credentials = QMap<QString,QVariant>(),
                                  const QStringList databases = QStringList(),
                                  Enums::ImportExportMode mode = Enums::Export);
    ~ImportExportDBWorker();

private:
    QMap<QString,QVariant> m_credentials;
    QStringList m_databases;
    Enums::ImportExportMode m_mode;

signals:
    void finished();
    void logData(QString text);
    void error(QString errorText);
    void newBaseCreated(QString baseName);

public slots:
    void process();
    void readyReadStandardOutput();
};

#endif // IMPORTEXPORTDBWORKER_H
