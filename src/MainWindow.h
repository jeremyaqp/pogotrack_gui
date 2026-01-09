#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSlider>
#include <QVBoxLayout>
#include <QScrollArea>
#include <opencv2/opencv.hpp>
#include <QDoubleSpinBox>
#include <QShortcut>
#include "ImageDisplay.h"

#define CONNECTED_COMPONENTS 0x01
#define HOUGH_CIRCLES        0x02

enum adaptativeMethod {
    MEAN_C,
    GAUSSIAN_C
};

struct HoughParams {
    double dp;
    double minDist;
    double param1;
    double param2;
    int minRadius;
    int maxRadius;
};


struct AdaptativeParams {
    adaptativeMethod method;
    int blockSize;
    double C;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    ImageDisplay *_display;
    QWidget *_sidePanel;
    QVBoxLayout *_sideLayout;
    QLabel *_threshValueLabel;

    cv::Mat _originalImage;
    cv::Mat _currentImage;
    cv::Mat _currentMask;

    void _setupUI();
    void _loadImage();
    void _displayImage(bool addToStack = true);
    void _displayImage(cv::Mat img, bool addToStack = true);

    uint8_t _currentOverlays = 0;
    cv::Mat _ccstats;
    cv::Mat _cccentroids;
    std::vector<cv::Vec3f> _HoughCircles;

    QSlider* _binThreshold;
    double _imgScale = 1.0;

    HoughParams _params = {1.0, 20.0, 10.0, 14.0, 40, 60};
    AdaptativeParams _adaptParams = {MEAN_C, 11, -10.0};

    QLineEdit *dpEdit;
    QLineEdit *minDistEdit;
    QLineEdit *param1Edit;
    QLineEdit *param2Edit;
    QLineEdit *minRadiusEdit;
    QLineEdit *maxRadiusEdit;

    QRadioButton *meanCBtn;
    QRadioButton *gaussianCBtn;
    QLineEdit *adaptCEdit;
    QLineEdit *adaptBlockSizeEdit;

    std::vector<cv::Mat> _displayedImageStack;
    std::vector<uint8_t> _overlayStack;
    int _stackIndex = -1; // Allows undo functionality

private slots:
    void resetImage();
    void applyThreshold();
    void validateThreshold();
    void connectedComponentsMode();
    void applyMask();
    void getHoughParams();
    void applyHoughCircles();
    void applyAdaptativeThreshold();
};

#endif // MAINWINDOW_H
