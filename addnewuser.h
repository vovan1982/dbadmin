#ifndef ADDNEWUSER_H
#define ADDNEWUSER_H

#include <QMap>
#include <QtSql>
#include <QDialog>
#include <QVariant>

namespace Ui {
class AddNewUser;
}

class AddNewUser : public QDialog
{
    Q_OBJECT

public:
    explicit AddNewUser(QWidget *parent = 0,QStringList databases = QStringList(), QMap<QString,QVariant> connectionCredentials = QMap<QString,QVariant>());
    ~AddNewUser();

private slots:
    void on_createButton_clicked();
signals:
    void createdNewUser(QMap<QString,QString> user);
private:
    QMap<QString,QVariant> m_connectionCredentials;
    Ui::AddNewUser *ui;
};

#endif // ADDNEWUSER_H
