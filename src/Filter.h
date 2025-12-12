#ifndef FILTER_H
#define FILTER_H

#include <opencv2/opencv.hpp>
#include <QString>

class Filter
{
public:
    virtual ~Filter() {}
    virtual QString name() const = 0;
    virtual cv::Mat apply(const cv::Mat& input) = 0;
};

#endif // FILTER_H
