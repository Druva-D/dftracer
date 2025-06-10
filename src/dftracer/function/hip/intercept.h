// Created by druva on 6/9/25

#include <dftracer/dftracer_config.hpp>
#ifdef DFTRACER_HIP_TRACING_ENABLE


#include <dftracer/function/generic_function.h>

#include <rocprofiler-sdk/buffer.h>
#include <rocprofiler-sdk/buffer_tracing.h>
#include <rocprofiler-sdk/registration.h>
#include <rocprofiler-sdk/rocprofiler.h>
#include <rocprofiler-sdk/cxx/name_info.hpp>

namespace dftracer {
class HIPFunction : public dftracer::GenericFunction {
 public:
  rocprofiler::sdk::buffer_name_info client_name_info;
  rocprofiler_buffer_id_t client_buffer;
  rocprofiler_context_id_t client_ctx;

  static void tool_tracing_callback(rocprofiler_context_id_t context,
                             rocprofiler_buffer_id_t buffer_id,
                             rocprofiler_record_header_t** headers,
                             size_t num_headers, void* user_data,
                             uint64_t drop_count);
  void initialize() {
    DFTRACER_LOG_DEBUG("HIP Intercept class initialized", "");
    void * tool_data;
    client_ctx = {0};
    client_name_info = rocprofiler::sdk::get_buffer_tracing_names();
    rocprofiler_create_context(&client_ctx);
    constexpr auto buffer_size_bytes = 4096;
    constexpr auto buffer_watermark_bytes =
        buffer_size_bytes - (buffer_size_bytes / 8);

    rocprofiler_create_buffer(client_ctx, buffer_size_bytes,
                              buffer_watermark_bytes,
                              ROCPROFILER_BUFFER_POLICY_LOSSLESS,
                              dftracer::HIPFunction::tool_tracing_callback, tool_data, &client_buffer);

    for (auto itr : {ROCPROFILER_BUFFER_TRACING_HSA_CORE_API,
                     ROCPROFILER_BUFFER_TRACING_HSA_AMD_EXT_API}) {
      rocprofiler_configure_buffer_tracing_service(client_ctx, itr, nullptr, 0,
                                                   client_buffer);
    }

    rocprofiler_configure_buffer_tracing_service(
        client_ctx, ROCPROFILER_BUFFER_TRACING_HIP_RUNTIME_API, nullptr, 0,
        client_buffer);

    rocprofiler_configure_buffer_tracing_service(
        client_ctx, ROCPROFILER_BUFFER_TRACING_KERNEL_DISPATCH, nullptr, 0,
        client_buffer);

    rocprofiler_configure_buffer_tracing_service(
        client_ctx, ROCPROFILER_BUFFER_TRACING_MEMORY_COPY, nullptr, 0,
        client_buffer);

    // May have incompatible kernel so only emit a warning here
    rocprofiler_configure_buffer_tracing_service(
        client_ctx, ROCPROFILER_BUFFER_TRACING_PAGE_MIGRATION, nullptr, 0,
        client_buffer);

    rocprofiler_configure_buffer_tracing_service(
        client_ctx, ROCPROFILER_BUFFER_TRACING_SCRATCH_MEMORY, nullptr, 0,
        client_buffer);

    auto client_thread = rocprofiler_callback_thread_t{};
    rocprofiler_create_callback_thread(&client_thread);

    rocprofiler_assign_callback_thread(client_buffer, client_thread);

    int valid_ctx = 0;
    rocprofiler_context_is_valid(client_ctx, &valid_ctx);

    if (valid_ctx == 0) {
      // notify rocprofiler that initialization failed
      // and all the contexts, buffers, etc. created
      // should be ignored
      throw std::runtime_error("HIP Intercept initialization failed");
    }

    rocprofiler_start_context(client_ctx);
  }

  void finalize() {
    rocprofiler_stop_context(client_ctx);
    rocprofiler_flush_buffer(client_buffer);
  }
};
}  // namespace dftracer

#endif