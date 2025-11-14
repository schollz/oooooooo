#include "SessionRecorder.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

SessionRecorder::SessionRecorder() : recording_(false), writerRunning_(false) {}

SessionRecorder::~SessionRecorder() {
  if (recording_.load()) {
    stopRecording();
  }
}

std::string SessionRecorder::generateTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  std::tm* tm_now = std::localtime(&time_t_now);

  std::ostringstream oss;
  oss << std::put_time(tm_now, "%Y%m%d-%H%M");
  return oss.str();
}

std::string SessionRecorder::generateFilename(const std::string& voiceId) {
  std::string folder = "oooooooo";
  if (!std::filesystem::exists(folder)) {
    std::filesystem::create_directory(folder);
  }
  return folder + "/oooooooo_" + sessionTimestamp_ + "_loop_" + voiceId +
         ".wav";
}

void SessionRecorder::initializeBuffer(VoiceBuffer& buffer, int channels) {
  buffer.buffer.resize(RING_BUFFER_SIZE * channels);
  buffer.writePos.store(0);
  buffer.readPos.store(0);
  buffer.hasAudio.store(false);
}

void SessionRecorder::startRecording(int numVoices, float sampleRate) {
  if (recording_.load()) {
    return;  // Already recording
  }

  numVoices_ = numVoices;
  sampleRate_ = sampleRate;
  sessionTimestamp_ = generateTimestamp();

  std::cout << "Starting session recording: " << sessionTimestamp_ << std::endl;

  // Initialize main mix buffer (stereo)
  initializeBuffer(mainMixBuffer_, 2);
  std::string mainFilename = generateFilename("all");
  mainMixBuffer_.file = std::make_unique<SndfileHandle>(
      mainFilename, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, 2,
      static_cast<int>(sampleRate_));

  if (mainMixBuffer_.file->error()) {
    std::cerr << "Error creating main mix file: "
              << mainMixBuffer_.file->strError() << std::endl;
    return;
  }

  // Initialize voice buffers (stereo)
  voiceBuffers_.resize(numVoices_);
  for (int i = 0; i < numVoices_; i++) {
    initializeBuffer(voiceBuffers_[i], 2);
    std::string voiceFilename = generateFilename(std::to_string(i));
    voiceBuffers_[i].file = std::make_unique<SndfileHandle>(
        voiceFilename, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, 2,
        static_cast<int>(sampleRate_));

    if (voiceBuffers_[i].file->error()) {
      std::cerr << "Error creating voice " << i
                << " file: " << voiceBuffers_[i].file->strError() << std::endl;
    }
  }

  recording_.store(true);
  writerRunning_.store(true);

  // Start writer thread
  writerThread_ =
      std::make_unique<std::thread>(&SessionRecorder::writerThreadFunc, this);

  std::cout << "Session recording started" << std::endl;
}

void SessionRecorder::stopRecording() {
  if (!recording_.load()) {
    return;
  }

  std::cout << "Stopping session recording..." << std::endl;

  recording_.store(false);
  writerRunning_.store(false);

  // Wait for writer thread to finish
  if (writerThread_ && writerThread_->joinable()) {
    writerThread_->join();
  }

  // Flush remaining data and close files
  writeAvailableData(mainMixBuffer_);
  mainMixBuffer_.file.reset();

  for (auto& voiceBuffer : voiceBuffers_) {
    if (voiceBuffer.hasAudio.load()) {
      writeAvailableData(voiceBuffer);
    }
    voiceBuffer.file.reset();
  }

  // Delete files for voices that had no audio
  for (int i = 0; i < numVoices_; i++) {
    if (!voiceBuffers_[i].hasAudio.load()) {
      std::string filename = generateFilename(std::to_string(i));
      std::filesystem::remove(filename);
    }
  }

  voiceBuffers_.clear();

  std::cout << "Session recording stopped" << std::endl;
}

void SessionRecorder::captureMainMix(const sample_t* left, const sample_t* right,
                                     size_t numFrames) {
  if (!recording_.load()) {
    return;
  }

  size_t writePos = mainMixBuffer_.writePos.load(std::memory_order_relaxed);

  for (size_t i = 0; i < numFrames; i++) {
    size_t idx = (writePos * 2) % mainMixBuffer_.buffer.size();
    mainMixBuffer_.buffer[idx] = static_cast<float>(left[i]);
    mainMixBuffer_.buffer[idx + 1] = static_cast<float>(right[i]);
    writePos++;
  }

  mainMixBuffer_.writePos.store(writePos, std::memory_order_release);
  mainMixBuffer_.hasAudio.store(true);
}

void SessionRecorder::captureVoice(int voice, const sample_t* left,
                                   const sample_t* right, size_t numFrames) {
  if (!recording_.load() || voice < 0 || voice >= numVoices_) {
    return;
  }

  auto& voiceBuffer = voiceBuffers_[voice];
  size_t writePos = voiceBuffer.writePos.load(std::memory_order_relaxed);

  // Check if there's actual audio (not just silence)
  bool hasSignal = false;
  for (size_t i = 0; i < numFrames; i++) {
    float leftSample = static_cast<float>(left[i]);
    float rightSample = static_cast<float>(right[i]);
    if (std::abs(leftSample) > 0.0001f || std::abs(rightSample) > 0.0001f) {
      hasSignal = true;
    }
    size_t idx = (writePos * 2) % voiceBuffer.buffer.size();
    voiceBuffer.buffer[idx] = leftSample;
    voiceBuffer.buffer[idx + 1] = rightSample;
    writePos++;
  }

  voiceBuffer.writePos.store(writePos, std::memory_order_release);
  if (hasSignal) {
    voiceBuffer.hasAudio.store(true);
  }
}

size_t SessionRecorder::writeAvailableData(VoiceBuffer& buffer) {
  size_t writePos = buffer.writePos.load(std::memory_order_acquire);
  size_t readPos = buffer.readPos.load(std::memory_order_relaxed);

  if (writePos == readPos) {
    return 0;  // No data to write
  }

  size_t available = writePos - readPos;
  size_t bufferSize = buffer.buffer.size();
  size_t channels = buffer.file->channels();

  // Write in chunks to handle ring buffer wraparound
  size_t totalWritten = 0;
  while (available > 0) {
    size_t startIdx = (readPos * channels) % bufferSize;
    size_t endIdx = bufferSize;
    size_t chunkFrames = std::min(available, (endIdx - startIdx) / channels);

    sf_count_t written =
        buffer.file->write(&buffer.buffer[startIdx], chunkFrames * channels);

    if (written <= 0) {
      std::cerr << "Error writing to file: " << buffer.file->strError()
                << std::endl;
      break;
    }

    size_t framesWritten = static_cast<size_t>(written) / channels;
    readPos += framesWritten;
    available -= framesWritten;
    totalWritten += framesWritten;
  }

  buffer.readPos.store(readPos, std::memory_order_release);

  if (totalWritten > 0) {
    std::cout << "Wrote " << totalWritten << " frames" << std::endl;
  }

  return totalWritten;
}

void SessionRecorder::writerThreadFunc() {
  std::cout << "Writer thread started" << std::endl;

  while (writerRunning_.load()) {
    // Write main mix
    writeAvailableData(mainMixBuffer_);

    // Write voices
    for (auto& voiceBuffer : voiceBuffers_) {
      if (voiceBuffer.hasAudio.load()) {
        writeAvailableData(voiceBuffer);
      }
    }

    // Sleep to avoid busy-waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "Writer thread stopped" << std::endl;
}
