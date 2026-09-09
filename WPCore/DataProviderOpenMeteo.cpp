#include "DataProviderOpenMeteo.h"
#include "AppLocale.h"
#include "Logger.h"
#include "utils.h"
#include "JsonValueHelper.h"

#include <functional>
#include <future>

#include <yyjson.h>

namespace
{
    using UnitType = DataProviderOpenMeteo::UnitType;
    using Config = DataProviderOpenMeteo::Config;
    using RealtimeWeather = DataProviderOpenMeteo::RealtimeWeather;
    using RealtimeAirQuality = DataProviderOpenMeteo::RealtimeAirQuality;
    using ForecastedWeather = DataProviderOpenMeteo::ForecastedWeather;

    // Open-Meteo uses WMO weather interpretation codes
    // https://open-meteo.com/en/docs#weathervariables
    struct WmoCodeText
    {
        int code;
        std::string_view text_es;
        std::string_view text_en;
        std::string_view icon_code;    // mapped to OpenWeather-like icon code
    };

    constexpr std::array<WmoCodeText, 27> WMO_TABLE = { {
        {0,  "Despejado",                          "Clear sky",                   "01"},
        {1,  "Mayormente despejado",                "Mainly clear",                "02"},
        {2,  "Parcialmente nublado",               "Partly cloudy",               "02"},
        {3,  "Nublado",                            "Overcast",                    "04"},
        {45, "Niebla",                             "Fog",                         "50"},
        {48, "Niebla con escarcha",                "Depositing rime fog",         "50"},
        {51, "Llovizna ligera",                    "Light drizzle",               "09"},
        {53, "Llovizna moderada",                  "Moderate drizzle",            "09"},
        {55, "Llovizna densa",                     "Dense drizzle",               "09"},
        {56, "Llovizna congelada ligera",          "Light freezing drizzle",      "09"},
        {57, "Llovizna congelada densa",           "Dense freezing drizzle",      "09"},
        {61, "Lluvia ligera",                      "Slight rain",                 "10"},
        {63, "Lluvia moderada",                    "Moderate rain",               "10"},
        {65, "Lluvia fuerte",                      "Heavy rain",                  "10"},
        {66, "Lluvia congelada ligera",            "Light freezing rain",         "10"},
        {67, "Lluvia congelada fuerte",            "Heavy freezing rain",         "10"},
        {71, "Nevada ligera",                      "Slight snow fall",            "13"},
        {73, "Nevada moderada",                    "Moderate snow fall",          "13"},
        {75, "Nevada fuerte",                      "Heavy snow fall",             "13"},
        {77, "Granos de nieve",                    "Snow grains",                 "13"},
        {80, "Chubascos ligeros",                  "Slight rain showers",         "09"},
        {81, "Chubascos moderados",                "Moderate rain showers",       "09"},
        {82, "Chubascos violentos",                "Violent rain showers",        "09"},
        {85, "Chubascos de nieve ligeros",         "Slight snow showers",         "13"},
        {86, "Chubascos de nieve fuertes",         "Heavy snow showers",          "13"},
        {95, "Tormenta",                           "Thunderstorm",                "11"},
        {96, "Tormenta con granizo ligero",        "Thunderstorm with slight hail","11"},
        {99, "Tormenta con granizo fuerte",        "Thunderstorm with heavy hail","11"},
    } };

    std::string wmoCodeToText(int code) {
        const auto is_spanish = tr::getLocale() == tr::Locale::SPANISH;
        for (const auto &item : WMO_TABLE) {
            if (item.code == code) {
                return std::string{ is_spanish ? item.text_es : item.text_en };
            }
        }
        return is_spanish ? "Desconocido" : "Unknown";
    }

    std::string wmoCodeToIcon(int code, bool is_day) {
        std::string_view base = "01";
        for (const auto &item : WMO_TABLE) {
            if (item.code == code) {
                base = item.icon_code;
                break;
            }
        }

        std::string result{ base };
        result += (is_day ? "d" : "n");
        return result;
    }

    const char* getTemperatureUnit(UnitType ut) {
        switch (ut) {
            case UnitType::Imperial:  return "°F";
            case UnitType::Metric:
            default:                  return "°C";
        }
    }

    const char* getSpeedUnit(UnitType ut) {
        switch (ut) {
            case UnitType::Imperial:  return "mph";
            case UnitType::Metric:
            default:                  return "km/h";
        }
    }

    const char* getTempUnitParam(UnitType ut) {
        switch (ut) {
            case UnitType::Imperial:  return "fahrenheit";
            case UnitType::Metric:
            default:                  return "celsius";
        }
    }

    const char* getSpeedUnitParam(UnitType ut) {
        switch (ut) {
            case UnitType::Imperial:  return "mph";
            case UnitType::Metric:
            default:                  return "kmh";
        }
    }

    const std::string& getWindDirectionText(int deg) {
        const static std::array<std::string, 8> directions = {
            "N", "NE", "E", "SE", "S", "SO", "O", "NO"
        };

        const int normalized = ((deg % 360) + 360) % 360;
        const int index = static_cast<int>((normalized + 22.5) / 45.0) % 8;

        return directions[index];
    }

    std::string jsonGetNumberAsStr1dp(yyjson_val *j_val, const char *key) {
        auto *j_item = yyjson_obj_get(j_val, key);
        if (j_item == nullptr || !yyjson_is_num(j_item)) {
            return {};
        }

        const auto num = yyjson_get_num(j_item);
        if (auto rounded = std::round(num * 10.0) / 10.0;
            std::abs(rounded - std::round(rounded)) < 1e-9) {
            return std::format("{:.0f}", rounded);
        } else {
            return std::format("{:.1f}", rounded);
        }
    }

    bool queryFrame(std::string_view host, std::string_view path, const utils::HttpParams &params,
                    const std::function<bool(yyjson_val*)> &func) {
        std::string content;
        auto status_code = utils::internetGetWithRetry(std::string{ host }, std::string{ path }, content, params);

        if (content.empty()) {
            Logger::instance().error(
                std::format("[{}] {}", status_code, tr::txt(tr::TID::ERR_INTERNET_EMPTY_RESPONSE)));
            return false;
        }

        std::unique_ptr<yyjson_doc, void(*)(yyjson_doc*)> doc(
            yyjson_read(content.c_str(), content.size(), 0),
            [](yyjson_doc *p) { yyjson_doc_free(p); }
        );

        if (doc == nullptr) {
            Logger::instance().error(tr::txt(tr::TID::ERR_PARSING_JSON_FAILED));
            return false;
        }

        auto *root = yyjson_doc_get_root(doc.get());

        if (status_code != 200) {
            // Open-Meteo returns error info with "reason" field
            auto reason = jvh::getString(root, "reason");
            if (reason.empty()) {
                reason = jvh::getString(root, "error");
            }
            Logger::instance().error(std::format("[{}] {}", status_code, reason));
            return false;
        }

        func(root);
        return true;
    }

    RealtimeWeather queryRealtimeWeather(const Location &loc, const Config &cfg) {
        RealtimeWeather rt_weather{};

        if (loc.latitude.empty() || loc.longitude.empty()) {
            Logger::instance().error(tr::txt(tr::TID::ERR_NO_LONG_LAT));
            return rt_weather;
        }

        constexpr std::string_view host{ "https://api.open-meteo.com" };
        constexpr std::string_view path{ "/v1/forecast" };

        utils::HttpParams params;
        params.data.emplace("latitude", loc.latitude);
        params.data.emplace("longitude", loc.longitude);
        params.data.emplace("current", "temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,weather_code,wind_speed_10m,wind_direction_10m");
        params.data.emplace("timezone", "auto");
        params.data.emplace("temperature_unit", getTempUnitParam(cfg.unit_type));
        params.data.emplace("wind_speed_unit", getSpeedUnitParam(cfg.unit_type));

        auto func = [&rt_weather, &cfg](yyjson_val *j_root) {
            auto *j_current = yyjson_obj_get(j_root, "current");
            if (j_current == nullptr) {
                Logger::instance().error(tr::txt(tr::TID::ERR_EXTRACT_DATA_FAILED));
                return;
            }

            rt_weather.temp = jsonGetNumberAsStr1dp(j_current, "temperature_2m");
            rt_weather.temp_feels_like = jsonGetNumberAsStr1dp(j_current, "apparent_temperature");
            rt_weather.humidity = jsonGetNumberAsStr1dp(j_current, "relative_humidity_2m");
            rt_weather.precipitation = jsonGetNumberAsStr1dp(j_current, "precipitation");
            rt_weather.wind_speed = jsonGetNumberAsStr1dp(j_current, "wind_speed_10m");

            const auto wmo_code = jvh::getInt(j_current, "weather_code");
            rt_weather.weather_text = wmoCodeToText(wmo_code);

            const auto is_day = jvh::getBool(j_current, "is_day");
            rt_weather.is_day = is_day ? "1" : "0";
            rt_weather.weather_code = wmoCodeToIcon(wmo_code, is_day);

            rt_weather.wind_direction = getWindDirectionText(jvh::getInt(j_current, "wind_direction_10m"));

            rt_weather.update_time = jvh::getString(j_current, "time");
            rt_weather.unit_type = cfg.unit_type;
        };

        if (!queryFrame(host, path, params, func)) {
            Logger::instance().error(tr::txt(tr::TID::ERR_QUERY_RTW_FAILED));
        }

        return rt_weather;
    }

    std::array<ForecastedWeather, 3> queryForecastedWeather(const Location &loc, const Config &cfg) {
        std::array<ForecastedWeather, 3> fc_weather_3d{};

        if (loc.latitude.empty() || loc.longitude.empty()) {
            Logger::instance().error(tr::txt(tr::TID::ERR_NO_LONG_LAT));
            return fc_weather_3d;
        }

        constexpr std::string_view host{ "https://api.open-meteo.com" };
        constexpr std::string_view path{ "/v1/forecast" };

        utils::HttpParams params;
        params.data.emplace("latitude", loc.latitude);
        params.data.emplace("longitude", loc.longitude);
        params.data.emplace("daily", "weather_code,temperature_2m_max,temperature_2m_min,uv_index_max,precipitation_probability_max,relative_humidity_2m_max");
        params.data.emplace("timezone", "auto");
        params.data.emplace("forecast_days", "3");
        params.data.emplace("temperature_unit", getTempUnitParam(cfg.unit_type));
        params.data.emplace("wind_speed_unit", getSpeedUnitParam(cfg.unit_type));

        auto func = [&fc_weather_3d, &cfg](yyjson_val *j_root) {
            auto *j_daily = yyjson_obj_get(j_root, "daily");
            if (j_daily == nullptr) {
                Logger::instance().error(tr::txt(tr::TID::ERR_EXTRACT_DATA_FAILED));
                return;
            }

            auto *j_codes = yyjson_obj_get(j_daily, "weather_code");
            auto *j_tmax = yyjson_obj_get(j_daily, "temperature_2m_max");
            auto *j_tmin = yyjson_obj_get(j_daily, "temperature_2m_min");
            auto *j_uv = yyjson_obj_get(j_daily, "uv_index_max");
            auto *j_pop = yyjson_obj_get(j_daily, "precipitation_probability_max");
            auto *j_hum = yyjson_obj_get(j_daily, "relative_humidity_2m_max");

            const auto max_size = std::min({
                yyjson_arr_size(j_codes),
                yyjson_arr_size(j_tmax),
                yyjson_arr_size(j_tmin),
                size_t{ 3 }
            });

            for (size_t i = 0; i < max_size; ++i) {
                const auto wmo = yyjson_get_int(yyjson_arr_get(j_codes, i));
                fc_weather_3d[i].weather_code_day = wmoCodeToIcon(wmo, true);
                fc_weather_3d[i].weather_text_day = wmoCodeToText(wmo);

                auto tmax = yyjson_get_num(yyjson_arr_get(j_tmax, i));
                auto tmin = yyjson_get_num(yyjson_arr_get(j_tmin, i));
                fc_weather_3d[i].temp_max = std::format("{:.0f}", std::round(tmax));
                fc_weather_3d[i].temp_min = std::format("{:.0f}", std::round(tmin));

                if (j_uv != nullptr) {
                    auto uv = yyjson_get_real(yyjson_arr_get(j_uv, i));
                    auto uv_rounded = std::round(uv * 10.0) / 10.0;
                    if (std::abs(uv_rounded - std::round(uv_rounded)) < 1e-9) {
                        fc_weather_3d[i].uv_index = std::format("{:.0f}", uv_rounded);
                    } else {
                        fc_weather_3d[i].uv_index = std::format("{:.1f}", uv_rounded);
                    }
                }

                if (j_pop != nullptr) {
                    fc_weather_3d[i].precipitation_probability = std::to_string(yyjson_get_int(yyjson_arr_get(j_pop, i)));
                }

                if (j_hum != nullptr) {
                    fc_weather_3d[i].humidity = std::to_string(yyjson_get_int(yyjson_arr_get(j_hum, i)));
                }

                fc_weather_3d[i].unit_type = cfg.unit_type;
            }
        };

        if (!queryFrame(host, path, params, func)) {
            Logger::instance().error(tr::txt(tr::TID::ERR_QUERY_FCW3D_FAILED));
        }

        return fc_weather_3d;
    }

    RealtimeAirQuality queryRealtimeAirQuality(const Location &loc, const Config &cfg) {
        RealtimeAirQuality rt_air{};

        if (loc.latitude.empty() || loc.longitude.empty()) {
            Logger::instance().error(tr::txt(tr::TID::ERR_NO_LONG_LAT));
            return rt_air;
        }

        constexpr std::string_view host{ "https://air-quality-api.open-meteo.com" };
        constexpr std::string_view path{ "/v1/air-quality" };

        utils::HttpParams params;
        params.data.emplace("latitude", loc.latitude);
        params.data.emplace("longitude", loc.longitude);
        params.data.emplace("current", "us_aqi,pm10,pm2_5");
        params.data.emplace("timezone", "auto");

        auto func = [&rt_air](yyjson_val *j_root) {
            auto *j_current = yyjson_obj_get(j_root, "current");
            if (j_current == nullptr) {
                return;
            }

            rt_air.aqi = jsonGetNumberAsStr1dp(j_current, "us_aqi");
            rt_air.pm2p5 = jsonGetNumberAsStr1dp(j_current, "pm2_5");
            rt_air.pm10 = jsonGetNumberAsStr1dp(j_current, "pm10");
        };

        if (!queryFrame(host, path, params, func)) {
            Logger::instance().error(tr::txt(tr::TID::ERR_QUERY_RTAQ_FAILED));
        }

        return rt_air;
    }

    Location geocodingExtractLocationInfo(yyjson_val *j_loc) {
        Location loc;

        loc.name = jvh::getString(j_loc, "name");
        loc.latitude = std::format("{:.4f}", yyjson_get_real(yyjson_obj_get(j_loc, "latitude")));
        loc.longitude = std::format("{:.4f}", yyjson_get_real(yyjson_obj_get(j_loc, "longitude")));

        auto country = jvh::getString(j_loc, "country");
        auto admin1 = jvh::getString(j_loc, "admin1");
        if (admin1.empty()) {
            loc.administrative_ownership = country;
        } else if (country.empty()) {
            loc.administrative_ownership = admin1;
        } else {
            loc.administrative_ownership = std::format("{}, {}", admin1, country);
        }

        return loc;
    }

    bool geocodingDirect(const std::string &query, Locations &queried_locations) {
        queried_locations.clear();

        constexpr std::string_view host{ "https://geocoding-api.open-meteo.com" };
        constexpr std::string_view path{ "/v1/search" };

        utils::HttpParams params;
        params.data.emplace("name", query);
        params.data.emplace("count", "10");
        params.data.emplace("language", std::string{ tr::txt(tr::TID::LC_OPEN_METEO) });
        params.data.emplace("format", "json");

        auto func = [&queried_locations](yyjson_val *j_root) {
            auto *j_results = yyjson_obj_get(j_root, "results");
            if (j_results == nullptr || !yyjson_is_arr(j_results)) {
                return;
            }

            size_t idx, max;
            yyjson_val *j_location;
            yyjson_arr_foreach(j_results, idx, max, j_location) {
                queried_locations.push_back(geocodingExtractLocationInfo(j_location));
            }
        };

        if (queryFrame(host, path, params, func)) {
            return true;
        }

        Logger::instance().error(tr::txt(tr::TID::ERR_QUERY_GEOCODING_FAILED));
        return false;
    }

    bool geocodingReverse(const std::string &latitude, const std::string &longitude,
                          Locations &queried_locations) {
        queried_locations.clear();

        // Open-Meteo does not provide reverse geocoding; use BigDataCloud free API
        // (no API key required, no registration)
        constexpr std::string_view host{ "https://api.bigdatacloud.net" };
        constexpr std::string_view path{ "/data/reverse-geocode-client" };

        utils::HttpParams params;
        params.data.emplace("latitude", latitude);
        params.data.emplace("longitude", longitude);
        params.data.emplace("localityLanguage", std::string{ tr::txt(tr::TID::LC_OPEN_METEO) });
        params.data.emplace("key", "free");

        auto func = [&queried_locations, &latitude, &longitude](yyjson_val *j_root) {
            Location loc;
            loc.latitude = latitude;
            loc.longitude = longitude;

            loc.name = jvh::getString(j_root, "locality");
            if (loc.name.empty()) {
                loc.name = jvh::getString(j_root, "city");
            }
            if (loc.name.empty()) {
                loc.name = jvh::getString(j_root, "principalSubdivision");
            }

            auto country = jvh::getString(j_root, "countryName");
            auto admin = jvh::getString(j_root, "principalSubdivision");
            if (admin.empty()) {
                loc.administrative_ownership = country;
            } else if (country.empty()) {
                loc.administrative_ownership = admin;
            } else {
                loc.administrative_ownership = std::format("{}, {}", admin, country);
            }

            if (!loc.name.empty()) {
                queried_locations.push_back(std::move(loc));
            }
        };

        if (queryFrame(host, path, params, func)) {
            return true;
        }

        Logger::instance().error(tr::txt(tr::TID::ERR_QUERY_GEOCODING_FAILED));
        return false;
    }
}

std::string DataProviderOpenMeteo::WeatherDataBlock::getWeatherSummary() const {
    std::ostringstream oss;

    // weather and temperature
    oss << getWeatherItem(WeatherTimeSlot::REALTIME, WeatherItem::WEATHER_TEXT) << " "
        << getWeatherItem(WeatherTimeSlot::REALTIME, WeatherItem::TEMPERATURE) << " "
        << "(" << realtime_weather.update_time << ")";

    // wind
    oss << "\n" << getWeatherItem(WeatherTimeSlot::REALTIME, WeatherItem::WIND);

    // humidity
    oss << std::format(" {}: {}", tr::txt(tr::TID::FMT_HUMIDITY),
                       getWeatherItem(WeatherTimeSlot::REALTIME, WeatherItem::HUMIDITY));

    // air quality
    oss << "\n"
        << "AQI: " << getWeatherItem(WeatherTimeSlot::REALTIME, WeatherItem::AIR_QUALITY)
        << " PM2.5: " << getWeatherItem(WeatherTimeSlot::REALTIME, WeatherItem::AIR_PM2P5)
        << " PM10: " << getWeatherItem(WeatherTimeSlot::REALTIME, WeatherItem::AIR_PM10);

    // forecast weather
    auto fcw_formatter = [&oss, this](WeatherTimeSlot wt, std::string_view str_wt) {
        if (getWeatherItem(wt, WeatherItem::WEATHER_TEXT).empty()) {
            return;
        }

        oss << "\n" << str_wt << ": " << getWeatherItem(wt, WeatherItem::WEATHER_TEXT)
            << " " << getWeatherItem(wt, WeatherItem::TEMPERATURE);
    };

    fcw_formatter(WeatherTimeSlot::TODAY, tr::txt(tr::TID::FMT_TODAY));
    fcw_formatter(WeatherTimeSlot::TOMMROW, tr::txt(tr::TID::FMT_TOMORROW));
    fcw_formatter(WeatherTimeSlot::DAY_AFTER_TOMMROW, tr::txt(tr::TID::FMT_DAT_AFTER_TOMORROW));

    oss << "\n";

    return oss.str();
}

std::string DataProviderOpenMeteo::WeatherDataBlock::getWeatherItem(WeatherTimeSlot time_slot, WeatherItem item) const {
    if (time_slot == WeatherTimeSlot::REALTIME) {
        if (item == WeatherItem::TEMPERATURE) {
            return std::format("{}{}", realtime_weather.temp, getTemperatureUnit(realtime_weather.unit_type));
        } else if (item == WeatherItem::WEATHER_TEXT) {
            return realtime_weather.weather_text;
        } else if (item == WeatherItem::WEATHER_CODE) {
            return realtime_weather.weather_code;
        } else if (item == WeatherItem::HUMIDITY) {
            return std::format("{}%", realtime_weather.humidity);
        } else if (item == WeatherItem::WIND) {
            return std::format("{} {}{}", realtime_weather.wind_direction,
                               realtime_weather.wind_speed, getSpeedUnit(realtime_weather.unit_type));
        } else if (item == WeatherItem::PRECIPITATION) {
            if (!realtime_weather.precipitation.empty()) {
                return std::format("{}mm", realtime_weather.precipitation);
            }
            return {};
        } else if (item == WeatherItem::AIR_QUALITY) {
            return realtime_air.aqi;
        } else if (item == WeatherItem::AIR_PM2P5) {
            return std::format("{}μg/m³", realtime_air.pm2p5);
        } else if (item == WeatherItem::AIR_PM10) {
            return std::format("{}μg/m³", realtime_air.pm10);
        } else if (item == WeatherItem::ALERTS) {
            return {};
        }
    } else {
        auto getForecastedWeatherContent = [](const ForecastedWeather &fcw, WeatherItem item) -> std::string {
            if (item == WeatherItem::TEMPERATURE) {
                return std::format("{}~{}{}", fcw.temp_min, fcw.temp_max, getTemperatureUnit(fcw.unit_type));
            } else if (item == WeatherItem::WEATHER_TEXT) {
                return fcw.weather_text_day;
            } else if (item == WeatherItem::WEATHER_CODE) {
                return fcw.weather_code_day;
            } else if (item == WeatherItem::HUMIDITY) {
                return fcw.humidity.empty() ? std::string{} : std::format("{}%", fcw.humidity);
            } else if (item == WeatherItem::UV_INDEX) {
                return fcw.uv_index;
            } else if (item == WeatherItem::PRECIPITATION) {
                return fcw.precipitation_probability.empty() ? std::string{} : std::format("{}%", fcw.precipitation_probability);
            } else {
                return {};
            }
        };

        if (time_slot == WeatherTimeSlot::TODAY) {
            return getForecastedWeatherContent(fc_weather_3d[0], item);
        } else if (time_slot == WeatherTimeSlot::TOMMROW) {
            return getForecastedWeatherContent(fc_weather_3d[1], item);
        } else if (time_slot == WeatherTimeSlot::DAY_AFTER_TOMMROW) {
            return getForecastedWeatherContent(fc_weather_3d[2], item);
        }
    }

    return {};
}

bool DataProviderOpenMeteo::geocodingDirect(const std::string &query, Locations &queried_locations) const {
    return ::geocodingDirect(query, queried_locations);
}

bool DataProviderOpenMeteo::geocodingReverse(const std::string &latitude, const std::string &longitude,
                                              Locations &queried_locations) const {
    return ::geocodingReverse(latitude, longitude, queried_locations);
}

WeatherDataCPtr DataProviderOpenMeteo::getWeatherData(const Location &loc) const {
    const auto data_block = std::make_shared<WeatherDataBlock>();

    auto fut_rt_weather = std::async(std::launch::async, [this, &loc] {
        return queryRealtimeWeather(loc, config);
    });

    auto fut_fc_weather_3d = std::async(std::launch::async, [this, &loc] {
        return queryForecastedWeather(loc, config);
    });

    auto fut_rt_air = std::async(std::launch::async, [this, &loc] {
        return queryRealtimeAirQuality(loc, config);
    });

    data_block->realtime_weather = fut_rt_weather.get();
    data_block->fc_weather_3d = fut_fc_weather_3d.get();
    data_block->realtime_air = fut_rt_air.get();

    return data_block;
}
