#include "zscrewdata.h"
#include "ui_zscrewdata.h"

ZScrewData::ZScrewData(QWidget *parent) :
    QWidget(parent)
    , ui(new Ui::ZScrewData)
{
    ui->setupUi(this);
}

ZScrewData::~ZScrewData()
{
    delete ui;
}

void ZScrewData::set(const ScrewInfo &screw)
{
    m_data = screw;
    ui->screwname->setText(m_data.controllerName);
    ui->screwchannel->setText(QString("%1").arg(m_data.channelId));
    ui->screwchannelname->setText(m_data.tighteningID);
    ui->screwserialnumber->setText(m_data.serialNumber);
    ui->screwvin->setText(m_data.vinNumber);
    ui->screwprogramnumber->setText(QString("%1").arg(m_data.programNumber));
    ui->screwprogramname->setText(m_data.programName);
    ui->screwtorque->setText(QString("%1").arg(m_data.torque, 0, 'f', 2));
    ui->screwtorquemin->setText(QString("%1").arg(m_data.minTorque, 0, 'f', 2));
    ui->screwtorquetarget->setText(QString("%1").arg(m_data.targetTorque, 0, 'f', 2));
    ui->screwtorquemax->setText(QString("%1").arg(m_data.maxTorque, 0, 'f', 2));
    ui->screwangle->setText(QString("%1").arg(m_data.angle, 0, 'f', 0));
    ui->screwanglemin->setText(QString("%1").arg(m_data.minAngle, 0, 'f', 0));
    ui->screwangletarget->setText(QString("%1").arg(m_data.targetAngle, 0, 'f', 0));
    ui->screwanglemax->setText(QString("%1").arg(m_data.maxAngle, 0, 'f', 0));
    ui->screwpower->setText(QString("%1").arg(m_data.power, 0, 'f', 0));
    ui->screwpowermin->setText(QString("%1").arg(m_data.powerMin, 0, 'f', 0));
    ui->screwpowermax->setText(QString("%1").arg(m_data.powerMax, 0, 'f', 0));
    ui->screwtime->setText(m_data.timeStamp);
    ui->screwlastchange->setText(m_data.timeLastChange);
    if(!m_data.ok)
        ui->screwname->setStyleSheet("QLabel { background-color : red; color: blue}");
    else
        ui->screwname->setStyleSheet("QLabel { background-color : green; color: white}");
}

void ZScrewData::operator << (QTextStream& stream)
{
    QString descr;

    if(descr.compare("cellId", Qt::CaseInsensitive))
        stream >> m_data.cellId;
    stream >> descr;
    if(descr.compare("channelId", Qt::CaseInsensitive))
        stream >> m_data.channelId;
    stream >> descr;
    if(descr.compare("controllerName", Qt::CaseInsensitive))
        stream >> m_data.controllerName;
    stream >> descr;
    if(descr.compare("vinNumber", Qt::CaseInsensitive))
        stream >> m_data.vinNumber;

}
void ZScrewData::operator >> (QTextStream& stream)
{
    stream << "cellId";
    stream << " ";
    stream << m_data.cellId;
    stream << "\r\n";
    stream << "channelId";
    stream << " ";
    stream << m_data.channelId;
    stream << "\r\n";
    stream << "controllerName";
    stream << " ";
    stream << m_data.controllerName;
    stream << "\r\n";
    stream << "vinNumber";
    stream << " ";
    stream << m_data.vinNumber;
    stream << "\r\n";

}
