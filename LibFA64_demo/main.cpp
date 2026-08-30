#include "LibFA.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

/* 中心 200x200 区域（640x480 测试图，无脸定位时使用） */
static int g_pxl[] = {220, 140, 420, 140, 420, 340, 220, 340};

static void setupExeDir()
{
    char exeDir[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exeDir, MAX_PATH);
    char *slash = std::strrchr(exeDir, '\\');
    if (slash)
        *slash = '\0';
    if (exeDir[0])
        _chdir(exeDir);
    _mkdir("demo_output");
}

enum FaceMode { FACE_FRONT, FACE_LEFT, FACE_RIGHT, FACE_NONE };

static int runFaceDetect(const char *imagePath, FaceMode mode, std::vector<int> *outPxl = nullptr)
{
    const char *modeName = "front";
    int expectCount = 36;
    if (mode == FACE_LEFT) {
        modeName = "left";
        expectCount = 13;
    } else if (mode == FACE_RIGHT) {
        modeName = "right";
        expectCount = 13;
    }

    if (GetFileAttributesA("shape_predictor_68_face_landmarks.dat") == INVALID_FILE_ATTRIBUTES) {
        std::printf("FAIL: shape_predictor_68_face_landmarks.dat not found in exe dir\n");
        return 2;
    }
    if (!initFaceDetector()) {
        std::printf("FAIL: initFaceDetector failed\n");
        return 3;
    }

    char path[1024];
    std::strncpy(path, imagePath, sizeof(path) - 1);
    T_CONTOUR c = {0, nullptr, nullptr};
    if (mode == FACE_LEFT)
        c = autoMarkLeftFaceByFile(path);
    else if (mode == FACE_RIGHT)
        c = autoMarkRightFaceByFile(path);
    else
        c = autoMarkFaceByFile(path);

    std::printf("face mode=%s count=%d (expect %d)\n", modeName, c.count, expectCount);
    if (c.count <= 0) {
        std::printf("FAIL: no face contour detected\n");
        std::printf("hint: front=36pts --front; *_R.jpg --right; *_L.jpg --left\n");
        freeContour(&c);
        return 4;
    }

    const char *txtName = "demo_output/contour.txt";
    const char *imgName = "demo_output/face_contour.jpg";
    if (mode == FACE_LEFT) {
        txtName = "demo_output/contour_left.txt";
        imgName = "demo_output/face_contour_left.jpg";
    } else if (mode == FACE_RIGHT) {
        txtName = "demo_output/contour_right.txt";
        imgName = "demo_output/face_contour_right.jpg";
    }

    std::ofstream txt(txtName);
    for (int i = 0; i < c.count; ++i)
        std::printf("  [%02d] logical=(%d,%d)\n", i, c.x[i], c.y[i]);
    std::printf("contour coords -> %s (cols: logical_x logical_y full_x full_y)\n", txtName);

    cv::Mat img = cv::imread(imagePath);
    if (img.empty()) {
        std::printf("FAIL: cannot read image for overlay\n");
        freeContour(&c);
        return 5;
    }

    std::printf("note: DLL uses 768 logical width; Y axis is bottom-origin (MagicFace style)\n");
    std::printf("      fullX = logicalX * width/768\n");
    std::printf("      fullY = height-1 - logicalY * width/768  (OpenCV top-origin)\n");

    const double toFullScale = static_cast<double>(img.cols) / 768.0;

    std::vector<cv::Point> poly;
    poly.reserve(static_cast<size_t>(c.count));
    for (int i = 0; i < c.count; ++i) {
        const int fullX = static_cast<int>(c.x[i] * toFullScale + 0.5);
        /* DLL: logicalY = (rows-1-y_top)*width/768 -> invert for OpenCV overlay */
        const int fullY = static_cast<int>(img.rows - 1 - c.y[i] * toFullScale + 0.5);
        poly.emplace_back(fullX, fullY);
        if (outPxl) {
            outPxl->push_back(fullX);
            outPxl->push_back(fullY);
        }
        if (txt)
            txt << i << " " << c.x[i] << " " << c.y[i]
                << " " << fullX << " " << fullY << "\n";
    }

    const cv::Scalar lineColor(0, 255, 0);
    const cv::Scalar pointColor(0, 0, 255);
    cv::polylines(img, poly, true, lineColor, 2, cv::LINE_AA);
    for (int i = 0; i < c.count; ++i) {
        cv::circle(img, poly[static_cast<size_t>(i)], 6, pointColor, -1, cv::LINE_AA);
        cv::putText(img, std::to_string(i),
                    cv::Point(poly[static_cast<size_t>(i)].x + 8, poly[static_cast<size_t>(i)].y - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, lineColor, 1, cv::LINE_AA);
    }

    if (!cv::imwrite(imgName, img)) {
        std::printf("FAIL: cannot write %s\n", imgName);
        freeContour(&c);
        return 6;
    }
    std::printf("face overlay -> %s\n", imgName);

    freeContour(&c);
    return 0;
}

static void runPores(const char *inFile, const int *pxl, int pxl_cnt, int nMin, int nMax)
{
    char outFile[1024];
    std::strncpy(outFile, "demo_output/analyse_pores.jpg", sizeof(outFile) - 1);
    T_ANA_RESULT r = analysePoresByFile(const_cast<char *>(inFile), outFile,
                                        const_cast<int *>(pxl), pxl_cnt, nMin, nMax);
    std::printf("pores: value=%d percent=%d chart=%.4f area%%=%.6f -> %s\n",
                r.value, r.percent, r.percent / 100.0, r.percent / 10000.0, outFile);
}

static bool loadRoiFromFile(const char *path, std::vector<int> *outPxl)
{
    if (!outPxl)
        return false;
    outPxl->clear();

    std::ifstream in(path);
    if (!in) {
        std::printf("FAIL: cannot open ROI file: %s\n", path);
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::vector<int> nums;
        int v = 0;
        while (iss >> v)
            nums.push_back(v);

        if (nums.size() >= 5) {
            outPxl->push_back(nums[3]);
            outPxl->push_back(nums[4]);
        } else if (nums.size() >= 3) {
            outPxl->push_back(nums[1]);
            outPxl->push_back(nums[2]);
        } else if (nums.size() >= 2) {
            outPxl->push_back(nums[0]);
            outPxl->push_back(nums[1]);
        }
    }

    if (outPxl->size() < 6) {
        std::printf("FAIL: ROI needs at least 3 points in %s\n", path);
        outPxl->clear();
        return false;
    }
    std::printf("ROI loaded: %d points from %s\n",
                static_cast<int>(outPxl->size() / 2), path);
    return true;
}

static void runAllAnalyses(const char *inFile, const int *pxl, int pxl_cnt)
{
    char outFile[1024];
    T_ANA_RESULT r;

    std::strncpy(outFile, "demo_output/analyse_spots.jpg", sizeof(outFile) - 1);
    r = analyseSpotsByFile(const_cast<char *>(inFile), outFile,
                           const_cast<int *>(pxl), pxl_cnt, 75, 125);
    std::printf("spots: value=%d percent=%d -> %s\n", r.value, r.percent, outFile);

    std::strncpy(outFile, "demo_output/analyse_wrinkle.jpg", sizeof(outFile) - 1);
    r = analyseWrinkleByFile(const_cast<char *>(inFile), outFile,
                             const_cast<int *>(pxl), pxl_cnt, 40, 500);
    std::printf("wrinkle: value=%d percent=%d -> %s\n", r.value, r.percent, outFile);

    std::strncpy(outFile, "demo_output/analyse_acnes.jpg", sizeof(outFile) - 1);
    r = analyseAcnesByFile(const_cast<char *>(inFile), outFile,
                           const_cast<int *>(pxl), pxl_cnt, 75, 125);
    std::printf("acnes: value=%d percent=%d -> %s\n", r.value, r.percent, outFile);

    std::strncpy(outFile, "demo_output/analyse_evenness.jpg", sizeof(outFile) - 1);
    r = analyseEvennessByFile(const_cast<char *>(inFile), outFile,
                              const_cast<int *>(pxl), pxl_cnt, 0, 0);
    std::printf("evenness: value=%d percent=%d -> %s\n", r.value, r.percent, outFile);
}

static void printUsage()
{
    std::printf(
        "usage:\n"
        "  libfa64_demo.exe --image <path> [options]\n"
        "\n"
        "  --roi <file>                 ROI from DB export (idx x y per line)\n"
        "  --nmin / --nmax              analyse params (default 75 / 125)\n"
        "  --front / --left / --right   auto face contour (36 / 13 / 13 points)\n"
        "  --analyse / --all            run all skin analyses in ROI\n"
        "\n"
        "examples:\n"
        "  libfa64_demo.exe --image 01_L.jpg --roi roi_g06_L_RGB.txt --pores --nmin 75 --nmax 125\n"
        "  libfa64_demo.exe --right --image 01_R.jpg --analyse\n");
}

int main(int argc, char **argv)
{
    const char *imagePath = "demo_output/synthetic_input.bmp";
    const char *roiPath = nullptr;
    bool runAnalyse = false;
    bool runPoresOnly = false;
    bool faceOnly = false;
    int nMin = 75;
    int nMax = 125;
    FaceMode faceMode = FACE_NONE;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--all") == 0 || std::strcmp(argv[i], "--analyse") == 0)
            runAnalyse = true;
        else if (std::strcmp(argv[i], "--pores") == 0)
            runPoresOnly = true;
        else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            printUsage();
            return 0;
        } else if (std::strcmp(argv[i], "--face") == 0)
            faceOnly = true;
        else if (std::strcmp(argv[i], "--roi") == 0 && i + 1 < argc)
            roiPath = argv[++i];
        else if (std::strcmp(argv[i], "--nmin") == 0 && i + 1 < argc)
            nMin = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--nmax") == 0 && i + 1 < argc)
            nMax = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--front") == 0) {
            faceMode = FACE_FRONT;
        } else if (std::strcmp(argv[i], "--left") == 0) {
            faceMode = FACE_LEFT;
        } else if (std::strcmp(argv[i], "--right") == 0) {
            faceMode = FACE_RIGHT;
        } else if (std::strcmp(argv[i], "--image") == 0 && i + 1 < argc)
            imagePath = argv[++i];
    }

    setupExeDir();

    std::printf("LibFA64 demo\n");
    std::printf("image: %s\n", imagePath);
    if (roiPath)
        std::printf("roi: %s\n", roiPath);
    std::printf("nMin=%d nMax=%d\n", nMin, nMax);

    if (faceMode == FACE_NONE && !roiPath)
        faceMode = FACE_FRONT;

    bool explicitFaceMode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--front") == 0 || std::strcmp(argv[i], "--left") == 0
            || std::strcmp(argv[i], "--right") == 0 || std::strcmp(argv[i], "--face") == 0) {
            explicitFaceMode = true;
            break;
        }
    }

    if ((faceOnly || explicitFaceMode) && !runAnalyse && !runPoresOnly && !roiPath)
        return runFaceDetect(imagePath, faceMode, nullptr);

    setExtraInfo(30, 1, SOURCE_RGB);

    const int pixels = libfaTestReadImage(imagePath);
    if (pixels <= 0) {
        std::printf("FAIL: cannot read image (put a BMP/JPG in demo_output/ or use --image path)\n");
        return 1;
    }
    std::printf("image pixels: %d\n", pixels);

    std::vector<int> roiPxl;
    if (roiPath) {
        if (!loadRoiFromFile(roiPath, &roiPxl))
            return 2;
    } else {
        const int rc = runFaceDetect(imagePath, faceMode, &roiPxl);
        if (rc != 0)
            return rc;
    }

    if (!runAnalyse && !runPoresOnly) {
        runPores(imagePath, roiPxl.data(), static_cast<int>(roiPxl.size()), nMin, nMax);
        std::printf("done. output in demo_output/\n");
        return 0;
    }

    if (roiPxl.empty()) {
        std::printf("WARN: no ROI, fallback to center rectangle\n");
        roiPxl.assign(g_pxl, g_pxl + 8);
    }

    std::printf("analyse ROI: %d points (%d coords)\n",
                static_cast<int>(roiPxl.size() / 2), static_cast<int>(roiPxl.size()));
    runPores(imagePath, roiPxl.data(), static_cast<int>(roiPxl.size()), nMin, nMax);
    if (runAnalyse)
        runAllAnalyses(imagePath, roiPxl.data(), static_cast<int>(roiPxl.size()));

    std::printf("done. output in demo_output/\n");
    return 0;
}
