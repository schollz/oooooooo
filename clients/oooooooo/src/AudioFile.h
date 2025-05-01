#ifndef AUDIO_FILE_H
#define AUDIO_FILE_H

#include <sndfile.h>

#include <cstring>
#include <string>

class AudioFile {
 public:
  // Constructor takes a file path
  AudioFile(const std::string& filePath);

  // Destructor to clean up resources
  ~AudioFile();

  // Check if file loaded successfully
  bool isValid() const;

  // Get error message if loading failed
  std::string getError() const;

  // Get sample rate of the audio file
  int getSampleRate() const;

  // Get number of channels in the audio file
  int getChannelCount() const;

  // Get total number of frames
  sf_count_t getFrameCount() const;

 private:
  SNDFILE* file;         // Handle to the sound file
  SF_INFO fileInfo;      // File information structure
  std::string errorMsg;  // Storage for error message
};

#endif  // AUDIO_FILE_H