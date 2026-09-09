#include "pch.h"

#include "ApiCollections.h"

WeatherApi& ApiCollections::GetApi(ApiType api_type) {
    switch (api_type) {
    case ApiType::QWeather:
        return wapi_qw_;

    case ApiType::OpenWeather:
        return wapi_ow_;

    case ApiType::OpenMeteo:
        return wapi_om_;

    case ApiType::WeatherComCnSpider:
        return wapi_wccs_;
    }

    return wapi_wccs_;
}

WapiWCCS& ApiCollections::GetApiWCCS() {
    return wapi_wccs_;
}

WapiQWeather& ApiCollections::GetApiQWeather() {
    return wapi_qw_;
}

WapiOpenWeather& ApiCollections::GetApiOpenWeather() {
    return wapi_ow_;
}

WapiOpenMeteo& ApiCollections::GetApiOpenMeteo() {
    return wapi_om_;
}

void ApiCollections::LoadConfigs(const CSimpleIniW &ini_file) {
    for (const std::array<WeatherApi*, 4> api_ptrs{ &wapi_wccs_, &wapi_qw_, &wapi_ow_, &wapi_om_ };
         auto* api : api_ptrs) {
        api->LoadConfig(ini_file);
    }
}

void ApiCollections::SaveConfigs(CSimpleIniW &ini_file) const {
    for (const std::array<const WeatherApi*, 4> api_ptrs{ &wapi_wccs_, &wapi_qw_, &wapi_ow_, &wapi_om_ };
         auto* api : api_ptrs) {
        api->SaveConfig(ini_file);
    }
}
