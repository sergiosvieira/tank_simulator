#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

class TransferManager {
public:
  // Default bandwidth in bytes per second (e.g., 4 MB/s)
  static constexpr double DEFAULT_BANDWIDTH = 4.0 * 1024 * 1024;

  static double calculate_transfer_time(double size_bytes,
                                        double bandwidth = DEFAULT_BANDWIDTH) {
    if (bandwidth <= 0)
      return 0.0;
    return size_bytes / bandwidth;
  }
};

#endif // TRANSFERMANAGER_H
