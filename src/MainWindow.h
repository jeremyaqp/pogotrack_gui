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
#include "ImageDisplay.h"

struct HoughParams {
    double dp;
    double minDist;
    double param1;
    double param2;
    int minRadius;
    int maxRadius;
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
    void _displayImage();

    QSlider* _binThreshold;
    double _imgScale = 1.0;

    HoughParams _params = {1.0, 20.0, 10.0, 14.0, 40, 60};

    QLineEdit *dpEdit;
    QLineEdit *minDistEdit;
    QLineEdit *param1Edit;
    QLineEdit *param2Edit;
    QLineEdit *minRadiusEdit;
    QLineEdit *maxRadiusEdit;

private slots:
    void resetImage();
    void applyThreshold();
    void connectedComponentsMode();
    void applyMask();
    void getHoughParams();
    void applyHoughCircles();
};

#endif // MAINWINDOW_H
