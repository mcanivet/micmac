﻿#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(int mode, QWidget *parent) :
        QMainWindow(parent),
        _ui(new Ui::MainWindow),
        _Engine(new cEngine),
        _layout(new QGridLayout),
        _zoomLayout(new QGridLayout),
        _params(new cParameters),
        _mode(mode)
{
    _ui->setupUi(this);

    _params->read();

    _Engine->setParams(_params);

    init(_params->getNbFen().x()*_params->getNbFen().y(), _mode > MASK3D);

    uint sy = 0;

    _layout->setContentsMargins(sy,sy,sy,sy);
    _layout->setHorizontalSpacing(sy);
    _layout->setVerticalSpacing(sy);
    _ui->OpenglLayout->setLayout(_layout);

#ifdef ELISE_Darwin
    _ui->actionRemove->setShortcut(QKeySequence(Qt::ControlModifier+ Qt::Key_Y));
    _ui->actionAdd->setShortcut(QKeySequence(Qt::ControlModifier+ Qt::Key_U));
#endif

    _ProgressDialog = new QProgressDialog("Loading files","Stop",0,100,this);

    connect(&_FutureWatcher, SIGNAL(finished()),_ProgressDialog,SLOT(cancel()));

    setUI();

    int cpt=0;
    for (int aK = 0; aK < _params->getNbFen().x();++aK)
        for (int bK = 0; bK < _params->getNbFen().y();++bK, cpt++)
            _layout->addWidget(getWidget(cpt), bK, aK);

    _signalMapper = new QSignalMapper (this);
    connectActions();

    labelShowMode(true);

    createRecentFileMenu();

    applyParams();

    if (_mode != MASK3D)
        setImagePosition(QPointF(-1.f,-1.f));
}

MainWindow::~MainWindow()
{
    delete _ui;
    delete _Engine;
    delete _RFMenu;
    delete _layout;
    delete _zoomLayout;
    delete _signalMapper;
    delete _params;
}

void MainWindow::connectActions()
{
    for (int aK = 0; aK < nbWidgets();++aK)
    {
        connect(getWidget(aK),	SIGNAL(filesDropped(const QStringList&)), this,	SLOT(addFiles(const QStringList&)));
        connect(getWidget(aK),	SIGNAL(overWidget(void*)), this,SLOT(changeCurrentWidget(void*)));

        connect(getWidget(aK),	SIGNAL(addPoint(QPointF)), this,SLOT(addPoint(QPointF)));
    }

    //File menu
    connect(_ui->actionClose_all, SIGNAL(triggered()), this, SLOT(closeAll()));

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        _recentFileActs[i] = new QAction(this);
        _recentFileActs[i]->setVisible(false);
        connect(_recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    //Zoom menu
    connect(_ui->action4_1_400,		    SIGNAL(triggered()),   _signalMapper, SLOT(map()));
    connect(_ui->action2_1_200,		    SIGNAL(triggered()),   _signalMapper, SLOT(map()));
    connect(_ui->action1_1_100,		    SIGNAL(triggered()),   _signalMapper, SLOT(map()));
    connect(_ui->action1_2_50,		    SIGNAL(triggered()),   _signalMapper, SLOT(map()));
    connect(_ui->action1_4_25,		    SIGNAL(triggered()),   _signalMapper, SLOT(map()));

    _signalMapper->setMapping (_ui->action4_1_400, 400);
    _signalMapper->setMapping (_ui->action2_1_200, 200);
    _signalMapper->setMapping (_ui->action1_1_100, 100);
    _signalMapper->setMapping (_ui->action1_2_50, 50);
    _signalMapper->setMapping (_ui->action1_4_25, 25);

    connect (_signalMapper, SIGNAL(mapped(int)), this, SLOT(zoomFactor(int)));
}
cAppli_SaisiePts *MainWindow::getAppliMetier() const
{
    return AppliMetier;
}

void MainWindow::setAppliMetier(cAppli_SaisiePts *value)
{
    AppliMetier = value;
}


void MainWindow::createRecentFileMenu()
{
    _RFMenu = new QMenu(tr("&Recent files"), this);

    _ui->menuFile->insertMenu(_ui->actionSettings, _RFMenu);

    for (int i = 0; i < MaxRecentFiles; ++i)
        _RFMenu->addAction(_recentFileActs[i]);

    updateRecentFileActions();
}

void MainWindow::setPostFix(QString str)
{
   _params->setPostFix(str);

   _Engine->setPostFix();
}

void MainWindow::progression()
{
    if(_incre)
        _ProgressDialog->setValue(*_incre);
}

void MainWindow::runProgressDialog(QFuture<void> future)
{
    _FutureWatcher.setFuture(future);
    _ProgressDialog->setWindowModality(Qt::WindowModal);
    _ProgressDialog->exec();

    future.waitForFinished();
}

void MainWindow::addFiles(const QStringList& filenames)
{
    if (filenames.size())
    {
        for (int i=0; i< filenames.size();++i)
        {
            if(!QFile(filenames[i]).exists())
            {
                QMessageBox::critical(this, "Error", "File does not exist (or bad argument)");
                return;
            }
        }

        _Engine->setFilenamesAndDir(filenames);

        QString suffix = QFileInfo(filenames[0]).suffix();

        if (suffix == "ply")
        {
            QTimer *timer_test = new QTimer(this);
            _incre = new int(0);
            connect(timer_test, SIGNAL(timeout()), this, SLOT(progression()));
            timer_test->start(10);

            runProgressDialog(QtConcurrent::run(_Engine, &cEngine::loadClouds,filenames,_incre));

            timer_test->stop();
            disconnect(timer_test, SIGNAL(timeout()), this, SLOT(progression()));
            delete _incre;
            delete timer_test;

            _mode = MASK3D;
        }
        else if (suffix == "xml")
        {
            runProgressDialog(QtConcurrent::run(_Engine, &cEngine::loadCameras, filenames));

            _ui->actionShow_cams->setChecked(true);

            _mode = MASK3D;
        }
        else // LOAD IMAGE
        {
            if (_mode <= MASK3D) closeAll();
            if (filenames.size() == 1) _mode = MASK2D;

            _Engine->loadImages(filenames);
        }

        _Engine->allocAndSetGLData(_mode > MASK3D, _params->getDefPtName());

        for (int aK = 0; aK < nbWidgets();++aK)
        {
            getWidget(aK)->setGLData(_Engine->getGLData(aK),_ui->actionShow_messages->isChecked());
            if (aK < filenames.size()) getWidget(aK)->getHistoryManager()->setFilename(_Engine->getFilenamesIn()[aK]);
        }

        for (int aK=0; aK < filenames.size();++aK) setCurrentFile(filenames[aK]);
    }
}

void MainWindow::on_actionFullScreen_toggled(bool state)
{   
    _params->setFullScreen(state);

    return state ? showFullScreen() : showNormal();
}

void MainWindow::on_actionShow_ball_toggled(bool state)
{
    if (_mode == MASK3D)
    {
        currentWidget()->setOption(cGLData::OpShow_Ball,state);

        if (state && _ui->actionShow_axis->isChecked())
        {
            currentWidget()->setOption(cGLData::OpShow_BBox,!state);
            _ui->actionShow_axis->setChecked(!state);
        }
    }
}

void MainWindow::on_actionShow_bbox_toggled(bool state)
{
    if (_mode == MASK3D)
        currentWidget()->setOption(cGLData::OpShow_BBox,state);
}

void MainWindow::on_actionShow_axis_toggled(bool state)
{
    if (_mode == MASK3D)
    {
        currentWidget()->setOption(cGLData::OpShow_Axis,state);

        if (state && _ui->actionShow_ball->isChecked())
        {
            currentWidget()->setOption(cGLData::OpShow_Ball,!state);
            _ui->actionShow_ball->setChecked(!state);
        }
    }
}

void MainWindow::on_actionShow_cams_toggled(bool state)
{
    if (_mode == MASK3D)
        currentWidget()->setOption(cGLData::OpShow_Cams,state);
}

void MainWindow::on_actionShow_messages_toggled(bool state)
{
    currentWidget()->setOption(cGLData::OpShow_Mess,state);

    labelShowMode(state);
}

void MainWindow::on_actionToggleMode_toggled(bool mode)
{
    if (_mode == MASK3D)
        currentWidget()->setInteractionMode(mode ? SELECTION : TRANSFORM_CAMERA,_ui->actionShow_messages->isChecked());
}

void MainWindow::on_actionHelpShortcuts_triggered()
{
    QString text = tr("File menu:") +"\n\n";
    if (_mode == MASK3D)
    {
        text += "Ctrl+P: \t" + tr("open .ply files")+"\n";
        text += "Ctrl+C: \t"+ tr("open .xml camera files")+"\n";
    }
    text += "Ctrl+O: \t"+tr("open image file")+"\n";
    if (_mode == MASK3D) text += "tr(""Ctrl+E: \t"+tr("save .xml selection infos")+"\n";
    text += "Ctrl+S: \t"+tr("save mask file")+"\n";
    text += "Ctrl+Maj+S: \t"+tr("save mask file as")+"\n";
    text += "Ctrl+X: \t"+tr("close files")+"\n";
    text += "Ctrl+Q: \t"+tr("quit") +"\n\n";
    text += tr("View menu:") +"\n\n";
    text += "F2: \t"+tr("full screen") +"\n";
    if (_mode == MASK3D)
    {
        text += "F3: \t"+tr("show axis") +"\n";
        text += "F4: \t"+tr("show ball") +"\n";
        text += "F5: \t"+tr("show bounding box") +"\n";
        text += "F6: \t"+tr("show cameras") +"\n";
    }
    text += "F7: \t"+tr("show messages") +"\n";

    if (_mode == MASK3D)
        text += tr("Key +/-: \tincrease/decrease point size") +"\n\n";
    else
    {
        text += tr("Key +/-: \tzoom +/-") + "\n";
        text += "9: \t"+tr("zoom fit") + "\n";
        text+= "4: \tzoom 400%\n";
        text+= "2: \tzoom 200%\n";
        text+= "1: \tzoom 100%\n";
        text+= "Ctrl+2: \tzoom 50%\n";
        text+= "Ctrl+4: \tzoom 25%\n";
    }

    text += "Shift+R: \t"+tr("reset view") +"\n\n";

    if (_mode <= MASK3D)
    {
        text += tr("Selection menu:") +"\n\n";
        if (_mode == MASK3D)
        {
            text += "F9: \t"+tr("move mode / selection mode (only 3D)") +"\n\n";
        }
        text += tr("Left click : \tadd a vertex to polyline") +"\n";
        text += tr("Right click: \tclose polyline or delete nearest vertex") +"\n";
        text += tr("Echap: \tdelete polyline") +"\n";

#ifdef ELISE_Darwin
        if (_mode == MASK3D)
        {
            text += tr("Ctrl+Y: \tadd points inside polyline") +"\n";
            text += tr("Ctrl+U: \tremove points inside polyline") +"\n";
        }
        else
        {
            text += tr("Ctrl+Y: \tadd pixels inside polyline") +"\n";
            text += tr("Ctrl+U: \tremove pixels inside polyline") +"\n";
        }
#else
        if (_mode == MASK3D)
        {
            text += tr("Space bar: \tadd points inside polyline") +"\n";
            text += tr("Del: \tremove points inside polyline") +"\n";
        }
        else
        {
            text += tr("Space bar: \tadd pixels inside polyline") +"\n";
            text += tr("Del: \tremove pixels inside polyline") +"\n";
        }
#endif

        text += tr("Shift+drag: \tinsert vertex in polyline") +"\n";
        text += tr("Ctrl+right click: remove last vertex") +"\n";
        text += tr("Drag & drop: move selected polyline vertex") +"\n";
        text += "Ctrl+A: \t"+tr("select all") +"\n";
        text += "Ctrl+D: \t"+tr("select none") +"\n";
        text += "Ctrl+R: \t"+tr("reset") +"\n";
        text += "Ctrl+I: \t"+tr("invert selection") +"\n";
    }
    else
    {
        text += tr("Click: \tadd point")+"\n";
        text += tr("Right click: \tchange selected point state")+"\n";
        text += tr("Drag & drop: \tmove selected point") +"\n";
        text += tr("Shift+right click: \tshow name menu")+"\n";
        text += tr("Ctrl+right click: \tshow window menu")+"\n\n";

        text += tr("History menu:") +"\n\n";
    }
    text += "Ctrl+Z: \t"+tr("undo last action") +"\n";
    text += "Ctrl+Shift+Z: "+tr("redo last action") +"\n";

    QMessageBox msgbox(QMessageBox::Information, tr("Saisie - shortcuts"),text);
    msgbox.setWindowFlags(msgbox.windowFlags() | Qt::WindowStaysOnTopHint);
    msgbox.exec();
}

void MainWindow::on_actionAbout_triggered()
{
    QFont font("Courier New", 9, QFont::Normal);

    QMessageBox msgbox(QMessageBox::NoIcon, tr("Saisie"),QString(getBanniereMM3D().c_str()));
    msgbox.setFont(font);

    //trick to enlarge QMessageBox...
    QSpacerItem* horizontalSpacer = new QSpacerItem(600, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)msgbox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

    msgbox.exec();
}

void MainWindow::on_actionAdd_triggered()
{
    currentWidget()->Select(ADD);
}

void MainWindow::on_actionSelect_none_triggered()
{
    currentWidget()->Select(NONE);
}

void MainWindow::on_actionInvertSelected_triggered()
{
    currentWidget()->Select(INVERT);
}

void MainWindow::on_actionSelectAll_triggered()
{
    currentWidget()->Select(ALL);
}

void MainWindow::on_actionReset_triggered()
{
    if (_mode != MASK3D)
    {
        closeAll();

        addFiles(_Engine->getFilenamesIn());
    }
    else
    {
        currentWidget()->Select(ALL);
    }
}

void MainWindow::on_actionRemove_triggered()
{
    if (_mode > MASK3D)
        currentWidget()->polygon().removeSelectedPoint();  //TODO: actuellement on ne garde pas le point selectionné (ajouter une action)
    else
        currentWidget()->Select(SUB);
}

void MainWindow::on_actionSetViewTop_triggered()
{
    if (_mode == MASK3D)
        currentWidget()->setView(TOP_VIEW);
}

void MainWindow::on_actionSetViewBottom_triggered()
{
    if (_mode == MASK3D)
        currentWidget()->setView(BOTTOM_VIEW);
}

void MainWindow::on_actionSetViewFront_triggered()
{
    if (_mode == MASK3D)
        currentWidget()->setView(FRONT_VIEW);
}

void MainWindow::on_actionSetViewBack_triggered()
{
    if (_mode == MASK3D)
        currentWidget()->setView(BACK_VIEW);
}

void MainWindow::on_actionSetViewLeft_triggered()
{
    if (_mode == MASK3D)
        currentWidget()->setView(LEFT_VIEW);
}

void MainWindow::on_actionSetViewRight_triggered()
{
    if (_mode == MASK3D)
        currentWidget()->setView(RIGHT_VIEW);
}

void MainWindow::on_actionReset_view_triggered()
{
    currentWidget()->resetView(true,true,true,true);
}

void MainWindow::on_actionZoom_Plus_triggered()
{
    currentWidget()->setZoom(currentWidget()->getZoom()*1.5f);
}

void MainWindow::on_actionZoom_Moins_triggered()
{
    currentWidget()->setZoom(currentWidget()->getZoom()/1.5f);
}

void MainWindow::on_actionZoom_fit_triggered()
{
    currentWidget()->zoomFit();
}

void MainWindow::zoomFactor(int aFactor)
{
    currentWidget()->zoomFactor(aFactor);
}

void MainWindow::on_actionLoad_plys_triggered()
{
    addFiles(QFileDialog::getOpenFileNames(NULL, tr("Open Cloud Files"),QString(), tr("Files (*.ply)")));
}

void MainWindow::on_actionLoad_camera_triggered()
{
    addFiles(QFileDialog::getOpenFileNames(NULL, tr("Open Camera Files"),QString(), tr("Files (*.xml)")));
}

void MainWindow::on_actionLoad_image_triggered()
{
    QString img_filename = QFileDialog::getOpenFileName(NULL, tr("Open Image File"),QString(), tr("File (*.*)"));

    if (!img_filename.isEmpty())
    {
        //TODO: factoriser
        QStringList & filenames = _Engine->getFilenamesIn();
        filenames.clear();
        filenames.push_back(img_filename);

        setCurrentFile(img_filename);

        addFiles(filenames);
    }
}

void MainWindow::on_actionSave_masks_triggered()
{
    _Engine->saveMask(currentWidgetIdx());
}

void MainWindow::on_actionSave_as_triggered()
{
    QString fname = QFileDialog::getSaveFileName(NULL, tr("Save mask Files"),QString(), tr("Files (*.*)"));

    if (!fname.isEmpty())
    {
        _Engine->setFilenameOut(fname);

        _Engine->saveMask(currentWidgetIdx());
    }
}

void MainWindow::on_actionSave_selection_triggered()
{
    currentWidget()->getHistoryManager()->save();
}

void MainWindow::on_actionSettings_triggered()
{
    cSettingsDlg uiSettings(this, _params);
    connect(&uiSettings, SIGNAL(hasChanged(bool)), this, SLOT(redraw(bool)));

    //uiSettings.setFixedSize(uiSettings.size());
    uiSettings.exec();

    /*#if defined(Q_OS_SYMBIAN)
        uiSettings.showMaximized();
    #else
        uiSettings.show();
    #endif*/

    disconnect(&uiSettings, 0, 0, 0);
}

void MainWindow::closeAll()
{
    _Engine->unloadAll();

    for (int aK=0; aK < nbWidgets(); ++aK)
        getWidget(aK)->reset();

    if (zoomWidget() != NULL)
    {
        zoomWidget()->reset();
        zoomWidget()->setOption(cGLData::OpShow_Mess,false);
    }
}

void MainWindow::closeCurrentWidget()
{
    _Engine->unloadAll();
    //_Engine->unload(currentWidgetIdx());

    currentWidget()->reset();
}

void MainWindow::openRecentFile()
{
    // A TESTER en multi images

#if WINVER == 0x0601 
	QAction *action = dynamic_cast<QAction *>(sender());
#else 
	QAction *action = qobject_cast<QAction *>(sender());
#endif

    if (action)
    {
        _Engine->setFilenamesAndDir(QStringList(action->data().toString()));

        addFiles(_Engine->getFilenamesIn());
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    // Rafraichit le menu des fichiers récents
    _curFile = fileName;
    setWindowFilePath(_curFile);

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    QString fname = QDir::toNativeSeparators(fileName);
    files.removeAll(fname);
    files.prepend(fname);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);
	
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {        
		#if WINVER == 0x0601 
			MainWindow *mainWin = dynamic_cast<MainWindow *>(widget);
		#else
			MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
		#endif
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
	
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 - %2").arg(i + 1).arg(strippedName(files[i]));
        _recentFileActs[i]->setText(text);
        _recentFileActs[i]->setData(files[i]);
        _recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        _recentFileActs[j]->setVisible(false);

    //m_RFMenu->setVisible(numRecentFiles > 0);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void hideAction(QAction* action, bool show)
{
    action->setVisible(show);
    action->setEnabled(show);
}

void MainWindow::setUI()
{
    labelShowMode(true);

    bool isMode3D = _mode == MASK3D;

    hideAction(_ui->actionLoad_plys,  isMode3D);
    hideAction(_ui->actionLoad_camera,isMode3D);
    hideAction(_ui->actionShow_cams,  isMode3D);
    hideAction(_ui->actionShow_axis,  isMode3D);
    hideAction(_ui->actionShow_ball,  isMode3D);
    hideAction(_ui->actionShow_bbox,  isMode3D);
    hideAction(_ui->actionShow_cams,  isMode3D);
    hideAction(_ui->actionToggleMode, isMode3D);

    _ui->menuStandard_views->menuAction()->setVisible(isMode3D);

    if (_mode > MASK3D)
    {
        if (_mode == POINT2D_INIT)          setWindowTitle("Micmac - SaisieAppuisInit QT");
        else if (_mode == POINT2D_PREDIC)   setWindowTitle("Micmac - SaisieAppuisPredic QT");

        //zoom Window
        _zoomLayout->addWidget(zoomWidget());
        _zoomLayout->setContentsMargins(2,2,2,2);

        _ui->zoomLayout->setLayout(_zoomLayout);
        _ui->zoomLayout->setContentsMargins(0,0,0,0);

        //disable some actions
        hideAction(_ui->actionAdd, false);
        hideAction(_ui->actionSelect_none, false);
        hideAction(_ui->actionInvertSelected, false);
        hideAction(_ui->actionSelectAll, false);
        hideAction(_ui->actionReset, false);

        hideAction(_ui->actionRemove, true);

        _ui->menuSelection->setTitle(tr("H&istory"));
    }
    else
    {
        _ui->verticalLayout->removeWidget(_ui->zoomLayout);
        _ui->verticalLayout->removeItem(_ui->verticalSpacer);

        delete _ui->zoomLayout;
        delete _ui->verticalSpacer;
    }
}

void  MainWindow::setGamma(float aGamma)
{
    _params->setGamma(aGamma);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (zoomWidget())
        _params->setZoomWindowValue(zoomWidget()->getZoom());

    _params->write();

    event->accept();
}

void MainWindow::redraw(bool nbWidgetsChanged)
{
    if (size() != _params->getSzFen())
    {
        if (_mode > MASK3D)
            resize(_params->getSzFen().width() + _ui->zoomLayout->width(), _params->getSzFen().height());
        else
            resize(_params->getSzFen());
    }

    if (nbWidgetsChanged)
    {
        delete _layout;
        _layout = new QGridLayout;

        int newWidgetNb = _params->getNbFen().x()*_params->getNbFen().y();
        int col =  _layout->columnCount();
        int row =  _layout->rowCount();

        if (col < _params->getNbFen().x() || row < _params->getNbFen().y())
        {
            widgetSetResize(newWidgetNb);

            int cpt = 0;
            for (; cpt < nbWidgets();++cpt)
                _layout->removeWidget(getWidget(cpt));

            cpt = 0;
            for (int aK =0; aK < _params->getNbFen().x();++aK)
                for (int bK =0; bK < _params->getNbFen().y();++bK)
                {
                    _layout->addWidget(getWidget(cpt), bK, aK);

                    if (cpt < _Engine->getData()->getNbImages())
                        getWidget(cpt)->setGLData(_Engine->getGLData(cpt),_ui->actionShow_messages);

                    cpt++;
                }
            _ui->OpenglLayout->setLayout(_layout);
        }
        else
        {
            //TODO
        }
    }
}

void MainWindow::setImagePosition(QPointF pt)
{
    QString text(tr("Image position : "));

    if (pt.x() >= 0.f && pt.y() >= 0.f)
    {
        GLWidget* glW = currentWidget();
        if (glW->hasDataLoaded() && !glW->getGLData()->is3D() && (glW->isPtInsideIm(pt)))
            text =  QString(text + QString::number(pt.x(),'f',1) + ", " + QString::number(pt.y(),'f',1)+" px");
    }

    _ui->label_PositionImage_1->setText(text);
    _ui->label_PositionImage_2->setText(text);
}

void MainWindow::setZoom(float val)
{
    _params->setZoomWindowValue(val);
}

void MainWindow::changeCurrentWidget(void *cuWid)
{
    GLWidget* glW = (GLWidget*)cuWid;

    setCurrentWidget(glW);

    if (_mode != MASK3D)
    {
        connect((GLWidget*)cuWid, SIGNAL(newImagePosition(QPointF)), this, SLOT(setImagePosition(QPointF)));

        connect((GLWidget*)cuWid, SIGNAL(gammaChanged(float)), this, SLOT(setGamma(float)));

        if (zoomWidget())
        {
            zoomWidget()->setGLData(glW->getGLData(),false,true,false,false);
            zoomWidget()->setZoom(_params->getZoomWindowValue());
            zoomWidget()->setOption(cGLData::OpShow_Mess,false);

            connect((GLWidget*)cuWid, SIGNAL(newImagePosition(QPointF)), zoomWidget(), SLOT(centerViewportOnImagePosition(QPointF)));
        }
    }
}

void MainWindow::RefreshPts()
{
    for (int var = 0; var < getAppliMetier()->nbImages(); ++var) {

        if(var<nbWidgets())
        {
            const std::vector<cSP_PointeImage *> &  aVP = getAppliMetier()->images(var)->VP();
            getWidget(var)->getGLData()->clearPolygon();

            for (int aK=0 ; aK<int(aVP.size()) ; aK++)
            {
                //if (WVisible(*(aVP[aK])))
                {
                    const cOneSaisie  & aSom = *(aVP[aK]->Saisie());
                    Pt2dr aP = aSom.PtIm();
                    //aP = mScr->to_win(aP);
                    //eEtatPointeImage aState = aSom.Etat();
                    getWidget(var)->addGlPoint(QPointF(aP.x,aP.y),QString(aSom.NamePt().c_str()));
                }
            }
            getWidget(var)->update();
        }
    }
}

void MainWindow::addPoint(QPointF point)
{
    Pt2dr aPGlob(point.x(),point.y());

    cCaseNamePoint aCNP("CHANGE",eCaseAutoNum);

    getAppliMetier()->images(0)->CreatePGFromPointeMono(aPGlob,eNSM_Pts,-1,&aCNP);

    RefreshPts();
}

void MainWindow::undo(bool undo)
{
    if (currentWidget()->getHistoryManager()->size())
    {
        if (_mode != MASK3D)
        {
            int idx = currentWidgetIdx();

            _Engine->reloadImage(idx);

            currentWidget()->setGLData(_Engine->getGLData(idx),_ui->actionShow_messages);
        }

        undo ? currentWidget()->getHistoryManager()->undo() : currentWidget()->getHistoryManager()->redo();
        currentWidget()->applyInfos();
    }
}

void MainWindow::applyParams()
{
    move(_params->getPosition());

    QSize szFen = _params->getSzFen();

    if (_params->getFullScreen())
    {
        showFullScreen();

        QRect screen = QApplication::desktop()->screenGeometry ( -1 );

        _params->setSzFen(screen.size());
        _params->setPosition(QPoint(0,0));

        _params->write();

        _ui->actionFullScreen->setChecked(true);
    }
    else if (_mode > MASK3D)
        resize(szFen.width() + _ui->zoomLayout->width(), szFen.height());
    else
        resize(szFen);
}

void MainWindow::labelShowMode(bool state)
{
    if ((!state) || (_mode == MASK3D))
    {
        _ui->label_PositionImage_1->hide();
        _ui->label_PositionImage_2->hide();
    }
    else
    {
        if(_mode == MASK2D)
        {
            _ui->label_PositionImage_1->hide();
            _ui->label_PositionImage_2->show();
        }
        else if(_mode > MASK3D)
        {
            _ui->label_PositionImage_1->show();
            _ui->label_PositionImage_2->hide();
        }
    }
}