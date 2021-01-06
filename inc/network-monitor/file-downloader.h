#ifndef _FILE_DOWNLOADER_H
#define _FILE_DOWNLOADER_H

#include <filesystem>
#include <string>

namespace NetworkMonitor {
bool DownloadFile(
    const std::string& fileUrl,
    const std::filesystem::path& destination,
    const std::filesystem::path& caCertFile = {}
);
}
#endif
