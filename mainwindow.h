#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "zopenprotocol.h"
#include "zpfcsprotocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
#define kMaxScrews 100

struct ConfigScrew
{
    QString   m_addrPfcsProtocol;
    QString   m_idSolPfcsProtocol;
    QString   m_idUnsolPfcsProtocol;
    quint16   m_solPfcsProtocol;
    quint16   m_unsolPfcsProtocol;
    quint16   m_channel;
    quint16   m_program;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void read(const QString& filename);
    void write(const QString& filename);

private:
    QSettings m_settings;
    ZOpenprotocol*  m_op110;
    ZPfcsProtocol*  m_pfcd[kMaxScrews];
private:
    QString   m_addrOpenProtocol;
    quint16   m_portOpenProtocol;
    ConfigScrew m_screw[kMaxScrews];
/*    QString   m_addrPfcsProtocol[kMaxScrews];
    QString   m_idSolPfcsProtocol[kMaxScrews];
    QString   m_idUnsolPfcsProtocol[kMaxScrews];
    quint16   m_solPfcsProtocol[kMaxScrews];
    quint16   m_unsolPfcsProtocol[kMaxScrews];*/

    Ui::MainWindow *ui;
public:
    static QStringList gOutStream;
};
#endif // MAINWINDOW_H
