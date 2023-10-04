#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDateTime>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <iostream>

QMutex messageMutex;
QFile g_log;
int   g_hour = 0;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&messageMutex);
    QDateTime dt = QDateTime::currentDateTime();
    int diff = abs(g_hour - dt.time().hour());
    if(!g_log.isOpen() || diff > 6)
    {
        if(g_log.isOpen())
        {
            g_log.flush();
            g_log.close();
        }
        g_hour = int(dt.time().hour() / 6) * 6;
        QString name = QString("Log%1%2%3%4.log").arg(dt.date().year(), 4, 10, QChar('0')).arg(dt.date().month(), 2, 10, QChar('0')).arg(dt.date().day(), 2, 10, QChar('0')).arg(g_hour, 2, 10, QChar('0'));
        g_log.setFileName(name);
        g_log.open(QFile::Append);
    }
    const QString txt(QString(QDateTime::currentDateTime().toString() + ": "
                              + QString("%1(%2): %3").arg(context.file).arg(context.line).arg(msg)));
    if(g_log.isOpen())
    {
        g_log.write(txt.toLatin1());
        g_log.write("\n");
        g_log.flush();
    }
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
    w.hide();
    return a.exec();
}
