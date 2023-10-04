#include "zpfcsprotocol.h"
#include <QNetworkInterface>

ZPfcsProtocol::ZPfcsProtocol(QObject *parent):
    QThread(parent)
    , m_tcpsol(nullptr)
    , m_tcpunsol(nullptr)
    , m_sol(0)
    , m_unsol(0)
    , m_quit(false)
    , m_run(false)
{
    m_newDataToSend = false;
    m_solSeqNumber = 100; // sequence number
    m_unsolSeqNumber = 0; // sequence number
}

ZPfcsProtocol::~ZPfcsProtocol()
{
    if(m_run)
    {
        m_quit = true;
        while(m_quit || m_run)
            sleep(1);
    }
 }

void ZPfcsProtocol::init(const QString& addr, const QString& solID, quint16 sol, const QString& unsolID, quint16 unsol, quint16 channel, quint16 program, const QString& name)
{
    m_addr = addr;
    m_name = name;
    m_solID = solID;
    m_unsolID = unsolID;
    m_sol = sol;
    m_unsol = unsol;
    m_channel = channel;
    m_program = program;
    if(m_solID.length() > 4)
        m_solID.truncate(4);
    if(m_solID.length() < 4)
    {
        for(qsizetype i = m_solID.length(); i < 4; i++)
            m_solID.push_back(QChar(' '));
    }
    if(m_unsolID.length() > 4)
        m_unsolID.truncate(4);
    if(m_unsolID.length() < 4)
    {
        for(qsizetype i = m_unsolID.length(); i < 4; i++)
            m_unsolID.push_back(QChar(' '));
    }
    qDebug() << tr("PFD ") << m_name << "-" << m_solID << "=" << m_addr << ":" << m_sol << "/" << m_unsolID << "=" << m_addr << ":" << m_unsol << " screw=" << m_channel << "." << m_program;
    start();
}
void ZPfcsProtocol::run()
{
    m_run = true;
    int step = 0;
    while(!m_quit)
    {
        if(!m_tcpsol || !m_tcpunsol || !m_tcpsol->isOpen() || !m_tcpunsol->isOpen())
            step = 0;

        switch(step)
        {
        case 0: // open ports
            if(openTcpSol() && openTcpUnsol())
                step = 1;
            else
                sleep(10);
            break;
        case 1: // wait unsol message
            for(int i = 0; i < 59 && !m_quit; i++)
            {
                if(m_newDataToSend)
                    sendSol0002();
                else if(checkMessageUnsol())
                    i = 0;
            }
            step = 2;
            break;
        case 2:
            sendSol9999();
            step = 3;
            break;
        case 3:
            sendUnsol9999();
            step = 1;
            break;
        }
    }
    if(m_tcpsol)
    {
        m_tcpsol->flush();
        m_tcpsol->close();
        delete m_tcpsol;
        m_tcpsol = 0l;
    }
    if(m_tcpunsol)
    {
        m_tcpunsol->flush();
        m_tcpunsol->close();
        delete m_tcpunsol;
        m_tcpunsol = 0l;
    }
    m_run = false;
    m_quit = false;
}
bool ZPfcsProtocol::sendSol9999()
{
    bool ret = false;
    QString str;

    QByteArray d;
    d += "CamDoBra";    // vendor company name
    str = QString("-%1-").arg(m_name, 4);
    d += str.toLatin1();// Model number
    d += "00.100:";     // Protocol Version + ":"
    d += ":Sol:1:AUTO:"; // last part
    QByteArray b;
    b += m_solID.toLatin1();
    b += "   ";
    str = QString("%1").arg(m_solSeqNumber, 6, 10, QChar('0'));
    b += str.toLatin1();
    b += "9999";
    str = QString("%1").arg(d.length(), 4, 10, QChar('0'));
    b += str.toLatin1();
    b += d;
    b += '\r';
    if(m_tcpsol)
        m_tcpsol->write(b);
    qDebug() << "PFCS tx:" << m_solID << ":" << b << "\n";
    if(m_tcpsol && m_tcpsol->waitForReadyRead(10000))
    {
        auto buff = m_tcpsol->readAll();
        qDebug() << "PFCS rx:" << m_solID << ":" << buff << "\n";
        ret = true;
    }
    else
        ++m_solSeqNumber;
    return ret;
}
bool ZPfcsProtocol::sendUnsol9999()
{
    bool ret = false;
    QString str;

    QByteArray d;
    d += "CamDoBra";    // vendor company name
    str = QString("-%1-").arg(m_name, 4);
    d += str.toLatin1();// Model number
    d += "00.100:";     // Protocol Version + ":"
    d += ":Unsol:1:AUTO:"; // last part
    QByteArray b;
    b += m_unsolID.toLatin1();
    b += "   ";
    str = QString("%1").arg(m_unsolSeqNumber, 6, 10, QChar('0'));
    b += str.toLatin1();
    b += "9999";
    str = QString("%1").arg(d.length(), 4, 10, QChar('0'));
    b += str.toLatin1();
    b += d;
    b += '\r';
    if(m_tcpunsol)
        m_tcpunsol->write(b);
    qDebug() << "PFCS tx:" << m_unsolID << ":" << b << "\n";
    if(m_tcpunsol && m_tcpunsol->waitForReadyRead(10000))
    {
        auto buff = m_tcpunsol->readAll();
        qDebug() << "PFCS rx:" << m_unsolID << ":" << buff << "\n";
        ret = true;
    }
    else
        ++m_unsolSeqNumber;
    return ret;
}

bool ZPfcsProtocol::openTcpSol()
{
    bool ret = false;
    if(!m_tcpsol)
    {
        m_tcpsol = new QTcpSocket();
    }
    else
    {
        m_tcpsol->flush();
        m_tcpsol->close();
        sleep(3);
    }
    if(!m_tcpsol)
        return ret;
    m_tcpsol->connectToHost(m_addr, m_sol);
    if(m_tcpsol->waitForConnected(10000))
    {
        ret = true;
        connect(m_tcpsol, &QTcpSocket::disconnected, this, [this]()
        {
            qDebug() << tr("PFD ") << m_name << " m_tcpsol disconnected";
            m_tcpsol->close(); delete m_tcpsol; m_tcpsol = 0l;
        });
    }
    else
    {
        qDebug() << tr("PFD: cannot open port ") << m_sol;
        m_tcpsol->close();
        delete m_tcpsol;
        m_tcpsol = nullptr;
    }
    return ret;
}
bool ZPfcsProtocol::openTcpUnsol()
{
    bool ret = false;
    if(!m_tcpunsol)
    {
        m_tcpunsol = new QTcpSocket();
    }
    else
    {
        m_tcpunsol->flush();
        m_tcpunsol->close();
        sleep(3);
    }
    if(!m_tcpunsol)
        return ret;
    m_tcpunsol->connectToHost(m_addr, m_unsol);
    if(m_tcpunsol->waitForConnected(10000))
    {
        ret = true;
        connect(m_tcpunsol, &QTcpSocket::disconnected, this, [this]()
        {
            qDebug() << tr("PFD ") << m_name << " m_tcpunsol disconnected";
            m_tcpunsol->close(); delete m_tcpunsol; m_tcpunsol = 0l;
        });
    }
    else
    {
        qDebug() << tr("PFD: cannot open port ") << m_unsol;
        m_tcpunsol->close();
        delete m_tcpunsol;
        m_tcpunsol = nullptr;
    }
    return ret;
}
bool ZPfcsProtocol::checkMessageUnsol()
{
    bool ret = false;

    if(m_tcpunsol->waitForReadyRead(1000))
    {
        QByteArray buff = m_tcpunsol->readAll();
        qDebug() << "PFD unsol: " << buff;
        ret = true;

        if(buff.length() >= 51)
        {
            auto id = buff.mid(0, 4);
            auto acknak = buff.mid(4, 3);
            auto seq = buff.mid(7, 6);
            m_unsolSeqNumber = atoi(seq);
            auto type_msg = buff.mid(13, 4);
            auto data_len = buff.mid(17, 4);
            auto type_code = buff.mid(21, 2);
            if(!type_msg.compare("0003"))
            {
                auto vin = buff.mid(23, 8);
                auto avi = buff.mid(31, 8);
                auto status = buff.mid(39, 2);
                auto tracker = buff.mid(41, 7);
                auto special = buff.mid(48, buff.length() - 1 - 48);
                qDebug() << id << "," << acknak << "," << seq << "," << type_msg << "," << data_len << "," << type_code << "," << vin << "," << avi << "," << status << "," << tracker << "," << special;
                if(buff[buff.length() - 1] == '\r')
                {
                    QByteArray b;
                    b += m_unsolID.toLatin1();
                    b += "ACK";
                    b += seq;
                    m_unsolSeqNumber = atoi(seq) + 1;
                    b += type_msg;
                    b += " \r";
                    m_tcpunsol->write(b);
                }
            }
        }
    }
    return ret;
}

bool ZPfcsProtocol::checkChnPrg(quint16 channel, quint16 program) const
{
    return (m_channel && m_program && m_channel == channel && m_program == program);
}
bool ZPfcsProtocol::checkName(const QString& name) const
{
    return m_name.compare(name, Qt::CaseInsensitive) == 0;
}
void ZPfcsProtocol::newData(const ScrewInfo& data)
{
    if(!m_newDataToSend)
    {
        m_newScrewData = data;
        m_newDataToSend = true;
    }
}
bool ZPfcsProtocol::sendSol0002()
{
    bool ret = false;
    QString str;

    QByteArray d;
    d += "0001CS";    // start data screwing
    str = m_newScrewData.vinNumber;
    for(int i = str.length(); i < 8; i++)
    {
        str += ' ';
    }
    d += str.mid(0, 8).toLatin1();
    d += "ER01";
    d += m_newScrewData.timeStamp.mid(2, 2).toLatin1(); // year
    d += m_newScrewData.timeStamp.mid(5, 2).toLatin1(); // month
    d += m_newScrewData.timeStamp.mid(8, 2).toLatin1(); // day
    d += m_newScrewData.timeStamp.mid(11, 2).toLatin1(); // hours
    d += m_newScrewData.timeStamp.mid(14, 2).toLatin1(); // minutes
    d += m_newScrewData.timeStamp.mid(17, 2).toLatin1(); // seconds
    if(m_newScrewData.ok)
        d += "PP0101";
    else
        d += "FF0101";
    if(!m_newScrewData.ok && (m_newScrewData.torque < m_newScrewData.minTorque || m_newScrewData.torque > m_newScrewData.maxTorque))
        d += 'F';
    else
        d += 'P';
    str = QString("%1").arg(m_newScrewData.maxTorque, 5, 'f', 1, QChar('0'));
    d += str.toLatin1();
    str = QString("%1").arg(m_newScrewData.minTorque, 5, 'f', 1, QChar('0'));
    d += str.toLatin1();
    str = QString("%1").arg(m_newScrewData.torque, 5, 'f', 1, QChar('0'));
    d += str.toLatin1();
    if(!m_newScrewData.ok && (m_newScrewData.angle < m_newScrewData.minAngle || m_newScrewData.angle > m_newScrewData.maxAngle))
        d += 'F';
    else
        d += 'P';
    str = QString("%1").arg(m_newScrewData.maxAngle, 5, 'f', 0, QChar('0'));
    d += str.toLatin1();
    str = QString("%1").arg(m_newScrewData.minAngle, 5, 'f', 0, QChar('0'));
    d += str.toLatin1();
    str = QString("%1").arg(m_newScrewData.angle, 5, 'f', 0, QChar('0'));
    d += str.toLatin1();
    QByteArray b;
    b += m_solID.toLatin1();
    b += "   ";
    str = QString("%1").arg(++m_solSeqNumber, 6, 10, QChar('0'));
    b += str.toLatin1();
    b += "0002";
    str = QString("%1").arg(d.length(), 4, 10, QChar('0'));
    b += str.toLatin1();
    b += d;
    b += '\r';
    m_tcpsol->write(b);
    m_newDataToSend = false;
    qDebug() << "PFCS tx:" << m_solID << ":" << b << "\n";
    if(m_tcpsol->waitForReadyRead(10000))
    {
        auto buff = m_tcpsol->readAll();
        qDebug() << "PFCS rx:" << m_solID << ":" << buff << "\n";
        ret = true;
    }
    return ret;

}
