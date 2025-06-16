// Created by druva on 6/9/25

#ifndef DFTRACER_HIP_INTERCEPT_H
#define DFTRACER_HIP_INTERCEPT_H

#include <dftracer/dftracer_config.hpp>
#ifdef DFTRACER_HIP_TRACING_ENABLE

#include <dftracer/function/generic_function.h>
#include <rocprofiler-sdk/buffer.h>
#include <rocprofiler-sdk/buffer_tracing.h>
#include <rocprofiler-sdk/registration.h>
#include <rocprofiler-sdk/rocprofiler.h>

#include <rocprofiler-sdk/cxx/name_info.hpp>

namespace dftracer {

using kernel_symbol_data_t =
    rocprofiler_callback_tracing_code_object_kernel_symbol_register_data_t;

class HIPFunction : public dftracer::GenericFunction {
 private:
  rocprofiler::sdk::buffer_name_info client_name_info;
  rocprofiler_buffer_id_t client_buffer;
  rocprofiler_context_id_t client_ctx;
  std::unordered_map<rocprofiler_kernel_id_t, kernel_symbol_data_t>
      client_kernels;

  TimeResolution transform_time(rocprofiler_timestamp_t timestamp);

 public:
  HIPFunction() : dftracer::GenericFunction() {
    DFTRACER_LOG_DEBUG("Creating HIPFunction instance",
                       "");  // Initialize parent
  }

  static void tool_tracing_callback(rocprofiler_context_id_t context,
                                    rocprofiler_buffer_id_t buffer_id,
                                    rocprofiler_record_header_t** headers,
                                    size_t num_headers, void* user_data,
                                    uint64_t drop_count);

  static void thread_precreate(rocprofiler_runtime_library_t lib,
                               void* tool_data);

  static void thread_postcreate(rocprofiler_runtime_library_t lib,
                                void* tool_data);

  static int tool_init(rocprofiler_client_finalize_t fini_func,
                       void* tool_data);
  static void tool_fini(void* tool_data);

  void initialize() override {
    rocprofiler_force_configure(&rocprofiler_configure);
    rocprofiler_start_context(client_ctx);
  }

  void finalize() override {
    rocprofiler_stop_context(client_ctx);
    rocprofiler_flush_buffer(client_buffer);
  }
};
}  // namespace dftracer

#endif
#endif