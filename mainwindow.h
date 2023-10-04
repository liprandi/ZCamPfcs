#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSystemTrayIcon>
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
    QString   m_name;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void read(const QString& filename);
    void write(const QString& filename);

protected:
    virtual void setVisible(bool visible) override;
    virtual void closeEvent(QCloseEvent *event) override;
private:
    void removeOldLines();
    void emptyResults110(const QString &newcis);
    void emptyResults150(const QString &newcis);
    void updateChannel110(const ScrewInfo& screw);
    void updateChannel150(const ScrewInfo& screw);
protected:
    void timerEvent(QTimerEvent *event) override;
private slots:
    void setIcon(int icon);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();
    void messageClicked();

private:
    void createActions();
    void createTrayIcon();
private:
    QSettings m_settings;
    ZOpenprotocol*  m_op110;
    ZOpenprotocol*  m_op150;
    ZPfcsProtocol*  m_pfcd[kMaxScrews];
private:
    QString   m_addr110;
    quint16   m_port110;
    QString   m_lastCis110;
    QString   m_addr150;
    quint16   m_port150;
    QString   m_lastCis150;
    ConfigScrew m_screw[kMaxScrews];
    bool m_newdata110;
    bool m_newdata150;
    int  m_cntloops;
    int m_cnt150_ok;

    Ui::MainWindow *ui;
private: // tray icon managment

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    int m_iconStatus;
    QIcon m_icon[3];
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
public:
    static QStringList gOutStream;
};
#endif // MAINWINDOW_H
