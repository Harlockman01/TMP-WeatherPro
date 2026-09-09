#pragma once

#include "WeatherAPI.h"

#include <WPCore/DataProviderOpenMeteo.h>

class WapiOpenMeteo final : public WeatherApi
{
public:
    enum class IconStyle
    {
        Default,
    };

    struct Config
    {
        // Open-Meteo is free, no API key. We keep config for unit type.
        DataProviderOpenMeteo::UnitType unit_type{ DataProviderOpenMeteo::UnitType::Metric };
    };

    const IconSheet* GetWeatherIcons() const override;
    int GetWeatherIconIndex(std::wstring_view weather_code) const override;
    DataProvider& GetProvider() override;
    const DataProvider& GetProvider() const override;
    void LoadConfig(const CSimpleIniW &ini_file) override;
    void SaveConfig(CSimpleIniW &ini) const override;

    DataProviderOpenMeteo& GetProviderOpenMeteo();
    const DataProviderOpenMeteo& GetProviderOpenMeteo() const;

    Config config;

private:
    DataProviderOpenMeteo provider_;
};
