//
// Created by druva on 6/9/25 from finstrument/functions.h
//

#ifndef DFTRACER_FUNCTION_H
#define DFTRACER_FUNCTION_H
/* Config Header */
#include <dftracer/dftracer_config.hpp>

/* Internal Header */
#include <dftracer/core/logging.h>
#include <dftracer/core/typedef.h>
#include <dftracer/df_logger.h>
#include <dftracer/utils/posix_internal.h>

/* External Header */
#include <dlfcn.h>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace dftracer {
class GenericFunction {
 private:
  static std::shared_ptr<GenericFunction> instance;
  static bool stop_trace;

 public:
  std::shared_ptr<DFTLogger> logger;
  GenericFunction() {
    DFTRACER_LOG_DEBUG("GenericFunction class intercepted", "");
    logger = DFT_LOGGER_INIT();
  }

  virtual void initialize();
  virtual void finalize();
  virtual void log_event();

  ~GenericFunction() {}
  static std::shared_ptr<GenericFunction> get_instance() {
    DFTRACER_LOG_DEBUG("POSIX class get_instance", "");
    if (!stop_trace && instance == nullptr) {
      instance = std::make_shared<GenericFunction>();
    }
    return instance;
  }
  bool is_active() { return !stop_trace; }
};

}  // namespace dftracer
#endif
#endif  // DFTRACER_FUNCTION_H