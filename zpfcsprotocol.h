#ifndef ZPFCSPROTOCOL_H
#define ZPFCSPROTOCOL_H

#include <QThread>
#include <QTcpSocket>
#include "screwinfo.h"

class ZPfcsProtocol: public QThread
{
    Q_OBJECT
public:
    explicit ZPfcsProtocol(QObject *parent = nullptr);
    ~ZPfcsProtocol() override;
    void run() override;

public:
    void init(const QString& addr, const QString& solID, quint16 sol, const QString &unsolID, quint16 unsol, quint16 channel, quint16 program, const QString& name);
    bool checkChnPrg(quint16 channel, quint16 program) const;
    bool checkName(const QString& name) const;
    void newData(const ScrewInfo& data);
private:
    bool openTcpSol();
    bool openTcpUnsol();
    bool checkMessageUnsol();
    bool sendSol9999();
    bool sendUnsol9999();
    bool sendSol0002();

private:
    QTcpSocket* m_tcpsol;
    QTcpSocket* m_tcpunsol;
    QString m_message;
    QString m_addr;
    QString m_name;
    QString m_solID;    // 4 letter machine ID for sol
    QString m_unsolID;  // 4 letter machine ID for unsol
    quint16 m_sol;      // sollicited port
    quint16 m_unsol;    // unsollicited port
    quint16 m_channel;   // channel connected to this screw
    quint16 m_program;   // program connected to this screw
    uint    m_solSeqNumber; // sequence number
    uint    m_unsolSeqNumber; // sequence number
    ScrewInfo m_newScrewData;    // screw info to send to PFCS
    bool    m_newDataToSend;    // request new send to PFCS
    bool m_quit;
    bool m_run;
};

#endif // ZPFCSPROTOCOL_H
