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
DEFINE_int32(balls, 10, "balls to generate in the scene");
DEFINE_int32(rays, 3, "number of rays to cast (doubled for each direction)");

struct Sphere {
    glm::vec3 m_center;
    cv::Vec3b m_color;
    float m_radius;

    Sphere(const glm::vec3& center,
        const cv::Vec3b& color,
        const float radius) :
        m_center(center),
        m_color(color),
        m_radius(radius) {}
};

struct Ray {
    glm::vec3 m_head;
    glm::vec3 m_dir; // normalize before init

    Ray(const glm::vec3& head,
        const glm::vec3& dir) :
        m_head(head),
        m_dir(dir) {}
};

using Scene = std::vector<Sphere>; // assume only spheres for simplicity

// we return glm::vec3(-1) to indicate no intersection
glm::vec3 intersectSphere(const Sphere& sphere, const Ray& ray) {
    // starting the ray offset is the same as displacing the world in the opposite direction
    glm::vec3 offsetCenter = sphere.m_center - ray.m_head;

    float det =
        4.0 * (pow(sphere.m_radius, 2.0) - pow(glm::length(offsetCenter), 2.0))
        + 4.0 * pow(glm::dot(ray.m_dir, offsetCenter), 2.0);
    if (det >= 0.0) {
        float intersectionTime1 = -0.5 * sqrt(det) + glm::dot(ray.m_dir, offsetCenter);
        float intersectionTime2 = 0.5 * sqrt(det) + glm::dot(ray.m_dir, offsetCenter);

        // want the smaller of the two > 0
        float intersectionTime = (intersectionTime1 < 0 && intersectionTime2 > 0)
            ? std::max(intersectionTime1, intersectionTime2)
            : std::min(intersectionTime1, intersectionTime2);

        if (intersectionTime >= 0.0) {
            return ray.m_head + intersectionTime * ray.m_dir;
        }
    }
    return glm::vec3(-1);
}

bool inShadow(const Scene& scene, const Ray& ray) {
    for (const Sphere& sphere : scene) {
        if (intersectSphere(sphere, ray) != glm::vec3(-1)) {
            return true;
        }
    }
    return false;
}

static constexpr int MAX_BOUNCES = 3; // number of bounces in raytrace

cv::Vec3b raytrace(const Scene& scene, const glm::vec3& light, const Ray& ray, int bounceIter, bool inAir) {
    if (bounceIter >= MAX_BOUNCES) {
        return cv::Vec3b(0, 0, 0);
    }
    
    for (const Sphere& sphere : scene) {
        glm::vec3 intersection = intersectSphere(sphere, ray);
        if (intersection != glm::vec3(-1)) {
            glm::vec3 normal = glm::normalize(intersection - sphere.m_center);
            bool kVizNormals = false;
            if (kVizNormals) {
                return cv::Vec3b(128 + normal.x * 127, 128 + normal.y * 127, 128 + normal.z * 127);
            }

            float eta = inAir ? 1.0f / 1.655f : 1.655f / 1.0;

            glm::vec3 lightDir = glm::normalize(light - intersection);
            glm::vec3 reflectDir = glm::normalize(glm::reflect(-ray.m_dir, normal)); // negate to flip out
            glm::vec3 refractDir = glm::normalize(glm::refract(ray.m_dir, normal, eta)); // negate to flip out

            const float kSmidgen = .05; // we offset from the intersection to avoid self-shadow detection
            Ray shadowRay(intersection + lightDir * kSmidgen, lightDir);
            Ray reflectRay(intersection + reflectDir * kSmidgen, reflectDir);
            Ray refractRay(intersection + refractDir * kSmidgen, refractDir);
            
            float brightness = inShadow(scene, shadowRay) ? 0.125 : 0.50;
            constexpr float fracReflection = 0.25;
            constexpr float fracRefract = 0.25;
            return brightness * sphere.m_color
                + fracReflection * raytrace(scene, light, reflectRay, bounceIter + 1, inAir)
                + fracRefract * raytrace(scene, light, refractRay, bounceIter + 1, !inAir);
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
    
    for (int ball = 0; ball < FLAGS_balls; ball++) {
        ;
        scene.emplace_back(
            glm::vec3(
                rand() % (FLAGS_width / 2) - FLAGS_width / 4, 
                rand() % (FLAGS_height / 2) - FLAGS_height / 4,
                rand() % 1000 + 500),
            cv::Vec3b(rand() % 255, rand() % 255, rand() % 255),
            rand() % 250
        );
    }

    glm::vec3 light(500.0, -500.0, 300.0);
    glm::vec3 origin(0.0);

    float focal = 300.0; // assumed camera focal in pixels
    cv::Mat output(cv::Size(FLAGS_width, FLAGS_height), CV_8UC3);
    for (int y = 0; y < FLAGS_height; y++) {
        for (int x = 0; x < FLAGS_width; x++) {
            cv::Vec3b color(0, 0, 0);

            for (int k = -FLAGS_rays; k <= FLAGS_rays; k++) {
                Ray ray(origin,
                    glm::normalize(glm::vec3(x - FLAGS_width / 2, y - FLAGS_height / 2 + k, focal)));
                color += raytrace(scene, light, ray, 0, true) / FLAGS_rays;
            }
            output.at<cv::Vec3b>(y, x) = color;
        }
    }

    cv::imshow("output", output);
    cv::waitKey(0);
    return 0;
}