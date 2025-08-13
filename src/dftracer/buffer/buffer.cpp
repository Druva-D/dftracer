#include <dftracer/buffer/buffer.h>
template <>
std::shared_ptr<dftracer::BufferManager>
    dftracer::Singleton<dftracer::BufferManager>::instance = nullptr;
template <>
bool dftracer::Singleton<dftracer::BufferManager>::stop_creating_instances =
    false;
namespace dftracer {
int BufferManager::initialize(const char* filename, HashType hostname_hash) {
  DFTRACER_LOG_DEBUG("BufferManager.initialize %s %d", filename, hostname_hash);
  auto conf =
      dftracer::Singleton<dftracer::ConfigurationManager>::get_instance();
  enable_compression = conf->compression;
  buffer_size = conf->write_buffer_size;
  if (buffer == nullptr) {
    buffer = (char*)malloc(buffer_size + 16*1024);
  }
  buffer_pos = 0;
  if (!buffer) {
    DFTRACER_LOG_ERROR("BufferManager.BufferManager Failed to allocate buffer",
                       "");
  }
  this->writer = dftracer::Singleton<dftracer::STDIOWriter>::get_instance();
  this->writer->initialize(filename);
  this->serializer = dftracer::Singleton<dftracer::JsonLines>::get_instance();
  if (enable_compression) {
    this->compressor =
        dftracer::Singleton<dftracer::ZlibCompression>::get_instance();
    this->compressor->initialize(buffer_size);
  }
  size_t size = this->serializer->initialize(buffer, hostname_hash);
  if (enable_compression && size > 0) {
    size = this->compressor->compress(buffer, size);
  }
  if (buffer_pos + size > 0) {
    buffer_pos += size;
    size = this->writer->write(buffer, size);
    if (buffer_pos >= buffer_size) {
      buffer_pos = 0;
    }
  }
  return 0;
}

int BufferManager::finalize(int index) {
  std::unique_lock<std::shared_mutex> lock(mtx);
  if (buffer) {
    size_t size = this->serializer->finalize(buffer + buffer_pos);
    if (enable_compression && size > 0) {
      size = this->compressor->compress(buffer + buffer_pos, size);
    }
    if (buffer_pos + size > 0) {
      buffer_pos += size;
      size = this->writer->write(buffer, buffer_pos, true);
    }
    if (enable_compression) this->compressor->finalize();
    this->writer->finalize(index);
    free(buffer);
    buffer = nullptr;
    buffer_size = 0;
    buffer_pos = 0;
  }
  return 0;
}

void BufferManager::log_data_event(
    int index, ConstEventNameType event_name, ConstEventNameType category,
    TimeResolution start_time, TimeResolution duration,
    std::unordered_map<std::string, std::any>* metadata, ProcessID process_id,
    ThreadID tid) {
  std::unique_lock<std::shared_mutex> lock(mtx);
  DFTRACER_LOG_DEBUG("BufferManager.log_data_event %d", index);
  size_t size = 0;
  if (this->serializer) {
    size =
        this->serializer->data(buffer + buffer_pos, index, event_name, category,
                               start_time, duration, metadata, process_id, tid);
  }
  if (size > 0 && this->enable_compression && this->compressor) {
    size = this->compressor->compress(buffer + buffer_pos, size);
  }
  if (buffer_pos + size > 0) {
    buffer_pos += size;
    size = this->writer->write(buffer, buffer_pos);
    if (buffer_pos >= buffer_size) {
      buffer_pos = 0;
    }
  }
}

void BufferManager::log_metadata_event(int index, ConstEventNameType name,
                                       ConstEventNameType value,
                                       ConstEventNameType ph,
                                       ProcessID process_id, ThreadID tid,
                                       bool is_string) {
  std::unique_lock<std::shared_mutex> lock(mtx);
  DFTRACER_LOG_DEBUG("BufferManager.log_metadata_event %d", index);
  size_t size = 0;
  if (this->serializer) {
    size = this->serializer->metadata(buffer + buffer_pos, index, name, value,
                                      ph, process_id, tid, is_string);
  }
  if (size > 0 && this->enable_compression && this->compressor) {
    size = this->compressor->compress(buffer + buffer_pos, size);
  }
  if (buffer_pos + size > 0) {
    buffer_pos += size;
    size = this->writer->write(buffer, buffer_pos);
    if (buffer_pos >= buffer_size) {
      buffer_pos = 0;
    }
  }
}
}  // namespace dftracer