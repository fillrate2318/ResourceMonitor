#pragma once

#include <optional>

#include "CpuTimes.h"

std::optional<double> calculateCpuUsage(const CpuTimes& previous, const CpuTimes& current);
