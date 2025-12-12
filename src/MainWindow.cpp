#include "MainWindow.h"
#include <QHBoxLayout>
#include <QPixmap>
#include <QImage>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::_setupUI()
{
    setWindowTitle("PogoTrack GUI");
    setWindowIcon(QIcon("../assets/icon.png"));
    // ---- Elements definition ----
    QWidget *centralWidget = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    _display = new ImageDisplay;
    _display->setMinimumSize(640, 480);
    setCentralWidget(_display);

    _sidePanel = new QWidget;
    _sideLayout = new QVBoxLayout;

    QPushButton *browse = new QPushButton("Open test image");
    QRadioButton *lineToolBtn = new QRadioButton("Line");
    QRadioButton *rectToolBtn = new QRadioButton("Rectangle");
    QRadioButton *circToolBtn = new QRadioButton("Circle");
    QPushButton *applyMaskBtn = new QPushButton("Apply Mask");
    QPushButton *resetBtn = new QPushButton("Reset");
    QPushButton *ccBtn = new QPushButton("Connected Components");

    _binThreshold = new QSlider(this);
    _binThreshold->setOrientation(Qt::Horizontal);
    _binThreshold->setRange(0, 255);
    _binThreshold->setValue(255);
    _binThreshold->setSingleStep(1.0);  // optional

    // ---- Import ----
    QLabel *importLabel = new QLabel("Import");
    importLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    _sideLayout->addWidget(importLabel);

    _sideLayout->addWidget(browse);
    _sideLayout->addSpacing(8);   // Space after category

    // ---- Tools ----
    QLabel *maskLabel = new QLabel("Tools");
    maskLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    _sideLayout->addWidget(maskLabel);

    QButtonGroup *toolGroup = new QButtonGroup(this);
    toolGroup->addButton(lineToolBtn);
    toolGroup->addButton(rectToolBtn);
    toolGroup->addButton(circToolBtn);
    lineToolBtn->setChecked(true); // default
    toolGroup->setExclusive(true);

    _sideLayout->addWidget(lineToolBtn);
    _sideLayout->addWidget(rectToolBtn);
    _sideLayout->addWidget(circToolBtn);
    _sideLayout->addWidget(applyMaskBtn);
    _sideLayout->addSpacing(8);

    // ---- Operations ----
    QLabel *operationLabel = new QLabel("Operations");
    operationLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    _sideLayout->addWidget(operationLabel);

    _threshValueLabel = new QLabel("Threshold : 255");

    _sideLayout->addWidget(_threshValueLabel);
    _sideLayout->addWidget(_binThreshold);
    _sideLayout->addWidget(ccBtn);
    _sideLayout->addWidget(resetBtn);

    // ---- Licencing ----
    QWidget *bottomRightBox = new QWidget;
    QVBoxLayout *brLayout = new QVBoxLayout(bottomRightBox);
    brLayout->setContentsMargins(0,0,0,0);
    brLayout->setSpacing(0);

    QLabel *teamLabel = new QLabel("Pogoteam 2025");
    QLabel *licenceLabel = new QLabel("CC BY-NC");
    teamLabel->setStyleSheet("font-size: 14px;");
    licenceLabel->setStyleSheet("font-size: 14px;");

    brLayout->addWidget(teamLabel);
    brLayout->addWidget(licenceLabel);
    brLayout->addStretch();  // push them up inside the box if needed

    _sideLayout->addStretch();  // push everything up
    _sideLayout->addWidget(bottomRightBox, 0, Qt::AlignLeft | Qt::AlignBottom);

    // ---- Global ----
    _sidePanel->setLayout(_sideLayout);
    _sidePanel->setFixedWidth(200);

    mainLayout->addWidget(_display, 1);
    mainLayout->addWidget(_sidePanel);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // ---- Connect buttons ----
    connect(browse, &QPushButton::clicked, this, &MainWindow::_loadImage);
    connect(_binThreshold, &QSlider::valueChanged, this, &MainWindow::applyThreshold);
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

void MainWindow::_displayImage()
{
    if(_currentImage.empty()) return;
    cv::Mat rgb;
    if (_currentImage.channels() == 1)
        cv::cvtColor(_currentImage, rgb, cv::COLOR_GRAY2RGB);
    else
        cv::cvtColor(_currentImage, rgb, cv::COLOR_BGR2RGB);

    _display->setImage(_currentImage);
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
    if(!_currentMask.empty()){
        cv::Mat maskedImage;
        _currentImage.copyTo(maskedImage, _currentMask);
        _currentImage = maskedImage;
    }

    // 3) Display
    _displayImage();
    _threshValueLabel->setText("Threshold : " + QString::number((int)thres));
}

void MainWindow::resetImage()
{
    _currentImage = _originalImage.clone();
    _display->showConnectedComponents(cv::Mat(), cv::Mat()); // clear CC overlay
    _display->resetLine();
    _displayImage();
}

void MainWindow::connectedComponentsMode()
{
    if(_currentImage.empty()) return;
    // Convert to grayscale if needed
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

    // Create a color output where each component has a different color
    _currentImage = cv::Mat::zeros(labels.size(), CV_8UC3);
    cv::RNG rng(12345);

    std::vector<cv::Vec3b> colors(n);
    colors[0] = cv::Vec3b(0,0,0); // background
    for (int i = 1; i < n; i++)
        colors[i] = cv::Vec3b(rng.uniform(50,255), rng.uniform(50,255), rng.uniform(50,255));

    for (int y = 0; y < labels.rows; y++)
        for (int x = 0; x < labels.cols; x++)
            _currentImage.at<cv::Vec3b>(y,x) = colors[ labels.at<int>(y,x) ];

    // Tell ImageDisplay to draw centroids + areas on top
    _display->showConnectedComponents(stats, centroids);

    // Show the updated colored image
    _displayImage();
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