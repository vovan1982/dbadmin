#ifndef CREATEDB_H
#define CREATEDB_H

#include <QMap>
#include <QDialog>
#include <QVariant>

namespace Ui {
class CreateDB;
}

class CreateDB : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDB(QWidget *parent = 0,
                      QStringList bases = QStringList(),
                      const QMap<QString, QVariant> &credentials = QMap<QString, QVariant>());
    ~CreateDB();

private slots:
    void on_createButton_clicked();
    void enabledForm();
    void disabledForm();
    void addNewBase(const QString &base);

signals:
    void databaseIsCreated(const QString &database);

private:
    Ui::CreateDB *ui;
    QStringList m_bases;
    QMap<QString, QVariant> m_credentials;
};

#endif // CREATEDB_H
