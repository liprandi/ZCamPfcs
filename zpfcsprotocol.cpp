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
}

ZPfcsProtocol::~ZPfcsProtocol()
{
    if(m_run)
    {
        m_quit = true;
        while(m_quit || m_run)
            sleep(1);
    }
    m_tcpunsol->close();
    delete m_tcpunsol;
    m_tcpunsol = 0l;
}

void ZPfcsProtocol::init(const QString& addr, const QString& machineID, quint16 sol, quint16 unsol)
{
    m_addr = addr;
    m_machineID = machineID;
    m_sol = sol;
    m_unsol = unsol;
    if(m_machineID.length() > 4)
        m_machineID.truncate(4);
    if(m_machineID.length() < 4)
    {
        for(qsizetype i = m_machineID.length(); i < 4; i++)
            m_machineID.push_back(QChar('0' + char(i)));
    }
    qDebug() << tr("PFD ") << m_machineID << "=" << m_addr << ":" << m_sol << "/" << m_unsol;
    m_tcpunsol =  new QTcpServer();
    if(!m_tcpunsol->listen(QHostAddress::Any, m_unsol))
    {
        qDebug() << tr("PFD error opening port ") << m_unsol;
        delete m_tcpunsol;
        m_tcpunsol = 0l;
    }
    else
    {
        QString ipAddress;
        const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        for (const QHostAddress &entry : ipAddressesList)
        {
            if(entry != QHostAddress::LocalHost && entry.toIPv4Address())
            {
                qDebug() << tr("PFD opened port ") << entry.toString() << ":" << m_unsol;
            }
        }
        connect(m_tcpunsol, &QTcpServer::newConnection, this, &ZPfcsProtocol::receivedUnsol);
    }
    start();
}
void ZPfcsProtocol::receivedUnsol()
{
    QTcpSocket *clientConnection = m_tcpunsol->nextPendingConnection();
    connect(clientConnection, &QTcpSocket::disconnected, clientConnection, &QObject::deleteLater);
    if(clientConnection->waitForReadyRead())
    {
        QByteArray buff = clientConnection->readAll();
        qDebug() << "PFCS reaceive \"" << buff << "\"";
    }
    else
        clientConnection->disconnectFromHost();
}
void ZPfcsProtocol::run()
{
    m_run = true;
    int step = 0;
    while(!m_quit)
    {
        switch(step)
        {
        case 0: if(openTcpSol())
                step = 1;
            break;
        case 1:
        case 2:
        case 3:
            break;

        }
    }
    if(m_tcpsol)
    {
        m_tcpsol->flush();
        m_tcpsol->close();
        delete m_tcpsol;
    }
    m_run = false;
    m_quit = false;
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
