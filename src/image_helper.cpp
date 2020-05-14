#include "image_helper.h"
#include "str_helper.h"

#include <unistd.h>
#include <memory.h>

#include <iostream>
#include <cmath>
#include <exception>
#include <mutex>
#include <map>
#include <future>

namespace image_helper {

const double kMercatorLong = 20037508.34;
const double PI = acos(-1.);

// 经纬度转墨卡托坐标
int latlon2Mercator(const double srcX, const double srcY,
        double* targetX, double* targetY) {
    *targetX = srcX * kMercatorLong / 180;
    double temp = std::log(std::tan((90 + srcY) * PI / 360)) / (PI / 180);
    *targetY = temp * kMercatorLong / 180;
    return 0;
}

// 依据当前的墨卡托坐标换算出对应级别的像素坐标
template<class T>
int mercator2Pixel(const int scaleLevel, const double coordX,
        const double coordY, T* pixelX, T* pixelY) {
    int pix = 256 * pow(2, scaleLevel);                                               
    double res = kMercatorLong / (256 * std::pow(2, scaleLevel - 1)); 
    *pixelX = coordX / res + pix / 2;
    *pixelY = coordY / res + pix / 2;
    return 0;
}

// 根据当前的缩放比例尺确定墨卡托坐标下的网格位置
int getGridCoord(const int scaleLevel, const double coordX,
        const double coordY, int* gridX, int* gridY) {
    CHECK_ARGS(coordX > -1e-8 && coordY > -1e-8 &&
            coordX < MC_BOUND && coordY < MC_BOUND,
            "Illegal input mc coord (%f, %f).", coordX, coordY);
    double pixelX, pixelY;
    mercator2Pixel(scaleLevel, coordX, coordY, &pixelX, &pixelY);
    *gridX = pixelX / 256;
    *gridY = pixelY / 256;
    return 0;
}

std::map<std::string, FREE_IMAGE_FORMAT> formatMap = {
        {"bmp", FIF_BMP}, {"cut", FIF_CUT}, {"dds", FIF_DDS}, {"gif", FIF_GIF},
        {"hdr", FIF_HDR}, {"ico", FIF_ICO}, {"iff", FIF_IFF}, {"lbm", FIF_IFF},
        {"jng", FIF_JNG}, {"jpg", FIF_JPEG}, {"jif", FIF_JPEG},
        {"jpeg", FIF_JPEG}, {"jpe", FIF_JPEG}, {"koa", FIF_KOALA},
        {"mng", FIF_MNG}, {"pbm", FIF_PBM}, {"pbmraw", FIF_PBMRAW},
        {"pcd", FIF_PCD}, {"pcx", FIF_PCX}, {"pgm", FIF_PGM},
        {"pgmraw", FIF_PGMRAW}, {"png", FIF_PNG}, {"ppm", FIF_PPM},
        {"ppmraw", FIF_PPMRAW}, {"psd", FIF_PSD}, {"ras", FIF_RAS},
        {"tga", FIF_TARGA}, {"targa", FIF_TARGA}, {"tif", FIF_TIFF},
        {"tiff", FIF_TIFF}, {"wbmp", FIF_WBMP}, {"xbm", FIF_XBM},
        {"xpm", FIF_XPM}};

// 依据文件名获取对应的FREEIMAGE格式枚举类型
FREE_IMAGE_FORMAT getImageFormat(const std::string& imagePath) {
    size_t dotIndex = imagePath.find_last_of('.');
    if (dotIndex == std::string::npos || dotIndex == imagePath.size() - 1) {
        return FIF_UNKNOWN;
    }
    std::string suffix = htk::toLower(imagePath.substr(dotIndex + 1));
    auto mapIter = formatMap.find(suffix);
    if (mapIter != formatMap.end()) {
        return mapIter->second;
    } else {
        return FIF_UNKNOWN;
    }
}

TileImages::TileImages(const std::string& srcImagePath, const int threadNum,
        const double x0, const double y0, const double x1, const double y1,
        const int scaleLevel) : threadNum_(threadNum),
        srcImagePath_(srcImagePath), scaleLevel_(scaleLevel) {
    if (std::abs(x0) > LLX_BOUND || std::abs(x1) > LLX_BOUND ||
            std::abs(y0) > LLY_BOUND || std::abs(y1) > LLY_BOUND ||
            x1 >= x0 || y1 <= y0) {
        std::string errInfo = "Illegal coord for src image: (";
        errInfo = errInfo + std::to_string(x0) + ", " +
                std::to_string(y0) + ")->(" + std::to_string(x1) +
                ", " + std::to_string(y1) + ").";
        throw std::runtime_error(errInfo.c_str());
    }
    if (scaleLevel < 0 || scaleLevel > MAX_SCALE_LEVEL) {
        std::string errInfo = "Illegal scale level(";
        errInfo = errInfo + std::to_string(scaleLevel) + ") for src image.";
        throw std::runtime_error(errInfo.c_str());
    }
    if (threadNum <= 0) {
        std::string errInfo = "Illegal thread number (";
        errInfo = errInfo + std::to_string(threadNum) + ").";
        throw std::runtime_error(errInfo.c_str());
    }
    if (static_cast<unsigned>(threadNum) >
            std::thread::hardware_concurrency()) {
        std::cerr << "Warning: Thread number is set to a number larger " <<
                "than local cpu capacity.\n";
    }
    latlon2Mercator(x0, y0, &x0_, &y0_);
    latlon2Mercator(x1, y1, &x1_, &y1_);
}

TileImages::TileImages(const std::string& srcImagePath) :
        threadNum_(std::thread::hardware_concurrency()),
        srcImagePath_(srcImagePath) {}

TileImages::TileImages(const std::string& srcImagePath, const int threadNum) :
        threadNum_(threadNum), srcImagePath_(srcImagePath) {
    if (threadNum <= 0) {
        std::string errInfo = "Illegal thread number (";
        errInfo = errInfo + std::to_string(threadNum) + ").";
        throw std::runtime_error(errInfo.c_str());
    }
    if (static_cast<unsigned>(threadNum) >
            std::thread::hardware_concurrency()) {
        std::cerr << "Warning: Thread number is set to a number larger " <<
                "than local cpu capacity.\n";
    }
}

TileImages::~TileImages() {
    for (auto imagePtrVec : images_) {
        for (auto imagePtr : imagePtrVec) {
            if (imagePtr) {
                FreeImage_Unload(imagePtr);
            }
        }
    }
}

int TileImages::setImageCoord(const double x0, const double y0,
        const double x1, const double y1) {
    CHECK_ARGS(std::abs(x1) < LLX_BOUND && std::abs(x0) < LLX_BOUND
            && std::abs(y1) < LLY_BOUND && std::abs(y0) < LLY_BOUND
            && x1 > x0 && y1 < y0,
            "Illegal coord for src image: (%f, %f)->(%f, %f).",
            x0, y0, x1, y1);
    latlon2Mercator(x0, y0, &x0_, &y0_);
    latlon2Mercator(x1, y1, &x1_, &y1_);
    return 0;
}

int TileImages::setScaleLevel(const int scaleLevel) {
    CHECK_ARGS(scaleLevel >= 0 && scaleLevel < MAX_SCALE_LEVEL,
            "Illegal scale level(%d) for src image.", scaleLevel);
    scaleLevel_ = scaleLevel;
    return 0;
}

int TileImages::setTileSize(const int width, const int height) {
    CHECK_ARGS(width > 0 && height > 0,
            "Illegal tile size: (%d, %d).", width, height);
    tileWidth_ = width;
    tileHeight_ = height;
    return 0;
}

int TileImages::setThreadNumber(const int threadNum) {
    CHECK_ARGS(threadNum > 0, "Thread number should be at least set to 1.");
    if (static_cast<unsigned>(threadNum) >
            std::thread::hardware_concurrency()) {
        std::cerr << "Warning: Thread number is set to a number(" <<
                threadNum_ << ") greater than cpu capacity (" <<
                std::thread::hardware_concurrency() << ")." << std::endl;
    }
    threadNum_ = threadNum;
    return 0;
}

int TileImages::setSamplingFilter(const FREE_IMAGE_FILTER upSamplingFilter,
        const FREE_IMAGE_FILTER downSamplingFilter) {
    upSamplingFilter_ = upSamplingFilter;
    downSamplingFilter_ = downSamplingFilter;
    return 0;
}

int TileImages::tiling() {
    CHECK_ARGS(images_.empty(), "Src image is already tiled.");
    CHECK_ARGS(access(srcImagePath_.c_str(), R_OK) >= 0,
            "Src image \"%s\" not exist or not readable.",
            srcImagePath_.c_str());
    CHECK_ARGS(x0_ < MC_BOUND && x0_ > -1, "Coord not set for src image.");
    CHECK_ARGS(x1_ > x0_ && y1_ < y0_, "%s (%f, %f)->(%f, %f).",
                "Illegal coord for src image: ", x0_, y0_, x1_, y1_);
    CHECK_ARGS(scaleLevel_ > -1, "Scale level not set for src image.");

    FIBITMAP* srcImage = nullptr;
    // 打开源图片
    std::cout << ">> Opening src image...\n";
    CHECK_RET(openSrcImage(&srcImage), "Failed to open src image in grid.");
    // 对原图片进行缩放
    std::cout << ">> Rescale src image...\n";
    if (scaleSrcImage(&srcImage) < 0) {
        FreeImage_Unload(srcImage);
        CHECK_ARGS(false, "Failed to scale src image in grid.");
    }
    // 对源图片进行填充以填满网格
    std::cout << ">> Filling src image...\n";
    if (fillSrcImage(&srcImage) < 0) {
        FreeImage_Unload(srcImage);
        CHECK_ARGS(false, "Failed to fill src image in grid.");
    }
    // 多线程切分原图片
    std::cout << ">> Cutting src image into tiles...\n";
    if (cutSrcImage(&srcImage) < 0) {
        FreeImage_Unload(srcImage);
        for (auto imagePtrVec : images_) {
            for (auto imagePtr : imagePtrVec) {
                if (imagePtr) {
                    FreeImage_Unload(imagePtr);
                }
            }
        }
        images_.clear();
        CHECK_ARGS(false, "Failed to cut src image into tiles.");
    }
    // 处理结束之后删除原图片内容
    FreeImage_Unload(srcImage);
    std::cout << ">> Tiling process successeded.\n";
    return 0;
}

int TileImages::getTile(FIBITMAP** tileImage, const int gridX,
        const int gridY) {
    CHECK_ARGS(!images_.empty(), "Please get tile image after tiling.");
    CHECK_ARGS(gridX >= gridX0_ &&  gridX <= gridX1_ && gridY >= gridY0_ &&
            gridY <= gridY1_, "Grid coord (%d, %d) %s (%d, %d)->(%d, %d)",
            "out of bound", gridX, gridY, gridX0_, gridY0_, gridX1_, gridY1_);
    *tileImage = images_[gridY - gridY0_][gridX - gridX0_];
    return 0;
}

int TileImages::getTile(FIBITMAP** tileImage, const double coordX,
        const int coordY) {
    CHECK_ARGS(!images_.empty(), "Please get tile image after tiling.");
    int gridX, gridY;
    CHECK_RET(getGridCoord(scaleLevel_, coordX, coordY, &gridX, &gridY),
            "Failed to get grid coord for mc coord (%f, %f).", coordX, coordY);
    CHECK_RET(getTile(tileImage, gridX, gridY),
            "Failed to get tile image with mc coord (%f, %f).",
            coordX, coordY);
    return 0;
}

int TileImages::getTileWithLatLon(FIBITMAP** tileImage, const double coordX,
        const int coordY) {
    CHECK_ARGS(!images_.empty(), "Please get tile image after tiling.");
    double coordMcX, coordMcY;
    latlon2Mercator(coordX, coordY, &coordMcX, &coordMcY);
    int gridX, gridY;
    CHECK_RET(getGridCoord(scaleLevel_, coordX, coordY, &gridX, &gridY),
            "Failed to get grid coord for mc coord (%f, %f).", coordX, coordY);
    CHECK_RET(getTile(tileImage, gridX, gridY),
            "Failed to get tile image with latlon (%f, %f).", coordX, coordY);
    return 0;
}

int TileImages::saveAllTiles(std::function<std::string(const int,
        const int)> pathGenerator) {
    CHECK_ARGS(!images_.empty(), "Please save tile image after tiling.");
    int gridWidth = gridX1_ - gridX0_;
    int gridHeight = gridY0_ - gridY1_;
    int totalCnt = (gridWidth + 1) * (gridHeight + 1);
    for (int gridY = gridY0_; gridY >= gridY1_; gridY--) {
        for (int gridX = gridX0_; gridX <= gridX1_; gridX++) {
            saveWorkList_.push_back(SaveWork {
                    images_[gridY0_ - gridY][gridX - gridX0_], gridX, gridY});
        }
    }
    int workPerThread = static_cast<double>(totalCnt) / threadNum_;
    int startIndex = 0;
    std::vector<std::thread*> threadList;
    std::vector<int> threadStatus(threadNum_, -1);
    program_helper::Progress progressBar(totalCnt);
    std::cout << ">> Start saving all the tile images.\n";
    for (int i = 0; i < threadNum_; i++) {
        if (i != threadNum_ - 1) {
            threadList.push_back(new std::thread(savingWorker, startIndex,
                    workPerThread, saveWorkList_, gridX0_, gridY0_,
                    pathGenerator, &progressBar, &threadStatus[i]));
            startIndex += workPerThread;
        } else {
            threadList.push_back(new std::thread(savingWorker, startIndex,
                    totalCnt - startIndex, saveWorkList_, gridX0_, gridY0_,
                    pathGenerator, &progressBar, &threadStatus[i]));
        }
    }
    for (int i = 0; i < threadNum_; i++) {
        threadList[i]->join();
        delete threadList[i];
    }
    for (int i = 0; i < threadNum_; i++) {
        CHECK_RET(threadStatus[i], "Error occurred in thread[%d].", i);
    }
    return 0;
}

void TileImages::savingWorker(const int startIndex, const int workCnt,
        const std::vector<SaveWork>& workList,
        const int gridX0, const int gridY0,
        std::function<std::string(const int, const int)> pathGenerator,
        program_helper::Progress* progressBar, int* result) {
    const int endIndex = startIndex + workCnt;
    std::string savePath;
    for (int i = startIndex; i < endIndex; i++) {
        const SaveWork& work = workList[i];
        if (!work.tileImage) {
            std::cerr << "Error: Can not save empty tile image.\n";
            return;
        }
        savePath = pathGenerator(work.gridX, work.gridY);
        FREE_IMAGE_FORMAT outputFormat = getImageFormat(savePath);
        if (outputFormat == FIF_UNKNOWN) {
            std::cerr << "Error: Unknown output format in path \"" <<
                    savePath << "\".\n";
            return;
        }
        if (!FreeImage_Save(outputFormat, work.tileImage,
                savePath.c_str(), 0)) {
            std::cerr << "Error: Failed to save image in coord (" <<
                    work.gridX << ", " << work.gridY << ").\n";
            return;
        }
        progressBar->addProgress(1);
    }
    *result = 0;
    return;
}

int TileImages::saveTile(const std::string& savePath, const int gridX,
        const int gridY) {
    std::cout << "-- Save tile image in grid (" << gridX << ", " << gridY <<
            ") to path \"" << savePath << "\"."<< std::endl;
    CHECK_ARGS(!images_.empty(), "Please save tile image after tiling.");
    CHECK_ARGS(gridX >= gridX0_ &&  gridX <= gridX1_ && gridY <= gridY0_ &&
            gridY >= gridY1_, "Grid coord (%d, %d) %s (%d, %d)->(%d, %d)",
            "out of bound", gridX, gridY, gridX0_, gridY0_, gridX1_, gridY1_);
    FREE_IMAGE_FORMAT outputFormat = getImageFormat(savePath);
    CHECK_ARGS(outputFormat != FIF_UNKNOWN,
            "Unknown output format in path \"%s\".", savePath.c_str());
    CHECK_ARGS(FreeImage_Save(outputFormat,
            images_[gridY0_ - gridY][gridX - gridX0_], savePath.c_str(), 0),
            "Failed to save image in coord (%d, %d).", gridX, gridY);
    return 0;
}

int TileImages::saveTile(const std::string& savePath, const double coordX,
        const int coordY) {
    CHECK_ARGS(!images_.empty(), "Please save tile image after tiling.");
    int gridX, gridY;
    CHECK_RET(getGridCoord(scaleLevel_, coordX, coordY, &gridX, &gridY),
            "Failed to get grid coord for mc coord (%f, %f).", coordX, coordY);
    CHECK_RET(saveTile(savePath, gridX, gridY),
            "Failed to save tile image with mc coord (%f, %f).",
            coordX, coordY);
    return 0;
}

int TileImages::saveTileWithLatLon(const std::string& savePath,
        const double coordX, const int coordY) {
    CHECK_ARGS(!images_.empty(), "Please save tile image after tiling.");
    double coordMcX, coordMcY;
    latlon2Mercator(coordX, coordY, &coordMcX, &coordMcY);
    int gridX, gridY;
    CHECK_RET(getGridCoord(scaleLevel_, coordX, coordY, &gridX, &gridY),
            "Failed to get grid coord for mc coord (%f, %f).", coordX, coordY);
    CHECK_RET(saveTile(savePath, gridX, gridY),
            "Failed to save tile image with latlon (%f, %f).", coordX, coordY);
    return 0;
}

int TileImages::openSrcImage(FIBITMAP** srcImage) {
    srcImageFormat_ = getImageFormat(srcImagePath_);
    CHECK_ARGS(srcImageFormat_ != FIF_UNKNOWN, "Unknown format of src image.");
    *srcImage = FreeImage_Load(srcImageFormat_, srcImagePath_.c_str());
    CHECK_ARGS(*srcImage, "Failed to open src image with freeimage api.");
    return 0;
}

int TileImages::scaleSrcImage(FIBITMAP** srcImage) {
    CHECK_RET(getGridCoord(scaleLevel_, x0_, y0_, &gridX0_, &gridY0_),
            "Failed to get grid coord for mc coord (%f, %f).", x0_, y0_);
    CHECK_RET(getGridCoord(scaleLevel_, x1_, y1_, &gridX1_, &gridY1_),
            "Failed to get grid coord for mc coord (%f, %f).", x1_, y1_);
    CHECK_ARGS(gridX1_ >= gridX0_ && gridY1_ <= gridY0_,
            "Error: Calculated grid coord (%d, %d)->(%d, %d) is illegal.");
    
    gridPixelWidth_ = (gridX1_ - gridX0_ + 1) * tileWidth_;
    gridPixelHeight_ = (gridY0_ - gridY1_ + 1) * tileHeight_;
    mercator2Pixel(scaleLevel_, x0_, y0_, &pixelX0_, &pixelY0_);
    mercator2Pixel(scaleLevel_, x1_, y1_, &pixelX1_, &pixelY1_);
    unsigned imagePixelWidth = pixelX1_ - pixelX0_;
    unsigned imagePixelHeight = pixelY0_ - pixelY1_;
  
    std::cout << "-- Rescale src image from "<< FreeImage_GetWidth(*srcImage)
            << "*" << FreeImage_GetHeight(*srcImage);
    std::cout << " to " << imagePixelWidth << "*" << imagePixelHeight <<
            std::endl;
    FIBITMAP* oldSrcImage = *srcImage;
    if (imagePixelWidth >= FreeImage_GetWidth(oldSrcImage)) {
        *srcImage = FreeImage_Rescale(oldSrcImage, imagePixelWidth,
                imagePixelHeight, upSamplingFilter_);
    } else {
        *srcImage = FreeImage_Rescale(oldSrcImage, imagePixelWidth,
                imagePixelHeight, downSamplingFilter_);
    }
    if (*srcImage == NULL) {
        *srcImage = oldSrcImage;
        CHECK_ARGS(false, "Failed to rescale src image.");
    } else {
        FreeImage_Unload(oldSrcImage);
    }
    return 0;
} 

int TileImages::fillSrcImage(FIBITMAP** srcImage) {
    FIBITMAP* newSrcImage = nullptr;
    // 填充缩放后的图片
    std::cout << "-- Fill src image to "<< gridPixelWidth_ << "*" <<
            gridPixelHeight_ << std::endl;
    newSrcImage = FreeImage_Allocate(gridPixelWidth_, gridPixelHeight_, 32);
    CHECK_ARGS(newSrcImage != NULL, "Failed to create background image.");
    if (!FreeImage_Paste(newSrcImage, *srcImage, pixelX0_ & 0xFF,
            256 - (pixelY0_ & 0xFF), 256)) {
        FreeImage_Unload(newSrcImage);
        CHECK_ARGS(false, "Failed to fill src image with empty background.");
        return 0;
    }
    FreeImage_Unload(*srcImage);
    *srcImage = newSrcImage;
    return 0;
}

int TileImages::cutSrcImage(FIBITMAP** srcImage) {
    int gridWidth = gridX1_ - gridX0_;
    int gridHeight = gridY0_ - gridY1_;
    int totalCnt = (gridWidth + 1) * (gridHeight + 1);
    std::cout << "-- Cut src image into " << totalCnt << " tiles\n";
    int gridx = 0;
    int gridy = 0;
    images_.resize(gridHeight + 1,
            std::vector<FIBITMAP*>(gridWidth + 1, nullptr));
    for (int i = 0; i < totalCnt; i++) {
        tileWorkList_.push_back(TileWork {&(images_[gridy][gridx]),
                gridx * tileWidth_, gridy * tileHeight_});
        if (++gridx > gridWidth) {
            gridx = 0;
            gridy++;
        }
    }
    int workPerThread = MIN_WORK_PER_THREAD;
    double threadCnt = static_cast<double>(totalCnt) / workPerThread;
    if (threadCnt - threadNum_ < 0.5) {
        threadNum_ = threadCnt * 10;
        threadNum_ = threadNum_ % 10 > 5 ? threadNum_ / 10 + 1 :
                threadNum_ / 10;
    } else {
        workPerThread = totalCnt / threadNum_;
    }
    int startIndex = 0;
    std::vector<std::thread*> threadList;
    std::vector<int> threadStatus(threadNum_, -1);
    program_helper::Progress progressBar(totalCnt);
    for (int i = 0; i < threadNum_; i++) {
        if (i != threadNum_ - 1) {
            threadList.push_back(new std::thread(tilingWorker, startIndex,
                    workPerThread, tileWorkList_, *srcImage, tileWidth_,
                    tileHeight_, &progressBar, &threadStatus[i]));
            startIndex += workPerThread;
        } else {
            threadList.push_back(new std::thread(tilingWorker, startIndex,
                    totalCnt - startIndex, tileWorkList_, *srcImage, tileWidth_,
                    tileHeight_, &progressBar, &threadStatus[i]));
        }
    }
    for (int i = 0; i < threadNum_; i++) {
        threadList[i]->join();
        delete threadList[i];
    }
    for (int i = 0; i < threadNum_; i++) {
        CHECK_RET(threadStatus[i], "Error occurred in thread[%d].", i);
    }
    return 0;
}

void TileImages::tilingWorker(const int startIndex, const int workCount,
        const std::vector<TileWork>& workList, FIBITMAP* srcImage,
        const int tileWidth, const int tileHeight,
        program_helper::Progress* progressBar, int* result) {
    const int endIndex = startIndex + workCount;
    for (int i = startIndex; i < endIndex; i++) {
        const TileWork& work = workList[i];
        *(work.tileImage) = FreeImage_Copy(srcImage, work.pixelX0,
                work.pixelY0, work.pixelX0 + tileWidth,
                work.pixelY0 + tileHeight);
        if (*(work.tileImage) == NULL) {
            std::cout << "Error: Failed cut tile image (PixelCoord: " <<
                    work.pixelX0 << ", " << work.pixelY0 <<
                    ") from src image." << std::endl;
            return;
        }
        progressBar->addProgress(1);
    }
    *result = 0;
    return;
}

} // namespace image_helper
