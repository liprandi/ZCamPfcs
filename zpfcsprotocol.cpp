#include "zpfcsprotocol.h"
#include <QNetworkInterface>
#include <chrono>

using namespace std::chrono_literals;

ZPfcsProtocol::ZPfcsProtocol(QObject *parent):
    QThread(parent)
    , m_tcpsol(nullptr)
    , m_tcpunsol(nullptr)
    , m_sol(0)
    , m_unsol(0)
    , m_quit(false)
    , m_run(false)
{
    m_solSeqNumber = 0; // sequence number
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

void ZPfcsProtocol::init(const QString& addr, const QString& solID, quint16 sol, const QString& unsolID, quint16 unsol)
{
    m_addr = addr;
    m_solID = solID;
    m_unsolID = unsolID;
    m_sol = sol;
    m_unsol = unsol;
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
    qDebug() << tr("PFD ") << m_solID << "=" << m_addr << ":" << m_sol << "/" << m_unsolID << "=" << m_addr << ":" << m_unsol;
    start();
}
void ZPfcsProtocol::run()
{
    m_run = true;
    int step = 0;
    while(!m_quit)
    {
        switch(step)
        {
        case 0: // open ports
            if(openTcpSol() && openTcpUnsol())
                step = 1;
            break;
        case 1: // wait unsol message
            if(!checkMessageUnsol())
                step = 2;
            break;
        case 2:
            sendSol9999();
            step = 3;
            wait(10s);
            break;
        case 3:
            sendUnsol9999();
            step = 2;
            wait(10s);
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

    QByteArray d;
    d += "CamDoBra";    // vendor company name
    d += "Paolo ";      // Model number
    d += "00.100:";     // Protocol Version + ":"
    d += ":Sol:1:AUTO:"; // last part
    QByteArray b;
    b += m_solID.toLatin1();
    b += "   ";
    b += QString("%1").arg(++m_solSeqNumber, 6, QChar('0')).toLatin1();
    b += "9999";
    b += QString("%1").arg(d.length(), 4, QChar('0')).toLatin1();
    b += d;
    b += '\r';
    m_tcpsol->write(b);
    qDebug() << "PFCS tx:" << m_solID << ":" << b << "\n";
    if(m_tcpsol->waitForReadyRead(10000))
    {
        auto buff = m_tcpsol->readAll();
        qDebug() << "PFCS rx:" << m_solID << ":" << buff << "\n";
        ret = true;
    }
    return ret;
}
bool ZPfcsProtocol::sendUnsol9999()
{
    bool ret = false;

    QByteArray d;
    d += "CamDoBra";    // vendor company name
    d += "Paolo ";      // Model number
    d += "00.100:";     // Protocol Version + ":"
    d += ":Unsol:1:AUTO:"; // last part
    QByteArray b;
    b += m_unsolID.toLatin1();
    b += "   ";
    b += QString("%1").arg(++m_unsolSeqNumber, 6, QChar('0')).toLatin1();
    b += "9999";
    b += QString("%1").arg(d.length(), 4, QChar('0')).toLatin1();
    b += d;
    b += '\r';
    m_tcpunsol->write(b);
    qDebug() << "PFCS tx:" << m_unsolID << ":" << b << "\n";
    if(m_tcpunsol->waitForReadyRead(10000))
    {
        auto buff = m_tcpunsol->readAll();
        qDebug() << "PFCS rx:" << m_unsolID << ":" << buff << "\n";
        ret = true;
    }
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
    m_tcpsol->connectToHost(m_addr, m_sol);
    if(m_tcpsol->waitForConnected(3000))
    {
        ret = true;
        connect(m_tcpsol, &QTcpSocket::disconnected, this, [this]{m_tcpsol->close(); delete m_tcpsol; m_tcpsol = 0l;});
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
    m_tcpunsol->connectToHost(m_addr, m_unsol);
    if(m_tcpunsol->waitForConnected(3000))
    {
        ret = true;
        connect(m_tcpunsol, &QTcpSocket::disconnected, this, [this]{m_tcpunsol->close(); delete m_tcpunsol; m_tcpunsol = 0l;});
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

    }
    return ret;
}
