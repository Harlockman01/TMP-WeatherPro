#pragma once

#include "DataDef.h"

#include <array>

class DataProviderOpenMeteo : public DataProvider
{
public:
    enum class UnitType
    {
        Metric,
        Imperial,
    };

    struct Config
    {
        UnitType unit_type{ UnitType::Metric };
        // Open-Meteo is free, no API key needed
    };

    struct RealtimeWeather
    {
        std::string temp;
        std::string temp_feels_like;
        std::string weather_text;
        std::string weather_code;
        std::string humidity;
        std::string wind_speed;
        std::string wind_direction;
        std::string precipitation;
        std::string is_day;
        std::string update_time;

        UnitType unit_type{ UnitType::Metric };
    };

    struct RealtimeAirQuality
    {
        std::string aqi;
        std::string pm2p5;
        std::string pm10;
    };

    struct ForecastedWeather
    {
        std::string weather_text_day;
        std::string weather_code_day;
        std::string temp_max;
        std::string temp_min;
        std::string humidity;
        std::string uv_index;
        std::string precipitation_probability;

        UnitType unit_type{ UnitType::Metric };
    };

    struct WeatherDataBlock : WeatherData
    {
        [[nodiscard]] std::string getWeatherSummary() const override;
        [[nodiscard]] std::string getWeatherItem(WeatherTimeSlot time_slot, WeatherItem item) const override;

        RealtimeWeather realtime_weather;
        RealtimeAirQuality realtime_air;
        std::array<ForecastedWeather, 3> fc_weather_3d;
    };

    bool geocodingDirect(const std::string &query, Locations &queried_locations) const override;
    bool geocodingReverse(const std::string &latitude, const std::string &longitude, Locations &queried_locations) const override;

    [[nodiscard]] WeatherDataCPtr getWeatherData(const Location &loc) const override;

    Config config;
};
