#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <opencv2/core.hpp>
#include <onnxruntime_cxx_api.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

struct FaceBox {
    float x1 = 0, y1 = 0, x2 = 0, y2 = 0, score = 0;
    float area() const { return (std::max)(0.f, x2 - x1) * (std::max)(0.f, y2 - y1); }
};

class ScrfdOnnx {
public:
    explicit ScrfdOnnx(const std::string &modelPath, int numThreads = 4);

    std::vector<FaceBox> detect(const cv::Mat &bgr, float scoreThresh = 0.5f, int inputSize = 640) const;

private:
    Ort::Env env_;
    Ort::SessionOptions sessionOptions_;
    std::unique_ptr<Ort::Session> session_;
    Ort::AllocatorWithDefaultOptions allocator_;

    std::string inputName_;
    std::vector<std::string> outputNames_;
    std::vector<const char *> outputNamePtrs_;

    int fmc_ = 0;
    int numAnchors_ = 1;
    bool useKps_ = false;
    std::vector<int> featStrides_;

    float inputMean_ = 127.5f;
    float inputStd_ = 128.f;
    float nmsThresh_ = 0.4f;

    mutable std::map<std::tuple<int, int, int>, std::vector<float>> centerCache_;

    void initFromModel();
    static std::vector<float> distance2bbox(const std::vector<float> &points, const float *distance, int count);
    std::vector<FaceBox> nms(std::vector<FaceBox> dets, float iouThresh) const;
    std::vector<FaceBox> forward(const cv::Mat &detImg, float scale, float scoreThresh) const;
};
