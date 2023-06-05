#ifndef ZPFCSPROTOCOL_H
#define ZPFCSPROTOCOL_H

#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>

class ZPfcsProtocol: public QThread
{
    Q_OBJECT
public:
    explicit ZPfcsProtocol(QObject *parent = nullptr);
    ~ZPfcsProtocol() override;
    void run() override;

public:
    void init(const QString& addr, const QString& machineID, quint16 sol, quint16 unsol);
private:
    bool openTcpSol();
private slots:
    void receivedUnsol();
private:
    QTcpSocket* m_tcpsol;
    QTcpServer* m_tcpunsol;
    QString m_message;
    QString m_addr;
    QString m_machineID;     // 4 letter machine ID
    quint16 m_sol;      // sollicited port
    quint16 m_unsol;    // unsollicited port
    bool m_quit;
    bool m_run;
};

#endif // ZPFCSPROTOCOL_H
