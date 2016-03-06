#include <iostream>
#include <unistd.h>

static void printConsoleProgressBar(double progress, QString message = QString()) {
    int barWidth = 70;

    if(!message.isEmpty())
        std::cout << message.toUtf8().constData() << "\r";
    std::cout << "[";
    int pos = int(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
    std::cout << std::endl;
}


int main(int argc, char *argv[]) {
    float progress = 0.0;
    while (progress <= 1.0) {
        int barWidth = 70;

        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();

        usleep(1000000);
        progress += 0.16; // for demonstration only
    }
    std::cout << std::endl;
}