#include "sd_logger.h"
#include <cstring>
#include "esphome/core/log.h"
#include "FS.h"
#include "SD.h"

namespace esphome {
namespace sd_logger {

static const char *TAG = "sd_logger.component";

void SDLogger::setup() {
  LOG_PIN("CS Pin: ", this->cs_);
  /*SD.begin(spi::Utility::get_pin_no(this->cs_));*/
  if (!SD.begin(spi::Utility::get_pin_no(this->cs_))) {
    ESP_LOGE(TAG, "Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    ESP_LOGW(TAG, "No SD card attached");
    return;
  }
  ESP_LOGI(TAG, "Initializing SD card...");
  if (!SD.begin(spi::Utility::get_pin_no(this->cs_))) {
    ESP_LOGE(TAG, "ERROR - SD card initialization failed!");
    return;  // init failed
  }
}

void SDLogger::loop() {}

void SDLogger::dump_config() { ESP_LOGCONFIG(TAG, "SD Logger"); }

void SDLogger::writeFile(const char *filename, const char *message) {
  std::string result = createFilename(filename);

  ESP_LOGI(TAG, "Writing file: %s", result.c_str());

  File file = SD.open(result.c_str(), FILE_WRITE);
  if (!file) {
    ESP_LOGE(TAG, "Failed to open file for writing: %s", result.c_str());
    return;
  }
  if (file.print(message)) {
    ESP_LOGI(TAG, "File written");
  } else {
    ESP_LOGE(TAG, "Write failed");
  }
  file.close();
}

void SDLogger::appendFile(const char *filename, const char *message) {
  std::string result = createFilename(filename);

  ESP_LOGI(TAG, "Appending to file: %s", result.c_str());
  File file = SD.open(result.c_str(), FILE_APPEND);
  if (!file) {
    ESP_LOGE(TAG, "Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    ESP_LOGI(TAG, "Message appended");
  } else {
    ESP_LOGE(TAG, "Append failed");
  }
  file.close();
}

std::string SDLogger::createFilename(const char *filename) {
  return std::string("/") + filename;
}

std::string SDLogger::getFirstFileFilename(const char *dir) {
  File root = SD.open(dir, FILE_READ);
  if (!root || !root.isDirectory()) {
    ESP_LOGE(TAG, "Failed to open directory");
    return "";
  }

  File entry = root.openNextFile();

  while (entry.isDirectory()) {
    entry.close();
    entry = root.openNextFile();
  }

  if (!entry) {
    ESP_LOGE(TAG, "No files found");
    root.close();
    return "";
  }

  std::string result = createFilename(entry.name());
  root.close();
  entry.close();

  return result;
}

std::string SDLogger::readFile(const char *filename) {
  String parameter;

  if (strncmp(filename, "/", 1) != 0) {
    ESP_LOGE(TAG, "Invalid filename");
    return "";
  }

  File file = SD.open(filename);
  if (!file) {
    ESP_LOGE(TAG, "Failed to open file");
    return "";
  }

  while (file.available()) {
    char c = file.read();
    if (isPrintable(c)) {
      parameter.concat(c);
    }
  }
  file.close();

  ESP_LOGI(TAG, "%s", parameter.c_str());
  return std::string(parameter.c_str());
}

void SDLogger::deleteFile(const char *filename) {
  if (strncmp(filename, "/", 1) == 0) {
    File file = SD.open(filename);
    if (file) {
      file.close();
      if (SD.remove(filename)) {
        ESP_LOGI(TAG, "File deleted: %s", filename);
      } else {
        ESP_LOGE(TAG, "Failed to delete: %s", filename);
      }
    } else {
      ESP_LOGE(TAG, "Failed to open file for deletion: %s", filename);
    }
  }
}

}  // namespace sd_logger
}  // namespace esphome
