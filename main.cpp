/*******************************************************************************
 * Filename     :   main.cpp
 * Content      :   Main raytracer
 * Created      :   July 4, 2020
 * Authors      :   Yash Patel
 * Language     :   C++17
*******************************************************************************/

#include <fstream>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

DEFINE_int32(width, 640, "image width");
DEFINE_int32(height, 480, "image height");

struct Sphere {
    glm::vec3 m_center;
    cv::Vec3b m_color;
    float m_radius;

    Sphere(glm::vec3 center,
           cv::Vec3b color,
           float radius) :
        m_center(center),
        m_color(color),
        m_radius(radius) {}
};

using Scene = std::vector<Sphere>; // assume only spheres for simplicity

cv::Vec3b raytrace(const Scene& scene, const glm::vec3& light, const glm::vec3& dir) {
    for (const Sphere& sphere : scene) {
        float det = 
            4.0 * (pow(sphere.m_radius, 2.0) - pow(glm::length(sphere.m_center), 2.0))
            + 4.0 * pow(glm::dot(dir, sphere.m_center), 2.0);
        if (det >= 0.0) {
            float intersectionTime = -0.5 * sqrt(det) + glm::dot(dir, sphere.m_center);
            glm::vec3 intersection = dir * intersectionTime;
            glm::vec3 normal = glm::normalize(intersection - sphere.m_center);
            glm::vec3 lightRay = glm::normalize(light - intersection);

            bool kVizNormals = false;
            if (kVizNormals) {
                return cv::Vec3b(128 + normal.x * 127, 128 + normal.y * 127, 128 + normal.z * 127);
            }

            float brightness = 1000.0 / glm::length(light - intersection);
            return sphere.m_color * brightness;
        }
    }
    return cv::Vec3b(0, 0, 0);
}

int main(int argc, char** argv) {
    FLAGS_stderrthreshold = 0;
    FLAGS_logtostderr = 0;

    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    // coordinate system: screen center is world (0, 0), world units are pixels 
    Scene scene;
    scene.emplace_back(
        glm::vec3(0.0, 0.0, 300.0), 
        cv::Vec3b(127, 0, 127),
        200.0
    );

    glm::vec3 light(100.0, 100.0, 300.0);

    float focal = 100.0; // assumed camera focal in pixels
    cv::Mat output(cv::Size(FLAGS_width, FLAGS_height), CV_8UC3);
    for (int y = 0; y < FLAGS_height; y++) {
        for (int x = 0; x < FLAGS_width; x++) {
            glm::vec3 dir = glm::normalize(
                glm::vec3(x - FLAGS_width / 2, y - FLAGS_height / 2, focal)
            );
            output.at<cv::Vec3b>(y, x) = raytrace(scene, light, dir);
        }
    }

    cv::imwrite("output.png", output);

    return 0;
}