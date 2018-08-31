#include <QThread>
#include <QFileDialog>
#include <QMessageBox>
#include "importexportdb.h"
#include "ui_importexportdb.h"
#include "importexportdbworker.h"

ImportExportDB::ImportExportDB(QWidget *parent, const QStringList databases, const QMap<QString, QVariant> &credentials, Enums::ImportExportMode mode) :
    QDialog(parent),
    m_databases(databases),
    m_mode(mode),
    m_credentials(credentials),
    ui(new Ui::ImportExportDB)
{
    ui->setupUi(this);
    ui->databases->addItems(databases);
    if(mode == Enums::Export){
        this->setWindowTitle(tr("Экспорт базы данных"));
        ui->databases->setEditable(false);
    }
    if(mode == Enums::Import){
        this->setWindowTitle(tr("Импорт базы данных"));
    }
}

ImportExportDB::~ImportExportDB()
{
    delete ui;
}

void ImportExportDB::on_selectFileButton_clicked()
{
    QString fn = "";
    if(m_mode == Enums::Export){
        fn = QFileDialog::getSaveFileName(this, tr("Укажите файл"),
                                                  QString(), tr("SQL files (*.sql);;All Files (*.*)"));

    }
    if(m_mode == Enums::Import){
        fn = QFileDialog::getOpenFileName(this, tr("Выберите файл"),
                                                  QString(), tr("SQL files (*.sql);;All Files (*.*)"));
    }
    if (fn.isEmpty()) return;
    ui->path->setText(fn);
}

void ImportExportDB::on_clearPatchButton_clicked()
{
    ui->path->setText("");
}

void ImportExportDB::on_applyButton_clicked()
{
    if(m_mode == Enums::Import && (ui->databases->currentText().isNull() || ui->databases->currentText().isEmpty()) ){
        QMessageBox::critical(this,tr("Ошибка"),
                              tr("Не указана база данных!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->databases->currentIndex()<0 && m_mode == Enums::Export ){
        QMessageBox::critical(this,tr("Ошибка"),
                              tr("Не указана база данных!!!"),
                              tr("Закрыть"));
        return;
    }
    if(ui->path->text().isNull() || ui->path->text().isEmpty()){
        QMessageBox::critical(this,tr("Ошибка"),
                              tr("Не указан путь к файлу базы данных!!!"),
                              tr("Закрыть"));
        return;
    }
    ui->log->clear();
    m_credentials["databaseName"] = ui->databases->currentText();
    m_credentials["pathToFile"] = ui->path->text();
    m_credentials["pathToMySqlTools"] = QString(QApplication::applicationDirPath()+"/MySql");
    if(m_mode == Enums::Export){
        disabledForm();
        ImportExportDBWorker *worker = new ImportExportDBWorker(m_credentials);
        runThread(worker);
    }
    if(m_mode == Enums::Import){
        disabledForm();
        if(m_databases.contains(ui->databases->currentText())){
            int ansver = QMessageBox::warning(this,tr("Внимание!!!"),
                                  tr("Вы выбрали существующую базу данных.\nПри импорте в существующую базу данных всё содержимое выбранной базы будет уничтожено.\nВы действительно хотите выполнить импорт в выбранную базу?"),
                                  tr("Да"),
                                  tr("Нет"));
            if(ansver == 1){
                enabledForm();
                return;
            }
        }
        ImportExportDBWorker *worker = new ImportExportDBWorker(m_credentials,m_databases,Enums::Import);
        runThread(worker);
    }
}

void ImportExportDB::enabledForm()
{
    ui->databases->setEnabled(true);
    ui->applyButton->setEnabled(true);
    ui->clearPatchButton->setEnabled(true);
    ui->path->setEnabled(true);
    ui->selectFileButton->setEnabled(true);
}

void ImportExportDB::disabledForm()
{
    ui->databases->setEnabled(false);
    ui->applyButton->setEnabled(false);
    ui->clearPatchButton->setEnabled(false);
    ui->path->setEnabled(false);
    ui->selectFileButton->setEnabled(false);
}

void ImportExportDB::runThread(ImportExportDBWorker *worker)
{
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker,&ImportExportDBWorker::logData,ui->log,&QPlainTextEdit::appendPlainText);
    connect(worker,&ImportExportDBWorker::error,ui->log,&QPlainTextEdit::appendPlainText);
    connect(worker,&ImportExportDBWorker::finished,this,&ImportExportDB::enabledForm);
    connect(worker,&ImportExportDBWorker::newBaseCreated,this,&ImportExportDB::newDatabaseCreated);
    connect(worker,&ImportExportDBWorker::newBaseCreated,this,&ImportExportDB::addNewDatabase);
    connect(this, SIGNAL(accepted()), worker, SIGNAL(finished()));
    connect(this, SIGNAL(rejected()), worker, SIGNAL(finished()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

void ImportExportDB::addNewDatabase(QString base)
{
    ui->databases->addItem(base);
    m_databases.append(base);
}
