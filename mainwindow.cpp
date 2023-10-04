#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "zscrewdata.h"
#include <QCloseEvent>
#include <QFile>
#include <QTextStream>
#include <chrono>

using namespace std::chrono_literals;

QStringList MainWindow::gOutStream;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(QApplication::applicationDirPath() + "/zcampfcs.ini", QSettings::IniFormat)
    , m_op110(0l)
    , m_op150(0l)
    , m_lastCis110("00000000")
    , m_lastCis150("00000000")
    , m_newdata110(false)
    , m_newdata150(false)
    , m_cntloops(0)
    , ui(new Ui::MainWindow)
    , m_iconStatus(0)
{
    m_icon[0] = QIcon(":/icon/cam");
    m_icon[1] = QIcon(":/icon/camWarning");
    m_icon[2] = QIcon(":/icon/camFault");
    createActions();
    createTrayIcon();

    m_addr110 = m_settings.value("op110/address", "172.29.197.104").toString();
    m_port110 = quint16(m_settings.value("op110/port", 4545).toUInt());
    qDebug() << m_addr110 << ":" << m_port110;
    m_addr150 = m_settings.value("op150/address", "172.29.197.107").toString();
    m_port150 = quint16(m_settings.value("op150/port", 4545).toUInt());
    qDebug() << m_addr150 << ":" << m_port150;

    for(int i = 0; i < kMaxScrews; i++)
    {
        auto& s = m_screw[i];
        m_pfcd[i] = 0l;
        s.m_solPfcsProtocol = 0;
        s.m_unsolPfcsProtocol = 0;

        s.m_addrPfcsProtocol = m_settings.value(QString("pfcsprotocol/addr_%1").arg(i + 1, 2, 10, QChar('0')), "0.0.0.0").toString();
        s.m_idSolPfcsProtocol = m_settings.value(QString("pfcsprotocol/idsol_%1").arg(i + 1, 2, 10, QChar('0')), "0.0.0.0").toString();
        s.m_solPfcsProtocol = quint16(m_settings.value(QString("pfcsprotocol/sol_%1").arg(i + 1, 2, 10, QChar('0')), 0).toUInt());
        s.m_idUnsolPfcsProtocol = m_settings.value(QString("pfcsprotocol/idunsol_%1").arg(i + 1, 2, 10, QChar('0')), "0.0.0.0").toString();
        s.m_unsolPfcsProtocol = quint16(m_settings.value(QString("pfcsprotocol/unsol_%1").arg(i + 1, 2, 10, QChar('0')), 0).toUInt());
        s.m_channel = quint16(m_settings.value(QString("pfcsprotocol/chn_%1").arg(i + 1, 2, 10, QChar('0')), 0).toUInt());
        s.m_program = quint16(m_settings.value(QString("pfcsprotocol/program_%1").arg(i + 1, 2, 10, QChar('0')), 0).toUInt());
        s.m_name = m_settings.value(QString("pfcsprotocol/name_%1").arg(i + 1, 2, 10, QChar('0')), "V---").toString();
    }


    m_op110 = new ZOpenprotocol(this);
    if(m_op110)
    {
        m_op110->ipAndPort(m_addr110, m_port110);
    }
    m_op150 = new ZOpenprotocol(this);
    if(m_op150)
    {
        m_op150->ipAndPort(m_addr150, m_port150);
    }
    for(int i = 0; i < kMaxScrews; i++)
    {
        auto& s = m_screw[i];
        if(s.m_solPfcsProtocol > 0 && s.m_unsolPfcsProtocol > 0)
        {
            m_pfcd[i] = new ZPfcsProtocol(this);
            if(m_pfcd[i])
            {
                m_pfcd[i]->init(s.m_addrPfcsProtocol, s.m_idSolPfcsProtocol, s.m_solPfcsProtocol, s.m_idUnsolPfcsProtocol, s.m_unsolPfcsProtocol, s.m_channel, s.m_program, s.m_name);
            }
        }
    }
    ui->setupUi(this);
    if(ui->groupBox110)
    {
        QGridLayout* grid = (QGridLayout*)(ui->groupBox110->layout());
        for(int i = 0; i < kMaxScrews && grid; i++)
        {
            auto& s = m_screw[i];
            if(s.m_solPfcsProtocol > 0 && s.m_unsolPfcsProtocol > 0 && !s.m_name.contains("V12"))
            {
                QLabel* icon = new QLabel(ui->groupBox110);
                QLabel* text = new QLabel(s.m_name, ui->groupBox110);
                icon->setPixmap(QPixmap(":/icon/oo.png"));
                icon->setAccessibleName(s.m_name);
                grid->addWidget(icon, s.m_channel, (s.m_program - 1) * 2);
                grid->addWidget(text, s.m_channel, (s.m_program - 1) * 2 + 1);
            }
        }
    }
    if(ui->groupBox150)
    {
        int cnt = 0;
        QGridLayout* grid = (QGridLayout*)(ui->groupBox150->layout());
        for(int i = 0; i < kMaxScrews && grid; i++)
        {
            auto& s = m_screw[i];
            if(s.m_solPfcsProtocol > 0 && s.m_unsolPfcsProtocol > 0 && s.m_name.contains("V12"))
            {
                QLabel* icon = new QLabel(ui->groupBox150);
                QLabel* text = new QLabel(s.m_name, ui->groupBox150);
                icon->setPixmap(QPixmap(":/icon/oo.png"));
                icon->setAccessibleName(s.m_name);
                grid->addWidget(icon, 1, cnt * 2);
                grid->addWidget(text, 1, cnt * 2 + 1);
                cnt++;
            }
        }
    }
//    read("default.screw");
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this]()
    {
        ui->textBrowser->clear();
        for(int i=0 ; i < gOutStream.length() ; i++)
            ui->textBrowser->append(gOutStream.at(i));
    });
    connect(m_op110, &ZOpenprotocol::dataReady, this, [this]()
    {
        m_newdata110 = true;
    });
    connect(m_op150, &ZOpenprotocol::dataReady, this, [this]()
    {
        m_newdata150 = true;
    });

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::messageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);

    setIcon(m_iconStatus);
    trayIcon->show();
    startTimer(1s);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if(m_newdata110 || m_newdata150 || ++m_cntloops >= 10)
    {
        setIcon(m_cntloops < 10? 0: 1);
        m_cntloops = 0;
        ScrewInfo screw;
        while(m_op110 && m_op110->getScrewData(screw))
        {
            ZScrewData* tmp = new ZScrewData(this);
            tmp->set(screw);
            ui->listscrews->layout()->addWidget(tmp);
            qDebug() << "add new op110 item to screw view";
            emptyResults110(screw.vinNumber);
            updateChannel110(screw);
        }
        while(m_op150 && m_op150->getScrewData(screw))
        {
            ZScrewData* tmp = new ZScrewData(this);
            tmp->set(screw);
            ui->listscrews->layout()->addWidget(tmp);
            qDebug() << "add new op150 item to screw view";
            emptyResults150(screw.vinNumber);
            updateChannel150(screw);
        }
        removeOldLines();
    }
 }
void MainWindow::removeOldLines()
{
    if(ui->listscrews->layout()->count() > 880)
    {
        auto wItem = ui->listscrews->layout()->takeAt(0);
        if(wItem != 0)
        {
            if(wItem->widget())
            {
                wItem->widget()->setParent(NULL);
                delete wItem;
            }
        }
    }
}

void MainWindow::emptyResults110(const QString& newcis)
{
    if(m_lastCis110.compare(newcis) != 0)
    {
        if(ui->groupBox110)
        {
            QObjectList widgetList = ui->groupBox110->children();
            for(auto *obj : widgetList)
            {
                if(obj->isWidgetType())
                {
                    QLabel* w = (QLabel*)(obj);
                    if(w)
                    {
                        const QString& str = w->accessibleName();
                        if(str.length() > 0 && str[0] == 'V')
                        {
                            w->setPixmap(QPixmap(":/icon/oo.png"));
                        }
                    }
                }
            }
        }
        m_lastCis110 = newcis;
        ui->lblCis110->setText(m_lastCis110);
    }
}
void MainWindow::emptyResults150(const QString& newcis)
{
    if(m_lastCis150.compare(newcis) != 0)
    {
        if(ui->groupBox150)
        {
            QObjectList widgetList = ui->groupBox150->children();
            for(auto *obj : widgetList)
            {
                if(obj->isWidgetType())
                {
                    QLabel* w = (QLabel*)(obj);
                    if(w)
                    {
                        const QString& str = w->accessibleName();
                        if(str.length() > 0 && str[0] == 'V')
                        {
                            w->setPixmap(QPixmap(":/icon/oo.png"));
                        }
                    }
                }
            }
        }
        m_cnt150_ok = 0;
        m_lastCis150 = newcis;
        ui->lblCis150->setText(m_lastCis150);
    }
}

void MainWindow::updateChannel110(const ScrewInfo& screw)
{
    for(const auto& chn: m_pfcd)
    {
        if(chn && ui->groupBox110)
        {
            if(chn->checkChnPrg(screw.channelId, screw.programNumber))
            {
                QObjectList widgetList = ui->groupBox110->children();
                for(auto *obj : widgetList)
                {
                    if(obj->isWidgetType())
                    {
                        QLabel* w = (QLabel*)(obj);
                        if(w)
                        {
                            if(chn->checkName(w->accessibleName()))
                            {
                                chn->newData(screw);
                                if(screw.ok)
                                    w->setPixmap(QPixmap(":/icon/ok.png"));
                                else
                                    w->setPixmap(QPixmap(":/icon/ko.png"));
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}
void MainWindow::updateChannel150(const ScrewInfo& screw)
{
    QString name("V12");
    name += QChar('5' + m_cnt150_ok);
    for(const auto& chn: m_pfcd)
    {
        if(chn && ui->groupBox150 && chn->checkName(name))
        {
            QObjectList widgetList = ui->groupBox150->children();
            for(auto *obj : widgetList)
            {
                if(obj->isWidgetType())
                {
                    QLabel* w = (QLabel*)(obj);
                    if(w)
                    {
                        if(chn->checkName(w->accessibleName()))
                        {
                            chn->newData(screw);
                            if(screw.ok)
                            {
                                w->setPixmap(QPixmap(":/icon/ok.png"));
                                m_cnt150_ok++;
                            }
                            else
                                w->setPixmap(QPixmap(":/icon/ko.png"));
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
}
MainWindow::~MainWindow()
{    
    m_settings.setValue("op110/address", m_addr110);
    m_settings.setValue("op110/port", m_port110);
    m_settings.setValue("op150/address", m_addr150);
    m_settings.setValue("op150/port", m_port150);
    for(int i = 0; i < kMaxScrews; i++)
    {
        auto& s = m_screw[i];
        if(s.m_solPfcsProtocol > 0 && s.m_unsolPfcsProtocol > 0)
        {
            m_settings.setValue(QString("pfcsprotocol/addr_%1").arg(i + 1, 2, 10, QChar('0')), s.m_addrPfcsProtocol);
            m_settings.setValue(QString("pfcsprotocol/idsol_%1").arg(i + 1, 2, 10, QChar('0')), s.m_idSolPfcsProtocol);
            m_settings.setValue(QString("pfcsprotocol/sol_%1").arg(i + 1, 2, 10, QChar('0')), s.m_solPfcsProtocol);
            m_settings.setValue(QString("pfcsprotocol/idunsol_%1").arg(i + 1, 2, 10, QChar('0')), s.m_idUnsolPfcsProtocol);
            m_settings.setValue(QString("pfcsprotocol/unsol_%1").arg(i + 1, 2, 10, QChar('0')), s.m_unsolPfcsProtocol);
            m_settings.setValue(QString("pfcsprotocol/chn_%1").arg(i + 1, 2, 10, QChar('0')), s.m_channel);
            m_settings.setValue(QString("pfcsprotocol/program_%1").arg(i + 1, 2, 10, QChar('0')), s.m_program);
            m_settings.setValue(QString("pfcsprotocol/name_%1").arg(i + 1, 2, 10, QChar('0')), s.m_name);
        }
    }
//    write("default.screw");

    delete ui;
    delete m_op110;
}

void MainWindow::read(const QString& filename)
{
    QFile f(filename);
    if(f.open(QFile::ReadOnly))
    {
        QTextStream stream(&f);
        while(!stream.atEnd())
        {
            ZScrewData* tmp = new ZScrewData(this);
            *tmp << stream;
            ui->listscrews->layout()->addWidget(tmp);
        }
        f.flush();
        f.close();
    }
}

void MainWindow::write(const QString& filename)
{
    QFile f(filename);
    if(f.open(QFile::WriteOnly))
    {
        QTextStream stream(&f);
        for(int i = 0; i < ui->listscrews->layout()->count(); i++)
        {
            auto wItem = ui->listscrews->layout()->takeAt(i);
            if(wItem != 0)
            {
                ZScrewData* screw = reinterpret_cast<ZScrewData*>(wItem->widget());
                if(screw)
                {
                    *screw >> stream;
                }
            }
        }
        f.flush();
        f.close();
    }
}
// tray icon managment
void MainWindow::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(trayIcon->isVisible())
    {
        showMessage();
        hide();
        event->ignore();
    }
}
void MainWindow::setIcon(int icon)
{
    if(icon < 0 || icon > 2)
        return;
    m_iconStatus = icon;
    trayIcon->setIcon(m_icon[m_iconStatus]);
    setWindowIcon(m_icon[m_iconStatus]);
    QString msg;
    switch(m_iconStatus)
    {
    case 1:
        msg = tr("Warning operation");
        break;
    case 2:
        msg = tr("Fault operation");
        break;
    default:
        msg = tr("Normal operation mode");
        break;
    }

    trayIcon->setToolTip(msg);
}
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if(isHidden())
            show();
        else
            hide();
        break;
    case QSystemTrayIcon::MiddleClick:
        showMessage();
        break;
    default:
        ;
    }
}
void MainWindow::showMessage()
{
    QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(m_iconStatus + 1);

    if(m_iconStatus >= 0)
    {
        trayIcon->showMessage(tr("Cam PFCS"), tr("Program in running"), msgIcon, 15 * 1000);
    }
}
void MainWindow::messageClicked()
{
}
void MainWindow::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}
