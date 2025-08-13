#include <dftracer/core/logging.h>
#include <dftracer/core/singleton.h>
#include <dftracer/serialization/json_line.h>

#include <cstring>
#include <memory>
#include <mutex>
namespace dftracer {
template <>
std::shared_ptr<JsonLines> Singleton<JsonLines>::instance = nullptr;
template <>
bool Singleton<JsonLines>::stop_creating_instances = false;
JsonLines::JsonLines() : include_metadata(false) {
  auto conf = Singleton<ConfigurationManager>::get_instance();
  include_metadata = conf->metadata;
}

size_t JsonLines::initialize(char *buffer, HashType hostname_hash) {
  this->hostname_hash = hostname_hash;
  buffer[0] = '[';
  buffer[1] = '\n';
  return 2;
}
size_t JsonLines::data(char *buffer, int index, ConstEventNameType event_name,
                       ConstEventNameType category, TimeResolution start_time,
                       TimeResolution duration,
                       std::unordered_map<std::string, std::any> *metadata,
                       ProcessID process_id, ThreadID thread_id) {
  size_t written_size = 0;
  if (include_metadata && metadata != nullptr) {
    std::stringstream all_stream;
    bool has_meta = false;
    std::stringstream meta_stream;
    auto meta_size = metadata->size();
    long unsigned int i = 0;
    for (const auto &item : *metadata) {
      has_meta = true;
      if (item.second.type() == typeid(unsigned int)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<unsigned int>(item.second);
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(int)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<int>(item.second);
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(const char *)) {
        meta_stream << "\"" << item.first << "\":\""
                    << std::any_cast<const char *>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(std::string)) {
        meta_stream << "\"" << item.first << "\":\""
                    << std::any_cast<std::string>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(size_t)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<size_t>(item.second) << "";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(uint16_t)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<uint16_t>(item.second) << "";
        if (i < meta_size - 1) meta_stream << ",";

      } else if (item.second.type() == typeid(HashType)) {
        meta_stream << "\"" << item.first << "\":\""
                    << std::any_cast<HashType>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";

      } else if (item.second.type() == typeid(long)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<long>(item.second) << "";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(ssize_t)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<ssize_t>(item.second) << "";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(off_t)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<off_t>(item.second) << "";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(off64_t)) {
        meta_stream << "\"" << item.first
                    << "\":" << std::any_cast<off64_t>(item.second) << "";
        if (i < meta_size - 1) meta_stream << ",";
      } else {
        DFTRACER_LOG_INFO("No conversion for type %s", item.first.c_str());
      }
      i++;
    }
    if (has_meta) {
      all_stream << "," << meta_stream.str();
    }
    written_size = sprintf(
        buffer,
        R"({"id":%d,"name":"%s","cat":"%s","pid":%d,"tid":%lu,"ts":%llu,"dur":%llu,"ph":"X","args":{"hhash":"%s"%s}})",
        index, event_name, category, process_id, thread_id, start_time,
        duration, this->hostname_hash, all_stream.str().c_str());
  } else {
    written_size = sprintf(
        buffer,
        R"({"id":%d,"name":"%s","cat":"%s","pid":%d,"tid":%lu,"ts":%llu,"dur":%llu,"ph":"X"})",
        index, event_name, category, process_id, thread_id, start_time,
        duration);
  }
  if (written_size > 0) {
    buffer[written_size++] = '\n';
    if (strcmp(event_name, "end") == 0 && strcmp(category, "dftracer") == 0) {
      buffer[written_size++] = ']';
    }
    buffer[written_size] = '\0';
  }
  DFTRACER_LOG_DEBUG("JsonLines.serialize %s", buffer);
  return written_size;
}

size_t JsonLines::metadata(char *buffer, int index, ConstEventNameType name,
                           ConstEventNameType value, ConstEventNameType ph,
                           ProcessID process_id, ThreadID thread_id,
                           bool is_string) {
  size_t written_size = 0;
  if (is_string) {
    written_size = sprintf(
        buffer,
        R"({"id":%d,"name":"%s","cat":"dftracer","pid":%d,"tid":%lu,"ph":"M","args":{"hhash":"%s","name":"%s","value":"%s"}})",
        index, ph, process_id, thread_id, this->hostname_hash, name, value);
  } else {
    written_size = sprintf(
        buffer,
        R"({"id":%d,"name":"%s","cat":"dftracer","pid":%dq,"tid":%lu,"ph":"M","args":{"hhash":"%s","name":"%s","value":%s}})",
        index, ph, process_id, thread_id, this->hostname_hash, name, value);
  }
  buffer[written_size++] = '\n';
  buffer[written_size] = '\0';
  DFTRACER_LOG_DEBUG("ChromeWriter.convert_json_metadata %s", buffer);
  return written_size;
}
}  // namespace dftracer