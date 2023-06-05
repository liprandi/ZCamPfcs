#include "zopenprotocol.h"

ZOpenprotocol::ZOpenprotocol(QObject *parent):
    QThread(parent)
    , m_tcp(nullptr)
    , m_port(0)
    , m_quit(false)
    , m_run(false)
{
}

ZOpenprotocol::~ZOpenprotocol()
{
    if(m_run)
    {
        m_quit = true;
        while(m_quit || m_run)
            sleep(1);
    }
}

void ZOpenprotocol::ipAndPort(const QString& addr, quint16 port)
{
    m_addr = addr;
    m_port = port;
    qDebug() << tr("Start ip:") << addr << ":" << port;
    start();
}

void ZOpenprotocol::run()
{
    m_run = true;
    int step = 0;
    while(!m_quit)
    {
        switch(step)
        {
            case 0: if(connectToServer())
                        step = 1;
                    break;
            case 1: if(connectMID0001()) // start communication with Openprotocol
                        step = 2;
                    else
                        step = 0;
                    break;
            case 2: if(connectMID0060()) // subscribe result
                        step = 3;
                    else
                        step = 1;
                    break;
            case 3: if(!connectMID0061()) // subscribe result
                        step = 2;
                    break;

        }
    }
    if(m_tcp)
    {
        const char* msg = "00200063            \x00";
        qDebug() << "->Openprotocol(" << (strlen(msg) + 1) << "):" << msg;
        m_tcp->write(msg, 21);   //MID0063 unsubscribe
        m_tcp->flush();
        m_tcp->close();
        delete m_tcp;
    }
    m_run = false;
    m_quit = false;
}
bool ZOpenprotocol::connectToServer()
{
    bool ret = false;
    if(!m_tcp)
    {
        m_tcp = new QTcpSocket();
    }
    else
    {
       m_tcp->flush();
       m_tcp->close();
       sleep(3);
    }
    m_tcp->connectToHost(m_addr, m_port);
    if(m_tcp->waitForConnected(3000))
    {
        ret = true;
    }
    else
    {
        qDebug() << tr("Tightening time-out connection");
        m_tcp->close();
        delete m_tcp;
        m_tcp = nullptr;
    }
    return ret;
}
bool ZOpenprotocol::connectMID0001()
{
    bool ret = false;
    if(m_tcp)
    {
        const char* msg = "00200001003         \x00";
        qDebug() << "->Openprotocol(" << (strlen(msg) + 1) << "):" << msg;
        m_tcp->write(msg, 21); // MID0001
        if(m_tcp->waitForReadyRead(3000))
        {
            auto data = m_tcp->readAll();
            qDebug() << "Openprotocol(" << data.length() << "): " << data;
            if(data.length() > 57)  // MID 0002 OK
            {
                m_data.torqueControllerName = data.mid(32, 25);
                removeRightSpace(m_data.torqueControllerName);
                ret = true;
            }
            else if(data.length() == 20) // MID 0003 OR MID004
            {
                if(data[10] == '4') // MID0004 OK
                {
                    ret = true;
                }
            }
        }
    }
    return ret;
}
bool ZOpenprotocol::connectMID0060()
{
    bool ret = false;
    if(m_tcp)
    {
        const char* msg = "00200060001         \x00";
        qDebug() << "->Openprotocol(" << (strlen(msg) + 1) << "):" << msg;
        m_tcp->write(msg, 21); // MID0060
        if(m_tcp->waitForReadyRead(3000))
        {
            auto data = m_tcp->readAll();
            qDebug() << "Openprotocol(" << data.length() << "): " << data;
            if(data.length() > 20)
            {
                if(data[7] == '5') // MID0005 OK
                {
                    ret = true;
                }
            }
        }
    }
    return ret;
}
bool ZOpenprotocol::connectMID0061()
{
    bool ret = false;
    if(m_tcp)
    {
        if(m_tcp->waitForReadyRead(3000))
        {
            auto data = m_tcp->readAll();
            qDebug() << "Openprotocol(" << data.length() << "): " << data;
            if(data.length() >= 385)
            {
                m_data.cellId = data.mid(22, 4).toULong();
                m_data.channelId = data.mid(28, 2).toULong();
                m_data.torqueControllerName = data.mid(32, 25);
                removeRightSpace(m_data.torqueControllerName);
                m_data.vinNumber = data.mid(59, 25);
                removeRightSpace(m_data.vinNumber);
                m_data.jobId = data.mid(86, 2).toULong();
                m_data.parameterSetId = data.mid(92, 3).toULong();
                m_data.strategy = data.mid(97, 2).toULong();
                m_data.tighteningStatus = data[120] == '1';
                 m_data.torqueStatus = data[126] - '1';
                m_data.angleStatus = data[127] - '1';
                m_data.minTorque = data.mid(159, 6).toFloat() / 100.f;
                m_data.maxTorque = data.mid(167, 6).toFloat() / 100.f;
                m_data.targetTorque = data.mid(175, 6).toFloat() / 100.f;
                m_data.torque = data.mid(183, 6).toFloat() / 100.f;
                m_data.minAngle = data.mid(191, 5).toFloat();
                m_data.maxAngle = data.mid(198, 5).toFloat();
                m_data.targetAngle = data.mid(205, 5).toFloat();
                m_data.angle = data.mid(212, 5).toFloat();
                m_data.timeStamp = data.mid(345, 19);
                m_data.timeLastChange = data.mid(366, 19);
                resultText();
                const char* msg = "00200062            \x00";
                qDebug() << "->Openprotocol(" << (strlen(msg) + 1) << "):" << msg;
                m_tcp->write(msg, 21); // MID0062
                ret = true;
            }
        }
        else
        {
            const char* msg = "00209999            \x00";
//            qDebug() << "->Openprotocol(" << (strlen(msg) + 1) << "):" << msg;
            m_tcp->write(msg, 21);
            if(m_tcp->waitForReadyRead(3000))
            {
                auto data = m_tcp->readAll();
//                qDebug() << "Openprotocol(" << data.length() << "): " << data;
                if(data.length() == 21)
                {
                    ret = true;
                }
            }
        }
    }
    return ret;
}
void ZOpenprotocol::removeRightSpace(QString& str)
{
    int idx = -1;
    for(int i = 0; i < str.length(); i++)
    {
        if(str[i] != ' ')
            idx = i;
    }
    if(idx > 0)
    {
        str.remove(idx + 1, str.length() - idx - 1);
    }
}
void ZOpenprotocol::resultText()
{
    m_message = QString("OPENPROTOCOL\n%1\nTorque: %2 Nm\n") // Ângulo: %3\n°
                .arg(m_data.tighteningStatus ? "1": "0")
                .arg(double(m_data.torque), 6, 'f', 2)
                /*.arg(double(m_data.angle), 6, 'f', 1)*/;
    emit dataReady(m_message);
}
