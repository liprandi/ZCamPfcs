#ifndef ZOPENPROTOCOL_H
#define ZOPENPROTOCOL_H

#include <QThread>
#include <QTcpSocket>

class ZOpenprotocol: public QThread
{
    Q_OBJECT
private:
    struct Data
    {
        ulong cellId;
        ulong channelId;
        QString torqueControllerName;
        QString vinNumber;
        ulong jobId;
        ulong parameterSetId;
        ulong strategy;
        bool tighteningStatus;
        int torqueStatus;
        int angleStatus;
        float minTorque;
        float maxTorque;
        float targetTorque;
        float torque;
        float minAngle;
        float maxAngle;
        float targetAngle;
        float angle;
        QString timeStamp;
        QString timeLastChange;
    };
public:
    explicit ZOpenprotocol(QObject *parent = nullptr);
    ~ZOpenprotocol() override;
    void run() override;
public slots:
    void ipAndPort(const QString& addr, quint16 port);
signals:
    void dataReady(const QString& message);
private:
    bool connectToServer();
    bool connectMID0001();
    bool connectMID0060();
    bool connectMID0061();
    void removeRightSpace(QString& str);
    void resultText();

private:
    QTcpSocket* m_tcp;
    Data m_data;
    QString m_message;
    QString m_addr;
    quint16 m_port;
    bool m_quit;
    bool m_run;
};

#endif // ZOPENPROTOCOL_H
