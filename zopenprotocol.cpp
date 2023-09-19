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
        const char* msg = "00200063            \x00";  //rev.003
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
                m_data.controllerName = data.mid(32, 25);
                removeRightSpace(m_data.controllerName);
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
        const char* msg = "00200060003         \x00";  //rev. 003
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
            int idx = 20;
            while(data.length() >= idx + 2)
            {
                int id = data.mid(idx, 2).toInt();
                int len;
                idx += 2;
                switch(id)
                {
                case 1:
                    len = 4;
                    m_data.cellId = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 2:
                    len = 2;
                    m_data.channelId = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 3:
                    len = 25;
                    m_data.controllerName = data.mid(idx, len);
                    idx += len;
                    break;
                case 4:
                    len = 25;
                    m_data.vinNumber = data.mid(idx, len);
                    removeRightSpace(m_data.vinNumber);
                    idx += len;
                    break;
                case 5:
                    len = 4;
                    m_data.jobId = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 6:
                    len = 3;
                    m_data.programNumber = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 7:
                    len = 2;
                    idx += len;
                    break;
                case 8:
                    len = 5;
                    idx += len;
                    break;
                case 9:
                    len = 4;
                    m_data.okCounterLimit = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 10:
                    len = 4;
                    m_data.counterValue = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 11:
                    len = 1;
                    m_data.tighteningStatus = data[idx] - '0';
                    idx += len;
                    break;
                case 12:
                    len = 1;
                    m_data.counterStatus = data[idx]  - '0';
                    idx += len;
                    break;
                case 13:
                    len = 1;
                    m_data.torqueStatus = data[idx]  - '0';
                    idx += len;
                    break;
                case 14:
                    len = 1;
                    m_data.angleStatus = data[idx]  - '0';
                    idx += len;
                    break;
                case 15:
                    len = 1;
                    m_data.totalAngleStatus = data[idx]  - '0';
                    idx += len;
                    break;
                case 16:
                    len = 1;
                    m_data.powerRedundancy = data[idx]  - '0';
                    idx += len;
                    break;
                case 17:
                case 18:
                case 19:
                    len = 1;
                    idx += len;
                    break;
                case 20:
                    len = 10;
                    m_data.tighteningStatusStr = data.mid(idx, len);
                    idx += len;
                    break;
                case 21:
                    len = 6;
                    m_data.minTorque = data.mid(idx, len).toFloat() / 100.f;
                    idx += len;
                    break;
                case 22:
                    len = 6;
                    m_data.maxTorque = data.mid(idx, len).toFloat() / 100.f;
                    idx += len;
                    break;
                case 23:
                    len = 6;
                    m_data.targetTorque = data.mid(idx, len).toFloat() / 100.f;
                    idx += len;
                    break;
                case 24:
                    len = 6;
                    m_data.torque = data.mid(idx, len).toFloat() / 100.f;
                    idx += len;
                    break;
                case 25:
                    len = 5;
                    m_data.minAngle = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 26:
                    len = 5;
                    m_data.maxAngle = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 27:
                    len = 5;
                    m_data.targetAngle = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 28:
                    len = 5;
                    m_data.angle = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 29:
                    len = 5;
                    m_data.totalAngleMin = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 30:
                    len = 5;
                    m_data.totalAngleMax = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 31:
                    len = 5;
                    m_data.totalAngle = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 32:
                    len = 3;
                    m_data.powerMin = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 33:
                    len = 3;
                    m_data.powerMax = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 34:
                    len = 3;
                    m_data.power = data.mid(idx, len).toFloat();
                    idx += len;
                    break;
                case 35:
                case 36:
                case 37:
                case 38:
                case 39:
                case 40:
                    len = 6;
                    idx += len;
                    break;
                case 41:
                    len = 10;
                    m_data.tighteningID = data.mid(idx, len);
                    idx += len;
                    break;
                case 42:
                    len = 5;
                    m_data.jobSeqNumber = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 43:
                    len = 5;
                    m_data.syncTighteningId = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                case 44:
                    len = 14;
                    m_data.serialNumber = data.mid(idx, len);
                    idx += len;
                    break;
                case 45:
                    len = 19;
                    m_data.timeStamp = data.mid(idx, len);
                    idx += len;
                    break;
                case 46:
                    len = 19;
                    m_data.timeLastChange = data.mid(idx, len);
                    idx += len;
                    break;
                case 47:
                    len = 25;
                    m_data.programName = data.mid(idx, len);
                    idx += len;
                    break;
                case 48:
                    len = 1;
                    m_data.torqueUnit = data[idx]  - '0';
                    idx += len;
                    break;
                case 49:
                    len = 2;
                    m_data.resultStatus = data.mid(idx, len).toULong();
                    idx += len;
                    break;
                }
            }
            resultText();
            const char* msg = "00200062            \x00";
            qDebug() << "->Openprotocol(" << (strlen(msg) + 1) << "):" << msg;
            m_tcp->write(msg, 21); // MID0062
            ret = true;
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
    emit dataReady(m_data);
}
