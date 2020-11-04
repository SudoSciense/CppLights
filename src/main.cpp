#include <iostream>
#include <string>
#include <curl/curl.h>
#include <unistd.h>
#include <json/json.h>

// The main of this program updates status by polling the Hue REST API
// and printing out based on changes. Not sure if polling like this is
// standard practice and if it is, a sensible polling frequency to use.

class CppLights
{

private:

    void initialize()
    {
        std::cout 
            << "Initializing..." 
            << std::endl;
    
        URL = std::getenv("CPPLIGHTS_URL");
        PORT = std::stoi(std::getenv("CPPLIGHTS_PORT"));
        user = std::getenv("CPPLIGHTS_USER");
        pollTime = std::stoi(std::getenv("CPPLIGHTS_POLLTIME_MICROSECS"));
        numLights = 0;
    }

public:

    std::string URL;
    std::string user;
    int PORT;
    int numLights;
    int pollTime;
    bool running;
    std::map<std::string, Json::Value> lightsMap;

    CppLights()
    {
        initialize();
    }

    std::string getUrl()
    {
        return URL;
    }

    int getPort()
    {
        return PORT;
    }

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void getNumLights()
    {
        CURL *curl;
        CURLcode res;
        std::string readBuffer;
        std::string getAllLightsUrl;

        Json::Value value;
        Json::Reader reader;
        Json::StyledWriter styledWriter;

        // Build URL
        getAllLightsUrl.append(URL);
        getAllLightsUrl.append(std::to_string(PORT));
        getAllLightsUrl.append("/api/");
        getAllLightsUrl.append(user);
        getAllLightsUrl.append("/lights");

        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, getAllLightsUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);

            bool parsingSuccessful = reader.parse(readBuffer, value);
            if (parsingSuccessful)
            {
                numLights = value.size();
            }
            curl_easy_cleanup(curl);
        }
    }

    // This is the actual run loop.
    void pollHueStatus()
    {
        running = true;
        unsigned int sleep_microseconds = pollTime;

        while (running)
        {
            updateLightArray();
            usleep(sleep_microseconds);
        }
    }

    bool updateLightArray()
    {
        bool isChanged = false;
        CURL *curl;
        CURLcode res;
        Json::Reader reader;
        std::string lightsUrl = "";
        std::string readBuffer;
        
        // Build URL
        lightsUrl.append(URL);
        lightsUrl.append(std::to_string(PORT));
        lightsUrl.append("/api/");
        lightsUrl.append(user);
        lightsUrl.append("/lights/");

        curl = curl_easy_init();
        if (curl)
        {
            for (int i=1; i <= numLights; i++)
            {
                Json::Value value;
                readBuffer.erase();
                curl_easy_setopt(curl, CURLOPT_URL, (lightsUrl + std::to_string(i)).c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
                res = curl_easy_perform(curl);

                bool parsingSuccessful = reader.parse(readBuffer, value);
                if (parsingSuccessful)
                {
                    int brightness = std::stoi(value["state"]["bri"].asString());
                    Json::Value light;

                    // Don't know what the limitations of the brightness value in the simulator are.
                    // I entered several values and it was still accepting state updates with 
                    // very very large numbers. I guess I'll just restrict the numbers between 
                    // 0 and 100.
                    if (brightness < 0 )
                    {
                        brightness = 0;
                    }
                    else if (brightness > 100 )
                    {
                        brightness = 100;
                    }
                    light["name"] = value["name"];
                    light["id"] = std::to_string(i);
                    light["on"] = value["state"]["on"];
                    light["brightness"] = std::to_string(brightness);

                    // While we have fresh data, might as well do a quick
                    // comparison to alert if the state has changed and 
                    // which attribute was affected.
                    checkForStateChanges(light);

                    // Update state map.
                    lightsMap[std::to_string(i)] = light;
                }
            }
        }
        curl_easy_cleanup(curl);
        return isChanged;
    }

    // Pretty print all the current lights.
    void printAllLightsState()
    {
        std::map<std::string, Json::Value>::iterator it;
        Json::Value jsonArray;
        for ( it = lightsMap.begin(); it != lightsMap.end(); it++ )
        {
            jsonArray.append(it->second);
        }
        std::cout << jsonArray.toStyledString() 
                    << std::endl;
    }

    // See if any of these attributes have seen changes.
    void checkForStateChanges(const Json::Value newestData)
    {
        Json::Value change;
        std::string id = newestData["id"].asString();

        // Make sure this has actually been set before.
        if (!lightsMap[id].empty())
        {
            if (lightsMap[id]["name"] != newestData["name"])
            {
                change["id"] = id;
                change["name"] = newestData["name"];
                std::cout << change.toStyledString() 
                        << std::endl;
                change.clear();
            }
            if (lightsMap[id]["on"] != newestData["on"])
            {
                change["id"] = id;
                change["on"] = newestData["on"];
                std::cout << change.toStyledString() 
                        << std::endl;
                change.clear();
            }
            if (lightsMap[id]["brightness"] != newestData["brightness"])
            {
                change["id"] = id;
                change["brightness"] = newestData["brightness"];
                std::cout << change.toStyledString() 
                        << std::endl;
                change.clear();
            }
        }
    }
};


int main(void)
{
  bool running = true;
  CppLights *cppLights = new CppLights();
  cppLights->getNumLights();
  cppLights->updateLightArray();
  cppLights->printAllLightsState();
  cppLights->pollHueStatus();
  return 0;
}
