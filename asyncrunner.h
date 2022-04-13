#ifndef ASYNCRUNNER_H
#define ASYNCRUNNER_H

#include <future>
#include <queue>

namespace MyRaytracer 
{
    // Runs tasks on a range of assigments in parallel.
    // For example if we have 20x30 pixels and calculation of every pixel requires some computations then on a multi-core machine
    // we can run 8 tasks in parallel (one task per core) and every task can work on a range of 75 pixels.
    // The run method is asynchronous and will wait until all assignments are processed.
    // This class is not designed for multi-thread usage.
    template <typename Task>
    class AsyncRunner
    {
    public:
        AsyncRunner(const Task &task) : task_(task) {
        }

        // Runs the task on all cores and waits for all instances to finish
        void run(unsigned int totalAssignments) {
            unsigned int parallelTasksCount = std::thread::hardware_concurrency();
            if (totalAssignments < std::thread::hardware_concurrency())
                parallelTasksCount = totalAssignments;

            unsigned int assignmentsPerTask = totalAssignments / parallelTasksCount;
            if (assignmentsPerTask * parallelTasksCount < totalAssignments) {
                assignmentsPerTask++;
            }

            for (unsigned int taskIdx = 0; taskIdx < parallelTasksCount; taskIdx++) {
                unsigned int taskAssignmentStartIdx = taskIdx * assignmentsPerTask;
                unsigned int taskAssignmentEndIdx = (taskIdx + 1) * assignmentsPerTask - 1;

                // The last task might get a different assignment
                if (taskAssignmentEndIdx >= totalAssignments) {
                    taskAssignmentEndIdx = totalAssignments - 1;
                }
                if (taskAssignmentStartIdx <= taskAssignmentEndIdx) {
                    runningTasks_.push(std::async(task_, taskAssignmentStartIdx, taskAssignmentEndIdx));
                }
            }

            
            // Wait for the parallel tasks to finish
            while (!runningTasks_.empty()) {
                runningTasks_.front().wait();
                runningTasks_.pop();
            }
        }

    private:
        Task task_;

        std::queue<std::future<void>> runningTasks_;
    };
}

#endif //ASYNCRUNNER_H