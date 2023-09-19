#ifndef ZOPENPROTOCOL_H
#define ZOPENPROTOCOL_H

#include <QThread>
#include <QTcpSocket>
#include "screwinfo.h"

class ZOpenprotocol: public QThread
{
    Q_OBJECT
public:
    explicit ZOpenprotocol(QObject *parent = nullptr);
    ~ZOpenprotocol() override;
    void run() override;
public slots:
    void ipAndPort(const QString& addr, quint16 port);
signals:
    void dataReady(const ScrewInfo& screw);
private:
    bool connectToServer();
    bool connectMID0001();
    bool connectMID0060();
    bool connectMID0061();
    void removeRightSpace(QString& str);
    void resultText();

private:
    QTcpSocket* m_tcp;
    ScrewInfo m_data;
    QString m_message;
    QString m_addr;
    quint16 m_port;
    bool m_quit;
    bool m_run;
};

#endif // ZOPENPROTOCOL_H
