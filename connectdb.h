#ifndef CONNECTDB_H
#define CONNECTDB_H

#include <QMap>
#include <QVariant>
#include <QDialog>

namespace Ui {
class ConnectDB;
}

class ConnectDB : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDB(QWidget *parent = 0,QStringList driversDB = QStringList());
    ~ConnectDB();
signals:
    void connectEstablished(const QMap<QString,QVariant> credentials);
private slots:
    void on_connectButton_clicked();
private:
    Ui::ConnectDB *ui;
};

#endif // CONNECTDB_H
