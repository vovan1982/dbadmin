#include <QtSql>
#include <QStringList>
#include <QMessageBox>
#include "dbadmin.h"
#include "ui_dbadmin.h"
#include "connectdb.h"
#include "addnewuser.h"
#include "createdb.h"
#include "optimizedb.h"
#include "enums.h"
#include "importexportdb.h"
#include "about.h"

DBAdmin::DBAdmin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DBAdmin)
{
    ui->setupUi(this);
    ui->splitterUser->setStretchFactor(1,1);
    ui->splitterCentral->setStretchFactor(1,1);

    DBDrivers.append("QMYSQL");
}

DBAdmin::~DBAdmin()
{
    delete ui;
}

void DBAdmin::on_actionConnectToServer_triggered()
{
    QStringList databases;
    currentUserData = QMap<QString,QString>();
    ConnectDB *cdb = new ConnectDB(this,DBDrivers);
    int nWidth = 286;
    int nHeight = 147;
    connect(cdb,&ConnectDB::connectEstablished,this,&DBAdmin::setConnectionCredentials);
    cdb->setAttribute(Qt::WA_DeleteOnClose);
    cdb->setGeometry(this->pos().x() + this->width()/2 - nWidth/2,
                   this->pos().y() + this->height()/2 - nHeight/2,
                   nWidth, nHeight);
    if(cdb->exec()){
        QSqlDatabase::removeDatabase("qt_sql_default_connection");
        QSqlDatabase db = QSqlDatabase::addDatabase(connectionCredentials.value("driver").toString());
        db.setHostName(connectionCredentials.value("host").toString());
        db.setPort(connectionCredentials.value("port").toInt());
        db.setUserName(connectionCredentials.value("login").toString());
        db.setPassword(connectionCredentials.value("pass").toString());
        if(!db.open()){
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При связи с базой данных возникла ошибка: %1").arg(db.lastError().text()),
                                  tr("Закрыть"));
            return;
        }
        ui->actionConnectToServer->setIcon(QIcon(":/32x32/ico/connect_established_32x32.png"));
        ui->actionConnectToServer->setEnabled(false);
        ui->actionDisconnectFromServer->setEnabled(true);
        ui->groupBoxDB->setEnabled(true);
        ui->groupBoxUsers->setEnabled(true);
        ui->actionCreateDB->setEnabled(true);
        ui->actionCreateUser->setEnabled(true);
        ui->actionImportDB->setEnabled(true);

        QSqlQuery sql = QSqlQuery();
        if(!sql.exec("SHOW DATABASES;"))
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При выполнении запроса к базе данных возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }
        while (sql.next())
        {
            if(sql.value(0).toString().compare("mysql",Qt::CaseInsensitive) != 0 &&
                    sql.value(0).toString().compare("performance_schema",Qt::CaseInsensitive) != 0 &&
                    sql.value(0).toString().compare("information_schema",Qt::CaseInsensitive) != 0 &&
                    sql.value(0).toString().compare("sys",Qt::CaseInsensitive) != 0)
                databases.append(sql.value(0).toString());
        }
        modelDB = new QStringListModel(databases);
        ui->listViewDataBases->setModel(modelDB);
        ui->comboBoxDataBases->addItem(tr("Выберите базу"));
        ui->comboBoxDataBases->addItems(databases);
        if(modelDB->rowCount() > 0){
            ui->listViewDataBases->setCurrentIndex(modelDB->index(0));
            ui->actionDeleteDB->setEnabled(true);
            ui->actionExportDB->setEnabled(true);
            ui->actionOptimizekDB->setEnabled(true);
        }

        if(!sql.exec("select host, user from mysql.user WHERE user NOT LIKE 'mysql.%' AND user NOT LIKE 'root';"))
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При выполнении запроса к базе данных возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }
        modelUser = new QStandardItemModel(ui->usersView);
        modelUser->blockSignals(false);
        QStringList userModelHeader;
        userModelHeader.append(tr("Пользователь"));
        userModelHeader.append(tr("Логин"));
        userModelHeader.append(tr("Хост"));
        userModelHeader.append(tr("Права"));
        modelUser->setHorizontalHeaderLabels(userModelHeader);
        while (sql.next())
        {
            QSqlQuery sql2 = QSqlQuery();
            QList<QStandardItem *> items;
            items.append(new QStandardItem(sql.value(1).toString()+"@"+sql.value(0).toString()));
            items.append(new QStandardItem(sql.value(1).toString()));
            items.append(new QStandardItem(sql.value(0).toString()));
            if(sql2.exec(QString("SHOW GRANTS FOR '%1'@'%2';")
                         .arg(sql.value(1).toString()).arg(sql.value(0).toString()))){
                if(sql2.size()>0){
                    QString grants = "";
                    while (sql2.next()) {
                        grants += sql2.value(0).toString()+"\n";
                    }
                    items.append(new QStandardItem(grants));
                }
            }else{
                items.append(new QStandardItem(""));
            }
            modelUser->appendRow(items);
        }
        mapper = new QDataWidgetMapper(this);
        mapper->setModel(modelUser);
        mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
        mapper->addMapping(ui->lineEditLogin, 1);
        mapper->addMapping(ui->lineEditHost, 2);
        mapper->addMapping(ui->showGrantsUser,3);
        ui->usersView->setModel(modelUser);
        connect(ui->usersView->selectionModel(),&QItemSelectionModel::currentRowChanged,mapper,&QDataWidgetMapper::setCurrentModelIndex);
        connect(ui->usersView->selectionModel(),&QItemSelectionModel::currentRowChanged,mapper,&QDataWidgetMapper::revert);
        connect(ui->usersView->selectionModel(),&QItemSelectionModel::currentRowChanged,this,&DBAdmin::setCurrentUserData);
        if(modelUser->rowCount()>0)
        {
            ui->usersView->setCurrentIndex(modelUser->index(0,0));
            ui->actionDeleteUser->setEnabled(true);
            ui->userDataTabWidget->setEnabled(true);
        }
    }
}

void DBAdmin::setConnectionCredentials(const QMap<QString,QVariant> credentials)
{
    connectionCredentials = credentials;
}

void DBAdmin::on_actionDisconnectFromServer_triggered()
{
    ui->actionConnectToServer->setIcon(QIcon(":/32x32/ico/connect_creating_32x32.png"));
    ui->actionConnectToServer->setEnabled(true);
    ui->actionDisconnectFromServer->setEnabled(false);
    ui->actionCreateDB->setEnabled(false);
    ui->actionDeleteDB->setEnabled(false);
    ui->actionImportDB->setEnabled(false);
    ui->actionExportDB->setEnabled(false);
    ui->actionOptimizekDB->setEnabled(false);
    ui->actionCreateUser->setEnabled(false);
    ui->actionDeleteUser->setEnabled(false);
    ui->groupBoxDB->setEnabled(false);
    ui->groupBoxUsers->setEnabled(false);
    ui->listViewDataBases->setModel(new QStringListModel());
    ui->usersView->setModel(new QStringListModel());
    delete modelUser;
    delete modelDB;
    delete mapper;
    connectionCredentials.clear();
    ui->lineEditLogin->setText("");
    ui->lineEditHost->setText("");
    ui->lineEditNewPass->setText("");
    ui->lineEditConfirmPass->setText("");
    ui->comboBoxDataBases->clear();
}

void DBAdmin::on_actionCreateUser_triggered()
{
    QStringList databases;
    for(int i=1;i<ui->comboBoxDataBases->count();++i)
    {
        databases.append(ui->comboBoxDataBases->itemText(i));
    }
    AddNewUser *anu = new AddNewUser(this,databases,connectionCredentials);
    connect(anu,&AddNewUser::createdNewUser,this,&DBAdmin::addCreatedUser);
    int nWidth = 379;
    int nHeight = 264;
    anu->setAttribute(Qt::WA_DeleteOnClose);
    anu->setGeometry(this->pos().x() + this->width()/2 - nWidth/2,
                   this->pos().y() + this->height()/2 - nHeight/2,
                   nWidth, nHeight);
    anu->exec();
}

void DBAdmin::addCreatedUser(QMap<QString, QString> user)
{
    QList<QStandardItem *> items;
    items.append(new QStandardItem(user.value("displayName")));
    items.append(new QStandardItem(user.value("name")));
    items.append(new QStandardItem(user.value("host")));
    items.append(new QStandardItem(user.value("grant")));
    modelUser->appendRow(items);
    if(!ui->usersView->currentIndex().isValid()){
        ui->usersView->setCurrentIndex(modelUser->index(0,0));
        ui->actionDeleteUser->setEnabled(true);
        ui->userDataTabWidget->setEnabled(true);
    }
}

void DBAdmin::on_actionDeleteUser_triggered()
{
    int ansver = QMessageBox::critical(this,tr("Ошибка"),
                          tr("Вы действительно хотите удалить выбранного пользователя?"),
                          tr("Да"),
                          tr("Нет"));
    if(ansver == 0){
        QSqlQuery sql = QSqlQuery();
        if(!sql.exec(QString("DROP USER '%1'@'%2';")
                     .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                     .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString()))
                )
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При удалении пользователя возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }
        modelUser->removeRow(ui->usersView->currentIndex().row());
        if(!ui->usersView->currentIndex().isValid()){
            ui->actionDeleteUser->setEnabled(false);
            ui->userDataTabWidget->setEnabled(false);
            ui->lineEditLogin->setText("");
            ui->lineEditHost->setText("");
            ui->lineEditNewPass->setText("");
            ui->lineEditConfirmPass->setText("");
            ui->showGrantsUser->setPlainText("");
        }
    }
}

void DBAdmin::on_comboBoxDataBases_currentIndexChanged(int index)
{
    if(index > 0){
        ui->groupBoxAccess->setEnabled(true);
        ui->setAccessButton->setEnabled(true);
    }else{
        ui->groupBoxAccess->setEnabled(false);
        ui->setAccessButton->setEnabled(false);
    }
}

void DBAdmin::on_setAccessButton_clicked()
{
    QSqlQuery sql = QSqlQuery();
    if(ui->isFullAccess->isChecked()){
        sql.exec(QString("REVOKE ALL ON %1.* FROM '%2'@'%3';").arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                 .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                 .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString()));
        if(!sql.exec(QString("GRANT ALL PRIVILEGES ON %1.* TO '%2'@'%3';FLUSH PRIVILEGES;")
                     .arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                     .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                     .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString())))
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При назначении прав пользователю возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }else{
            QString grantsStr = ui->showGrantsUser->toPlainText().trimmed();
            if(grantsStr.contains(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()),Qt::CaseInsensitive)){
                QStringList text = grantsStr.split("\n");
                grantsStr = "";
                for(int i = 0; i<text.size();++i){
                    if(QString(text.at(i)).contains(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))){
                        grantsStr += QString("GRANT ALL PRIVILEGES ON `%1`.* TO `%2`@`%3`\n")
                                .arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                                .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                                .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString());
                    }else{
                        grantsStr += text.at(i) + "\n";
                    }
                }
                ui->showGrantsUser->clear();
                ui->showGrantsUser->appendPlainText(grantsStr.trimmed());
                mapper->submit();
            }else{
                grantsStr += QString("\nGRANT ALL PRIVILEGES ON `%1`.* TO `%2`@`%3`")
                        .arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                        .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                        .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString());
                ui->showGrantsUser->clear();
                ui->showGrantsUser->appendPlainText(grantsStr);
                mapper->submit();
            }
        }
    }
    if(ui->isReadOnly->isChecked()){
        sql.exec(QString("REVOKE ALL ON %1.* FROM '%2'@'%3';").arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                 .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                 .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString()));
        if(!sql.exec(QString("GRANT SELECT ON %1.* TO '%2'@'%3';FLUSH PRIVILEGES;").arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                     .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                     .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString())))
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При назначении прав пользователю возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }else{
            QString grantsStr = ui->showGrantsUser->toPlainText().trimmed();
            if(grantsStr.contains(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()),Qt::CaseInsensitive)){
                QStringList text = grantsStr.split("\n");
                grantsStr = "";
                for(int i = 0; i<text.size();++i){
                    if(QString(text.at(i)).contains(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))){
                        grantsStr += QString("GRANT SELECT ON `%1`.* TO `%2`@`%3`\n")
                                .arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                                .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                                .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString());
                    }else{
                        grantsStr += text.at(i) + "\n";
                    }
                }
                ui->showGrantsUser->clear();
                ui->showGrantsUser->appendPlainText(grantsStr.trimmed());
                mapper->submit();
            }else{
                grantsStr += QString("\nGRANT SELECT ON `%1`.* TO `%2`@`%3`")
                        .arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                        .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                        .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString());
                ui->showGrantsUser->clear();
                ui->showGrantsUser->appendPlainText(grantsStr);
                mapper->submit();
            }
        }
    }
    if(ui->isRemoveGrants->isChecked()){
        if(!sql.exec(QString("REVOKE ALL ON %1.* FROM '%2'@'%3';").arg(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))
                 .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString())
                    .arg(modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString()))){
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При удалении прав пользователю возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }else{
            QString grantsStr = ui->showGrantsUser->toPlainText().trimmed();
            if(grantsStr.contains(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()),Qt::CaseInsensitive)){
                QStringList text = grantsStr.split("\n");
                grantsStr = "";
                for(int i = 0; i<text.size();++i){
                    if(!QString(text.at(i)).contains(ui->comboBoxDataBases->itemText(ui->comboBoxDataBases->currentIndex()))){
                        if(!QString(text.at(i)).isEmpty())
                            grantsStr += text.at(i) + "\n";
                    }
                }
                ui->showGrantsUser->clear();
                ui->showGrantsUser->appendPlainText(grantsStr.trimmed());
                mapper->submit();
            }
        }
    }
    ui->comboBoxDataBases->setCurrentIndex(0);
}

void DBAdmin::on_lineEditLogin_textEdited(const QString)
{
    if(!compareCurrentUserData()){
        setEnabledButton(true);
    }else{
        setEnabledButton(false);
    }
}

void DBAdmin::setCurrentUserData()
{
    currentUserData["displayName"] = modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),0)).toString();
    currentUserData["login"] = modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),1)).toString();
    currentUserData["host"] = modelUser->data(modelUser->index(ui->usersView->currentIndex().row(),2)).toString();
    ui->lineEditNewPass->setText("");
    ui->lineEditConfirmPass->setText("");
    setEnabledButton(false);
}

bool DBAdmin::compareCurrentUserData()
{
    if(ui->lineEditLogin->text().compare(currentUserData.value("login")) != 0){
        return false;
    }
    if(ui->lineEditHost->text().compare(currentUserData.value("host")) != 0){
        return false;
    }
    if(!ui->lineEditNewPass->text().isEmpty() && !ui->lineEditNewPass->text().isNull()){
        return false;
    }
    if(!ui->lineEditConfirmPass->text().isEmpty() && !ui->lineEditConfirmPass->text().isNull()){
        return false;
    }
    return true;
}

void DBAdmin::setEnabledButton(bool enabled)
{
    ui->saveEditButton->setEnabled(enabled);
    ui->revertEditButton->setEnabled(enabled);
}

void DBAdmin::on_lineEditHost_textEdited(const QString)
{
    if(!compareCurrentUserData()){
        setEnabledButton(true);
    }else{
        setEnabledButton(false);
    }
}

void DBAdmin::on_lineEditNewPass_textEdited(const QString)
{
    if(!compareCurrentUserData()){
        setEnabledButton(true);
    }else{
        setEnabledButton(false);
    }
}

void DBAdmin::on_lineEditConfirmPass_textEdited(const QString)
{
    if(!compareCurrentUserData()){
        setEnabledButton(true);
    }else{
        setEnabledButton(false);
    }
}

void DBAdmin::on_revertEditButton_clicked()
{
    mapper->revert();
    ui->lineEditNewPass->setText("");
    ui->lineEditConfirmPass->setText("");
    setEnabledButton(false);
}

void DBAdmin::on_saveEditButton_clicked()
{
    bool passwordMustUpdated = false;
    if((!ui->lineEditNewPass->text().isNull() && !ui->lineEditNewPass->text().isEmpty()) ||
            (!ui->lineEditConfirmPass->text().isNull() && !ui->lineEditConfirmPass->text().isEmpty()))
    {
        if(ui->lineEditNewPass->text().compare(ui->lineEditConfirmPass->text()) != 0){
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("Пароль и подтверждение пароля не совпадают!!!"),
                                  tr("Закрыть"));
            return;
        }else{
            passwordMustUpdated = true;
        }
    }
    if(ui->lineEditLogin->text().compare(currentUserData.value("login")) != 0 || ui->lineEditHost->text().compare(currentUserData.value("host")) != 0)
    {
        for(int i = 0;i<modelUser->rowCount();++i){
            if(i != ui->usersView->currentIndex().row()){
                if ((ui->lineEditLogin->text().compare(modelUser->data(modelUser->index(i,1)).toString()) == 0 &&
                        ui->lineEditHost->text().compare(modelUser->data(modelUser->index(i,2)).toString()) == 0) ||
                        ui->lineEditLogin->text().compare("root") == 0 ||
                        ui->lineEditLogin->text().compare("mysql") == 0){
                    QMessageBox::critical(this,tr("Ошибка"),
                                          tr("Пользователь с указанным логином и хостом уже существует!!!"),
                                          tr("Закрыть"));
                    return;
                }
            }
        }
        QSqlQuery sql = QSqlQuery();
        if(!sql.exec(QString("RENAME USER '%1'@'%2' TO '%3'@'%4';")
                     .arg(currentUserData.value("login"))
                     .arg(currentUserData.value("host"))
                     .arg(ui->lineEditLogin->text())
                     .arg(ui->lineEditHost->text())))
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При переименовании пользователя возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }else{
            modelUser->setData(modelUser->index(ui->usersView->currentIndex().row(),0),QString(ui->lineEditLogin->text() + "@" + ui->lineEditHost->text()));
            QString grantsStr = ui->showGrantsUser->toPlainText().trimmed();
            QStringList text = grantsStr.split("\n");
            grantsStr = "";
            for(int i = 0; i<text.size();++i){
                grantsStr += QString(QString(text.at(i)).replace(currentUserData.value("login"),ui->lineEditLogin->text())).replace(currentUserData.value("host"),ui->lineEditHost->text()) + "\n";
            }
            ui->showGrantsUser->clear();
            ui->showGrantsUser->appendPlainText(grantsStr.trimmed());
            mapper->submit();

        }
    }
    if(passwordMustUpdated){
        QSqlQuery sql = QSqlQuery();
        if(!sql.exec(QString("SET PASSWORD FOR %1 = '%2';").arg(ui->lineEditLogin->text()).arg(ui->lineEditNewPass->text()))){
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При установке нового пароля пользователю возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
        }
    }
    setCurrentUserData();
}

void DBAdmin::on_actionCreateDB_triggered()
{
    QStringList databases = modelDB->stringList();
    CreateDB *crdb = new CreateDB(this,databases,connectionCredentials);
    connect(crdb,&CreateDB::databaseIsCreated,this,&DBAdmin::addCreatedDataBase);
    crdb->setAttribute(Qt::WA_DeleteOnClose);
    int nWidth = 370;
    int nHeight = 346;
    crdb->setGeometry(this->pos().x() + this->width()/2 - nWidth/2,
                   this->pos().y() + this->height()/2 - nHeight/2,
                   nWidth, nHeight);
    crdb->exec();
}

void DBAdmin::addCreatedDataBase(const QString &database)
{
    modelDB->insertRow(modelDB->rowCount());
    modelDB->setData(modelDB->index(modelDB->rowCount()-1),database);
    ui->comboBoxDataBases->addItem(database);
    if(!ui->listViewDataBases->currentIndex().isValid()){
        ui->listViewDataBases->setCurrentIndex(modelDB->index(0));
        ui->actionDeleteDB->setEnabled(true);
        ui->actionExportDB->setEnabled(true);
        ui->actionOptimizekDB->setEnabled(true);
    }
}

void DBAdmin::on_actionDeleteDB_triggered()
{
    int ansver = QMessageBox::critical(this,tr("Ошибка"),
                          tr("Вы действительно хотите удалить выбранную базу данных?"),
                          tr("Да"),
                          tr("Нет"));
    if(ansver == 0){
        QSqlQuery sql = QSqlQuery();
        if(!sql.exec(QString("DROP DATABASE %1;")
                     .arg(modelDB->data(modelDB->index(ui->listViewDataBases->currentIndex().row())).toString()))
                )
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При удалении базы данных возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }
        ui->comboBoxDataBases->removeItem(ui->comboBoxDataBases->findText(modelDB->data(modelDB->index(ui->listViewDataBases->currentIndex().row())).toString()));
        modelDB->removeRow(ui->listViewDataBases->currentIndex().row());
        if(!ui->listViewDataBases->currentIndex().isValid()){
            ui->actionDeleteDB->setEnabled(false);
            ui->actionExportDB->setEnabled(false);
            ui->actionOptimizekDB->setEnabled(false);
        }
    }
}

void DBAdmin::on_actionOptimizekDB_triggered()
{
    QMap<QString, QVariant> connectionData = connectionCredentials;
    connectionData["databaseName"] = modelDB->data(modelDB->index(ui->listViewDataBases->currentIndex().row())).toString();
    OptimizeDB *odb = new OptimizeDB(this,connectionData);
    odb->setAttribute(Qt::WA_DeleteOnClose);
    odb->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    int nWidth = 258;
    int nHeight = 58;
    odb->setGeometry(this->pos().x() + this->width()/2 - nWidth/2,
                   this->pos().y() + this->height()/2 - nHeight/2,
                   nWidth, nHeight);
    odb->exec();
}

void DBAdmin::on_actionExportDB_triggered()
{
    ImportExportDB *iedb = new ImportExportDB(this,modelDB->stringList(),connectionCredentials);
    iedb->setAttribute(Qt::WA_DeleteOnClose);
    int nWidth = 360;
    int nHeight = 197;
    iedb->setGeometry(this->pos().x() + this->width()/2 - nWidth/2,
                   this->pos().y() + this->height()/2 - nHeight/2,
                   nWidth, nHeight);
    iedb->exec();
}

void DBAdmin::on_actionImportDB_triggered()
{
    ImportExportDB *iedb = new ImportExportDB(this,modelDB->stringList(),connectionCredentials,Enums::Import);
    connect(iedb,&ImportExportDB::newDatabaseCreated,this,&DBAdmin::addCreatedDataBase);
    iedb->setAttribute(Qt::WA_DeleteOnClose);
    int nWidth = 360;
    int nHeight = 197;
    iedb->setGeometry(this->pos().x() + this->width()/2 - nWidth/2,
                   this->pos().y() + this->height()/2 - nHeight/2,
                   nWidth, nHeight);
    iedb->exec();
}

void DBAdmin::on_actionAbout_triggered()
{
    About *a = new About(this);
    a->setAttribute(Qt::WA_DeleteOnClose);
    a->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    int nWidth = 353;
    int nHeight = 226;
    a->setGeometry(this->pos().x() + this->width()/2 - nWidth/2,
                   this->pos().y() + this->height()/2 - nHeight/2,
                   nWidth, nHeight);
    a->exec();
}
