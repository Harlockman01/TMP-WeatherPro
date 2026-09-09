#pragma once

#include "WeatherApiWCCS.h"
#include "WeatherApiQWeather.h"
#include "WeatherApiOpenWeather.h"
#include "WeatherApiOpenMeteo.h"

class ApiCollections
{
public:
    WeatherApi& GetApi(ApiType api_type);
    WapiWCCS& GetApiWCCS();
    WapiQWeather& GetApiQWeather();
    WapiOpenWeather& GetApiOpenWeather();
    WapiOpenMeteo& GetApiOpenMeteo();

    void LoadConfigs(const CSimpleIniW &ini_file);
    void SaveConfigs(CSimpleIniW &ini_file) const;

private:
    WapiWCCS wapi_wccs_;
    WapiQWeather wapi_qw_;
    WapiOpenWeather wapi_ow_;
    WapiOpenMeteo wapi_om_;
};
