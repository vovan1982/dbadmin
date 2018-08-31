#ifndef IMPORTEXPORTDB_H
#define IMPORTEXPORTDB_H

#include <QMap>
#include <QDialog>
#include <QVariant>
#include "enums.h"


namespace Ui {
class ImportExportDB;
}

class ImportExportDBWorker;

class ImportExportDB : public QDialog
{
    Q_OBJECT

public:
    explicit ImportExportDB(QWidget *parent = 0,
                            const QStringList databases = QStringList(),
                            const QMap<QString, QVariant> &credentials = QMap<QString, QVariant>(),
                            Enums::ImportExportMode mode = Enums::Export);
    ~ImportExportDB();

private:
    QStringList m_databases;
    Enums::ImportExportMode m_mode;
    QMap<QString, QVariant> m_credentials;
    Ui::ImportExportDB *ui;
    void runThread(ImportExportDBWorker *worker);

signals:
    void newDatabaseCreated(QString baseName);

private slots:
    void on_selectFileButton_clicked();
    void on_clearPatchButton_clicked();
    void on_applyButton_clicked();
    void enabledForm();
    void disabledForm();
    void addNewDatabase(QString base);


};

#endif // IMPORTEXPORTDB_H
