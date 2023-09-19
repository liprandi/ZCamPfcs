#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "zscrewdata.h"
#include <QFile>
#include <QTextStream>

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

    for(int i = 0; i < kMaxScrews; i++)
    {
        m_pfcd[i] = 0l;
        m_solPfcsProtocol[i] = 0;
        m_unsolPfcsProtocol[i] = 0;

        m_addrPfcsProtocol[i] = m_settings.value(QString("pfcsprotocol/addr_%1").arg(i + 1, 2, 10, QChar('0')), "0.0.0.0").toString();
        m_idSolPfcsProtocol[i] = m_settings.value(QString("pfcsprotocol/idsol_%1").arg(i + 1, 2, 10, QChar('0')), "0.0.0.0").toString();
        m_solPfcsProtocol[i] = quint16(m_settings.value(QString("pfcsprotocol/sol_%1").arg(i + 1, 2, 10, QChar('0')), 0).toUInt());
        m_idUnsolPfcsProtocol[i] = m_settings.value(QString("pfcsprotocol/idunsol_%1").arg(i + 1, 2, 10, QChar('0')), "0.0.0.0").toString();
        m_unsolPfcsProtocol[i] = quint16(m_settings.value(QString("pfcsprotocol/unsol_%1").arg(i + 1, 2, 10, QChar('0')), 0).toUInt());
    }


    m_op110 = new ZOpenprotocol(this);
    if(m_op110)
    {
        m_op110->ipAndPort(m_addrOpenProtocol, m_portOpenProtocol);
    }
    for(int i = 0; i < kMaxScrews; i++)
    {
        if(m_solPfcsProtocol[i] > 0 && m_unsolPfcsProtocol[i] > 0)
        {
            m_pfcd[i] = new ZPfcsProtocol(this);
            if(m_pfcd[i])
            {
                m_pfcd[i]->init(m_addrPfcsProtocol[i], m_idSolPfcsProtocol[i], m_solPfcsProtocol[i], m_idUnsolPfcsProtocol[i], m_unsolPfcsProtocol[i]);
            }
        }
    }
    ui->setupUi(this);
    read("default.screw");
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this]()
    {
        ui->textBrowser->clear();
        for(int i=0 ; i < gOutStream.length() ; i++)
            ui->textBrowser->append(gOutStream.at(i));
    });
    connect(m_op110, &ZOpenprotocol::dataReady, this, [this](const ScrewInfo& screw)
    {
        ZScrewData* tmp = new ZScrewData(this);
        tmp->set(screw);
        ui->listscrews->layout()->addWidget(tmp);
        qDebug() << "add new item to screw view";
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
    });
}

MainWindow::~MainWindow()
{
    m_settings.setValue("openprotocol/address", m_addrOpenProtocol);
    m_settings.setValue("openprotocol/port", m_portOpenProtocol);
    for(int i = 0; i < kMaxScrews; i++)
    {
        if(m_solPfcsProtocol[i] > 0 && m_unsolPfcsProtocol[i] > 0)
        {
            m_settings.setValue(QString("pfcsprotocol/addr_%1").arg(i + 1, 2, 10, QChar('0')), m_addrPfcsProtocol[i]);
            m_settings.setValue(QString("pfcsprotocol/idsol_%1").arg(i + 1, 2, 10, QChar('0')), m_idSolPfcsProtocol[i]);
            m_settings.setValue(QString("pfcsprotocol/sol_%1").arg(i + 1, 2, 10, QChar('0')), m_solPfcsProtocol[i]);
            m_settings.setValue(QString("pfcsprotocol/idunsol_%1").arg(i + 1, 2, 10, QChar('0')), m_idUnsolPfcsProtocol[i]);
            m_settings.setValue(QString("pfcsprotocol/unsol_%1").arg(i + 1, 2, 10, QChar('0')), m_unsolPfcsProtocol[i]);
        }
    }
    write("default.screw");

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
    else
    {
        // debug
        for(int i = 0; i < 4; i++)
        {
            ZScrewData* tmp = new ZScrewData(this);
            ScrewInfo d;
            d.cellId = i + i * 10;
            d.channelId = d.cellId * 123;
            d.controllerName = QString("controller%1").arg(d.channelId);
            d.controllerName = QString("vin%1").arg(d.channelId);
            ui->listscrews->layout()->addWidget(tmp);
        }
        // debug
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
