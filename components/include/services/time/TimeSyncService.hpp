/**
 * @file TimeSyncService.hpp
 * @brief Primitive SNTP time sync service.
 */

#pragma once

#include "esp_err.h"

class TimeSyncService {
public:
  TimeSyncService() = default;

  void setTimezone(const char *timezone) { m_timezone = timezone; }
  esp_err_t sync();

  bool isTimeSet() const { return m_timeSet; }

private:
  const char *m_timezone = "CST-2";
  bool m_timeSet = false;
};
