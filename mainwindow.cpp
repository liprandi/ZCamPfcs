#include "mainwindow.h"
#include "./ui_mainwindow.h"

QStringList MainWindow::gOutStream;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings(QApplication::applicationDirPath() + "/zcampfcs.ini", QSettings::IniFormat)
    , m_op110(0l)
    , ui(new Ui::MainWindow)
{
    m_addrOpenProtocol = m_settings.value("openprotocol/address", "172.29.197.102").toString();
    m_portOpenProtocol = quint16(m_settings.value("openprotocol/port", 4545).toUInt());
    qDebug() << m_addrOpenProtocol << ":" << m_portOpenProtocol;

    m_op110 = new ZOpenprotocol(this);
    if(m_op110)
    {
        m_op110->ipAndPort(m_addrOpenProtocol, m_portOpenProtocol);
    }
    ui->setupUi(this);
    connect(ui->tabWidget, &QTabWidget::currentChanged, [&]()
    {
        ui->textBrowser->clear();
        for(int i=0 ; i < gOutStream.length() ; i++)
            ui->textBrowser->append(gOutStream.at(i));
    });
}

MainWindow::~MainWindow()
{
    m_settings.setValue("openprotocol/address", m_addrOpenProtocol);
    m_settings.setValue("openprotocol/port", m_portOpenProtocol);
    delete ui;
    delete m_op110;
}

