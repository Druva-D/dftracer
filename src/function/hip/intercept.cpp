#include <rocprofiler-sdk/buffer.h>
#include <rocprofiler-sdk/buffer_tracing.h>
#include <rocprofiler-sdk/callback_tracing.h>
#include <rocprofiler-sdk/external_correlation.h>
#include <rocprofiler-sdk/fwd.h>
#include <rocprofiler-sdk/internal_threading.h>
#include <rocprofiler-sdk/registration.h>
#include <rocprofiler-sdk/rocprofiler.h>

void tool_tracing_callback(rocprofiler_context_id_t context,
                           rocprofiler_buffer_id_t buffer_id,
                           rocprofiler_record_header_t** headers,
                           size_t num_headers, void* user_data,
                           uint64_t drop_count) {
  {
    auto function = dftracer::HipInterceptFunction::get_instance();
    auto client_name_info = function->client_name_info;
    assert(user_data != nullptr);
    assert(drop_count == 0 && "drop count should be zero for lossless policy");

    if (num_headers == 0)
      throw std::runtime_error{
          "rocprofiler invoked a buffer callback with no headers. this should "
          "never happen"};
    else if (headers == nullptr)
      throw std::runtime_error{
          "rocprofiler invoked a buffer callback with a null pointer to the "
          "array of headers. this should never happen"};

    for (size_t i = 0; i < num_headers; ++i) {
      auto* header = headers[i];

      auto kind_name = std::string{};
      if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING) {
        const char* _name = nullptr;
        auto _kind =
            static_cast<rocprofiler_buffer_tracing_kind_t>(header->kind);
        rocprofiler_query_buffer_tracing_kind_name(_kind, &_name, nullptr);

        if (_name) {
          static size_t len = 15;

          kind_name = std::string{_name};
          len = std::max(len, kind_name.length());
          kind_name.resize(len, ' ');
          kind_name += " :: ";
        }
      }

      if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING &&
          (header->kind == ROCPROFILER_BUFFER_TRACING_HSA_CORE_API ||
           header->kind == ROCPROFILER_BUFFER_TRACING_HSA_AMD_EXT_API ||
           header->kind == ROCPROFILER_BUFFER_TRACING_HSA_IMAGE_EXT_API ||
           header->kind == ROCPROFILER_BUFFER_TRACING_HSA_FINALIZE_EXT_API)) {
        auto* record =
            static_cast<rocprofiler_buffer_tracing_hsa_api_record_t*>(
                header->payload);

        // Create metadata for HSA API calls
        auto metadata = new std::unordered_map<std::string, std::any>();
        metadata->insert_or_assign("context", context.handle);
        metadata->insert_or_assign("buffer_id", buffer_id.handle);
        metadata->insert_or_assign("extern_cid",
                                   record->correlation_id.external.value);
        metadata->insert_or_assign("kind", record->kind);
        metadata->insert_or_assign("operation", record->operation);

        function->logger->log(client_name_info[record->kind][record->operation],
                              kind_name, record->start_timestamp,
                              record->end_timestamp - record->start_timestamp,
                              record->thread_id, metadata);

        if (record->start_timestamp > record->end_timestamp) {
          auto msg = std::stringstream{};
          msg << "hsa api: start > end (" << record->start_timestamp << " > "
              << record->end_timestamp << "). diff = "
              << (record->start_timestamp - record->end_timestamp);
          std::cerr << "threw an exception " << msg.str() << "\n" << std::flush;
          // throw std::runtime_error{msg.str()};
        }

      } else if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING &&
                 header->kind == ROCPROFILER_BUFFER_TRACING_HIP_RUNTIME_API) {
        auto* record =
            static_cast<rocprofiler_buffer_tracing_hip_api_record_t*>(
                header->payload);

        // Create metadata for HIP API calls
        auto metadata = new std::unordered_map<std::string, std::any>();
        metadata->insert_or_assign("context", context.handle);
        metadata->insert_or_assign("buffer_id", buffer_id.handle);
        metadata->insert_or_assign("extern_cid",
                                   record->correlation_id.external.value);
        metadata->insert_or_assign("kind", record->kind);
        metadata->insert_or_assign("operation", record->operation);

        function->logger->log(client_name_info[record->kind][record->operation],
                              kind_name, record->start_timestamp,
                              record->end_timestamp - record->start_timestamp,
                              record->thread_id, metadata);

        if (record->start_timestamp > record->end_timestamp) {
          auto msg = std::stringstream{};
          msg << "hip api: start > end (" << record->start_timestamp << " > "
              << record->end_timestamp << "). diff = "
              << (record->start_timestamp - record->end_timestamp);
          std::cerr << "threw an exception " << msg.str() << "\n" << std::flush;
          // throw std::runtime_error{msg.str()};
        }

      } else if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING &&
                 header->kind == ROCPROFILER_BUFFER_TRACING_KERNEL_DISPATCH) {
        auto* record =
            static_cast<rocprofiler_buffer_tracing_kernel_dispatch_record_t*>(
                header->payload);

        // Create metadata for kernel dispatch
        auto metadata = new std::unordered_map<std::string, std::any>();
        metadata->insert_or_assign("context", context.handle);
        metadata->insert_or_assign("buffer_id", buffer_id.handle);
        metadata->insert_or_assign("extern_cid",
                                   record->correlation_id.external.value);
        metadata->insert_or_assign("kind", record->kind);
        metadata->insert_or_assign("operation", record->operation);
        metadata->insert_or_assign("agent_id",
                                   record->dispatch_info.agent_id.handle);
        metadata->insert_or_assign("queue_id",
                                   record->dispatch_info.queue_id.handle);
        metadata->insert_or_assign("kernel_id",
                                   record->dispatch_info.kernel_id);
        metadata->insert_or_assign("private_segment_size",
                                   record->dispatch_info.private_segment_size);
        metadata->insert_or_assign("group_segment_size",
                                   record->dispatch_info.group_segment_size);
        metadata->insert_or_assign("workgroup_size_x",
                                   record->dispatch_info.workgroup_size.x);
        metadata->insert_or_assign("workgroup_size_y",
                                   record->dispatch_info.workgroup_size.y);
        metadata->insert_or_assign("workgroup_size_z",
                                   record->dispatch_info.workgroup_size.z);
        metadata->insert_or_assign("grid_size_x",
                                   record->dispatch_info.grid_size.x);
        metadata->insert_or_assign("grid_size_y",
                                   record->dispatch_info.grid_size.y);
        metadata->insert_or_assign("grid_size_z",
                                   record->dispatch_info.grid_size.z);

        function->logger->log(
            client_kernels.at(record->dispatch_info.kernel_id).kernel_name,
            kind_name, record->start_timestamp,
            record->end_timestamp - record->start_timestamp, record->thread_id,
            metadata);

        if (record->start_timestamp > record->end_timestamp)
          throw std::runtime_error("kernel dispatch: start > end");

      } else if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING &&
                 header->kind == ROCPROFILER_BUFFER_TRACING_MEMORY_COPY) {
        auto* record =
            static_cast<rocprofiler_buffer_tracing_memory_copy_record_t*>(
                header->payload);

        // Create metadata for memory copy
        auto metadata = new std::unordered_map<std::string, std::any>();
        metadata->insert_or_assign("context", context.handle);
        metadata->insert_or_assign("buffer_id", buffer_id.handle);
        metadata->insert_or_assign("extern_cid",
                                   record->correlation_id.external.value);
        metadata->insert_or_assign("kind", record->kind);
        metadata->insert_or_assign("operation", record->operation);
        metadata->insert_or_assign("src_agent_id", record->src_agent_id.handle);
        metadata->insert_or_assign("dst_agent_id", record->dst_agent_id.handle);

        function->logger->log(
            client_name_info.at(record->kind, record->operation), kind_name,
            record->start_timestamp,
            record->end_timestamp - record->start_timestamp, record->thread_id,
            metadata);

        if (record->start_timestamp > record->end_timestamp)
          throw std::runtime_error("memory copy: start > end");

      } else if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING &&
                 header->kind == ROCPROFILER_BUFFER_TRACING_PAGE_MIGRATION) {
        auto* record =
            static_cast<rocprofiler_buffer_tracing_page_migration_record_t*>(
                header->payload);

        // Create metadata for page migration
        auto metadata = new std::unordered_map<std::string, std::any>();
        metadata->insert_or_assign("kind", record->kind);
        metadata->insert_or_assign("operation", record->operation);

        // Add operation-specific details to metadata
        switch (record->operation) {
          case ROCPROFILER_PAGE_MIGRATION_PAGE_MIGRATE: {
            metadata->insert_or_assign("read_fault",
                                       record->page_fault.read_fault);
            metadata->insert_or_assign("migrated", record->page_fault.migrated);
            metadata->insert_or_assign("node_id", record->page_fault.node_id);
            metadata->insert_or_assign("address", record->page_fault.address);
            break;
          }
          case ROCPROFILER_PAGE_MIGRATION_PAGE_FAULT: {
            metadata->insert_or_assign("start_addr",
                                       record->page_migrate.start_addr);
            metadata->insert_or_assign("end_addr",
                                       record->page_migrate.end_addr);
            metadata->insert_or_assign("from_node",
                                       record->page_migrate.from_node);
            metadata->insert_or_assign("to_node", record->page_migrate.to_node);
            metadata->insert_or_assign("prefetch_node",
                                       record->page_migrate.prefetch_node);
            metadata->insert_or_assign("preferred_node",
                                       record->page_migrate.preferred_node);
            metadata->insert_or_assign("trigger", record->page_migrate.trigger);
            break;
          }
          case ROCPROFILER_PAGE_MIGRATION_QUEUE_SUSPEND: {
            metadata->insert_or_assign("rescheduled",
                                       record->queue_suspend.rescheduled);
            metadata->insert_or_assign("node_id",
                                       record->queue_suspend.node_id);
            metadata->insert_or_assign("trigger",
                                       record->queue_suspend.trigger);
            break;
          }
          case ROCPROFILER_PAGE_MIGRATION_UNMAP_FROM_GPU: {
            metadata->insert_or_assign("node_id",
                                       record->unmap_from_gpu.node_id);
            metadata->insert_or_assign("start_addr",
                                       record->unmap_from_gpu.start_addr);
            metadata->insert_or_assign("end_addr",
                                       record->unmap_from_gpu.end_addr);
            metadata->insert_or_assign("trigger",
                                       record->unmap_from_gpu.trigger);
            break;
          }
          default:
            throw std::runtime_error{"unexpected page migration value"};
        }

        // Note: page migration uses record->pid instead of getpid()
        function->logger->log(
            client_name_info.at(record->kind, record->operation), kind_name,
            record->start_timestamp,
            record->end_timestamp - record->start_timestamp, record->pid,
            metadata);

        if (record->start_timestamp > record->end_timestamp)
          throw std::runtime_error("page migration: start > end");

      } else if (header->category == ROCPROFILER_BUFFER_CATEGORY_TRACING &&
                 header->kind == ROCPROFILER_BUFFER_TRACING_SCRATCH_MEMORY) {
        auto* record =
            static_cast<rocprofiler_buffer_tracing_scratch_memory_record_t*>(
                header->payload);

        // Create metadata for scratch memory
        auto metadata = new std::unordered_map<std::string, std::any>();
        metadata->insert_or_assign("context", context.handle);
        metadata->insert_or_assign("buffer_id", buffer_id.handle);
        metadata->insert_or_assign("extern_cid",
                                   record->correlation_id.external.value);
        metadata->insert_or_assign("kind", record->kind);
        metadata->insert_or_assign("operation", record->operation);
        metadata->insert_or_assign("agent_id", record->agent_id.handle);
        metadata->insert_or_assign("queue_id", record->queue_id.handle);
        metadata->insert_or_assign("flags", record->flags);

        function->logger->log(
            client_name_info.at(record->kind, record->operation), kind_name,
            record->start_timestamp,
            record->end_timestamp - record->start_timestamp, record->thread_id,
            metadata);

      } else {
        auto _msg = std::stringstream{};
        _msg << "unexpected rocprofiler_record_header_t category + kind: ("
             << header->category << " + " << header->kind << ")";
        throw std::runtime_error{_msg.str()};
      }
    }
  }
}