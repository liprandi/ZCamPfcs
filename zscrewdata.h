#ifndef ZSCREWDATA_H
#define ZSCREWDATA_H

#include <QWidget>
#include <QTextStream>
#include "screwinfo.h"

namespace Ui {
class ZScrewData;
}

class ZScrewData : public QWidget
{
    Q_OBJECT

public:
    explicit ZScrewData(QWidget *parent = nullptr);
    ~ZScrewData();

    void set(const ScrewInfo& screw);
    void operator << (QTextStream& stream);
    void operator >> (QTextStream& stream);

private:
    ScrewInfo m_data;
    Ui::ZScrewData *ui;
};

#endif // ZSCREWDATA_H
