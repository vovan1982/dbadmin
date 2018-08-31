#include <QMessageBox>
#include "addnewuser.h"
#include "ui_addnewuser.h"

AddNewUser::AddNewUser(QWidget *parent, QStringList databases, QMap<QString,QVariant> connectionCredentials) :
    QDialog(parent),
    m_connectionCredentials(connectionCredentials),
    ui(new Ui::AddNewUser)
{
    ui->setupUi(this);
    ui->database->addItems(databases);
}

AddNewUser::~AddNewUser()
{
    delete ui;
}

void AddNewUser::on_createButton_clicked()
{
    if(ui->login->text().isNull() || ui->login->text().isEmpty()){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не заполненно поле \"Логин\"!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->host->text().isNull() || ui->host->text().isEmpty()){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не аполненно поле \"Хост\"!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->pass->text().isNull() || ui->pass->text().isEmpty()){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не заполненно поле \"Пароль\"!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->confirmPass->text().isNull() || ui->confirmPass->text().isEmpty()){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не заполненно поле \"Подтверждение пароля\"!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->pass->text().compare(ui->confirmPass->text()) !=0){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Пароли указанные в полях \"Пароль\" и \"Подтверждение пароля\" не совпадают!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->database->currentIndex() < 0){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не выбранна база данных для назначения прав!!!"),
                              tr("Закрыть"));
        return;
    }

    QSqlDatabase db;
    if (QSqlDatabase::contains("create_new_user"))
        db = QSqlDatabase::database("create_new_user");
    else
        db = QSqlDatabase::addDatabase(m_connectionCredentials.value("driver").toString(),"create_new_user");
    db.setHostName(m_connectionCredentials.value("host").toString());
    db.setPort(m_connectionCredentials.value("port").toInt());
    db.setUserName(m_connectionCredentials.value("login").toString());
    db.setPassword(m_connectionCredentials.value("pass").toString());
    if (!db.open()) {
        QMessageBox::critical(this,tr("Ошибка подключения"),
                              tr("При подключении к базе данных возникла ошибка: %1").arg(db.lastError().text()),
                              tr("Закрыть"));
    }else{
        QSqlQuery sql = QSqlQuery(db);
        if(!sql.exec(QString("CREATE USER '%1'@'%2' IDENTIFIED BY '%3';").arg(ui->login->text()).arg(ui->host->text()).arg(ui->pass->text())))
        {
            QMessageBox::critical(this,tr("Ошибка"),
                                  tr("При создании пользователя возникла ошибка: %1").arg(sql.lastError().text()),
                                  tr("Закрыть"));
            return;
        }
        QMap<QString,QString> user;
        user["displayName"] = ui->login->text() + "@" + ui->host->text();
        user["name"] = ui->login->text();
        user["host"] = ui->host->text();
        if(ui->fullAccess->isChecked()){
            if(!sql.exec(QString("GRANT ALL PRIVILEGES ON %1.* TO '%2'@'%3';FLUSH PRIVILEGES;").arg(ui->database->itemText(ui->database->currentIndex())).arg(ui->login->text()).arg(ui->host->text())))
            {
                QMessageBox::critical(this,tr("Ошибка"),
                                      tr("При назначении прав пользователю возникла ошибка: %1").arg(sql.lastError().text()),
                                      tr("Закрыть"));
                return;
            }else
                user["grant"] = QString("GRANT USAGE ON *.* TO `%1`@`%2`\nGRANT ALL PRIVILEGES ON `%3`.* TO `%1`@`%2`").arg(ui->login->text()).arg(ui->host->text()).arg(ui->database->itemText(ui->database->currentIndex()));
        }else{
            if(!sql.exec(QString("GRANT SELECT ON %1.* TO '%2'@'%3';FLUSH PRIVILEGES;").arg(ui->database->itemText(ui->database->currentIndex())).arg(ui->login->text()).arg(ui->host->text())))
            {
                QMessageBox::critical(this,tr("Ошибка"),
                                      tr("При назначении прав пользователю возникла ошибка: %1").arg(sql.lastError().text()),
                                      tr("Закрыть"));
                return;
            }
            else
                user["grant"] = QString("GRANT USAGE ON *.* TO `%1`@`%2`\nGRANT SELECT ON `%3`.* TO `%1`@`%2`").arg(ui->login->text()).arg(ui->host->text()).arg(ui->database->itemText(ui->database->currentIndex()));
        }
        emit createdNewUser(user);
        int ansver = QMessageBox::question(this,tr("Создание пользоватля"),
                              tr("Создание пользователя успешно завершено.\nХотите создать ещё одного пользователя?"),
                              tr("Да"),
                              tr("Нет"));
        if(ansver == 1){
            db.close();
            accept();
        }
        db.close();
        ui->login->setText("");
        ui->host->setText("%");
        ui->pass->setText("");
        ui->confirmPass->setText("");
        ui->database->setCurrentIndex(0);
        ui->fullAccess->setChecked(true);
    }
}
