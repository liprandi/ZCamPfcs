#ifndef ZPFCSPROTOCOL_H
#define ZPFCSPROTOCOL_H

#include <QThread>
#include <QTcpSocket>

class ZPfcsProtocol: public QThread
{
    Q_OBJECT
public:
    explicit ZPfcsProtocol(QObject *parent = nullptr);
    ~ZPfcsProtocol() override;
    void run() override;

public:
    void init(const QString& addr, const QString& solID, quint16 sol, const QString &unsolID, quint16 unsol);
private:
    bool openTcpSol();
    bool openTcpUnsol();
    bool checkMessageUnsol();
    bool sendSol9999();
    bool sendUnsol9999();
private:
    QTcpSocket* m_tcpsol;
    QTcpSocket* m_tcpunsol;
    QString m_message;
    QString m_addr;
    QString m_solID;    // 4 letter machine ID for sol
    QString m_unsolID;  // 4 letter machine ID for unsol
    quint16 m_sol;      // sollicited port
    quint16 m_unsol;    // unsollicited port
    uint    m_solSeqNumber; // sequence number
    uint    m_unsolSeqNumber; // sequence number
    bool m_quit;
    bool m_run;
};

#endif // ZPFCSPROTOCOL_H
