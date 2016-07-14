#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QVector>
namespace Ui {
class Dialog;
}

struct dataToRead
{
    int countVoltage;
    int countCurrent;
    int countEffectivePower;
    int countReactivePower;
    int countApparentPower;
    int countPowerFactor;
    QByteArray voltage;
    QByteArray current;
    QByteArray effectivePower;
    QByteArray reactivePower;
    QByteArray apprentPower;
    QByteArray powerFactor;
};
struct dataToPlot
{
    QVector<float> A;
    QVector<float> B;
    QVector<float> C;
    QVector<float> S;
};

enum readType
{
    NoneType=0,
    ReadVoltage,
    ReadCurrent,
    ReadEffectivePower,
    ReadReactivePower,
    ReadApparentPower,
    ReadPowerFactor
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    void initPort();
    void initData();
    void initDataToPlot();
    void initConnections();
    void getVoltage();
    void getCurrent();
    void getEffectivePower();
    void getReactivePower();
    void getApparentPower();
    void getPowerFactor();
    ~Dialog();

private slots:
    void on_launch_clicked();
    void getData();
    void plotVoltage(float A,float B,float C);
    void plotCurrent(float A,float B,float C);
    void plotEffectivePower(float A,float B,float C,float S);
    void plotReactivePower(float A,float B,float C,float S);
    void plotApparentPower(float A,float B,float C,float S);
    void plotPowerFactor(float A,float B,float C,float S);

signals:
    void voltageDataGot(float A,float B,float C);
    void currentDataGot(float A,float B,float C);
    void effectivePowerDataGot(float A,float B,float C,float S);
    void reactivePowerDataGot(float A,float B,float C,float S);
    void apparentPowerDataGot(float A,float B,float C,float S);
    void powerFactorDataGot(float A,float B,float C,float S);


private:
    Ui::Dialog *ui;
    QSerialPort * portWrite;
    QSerialPort * portRead;
    dataToRead m_data;
    readType m_readType;
    dataToPlot m_voltage;




};

#endif // DIALOG_H
