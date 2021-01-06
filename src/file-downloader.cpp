#include <network-monitor/file-downloader.h>
#include <curl/curl.h>

#include <filesystem>
#include <string>

bool NetworkMonitor::DownloadFile(
    const std::string& fileUrl,
    const std::filesystem::path& destination,
    const std::filesystem::path& caCertFile
)
{
    CURL* curl {curl_easy_init()};
    if (!curl)
    {
        return false;
    }
    // Configure curl to use secure connection over TLS
	curl_easy_setopt(curl, CURLOPT_URL, fileUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_CAINFO, caCertFile.c_str());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
	
	// open destination file
	std::FILE* fp {fopen(destination.c_str(), "wb")};
	if (!fp)
	{
		// free any resources used behind the scenes
		curl_easy_cleanup(curl);
        return false;    
	}
    // tell curl where to write data
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    // perform the actual request/download
    CURLcode result = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    // close file
    fclose(fp);    

	return true; 
}
