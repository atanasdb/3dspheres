#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>

#include "asyncrunner.h"
#include "settings.h"

namespace MyRaytracer
{
    struct Ray;
    class SceneData;

    class RayTracer
    {
    public:
        RayTracer(SceneData &sceneData);

        void setFrameBuffer(uchar *frameBuffer) { frameBuffer_ = frameBuffer; }
        void setAntiAliasing(bool antiAliasingEnabled) { antiAliasingEnabled_ = antiAliasingEnabled; }
        void setZoomLevel(int zoomLevel) { zoomLevel_ = zoomLevel; }

        // Renders a frame and puts the output in frameBuffer
        void traceFrame();

    private:
        // Functor that allows the parallelisation of rendering a frame
        struct RayTraceParallelTask
        {
        public:
            RayTraceParallelTask(RayTracer& outer) : outer_(outer) {}
            // Traverse the range of pixels passed and does a ray trace for each of them
            void operator()(unsigned int taskPixelStartIdx, unsigned int taskPixelEndIdx);
            
        private:
            // Calculates the intensity of a specific ray
            double rayTrace(const Ray &ray);

            RayTracer &outer_;
        };
        AsyncRunner<RayTraceParallelTask> parallelRaytraceRunner_;

        SceneData &sceneData_;
        uchar *frameBuffer_;
        bool antiAliasingEnabled_;
        int zoomLevel_;
        glm::dvec2 samplesGridDeltas_[4];
    };
}

#endif  //RAYTRACER_H