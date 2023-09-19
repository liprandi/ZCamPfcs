#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDateTime>
#include <QMutex>
#include <iostream>

QMutex messageMutex;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&messageMutex);

    const QString txt(QString(QDateTime::currentDateTime().toString() + ": "
                              + QString("%1(%2): %3").arg(context.file).arg(context.line).arg(msg)));
    switch (type) {
    case QtInfoMsg:
    case QtDebugMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        // redundant check, could be removed, or the
        // upper if statement could be removed
        MainWindow::gOutStream << txt;
        std::cout << txt.toStdString() << std::endl;
        if(MainWindow::gOutStream.count() > 200)
            MainWindow::gOutStream.removeFirst();
        break;
    case QtFatalMsg:
        abort();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(myMessageOutput);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ZCamPfcs_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
