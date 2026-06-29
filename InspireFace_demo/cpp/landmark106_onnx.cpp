#include "landmark106_onnx.h"

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

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

} // namespace

Landmark106Onnx::Landmark106Onnx(const std::string &modelPath, int numThreads)
    : env_(ORT_LOGGING_LEVEL_WARNING, "landmark106")
{
    sessionOptions_.SetIntraOpNumThreads(numThreads);
    sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
#ifdef _WIN32
    session_ = std::make_unique<Ort::Session>(env_, toWide(modelPath).c_str(), sessionOptions_);
#else
    session_ = std::make_unique<Ort::Session>(env_, modelPath.c_str(), sessionOptions_);
#endif

    Ort::AllocatedStringPtr inName = session_->GetInputNameAllocated(0, allocator_);
    Ort::AllocatedStringPtr outName = session_->GetOutputNameAllocated(0, allocator_);
    inputName_ = inName.get();
    outputName_ = outName.get();

    auto inShape = session_->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    if (inShape.size() >= 4 && inShape[2] > 0)
        inputSize_ = static_cast<int>(inShape[2]);

    auto outShape = session_->GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    if (outShape.size() >= 2 && outShape[1] > 0)
        landmarkCount_ = static_cast<int>(outShape[1] / 2);
}

cv::Mat Landmark106Onnx::estimateAffine(cv::Point2f center, float scale, int imageSize)
{
    cv::Mat M = cv::getRotationMatrix2D(center, 0.0, scale);
    double *m = M.ptr<double>(0);
    m[2] += imageSize * 0.5 - center.x;
    m[5] += imageSize * 0.5 - center.y;
    return M;
}

void Landmark106Onnx::transformPoints(std::vector<cv::Point2f> &pts, const cv::Mat &affineInv)
{
    if (pts.empty())
        return;
    cv::transform(pts, pts, affineInv);
}

std::vector<cv::Point2f> Landmark106Onnx::detect(const cv::Mat &bgr, const FaceBox &box) const
{
    if (bgr.empty())
        return {};

    const float w = box.x2 - box.x1;
    const float h = box.y2 - box.y1;
    const cv::Point2f center((box.x1 + box.x2) * 0.5f, (box.y1 + box.y2) * 0.5f);
    const float scale = static_cast<float>(inputSize_) / ((std::max)(w, h) * 1.5f);

    const cv::Mat affine = estimateAffine(center, scale, inputSize_);
    cv::Mat crop;
    cv::warpAffine(bgr, crop, affine, cv::Size(inputSize_, inputSize_), cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                   cv::Scalar(0, 0, 0));

    cv::Mat blob = cv::dnn::blobFromImage(crop, 1.f / inputStd_, crop.size(),
                                          cv::Scalar(inputMean_, inputMean_, inputMean_), true, false, CV_32F);

    std::vector<int64_t> inputShape = {1, 3, inputSize_, inputSize_};
    Ort::MemoryInfo mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(mem, blob.ptr<float>(), blob.total(),
                                                             inputShape.data(), inputShape.size());

    const char *inputNames[] = {inputName_.c_str()};
    const char *outputNames[] = {outputName_.c_str()};
    auto outputs = session_->Run(Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1, outputNames, 1);

    const float *pred = outputs[0].GetTensorData<float>();
    auto outShape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    int count = landmarkCount_;
    if (outShape.size() >= 2 && outShape[1] > 0)
        count = static_cast<int>(outShape[1] / 2);

    std::vector<cv::Point2f> pts;
    pts.reserve(static_cast<size_t>(count));
    const float half = static_cast<float>(inputSize_ / 2);
    for (int i = 0; i < count; ++i) {
        float x = (pred[i * 2 + 0] + 1.f) * half;
        float y = (pred[i * 2 + 1] + 1.f) * half;
        pts.emplace_back(x, y);
    }

    cv::Mat affineInv;
    cv::invertAffineTransform(affine, affineInv);
    transformPoints(pts, affineInv);
    return pts;
}
