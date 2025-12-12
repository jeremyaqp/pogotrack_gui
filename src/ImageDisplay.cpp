#include "ImageDisplay.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>


ImageDisplay::ImageDisplay(QWidget *parent)
    : QWidget(parent),
      scale(1.0),
      panOffset(0,0),
      leftDragging(false),
      rightDragging(false)
{
    setMouseTracking(true);

    positionLabel = new QLabel(this);
    positionLabel->setText("");
    positionLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,128);"); // optional styling
    positionLabel->move(10, 10); // position inside ImageDisplay
    positionLabel->setMinimumWidth(180);  // ensure enough width
    positionLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    positionLabel->show();

    pixelLabel = new QLabel(this);
    pixelLabel->setText("");
    pixelLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,128);"); // optional styling
    pixelLabel->move(10, 30); // position inside ImageDisplay
    pixelLabel->setMinimumWidth(180);  // ensure enough width
    pixelLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    pixelLabel->show();

    lineLabel = new QLabel(this);
    lineLabel->setText("");
    lineLabel->setStyleSheet("color: white; background-color: rgba(0,0,0,128);"); // optional styling
    lineLabel->move(10, 50); // position inside ImageDisplay
    lineLabel->setMinimumWidth(180);  // ensure enough width
    lineLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    lineLabel->show();

    lineLabel2 = new QLabel(this);
    lineLabel2->setText("");
    lineLabel2->setStyleSheet("color: white; background-color: rgba(0,0,0,128);"); // optional styling
    lineLabel2->move(10, 70); // position inside ImageDisplay
    lineLabel2->setMinimumWidth(180);  // ensure enough width
    lineLabel2->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    lineLabel2->show();
}

// Convert cv::Mat (BGR) to QImage (RGB)
QImage ImageDisplay::matToQImage(const cv::Mat &mat)
{
    cv::Mat rgb;
    if(mat.channels() == 3)
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    else if(mat.channels() == 1)
        cv::cvtColor(mat, rgb, cv::COLOR_GRAY2RGB);
    else
        rgb = mat;

    return QImage((const uchar*)rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
}

QString ImageDisplay::getPixelValue(const QPoint &widgetPos) const
{
    if (qimg.isNull())
        return QString();

    // Map widget coordinates to image coordinates (account for pan & scale)
    int x = std::round((widgetPos.x() - panOffset.x()) / scale);
    int y = std::round((widgetPos.y() - panOffset.y()) / scale);

    // Check bounds
    if (x < 0 || y < 0 || x >= qimg.width() || y >= qimg.height())
        return QString();

    // Get pixel
    QColor color = QColor(qimg.pixel(x, y));

    if (qimg.format() == QImage::Format_Grayscale8) {
        return QString::number(color.red());  // grayscale value
    } else {
        return QString("R: %1 - G: %2 - B: %3")
                .arg(color.red())
                .arg(color.green())
                .arg(color.blue());
    }
}

QString ImageDisplay::getPixelPosition(const QPoint &widgetPos) const
{
    if (qimg.isNull())
        return QString();
    int x = std::round((widgetPos.x() - panOffset.x()) / scale);
    int y = std::round((widgetPos.y() - panOffset.y()) / scale);

    // Check bounds
    if (x < 0 || y < 0 || x >= qimg.width() || y >= qimg.height())
        return QString();
    return QString("X: %1 - Y: %2")
                .arg((double)x, 6, 'f', 0)
                .arg((double)y, 6, 'f', 0);

}

void ImageDisplay::setImage(const cv::Mat &mat)
{
    qimg = matToQImage(mat);
    update();
}

void ImageDisplay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::LosslessImageRendering);

    painter.fillRect(rect(), QColor(200, 200, 200));

    if (!qimg.isNull()) {
        // Draw image with scaling and panning
        QPoint topLeft = panOffset;
        QSize scaledSize(qimg.width() * scale, qimg.height() * scale);
        painter.drawImage(QRect(topLeft, scaledSize), qimg);
    }
    if (leftDragging){
        // Draw current dragging circle
        painter.setPen(QPen(Qt::red, 3));
        switch(leftClicTool){
            case DRAW_CIRCLE: {
                QPoint center(
                    (_lineStart.x() + _lineEnd.x()) / 2,
                    (_lineStart.y() + _lineEnd.y()) / 2
                );

                int dx = _lineEnd.x() - _lineStart.x();
                int dy = _lineEnd.y() - _lineStart.y();
                int radius = std::sqrt(dx*dx + dy*dy) / 2;

                painter.drawEllipse(center, radius, radius);
                break;
            }
            case DRAW_RECT: {
                QRect rect(_lineStart, _lineEnd);
                painter.drawRect(rect);
                break;
            }
            case DRAW_LINE:
                painter.drawLine(_lineStart, _lineEnd);
                break;
            default:
                break;
        }
    }
    if (_drawCC)
    {
        painter.setPen(QPen(Qt::yellow, 2));

        for (size_t i = 0; i < _ccCenters.size(); i++)
        {
            QPointF p(
                _ccCenters[i].x() * scale + panOffset.x(),
                _ccCenters[i].y() * scale + panOffset.y()
            );

            // Draw centroid
            painter.drawEllipse(p, 4, 4);

            // Draw area text
            painter.drawText(p + QPointF(6, -6),
                            QString::number(_ccAreas[i]) + " (" + QString::number(_ccCenters[i].x(), 'f', 0) + ", " + QString::number(_ccCenters[i].y(), 'f', 0) + ")");
        }
    }
    if (_drawHough)
    {
        painter.setPen(QPen(Qt::green, 2));

        for (const auto& circle : _circles)
        {
            QPoint center(
                circle[0] * scale + panOffset.x(),
                circle[1] * scale + panOffset.y()
            );
            int radius = circle[2] * scale;

            // Draw circle
            painter.drawEllipse(center, radius, radius);
        }
    }

}

void ImageDisplay::mousePressEvent(QMouseEvent *event)
{
    lastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        leftDragging = true;
        _lineStart = event->pos();
    } else if (event->button() == Qt::RightButton) {
        rightDragging = true;
    }
}

void ImageDisplay::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - lastMousePos;

    _mouseX = event->pos().x();
    _mouseY = event->pos().y();

    if (rightDragging) {
        panOffset += delta;
        update();
    } else if (leftDragging) {
        _lineEnd = event->pos();
        update();
    }

    lastMousePos = event->pos();
    positionLabel->setText(getPixelPosition(event->pos()));
    pixelLabel->setText(getPixelValue(event->pos()));
    switch(leftClicTool){
        case DRAW_LINE:
            setLabelLine();
            break;
        case DRAW_CIRCLE:
            setLabelCirc();
            break;
        case DRAW_RECT:
            setLabelRect();
            break;
        default:
            break;
    }
}

void ImageDisplay::setLabelLine() const{
    if(_lineEnd.isNull() || _lineStart.isNull()) return;
    int x1 = std::round((_lineStart.x() - panOffset.x()) / scale);
    int y1 = std::round((_lineStart.y() - panOffset.y()) / scale);
    int x2 = std::round((_lineEnd.x() - panOffset.x()) / scale);
    int y2 = std::round((_lineEnd.y() - panOffset.y()) / scale);
    double trueLength = std::sqrt(std::pow(x2-x1, 2) + std::pow(y2-y1, 2));
    lineLabel->setText("Length: " + QString("%1").arg(trueLength,  6, 'f', 0));
    lineLabel2->setText("");
}

void ImageDisplay::setLabelRect() const{
    if(_lineEnd.isNull() || _lineStart.isNull()) return;

    QPoint center(
        ((_lineStart.x() + _lineEnd.x()) / 2 - panOffset.x()) / scale,
        ((_lineStart.y() + _lineEnd.y()) / 2 - panOffset.y()) / scale
    );
    int x1 = std::round((_lineStart.x() - panOffset.x()) / scale);
    int y1 = std::round((_lineStart.y() - panOffset.y()) / scale);
    int x2 = std::round((_lineEnd.x() - panOffset.x()) / scale);
    int y2 = std::round((_lineEnd.y() - panOffset.y()) / scale);
    double widthLength  = std::abs(x2 - x1);
    double heightLength = std::abs(y2 - y1);
    lineLabel->setText(QString("W: %1 - H: %2").arg(widthLength,  6, 'f', 0).arg(heightLength, 6, 'f', 0));
    lineLabel2->setText(QString("Cx: %1 - Cy: %2").arg((double)center.x(),  6, 'f', 0).arg((double)center.y(), 6, 'f', 0));
}

void ImageDisplay::setLabelCirc() const{
    if(_lineEnd.isNull() || _lineStart.isNull()) return;
    QPoint center(
        ((_lineStart.x() + _lineEnd.x()) / 2 - panOffset.x()) / scale,
        ((_lineStart.y() + _lineEnd.y()) / 2 - panOffset.y()) / scale
    );

    int dx = (_lineEnd.x() - _lineStart.x())/scale;
    int dy = (_lineEnd.y() - _lineStart.y())/scale;
    double radius = std::sqrt(dx*dx + dy*dy) / 2;
    lineLabel->setText(QString("R: %1").arg(radius,  6, 'f', 0));
    lineLabel2->setText(QString("Cx: %1 - Cy: %2").arg((double)center.x(),  6, 'f', 0).arg((double)center.y(), 6, 'f', 0));
}


void ImageDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        leftDragging = false;
    } else if (event->button() == Qt::RightButton) {
        rightDragging = false;
    }
}
void ImageDisplay::wheelEvent(QWheelEvent *event)
{
    double factor = 1.0 + event->angleDelta().y() / 600.0; // smooth zoom

    // Compute image coordinates under cursor before zoom
    QPoint cursorPos = QPoint(_mouseX, _mouseY);
    QPoint imgCoord = (cursorPos - panOffset) / scale;

    // Apply zoom
    scale *= factor;
    if (scale < 0.1) scale = 0.1;
    if (scale > 10.0) scale = 10.0;

    // Adjust panOffset so that imgCoord stays under cursor
    panOffset = cursorPos - imgCoord * scale;
    update();
}

void ImageDisplay::showConnectedComponents(const cv::Mat &stats,
                                           const cv::Mat &centroids)
{
    _ccCenters.clear();
    _ccAreas.clear();

    int n = stats.rows;  // number of components

    for (int i = 1; i < n; i++)  // skip background (i=0)
    {
        double cx = centroids.at<double>(i, 0);
        double cy = centroids.at<double>(i, 1);
        int area = stats.at<int>(i, cv::CC_STAT_AREA);

        _ccCenters.emplace_back(cx, cy);
        _ccAreas.push_back(area);
    }

    _drawCC = true;
    update();  // triggers paintEvent
}

cv::Mat ImageDisplay::getMaskFromTool() const{
    if (qimg.isNull())
        return cv::Mat();

    // Create mask
    cv::Mat mask = cv::Mat::zeros(qimg.height(), qimg.width(), CV_8UC1);

    // Map widget coordinates to image coordinates (account for pan & scale)
    int x1 = std::round((_lineStart.x() - panOffset.x()) / scale);
    int y1 = std::round((_lineStart.y() - panOffset.y()) / scale);
    int x2 = std::round((_lineEnd.x() - panOffset.x()) / scale);
    int y2 = std::round((_lineEnd.y() - panOffset.y()) / scale);

    switch(leftClicTool){
        case DRAW_CIRCLE: {
            QPoint center(
                (x1 + x2) / 2,
                (y1 + y2) / 2
            );

            int dx = x2 - x1;
            int dy = y2 - y1;
            int radius = std::sqrt(dx*dx + dy*dy) / 2;

            cv::circle(mask, cv::Point(center.x(), center.y()), radius, cv::Scalar(255), -1);
            break;
        }
        case DRAW_RECT: {
            cv::rectangle(mask, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255), -1);
            break;
        }
        default:
            break;
    }

    return mask;
}

void ImageDisplay::showHoughCircles(const std::vector<cv::Vec3f>& circles)
{
    _circles = circles;
    _drawHough = true;
    update();  // triggers paintEvent
}