#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QVector>
#include <opencv2/opencv.hpp>
#include <QLabel>


enum leftClicToolType {DRAW_LINE, DRAW_CIRCLE, DRAW_RECT, NONE};

class ImageDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit ImageDisplay(QWidget *parent = nullptr);

    // Set image from cv::Mat
    void setImage(const cv::Mat &mat);
    QString getPixelValue(const QPoint &widgetPos) const;
    QString getPixelPosition(const QPoint &widgetPos) const;
    void setLabelLine() const;
    void setLabelRect() const;
    void setLabelCirc() const;
    void showConnectedComponents(const cv::Mat &stats,
                                           const cv::Mat &centroids);
    void hideConnectedComponents();
    leftClicToolType leftClicTool = DRAW_LINE;

    void resetLine(){
        _lineStart = QPoint();
        _lineEnd = QPoint();
    };
    cv::Mat getMaskFromTool() const;

    void showHoughCircles(const std::vector<cv::Vec3f>& circles);
    void hideHoughCircles(){
        _drawHough = false;
        update();
    };

protected:
    // Qt event overrides
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QImage qimg;                 // Current image to display
    double scale;                // Zoom factor
    QPoint panOffset;            // Current pan offset
    QPoint lastMousePos;         // Last mouse position for drag
    QLabel *positionLabel;
    QLabel *pixelLabel;
    QLabel *lineLabel;
    QLabel *lineLabel2;

    bool leftDragging;           // Panning
    bool rightDragging;          // Drawing circle
    QPoint _lineStart;          
    QPoint _lineEnd;    

    double _mouseX;
    double _mouseY;

    std::vector<QPointF> _ccCenters;
    std::vector<int> _ccAreas;
    bool _drawCC = false;

    // Helpers
    QImage matToQImage(const cv::Mat &mat);
    std::vector<cv::Vec3f> _circles;
    bool _drawHough = false;
};

#endif // IMAGEDISPLAY_H
