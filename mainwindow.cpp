#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QString dbFileName, QString evid, QString myPLat, QString myPLon, QString pVel, QString sVel, QString intenP, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QIcon icon("/home/sysop/KGOM/params/images/PCIcon.png");
    setWindowIcon(icon);

    // check for accessing internet
    QProcess process;
    QString cmd = "ping -W 1 8.8.8.8 -c 1";
    process.start(cmd);
    process.waitForFinished(-1); // will wait forever until finished
    QString stdout = process.readAllStandardOutput();

    if(stdout.indexOf("ttl") == -1) // return -1 means that this system can't access internet
        canAccessInternet = false;
    else
        canAccessInternet = true;

    codec = QTextCodec::codecForName("utf-8");
    myPosition_Lat = myPLat.toDouble();
    myPosition_Lon = myPLon.toDouble();
    pVelocity = pVel.toDouble();
    sVelocity = sVel.toDouble();
    needAnimation = false;
    intenPath = intenP;

    decorationGUI();

    // Load a osm map & Setup a Map for ALERT tab
    QQuickView *alertView = new QQuickView();
    aMapContainer = QWidget::createWindowContainer(alertView, this);
    alertView->setResizeMode(QQuickView::SizeRootObjectToView);
    aMapContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    aMapContainer->setFocusPolicy(Qt::TabFocus);
    aMapContainer->setStyleSheet("background-color: black;");
    if(canAccessInternet)
        alertView->setSource(QUrl(QStringLiteral("qrc:/Viewmap.qml")));
    else
        alertView->setSource(QUrl(QStringLiteral("qrc:/ViewmapForOffline.qml")));
    ui->alertMapLO->addWidget(aMapContainer);
    aRootObj = alertView->rootObject();
    QMetaObject::invokeMethod(this->aRootObj, "clearMap", Q_RETURN_ARG(QVariant, aReturnedValue));
    QMetaObject::invokeMethod(this->aRootObj, "createEEWStarMarker", Q_RETURN_ARG(QVariant, aReturnedValue));
    QMetaObject::invokeMethod(this->aRootObj, "createMyPositionMarker", Q_RETURN_ARG(QVariant, aReturnedValue));

    getEventInfo(dbFileName, evid);

    ui->eventTimeLCD->display("3");
    mainTimer = new QTimer;
    connect(mainTimer, SIGNAL(timeout()), this, SLOT(goEvent()));
    mainTimer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::goEvent()
{
    QString myT = QString::number(ui->eventTimeLCD->intValue());
    if(myT == "3") ui->eventTimeLCD->display("2");
    else if(myT == "2") ui->eventTimeLCD->display("1");
    else if(myT == "1")
    {
        ui->eventTimeLCD->setDigitCount(10);
        ui->eventTimeLCD->display(eventStartTimeKST.toString("hh:mm:ss"));
        mainTimer->setInterval(100);
        if(needAnimation)
        {
            showAnimation();
        }
    }
    else
    {
        eventStartTimeKST = eventStartTimeKST.addMSecs(100);
        ui->eventTimeLCD->display(eventStartTimeKST.toString("hh:mm:ss"));
        if(needAnimation)
        {
            showAnimation();
        }
    }
}

void MainWindow::showAnimation()
{
    //qDebug() << remainSecSRelative;
    remainSecPRelative = myRound(remainSecPRelative - 0.1, 1);
    remainSecSRelative = myRound(remainSecSRelative - 0.1, 1);

    double radiusP = (myRound(remainSecPAbsolute - remainSecPRelative, 1)) * pVelocity * 1000;
    double radiusS = (myRound(remainSecSAbsolute - remainSecSRelative, 1)) * sVelocity * 1000;

    //qDebug() << radiusP << radiusS << remainSecPAbsolute << remainSecPRelative;

    int intenI = intensity.toInt();
    QString tColor;
    if(intenI == 5) tColor = "black";
    else tColor = "black";
    //else tColor = "white";
    //qDebug() << "intenI " << intenI;
    QMetaObject::invokeMethod(this->aRootObj, "addCircleForAnimation",
                              Q_RETURN_ARG(QVariant, aReturnedValue),
                              Q_ARG(QVariant, aniLat), Q_ARG(QVariant, aniLon),
                              Q_ARG(QVariant, radiusP), Q_ARG(QVariant, radiusS),
                              Q_ARG(QVariant, (int)remainSecSRelative),
                              Q_ARG(QVariant, intensity),
                              Q_ARG(QVariant, vibration),
                              Q_ARG(QVariant, tColor),
                              Q_ARG(QVariant, getIntenColor(intenI).name()));
    //aniCount++;

    if(remainSecSRelative <= -10)
        mainTimer->stop();
}

void MainWindow::decorationGUI()
{
    QPixmap pm(65,65);
    pm.fill(QColor(243, 243, 243));
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setFont(QFont("Open Sans", 10, QFont::Bold));
    p.setPen(QPen(getMagColor(2)));
    p.setBrush(QBrush(getMagColor(2)));
    p.drawEllipse(0, 0, 65, 65);
    p.setPen(QPen(Qt::black));
    p.drawText(pm.rect(), Qt::AlignCenter, getMagText(2));
    ui->mag1LB->setPixmap(pm);
    p.setPen(QPen(getMagColor(4)));
    p.setBrush(QBrush(getMagColor(4)));
    p.drawEllipse(0, 0, 65, 65);
    p.setPen(QPen(Qt::black));
    p.drawText(pm.rect(), Qt::AlignCenter, getMagText(4));
    ui->mag2LB->setPixmap(pm);
    p.setPen(QPen(getMagColor(5)));
    p.setBrush(QBrush(getMagColor(5)));
    p.drawEllipse(0, 0, 65, 65);
    p.setPen(QPen(Qt::white));
    p.drawText(pm.rect(), Qt::AlignCenter, getMagText(5));
    ui->mag3LB->setPixmap(pm);
    p.setPen(QPen(getMagColor(7)));
    p.setBrush(QBrush(getMagColor(7)));
    p.drawEllipse(0, 0, 65, 65);
    p.setPen(QPen(Qt::white));
    p.drawText(pm.rect(), Qt::AlignCenter, getMagText(7));
    ui->mag4LB->setPixmap(pm);
}

void MainWindow::getEventInfo(QString dbFileName, QString evid)
{
    // Open DB
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFileName);
    if(!db.open())
    {
        QMessageBox msgBox;
        msgBox.setText(db.lastError().text());
        msgBox.exec();
        exit(1);
    }

    this->eventModel = new QSqlQueryModel();
    this->onsiteModel = new QSqlQueryModel();
    this->eewModel = new QSqlQueryModel();
    this->pgaModel = new QSqlQueryModel();
    this->pgaDetectModel = new QSqlQueryModel();

    QString query;

    QVector<_KGOnSite_Info_t> onsiteInfos;
    QVector<_EEWInfo> eewInfos;
    QVector<_KGKIIS_GMPEAK_EVENT_STA_t> pgaInfos;
    QVector<_KGKIIS_GMPEAK_EVENT_STA_t> pgaDetectInfos;

    QString pgaChannel;
    int pgaTime;

    query = "select * from pgaDetectInfo where evid =" + evid;
    this->pgaDetectModel->setQuery(query);

    if(this->pgaDetectModel->rowCount() > 0)
    {
        for(int i=0;i<this->pgaDetectModel->rowCount();i++)
        {
            _KGKIIS_GMPEAK_EVENT_STA_t info;
            info.version = this->pgaDetectModel->record(i).value("version").toInt();

            strcpy(info.sta, this->pgaDetectModel->record(i).value("sta").toString().toLatin1().constData());
            strcpy(info.chan, this->pgaDetectModel->record(i).value("chan").toString().toLatin1().constData());
            strcpy(info.net, this->pgaDetectModel->record(i).value("net").toString().toLatin1().constData());
            strcpy(info.loc, this->pgaDetectModel->record(i).value("loc").toString().toLatin1().constData());
            info.lat   = this->pgaDetectModel->record(i).value("lat").toDouble();
            info.lon   = this->pgaDetectModel->record(i).value("lon").toDouble();
            info.time  = this->pgaDetectModel->record(i).value("time").toInt();
            info.maxH = this->pgaDetectModel->record(i).value("maxH").toFloat();
            pgaDetectInfos.push_back(info);
        }
    }

    query = "select * from pgaInfo where evid =" + evid;
    this->pgaModel->setQuery(query);

    if(this->pgaModel->rowCount() > 0)
    {
        for(int i=0;i<this->pgaModel->rowCount();i++)
        {
            _KGKIIS_GMPEAK_EVENT_STA_t info;
            info.version = this->pgaModel->record(i).value("version").toInt();
            //char *temp;
            //temp = (char*) malloc(sizeof(char));
            //strcpy(temp, this->pgaModel->record(i).value("msg_type").toString().toLatin1().constData());
            //info.msg_type = temp[0];
            strcpy(info.sta, this->pgaModel->record(i).value("sta").toString().toLatin1().constData());
            strcpy(info.chan, this->pgaModel->record(i).value("chan").toString().toLatin1().constData());
            strcpy(info.net, this->pgaModel->record(i).value("net").toString().toLatin1().constData());
            strcpy(info.loc, this->pgaModel->record(i).value("loc").toString().toLatin1().constData());
            info.lat   = this->pgaModel->record(i).value("lat").toDouble();
            info.lon   = this->pgaModel->record(i).value("lon").toDouble();
            pgaChannel = this->pgaModel->record(i).value("target_chan").toString();
            pgaTime    = this->pgaModel->record(i).value("e_time").toInt();
            info.time  = this->pgaModel->record(i).value("time").toInt();
            info.maxZ = this->pgaModel->record(i).value("maxZ").toFloat();
            info.maxN = this->pgaModel->record(i).value("maxN").toFloat();
            info.maxE = this->pgaModel->record(i).value("maxE").toFloat();
            info.maxH = this->pgaModel->record(i).value("maxH").toFloat();
            info.maxT = this->pgaModel->record(i).value("maxT").toFloat();
            pgaInfos.push_back(info);
            //free(temp);
        }
    }

    query = "select * from onsiteInfo where evid =" + evid;
    this->onsiteModel->setQuery(query);

    if(this->onsiteModel->rowCount() > 0) // input last onsiteinfo only to infos.
    {
        for(int i=0;i<this->onsiteModel->rowCount();i++)
        {
            _KGOnSite_Info_t info;
            info.version = this->onsiteModel->record(i).value("version").toInt();
            //char *temp;
            //temp = (char*) malloc(sizeof(char));
            //strcpy(temp, this->onsiteModel->record(i).value("msg_type").toString().toLatin1().constData());
            //info.msg_type = temp[0];
            strcpy(info.sta, this->onsiteModel->record(i).value("sta").toString().toLatin1().constData());
            strcpy(info.chan, this->onsiteModel->record(i).value("chan").toString().toLatin1().constData());
            strcpy(info.net, this->onsiteModel->record(i).value("net").toString().toLatin1().constData());
            strcpy(info.loc, this->onsiteModel->record(i).value("loc").toString().toLatin1().constData());
            //strcpy(temp, this->onsiteModel->record(i).value("duration").toString().toLatin1().constData());
            //info.duration = temp[0];
            //strcpy(temp, this->onsiteModel->record(i).value("type").toString().toLatin1().constData());
            //info.type = temp[0];
            info.ttime = this->onsiteModel->record(i).value("ttime").toDouble();
            info.disp_count = this->onsiteModel->record(i).value("disp_count").toFloat();
            info.displacement = this->onsiteModel->record(i).value("displacement").toFloat();
            info.disp_time = this->onsiteModel->record(i).value("disp_time").toDouble();
            info.pgv = this->onsiteModel->record(i).value("pgv").toFloat();
            info.pgv_uncertainty_low = this->onsiteModel->record(i).value("pgv_uncertainty_low").toFloat();
            info.pgv_uncertainty_high = this->onsiteModel->record(i).value("pgv_uncertainty_high").toFloat();
            info.alert = this->onsiteModel->record(i).value("alert").toInt();
            info.snr = this->onsiteModel->record(i).value("snr").toFloat();
            info.log_disp_ratio = this->onsiteModel->record(i).value("log_disp_ratio").toFloat();
            info.log_PdPv = this->onsiteModel->record(i).value("log_PdPv").toFloat();
            info.dB_Pd = this->onsiteModel->record(i).value("dB_Pd").toFloat();
            info.intensity = this->onsiteModel->record(i).value("intensity").toFloat();
            info.intensity_uncertainty_low = this->onsiteModel->record(i).value("intensity_uncertainty_low").toFloat();
            info.intensity_uncertainty_high = this->onsiteModel->record(i).value("intensity_uncertainty_high").toFloat();
            info.magnitude = this->onsiteModel->record(i).value("magnitude").toFloat();
            info.magnitude_uncertainty_low = this->onsiteModel->record(i).value("magnitude_uncertainty_low").toFloat();
            info.magnitude_uncertainty_high = this->onsiteModel->record(i).value("magnitude_uncertainty_high").toFloat();
            info.distance = this->onsiteModel->record(i).value("distance").toFloat();
            info.distance_uncertainty_low = this->onsiteModel->record(i).value("distance_uncertainty_low").toFloat();
            info.distance_uncertainty_high = this->onsiteModel->record(i).value("distance_uncertainty_high").toFloat();
            info.evid = evid.toInt();
            //strcpy(temp, this->onsiteModel->record(i).value("fromWhere").toString().toLatin1().constData());
            //info.fromWhere = temp[0];
            info.lat = this->onsiteModel->record(i).value("lat").toDouble();
            info.lon = this->onsiteModel->record(i).value("lon").toDouble();
            info.elev = this->onsiteModel->record(i).value("elev").toDouble();
            onsiteInfos.push_back(info);
            //free(temp);
        }
    }

    query = "SELECT * FROM eewInfo WHERE evid = " + evid + " ORDER BY rowid DESC";
    this->eewModel->setQuery(query);

    if(this->eewModel->rowCount() > 0)
    {
        for(int i=0;i<this->eewModel->rowCount();i++)
        {
            _EEWInfo eewInfo;
            eewInfo.evid = this->eewModel->record(i).value("evid").toInt();
            //char *temp;
            //temp = (char*) malloc(sizeof(char));
            //strcpy(temp, this->eewModel->record(i).value("type").toString().toLatin1().constData());
            //eewInfo.type = temp[0];
            eewInfo.eew_evid = this->eewModel->record(i).value("eew_evid").toInt();
            eewInfo.version = this->eewModel->record(i).value("version").toInt();
            int mc = this->eewModel->record(i).value("message_category").toInt();
            if(mc == 0) eewInfo.message_category = LIVE;
            else eewInfo.message_category = TEST;
            int mt = this->eewModel->record(i).value("message_type").toInt();
            if(mt == 0) eewInfo.message_type = NEW;
            else if(mt == 1) eewInfo.message_type = UPDATE;
            else eewInfo.message_type = DELETE;
            eewInfo.magnitude = this->eewModel->record(i).value("magnitude").toDouble();
            eewInfo.magnitude_uncertainty = this->eewModel->record(i).value("magnitude_uncertainty").toDouble();
            eewInfo.latitude = this->eewModel->record(i).value("latitude").toDouble();
            eewInfo.latitude_uncertainty = this->eewModel->record(i).value("latitude_uncertainty").toDouble();
            eewInfo.longitude = this->eewModel->record(i).value("longitude").toDouble();
            eewInfo.longitude_uncertainty = this->eewModel->record(i).value("longitude_uncertainty").toDouble();
            eewInfo.depth = this->eewModel->record(i).value("depth").toDouble();
            eewInfo.depth_uncertainty = this->eewModel->record(i).value("depth_uncertainty").toDouble();
            eewInfo.origin_time = this->eewModel->record(i).value("origin_time").toDouble();
            eewInfo.origin_time_uncertainty = this->eewModel->record(i).value("origin_time_uncertainty").toDouble();
            eewInfo.number_stations = this->eewModel->record(i).value("number_stations").toDouble();
            eewInfo.number_triggers = this->eewModel->record(i).value("number_triggers").toDouble();
            eewInfo.sent_flag = this->eewModel->record(i).value("sent_flag").toInt();
            eewInfo.sent_time = this->eewModel->record(i).value("sent_time").toDouble();
            eewInfo.percentsta = this->eewModel->record(i).value("percentsta").toDouble();
            eewInfo.misfit = this->eewModel->record(i).value("misfit").toDouble();
            eewInfo.sent_flag = this->eewModel->record(i).value("sent_flag").toInt();
            //free(temp);
            eewInfos.push_back(eewInfo);
        }
    }

    setAlertTab(onsiteInfos, eewInfos, pgaTime, pgaDetectInfos, pgaInfos, evid);
    //detailview->setup(onsiteInfos, eewInfos, pgaChannel, pgaTime, pgaInfos, evidS, configure.KGOM_HOME);
}

void MainWindow::setAlertTab(QVector<_KGOnSite_Info_t> onsiteInfos, QVector<_EEWInfo> eewInfos,
                             int pgaTime, QVector<_KGKIIS_GMPEAK_EVENT_STA_t> pgaDetectInfos, QVector<_KGKIIS_GMPEAK_EVENT_STA_t> pgaInfos, QString evid)
{
    QLayoutItem *child;

    while ((child = ui->alertListVLO->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    QMetaObject::invokeMethod(this->aRootObj, "clearMap", Q_RETURN_ARG(QVariant, aReturnedValue));

    double minLat = 999, maxLat = 0;
    double minLon = 999, maxLon = 0;

    ui->evidLB->setText("Event ID:" + evid);

    QString query = "SELECT lddate FROM event WHERE evid=" +  evid;
    this->eventModel->setQuery(query);

    if(!eewInfos.isEmpty())
    {
        QDateTime ttimeUTC, ttimeKST;
        ttimeUTC.setTimeSpec(Qt::UTC);
        ttimeUTC.setTime_t(eewInfos.first().origin_time);
        ttimeKST = convertKST(ttimeUTC);
        ui->timeLB->setText(ttimeKST.toString("yyyy-MM-dd hh:mm:ss ") +"KST");
        ui->eventTimeLCD->display(ttimeKST.toString("hh:mm:ss"));
        eventStartTimeKST = ttimeKST;

        double dist = getDistance(eewInfos.first().latitude, eewInfos.first().longitude, myPosition_Lat, myPosition_Lon);

        EewInfo *eew = new EewInfo;
        eew->setup(eewInfos.first(), QString::number(dist, 'f', 0), myPosition_Lat, myPosition_Lon);
        ui->alertListVLO->addWidget(eew);

        if(eewInfos.first().latitude < minLat) minLat = eewInfos.first().latitude;
        if(eewInfos.first().latitude > maxLat) maxLat = eewInfos.first().latitude;
        if(eewInfos.first().longitude < minLon) minLon = eewInfos.first().longitude;
        if(eewInfos.first().longitude > maxLon) maxLon = eewInfos.first().longitude;

        drawEEWOnMap(eewInfos.first());

        remainSecPAbsolute = (myRound(dist / pVelocity, 1));
        remainSecSAbsolute = (myRound(dist / sVelocity, 1));

        remainSecPRelative = remainSecPAbsolute;
        remainSecSRelative = remainSecSAbsolute;

        aniLat = eewInfos.first().latitude;
        aniLon = eewInfos.first().longitude;

        needAnimation = true;
        intensity = QString::number(getIntenFromMag(eewInfos.first().magnitude, dist));
        vibration = getStringFromInten(getIntenFromMag(eewInfos.first().magnitude, dist));
    }

    if(!onsiteInfos.isEmpty())
    {
        // sort infos,  using ttime/sta/net
        QVector<_KGOnSite_Info_t> tempInfos;

        for(int i=0;i<onsiteInfos.count();i++)
        {
            _KGOnSite_Info_t info = onsiteInfos.at(i);
            if(tempInfos.isEmpty())
                tempInfos.push_back(info);

            int isNewOrUpdate;  // new = 0, update = 1
            int index;
            for(int j=0;j<tempInfos.count();j++)
            {
                _KGOnSite_Info_t tempinfo = tempInfos.at(j);
                if(QString(info.sta).startsWith(QString(tempinfo.sta)) &&
                        info.ttime == tempinfo.ttime &&
                        QString(info.net).startsWith(QString(tempinfo.net)))
                {
                    isNewOrUpdate = 1;
                    index = j;
                    break;
                }
                isNewOrUpdate = 0;
            }
            if(isNewOrUpdate == 0)      // is new
                tempInfos.push_back(info);
            else                        // is update
                tempInfos.replace(index, info);
        }

        for(int i=0;i<tempInfos.count();i++)
        {
            if(i == 0 && eewInfos.count() == 0)
            {
                QDateTime ttimeUTC, ttimeKST;
                ttimeUTC.setTimeSpec(Qt::UTC);
                ttimeUTC.setTime_t(tempInfos.at(i).ttime);

                ttimeKST = convertKST(ttimeUTC);
                ui->timeLB->setText(ttimeKST.toString("yyyy-MM-dd hh:mm:ss ") +"KST");
                ui->eventTimeLCD->display(ttimeKST.toString("hh:mm:ss"));
                eventStartTimeKST = ttimeKST;
            }

            OnsiteInfo *onsiteinfo = new OnsiteInfo;
            onsiteinfo->setup(tempInfos.at(i));
            ui->alertListVLO->addWidget(onsiteinfo);

            if(QString(tempInfos.at(i).fromWhere).startsWith("K"))
            {
                drawOnsiteOnMap(1, tempInfos.at(i));

                if(tempInfos.at(i).lat < minLat) minLat = tempInfos.at(i).lat;
                if(tempInfos.at(i).lat > maxLat) maxLat = tempInfos.at(i).lat;
                if(tempInfos.at(i).lon < minLon) minLon = tempInfos.at(i).lon;
                if(tempInfos.at(i).lon > maxLon) maxLon = tempInfos.at(i).lon;
            }
            else if(QString(tempInfos.at(i).fromWhere).startsWith("L"))
            {
                drawOnsiteOnMap(0, tempInfos.at(i));

                if(tempInfos.at(i).lat < minLat) minLat = tempInfos.at(i).lat;
                if(tempInfos.at(i).lat > maxLat) maxLat = tempInfos.at(i).lat;
                if(tempInfos.at(i).lon < minLon) minLon = tempInfos.at(i).lon;
                if(tempInfos.at(i).lon > maxLon) maxLon = tempInfos.at(i).lon;
            }
        }
    }

    if(!pgaDetectInfos.isEmpty())
    {
        QVector<_KGKIIS_GMPEAK_EVENT_STA_t> tempInfos;

        for(int i=0;i<pgaDetectInfos.count();i++)
        {
            _KGKIIS_GMPEAK_EVENT_STA_t info = pgaDetectInfos.at(i);

            if(tempInfos.isEmpty())
            {
                tempInfos.append(info);
                continue;
            }

            bool inserted = false;

            for(int j=0;j<tempInfos.size();j++)
            {
                _KGKIIS_GMPEAK_EVENT_STA_t tempinfo = tempInfos.at(j);
                if(info.time < tempinfo.time)
                {
                    tempInfos.insert(j, info);
                    inserted = true;
                    break;
                }
            }

            if(!inserted)
                tempInfos.append(info);
        }

        for(int i=0;i<tempInfos.size();i++)
        {
            PgaAlertInfo *pgaAlertInfo = new PgaAlertInfo;
            pgaAlertInfo->setup(tempInfos.at(i));
            ui->alertListVLO->addWidget(pgaAlertInfo);

            /*
            drawPGAOnMap(pgaInfos.at(i));
            if(pgaInfos.at(i).lat < minLat) minLat = pgaInfos.at(i).lat;
            if(pgaInfos.at(i).lat > maxLat) maxLat = pgaInfos.at(i).lat;
            if(pgaInfos.at(i).lon < minLon) minLon = pgaInfos.at(i).lon;
            if(pgaInfos.at(i).lon > maxLon) maxLon = pgaInfos.at(i).lon;
            */
        }
    }

    if(pgaInfos.count() != 0)
    {
        PgaInfo *pgainfo = new PgaInfo;
        pgainfo->setup(pgaTime, pgaInfos);
        ui->alertListVLO->addWidget(pgainfo);

        for(int i=0;i<pgaInfos.count();i++)
        {
            drawPGAOnMap(pgaInfos.at(i));
            if(pgaInfos.at(i).lat < minLat) minLat = pgaInfos.at(i).lat;
            if(pgaInfos.at(i).lat > maxLat) maxLat = pgaInfos.at(i).lat;
            if(pgaInfos.at(i).lon < minLon) minLon = pgaInfos.at(i).lon;
            if(pgaInfos.at(i).lon > maxLon) maxLon = pgaInfos.at(i).lon;
        }
    }

    ui->alertListVLO->addStretch(1);

    if(myPosition_Lat < minLat) minLat = myPosition_Lat;
    if(myPosition_Lat > maxLat) maxLat = myPosition_Lat;
    if(myPosition_Lon < minLon) minLon = myPosition_Lon;
    if(myPosition_Lon > maxLon) maxLon = myPosition_Lon;

    QMetaObject::invokeMethod(this->aRootObj, "addMyPositionMarker",
                              Q_RETURN_ARG(QVariant, aReturnedValue),
                              Q_ARG(QVariant, myPosition_Lat), Q_ARG(QVariant, myPosition_Lon));

    double dist = getDistance(minLat, minLon, maxLat, maxLon);
    QMetaObject::invokeMethod(this->aRootObj, "adjustMap", Q_RETURN_ARG(QVariant, aReturnedValue),
                              Q_ARG(QVariant, (minLat+maxLat)/2), Q_ARG(QVariant, (minLon+maxLon)/2), Q_ARG(QVariant, dist));
}

void MainWindow::drawEEWOnMap(_EEWInfo eewInfo)
{
    QString tempMag = QString::number(eewInfo.magnitude, 'f', 1);

    QMetaObject::invokeMethod(this->aRootObj, "addEEWStarMarker",
                              Q_RETURN_ARG(QVariant, aReturnedValue),
                              Q_ARG(QVariant, eewInfo.latitude), Q_ARG(QVariant, eewInfo.longitude),
                              Q_ARG(QVariant, tempMag));
}

void MainWindow::drawPGAOnMap(_KGKIIS_GMPEAK_EVENT_STA_t pgaInfo)
{
    double lat, lon;
    lat = pgaInfo.lat;
    lon = pgaInfo.lon;
    QString pgaS;

    pgaS = QString::number(pgaInfo.maxH, 'f', 4);

    QString text = QString(pgaInfo.sta) + "\n" + pgaS + " gal";

    int net;    // 1 : KISS, 0 : LOCAL
    if(QString(pgaInfo.net).startsWith("KG") || QString(pgaInfo.net).startsWith("KS"))
        net = 1;
    else
        net = 0;

    QMetaObject::invokeMethod(this->aRootObj, "addStaMarker",
                              Q_RETURN_ARG(QVariant, aReturnedValue),
                              Q_ARG(QVariant, net),
                              Q_ARG(QVariant, lat), Q_ARG(QVariant, lon),
                              Q_ARG(QVariant, text));
}

void MainWindow::drawOnsiteOnMap(int net, _KGOnSite_Info_t info)
{
    double radius = info.distance * 1000;

    double lat, lon;
    lat = info.lat;
    lon = info.lon;

    QMetaObject::invokeMethod(this->aRootObj, "addStaMarker",
                              Q_RETURN_ARG(QVariant, aReturnedValue),
                              Q_ARG(QVariant, net),
                              Q_ARG(QVariant, lat), Q_ARG(QVariant, lon),
                              Q_ARG(QVariant, info.sta));


    QMetaObject::invokeMethod(this->aRootObj, "addCircle",
                              Q_RETURN_ARG(QVariant, aReturnedValue),
                              Q_ARG(QVariant, lat), Q_ARG(QVariant, lon),
                              Q_ARG(QVariant, radius));
}

