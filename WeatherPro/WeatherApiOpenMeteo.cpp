#include "pch.h"
#include "WeatherApiOpenMeteo.h"
#include "Common.h"

#include <WPCore/DataProviderOpenMeteo.h>

namespace
{
    constexpr std::wstring_view WSV_OPEN_METEO{ L"open_meteo" };
    constexpr std::wstring_view WSV_UNIT_TYPE{ L"unit_type" };
    constexpr std::wstring_view WSV_METRIC{ L"metric" };
    constexpr std::wstring_view WSV_IMPERIAL{ L"imperial" };

    DataProviderOpenMeteo::UnitType ParseUnitType(std::wstring_view str_unit_type) {
        if (str_unit_type == WSV_IMPERIAL) {
            return DataProviderOpenMeteo::UnitType::Imperial;
        } else {
            return DataProviderOpenMeteo::UnitType::Metric;    // default
        }
    }

    const wchar_t* ToCharacters(DataProviderOpenMeteo::UnitType unit_type) {
        switch (unit_type) {
        case DataProviderOpenMeteo::UnitType::Imperial:
            return WSV_IMPERIAL.data();

        case DataProviderOpenMeteo::UnitType::Metric:
        default:
            return WSV_METRIC.data();
        }
    }
}

const IconSheet* WapiOpenMeteo::GetWeatherIcons() const {
    // Open-Meteo uses the same icon set as OpenWeather (mapped via WMO codes)
    return IconManager::Instance().GetIconSheet(IconResType::OpenWeather);
}

int WapiOpenMeteo::GetWeatherIconIndex(std::wstring_view weather_code) const {
    // Reuse OpenWeather's icon mapping - WMO codes are converted to OW-style icons
    // (e.g. "01d", "02n", "10d") in DataProviderOpenMeteo.
    static const std::unordered_map<std::wstring_view, int> code_index_map{
        {L"01d", 0},
        {L"02d", 2},
        {L"03d", 4},
        {L"04d", 5},
        {L"09d", 6},
        {L"10d", 7},
        {L"11d", 9},
        {L"13d", 10},
        {L"50d", 11},
        {L"01n", 1},
        {L"02n", 3},
        {L"03n", 4},
        {L"04n", 5},
        {L"09n", 6},
        {L"10n", 8},
        {L"11n", 9},
        {L"13n", 10},
        {L"50n", 11},
    };

    int idx{ 12 };

    if (code_index_map.contains(weather_code)) {
        idx = code_index_map.at(weather_code);
    }

    return idx;
}

DataProvider& WapiOpenMeteo::GetProvider() {
    return provider_;
}

const DataProvider& WapiOpenMeteo::GetProvider() const {
    return provider_;
}

DataProviderOpenMeteo& WapiOpenMeteo::GetProviderOpenMeteo() {
    return provider_;
}

const DataProviderOpenMeteo& WapiOpenMeteo::GetProviderOpenMeteo() const {
    return provider_;
}

void WapiOpenMeteo::LoadConfig(const CSimpleIniW &ini_file) {
    const cmn::IniLoader ini_helper(ini_file, WSV_OPEN_METEO);
    config.unit_type = ParseUnitType(ini_helper.GetValueW(WSV_UNIT_TYPE, WSV_METRIC));
    provider_.config.unit_type = config.unit_type;
}

void WapiOpenMeteo::SaveConfig(CSimpleIniW &ini_file) const {
    const cmn::IniSaver ini_helper(ini_file, WSV_OPEN_METEO);
    ini_helper.SetValueW(WSV_UNIT_TYPE, ToCharacters(config.unit_type));
}
