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

private slots:
    void resetImage();
    void applyThreshold();
    void connectedComponentsMode();
    void applyMask();
};

#endif // MAINWINDOW_H
