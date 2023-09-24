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
    }


    m_op110 = new ZOpenprotocol(this);
    if(m_op110)
    {
        m_op110->ipAndPort(m_addrOpenProtocol, m_portOpenProtocol);
    }
    for(int i = 0; i < kMaxScrews; i++)
    {
        auto& s = m_screw[i];
        if(s.m_solPfcsProtocol > 0 && s.m_unsolPfcsProtocol > 0)
        {
            m_pfcd[i] = new ZPfcsProtocol(this);
            if(m_pfcd[i])
            {
                m_pfcd[i]->init(s.m_addrPfcsProtocol, s.m_idSolPfcsProtocol, s.m_solPfcsProtocol, s.m_idUnsolPfcsProtocol, s.m_unsolPfcsProtocol, s.m_channel, s.m_program);
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
    connect(m_op110, &ZOpenprotocol::dataReady, this, [this](ScrewInfo* screw)
    {
        ZScrewData* tmp = new ZScrewData(this);
        tmp->set(*screw);
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
        for(const auto chn: m_pfcd)
        {
            if(chn)
            {
                if(chn->checkChnPrg(screw->channelId, screw->programNumber))
                {
                    chn->newData(*screw);
                    break;
                }
            }
        }
		delete screw;			 
    });
}

MainWindow::~MainWindow()
{
    m_settings.setValue("openprotocol/address", m_addrOpenProtocol);
    m_settings.setValue("openprotocol/port", m_portOpenProtocol);
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
