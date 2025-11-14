#ifndef SESSION_RECORDER_H
#define SESSION_RECORDER_H

#include <sndfile.hh>

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// Use double as sample type to match softcut
using sample_t = double;

class SessionRecorder {
 public:
  SessionRecorder();
  ~SessionRecorder();

  // Start recording with timestamp-based filenames
  void startRecording(int numVoices, float sampleRate);

  // Stop recording and close files
  void stopRecording();

  // Check if currently recording
  bool isRecording() const { return recording_.load(); }

  // Called from audio thread - captures audio frames
  void captureMainMix(const sample_t* left, const sample_t* right,
                      size_t numFrames);
  void captureVoice(int voice, const sample_t* left, const sample_t* right,
                    size_t numFrames);

 private:
  std::atomic<bool> recording_;
  std::string sessionTimestamp_;
  int numVoices_;
  float sampleRate_;

  // Ring buffers for thread-safe audio capture
  static const size_t RING_BUFFER_SIZE = 48000 * 10;  // 10 seconds at 48kHz
  struct VoiceBuffer {
    std::vector<float> buffer;
    std::atomic<size_t> writePos;
    std::atomic<size_t> readPos;
    std::unique_ptr<SndfileHandle> file;
    std::atomic<bool> hasAudio;

    VoiceBuffer() : writePos(0), readPos(0), hasAudio(false) {}
    VoiceBuffer(VoiceBuffer&& other) noexcept
        : buffer(std::move(other.buffer)),
          writePos(other.writePos.load()),
          readPos(other.readPos.load()),
          file(std::move(other.file)),
          hasAudio(other.hasAudio.load()) {}
    VoiceBuffer& operator=(VoiceBuffer&& other) noexcept {
      if (this != &other) {
        buffer = std::move(other.buffer);
        writePos.store(other.writePos.load());
        readPos.store(other.readPos.load());
        file = std::move(other.file);
        hasAudio.store(other.hasAudio.load());
      }
      return *this;
    }
    // Delete copy constructor and assignment
    VoiceBuffer(const VoiceBuffer&) = delete;
    VoiceBuffer& operator=(const VoiceBuffer&) = delete;
  };

  VoiceBuffer mainMixBuffer_;
  std::vector<VoiceBuffer> voiceBuffers_;

  // Worker thread for writing to disk
  std::unique_ptr<std::thread> writerThread_;
  std::atomic<bool> writerRunning_;

  void writerThreadFunc();
  std::string generateTimestamp();
  std::string generateFilename(const std::string& voiceId);
  void initializeBuffer(VoiceBuffer& buffer, int channels);
  size_t writeAvailableData(VoiceBuffer& buffer);
};

#endif  // SESSION_RECORDER_H
