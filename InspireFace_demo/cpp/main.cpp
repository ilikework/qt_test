#include "landmark106_onnx.h"
#include "scrfd_onnx.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#endif

static const int kJawIndices[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                                  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
static const int kRoi13Indices[] = {1, 5, 9, 13, 17, 21, 25, 29, 32, 72, 74, 86, 90};

static void setupExeDir()
{
#ifdef _WIN32
    char exeDir[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exeDir, MAX_PATH);
    char *slash = std::strrchr(exeDir, '\\');
    if (slash)
        *slash = '\0';
    if (exeDir[0])
        _chdir(exeDir);
    _mkdir("demo_output");
#endif
}

static FaceBox pickLargest(const std::vector<FaceBox> &boxes)
{
    FaceBox best;
    float bestArea = 0.f;
    for (const auto &b : boxes) {
        if (b.area() > bestArea) {
            bestArea = b.area();
            best = b;
        }
    }
    return best;
}

static void drawPoly(cv::Mat &img, const std::vector<cv::Point> &poly, const cv::Scalar &color, bool closed,
                     bool label)
{
    if (poly.size() < 2)
        return;
    if (closed)
        cv::polylines(img, poly, true, color, 2, cv::LINE_AA);
    for (size_t i = 0; i < poly.size(); ++i) {
        cv::circle(img, poly[i], 4, color, -1, cv::LINE_AA);
        if (label) {
            cv::putText(img, std::to_string(i), poly[i] + cv::Point(6, -6), cv::FONT_HERSHEY_SIMPLEX, 0.45, color, 1,
                        cv::LINE_AA);
        }
    }
}

static std::vector<cv::Point> subset(const std::vector<cv::Point2f> &all, const int *idx, int count)
{
    std::vector<cv::Point> out;
    out.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        if (idx[i] < 0 || idx[i] >= static_cast<int>(all.size()))
            continue;
        const auto &p = all[static_cast<size_t>(idx[i])];
        out.emplace_back(static_cast<int>(p.x + 0.5f), static_cast<int>(p.y + 0.5f));
    }
    return out;
}

static void printUsage()
{
    std::printf(
        "usage:\n"
        "  insightface_cpp_demo.exe --image <path> [--models <buffalo_l_dir>]\n"
        "\n"
        "models dir must contain det_10g.onnx and 2d106det.onnx\n"
        "outputs: demo_output/landmarks_all.jpg, landmarks_jaw.jpg, landmarks_roi13.jpg\n");
}

int main(int argc, char **argv)
{
    const char *imagePath = nullptr;
    const char *modelsDir = "models/buffalo_l";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            printUsage();
            return 0;
        }
        if (std::strcmp(argv[i], "--image") == 0 && i + 1 < argc)
            imagePath = argv[++i];
        else if (std::strcmp(argv[i], "--models") == 0 && i + 1 < argc)
            modelsDir = argv[++i];
    }

    setupExeDir();

    if (!imagePath) {
        printUsage();
        std::printf("FAIL: --image is required\n");
        return 1;
    }

    const std::string detPath = std::string(modelsDir) + "/det_10g.onnx";
    const std::string lmkPath = std::string(modelsDir) + "/2d106det.onnx";

    cv::Mat img = cv::imread(imagePath);
    if (img.empty()) {
        std::printf("FAIL: cannot read image: %s\n", imagePath);
        return 2;
    }

    std::printf("InsightFace C++ demo (ONNX Runtime, same model family as InspireFace)\n");
    std::printf("image : %s (%dx%d)\n", imagePath, img.cols, img.rows);
    std::printf("models: %s\n", modelsDir);

    try {
        ScrfdOnnx detector(detPath);
        Landmark106Onnx landmark(lmkPath);

        const std::vector<FaceBox> boxes = detector.detect(img, 0.5f, 640);
        if (boxes.empty()) {
            std::printf("FAIL: no face detected\n");
            return 4;
        }

        const FaceBox face = pickLargest(boxes);
        const std::vector<cv::Point2f> lm106 = landmark.detect(img, face);
        if (lm106.size() < 33) {
            std::printf("FAIL: landmark count=%zu (expected >=106)\n", lm106.size());
            return 5;
        }

        std::printf("faces=%zu, using largest bbox score=%.3f\n", boxes.size(), face.score);
        std::printf("landmarks=%zu\n", lm106.size());

        const std::vector<cv::Point> jaw =
            subset(lm106, kJawIndices, static_cast<int>(sizeof(kJawIndices) / sizeof(kJawIndices[0])));
        const std::vector<cv::Point> roi13 =
            subset(lm106, kRoi13Indices, static_cast<int>(sizeof(kRoi13Indices) / sizeof(kRoi13Indices[0])));

        cv::Mat visAll = img.clone();
        cv::rectangle(visAll, cv::Point(static_cast<int>(face.x1), static_cast<int>(face.y1)),
                      cv::Point(static_cast<int>(face.x2), static_cast<int>(face.y2)), cv::Scalar(0, 255, 0), 2);
        for (size_t i = 0; i < lm106.size(); ++i)
            cv::circle(visAll, lm106[i], 2, cv::Scalar(0, 200, 255), -1, cv::LINE_AA);

        cv::Mat visJaw = img.clone();
        cv::rectangle(visJaw, cv::Point(static_cast<int>(face.x1), static_cast<int>(face.y1)),
                      cv::Point(static_cast<int>(face.x2), static_cast<int>(face.y2)), cv::Scalar(0, 255, 0), 2);
        drawPoly(visJaw, jaw, cv::Scalar(0, 255, 0), true, true);

        cv::Mat visRoi = img.clone();
        cv::rectangle(visRoi, cv::Point(static_cast<int>(face.x1), static_cast<int>(face.y1)),
                      cv::Point(static_cast<int>(face.x2), static_cast<int>(face.y2)), cv::Scalar(0, 255, 0), 2);
        drawPoly(visRoi, roi13, cv::Scalar(0, 0, 255), true, true);

        cv::imwrite("demo_output/landmarks_all.jpg", visAll);
        cv::imwrite("demo_output/landmarks_jaw.jpg", visJaw);
        cv::imwrite("demo_output/landmarks_roi13.jpg", visRoi);

        std::ofstream txt("demo_output/landmarks.txt");
        txt << "# image " << imagePath << "\n";
        txt << "# bbox " << face.x1 << " " << face.y1 << " " << face.x2 << " " << face.y2 << "\n";
        for (size_t i = 0; i < lm106.size(); ++i)
            txt << "lm106 " << i << " " << lm106[i].x << " " << lm106[i].y << "\n";

        std::printf("output -> demo_output/landmarks_all.jpg\n");
        std::printf("output -> demo_output/landmarks_jaw.jpg\n");
        std::printf("output -> demo_output/landmarks_roi13.jpg\n");
        std::printf("done.\n");
        return 0;
    } catch (const std::exception &ex) {
        std::printf("FAIL: %s\n", ex.what());
        std::printf("hint: run scripts\\setup_deps.ps1 to download ONNX Runtime + buffalo_l models\n");
        return 10;
    }
}
