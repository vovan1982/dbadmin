#ifndef DBADMIN_H
#define DBADMIN_H

#include <QMap>
#include <QVariant>
#include <QMainWindow>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QDataWidgetMapper>

namespace Ui {
class DBAdmin;
}

class DBAdmin : public QMainWindow
{
    Q_OBJECT

public:
    explicit DBAdmin(QWidget *parent = 0);
    ~DBAdmin();

private slots:
    void on_actionConnectToServer_triggered();
    void setConnectionCredentials(const QMap<QString,QVariant> credentials);
    void on_actionDisconnectFromServer_triggered();
    void on_actionCreateUser_triggered();
    void addCreatedUser(QMap<QString,QString> user);
    void on_actionDeleteUser_triggered();
    void on_comboBoxDataBases_currentIndexChanged(int index);
    void on_setAccessButton_clicked();
    void on_lineEditLogin_textEdited(const QString );
    void setCurrentUserData();
    void on_lineEditHost_textEdited(const QString );
    void on_lineEditNewPass_textEdited(const QString );
    void on_lineEditConfirmPass_textEdited(const QString );
    void on_revertEditButton_clicked();
    void on_saveEditButton_clicked();
    void on_actionCreateDB_triggered();
    void addCreatedDataBase(const QString &database);
    void on_actionDeleteDB_triggered();
    void on_actionOptimizekDB_triggered();
    void on_actionExportDB_triggered();

    void on_actionImportDB_triggered();

    void on_actionAbout_triggered();

private:
    Ui::DBAdmin *ui;
    QMap<QString,QVariant> connectionCredentials;
    QStringListModel *modelDB;
    QStandardItemModel *modelUser;
    QDataWidgetMapper *mapper;
    QStringList DBDrivers;
    QMap<QString,QString> currentUserData;
    bool compareCurrentUserData();
    void setEnabledButton(bool enabled);
};

#endif // DBADMIN_H
