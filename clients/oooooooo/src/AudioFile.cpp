#include "AudioFile.h"

AudioFile::AudioFile(const std::string& filePath) : file(nullptr) {
  // Initialize fileInfo structure
  memset(&fileInfo, 0, sizeof(fileInfo));

  // Try to open the audio file
  file = sf_open(filePath.c_str(), SFM_READ, &fileInfo);

  // Check if file opened successfully
  if (!file) {
    // Store the error message
    errorMsg = sf_strerror(nullptr);
  }
}

AudioFile::~AudioFile() {
  // Close the file if it's open
  if (file) {
    sf_close(file);
    file = nullptr;
  }
}

bool AudioFile::isValid() const { return file != nullptr; }

std::string AudioFile::getError() const { return errorMsg; }

int AudioFile::getSampleRate() const {
  if (!isValid()) {
    return 0;
  }
  return fileInfo.samplerate;
}

int AudioFile::getChannelCount() const {
  if (!isValid()) {
    return 0;
  }
  return fileInfo.channels;
}

sf_count_t AudioFile::getFrameCount() const {
  if (!isValid()) {
    return 0;
  }
  return fileInfo.frames;
}