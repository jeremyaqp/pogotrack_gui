#include "MainWindow.h"
#include <QHBoxLayout>
#include <QPixmap>
#include <QImage>
#include <QFileDialog>
#include <QLineEdit>
#include <QCursor>
#include <QMessageBox>
#include <QApplication>
#include <QGroupBox>
#include <QScrollArea>
#include <QSplitter>
#include "collapsible/Section.h"

using namespace ui;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::_setupUI()
{
    setWindowTitle("PogoTrack GUI");
    setWindowIcon(QIcon(":/assets/icon.png"));
    // ---- Elements definition ----
    QWidget *centralWidget = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    _display = new ImageDisplay;
    _display->setMinimumSize(320, 240);

    _sidePanel = new QWidget;
    _sideLayout = new QVBoxLayout;
    QScrollArea* scrollArea     = new QScrollArea();

    // Helper function for thresholds
    auto configThreshold = [&](QSlider* sl){
        sl->setOrientation(Qt::Horizontal);
        sl->setRange(0, 255);
        sl->setValue(0);
        sl->setSingleStep(1.0);
    };

    // ################################################################# Import
    QLabel *importLabel = new QLabel("Import");
    importLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    _sideLayout->addWidget(importLabel);

    QPushButton *browse = new QPushButton("Open test image");
    _sideLayout->addWidget(browse);
    _sideLayout->addSpacing(8);   // Space after category

    // ################################################################# Tools
    QLabel *maskLabel = new QLabel("Measuring Tools");
    maskLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    _sideLayout->addWidget(maskLabel);

    QButtonGroup *toolGroup     = new QButtonGroup(this);
    QRadioButton *lineToolBtn   = new QRadioButton("Line");
    QRadioButton *rectToolBtn   = new QRadioButton("Rectangle");
    QRadioButton *circToolBtn   = new QRadioButton("Circle");
    toolGroup->addButton(lineToolBtn);
    toolGroup->addButton(rectToolBtn);
    toolGroup->addButton(circToolBtn);
    lineToolBtn->setChecked(true); // default
    toolGroup->setExclusive(true);

    _sideLayout->addWidget(lineToolBtn);
    _sideLayout->addWidget(rectToolBtn);
    _sideLayout->addWidget(circToolBtn);

    QPushButton *applyMaskBtn = new QPushButton("Apply Mask");
    _sideLayout->addWidget(applyMaskBtn);
    _sideLayout->addSpacing(8);

    // ################################################################# Operations
    QLabel *operationLabel = new QLabel("Operations");
    operationLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    _sideLayout->addWidget(operationLabel);

    // ---------------------------------------- Bin Threshold
    _threshValueLabel = new QLabel("Binary Threshold : 0");
    _sideLayout->addWidget(_threshValueLabel);

    _binThreshold = new QSlider(this);
    _sideLayout->addWidget(_binThreshold);
    configThreshold(_binThreshold);

    // ---------------------------------------- Connected Components

    QPushButton *ccBtn = new QPushButton("Connected Components");
    _sideLayout->addWidget(ccBtn);

    // ---------------------------------------- Hough Circles
    QVBoxLayout *houghVbox = new QVBoxLayout;
    QPushButton *houghBtn  = new QPushButton("Hough Circles");
    houghVbox->addWidget(houghBtn);
    // Helper lambda to add a label and input on the same line
    auto addLabelAndInputHough = [&](const QString &text, QLineEdit *edit) {
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(new QLabel(text));
        hLayout->addWidget(edit);
        houghVbox->addLayout(hLayout);
    };

    Section* HoughSection = new Section("Hough Circles", 300, this);

    _HoughDpEdit                = new QLineEdit("1.0");
    _HoughMinDistEdit           = new QLineEdit("20.0");
    _HoughParam1Edit            = new QLineEdit("10");
    _HoughParam2Edit            = new QLineEdit("14");
    _HoughMinRadiusEdit         = new QLineEdit("40");
    _HoughMaxRadiusEdit         = new QLineEdit("60");

    addLabelAndInputHough("dp:", _HoughDpEdit);
    addLabelAndInputHough("minDist:", _HoughMinDistEdit);
    addLabelAndInputHough("param1:", _HoughParam1Edit);
    addLabelAndInputHough("param2:", _HoughParam2Edit);
    addLabelAndInputHough("minRadius:", _HoughMinRadiusEdit);
    addLabelAndInputHough("maxRadius:", _HoughMaxRadiusEdit);
    HoughSection->setContentLayout(*houghVbox);
    _sideLayout->addWidget(HoughSection);

    // ---------------------------------------- Adaptive Threshold
    QVBoxLayout *adaptativeVBox = new QVBoxLayout;
    QPushButton *adaptBtn       = new QPushButton("Adaptive Threshold");
    adaptativeVBox->addWidget(adaptBtn);
    auto addLabelAndInputAdaptative = [&](const QString &text, QLineEdit *edit) {
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(new QLabel(text));
        hLayout->addWidget(edit);
        adaptativeVBox->addLayout(hLayout);
    };
    Section* AdaptiveSection = new Section("Adaptive Threshold", 300, this);

    _AdaptiveMeanCBtn           = new QRadioButton("Mean C");
    _AdaptiveGaussianCBtn       = new QRadioButton("Gaussian C");
    _AdaptiveCEdit              = new QLineEdit("-10.0");
    _AdaptiveBlocksizeEdit      = new QLineEdit("11");

    QButtonGroup *adaptMethodGroup = new QButtonGroup(this);
    adaptMethodGroup->addButton(_AdaptiveMeanCBtn);
    adaptMethodGroup->addButton(_AdaptiveGaussianCBtn);
    adaptMethodGroup->setExclusive(true);
    adaptativeVBox->addWidget(_AdaptiveMeanCBtn);
    adaptativeVBox->addWidget(_AdaptiveGaussianCBtn);
    _AdaptiveMeanCBtn->setChecked(true); // default
    addLabelAndInputAdaptative("C:", _AdaptiveCEdit);
    addLabelAndInputAdaptative("Block size:", _AdaptiveBlocksizeEdit);
    AdaptiveSection->setContentLayout(*adaptativeVBox);
    _sideLayout->addWidget(AdaptiveSection);


    // ---------------------------------------- RGB Thresholds
    QVBoxLayout *spThreshVBox = new QVBoxLayout;
    Section* spThreshSection = new Section("RGB Thresholds", 300, this);
    spThreshSection->setToolTip("Apply TOZERO or TOZERO_INV separately on each channel");

    _threshValueLabel_R    = new QLabel("Red Threshold : 0");
    _threshValueLabel_G    = new QLabel("Green Threshold : 0");
    _threshValueLabel_B    = new QLabel("Blue Threshold : 0");
    _binThreshold_R        = new QSlider(this);
    _binThreshold_G        = new QSlider(this);
    _binThreshold_B        = new QSlider(this);
    QHBoxLayout * hGroup_R = new QHBoxLayout();
    QHBoxLayout * hGroup_G = new QHBoxLayout();
    QHBoxLayout * hGroup_B = new QHBoxLayout();

    _invertThresholdR      = new QCheckBox("Invert");
    _invertThresholdG      = new QCheckBox("Invert");
    _invertThresholdB      = new QCheckBox("Invert");

    spThreshVBox->addWidget(_threshValueLabel_R);
    hGroup_R->addWidget(_binThreshold_R);
    hGroup_R->addWidget(_invertThresholdR);
    spThreshVBox->addLayout(hGroup_R);
    spThreshVBox->addWidget(_threshValueLabel_G);
    hGroup_G->addWidget(_binThreshold_G);
    hGroup_G->addWidget(_invertThresholdG);
    spThreshVBox->addLayout(hGroup_G);
    spThreshVBox->addWidget(_threshValueLabel_B);
    hGroup_B->addWidget(_binThreshold_B);
    hGroup_B->addWidget(_invertThresholdB);
    spThreshVBox->addLayout(hGroup_B);

    configThreshold(_binThreshold_R);
    configThreshold(_binThreshold_G);
    configThreshold(_binThreshold_B);

    spThreshSection->setContentLayout(*spThreshVBox);
    _sideLayout->addWidget(spThreshSection);


    // ---------------------------------------- Background Removal
    QVBoxLayout *bgRemoveVBox = new QVBoxLayout;
    Section* bgRemoveSection  = new Section("Background Removal", 300, this);
    _bgRemovalSlider          = new QSlider(this);
    QPushButton *browseBg     = new QPushButton("Open background image");

    bgRemoveVBox->addWidget(_bgRemovalSlider);
    bgRemoveVBox->addWidget(browseBg);

    _bgRemovalSlider->setOrientation(Qt::Horizontal);
    _bgRemovalSlider->setRange(0,100); // 0 to 100 for percentage
    _bgRemovalSlider->setValue(0);
    _bgRemovalSlider->setSingleStep(1.0);

    bgRemoveSection->setContentLayout(*bgRemoveVBox);
    _sideLayout->addWidget(bgRemoveSection);
    _bgRemovalSlider->setVisible(false);

    // ----------------------------------------  Reset

    QPushButton *resetBtn       = new QPushButton("Reset");
    _sideLayout->addWidget(resetBtn);

    // ################################################################# License
    QWidget *bottomRightBox = new QWidget;
    QVBoxLayout *brLayout = new QVBoxLayout(bottomRightBox);
    brLayout->setContentsMargins(0,0,0,0);
    brLayout->setSpacing(0);

    QLabel *teamLabel = new QLabel("Pogoteam 2025");
    QLabel *licenceLabel = new QLabel();
    QPixmap licencePixmap(":/assets/by-nc.png");
    teamLabel->setStyleSheet("font-size: 14px;");
    licenceLabel->setPixmap(licencePixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    brLayout->addWidget(teamLabel);
    brLayout->addWidget(licenceLabel);
    brLayout->addStretch();  // push them up inside the box if needed

    _sideLayout->addStretch();  // push everything up
    _sideLayout->addWidget(bottomRightBox, 0, Qt::AlignLeft | Qt::AlignBottom);

    // ################################################################# General layout
    _sidePanel->setLayout(_sideLayout);
    scrollArea->setWidget(_sidePanel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumWidth(200);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(_display);
    splitter->addWidget(scrollArea);

    mainLayout->addWidget(splitter, 1);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // ################################################################# Connections
    connect(browse, &QPushButton::clicked, this, &MainWindow::_loadImage);
    connect(_binThreshold, &QSlider::valueChanged, this, &MainWindow::applyThreshold);
    connect(_binThreshold, &QSlider::sliderReleased, this, &MainWindow::validateThreshold);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetImage);
    connect(ccBtn, &QPushButton::clicked, this, &MainWindow::connectedComponentsMode);
    connect(applyMaskBtn, &QPushButton::clicked, this, &MainWindow::applyMask);

    connect(toolGroup, &QButtonGroup::idClicked, this, [=](int id) {
        if (toolGroup->button(id) == lineToolBtn) {
            _display->leftClicTool = DRAW_LINE;
        }
        else if (toolGroup->button(id) == rectToolBtn) {
            _display->leftClicTool = DRAW_RECT;
        } else if (toolGroup->button(id) == circToolBtn) {
            _display->leftClicTool = DRAW_CIRCLE;
        }
    });
    connect(houghBtn, &QPushButton::clicked, this, &MainWindow::applyHoughCircles);
    connect(_HoughDpEdit, &QLineEdit::editingFinished, this, &MainWindow::getHoughParams);
    connect(_HoughMinDistEdit, &QLineEdit::editingFinished, this, &MainWindow::getHoughParams);
    connect(_HoughParam1Edit, &QLineEdit::editingFinished, this, &MainWindow::getHoughParams);
    connect(_HoughParam2Edit, &QLineEdit::editingFinished, this, &MainWindow::getHoughParams);
    connect(_HoughMinRadiusEdit, &QLineEdit::editingFinished, this, &MainWindow::getHoughParams);
    connect(_HoughMaxRadiusEdit, &QLineEdit::editingFinished, this, &MainWindow::getHoughParams);

    connect(adaptBtn, &QPushButton::clicked, this, &MainWindow::applyAdaptativeThreshold);
    connect(adaptMethodGroup, &QButtonGroup::idClicked, this, [=](int id) {
        if (adaptMethodGroup->button(id) == _AdaptiveMeanCBtn ) {
            _adaptParams.method = cv::ADAPTIVE_THRESH_MEAN_C;
        } else {
            _adaptParams.method = cv::ADAPTIVE_THRESH_GAUSSIAN_C;
        }
    });

    connect(_binThreshold_R, &QSlider::valueChanged, this, &MainWindow::applySpecialThreshold);
    connect(_binThreshold_R, &QSlider::sliderReleased, this, &MainWindow::validateThreshold);
    connect(_binThreshold_G, &QSlider::valueChanged, this, &MainWindow::applySpecialThreshold);
    connect(_binThreshold_G, &QSlider::sliderReleased, this, &MainWindow::validateThreshold);
    connect(_binThreshold_B, &QSlider::valueChanged, this, &MainWindow::applySpecialThreshold);
    connect(_binThreshold_B, &QSlider::sliderReleased, this, &MainWindow::validateThreshold);
    connect(_invertThresholdR, &QCheckBox::stateChanged, this, [=]() {
        applySpecialThreshold();
        validateThreshold();
    });
    connect(_invertThresholdG, &QCheckBox::stateChanged, this, [=]() {
        applySpecialThreshold();
        validateThreshold();
    });
    connect(_invertThresholdB, &QCheckBox::stateChanged, this, [=]() {
        applySpecialThreshold();
        validateThreshold();
    });

    // ################################################################# Shortcuts
    QShortcut *undoShortcut = new QShortcut(QKeySequence(QKeySequence::Undo), this);
    connect(undoShortcut, &QShortcut::activated, this, [=]() {
        if (_stackIndex > 0) {
            _stackIndex--;
            _currentImage = _displayedImageStack[_stackIndex];
            _currentOverlays = _overlayStack[_stackIndex];
            _displayImage(false);
        }
    });
    QShortcut *redoShortcut = new QShortcut(QKeySequence(QKeySequence::Redo), this);
    connect(redoShortcut, &QShortcut::activated, this, [=]() {
        if (_stackIndex + 1 < static_cast<int>(_displayedImageStack.size())) {
            _stackIndex++;
            _currentImage = _displayedImageStack[_stackIndex];
            _currentOverlays = _overlayStack[_stackIndex];
            _displayImage(false);
        }
    });

    connect(browseBg, &QPushButton::clicked, this, &MainWindow::_loadBackgroundImage);
    connect(_bgRemovalSlider, &QSlider::valueChanged, this, &MainWindow::removeBackground);
    connect(_bgRemovalSlider,  &QSlider::sliderReleased, this, &MainWindow::validateThreshold);

    _displayImage(false);
}

void MainWindow::_loadImage()
{
    QString path =
    QFileDialog::getOpenFileName(this, "Open a file", ".",
        "Images (*.png *.bmp *.jpg);");

    _originalImage = cv::imread(path.toStdString());
    if (_originalImage.empty())
        _originalImage = cv::Mat::zeros(480, 640, CV_8UC3);

    _currentImage = _originalImage.clone();
    _displayImage();
}

void MainWindow::_loadBackgroundImage()
{
    QString path =
    QFileDialog::getOpenFileName(this, "Open a file", ".",
        "Images (*.png *.bmp *.jpg);");

    _bgImage = cv::imread(path.toStdString());
    if (!_bgImage.empty())
        _bgRemovalSlider->setVisible(true);

    if(_bgImage.size() != _originalImage.size()){
        QMessageBox::warning(this, "Size mismatch", "Background image size does not match original image size. Background removal will not work.");
        _bgImage = cv::Mat();
        _bgRemovalSlider->setVisible(false);
    }
    _sidePanel->updateGeometry();

    _displayImage();
}


void MainWindow::_displayImage(bool addToStack)
{
    _displayImage(_currentImage, addToStack);
}


void MainWindow::_displayImage(cv::Mat img, bool addToStack)
{
    if(img.empty()) return;
    cv::Mat rgb;
    if (img.channels() == 1)
        cv::cvtColor(img, rgb, cv::COLOR_GRAY2RGB);
    else
        cv::cvtColor(img, rgb, cv::COLOR_BGR2RGB);

    if(_currentOverlays & CONNECTED_COMPONENTS)
        _display->showConnectedComponents(_ccstats, _cccentroids);
    else
        _display->hideConnectedComponents();
    if(_currentOverlays & HOUGH_CIRCLES)
        _display->showHoughCircles(_HoughCircles);
    else
        _display->hideHoughCircles();

    _display->setImage(img);
    if(!addToStack) return;
    _stackIndex++;
    // Store in stack
    if(_stackIndex < static_cast<int>(_displayedImageStack.size())){
        _displayedImageStack[_stackIndex] = img.clone();
        _overlayStack[_stackIndex] = _currentOverlays;
        // Remove any redo history
        _displayedImageStack.resize(_stackIndex + 1);
        _overlayStack.resize(_stackIndex + 1);
        printf("Resized stack to %zu\n", _displayedImageStack.size());
    } else {
        _displayedImageStack.push_back(img.clone());
        _overlayStack.push_back(_currentOverlays);
    }
}


void MainWindow::applyThreshold()
{
    if(_currentImage.empty()) return;
    // convert to grayscale, single channel with max over rgb
    // 1) Max-channel grayscale
    cv::Mat ch[3];
    cv::split(_originalImage, ch);        // B, G, R
    cv::Mat maxGray;
    cv::max(ch[0], ch[1], maxGray);
    cv::max(maxGray, ch[2], maxGray);

    // 2) Apply threshold
    double thres = _binThreshold->value();
    cv::threshold(maxGray, _currentImage, thres, 255, cv::THRESH_BINARY);
    cv::cvtColor(_currentImage, _currentImage, cv::COLOR_GRAY2BGR); // convert back to 3 channels for consistency with other operations
    if(!_currentMask.empty()){
        cv::Mat maskedImage;
        _currentImage.copyTo(maskedImage, _currentMask);
        _currentImage = maskedImage;
    }
    _currentOverlays = 0; // reset overlays
    // 3) Display
    _displayImage(false);
    _threshValueLabel->setText("Binary Threshold : " + QString::number((int)thres));
}


void MainWindow::removeBackground()
{
    if(_bgImage.empty()) return;
    if(_originalImage.empty()) return;
    cv::Mat result;
    cv::addWeighted(_bgImage, 1.0, _originalImage, -(double)_bgRemovalSlider->value() / 100.0, 0.0, result);
    
    _currentImage = result;
    _displayImage(false);
}


void MainWindow::applySpecialThreshold()
{
    if(_currentImage.empty()) return;
    if(_originalImage.empty()) return;

    if (_originalImage.channels() != 3) {
        qDebug() << "Image is not 3-channel!";
        return;
    }
    // convert to grayscale, single channel with max over rgb
    // 1) Max-channel grayscale
    cv::Mat ch[3];
    cv::split(_originalImage, ch);        // B, G, R

    double thres_R = _binThreshold_R->value();
    cv::threshold(ch[2], ch[2], thres_R, 255, 
        _invertThresholdR->isChecked() ? cv::THRESH_TOZERO_INV : cv::THRESH_TOZERO);
    double thres_G = _binThreshold_G->value();
    cv::threshold(ch[1], ch[1], thres_G, 255, 
        _invertThresholdG->isChecked() ? cv::THRESH_TOZERO_INV : cv::THRESH_TOZERO);
    double thres_B = _binThreshold_B->value();
    cv::threshold(ch[0], ch[0], thres_B, 255, 
        _invertThresholdB->isChecked() ? cv::THRESH_TOZERO_INV : cv::THRESH_TOZERO);
    
    cv::merge(ch, 3, _currentImage); // merge back to 3 channels
    if(!_currentMask.empty()){
        cv::Mat maskedImage;
        _currentImage.copyTo(maskedImage, _currentMask);
        _currentImage = maskedImage;
    }
    _currentOverlays = 0; // reset overlays
    // 3) Display
    _displayImage(false);
    _threshValueLabel_R->setText("Red Threshold : " + QString::number((int)thres_R));
    _threshValueLabel_G->setText("Green Threshold : " + QString::number((int)thres_G));
    _threshValueLabel_B->setText("Blue Threshold : " + QString::number((int)thres_B));
}


void MainWindow::validateThreshold()
{
    _displayImage(true);
}

void MainWindow::resetImage()
{
    _currentImage = _originalImage.clone();
    _currentMask = cv::Mat();
    _bgImage = cv::Mat();
    _currentOverlays = 0;
    _stackIndex = -1;
    _displayedImageStack.clear();
    _overlayStack.clear();
    _HoughDpEdit->setText(QString::number(DEFAULT_HOUGH_DP));
    _HoughMinDistEdit->setText(QString::number(DEFAULT_HOUGH_MIN_DIST));
    _HoughParam1Edit->setText(QString::number(DEFAULT_HOUGH_PARAM1));
    _HoughParam2Edit->setText(QString::number(DEFAULT_HOUGH_PARAM2));
    _HoughMinRadiusEdit->setText(QString::number(DEFAULT_HOUGH_MIN_RADIUS));
    _HoughMaxRadiusEdit->setText(QString::number(DEFAULT_HOUGH_MAX_RADIUS));
    DEFAULT_ADAPT_METHOD == cv::ADAPTIVE_THRESH_MEAN_C ? _AdaptiveMeanCBtn->setChecked(true) : _AdaptiveGaussianCBtn->setChecked(true);
    _AdaptiveCEdit->setText(QString::number(DEFAULT_ADAPT_C));
    _AdaptiveBlocksizeEdit->setText(QString::number(DEFAULT_ADAPT_BLOCK_SIZE));

    _binThreshold->setValue(0);
    _binThreshold_R->setValue(0);
    _binThreshold_G->setValue(0);
    _binThreshold_B->setValue(0);
    _invertThresholdR->setChecked(false);
    _invertThresholdG->setChecked(false);
    _invertThresholdB->setChecked(false);

    _threshValueLabel->setText("Binary Threshold : 0");
    _threshValueLabel_R->setText("Red Threshold : 0");
    _threshValueLabel_G->setText("Green Threshold : 0");
    _threshValueLabel_B->setText("Blue Threshold : 0");

    _bgRemovalSlider->setVisible(false);
    _sidePanel->updateGeometry();

    _displayImage();
}

void MainWindow::connectedComponentsMode()
{
    if(_currentImage.empty()) return;
    cv::Mat gray;
    if (_currentImage.channels() == 3)
        cv::cvtColor(_currentImage, gray, cv::COLOR_BGR2GRAY);
    else
        gray = _currentImage.clone();

    // Ensure binary image (threshold if needed)
    cv::Mat binImg;
    cv::threshold(gray, binImg, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // Connected components
    cv::Mat labels, stats, centroids;
    int n = cv::connectedComponentsWithStats(binImg, labels, stats, centroids);

    // Create a color output where each component has a different color, for display purpose
    cv::Mat coloredLabels = cv::Mat::zeros(labels.size(), CV_8UC3);
    cv::RNG rng(12345);

    std::vector<cv::Vec3b> colors(n);
    colors[0] = cv::Vec3b(0,0,0); // background
    for (int i = 1; i < n; i++)
        colors[i] = cv::Vec3b(rng.uniform(50,255), rng.uniform(50,255), rng.uniform(50,255));

    for (int y = 0; y < labels.rows; y++)
        for (int x = 0; x < labels.cols; x++)
            coloredLabels.at<cv::Vec3b>(y,x) = colors[ labels.at<int>(y,x) ];

    _currentOverlays |= CONNECTED_COMPONENTS;
    _ccstats = stats;
    _cccentroids = centroids;
    // Show the updated colored image
    _displayImage(coloredLabels);
}

void MainWindow::applyMask()
{
    if(_currentImage.empty()) return;

    _currentMask = _display->getMaskFromTool();
    if(_currentMask.empty()) return;

    // Apply mask to current image
    cv::Mat maskedImage;
    _currentImage.copyTo(maskedImage, _currentMask);

    _currentImage = maskedImage;
    _displayImage();
}

void MainWindow::getHoughParams()
{
    _params.dp        = _HoughDpEdit->text().toDouble();
    _params.minDist   = _HoughMinDistEdit->text().toDouble();
    _params.param1    = _HoughParam1Edit->text().toDouble();
    _params.param2    = _HoughParam2Edit->text().toDouble();
    _params.minRadius = _HoughMinRadiusEdit->text().toInt();
    _params.maxRadius = _HoughMaxRadiusEdit->text().toInt();
}

void MainWindow::applyHoughCircles()
{
    if(_currentImage.empty()) return;

    // Convert to grayscale
    cv::Mat gray;
    if (_currentImage.channels() == 3){
        // 1) Max-channel grayscale
        cv::Mat ch[3];
        cv::split(_currentImage, ch);
        cv::max(ch[0], ch[1], gray);
        cv::max(gray, ch[2], gray);
    } else {
        gray = _currentImage.clone();
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Apply Hough Circle Transform
    try {
    cv::HoughCircles(gray, _HoughCircles, cv::HOUGH_GRADIENT,
                     _params.dp, _params.minDist,
                     _params.param1, _params.param2,
                     _params.minRadius, _params.maxRadius);
    } catch (const cv::Exception &e) {
        QMessageBox::critical(this, "Hough Circles Error",
                              QString("Error: %1").arg(e.what()));
        QApplication::restoreOverrideCursor();
        return;
    }
    int numCircles = static_cast<int>(_HoughCircles.size());
    QApplication::restoreOverrideCursor();

    QMessageBox::information(this, "Hough Circles Result",
                             QString("Found %1 circles").arg(numCircles));

    _currentOverlays |= HOUGH_CIRCLES;
    // Display the updated image with circles
    _displayImage();
}

void MainWindow::applyAdaptativeThreshold()
{
    if(_originalImage.empty()) return;
    // convert to grayscale, single channel with max over rgb
    // 1) Max-channel grayscale
    cv::Mat ch[3];
    cv::split(_originalImage, ch);        // B, G, R
    cv::Mat maxGray;
    cv::max(ch[0], ch[1], maxGray);
    cv::max(maxGray, ch[2], maxGray);

    // 2) Apply adaptative threshold
    int blockSize = _AdaptiveBlocksizeEdit->text().toInt();
    if (blockSize % 2 == 0) blockSize += 1; // must be odd
    double C = _AdaptiveCEdit->text().toDouble();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    try {
        cv::adaptiveThreshold(maxGray, _currentImage, 255, _adaptParams.method,
                            cv::THRESH_BINARY, blockSize, C);
    } catch (const cv::Exception &e) {
        QMessageBox::critical(this, "Adaptative Threshold Error",
                              QString("Error: %1").arg(e.what()));
        QApplication::restoreOverrideCursor();
        return;
    }
    QApplication::restoreOverrideCursor();
    if(!_currentMask.empty()){
        cv::Mat maskedImage;
        _currentImage.copyTo(maskedImage, _currentMask);
        _currentImage = maskedImage;
    }

    _displayImage();
}