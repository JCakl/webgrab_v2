//
//  graber.cpp
//  netApp
//
//  Created by Jan Cakl on 06.03.2022.
//

#include "graber.hpp"
#include <iostream>
#include "curl.h"

#define MIN 100
#define MAX 999

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

bool grabUrl( std::string urlAdress )
{
    // download file
    FILE *fp;
    CURLcode res;
    char *url = const_cast<char*>(urlAdress.c_str());
    char outfilename[ sizeof(urlAdress) + 1];
    
    std::string fileName = "output_";
    
    int output = MIN + (rand() % static_cast<int>(MAX - MIN + 1));
    
    fileName += std::to_string(output) + ".out";
    
    strcpy(outfilename, fileName.c_str());
                
    CURL *curl = curl_easy_init();
    if (curl){
        fp = fopen(outfilename,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        
        res = curl_easy_perform(curl);
                
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
        
        switch (res) {
            case CURLE_OK:
                std::cout << "All fine. Proceed as usual." << std::endl;
                return true;
                break;

            case CURLE_UNSUPPORTED_PROTOCOL:
                std::cout << "The URL you passed to libcurl used a protocol that this libcurl does not support." << std::endl;
                break;
            case CURLE_FAILED_INIT:
                std::cout << "Early initialization code failed." << std::endl;
                break;
            case CURLE_URL_MALFORMAT:
                std::cout << "The URL was not properly formatted.";
                break;
                
            case CURLE_COULDNT_RESOLVE_HOST:
                std::cout << "Couldn't resolve host. The given remote host was not resolved." << std::endl;
                break;
                            
            default:
                std::cout << "[CURL] Unhandled error " << std::endl;
                break;
        }
    }
    return false;
}
