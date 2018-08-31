#include "dbadmin.h"
#include <QApplication>
#include <windows.h>
#include <signal.h>

void handler_sigsegv(int signum)
{
    MessageBoxA(NULL,"SIGSEGV Error!","POSIX Signal",MB_ICONSTOP);
    // открепить обработчик и явно завершить приложение
    signal(signum, SIG_DFL);
    exit(3);
}

void handler_sigfpe(int signum)
{
    MessageBoxA(NULL,"SIGFPE Error!","POSIX Signal",MB_ICONSTOP);
    // открепить обработчик и явно завершить приложение
    signal(signum, SIG_DFL);
    exit(3);
}

int main(int argc, char *argv[])
{
    // установим наши обработчики на два сигнала
    signal(SIGSEGV, handler_sigsegv);
    signal(SIGFPE, handler_sigfpe);

    QApplication a(argc, argv);
    DBAdmin w;
    w.show();

    return a.exec();
}
