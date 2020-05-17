#include "program_helper.h"
#include "str_helper.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <cstdlib>

namespace program_helper {

int getTerminalSize(winsize* terminalSize) {
    if (isatty(STDOUT_FILENO) == 0) {
        std::cerr << "Error: No terminal window found." << std::endl;
        return -1;
    }
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, terminalSize) < 0) {
        std::cerr << "Error: Ioctl TIOCGWINSZ error." << std::endl;
        return -1;
    }
    return 0;
}

const char Progress::emptyBarUnit = '-';
const char Progress::fullBarUnit = '#';
const int Progress::maxLength = 160;

Progress::Progress(const int totalCount) : totalCount_(totalCount) {
    struct winsize terminalSize;
    // 获取当前的终端大小并设定进度条长度
    if (getTerminalSize(&terminalSize) >= 0) {
        barLength_ = std::min(maxLength,
                static_cast<int>(terminalSize.ws_col));
        barLength_ = barLength_ - 14 -
                (std::to_string(totalCount).length() << 1);
    } else {
        barLength_ = (maxLength >> 1) - 14 -
                (std::to_string(totalCount).length() << 1);
    }
    barLength_ = barLength_ < 10 ? 10 : barLength_;
    progressBar_ = new char[barLength_ + 1];
    for (int i = 0; i < barLength_; i++) {
        progressBar_[i] = emptyBarUnit;
    }
    progressBar_[barLength_] = 0;
}

Progress::~Progress() {
    delete [] progressBar_;
}

void Progress::addProgress(const int newProgress) {
    std::lock_guard<std::mutex> progressGuard(progressLock_);
    int newProgressCount = progressCount_ + newProgress;
    percentage_ = static_cast<double>(newProgressCount * 100) / totalCount_;
    int oldFilledBarCount = progressCount_ * barLength_ / totalCount_;
    int newFilledBarCount = newProgressCount * barLength_ / totalCount_;
    progressCount_ = newProgressCount;
    if (newFilledBarCount > oldFilledBarCount) {
        for (int i = oldFilledBarCount; i < newFilledBarCount; i++) {
            progressBar_[i] = fullBarUnit;
        }
        dumpProgress();
    }
}

void Progress::dumpProgress() {
    printf("[%s]  %d/%d  %.2f%%", progressBar_, progressCount_, totalCount_,
            percentage_);
    if (progressCount_ == totalCount_) {
        printf("\n");
    } else {
        printf("\r");
        fflush(stdout);
    }
}

} //namespace program_helper
