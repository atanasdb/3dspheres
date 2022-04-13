#include <QImage>
#include <QDebug>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "raytracer.h"
#include "scenedata.h"

namespace MyRaytracer
{
    RayTracer::RayTracer(SceneData &sceneData) : 
        sceneData_(sceneData),
        antiAliasingEnabled_(true),
        zoomLevel_(100),
        frameBuffer_(nullptr),
        parallelRaytraceRunner_(*this)
    {
        samplesGridDeltas_[0] =  glm::dvec2(-0.3, -0.3);
        samplesGridDeltas_[1] =  glm::dvec2(+0.3, -0.3);
        samplesGridDeltas_[2] =  glm::dvec2(-0.3, +0.3);
        samplesGridDeltas_[3] =  glm::dvec2(+0.3, +0.3);
    }

    double RayTracer::RayTraceParallelTask::rayTrace(const Ray &ray)
    {
        double shade = 0;

        Intersection intersection;
        glm::dvec3 lightPos = outer_.sceneData_.lightPos();

        if (outer_.sceneData_.getIntersection(ray, intersection)) {
            glm::dvec3 lightVector = glm::normalize(lightPos - intersection.point);
            shade = glm::dot(lightVector, intersection.surfaceNormal);
            // Put some ambient light in all cases
            if (shade < 0)
                shade = 0.2 * (1 + shade);
            else 
                shade = (0.2 + 0.8 * shade);
        }

        return shade;
    }

    void RayTracer::RayTraceParallelTask::operator()(unsigned int taskPixelStartIdx, 
                                                     unsigned int taskPixelEndIdx)
    {
        // Find out the first pixel in the range this task is responsible for
        int y = taskPixelStartIdx / kSceneWidth;
        int x = taskPixelStartIdx - y * kSceneWidth;

        Ray ray;
        ray.origin = glm::dvec3(0, 0, 0);

        for (unsigned int pixelIdx = taskPixelStartIdx; pixelIdx <= taskPixelEndIdx; pixelIdx++) {
            double intensity = 0;

            if (outer_.antiAliasingEnabled_) {
                // Take 4 samples for better anti-aliasing
                for (int s = 0; s < 4; s++) {
                    ray.direction = glm::normalize(glm::dvec3((double)(outer_.samplesGridDeltas_[s].x + x - kSceneWidth / 2),
                                                              (double)(outer_.samplesGridDeltas_[s].y + y - kSceneHeight / 2),
                                                              outer_.zoomLevel_));
                    intensity += rayTrace(ray);
                }
                intensity = intensity / 4;
            } else {
                // Take just one sample, going straight to the pixel
                ray.direction = glm::normalize(glm::dvec3((double)(x - (int)kSceneWidth / 2),
                                                          (double)(y - (int)kSceneHeight / 2),
                                                          outer_.zoomLevel_));
                intensity = rayTrace(ray);
            }

            outer_.frameBuffer_[(kSceneHeight - 1 - y) * kSceneWidth + x] = 256 * intensity;

            // Go to the next pixel
            x++;
            if (x >= kSceneWidth) {
                y++; x = 0;
            }
        }
    }

    void RayTracer::traceFrame()
    {
        parallelRaytraceRunner_.run(kSceneWidth * kSceneHeight);
    }
}