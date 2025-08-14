#include "thread.hpp"
#include "overwrite_macros.hpp"

namespace algocor
{

void setThreadName(const std::string& name)
{
    // pthread_setname_np requires the name to be no more than 16 characters
    if (name.size() > 15) {
        LOG_ERROR("Thread name is too long (max 15 characters)");
        return;
    }

    // Set the thread name
    if (pthread_setname_np(pthread_self(), name.c_str()) != 0) {
        LOG_ERROR("Failed to set thread name");
    }
}

bool pinThreadToCore(int core)
{
    // Get the number of available CPU cores
    unsigned int numCores = std::thread::hardware_concurrency();

    // Check if the requested core is out of range
    if (core < 0 || core >= static_cast<int>(numCores)) {
        LOG_ERROR("Core {} is out of range. Available cores: 0 to {}", core, numCores - 1);
        return false;
    }

    // Get the current thread's native handle
    pthread_t thread = pthread_self();

    // Create a CPU set with the specified core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);       // Clear the set
    CPU_SET(core, &cpuset);  // Add the core to the set

    // Set the thread's affinity
    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0) {
        LOG_ERROR("Failed to pin thread to core {}", core);
        return false;
    }

    return true;
}

bool setSchedulerPolicy(int policy, int priority)
{
    struct sched_param param {};

    // Set the priority
    param.sched_priority = priority;

    // Set the scheduler policy
    if (sched_setscheduler(0, policy, &param) == -1) {
        LOG_ERROR("Failed to set scheduler policy: {}", strerror(errno));
        return false;
    }

    // Verify the scheduler policy
    int currentPolicy = sched_getscheduler(0);
    if (currentPolicy == -1) {
        LOG_ERROR("Failed to get scheduler policy: {}", strerror(errno));
        return false;
    }

    // Print the current scheduler policy
    const char* policyName;
    switch (currentPolicy) {
        case SCHED_FIFO:
            policyName = "SCHED_FIFO";
            break;
        case SCHED_RR:
            policyName = "SCHED_RR";
            break;
        case SCHED_OTHER:
            policyName = "SCHED_OTHER";
            break;
        default:
            policyName = "Unknown";
            break;
    }
    LOG_INFO("Scheduler policy set to: {}", policyName);

    return true;
}

}  // namespace algocor
