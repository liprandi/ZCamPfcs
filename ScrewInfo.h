#ifndef SCREWINFO_H
#define SCREWINFO_H
#include <QString>

struct ScrewInfo
{
    ulong cellId;
    ulong channelId;
    QString controllerName;
    QString vinNumber;
    QString serialNumber;
    QString programName;
    ulong jobId;
    ulong programNumber;
    ulong okCounterLimit;
    ulong counterValue;
    ulong jobSeqNumber;
    ulong syncTighteningId;
    ulong resultStatus;
    uchar tighteningStatus;
    uchar torqueStatus;
    uchar angleStatus;
    uchar totalAngleStatus;
    uchar powerRedundancy;
    uchar torqueUnit;

    float minTorque;
    float maxTorque;
    float targetTorque;
    float torque;
    float minAngle;
    float maxAngle;
    float targetAngle;
    float angle;
    float totalAngleMin;
    float totalAngleMax;
    float totalAngle;
    float powerMin;
    float powerMax;
    float power;
    QString timeStamp;
    QString timeLastChange;
    uchar counterStatus;
    QString tighteningID;
    QString tighteningStatusStr;
};

#endif // SCREWINFO_H
