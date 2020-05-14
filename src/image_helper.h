#ifndef IMAGE_HELPER_H
#define IMAGE_HELPER_H

#include "program_helper.h"
#include <FreeImagePlus.h>

#include <string>
#include <vector>
#include <functional>

namespace image_helper {

// 多线程模型下，每一个线程最少的工作数目
#define MIN_WORK_PER_THREAD 16

// 最大的缩放等级
#define MAX_SCALE_LEVEL 20

// 经纬度坐标的绝对值上界
#define LLX_BOUND 180.00000001
#define LLY_BOUND 90.00000001

// 墨卡托坐标的边界
#define MC_BOUND 40075017

class TileImages {
public:
    // 简单构造函数，需要设置其他参数
    TileImages(const std::string& srcImagePath);
    // 简单构造函数，需要设置其他参数
    TileImages(const std::string& srcImagePath, const int threadNum);
    // 快速构造函数(使用经纬度坐标, 不建议使用，出错会返回runtime_exception)
    TileImages(const std::string& srcImagePath, const int threadNum,
            const double x0, const double y0, const double x1,
            const double y1, const int scaleLevel);
    // 析构函数
    ~TileImages();

    // 设置原图片左上角和右下角的经纬度坐标
    int setImageCoord(const double x0, const double y0,
            const double x1, const double y1);
    // 设置当前图片的比例尺等级
    int setScaleLevel(const int scaleLevel);
    // 设置切割图片的尺寸大小(默认为256 * 256)(可以使用默认值)
    int setTileSize(const int width, const int height);
    // 设置执行的线程数目(可以使用默认值)
    int setThreadNumber(const int threadNum);
    
    // 设置图片缩放使用的采样过滤器(可以使用默认值)
    // 可选过滤器如下
    // FILTER_BOX: 箱形(Box), 脉冲,傅立叶窗,1阶(常量) B样条
    // FILTER_BLINEEAR: 双线性(Bilinear)滤镜
    // FILTER BSPLINE: 4阶(立方)B样条
    // FILTER BICUBIC: Mitchell-Netravali双参数立方(two-param cubic)滤镜
    // FILTER CATMULLROM: Catmull-Rom样条,Overhauser样条
    // FILTER LANCZOS3: Lanczos-windowed sinc滤镜
    int setSamplingFilter(const FREE_IMAGE_FILTER upSamplingFilter,
            const FREE_IMAGE_FILTER downSamplingFilter);

    // 进行图片切分，该函数只需要调用一次
    int tiling();
    
    // 获取一个网格坐标下的瓦片图片数据(只读数据，不允许修改)
    int getTile(FIBITMAP** tileImage, const int gridX, const int gridY);
    // 获取一个墨卡托坐标下的瓦片图片数据(只读数据，不允许修改)
    int getTile(FIBITMAP** tileImage, const double coordX, const int coordY);
    // 获取一个经纬度坐标下的瓦片图片数据(使用专门的函数处理经纬度)
    int getTileWithLatLon(FIBITMAP** tileImage, const double coordX,
            const int coordY);

    // 自动保存所有的瓦片图，对应的保存路径由给定的函数生成
    int saveAllTiles(std::function<std::string(const int, const int)>
            pathGenerator);
    // 保存网格坐标下的瓦片图片(路径包含保存的图片文件名)
    int saveTile(const std::string& savePath, const int gridX,
            const int gridY);
    // 保存墨卡托坐标下的瓦片图片(路径包含保存的图片文件名)
    int saveTile(const std::string& savePath, const double coordX,
            const int coordY);
    // 保存经纬度坐标下的瓦片图片(路径包含保存的图片文件名)
    int saveTileWithLatLon(const std::string& savePath, const double coordX,
            const int coordY);

private:
    // 打开输入的源图片
    int openSrcImage(FIBITMAP** srcImage);
    // 根据当前比例尺进行原始图片进行缩放
    int scaleSrcImage(FIBITMAP** srcImage);
    // 根据当前网格位置对源图片进行填充
    int fillSrcImage(FIBITMAP** srcImage);
    // 执行多线程的图片裁剪工作
    int cutSrcImage(FIBITMAP** srcImage);

private:
    // 多线程执行的任务信息
    struct TileWork {
        // 目标瓦片图片的指针
        FIBITMAP** tileImage;
        // 目标瓦片图片左上角的像素坐标
        const int pixelX0, pixelY0;
    };
    
    // 多线程执行的Worker函数
    static void tilingWorker(const int startIndex, const int workCnt,
            const std::vector<TileWork>& workList, FIBITMAP* srcImage,
            const int tileWidth, const int tileHeight,
            program_helper::Progress* progressBar, int* result);
    
    // 多线程执行的任务信息
    struct SaveWork {
        // 目标瓦片图片的指针
        FIBITMAP* tileImage;
        // 目标瓦片图片左上角的像素坐标
        const int gridX, gridY;
    };
    
    // 多线程执行保存的Worker函数
    static void savingWorker(const int startIndex, const int workCnt,
            const std::vector<SaveWork>& workList, 
            const int gridX0, const int gridY0,
            std::function<std::string(const int, const int)> pathGenerator,
            program_helper::Progress* progressBar, int* result);
    
    // 执行使用的线程数目
    int threadNum_ = -1;
    // 进行图片缩放使用的过滤器类型
    FREE_IMAGE_FILTER upSamplingFilter_ = FILTER_BSPLINE;
    FREE_IMAGE_FILTER downSamplingFilter_ = FILTER_BOX;
    // 多线程执行的Worker队列
    std::vector<TileWork> tileWorkList_;
    // 多线程执行保存图片的Worker队列
    std::vector<SaveWork> saveWorkList_;

private:
    // 原始图片的路径
    const std::string srcImagePath_;
    // 原始图片的格式类型
    FREE_IMAGE_FORMAT srcImageFormat_;
   
    // 瓦片图片的默认大小，单位为像素
    int tileWidth_ = 256;
    int tileHeight_ = 256;
    // 原始图片的左上角点和右下角点的墨卡托坐标
    double x0_ = -1;
    double y0_ = -1;
    double x1_ = -1;
    double y1_ = -1;
    // 当前的比例尺等级
    int scaleLevel_ = -1;

    // 原图片最左上角和右下角图片的像素编号
    int pixelX0_, pixelY0_, pixelX1_, pixelY1_;
    // 原图片最左上角和右下角的图片的网格编号
    int gridX0_, gridY0_, gridX1_, gridY1_;
    // 源图片所在网格边界框的像素高宽
    int gridPixelWidth_, gridPixelHeight_;
    // 所有分割后图片的存储实体
    std::vector<std::vector<FIBITMAP*>> images_;
};

} // namespace image_helper

#endif // IMAGE_HELPER_H
