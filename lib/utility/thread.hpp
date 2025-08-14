#pragma once

#include <string>

namespace algocor
{

void setThredName(const std::string& name);
[[nodiscard]] bool pinThreadToCore(int core);
[[nodiscard]] bool setSchedulerPolicy(int policy, int priority);

}  // namespace algocor
