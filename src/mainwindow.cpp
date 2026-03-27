/****************************************************************************\
 Copyright (c) 2010-2014 Stitch Works Software
 Brian C. Milco <bcmilco@gmail.com>

 This file is part of Crochet Charts.

 Crochet Charts is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Crochet Charts is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Crochet Charts. If not, see <http://www.gnu.org/licenses/>.

 \****************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stitchlibraryui.h"
#include "exportui.h"

#include "application.h"

#include "appinfo.h"
#include "settings.h"
#include "settingsui.h"
#include "theme.h"

#include "crochettab.h"
#include "stitchlibrary.h"
#include "stitchset.h"
#include "stitchpalettedelegate.h"

#include "stitchreplacerui.h"
#include "colorreplacer.h"

#include "debug.h"
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <QStandardItemModel>
#include <QStandardItem>

#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include <QActionGroup>
#include <QCloseEvent>
#include <QUndoStack>
#include <QUndoView>
#include <QTimer>

#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QMimeData>
#include <QSet>
#include <QStatusBar>

namespace {

int groupableSelectionCount(const QList<QGraphicsItem*> &items)
{
    int count = 0;
    foreach(QGraphicsItem *item, items) {
        if(item && item->type() != QGraphicsEllipseItem::Type)
            ++count;
    }
    return count;
}

int selectedGroupCount(const QList<QGraphicsItem*> &items)
{
    int count = 0;
    foreach(QGraphicsItem *item, items) {
        if(item && item->type() == ItemGroup::Type)
            ++count;
    }
    return count;
}

bool selectionItemLayer(QGraphicsItem *item, unsigned int *layerId)
{
    if(!item || !layerId)
        return false;

    switch(item->type()) {
    case Cell::Type:
        *layerId = qgraphicsitem_cast<Cell*>(item)->layer();
        return true;
    case Indicator::Type:
        *layerId = qgraphicsitem_cast<Indicator*>(item)->layer();
        return true;
    case ItemGroup::Type:
        *layerId = qgraphicsitem_cast<ItemGroup*>(item)->layer();
        return true;
    case ChartImage::Type:
        *layerId = qgraphicsitem_cast<ChartImage*>(item)->layer();
        return true;
    default:
        return false;
    }
}

QSet<unsigned int> selectionLayers(const QList<QGraphicsItem*> &items)
{
    QSet<unsigned int> layers;
    foreach(QGraphicsItem *item, items) {
        unsigned int layerId = 0;
        if(selectionItemLayer(item, &layerId))
            layers.insert(layerId);
    }
    return layers;
}

}

MainWindow::MainWindow(QStringList fileNames, QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    mUpdater(0),
    mResizeUI(0),
    mAlignDock(0),
    mRowsDock(0),
    mMirrorDock(0),
    mPropertiesDock(0),
    mEditMode(9),
    mStitch("ch"),
    mFgColor(QColor(Qt::black)),
    mBgColor(QColor(Qt::white))
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ui->setupUi(this);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTab(int)));
    
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    setUnifiedTitleAndToolBarOnMac(true);
    
#ifndef APPLE_APP_STORE
    bool checkForUpdates = Settings::inst()->value("checkForUpdates").toBool();
    if(checkForUpdates)
        checkUpdates();
#endif

    setupStitchPalette();
	setupLayersDock();
    setupDocks();
    
    mFile = new FileFactory(this);
    loadFiles(fileNames);

	setAcceptDrops(true);

    setApplicationTitle();
    setupNewTabDialog();

    setupMenus();
    setEditMode(mEditMode);
    updateSelectionDependentActions();
    readSettings();
    ui->newDocument->setProperty("panelSurface", true);
    Theme::polishMainWindow(this);

#ifdef Q_OS_MAC
    //File icon for titlebar
    fileIcon = QIcon(":/images/stitchworks-pattern.svg");
#else
    fileIcon = QIcon(":/images/CrochetCharts.png");
    setWindowIcon(fileIcon);
#endif
    QApplication::restoreOverrideCursor();
}

MainWindow::~MainWindow()
{
	delete mModeGroup;
	delete mSelectGroup;
	delete mGridGroup;
	delete ui;
	delete mFile;
	
	if (mUpdater)
		delete mUpdater;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
	QStringList files;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString &fileName = url.toLocalFile();
        files.append(fileName);
    }
	loadFiles(files);
}

void MainWindow::loadFiles(QStringList fileNames)
{
    
    if(fileNames.count() < 1)
        return;

    if(ui->tabWidget->count() < 1) {
        mFile->fileName = fileNames.takeFirst();
        int error = mFile->load();

        if(error != FileFactory::No_Error) {
            showFileError(error);
            return;
        }

        Settings::inst()->files.insert(mFile->fileName.toLower(), this);
        addToRecentFiles(mFile->fileName);
        ui->newDocument->hide();
    }

    foreach(QString fileName, fileNames) {
        QStringList files;
        files.append(fileName);
        MainWindow* newWin = new MainWindow(files);
        newWin->move(x() + 40, y() + 40);
        newWin->show();
        newWin->raise();
        newWin->activateWindow();
        Settings::inst()->files.insert(mFile->fileName.toLower(), newWin);
        addToRecentFiles(fileName);
    }
}

void MainWindow::checkUpdates(bool silent)
{
    if(mUpdater) {
        delete mUpdater;
        mUpdater = 0;
    }
    
    //TODO: check for updates in a separate thread.
    mUpdater = new Updater(this);
    // append the updater to the centralWidget to keep it out of the way of the menus.
    ui->centralWidget->layout()->addWidget(mUpdater); 
        
    mUpdater->checkForUpdates(silent); //check at startup is always silent.
}

void MainWindow::setApplicationTitle()
{
    QString curFile = mFile->fileName;
    setWindowModified(false);

    QString shownName = "";
    QString join = "";
    QIcon icon;

    if(curCrochetTab()) {
        if (curFile.isEmpty()) {
            shownName = "my design.pattern[*]";
        } else {
            shownName = QFileInfo(curFile).fileName() + "[*]";
#ifdef Q_OS_MAC
            icon = fileIcon;
#else
            icon = QIcon(":/images/CrochetCharts.png");
#endif
        }
        join = " - ";
    }
    QString title;

#ifdef Q_OS_MAC
    title = tr("%1%3%2").arg(shownName).arg(qApp->applicationName()).arg(join);
#else
    title = tr("%2%3%1").arg(shownName).arg(qApp->applicationName()).arg(join);
#endif

    setWindowTitle(title);
    setWindowFilePath(curFile);
    setWindowIcon(icon);

}

void MainWindow::setupNewTabDialog()
{
    int rows = Settings::inst()->value("rowCount").toInt();
    int stitches = Settings::inst()->value("stitchCount").toInt();
    QString defSt = Settings::inst()->value("defaultStitch").toString();
    QString defStyle = Settings::inst()->value("chartStyle").toString();
    int incBy = Settings::inst()->value("increaseBy").toInt();
    
    ui->rows->setValue(rows);
    ui->stitches->setValue(stitches);
    ui->increaseBy->setValue(incBy);

    ui->chartTemplate->clear();
    ui->chartTemplate->addItem(tr("Custom"), "custom");
    ui->chartTemplate->addItem(tr("Granny Square"), "granny_square");
    ui->grannyPreset->clear();
    ui->grannyPreset->addItem(tr("4-Round Granny Square"), "granny_4_round");
    ui->grannyPreset->addItem(tr("6-Round Granny Square"), "granny_6_round");
    ui->grannyPreset->addItem(tr("Solid Granny Square"), "solid_granny_square");
    ui->grannyStartType->clear();
    ui->grannyStartType->addItem(tr("Magic Ring"), "magic_ring");
    ui->grannyStartType->addItem(tr("Chain Ring"), "chain_ring");
    ui->grannyPreset->setCurrentIndex(1);
    ui->grannyStartType->setCurrentIndex(0);
    ui->grannyCornerArches->setValue(1);
    setGrannyTemplateControlsVisible(false);
    
    ui->defaultStitch->addItems(StitchLibrary::inst()->stitchList());
    ui->defaultStitch->setCurrentIndex(ui->defaultStitch->findText(defSt));

    ui->chartStyle->setCurrentIndex(ui->chartStyle->findText(defStyle));

    newChartUpdateStyle(defStyle);
    connect(ui->chartStyle, SIGNAL(currentIndexChanged(QString)), SLOT(newChartUpdateStyle(QString)));
    connect(ui->chartTemplate, SIGNAL(currentIndexChanged(int)), SLOT(newChartUpdateTemplate(int)));
    connect(ui->grannyPreset, SIGNAL(currentIndexChanged(int)), SLOT(newGrannyPresetChanged(int)));
    
    connect(ui->newDocBttnBox, SIGNAL(accepted()), this, SLOT(newChart()));
    connect(ui->newDocBttnBox, SIGNAL(rejected()), ui->newDocument, SLOT(hide()));   
}

void MainWindow::newChartUpdateStyle(QString style)
{

    if(style == tr("Blank")) {
        ui->rows->setVisible(false);
        ui->rowsLbl->setVisible(false);
        ui->stitches->setVisible(false);
        ui->stitchesLbl->setVisible(false);
        ui->rowSpacing->setVisible(false);
        ui->rowSpacingLbl->setVisible(false);
        ui->defaultStitch->setVisible(false);
        ui->defaultStitchLbl->setVisible(false);
        ui->increaseBy->setVisible(false);
        ui->increaseByLbl->setVisible(false);
    } else if(style == tr("Rounds")){
        ui->rows->setVisible(true);
        ui->rowsLbl->setVisible(true);
        ui->stitches->setVisible(true);
        ui->stitchesLbl->setVisible(true);
        ui->rowSpacing->setVisible(true);
        ui->rowSpacingLbl->setVisible(true);
        ui->defaultStitch->setVisible(true);
        ui->defaultStitchLbl->setVisible(true);
        ui->rowsLbl->setText(tr("Rounds:"));
        ui->stitchesLbl->setText(tr("Starting Stitches:"));
        ui->increaseBy->setVisible(true);
        ui->increaseByLbl->setVisible(true);
    } else if (style == tr("Rows")) {
        ui->rows->setVisible(true);
        ui->rowsLbl->setVisible(true);
        ui->stitches->setVisible(true);
        ui->stitchesLbl->setVisible(true);
        ui->rowSpacing->setVisible(true);
        ui->rowSpacingLbl->setVisible(true);
        ui->defaultStitch->setVisible(true);
        ui->defaultStitchLbl->setVisible(true);
        ui->rowsLbl->setText(tr("Rows:"));
        ui->stitchesLbl->setText(tr("Stitches:"));
        ui->increaseBy->setVisible(false);
        ui->increaseByLbl->setVisible(false);
    }
}

void MainWindow::newChartUpdateTemplate(int templateIndex)
{
    Q_UNUSED(templateIndex);
    applyChartTemplatePreset(currentChartTemplateKey());
}

void MainWindow::newGrannyPresetChanged(int presetIndex)
{
    Q_UNUSED(presetIndex);
    if(currentChartTemplateKey() == "granny_square")
        applyGrannyPreset(currentGrannyPresetKey());
}

void MainWindow::propertiesUpdate(QString property, QVariant newValue)
{

    if(!curCrochetTab())
        return;

    curCrochetTab()->propertiesUpdate(property, newValue);

}

void MainWindow::reloadLayerContent(QList<ChartLayer*>& layers, ChartLayer* selected)
{
	CrochetTab* tab = curCrochetTab();
	if (tab == NULL)
		return;

    QObject *source = sender();
    if(source && source != tab)
        return;
	
	QTreeView* view = ui->layersView;
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(view->model());
	
	//if there isn't a model yet, create one
	if (model == NULL) {
		model = new QStandardItemModel(0,2,this);
		//and connect the signals
		connect(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(layerModelChanged(const QModelIndex&)));
		view->setModel(model);
        connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(selectLayer(QModelIndex)));
	}
	
	//cleanup previous content
	model->clear();
	
	//set up the header of the model
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Visible"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
	
		
	//dont emit signals while populating the model
	model->blockSignals(true);
	
	QStandardItem* selecteditem = NULL;
	ChartLayer* layer;
	for (int i = 0 ; i < layers.count() ; i++) {
		//create the item
		layer = layers[i];
		
		QStandardItem* item = new QStandardItem(layer->name());
		
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		
		//set the checkbox
		item->setCheckable(true);
			
		//and add it to the list
		model->appendRow(item);
		
		if (layer->visible())
			item->setCheckState(Qt::Checked);
		else
			item->setCheckState(Qt::Unchecked);
		
		item->setData(QVariant::fromValue(static_cast<void*>(layer)), Qt::UserRole+5);
		
		if (layer == selected)
			selecteditem = item;
	}
	//let the model send signals again
	model->blockSignals(false);
	
	//finally, select the currently selected layer
	if (selecteditem != NULL)
		view->setCurrentIndex(selecteditem->index());
}

void MainWindow::setupStitchPalette()
{

    StitchSet* set = StitchLibrary::inst()->masterStitchSet();
    mProxyModel = new QSortFilterProxyModel(this);

    mProxyModel->setSourceModel(set);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->allStitches->setModel(mProxyModel);

    //TODO: setup a proxywidget that can hold header sections?
    StitchPaletteDelegate* delegate = new StitchPaletteDelegate(ui->allStitches);
    ui->allStitches->setItemDelegate(delegate);
    ui->allStitches->hideColumn(2);
    ui->allStitches->hideColumn(3);
    ui->allStitches->hideColumn(4);
    ui->allStitches->hideColumn(5);

    connect(ui->allStitches, SIGNAL(clicked(QModelIndex)), SLOT(selectStitch(QModelIndex)));
    connect(ui->patternStitches, SIGNAL(clicked(QModelIndex)), SLOT(selectStitch(QModelIndex)));

    connect(ui->patternColors, SIGNAL(clicked(QModelIndex)), SLOT(selectColor(QModelIndex)));
    connect(ui->fgColor, SIGNAL(colorChanged(QColor)), SLOT(addColor(QColor)));
    connect(ui->bgColor, SIGNAL(colorChanged(QColor)), SLOT(addColor(QColor)));
    connect(ui->patternColors, SIGNAL(bgColorSelected(QModelIndex)), SLOT(selectColor(QModelIndex)));

    connect(ui->stitchFilter, SIGNAL(textChanged(QString)), SLOT(filterStitchList(QString)));
}

void MainWindow::setupLayersDock()
{
	connect(ui->addLayerBtn, SIGNAL(released()), this, SLOT(addLayer()));
	connect(ui->removeLayerBtn, SIGNAL(released()), this, SLOT(removeLayer()));
	connect(ui->mergeBtn, SIGNAL(released()), this, SLOT(mergeLayer()));
	connect(ui->layersView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectLayer(const QModelIndex&)));
}

void MainWindow::setupDocks()
{
    //Undo Dock.
    mUndoDock = new QDockWidget(this);
    mUndoDock->setVisible(false);
    mUndoDock->setObjectName("undoHistory");
    QUndoView* view = new QUndoView(&mUndoGroup, mUndoDock);
    mUndoDock->setWidget(view);
    mUndoDock->setWindowTitle(tr("Undo History"));
    mUndoDock->setFloating(true);
	
	//Resize Dock
	mResizeUI = new ResizeUI(ui->tabWidget, this);
	connect(mResizeUI, SIGNAL(visibilityChanged(bool)), ui->actionShowResizeDock, SLOT(setChecked(bool)));
	connect(mResizeUI, SIGNAL(resize(QRectF)), SLOT(resize(QRectF)));
	connect(ui->tabWidget, SIGNAL(currentChanged(int)), mResizeUI, SLOT(updateContent(int)));
	
    //Align & Distribute Dock
    mAlignDock = new AlignDock(this);
    connect(mAlignDock, SIGNAL(align(int)), SLOT(alignSelection(int)));
    connect(mAlignDock, SIGNAL(distribute(int)), SLOT(distributeSelection(int)));
    connect(mAlignDock, SIGNAL(visibilityChanged(bool)), ui->actionShowAlignDock, SLOT(setChecked(bool)));

    //Rows & Stitches Dock.
    mRowsDock = new RowsDock(this);
    connect(mRowsDock, SIGNAL(arrangeGrid(QSize,QSize,QSize,bool)), SLOT(arrangeGrid(QSize,QSize,QSize,bool)));
    connect(mRowsDock, SIGNAL(visibilityChanged(bool)), ui->actionShowRowsDock, SLOT(setChecked(bool)));
    
    //Mirror & Rotate.
    mMirrorDock = new MirrorDock(this);
    connect(mMirrorDock, SIGNAL(mirror(int)), SLOT(mirror(int)));
	connect(mMirrorDock, SIGNAL(copy(int)), SLOT(copy(int)));
    connect(mMirrorDock, SIGNAL(rotate(qreal)), SLOT(rotate(qreal)));
    connect(mMirrorDock, SIGNAL(visibilityChanged(bool)), ui->actionShowMirrorDock, SLOT(setChecked(bool)));

    mPropertiesDock = new PropertiesDock(ui->tabWidget, this);
    connect(mPropertiesDock, SIGNAL(visibilityChanged(bool)), ui->actionShowProperties, SLOT(setChecked(bool)));
    connect(mPropertiesDock, SIGNAL(propertiesUpdated(QString,QVariant)), SLOT(propertiesUpdate(QString,QVariant)));
	connect(mPropertiesDock, SIGNAL(setGridType(QString)), SLOT(setSelectedGridMode(QString)));
}

void MainWindow::setupMenus()
{
    //File Menu
    connect(ui->menuFile, SIGNAL(aboutToShow()), SLOT(menuFileAboutToShow()));
    connect(ui->menuFile, SIGNAL(aboutToShow()), SLOT(menuRecentFilesAboutToShow()));
    connect(ui->actionNew, SIGNAL(triggered()), SLOT(fileNew()));
    connect(ui->actionOpen, SIGNAL(triggered()), SLOT(fileOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(fileSave()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), SLOT(fileSaveAs()));

    connect(ui->actionPrint, SIGNAL(triggered()), SLOT(filePrint()));
    connect(ui->actionPrintPreview, SIGNAL(triggered()), SLOT(filePrintPreview()));
    connect(ui->actionExport, SIGNAL(triggered()), SLOT(fileExport()));

    connect(ui->actionQuit, SIGNAL(triggered()), SLOT(close()));

    ui->actionOpen->setIcon(QIcon::fromTheme("document-open", QIcon(":/images/fileopen.png")));
    ui->actionNew->setIcon(QIcon::fromTheme("document-new", QIcon(":/images/filenew.png")));
    ui->actionSave->setIcon(QIcon::fromTheme("document-save", QIcon(":/images/filesave.png")));
    ui->actionSaveAs->setIcon(QIcon::fromTheme("document-save-as", QIcon(":/images/filesave.png")));

    ui->actionPrint->setIcon(QIcon::fromTheme("document-print", QIcon(":/images/fileprint.png")));
    ui->actionPrintPreview->setIcon(QIcon::fromTheme("document-print-preview", QIcon(":/images/document-print-preview.png")));

    /*document-export*/
    ui->actionQuit->setIcon(QIcon::fromTheme("application-exit", QIcon(":/images/application-exit.png")));

    setupRecentFiles();

    //Edit Menu
    connect(ui->menuEdit, SIGNAL(aboutToShow()), SLOT(menuEditAboutToShow()));

    mActionUndo = mUndoGroup.createUndoAction(this, tr("Undo"));
    mActionRedo = mUndoGroup.createRedoAction(this, tr("Redo"));

    ui->menuEdit->insertAction(ui->actionCopy, mActionUndo);
    ui->menuEdit->insertAction(ui->actionCopy, mActionRedo);
    ui->menuEdit->insertSeparator(ui->actionCopy);

    ui->mainToolBar->insertAction(0, mActionUndo);
    ui->mainToolBar->insertAction(0, mActionRedo);
    ui->mainToolBar->insertSeparator(mActionUndo);

    mActionUndo->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/images/editundo.png")));
    mActionRedo->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/images/editredo.png")));
    mActionUndo->setShortcut(QKeySequence::Undo);
    mActionRedo->setShortcut(QKeySequence::Redo);

    ui->actionCopy->setIcon(QIcon::fromTheme("edit-copy", QIcon(":/images/editcopy.png")));
    ui->actionCut->setIcon(QIcon::fromTheme("edit-cut", QIcon(":/images/editcut.png")));
    ui->actionPaste->setIcon(QIcon::fromTheme("edit-paste", QIcon(":/images/editpaste.png")));

    connect(ui->actionCopy, SIGNAL(triggered()), SLOT(copy()));
    connect(ui->actionPaste, SIGNAL(triggered()), SLOT(paste()));
    connect(ui->actionCut, SIGNAL(triggered()), SLOT(cut()));
	connect(ui->actionInsertImage, SIGNAL(triggered()), SLOT(insertImage()));

    ui->fgColor->setColor(QColor(Settings::inst()->value("stitchPrimaryColor").toString()));
    ui->bgColor->setColor(QColor(Qt::white));

    //View Menu
    connect(ui->menuView, SIGNAL(aboutToShow()), SLOT(menuViewAboutToShow()));
    connect(ui->actionShowStitches, SIGNAL(triggered()), SLOT(viewShowStitches()));
    connect(ui->actionShowPatternColors, SIGNAL(triggered()), SLOT(viewShowPatternColors()));
    connect(ui->actionShowPatternStitches, SIGNAL(triggered()), SLOT(viewShowPatternStitches()));
	connect(ui->actionShowLayers, SIGNAL(triggered()), SLOT(viewShowLayers()));

    connect(ui->actionShowUndoHistory, SIGNAL(triggered()), SLOT(viewShowUndoHistory()));
    
    connect(ui->actionShowMainToolbar, SIGNAL(triggered()), SLOT(viewShowMainToolbar()));
    connect(ui->actionShowEditModeToolbar, SIGNAL(triggered()), SLOT(viewShowEditModeToolbar()));
    
    connect(ui->actionViewFullScreen, SIGNAL(triggered(bool)), SLOT(viewFullScreen(bool)));

    connect(ui->actionZoomIn, SIGNAL(triggered()), SLOT(viewZoomIn()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), SLOT(viewZoomOut()));
    
    ui->actionZoomIn->setIcon(QIcon::fromTheme("zoom-in", QIcon(":/images/zoomin.png")));
    ui->actionZoomOut->setIcon(QIcon::fromTheme("zoom-out", QIcon(":/images/zoomout.png")));
    ui->actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    ui->actionZoomOut->setShortcut(QKeySequence::ZoomOut);

    connect(ui->actionShowProperties, SIGNAL(triggered()), SLOT(viewShowProperties()));
    
    //Modes menu
    connect(ui->menuModes, SIGNAL(aboutToShow()), SLOT(menuModesAboutToShow()));

    mModeGroup = new QActionGroup(this);
    mModeGroup->addAction(ui->actionStitchMode);
    mModeGroup->addAction(ui->actionMoveMode);
    mModeGroup->addAction(ui->actionColorMode);
    mModeGroup->addAction(ui->actionAngleMode);
    mModeGroup->addAction(ui->actionStretchMode);
    mModeGroup->addAction(ui->actionCreateRows);
    mModeGroup->addAction(ui->actionIndicatorMode);

    connect(mModeGroup, SIGNAL(triggered(QAction*)), SLOT(changeTabMode(QAction*)));

	mSelectGroup = new QActionGroup(this);
	mSelectGroup->addAction(ui->actionBoxSelectMode);
	mSelectGroup->addAction(ui->actionLassoSelectMode);
	mSelectGroup->addAction(ui->actionLineSelectMode);
	ui->actionBoxSelectMode->setChecked(true);
	
	connect(mSelectGroup, SIGNAL(triggered(QAction*)), SLOT(changeSelectMode(QAction*)));
    connect(ui->actionNextSelectMode, SIGNAL(triggered()), SLOT(nextSelectMode()));
	
	addAction(ui->actionNextSelectMode);
	
	mGridGroup = new QActionGroup(this);
	mGridGroup->addAction(ui->actionGridNone);
	mGridGroup->addAction(ui->actionGridSquare);
	mGridGroup->addAction(ui->actionGridRound);
	mGridGroup->addAction(ui->actionGridTriangle);
	ui->actionGridNone->setChecked(true);
	
	connect(mGridGroup, SIGNAL(triggered(QAction*)), SLOT(changeGridMode(QAction*)));
	connect(mGridGroup, SIGNAL(triggered(QAction*)), mPropertiesDock, SLOT(propertyUpdated()));
	connect(ui->actionNextGridMode, SIGNAL(triggered()), SLOT(nextGridMode()));
	connect(ui->actionNextGridMode, SIGNAL(triggered()), mPropertiesDock, SLOT(propertyUpdated()));
    connect(ui->actionSnapToGrid, SIGNAL(toggled(bool)), SLOT(toggleSnapToGrid(bool)));
    ui->actionSnapToGrid->setChecked(Settings::inst()->value("snapToGrid").toBool());
	
	addAction(ui->actionNextGridMode);
	
    //Charts Menu
    connect(ui->actionAddChart, SIGNAL(triggered()), SLOT(documentNewChart()));
    connect(ui->actionRemoveTab, SIGNAL(triggered()), SLOT(removeCurrentTab()));

    connect(ui->actionShowChartCenter, SIGNAL(triggered()), SLOT(chartsShowChartCenter()));
    
    ui->actionRemoveTab->setIcon(QIcon::fromTheme("tab-close", QIcon(":/images/tabclose.png")));
    
    connect(ui->menuChart, SIGNAL(aboutToShow()), SLOT(menuChartAboutToShow()));
    connect(ui->actionEditName, SIGNAL(triggered()), SLOT(chartEditName()));
    //TODO: get more icons from the theme for use with table editing.
    //http://doc.qt.nokia.com/4.7/qstyle.html#StandardPixmap-enum

    connect(ui->actionCreateRows, SIGNAL(toggled(bool)), SLOT(chartCreateRows(bool)));
    
    //Stitches Menu
    connect(ui->actionShowAlignDock, SIGNAL(triggered()), SLOT(viewShowAlignDock()));
    connect(ui->actionShowRowsDock, SIGNAL(triggered()), SLOT(viewShowRowsDock()));
    connect(ui->actionShowMirrorDock, SIGNAL(triggered()), SLOT(viewShowMirrorDock()));
	connect(ui->actionShowResizeDock, SIGNAL(triggered()), SLOT(viewShowResizeDock()));

    connect(ui->actionGroup, SIGNAL(triggered()), SLOT(group()));
    connect(ui->actionUngroup, SIGNAL(triggered()), SLOT(ungroup()));

    connect(ui->actionReplaceStitch, SIGNAL(triggered()), SLOT(stitchesReplaceStitch()));
    connect(ui->actionColorReplacer, SIGNAL(triggered()), SLOT(stitchesReplaceColor()));

    //stitches menu
    connect(ui->menuStitches, SIGNAL(aboutToShow()), SLOT(menuStitchesAboutToShow()));

    //Tools Menu
    connect(ui->menuTools, SIGNAL(aboutToShow()), SLOT(menuToolsAboutToShow()));
    connect(ui->actionOptions, SIGNAL(triggered()), SLOT(toolsOptions()));
    connect(ui->actionStitchLibrary, SIGNAL(triggered()), SLOT(toolsStitchLibrary()));
    connect(ui->actionCheckForUpdates, SIGNAL(triggered()), SLOT(toolsCheckForUpdates()));
    
#ifdef APPLE_APP_STORE
    ui->actionCheckForUpdates->setVisible(false);
#endif

    //Help Menu
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(helpAbout()));
    connect(ui->actionCrochetHelp, SIGNAL(triggered()), SLOT(helpCrochetHelp()));
    
    //misc items
    connect(&mUndoGroup, SIGNAL(isModified(bool)), SLOT(documentIsModified(bool)));
    connect(ui->clearBttn, SIGNAL(clicked()), SLOT(clearStitchFilter()));
    
    updateMenuItems();
}

void MainWindow::openRecentFile()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QAction* action = qobject_cast<QAction*>(sender());
    if (action && QFileInfo(action->data().toString()).exists()) {
        QStringList files;
        files.append(action->data().toString());
        loadFiles(files);
    }

    setApplicationTitle();
    updateMenuItems();
    QApplication::restoreOverrideCursor();
}

void MainWindow::addToRecentFiles(QString fileName)
{
    Settings::inst()->addRecentFile(fileName);
}

void MainWindow::menuRecentFilesAboutToShow()
{
    setupRecentFiles();
}

void MainWindow::setupRecentFiles()
{
    QStringList files;
    QStringList list = Settings::inst()->recentFiles();
    
    int maxRecentFiles = Settings::inst()->value("maxRecentFiles").toInt();
    mRecentFilesActs.clear();

    int i = 0;
    //remove any files that have been deleted or are not available.
    foreach(QString file, list) {
        if(QFileInfo(file).exists()) {
            files.append(file);
            i++;
            if(i >= maxRecentFiles)
                break;
        }
    }
    
    for(int i = 0; i < files.count(); ++i) {

        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        QAction* a = new QAction(this);
        connect(a, SIGNAL(triggered()), SLOT(openRecentFile()));
        
        a->setText(text);
        a->setData(files[i]);
        if(i < maxRecentFiles)
            a->setVisible(true);
        else
            a->setVisible(false);
        mRecentFilesActs.append(a);
    }

    ui->menuOpenRecent->clear();
    ui->menuOpenRecent->addActions(mRecentFilesActs);

    //update the master list.
    Settings::inst()->setRecentFiles(files);
}

void MainWindow::updateMenuItems()
{
    menuFileAboutToShow();
    menuEditAboutToShow();
    menuViewAboutToShow();
    menuModesAboutToShow();
    menuChartAboutToShow();
    menuStitchesAboutToShow();
}

void MainWindow::filePrint()
{
    //TODO: page count isn't working...
    QPrinter printer;
    QPrintDialog* dialog = new QPrintDialog(&printer, this);

    if(dialog->exec() != QDialog::Accepted)
        return;

    print(&printer);
}

void MainWindow::print(QPrinter* printer)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    
    int tabCount = ui->tabWidget->count();
    QPainter* p = new QPainter();
    
    p->begin(printer);

    bool firstPass = true;
    for(int i = 0; i < tabCount; ++i) {
        if(!firstPass)
            printer->newPage();
        
        CrochetTab* tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(i));
        tab->renderChart(p);
        firstPass = false;
    }
    p->end();
    
    QApplication::restoreOverrideCursor();
}

void MainWindow::filePrintPreview()
{
    //FIXME: this isn't working
    QPrinter* printer = new QPrinter(QPrinter::HighResolution);
    QPrintPreviewDialog* dialog = new QPrintPreviewDialog(printer, this);
    connect(dialog, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
    
    dialog->exec();
}

void MainWindow::fileExport()
{
    if(!hasTab())
        return;
    
    ExportUi d(ui->tabWidget, &mPatternStitches, &mPatternColors, this);
    d.exec();
}

void MainWindow::addColor(QColor color)
{
    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        CrochetTab* tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(i));
        if(!tab)
            return;

        if(sender() == ui->fgColor) {
            tab->setEditFgColor(color);
        } else if (sender() == ui->bgColor) {
            tab->setEditBgColor(color);
        }
    }

}

void MainWindow::updateDefaultStitchColor(QColor originalColor, QColor newColor)
{
    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        CrochetTab *tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(i));
        if(!tab)
            continue;

        tab->updateDefaultStitchColor(originalColor, newColor);
    }
}

void MainWindow::selectStitch(QModelIndex index)
{
    QModelIndex idx;
    
    if(sender() == ui->allStitches) {
        const QSortFilterProxyModel *model =  static_cast<const QSortFilterProxyModel*>(index.model());
        idx = model->mapToSource(model->index(index.row(), 0));
        
    } else
        idx = index;
    
    QString stitch = idx.data(Qt::DisplayRole).toString();

    if(stitch.isEmpty())
        return;
    
    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        CrochetTab *tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(i));
        if(tab)
            tab->setEditStitch(stitch);
    }

    setEditMode(10);
}

void MainWindow::selectColor(QModelIndex index)
{
    QString color = index.data(Qt::ToolTipRole).toString();

    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        CrochetTab *tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(i));
        if(tab) {
            tab->setEditFgColor(color);
            ui->fgColor->setColor(color);
        }
    }
}

void MainWindow::filterStitchList(QString newText)
{
    QRegExp regExp(newText, Qt::CaseInsensitive, QRegExp::FixedString);
    mProxyModel->setFilterRegExp(regExp);
}

void MainWindow::clearStitchFilter()
{
    ui->stitchFilter->clear();
}

void MainWindow::documentNewChart()
{
    int rows = Settings::inst()->value("rowCount").toInt();
    int stitches = Settings::inst()->value("stitchCount").toInt();
    int incBy = Settings::inst()->value("increaseBy").toInt();
    
    ui->rows->setValue(rows);
    ui->stitches->setValue(stitches);
    ui->increaseBy->setValue(incBy);
    
    ui->chartTitle->setText(nextChartName());
    applyChartTemplatePreset(currentChartTemplateKey());

    if(ui->newDocument->isVisible()) {
        QPalette pal = ui->newDocument->palette();
        mNewDocWidgetColor = ui->newDocument->palette().color(ui->newDocument->backgroundRole());
        pal.setColor(ui->newDocument->backgroundRole(), ui->newDocument->palette().highlight().color());
        ui->newDocument->setPalette(pal);
        QTimer::singleShot(1500, this, SLOT(flashNewDocDialog()));
    } else 
        ui->newDocument->show();
}

void MainWindow::helpCrochetHelp()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QString path = QApplication::applicationDirPath();
    QString file ="";
#ifdef Q_OS_WIN
    file = QString("%1/Crochet_Charts_User_Guide_%2.pdf").arg(path).arg(AppInfo::inst()->appVersionShort);
    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
#endif

#ifdef Q_OS_MAC
    file = QString("%1/CrochetCharts_User_Guide_%2.pdf").arg(path).arg(AppInfo::inst()->appVersionShort);
    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
#endif

#ifdef Q_OS_LINUX
    file = QString("%1/../share/CrochetCharts/CrochetCharts_User_Guide_%2.pdf").arg(path).arg(AppInfo::inst()->appVersionShort);
    QDesktopServices::openUrl(QUrl::fromLocalFile(file));
#endif //Q_OS_WIN

    QApplication::restoreOverrideCursor();

}

void MainWindow::helpAbout()
{
    QString aboutInfo = QString(tr("<h1>%1</h1>"
                                   "<p>Version: %2 (built on %3)</p>"
                                   "<p>Copyright (c) %4 %5</p>"
                                   "<p>This software is for creating crochet charts that"
                                   " can be exported in many different file types.</p>")
                                .arg(qApp->applicationName())
                                .arg(qApp->applicationVersion())
                                .arg(AppInfo::inst()->appBuildInfo)
                                .arg(AppInfo::inst()->projectLife)
                                .arg(qApp->organizationName())
                                );
    QString fName = Settings::inst()->value("firstName").toString();
    QString lName = Settings::inst()->value("lastName").toString();
    QString email = Settings::inst()->value("email").toString();
    QString sn = Settings::inst()->value("serialNumber").toString();

    QString dedication = tr("<p>This version is dedicated to:<br /> My Grandmother (Aug 20, 1926 - Jan 8, 2013)</p>");
    aboutInfo.append(dedication);
    
    QString licenseInfo;

    licenseInfo = QString(tr("<p>This version is released under the GPLv3 open source license.</p>"));

    aboutInfo.append(licenseInfo);
    QMessageBox::about(this, tr("About Crochet Charts"), aboutInfo);
}

void MainWindow::closeEvent(QCloseEvent* event)
{

    if(safeToClose()) {
        Settings::inst()->setValue("geometry", saveGeometry());
        Settings::inst()->setValue("windowState", saveState());

        if(Settings::inst()->files.contains(mFile->fileName.toLower()))
            Settings::inst()->files.remove(mFile->fileName.toLower());

        mFile->cleanUp();

        mPropertiesDock->closing = true;
        QMainWindow::closeEvent(event);
    } else {
        event->ignore();
    }

}

bool MainWindow::safeToClose()
{
    //only prompt to save the file if it has tabs.
    if(ui->tabWidget->count() > 0) {
        if(isWindowModified())
            return promptToSave();
    }

    return true;
}

bool MainWindow::promptToSave()
{
    QString niceName = QFileInfo(mFile->fileName).baseName();
    if(niceName.isEmpty())
        niceName = "my design";
    
    QMessageBox msgbox(this);
    msgbox.setText(tr("The document '%1' has unsaved changes.").arg(niceName));
    msgbox.setInformativeText(tr("Do you want to save the changes?"));
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    
    int results = msgbox.exec();
    
    if(results == QMessageBox::Cancel)
        return false;
    else if(results == QMessageBox::Discard)
        return true;
    else if (results == QMessageBox::Save) {
        //FIXME: if the user cancels the fileSave() we should drop them back to the window not close it.
        fileSave();
        return true;
    }

    return false;
}

void MainWindow::readSettings()
{
    //TODO: For full session restoration reimplement QApplication::commitData()
    //See: http://doc.qt.nokia.com/stable/session.html
    restoreGeometry(Settings::inst()->value("geometry").toByteArray());
    restoreState(Settings::inst()->value("windowState").toByteArray());

}

void MainWindow::menuToolsAboutToShow()
{
}

void MainWindow::changeSelectMode(QAction* action)
{
	CrochetTab* tab = curCrochetTab();
	if (tab) {
		if (action == ui->actionBoxSelectMode)
			tab->setSelectMode(Scene::BoxSelect);
		else if (action == ui->actionLassoSelectMode)
			tab->setSelectMode(Scene::LassoSelect);
		else if (action == ui->actionLineSelectMode)
			tab->setSelectMode(Scene::LineSelect);
	}
}

void MainWindow::nextSelectMode()
{
	if (ui->actionBoxSelectMode->isChecked())
		ui->actionLassoSelectMode->trigger();
	else if (ui->actionLassoSelectMode->isChecked())
		ui->actionLineSelectMode->trigger();
	else if (ui->actionLineSelectMode->isChecked())
		ui->actionBoxSelectMode->trigger();
}

void MainWindow::changeGridMode(QAction* action)
{
	CrochetTab* tab = curCrochetTab();
	if (tab) {
		if (action == ui->actionGridNone)
			tab->setGuidelinesType("None");
		else if (action == ui->actionGridSquare)
			tab->setGuidelinesType("Rows");
		else if (action == ui->actionGridRound)
			tab->setGuidelinesType("Rounds");
		else if (action == ui->actionGridTriangle)
			tab->setGuidelinesType("Triangles");
	}
    updateGridSnapActionState();
}

void MainWindow::setSelectedGridMode(QString gmode) {
	CrochetTab* tab = curCrochetTab();
	if (tab == NULL)
		return;
		
	ui->actionGridNone->blockSignals(true);
	ui->actionGridSquare->blockSignals(true);
	ui->actionGridRound->blockSignals(true);
	ui->actionGridTriangle->blockSignals(true);
	
	ui->actionGridNone->setChecked(false);
	ui->actionGridSquare->setChecked(false);
	ui->actionGridRound->setChecked(false);
	ui->actionGridTriangle->setChecked(false);
	
	
	if (gmode.compare("None") == 0) {
		ui->actionGridNone->setChecked(true);
	} else if (gmode.compare("Rows") == 0) {
		ui->actionGridSquare->setChecked(true);
	} else if (gmode.compare("Rounds") == 0) {
		ui->actionGridRound->setChecked(true);
	} else if (gmode.compare("Triangles") == 0) {
		ui->actionGridTriangle->setChecked(true);
	}
	ui->actionGridNone->blockSignals(false);
	ui->actionGridSquare->blockSignals(false);
	ui->actionGridRound->blockSignals(false);
	ui->actionGridTriangle->blockSignals(false);

    updateGridSnapActionState();
}

void MainWindow::nextGridMode()
{
	if (ui->actionGridNone->isChecked())
		ui->actionGridSquare->trigger();
	else if (ui->actionGridSquare->isChecked())
		ui->actionGridRound->trigger();
	else if (ui->actionGridRound->isChecked())
		ui->actionGridTriangle->trigger();
	else if (ui->actionGridTriangle->isChecked())
		ui->actionGridNone->trigger();
}

void MainWindow::toggleSnapToGrid(bool state)
{
    Settings::inst()->setValue("snapToGrid", state);

    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        CrochetTab* tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(i));
        if(tab)
            tab->setSnapToGrid(state);
    }

    updateGridSnapActionState();
    if(statusBar())
        statusBar()->showMessage(state ? tr("Snap to grid enabled") : tr("Snap to grid disabled"), 2500);
}
	
void MainWindow::toolsOptions()
{
    SettingsUi dialog(this);

    dialog.exec();
    if(curCrochetTab()) {
        curCrochetTab()->sceneUpdate();
        if(dialog.stitchColorUpdated)
            updateDefaultStitchColor(dialog.mOriginalColor, dialog.mNewColor);
    }
}

void MainWindow::fileOpen()
{
    QString fileLoc = Settings::inst()->value("fileLocation").toString();
    
    QFileDialog* fd = new QFileDialog(this, tr("Open Pattern File"), fileLoc, tr("Pattern File (*.pattern);; All files (*.*)"));
    fd->setWindowFlags(Qt::Sheet);
    fd->setObjectName("fileopendialog");
    fd->setViewMode(QFileDialog::List);
    fd->setFileMode( QFileDialog::ExistingFile );
    fd->setAcceptMode(QFileDialog::AcceptOpen);
    fd->open(this, SLOT(loadFile(QString)));
   
}

void MainWindow::loadFile(QString fileName)
{
    
    if(fileName.isEmpty() || fileName.isNull())
        return;
    
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    
    if(!Settings::inst()->files.contains(fileName.toLower())) {
        if(ui->tabWidget->count() > 0) {
            QStringList files;
            files.append(fileName);
            MainWindow* newWin = new MainWindow(files);
            newWin->move(x() + 40, y() + 40);
            newWin->show();
            newWin->raise();
            newWin->activateWindow();
            Settings::inst()->files.insert(fileName.toLower(), newWin);
        } else {
            ui->newDocument->hide();
            mFile->fileName = fileName;
            int error = mFile->load();

            if(error != FileFactory::No_Error) {
                showFileError(error);
                return;
            }

            Settings::inst()->files.insert(mFile->fileName.toLower(), this);
        }
        
        addToRecentFiles(fileName);
        
        setApplicationTitle();
        updateMenuItems();
    } else {
        //show the window if it's already open.
        MainWindow* win = Settings::inst()->files.find(fileName.toLower()).value();
        win->raise();
    }
    QApplication::restoreOverrideCursor();   
}

void MainWindow::fileSave()
{

    if(ui->tabWidget->count() <= 0) {
        QMessageBox msgbox;
        msgbox.setText(tr("%1 cannot save a document without at least one (1) chart.").arg(qAppName()));
        msgbox.exec();
        return;
    }
    
    if(mFile->fileName.isEmpty())
        fileSaveAs();
    else {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        FileFactory::FileError err = mFile->save();
        if(err != FileFactory::No_Error) {
            qWarning() << "There was an error saving the file: " << err;
            QMessageBox msgbox;
            msgbox.setText(tr("There was an error saving the file."));
            msgbox.setIcon(QMessageBox::Critical);
            msgbox.exec();
        }

        documentIsModified(false);
        QApplication::restoreOverrideCursor();
    }
}

void MainWindow::fileSaveAs()
{
    QString fileLoc = Settings::inst()->value("fileLocation").toString();

    QFileDialog* fd = new QFileDialog(this, tr("Save Pattern File"), fileLoc,
                                      tr("Pattern v1.2 (*.pattern);;Pattern v1.0/v1.1 (*.pattern)"));
    fd->setWindowFlags(Qt::Sheet);
    fd->setObjectName("filesavedialog");
    fd->setViewMode(QFileDialog::List);
    fd->setFileMode( QFileDialog::AnyFile );
    fd->setAcceptMode(QFileDialog::AcceptSave);
    if(mFile->fileName.isEmpty())
        fd->selectFile("my design.pattern");
    else
        fd->selectFile(mFile->fileName);

    fd->open(this, SLOT(saveFileAs(QString)));
}

void MainWindow::saveFileAs(QString fileName)
{    
    if(fileName.isEmpty())
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QFileDialog *fd = qobject_cast<QFileDialog*>(sender());

    FileFactory::FileVersion fver = FileFactory::Version_1_2;
    if(fd->selectedNameFilter() == "Pattern v1.0/v1.1 (*.pattern)")
        fver = FileFactory::Version_1_0;

    if(!fileName.endsWith(".pattern", Qt::CaseInsensitive)) {
        fileName += ".pattern";
    }
    
    //update the list of open files.
    if(Settings::inst()->files.contains(mFile->fileName.toLower()))
        Settings::inst()->files.remove(mFile->fileName.toLower());
    Settings::inst()->files.insert(fileName.toLower(), this);
    addToRecentFiles(fileName);

    mFile->fileName = fileName;
    mFile->save(fver);

    setApplicationTitle();
    documentIsModified(false);
    QApplication::restoreOverrideCursor();
}

void MainWindow::showFileError(int error)
{
    QApplication::restoreOverrideCursor();
    QMessageBox msgbox;
    msgbox.setText(tr("There was an error loading the file %1.").arg(mFile->fileName));
    msgbox.setIcon(QMessageBox::Critical);
    if(error == FileFactory::Err_NewerFileVersion) {
        msgbox.setInformativeText(tr("It appears to have been created with a newer version of %1.")
                                    .arg(AppInfo::inst()->appName));
    } else if ( error == FileFactory::Err_WrongFileType ) {
        msgbox.setInformativeText(tr("This file does not appear to be a %1 file.")
                                    .arg(AppInfo::inst()->appName));
    }
    msgbox.exec();
}

void MainWindow::viewFullScreen(bool state)
{
    if(state)
        showFullScreen();
    else
        showNormal();
}

void MainWindow::menuFileAboutToShow()
{
    bool state = hasTab();

    ui->actionSave->setEnabled(state);
    ui->actionSaveAs->setEnabled(state);

    ui->actionPrint->setEnabled(state);
    ui->actionPrintPreview->setEnabled(state);
    
    ui->actionExport->setEnabled(state);

}

void MainWindow::menuEditAboutToShow()
{
    bool state = hasTab();
    
    ui->actionCopy->setEnabled(state);
    ui->actionCut->setEnabled(state);
    ui->actionPaste->setEnabled(state);
	ui->actionInsertImage->setEnabled(state);
}

void MainWindow::menuViewAboutToShow()
{
    ui->actionShowStitches->setChecked(ui->allStitchesDock->isVisible());
    ui->actionShowPatternColors->setChecked(ui->patternColorsDock->isVisible());
    ui->actionShowPatternStitches->setChecked(ui->patternStitchesDock->isVisible());
	ui->actionShowLayers->setChecked(ui->layersDock->isVisible());

    ui->actionShowUndoHistory->setChecked(mUndoDock->isVisible());
    
    ui->actionShowEditModeToolbar->setChecked(ui->editModeToolBar->isVisible());
    ui->actionShowMainToolbar->setChecked(ui->mainToolBar->isVisible());
    
    ui->actionViewFullScreen->setChecked(isFullScreen());

    bool state = hasTab();
    ui->actionZoomIn->setEnabled(state);
    ui->actionZoomOut->setEnabled(state);

    ui->actionShowRowsDock->setChecked(mRowsDock->isVisible());
    ui->actionShowProperties->setChecked(mPropertiesDock->isVisible());

}

void MainWindow::menuStitchesAboutToShow()
{

    ui->actionShowAlignDock->setChecked(mAlignDock->isVisible());
    ui->actionShowMirrorDock->setChecked(mMirrorDock->isVisible());

    bool hasItems = (mPatternStitches.count() > 0 ? true : false);
    ui->actionReplaceStitch->setEnabled(hasTab() && curCrochetTab() && hasItems);

    hasItems = (mPatternColors.count() > 0 ? true : false);
    ui->actionColorReplacer->setEnabled(hasTab() && curCrochetTab() && hasItems);

    updateSelectionDependentActions();

}

void MainWindow::updateSelectionDependentActions()
{
    int selectedCount = 0;
    int selectedGroups = 0;
    int groupableCount = 0;
    QSet<unsigned int> selectedLayers;

    CrochetTab *tab = curCrochetTab();
    if(tab && tab->scene()) {
        const QList<QGraphicsItem*> selection = tab->scene()->selectedItems();
        selectedCount = selection.count();
        groupableCount = groupableSelectionCount(selection);
        selectedLayers = selectionLayers(selection);
        foreach(QGraphicsItem *item, selection) {
            if(item && item->type() == ItemGroup::Type)
                ++selectedGroups;
        }
    }

    const bool canGroupSelection = hasTab() && curCrochetTab() && selectedCount > 1
        && groupableCount > 1 && selectedLayers.count() <= 1;

    ui->actionGroup->setEnabled(canGroupSelection);
    ui->actionUngroup->setEnabled(hasTab() && curCrochetTab() && selectedGroups > 0);

    QString groupToolTip = tr("Group the selected items");
    if(selectedLayers.count() > 1)
        groupToolTip = tr("Grouping is limited to items from the same layer");
    else if(groupableCount <= 1)
        groupToolTip = tr("Select at least two movable items to create a group");
    ui->actionGroup->setToolTip(groupToolTip);
}

void MainWindow::stitchesReplaceStitch()
{

    CrochetTab *tab = curCrochetTab();
    if(!tab)
        return;

    if(mPatternStitches.count() <= 0)
        return;

    QString curStitch = Settings::inst()->value("defaultStitch").toString();

    if(curCrochetTab()) {
		//Check if the list is empty. If this is not the case, the .first() statement will crash.
		if (!curCrochetTab()->selectedItems().empty()) {
			QGraphicsItem *i = curCrochetTab()->selectedItems().first();
			if(i && i->type() == Cell::Type) {
				Cell *c = qgraphicsitem_cast<Cell*>(i);
				if(c)
					curStitch = c->name();
			}
		}
    }

    StitchReplacerUi *sr = new StitchReplacerUi(curStitch, mPatternStitches.keys(), this);

    if(sr->exec() == QDialog::Accepted) {
        if(!sr->original.isEmpty())
            tab->replaceStitches(sr->original, sr->replacement);
    }

}

void MainWindow::stitchesReplaceColor()
{
    CrochetTab *tab = curCrochetTab();
    if(!tab)
        return;

    if(mPatternColors.count() <= 0)
        return;

    ColorReplacer *cr = new ColorReplacer(mPatternColors.keys(), this);

    if(cr->exec() == QDialog::Accepted) {
        tab->replaceColor(cr->originalColor, cr->newColor, cr->selection);
    }

}

void MainWindow::fileNew()
{

    if(ui->tabWidget->count() > 0) {
        MainWindow* newWin = new MainWindow;
        newWin->move(x() + 40, y() + 40);
        newWin->show();
        newWin->ui->newDocument->show();
    } else {
        if(ui->newDocument->isVisible()) {
            QPalette pal = ui->newDocument->palette();
            mNewDocWidgetColor = ui->newDocument->palette().color(ui->newDocument->backgroundRole());
            pal.setColor(ui->newDocument->backgroundRole(), ui->newDocument->palette().highlight().color());
            ui->newDocument->setPalette(pal);
            QTimer::singleShot(1500, this, SLOT(flashNewDocDialog()));
        } else
            ui->newDocument->show();
    }
    
}

void MainWindow::flashNewDocDialog()
{
    QPalette pal = ui->newDocument->palette();
    pal.setColor(ui->newDocument->backgroundRole(), mNewDocWidgetColor);
    ui->newDocument->setPalette(pal);
}

void MainWindow::newChart()
{
    ui->newDocument->hide();

    int rows = ui->rows->text().toInt();
    int cols = ui->stitches->text().toInt();
    QString defStitch = ui->defaultStitch->currentText();
    QString name = ui->chartTitle->text();
    int incBy = ui->increaseBy->text().toInt();
    QString templateKey = currentChartTemplateKey();
    
    QString style = ui->chartStyle->currentText();

    Scene::ChartStyle st = Scene::Rows;

    if(style == tr("Blank"))
        st = Scene::Blank;
    else if(style == tr("Rounds"))
        st = Scene::Rounds;
    else
        st = Scene::Rows;

    if(docHasChartName(name))
        name = nextChartName(name);

    CrochetTab* tab = createTab(st);

    if(name.isEmpty())
            name = nextChartName();

    ui->tabWidget->addTab(tab, name);
    ui->tabWidget->setCurrentWidget(tab);

    QString ddValue = ui->rowSpacing->currentText();
    qreal rowHeight = 96;

    if(ddValue == tr("1 Chain"))
        rowHeight = 32;
    else if (ddValue == tr("2 Chains"))
        rowHeight = 64;
    else if (ddValue == tr("3 Chains"))
        rowHeight = 96;
    else if (ddValue == tr("4 Chains"))
        rowHeight = 128;
    else if (ddValue == tr("5 Chains"))
        rowHeight = 160;
    else if (ddValue == tr("6 Chains"))
        rowHeight = 182;

    tab->createChart(st, rows, cols, defStitch, QSizeF(32, rowHeight), incBy);
    applyChartTemplateToTab(tab, templateKey, rows, cols, rowHeight);

    tab->setEditFgColor(ui->fgColor->color());
    tab->setEditBgColor(ui->bgColor->color());

    updateMenuItems();

    setApplicationTitle();
    //Only mark a document as modified if we're adding another tab to it.
    if(ui->tabWidget->count() > 1)
        documentIsModified(true);
}

QString MainWindow::currentChartTemplateKey() const
{
    return ui->chartTemplate->itemData(ui->chartTemplate->currentIndex()).toString();
}

QString MainWindow::currentGrannyPresetKey() const
{
    return ui->grannyPreset->itemData(ui->grannyPreset->currentIndex()).toString();
}

QString MainWindow::currentGrannyStartTypeKey() const
{
    return ui->grannyStartType->itemData(ui->grannyStartType->currentIndex()).toString();
}

void MainWindow::setGrannyTemplateControlsVisible(bool visible)
{
    ui->grannyPresetLbl->setVisible(visible);
    ui->grannyPreset->setVisible(visible);
    ui->grannyStartLbl->setVisible(visible);
    ui->grannyStartType->setVisible(visible);
    ui->grannyCornerArchesLbl->setVisible(visible);
    ui->grannyCornerArches->setVisible(visible);
}

void MainWindow::applyChartTemplatePreset(const QString& templateKey)
{
    if(templateKey == "custom") {
        setGrannyTemplateControlsVisible(false);
        ui->chartTitle->setText(nextChartName());
        ui->rows->setValue(Settings::inst()->value("rowCount").toInt());
        ui->stitches->setValue(Settings::inst()->value("stitchCount").toInt());
        ui->increaseBy->setValue(Settings::inst()->value("increaseBy").toInt());

        const int styleIndex = ui->chartStyle->findText(Settings::inst()->value("chartStyle").toString());
        if(styleIndex >= 0)
            ui->chartStyle->setCurrentIndex(styleIndex);

        const int defaultStitchIndex = ui->defaultStitch->findText(Settings::inst()->value("defaultStitch").toString());
        if(defaultStitchIndex >= 0)
            ui->defaultStitch->setCurrentIndex(defaultStitchIndex);

        return;
    }

    if(templateKey != "granny_square")
        return;

    setGrannyTemplateControlsVisible(true);
    applyGrannyPreset(currentGrannyPresetKey());
}

void MainWindow::applyGrannyPreset(const QString& presetKey)
{
    const int roundsIndex = ui->chartStyle->findText(tr("Rounds"));
    if(roundsIndex >= 0)
        ui->chartStyle->setCurrentIndex(roundsIndex);

    const int stitchIndex = ui->defaultStitch->findText("dc");
    if(stitchIndex >= 0)
        ui->defaultStitch->setCurrentIndex(stitchIndex);

    if(presetKey == "granny_4_round") {
        ui->chartTitle->setText(tr("Granny Square 4-Round"));
        ui->rows->setValue(4);
        ui->stitches->setValue(4);
        ui->increaseBy->setValue(4);
        ui->grannyCornerArches->setValue(1);

        const int spacingIndex = ui->rowSpacing->findText(tr("3 Chains"));
        if(spacingIndex >= 0)
            ui->rowSpacing->setCurrentIndex(spacingIndex);
    } else if(presetKey == "solid_granny_square") {
        ui->chartTitle->setText(tr("Solid Granny Square"));
        ui->rows->setValue(6);
        ui->stitches->setValue(4);
        ui->increaseBy->setValue(4);
        ui->grannyCornerArches->setValue(0);

        const int spacingIndex = ui->rowSpacing->findText(tr("2 Chains"));
        if(spacingIndex >= 0)
            ui->rowSpacing->setCurrentIndex(spacingIndex);
    } else {
        ui->chartTitle->setText(tr("Granny Square"));
        ui->rows->setValue(6);
        ui->stitches->setValue(4);
        ui->increaseBy->setValue(4);
        ui->grannyCornerArches->setValue(1);

        const int spacingIndex = ui->rowSpacing->findText(tr("3 Chains"));
        if(spacingIndex >= 0)
            ui->rowSpacing->setCurrentIndex(spacingIndex);
    }
}

void MainWindow::applyChartTemplateToTab(CrochetTab* tab, const QString& templateKey,
                                         int rows, int cols, qreal rowHeight)
{
    if(!tab || templateKey != "granny_square")
        return;

    const QString grannyStartType = currentGrannyStartTypeKey();
    const int cornerArches = ui->grannyCornerArches->value();
    int guidelineColumns = qMax(4, qMax(1, cornerArches) * 4);
    if(grannyStartType == "chain_ring")
        guidelineColumns += 4;

    tab->setChartCenter(true);
    ui->actionShowChartCenter->setChecked(true);

    Guidelines guidelines;
    guidelines.setType("Rounds");
    guidelines.setRows(rows);
    guidelines.setColumns(guidelineColumns);
    guidelines.setCellHeight(static_cast<int>(rowHeight));
    guidelines.setCellWidth(32);

    QVariant value;
    value.setValue(guidelines);
    tab->propertiesUpdate("Guidelines", value);
    tab->propertiesUpdate("AlignAngle", QVariant(true));

    setSelectedGridMode("Rounds");
    updateGuidelines(guidelines);
    mPropertiesDock->propertyUpdated();
}

CrochetTab* MainWindow::createTab(Scene::ChartStyle style)
{

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    CrochetTab* tab = new CrochetTab(style, mEditMode, mStitch, mFgColor, mBgColor, ui->tabWidget);
    tab->setPatternStitches(&mPatternStitches);
    tab->setPatternColors(&mPatternColors);

    connect(tab, SIGNAL(chartStitchChanged()), SLOT(updatePatternStitches()));
    connect(tab, SIGNAL(chartColorChanged()), SLOT(updatePatternColors()));
    connect(tab, SIGNAL(chartColorChanged()), mPropertiesDock, SLOT(propertyUpdated()));
    connect(tab, SIGNAL(tabModified(bool)), SLOT(documentIsModified(bool)));
    connect(tab, SIGNAL(guidelinesUpdated(Guidelines)), SLOT(updateGuidelines(Guidelines)));
	connect(tab, SIGNAL(layersChanged(QList<ChartLayer*>&, ChartLayer*)), this, SLOT(reloadLayerContent(QList<ChartLayer*>&, ChartLayer*)));
	connect(tab->scene(), SIGNAL(sceneRectChanged(const QRectF&)), mResizeUI, SLOT(updateContent()));
	connect(tab->scene(), SIGNAL(showPropertiesSignal()), SLOT(viewMakePropertiesVisible()));
    connect(tab->scene(), SIGNAL(selectionChanged()), SLOT(updateSelectionDependentActions()));
    tab->setSnapToGrid(ui->actionSnapToGrid->isChecked());

    mUndoGroup.addStack(tab->undoStack());
    
    QApplication::restoreOverrideCursor();

    return tab;
}

void MainWindow::updateGridSnapActionState()
{
    CrochetTab* tab = curCrochetTab();
    const bool hasGrid = tab && tab->scene() && tab->scene()->guidelines().type() != "None";
    ui->actionSnapToGrid->setEnabled(hasTab() && hasGrid);
    ui->actionSnapToGrid->setToolTip(hasGrid
        ? tr("Snap moved and inserted items to the active grid")
        : tr("Snap to grid requires an active square, round, or triangle guide"));
}

QString MainWindow::nextChartName(QString baseName)
{
    QString nextName = baseName;

    int i = 1;
    
    while(docHasChartName(nextName)) {
        nextName = baseName + QString::number(i);
        i++;
    }
    
    return nextName;
}

bool MainWindow::docHasChartName(QString name)
{
    int tabCount = ui->tabWidget->count();
    for(int i = 0; i < tabCount; ++i) {
        if (ui->tabWidget->tabText(i) == name)
            return true;
    }

    return false;
}

void MainWindow::viewShowStitches()
{
    ui->allStitchesDock->setVisible(ui->actionShowStitches->isChecked());
}

void MainWindow::viewShowPatternColors()
{
    ui->patternColorsDock->setVisible(ui->actionShowPatternColors->isChecked());
}

void MainWindow::viewShowLayers()
{
	ui->layersDock->setVisible(ui->actionShowLayers->isChecked());
}

void MainWindow::viewShowPatternStitches()
{
    ui->patternStitchesDock->setVisible(ui->actionShowPatternStitches->isChecked());
}

void MainWindow::viewShowUndoHistory()
{
    mUndoDock->setVisible(ui->actionShowUndoHistory->isChecked());
}

void MainWindow::viewMakePropertiesVisible()
{
	ui->actionShowProperties->setChecked(true);
	viewShowProperties();
}

void MainWindow::viewShowProperties()
{
    mPropertiesDock->setVisible(ui->actionShowProperties->isChecked());
}

void MainWindow::viewShowResizeDock()
{
	mResizeUI->setVisible(ui->actionShowResizeDock->isChecked());
}

void MainWindow::viewShowEditModeToolbar()
{
    ui->editModeToolBar->setVisible(ui->actionShowEditModeToolbar->isChecked());
}

void MainWindow::viewShowMainToolbar()
{
    ui->mainToolBar->setVisible(ui->actionShowMainToolbar->isChecked());
}

void MainWindow::viewShowAlignDock()
{
    mAlignDock->setVisible(ui->actionShowAlignDock->isChecked());
}

void MainWindow::viewShowRowsDock()
{
    mRowsDock->setVisible(ui->actionShowRowsDock->isChecked());
}

void MainWindow::viewShowMirrorDock()
{
    mMirrorDock->setVisible(ui->actionShowMirrorDock->isChecked());
}

void MainWindow::menuModesAboutToShow()
{
    bool enabled = false;
    bool selected = false;
    bool checkedItem = false;
    bool used = false;
    
    QStringList modes;
    if(hasTab()) {
        CrochetTab* tab = qobject_cast<CrochetTab*>(ui->tabWidget->currentWidget());
        modes = tab->editModes();
    }

    foreach(QAction* a, mModeGroup->actions()) {
        if(modes.contains(a->text()))
            enabled = true;
        else
            enabled = false;

        if(mModeGroup->checkedAction() && mModeGroup->checkedAction() == a)
            checkedItem = true;
        
        if(enabled && !used && (!mModeGroup->checkedAction() || checkedItem)) {
            selected = true;
            used = true;
        }
        
        a->setEnabled(enabled);
        a->setChecked(selected);
        selected = false;
    }   
	
	bool state = hasTab();
	
	ui->actionBoxSelectMode->setEnabled(state);
	ui->actionLassoSelectMode->setEnabled(state);
	ui->actionLineSelectMode->setEnabled(state);
    updateGridSnapActionState();
}

void MainWindow::changeTabMode(QAction* a)
{
    int mode = -1;
    
    if(a == ui->actionMoveMode)
        mode = 9;
    else if(a == ui->actionStitchMode)
        mode = 10;
    else if(a == ui->actionColorMode)
        mode = 11;
    else if(a == ui->actionCreateRows)
        mode = 12;
    else if(a == ui->actionAngleMode)
        mode = 14;
    else if(a == ui->actionStretchMode)
        mode = 15;
    else if(a == ui->actionIndicatorMode)
        mode = 16;
    
    setEditMode(mode);
}

void MainWindow::setEditMode(int mode)
{
    mEditMode = mode;
    
    if(mode == 9)
        ui->actionMoveMode->setChecked(true);
    else if(mode == 10)
        ui->actionStitchMode->setChecked(true);
    else if(mode == 11)
        ui->actionColorMode->setChecked(true);
    else if(mode == 12)
        ui->actionCreateRows->setChecked(true);
    else if(mode == 14)
        ui->actionAngleMode->setChecked(true);
    else if(mode == 15)
        ui->actionStretchMode->setChecked(true);
    else if(mode == 16)
        ui->actionIndicatorMode->setChecked(true);
    
    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        CrochetTab* tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(i));
        if(tab) {
            tab->setEditMode(mEditMode);
            bool state = mode == 12 ? true : false;
            tab->showRowEditor(state);
        }
    }

    if(statusBar()) {
        QAction* checkedAction = mModeGroup->checkedAction();
        if(checkedAction)
            statusBar()->showMessage(tr("Mode: %1").arg(checkedAction->text()), 2500);
    }
}

void MainWindow::menuChartAboutToShow()
{
    bool state = hasTab();
    ui->actionRemoveTab->setEnabled(state);
    ui->actionEditName->setEnabled(state);
    ui->actionShowChartCenter->setEnabled(state);
    updateGridSnapActionState();

    CrochetTab* tab = curCrochetTab();
    if(tab) {
        ui->actionShowChartCenter->setChecked(tab->hasChartCenter());
    } else {
        ui->actionShowChartCenter->setChecked(false);
    }

}

void MainWindow::chartsShowChartCenter()
{

    CrochetTab* tab = curCrochetTab();
    if(tab) {
        tab->setChartCenter(ui->actionShowChartCenter->isChecked());
    }

}

void MainWindow::chartEditName()
{
    if(!ui->tabWidget->currentWidget())
        return;
    
    int curTab = ui->tabWidget->currentIndex();
    QString currentName  = ui->tabWidget->tabText(curTab);
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Set Chart Name"), tr("Chart name:"),
                                            QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.isEmpty()) {
        ui->tabWidget->setTabText(curTab, newName);
        if(newName != currentName)
            documentIsModified(true);
    }
}

void MainWindow::toolsCheckForUpdates()
{
    bool silent = false;
    checkUpdates(silent);
}

void MainWindow::toolsStitchLibrary()
{
    StitchLibraryUi d(this);
    d.exec();
    
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    StitchLibrary::inst()->saveAllSets();
    QApplication::restoreOverrideCursor();

    StitchLibrary::inst()->reloadAllStitchIcons();
}

void MainWindow::viewZoomIn()
{
    CrochetTab* tab = curCrochetTab();
    if(!tab)
        return;
    tab->zoomIn();
}

void MainWindow::viewZoomOut()
{
    CrochetTab* tab = curCrochetTab();
    if(!tab)
        return;
    tab->zoomOut();
}

CrochetTab* MainWindow::curCrochetTab()
{
    CrochetTab* tab = qobject_cast<CrochetTab*>(ui->tabWidget->currentWidget());
    return tab;
}

bool MainWindow::hasTab()
{
    CrochetTab* cTab = qobject_cast<CrochetTab*>(ui->tabWidget->currentWidget());
    if(!cTab)
        return false;

    return true;
}

QTabWidget* MainWindow::tabWidget()
{
    return ui->tabWidget;
}

void MainWindow::tabChanged(int newTab)
{
    if(newTab == -1) {
        updateSelectionDependentActions();
        updateGridSnapActionState();
        return;
    }

    CrochetTab* tab = qobject_cast<CrochetTab*>(ui->tabWidget->widget(newTab));
    if(!tab) {
        updateSelectionDependentActions();
        updateGridSnapActionState();
        return;
    }
    
    mUndoGroup.setActiveStack(tab->undoStack());
    setSelectedGridMode(tab->scene()->guidelines().type());
    ui->actionSnapToGrid->blockSignals(true);
    ui->actionSnapToGrid->setChecked(tab->snapToGrid());
    ui->actionSnapToGrid->blockSignals(false);
    updateGridSnapActionState();
    updateSelectionDependentActions();
}

void MainWindow::removeCurrentTab()
{
    removeTab(ui->tabWidget->currentIndex());
}

void MainWindow::removeTab(int tabIndex)
{
    if(tabIndex < 0)
        return;

    QMessageBox msgbox;
    
    if(ui->tabWidget->count() == 1) {
        msgbox.setText(tr("A document must have at least one (1) chart."));
        msgbox.setIcon(QMessageBox::Information);
        msgbox.exec();
        return;
    }
    
    msgbox.setWindowTitle(tr("Delete Chart"));
    msgbox.setText(tr("Are you sure you want to delete this chart from the document?"));
    msgbox.setInformativeText(tr("Deleting a chart from the document is a permanent procedure."));
    msgbox.setIcon(QMessageBox::Question);
    /*QPushButton* removeChart =*/ msgbox.addButton(tr("Delete the chart"), QMessageBox::AcceptRole);
    QPushButton* keepChart = msgbox.addButton(tr("Keep the chart"), QMessageBox::RejectRole);

    msgbox.exec();
    
    if(msgbox.clickedButton() == keepChart)
        return;
    
    //FIXME: Make removing a tab undo-able, using a *tab and chart name.
    ui->tabWidget->removeTab(tabIndex);

    documentIsModified(true);
    
    //update the title and menus
    setApplicationTitle();
    updateMenuItems();
    updateSelectionDependentActions();
}

void MainWindow::addLayer()
{
	QString name = ui->layerName->text();
	CrochetTab* tab = curCrochetTab();
	if (tab != NULL)
		tab->addLayer(name);
}

void MainWindow::removeLayer()
{
	CrochetTab* tab = curCrochetTab();
	if (tab != NULL)
		tab->removeSelectedLayer();
}

void MainWindow::mergeLayer()
{
	QTreeView* view = ui->layersView;
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(view->model());
	
	if (model == NULL)
		return;
	
	//first we get the currently selected item
	QStandardItem* itemfrom = model->itemFromIndex(view->selectionModel()->currentIndex());
	ChartLayer* from = static_cast<ChartLayer*>(itemfrom->data(Qt::UserRole+5).value<void*>());
	
	//and then we fetch the next item
    QStandardItem* itemto = model->item(view->selectionModel()->currentIndex().row() + 1, 0);
	if (itemto == NULL)
		return;
	ChartLayer* to = static_cast<ChartLayer*>(itemto->data(Qt::UserRole+5).value<void*>());
	
	//and call the function
	CrochetTab* tab = curCrochetTab();
	if (tab != NULL)
		tab->mergeLayer(from->uid(), to->uid());
}

void MainWindow::selectLayer(const QModelIndex& index)
{
	QTreeView* view = ui->layersView;
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(view->model());
	
	if (model == NULL)
		return;
	
	QStandardItem* item = model->itemFromIndex(index);
	ChartLayer* layer = static_cast<ChartLayer*>(item->data(Qt::UserRole+5).value<void*>());
		
	CrochetTab* tab = curCrochetTab();
	if (tab != NULL) {
		tab->selectLayer(layer->uid());
        updateSelectionDependentActions();
        if(statusBar())
            statusBar()->showMessage(tr("Active layer: %1").arg(layer->name()), 2500);
    }
}

void MainWindow::layerModelChanged(const QModelIndex& index)
{
	QTreeView* view = ui->layersView;
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(view->model());
	
	if (model == NULL)
		return;
	
	QStandardItem* item = model->itemFromIndex(index);
	
	//scrape the data of the item
	ChartLayer* layer = static_cast<ChartLayer*>(item->data(Qt::UserRole+5).value<void*>());
	QString name = item->text();
	bool checked = item->checkState() == Qt::Checked;//item->data(Qt::CheckStateRole).toBool();
	
	layer->setName(name);
	layer->setVisible(checked);
	
    CrochetTab* tab = curCrochetTab();
    if(!tab)
        return;

	tab->editedLayer(layer);
    updateSelectionDependentActions();
    if(statusBar()) {
        const QString visibility = checked ? tr("visible") : tr("hidden");
        statusBar()->showMessage(tr("Layer %1 is now %2").arg(layer->name(), visibility), 2500);
    }
}

void MainWindow::updatePatternStitches()
{
    if(ui->tabWidget->count() <= 0)
        return;

    //FIXME: this whole thing needs to be worked out, but the very least is make this use a shared icon.
    ui->patternStitches->clear();
    QMapIterator<QString, int> i(mPatternStitches);
    while (i.hasNext()) {
        i.next();
        QList<QListWidgetItem*> items = ui->patternStitches->findItems(i.key(), Qt::MatchExactly);
        if(items.count() == 0) {
            Stitch* s = StitchLibrary::inst()->findStitch(i.key(), true);
            QPixmap pix = QPixmap(QSize(32, 32));

            pix.load(s->file());
            QIcon icon = QIcon(pix);
            QListWidgetItem* item = new QListWidgetItem(icon, i.key(), ui->patternStitches);
            ui->patternStitches->addItem(item);
        }
    }
}

void MainWindow::updatePatternColors()
{
    if(ui->tabWidget->count() <= 0)
        return;

    ui->patternColors->clear();

    QString prefix = Settings::inst()->value("colorPrefix").toString();

    QStringList keys = mPatternColors.keys();
    QMap<qint64, QString> sortedColors;
    
    foreach(QString key, keys) {
        qint64 added = mPatternColors.value(key).value("added");
        sortedColors.insert(added, key);
    }

    int i = 1;
    QList<qint64> sortedKeys = sortedColors.keys();
    foreach(qint64 sortedKey, sortedKeys) {
        QString color = sortedColors.value(sortedKey);
        QList<QListWidgetItem*> items = ui->patternColors->findItems(color, Qt::MatchExactly);
        if(items.count() == 0) {
            QPixmap pix = ColorListWidget::drawColorBox(color, QSize(32, 32));
            QIcon icon = QIcon(pix);
            
            QListWidgetItem* item = new QListWidgetItem(icon, prefix + QString::number(i), ui->patternColors);
            item->setToolTip(color);
            item->setData(Qt::UserRole, QVariant(color));
            ui->patternColors->addItem(item);
            ++i;
        }
    }
}

void MainWindow::documentIsModified(bool isModified)
{
#ifdef Q_OS_MAC
    QString curFile = mFile->fileName;

    if (!curFile.isEmpty()) {
        if (!isModified) {
            setWindowIcon(fileIcon);
        } else {
            static QIcon darkIcon;

            if (darkIcon.isNull())
                darkIcon = QIcon(":/images/stitchworks-pattern-dark.svg");
            setWindowIcon(darkIcon);
        }
    }
#endif
    setWindowModified(isModified);
}

void MainWindow::chartCreateRows(bool state)
{
    CrochetTab* tab = requireCurrentTab(tr("Row editor"));
    if(tab) tab->showRowEditor(state);
}

void MainWindow::alignSelection(int style)
{
    CrochetTab* tab = requireCurrentTab(tr("Align selection"));
    if(tab) tab->alignSelection(style);
}

void MainWindow::distributeSelection(int style)
{
    CrochetTab* tab = requireCurrentTab(tr("Distribute selection"));
    if(tab) tab->distributeSelection(style);
}

void MainWindow::arrangeGrid(QSize grid, QSize alignment, QSize spacing, bool useSelection)
{
    CrochetTab* tab = requireCurrentTab(tr("Arrange"));
    if(tab) tab->arrangeGrid(grid, alignment, spacing, useSelection);
}

void MainWindow::copy(int direction)
{
    CrochetTab* tab = requireCurrentTab(tr("Directional copy"));
    if(tab) tab->copy(direction);
}

void MainWindow::mirror(int direction)
{
    CrochetTab* tab = requireCurrentTab(tr("Mirror"));
    if(tab) tab->mirror(direction);
}

void MainWindow::rotate(qreal degrees)
{
    CrochetTab* tab = requireCurrentTab(tr("Rotate"));
    if(tab) tab->rotate(degrees);
}

void MainWindow::resize(QRectF scenerect)
{
    CrochetTab* tab = requireCurrentTab(tr("Resize chart"));
    if(tab) tab->resizeScene(scenerect);
}

void MainWindow::updateGuidelines(Guidelines guidelines)
{
    mPropertiesDock->loadProperties(guidelines);
}

void MainWindow::copy()
{
    CrochetTab* tab = requireCurrentTab(tr("Copy"));
    if(tab) tab->copy();
}

void MainWindow::cut()
{
    CrochetTab* tab = requireCurrentTab(tr("Cut"));
    if(tab) tab->cut();
}

void MainWindow::paste()
{
    CrochetTab* tab = requireCurrentTab(tr("Paste"));
    if(tab) tab->paste();
}

void MainWindow::insertImage()
{
	//first choose the image we want to add
	QString file = QFileDialog::getOpenFileName(this, tr("Open Image"),
		QString(), tr("Image Files (*.png *.jpg *.bmp *.tga *.gif)"));
    if(file.isEmpty())
        return;
		
	CrochetTab* tab = requireCurrentTab(tr("Insert image"));
	
	//get the location of the current center of the scene
	if (!tab)
		return;
		
	ChartView* view = tab->view();
	
	if (!view)
		return;
		
	QPointF pos = view->sceneRect().center();
	
    tab->insertImage(file, pos);
}

void MainWindow::group()
{
    CrochetTab* tab = requireCurrentTab(tr("Group"));
    if(!tab)
        return;

    const QList<QGraphicsItem*> selection = tab->scene()->selectedItems();
    if(selection.count() <= 1) {
        notifyUnavailableAction(tr("Group"), tr("select at least two items"));
        return;
    }
    if(groupableSelectionCount(selection) <= 1) {
        notifyUnavailableAction(tr("Group"), tr("select at least two non-center items"));
        return;
    }
    if(selectionLayers(selection).count() > 1) {
        notifyUnavailableAction(tr("Group"), tr("selected items are on different layers"));
        return;
    }

    tab->group();
}

void MainWindow::ungroup()
{
    CrochetTab* tab = requireCurrentTab(tr("Ungroup"));
    if(!tab)
        return;

    const QList<QGraphicsItem*> selection = tab->scene()->selectedItems();
    if(selection.isEmpty()) {
        notifyUnavailableAction(tr("Ungroup"), tr("nothing is selected"));
        return;
    }
    if(selectedGroupCount(selection) <= 0) {
        notifyUnavailableAction(tr("Ungroup"), tr("selection does not contain a group"));
        return;
    }

    tab->ungroup();

}

void MainWindow::notifyUnavailableAction(const QString& action, const QString& reason)
{
    const QString message = tr("%1 is unavailable: %2").arg(action, reason);
    qWarning() << message;
    if(statusBar())
        statusBar()->showMessage(message, 4000);
}

CrochetTab* MainWindow::requireCurrentTab(const QString& action)
{
    CrochetTab* tab = curCrochetTab();
    if(!tab)
        notifyUnavailableAction(action, tr("no chart tab is open"));
    return tab;
}
