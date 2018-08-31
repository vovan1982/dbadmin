#include <QRegExpValidator>
#include <QMessageBox>
#include "createdb.h"
#include "ui_createdb.h"
#include "createdbworker.h"

CreateDB::CreateDB(QWidget *parent, QStringList bases, const QMap<QString, QVariant> &credentials) :
    QDialog(parent),
    ui(new Ui::CreateDB),
    m_bases(bases),
    m_credentials(credentials)
{
    ui->setupUi(this);

    QRegExp validProfileDataBaseNameRegEx("^[0-9a-zA-Z_-]+$");
    QValidator* profileExtensionValidator;
    validProfileDataBaseNameRegEx.setCaseSensitivity(Qt::CaseInsensitive);
    profileExtensionValidator = new QRegExpValidator(validProfileDataBaseNameRegEx, this);
    ui->baseName->setValidator(profileExtensionValidator);
}

CreateDB::~CreateDB()
{
    delete ui;
}

void CreateDB::on_createButton_clicked()
{
    if(ui->baseName->text().isEmpty() || ui->baseName->text().isNull()){
        QMessageBox::critical(this,tr("Ошибка"),
                              tr("Не заполнено имя базы данных!!!"),
                              tr("Закрыть"));
        return;
    }
    if(m_bases.contains(ui->baseName->text())){
        QMessageBox::critical(this,tr("Ошибка"),
                              tr("База данных с таким именем уже существует!!!"),
                              tr("Закрыть"));
        return;
    }

    m_credentials["databaseName"] = ui->baseName->text();
    m_credentials["emptyBase"] = ui->emptyBase->isChecked();
    m_credentials["emptyBaseWithDirectories"] = ui->emptyBaseWithDirectories->isChecked();
    m_credentials["demoBase"] = ui->demoBase->isChecked();

    disabledForm();
    ui->log->setPlainText("");
    CreateDBWorker *worker = new CreateDBWorker(m_credentials);
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker,&CreateDBWorker::logData,ui->log,&QPlainTextEdit::appendPlainText);
    connect(worker,&CreateDBWorker::error,ui->log,&QPlainTextEdit::appendPlainText);
    connect(worker,&CreateDBWorker::finished,this,&CreateDB::enabledForm);
    connect(worker,&CreateDBWorker::newBaseCreated,this,&CreateDB::databaseIsCreated);
    connect(worker,&CreateDBWorker::newBaseCreated,this,&CreateDB::addNewBase);
    connect(this, SIGNAL(accepted()), worker, SIGNAL(finished()));
    connect(this, SIGNAL(rejected()), worker, SIGNAL(finished()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

void CreateDB::enabledForm()
{
    ui->baseName->setEnabled(true);
    ui->typeBase->setEnabled(true);
    ui->createButton->setEnabled(true);
}

void CreateDB::disabledForm()
{
    ui->baseName->setEnabled(false);
    ui->typeBase->setEnabled(false);
    ui->createButton->setEnabled(false);
}

void CreateDB::addNewBase(const QString &base)
{
    m_bases.append(base);
}
