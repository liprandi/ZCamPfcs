#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "zopenprotocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QSettings m_settings;
    ZOpenprotocol*  m_op110;
private:
    QString   m_addrOpenProtocol;
    quint16   m_portOpenProtocol;

    Ui::MainWindow *ui;
public:
    static QStringList gOutStream;
};
#endif // MAINWINDOW_H
