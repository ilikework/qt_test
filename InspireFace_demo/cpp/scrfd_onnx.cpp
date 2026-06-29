#include "scrfd_onnx.h"

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include <array>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

std::wstring toWide(const std::string &s)
{
    if (s.empty())
        return {};
    const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring out(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), len);
    if (!out.empty() && out.back() == L'\0')
        out.pop_back();
    return out;
}

float iou(const FaceBox &a, const FaceBox &b)
{
    const float xx1 = (std::max)(a.x1, b.x1);
    const float yy1 = (std::max)(a.y1, b.y1);
    const float xx2 = (std::min)(a.x2, b.x2);
    const float yy2 = (std::min)(a.y2, b.y2);
    const float w = (std::max)(0.f, xx2 - xx1);
    const float h = (std::max)(0.f, yy2 - yy1);
    const float inter = w * h;
    const float uni = a.area() + b.area() - inter;
    return uni <= 0.f ? 0.f : inter / uni;
}

} // namespace

ScrfdOnnx::ScrfdOnnx(const std::string &modelPath, int numThreads)
    : env_(ORT_LOGGING_LEVEL_WARNING, "scrfd")
{
    sessionOptions_.SetIntraOpNumThreads(numThreads);
    sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
#ifdef _WIN32
    session_ = std::make_unique<Ort::Session>(env_, toWide(modelPath).c_str(), sessionOptions_);
#else
    session_ = std::make_unique<Ort::Session>(env_, modelPath.c_str(), sessionOptions_);
#endif
    initFromModel();
}

void ScrfdOnnx::initFromModel()
{
    Ort::AllocatedStringPtr inName = session_->GetInputNameAllocated(0, allocator_);
    inputName_ = inName.get();

    const size_t outCount = session_->GetOutputCount();
    outputNames_.clear();
    outputNamePtrs_.clear();
    for (size_t i = 0; i < outCount; ++i) {
        Ort::AllocatedStringPtr name = session_->GetOutputNameAllocated(i, allocator_);
        outputNames_.push_back(name.get());
    }
    for (const auto &n : outputNames_)
        outputNamePtrs_.push_back(n.c_str());

    if (outCount == 6) {
        fmc_ = 3;
        featStrides_ = {8, 16, 32};
        numAnchors_ = 2;
        useKps_ = false;
    } else if (outCount == 9) {
        fmc_ = 3;
        featStrides_ = {8, 16, 32};
        numAnchors_ = 2;
        useKps_ = true;
    } else if (outCount == 10) {
        fmc_ = 5;
        featStrides_ = {8, 16, 32, 64, 128};
        numAnchors_ = 1;
        useKps_ = false;
    } else if (outCount == 15) {
        fmc_ = 5;
        featStrides_ = {8, 16, 32, 64, 128};
        numAnchors_ = 1;
        useKps_ = true;
    } else {
        throw std::runtime_error("unsupported SCRFD output count: " + std::to_string(outCount));
    }
}

std::vector<float> ScrfdOnnx::distance2bbox(const std::vector<float> &points, const float *distance, int count)
{
    std::vector<float> out(static_cast<size_t>(count) * 4);
    for (int i = 0; i < count; ++i) {
        const float px = points[static_cast<size_t>(i) * 2];
        const float py = points[static_cast<size_t>(i) * 2 + 1];
        out[static_cast<size_t>(i) * 4 + 0] = px - distance[static_cast<size_t>(i) * 4 + 0];
        out[static_cast<size_t>(i) * 4 + 1] = py - distance[static_cast<size_t>(i) * 4 + 1];
        out[static_cast<size_t>(i) * 4 + 2] = px + distance[static_cast<size_t>(i) * 4 + 2];
        out[static_cast<size_t>(i) * 4 + 3] = py + distance[static_cast<size_t>(i) * 4 + 3];
    }
    return out;
}

std::vector<FaceBox> ScrfdOnnx::nms(std::vector<FaceBox> dets, float iouThresh) const
{
    std::sort(dets.begin(), dets.end(), [](const FaceBox &a, const FaceBox &b) { return a.score > b.score; });
    std::vector<FaceBox> keep;
    std::vector<bool> removed(dets.size(), false);
    for (size_t i = 0; i < dets.size(); ++i) {
        if (removed[i])
            continue;
        keep.push_back(dets[i]);
        for (size_t j = i + 1; j < dets.size(); ++j) {
            if (!removed[j] && iou(dets[i], dets[j]) > iouThresh)
                removed[j] = true;
        }
    }
    return keep;
}

std::vector<FaceBox> ScrfdOnnx::forward(const cv::Mat &detImg, float scale, float scoreThresh) const
{
    cv::Mat blob = cv::dnn::blobFromImage(detImg, 1.f / inputStd_, detImg.size(),
                                          cv::Scalar(inputMean_, inputMean_, inputMean_), true, false, CV_32F);

    std::vector<int64_t> inputShape = {1, 3, detImg.rows, detImg.cols};
    Ort::MemoryInfo mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(mem, blob.ptr<float>(), blob.total(),
                                                             inputShape.data(), inputShape.size());

    const char *inputNames[] = {inputName_.c_str()};
    auto outputs = session_->Run(Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1,
                                 outputNamePtrs_.data(), outputNamePtrs_.size());

    const int inputHeight = detImg.rows;
    const int inputWidth = detImg.cols;
    std::vector<FaceBox> proposals;

    for (int idx = 0; idx < fmc_; ++idx) {
        const int stride = featStrides_[static_cast<size_t>(idx)];
        const float *scores = outputs[static_cast<size_t>(idx)].GetTensorData<float>();
        const float *bboxPreds = outputs[static_cast<size_t>(idx + fmc_)].GetTensorData<float>();

        auto scoreInfo = outputs[static_cast<size_t>(idx)].GetTensorTypeAndShapeInfo();
        auto scoreShape = scoreInfo.GetShape();
        const int height = inputHeight / stride;
        const int width = inputWidth / stride;
        const int anchorCount = height * width * numAnchors_;

        const auto key = std::make_tuple(height, width, stride);
        std::vector<float> centers;
        auto it = centerCache_.find(key);
        if (it != centerCache_.end()) {
            centers = it->second;
        } else {
            centers.resize(static_cast<size_t>(anchorCount) * 2);
            int k = 0;
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    for (int a = 0; a < numAnchors_; ++a) {
                        centers[static_cast<size_t>(k) * 2 + 0] = static_cast<float>(x * stride);
                        centers[static_cast<size_t>(k) * 2 + 1] = static_cast<float>(y * stride);
                        ++k;
                    }
                }
            }
            centerCache_[key] = centers;
        }

        std::vector<float> distances(static_cast<size_t>(anchorCount) * 4);
        for (int i = 0; i < anchorCount; ++i) {
            distances[static_cast<size_t>(i) * 4 + 0] = bboxPreds[static_cast<size_t>(i) * 4 + 0] * stride;
            distances[static_cast<size_t>(i) * 4 + 1] = bboxPreds[static_cast<size_t>(i) * 4 + 1] * stride;
            distances[static_cast<size_t>(i) * 4 + 2] = bboxPreds[static_cast<size_t>(i) * 4 + 2] * stride;
            distances[static_cast<size_t>(i) * 4 + 3] = bboxPreds[static_cast<size_t>(i) * 4 + 3] * stride;
        }

        const std::vector<float> boxes = distance2bbox(centers, distances.data(), anchorCount);
        for (int i = 0; i < anchorCount; ++i) {
            const float score = scores[i];
            if (score < scoreThresh)
                continue;
            FaceBox fb;
            fb.x1 = boxes[static_cast<size_t>(i) * 4 + 0] / scale;
            fb.y1 = boxes[static_cast<size_t>(i) * 4 + 1] / scale;
            fb.x2 = boxes[static_cast<size_t>(i) * 4 + 2] / scale;
            fb.y2 = boxes[static_cast<size_t>(i) * 4 + 3] / scale;
            fb.score = score;
            proposals.push_back(fb);
        }
    }

    return nms(proposals, nmsThresh_);
}

std::vector<FaceBox> ScrfdOnnx::detect(const cv::Mat &bgr, float scoreThresh, int inputSize) const
{
    if (bgr.empty())
        return {};

    const float imRatio = static_cast<float>(bgr.rows) / static_cast<float>(bgr.cols);
    const float modelRatio = static_cast<float>(inputSize) / static_cast<float>(inputSize);
    int newWidth = 0;
    int newHeight = 0;
    if (imRatio > modelRatio) {
        newHeight = inputSize;
        newWidth = static_cast<int>(newHeight / imRatio);
    } else {
        newWidth = inputSize;
        newHeight = static_cast<int>(newWidth * imRatio);
    }

    cv::Mat resized;
    cv::resize(bgr, resized, cv::Size(newWidth, newHeight));
    cv::Mat detImg(inputSize, inputSize, CV_8UC3, cv::Scalar(0, 0, 0));
    resized.copyTo(detImg(cv::Rect(0, 0, newWidth, newHeight)));
    const float detScale = static_cast<float>(newHeight) / static_cast<float>(bgr.rows);
    return forward(detImg, detScale, scoreThresh);
}
