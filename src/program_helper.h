#ifndef PROGRAM_HELPER_H
#define PROGRAM_HELPER_H

#include <string>
#include <vector>
#include <mutex>
#include <map>

#define CHECK_EXIT(expr, info, ...) { \
    int errCode = expr; \
    if (errCode < 0) { \
        fprintf(stderr, (std::string("[ERROR] ") + __FILE__ + "[%d]: " + \
                info + " In [%s]." + " Error code = %d\n").c_str(), \
                __LINE__, ##__VA_ARGS__, __func__, errCode); \
        exit(errCode); \
    } \
}

#define CHECK_RET(expr, info, ...) { \
    int retCode = expr; \
    if (retCode < 0) { \
        fprintf(stderr, (std::string("[ERROR] ") + __FILE__ + "[%d]: " + \
                info + " In [%s]." + " Return %d\n").c_str(), \
                __LINE__, ##__VA_ARGS__, __func__, retCode); \
        return retCode; \
    } \
}

#define CHECK_WARN(expr, info, ...) { \
    if (!(expr)) { \
        fprintf(stderr, (std::string("[WARN] ") + __FILE__ + "[%d]: " + \
            info + " In [%s].\n").c_str(), __LINE__, \
            ##__VA_ARGS__, __func__); \
    } \
}

#define CHECK_ARGS(expr, info, ...) { \
    if (!(expr)) { \
        fprintf(stderr, (std::string("[ERROR] ") + __FILE__ + "[%d]: " + \
                info + " In [%s].\n").c_str(), __LINE__, \
                ##__VA_ARGS__, __func__); \
        return -1; \
    } \
}

namespace program_helper {

// 多线程支持的自适应进度条显示类
class Progress {
public:
    // 构造函数
    Progress(const int totalCount);
    // 析构函数
    ~Progress();
    // 添加新的进度数值
    void addProgress(const int newProgress);

private:
    // 打印信息
    void dumpProgress();

    // 支持多线程进度条
    std::mutex progressLock_;
    
    // 当前进度条的总长度
    int barLength_;
    // 当前要写的barCount
    int barCount_ = 0;
    // 进度条
    char* progressBar_;
    
    // 当前进度的百分比
    double percentage_ = 0;
    // 当前进度的实际数值
    int progressCount_ = 0;
    // 总进度数值
    const int totalCount_;

    // 进度条为空显示的字符
    static const char emptyBarUnit;
    // 进度条占用时显示的字符
    static const char fullBarUnit;
    // 进度条的最长长度
    static const int maxLength;
};

} //namespace program_helper

#endif // PROGRAM_HELPER_H
