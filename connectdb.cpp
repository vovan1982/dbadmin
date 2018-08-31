#include <QtSql>
#include <QMessageBox>
#include <QSettings>
#include "connectdb.h"
#include "ui_connectdb.h"

ConnectDB::ConnectDB(QWidget *parent, QStringList driversDB) :
    QDialog(parent),
    ui(new Ui::ConnectDB)
{
    ui->setupUi(this);
    ui->driver->addItems(driversDB);

    // Устанавливаем предыдущие значения для подключения к БД
    QSettings settings( QApplication::applicationDirPath()+"/connection.ini", QSettings::IniFormat );
    ui->login->setText(settings.value("Connection/UserName","").toString());
    ui->port->setValue(settings.value("Connection/Port",3306).toInt());
    ui->host->setText(settings.value("Connection/HostName","localhost").toString());
    ui->pass->setText(settings.value("Connection/Password","").toString());
    ui->driver->setCurrentIndex(ui->driver->findText(settings.value("Connection/Driver","").toString()));
}

ConnectDB::~ConnectDB()
{
    delete ui;
}

void ConnectDB::on_connectButton_clicked()
{
    QSqlDatabase::removeDatabase("qt_sql_default_connection");
    if(ui->login->text().isNull() || ui->login->text().isEmpty()){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не заполненно поле \"Логин\"!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->pass->text().isNull() || ui->pass->text().isEmpty()){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не заполненно поле \"Пароль\"!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->host->text().isNull() || ui->host->text().isEmpty()){
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не указан сервер для подключения!!!"),
                              tr("Закрыть"));
        return;
    }

    QMap<QString,QVariant> connectionData;
    QSqlDatabase db = QSqlDatabase::addDatabase(ui->driver->itemText(ui->driver->currentIndex()));
    db.setHostName(ui->host->text());
    db.setPort(ui->port->value());
    db.setUserName(ui->login->text());
    db.setPassword(ui->pass->text());
    if (!db.open()) {
        QMessageBox::critical(this,tr("Ошибка подключения"),
                              tr("При подключении к базе данных возникла ошибка: %1").arg(db.lastError().text()),
                              tr("Закрыть"));

    }else{
        connectionData["host"] = ui->host->text();
        connectionData["port"] = ui->port->value();
        connectionData["driver"] = ui->driver->itemText(ui->driver->currentIndex());
        connectionData["login"] = ui->login->text();
        connectionData["pass"] = ui->pass->text();
        emit connectEstablished(connectionData);
        db.close();
        QSettings settings( QApplication::applicationDirPath()+"/connection.ini", QSettings::IniFormat );
        settings.beginGroup( "Connection" );
        settings.setValue( "HostName", ui->host->text() );
        settings.setValue( "Port", ui->port->value() );
        settings.setValue( "UserName", ui->login->text() );
        settings.setValue( "Driver", ui->driver->itemText(ui->driver->currentIndex()) );
        settings.endGroup();
        accept();
    }
}
