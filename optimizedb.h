#ifndef OPTIMIZEDB_H
#define OPTIMIZEDB_H

#include <QMap>
#include <QDialog>
#include <QVariant>

namespace Ui {
class OptimizeDB;
}

class OptimizeDB : public QDialog
{
    Q_OBJECT

public:
    explicit OptimizeDB(QWidget *parent = 0, const QMap<QString,QVariant> &connectionData = QMap<QString,QVariant>() );
    ~OptimizeDB();

private:
    Ui::OptimizeDB *ui;

public slots:
    void showError(QString errorText);
    void successfullyMessage();
};

#endif // OPTIMIZEDB_H
