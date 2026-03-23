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
#include <QCheckBox>
#include "ImageDisplay.h"

#define CONNECTED_COMPONENTS 0x01
#define HOUGH_CIRCLES        0x02

#define DEFAULT_HOUGH_DP 1.0
#define DEFAULT_HOUGH_MIN_DIST 20.0
#define DEFAULT_HOUGH_PARAM1 10.0
#define DEFAULT_HOUGH_PARAM2 14.0
#define DEFAULT_HOUGH_MIN_RADIUS 40
#define DEFAULT_HOUGH_MAX_RADIUS 60
#define DEFAULT_ADAPT_METHOD cv::ADAPTIVE_THRESH_MEAN_C
#define DEFAULT_ADAPT_BLOCK_SIZE 11
#define DEFAULT_ADAPT_C -10.0

struct HoughParams {
    double dp;
    double minDist;
    double param1;
    double param2;
    int minRadius;
    int maxRadius;
};


struct AdaptativeParams {
    int method;
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

    HoughParams _params = {DEFAULT_HOUGH_DP, DEFAULT_HOUGH_MIN_DIST, DEFAULT_HOUGH_PARAM1, DEFAULT_HOUGH_PARAM2, DEFAULT_HOUGH_MIN_RADIUS, DEFAULT_HOUGH_MAX_RADIUS};
    AdaptativeParams _adaptParams = {DEFAULT_ADAPT_METHOD, DEFAULT_ADAPT_BLOCK_SIZE, DEFAULT_ADAPT_C};

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

    QLabel *_threshValueLabel_R;
    QLabel *_threshValueLabel_G;
    QLabel *_threshValueLabel_B;
    QSlider* _binThreshold_R;
    QSlider* _binThreshold_G;
    QSlider* _binThreshold_B;
    QCheckBox* _invertThresholdR;
    QCheckBox* _invertThresholdG;
    QCheckBox* _invertThresholdB;

private slots:
    void resetImage();
    void applyThreshold();
    void applySpecialThreshold();
    void validateThreshold();
    void connectedComponentsMode();
    void applyMask();
    void getHoughParams();
    void applyHoughCircles();
    void applyAdaptativeThreshold();
};

#endif // MAINWINDOW_H
