#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"
#include "regends.h"
#include "onsiteinfo.h"
#include "eewinfo.h"
#include "pgainfo.h"

#include <QMainWindow>

#include <QQuickView>
#include <QtQuick>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:    
    explicit MainWindow(QString dbFileName = nullptr,
                        QString evid = nullptr,
                        QString myPLat = nullptr, QString myPLon = nullptr,
                        QString pVel = nullptr, QString sVel = nullptr, QString intenP = nullptr,
                        QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTextCodec *codec;

    double myPosition_Lat, myPosition_Lon;
    double pVelocity, sVelocity;
    QString intenPath;

    int totalDuration;
    QDateTime eventStartTime;
    QTimer *mainTimer;

    // About alert Map & communicating beetween qml and cpp
    QObject *aRootObj;
    QVariant aReturnedValue;
    QWidget *aMapContainer;

    // About Database & table
    QSqlDatabase db;
    QSqlQueryModel *eventModel;
    QSqlQueryModel *onsiteModel;
    QSqlQueryModel *pgaModel;
    QSqlQueryModel *eewModel;

    // About animations
    QDateTime endTimeP, endTimeS;
    double remainSecPAbsolute, remainSecSAbsolute;
    double remainSecPRelative, remainSecSRelative;
    int blinkCount;
    double aniLat, aniLon;
    QTimer *aniTimer;
    QString intensity, vibration;
    QColor blinkColor;
    bool needAnimation;
    void showAnimation();

    // About decoration gui
    void decorationGUI();

    void getEventInfo(QString, QString);
    void setAlertList(QVector<_KGOnSite_Info_t>, QVector<_EEWInfo>, QString, int, QVector<_KGKIIS_GMPEAK_EVENT_STA_t>, QString);

    // draw circle, marker on map
    void drawEEWOnMap(_EEWInfo);
    void drawOnsiteOnMap(int, _KGOnSite_Info_t);
    void drawPGAOnMap(QString, _KGKIIS_GMPEAK_EVENT_STA_t);
    void drawIntensityOnMap(QString, QString, QString);

private slots:
    void goEvent();
};

#endif // MAINWINDOW_H
