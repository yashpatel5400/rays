/*******************************************************************************
 * Filename     :   main.cpp
 * Content      :   Main raytracer
 * Created      :   July 4, 2020
 * Authors      :   Yash Patel
 * Language     :   C++17
*******************************************************************************/

#include <fstream>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

DEFINE_int32(width, 640, "image width");
DEFINE_int32(height, 480, "image height");

int main(int argc, char** argv) {
    FLAGS_stderrthreshold = 0;
    FLAGS_logtostderr = 0;

    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    cv::Mat output(cv::Size(FLAGS_width, FLAGS_height), CV_8UC3);
    for (int y = 0; y < FLAGS_height; y++) {
        for (int x = 0; x < FLAGS_width; x++) {
            output.at<cv::Vec3b>(y, x) = cv::Vec3b(127, 127, 127);
        }
    }

    cv::imwrite("output.png", output);

    return 0;
}