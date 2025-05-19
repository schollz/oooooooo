//
// Created by emb on 1/20/20.
//

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "BufDiskWorker.h"
#include "Display.h"
#include "OscInterface.h"
#include "SoftcutClient.h"

// Global variables for shutdown coordination
static std::unique_ptr<Display> g_display;
static std::unique_ptr<softcut_jack_osc::SoftcutClient> g_sc;
static bool g_shouldQuit = false;
static std::mutex g_shutdownMutex;  // To protect shutdown sequence

static inline void sleep(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Central signal handler
static void signalHandler(int sig) {
  std::cout << "Received signal " << sig << ", initiating shutdown..."
            << std::endl;

  // Lock to ensure exclusive access during shutdown
  std::lock_guard<std::mutex> lock(g_shutdownMutex);

  // Set quit flag to exit main loop
  g_shouldQuit = true;

  // Request display shutdown
  if (g_display) {
    g_display->requestShutdown();
  }

  // create a thread timer to terminate everything in two seconds
  std::thread([&]() {
    sleep(1000);
    std::cout << "Force quitting..." << std::endl;
    exit(1);
  }).detach();
}

// Function to set the quit flag when window is closed
static void onDisplayQuit() {
  std::cout << "Display window closed, initiating shutdown..." << std::endl;

  // Lock to ensure exclusive access during shutdown
  std::lock_guard<std::mutex> lock(g_shutdownMutex);

  // Set quit flag to exit main loop
  g_shouldQuit = true;
}

int main() {
  using namespace softcut_jack_osc;

  // Set up signal handling
  std::signal(SIGINT, signalHandler);
  std::signal(SIGTERM, signalHandler);

  try {
    // Initialize SoftcutClient
    g_sc = std::make_unique<SoftcutClient>();
    g_sc->setup();
    BufDiskWorker::init(static_cast<float>(g_sc->getSampleRate()));
    g_sc->start();
    g_sc->init();
    g_sc->connectAdcPorts();
    g_sc->connectDacPorts();

    // Initialize OSC Interface
    OscInterface::init(g_sc.get());

    // Create and start the display
    g_display = std::make_unique<Display>(800, 600);
    g_display->init(g_sc.get(), SoftcutClient::NumVoices);
    g_display->setQuitCallback(onDisplayQuit);
    g_display->start();

    std::cout << "Entering main loop..." << std::endl;

    // Setup loops

    // Main application loop
    while (true) {
      {
        // Check if we should quit with mutex protection
        std::lock_guard<std::mutex> lock(g_shutdownMutex);
        if (g_shouldQuit || OscInterface::shouldQuit()) {
          break;
        }
      }
      sleep(100);
    }

    std::cout << "Exiting main loop..." << std::endl;

    // Critical cleanup order:
    // 1. First stop the display - make sure this happens BEFORE cleaning up
    // OscInterface
    std::cout << "Stopping display..." << std::endl;
    if (g_display) {
      g_display->stop();
      g_display.reset();
    }

    // 2. Deinitialize OSC Interface
    std::cout << "Cleaning up OSC Interface..." << std::endl;
    OscInterface::deinit();

    // 3. Finally stop SoftcutClient
    std::cout << "Stopping SoftcutClient..." << std::endl;
    if (g_sc) {
      g_sc->stop();
      g_sc->cleanup();
      g_sc.reset();
    }

    std::cout << "Cleanup complete" << std::endl;
    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception!" << std::endl;
    return 1;
  }
}