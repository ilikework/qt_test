#pragma once

#include "scrfd_onnx.h"

#include <opencv2/core.hpp>
#include <onnxruntime_cxx_api.h>

#include <memory>
#include <string>
#include <vector>

class Landmark106Onnx {
public:
    explicit Landmark106Onnx(const std::string &modelPath, int numThreads = 4);

    std::vector<cv::Point2f> detect(const cv::Mat &bgr, const FaceBox &box) const;

private:
    Ort::Env env_;
    Ort::SessionOptions sessionOptions_;
    std::unique_ptr<Ort::Session> session_;
    Ort::AllocatorWithDefaultOptions allocator_;

    std::string inputName_;
    std::string outputName_;
    int inputSize_ = 192;
    int landmarkCount_ = 106;
    float inputMean_ = 127.5f;
    float inputStd_ = 128.f;

    static cv::Mat estimateAffine(cv::Point2f center, float scale, int imageSize);
    static void transformPoints(std::vector<cv::Point2f> &pts, const cv::Mat &affineInv);
};
