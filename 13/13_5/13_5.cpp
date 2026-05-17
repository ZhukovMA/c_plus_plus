#include <benchmark/benchmark.h>

#include <nlohmann/json.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <system_error>

namespace cfg
{
    inline constexpr std::string_view json_sample =
        R"({"i":123456789,"x":12.34567})";

    inline constexpr std::size_t int_pos = 5;
    inline constexpr std::size_t int_digits = 9;

    inline constexpr std::size_t float_pos = 19;
    inline constexpr std::size_t float_int_digits = 2;
    inline constexpr std::size_t float_frac_digits = 5;
    inline constexpr std::size_t float_len =
        float_int_digits + 1 + float_frac_digits;

    static_assert(json_sample[int_pos] == '1');
    static_assert(json_sample[float_pos] == '1');
    static_assert(json_sample[float_pos + float_int_digits] == '.');
}

struct Numbers
{
    int i;
    float x;
};

constexpr std::uint32_t pow10_u32(std::size_t n) noexcept
{
    std::uint32_t r = 1;

    for (std::size_t k = 0; k < n; ++k)
        r *= 10;

    return r;
}

template <std::size_t Digits>
inline int parse_uint_fixed(const char* p) noexcept
{
    int value = 0;

    for (std::size_t k = 0; k < Digits; ++k)
        value = value * 10 + (p[k] - '0');

    return value;
}

template <std::size_t IntDigits, std::size_t FracDigits>
inline float parse_float_fixed(const char* p) noexcept
{
    const bool negative = *p == '-';

    if (negative)
        ++p;

    const int whole = parse_uint_fixed<IntDigits>(p);

    p += IntDigits + 1;

    const int frac = parse_uint_fixed<FracDigits>(p);

    const double result =
        static_cast<double>(whole)
        + static_cast<double>(frac) / static_cast<double>(pow10_u32(FracDigits));

    return static_cast<float>(negative ? -result : result);
}

template <std::size_t Digits>
inline void write_uint_fixed(char* out, int value) noexcept
{
    assert(value >= 0);

    for (std::size_t k = 0; k < Digits; ++k)
    {
        out[Digits - 1 - k] = static_cast<char>('0' + value % 10);
        value /= 10;
    }
}

template <std::size_t IntDigits, std::size_t FracDigits>
inline void write_float_fixed(char* out, float value) noexcept
{
    assert(value >= 0.0f);

    const auto scale = pow10_u32(FracDigits);

    const auto scaled = static_cast<std::uint64_t>(
        std::llround(static_cast<double>(value) * static_cast<double>(scale))
    );

    const auto whole = static_cast<int>(scaled / scale);
    const auto frac = static_cast<int>(scaled % scale);

    write_uint_fixed<IntDigits>(out, whole);

    out[IntDigits] = '.';

    write_uint_fixed<FracDigits>(out + IntDigits + 1, frac);
}

inline Numbers read_by_nlohmann(std::string_view s)
{
    const auto json = nlohmann::json::parse(s.begin(), s.end());

    return {
        json.at("i").get<int>(),
        json.at("x").get<float>()
    };
}

inline Numbers read_by_rapidjson(std::string_view s)
{
    rapidjson::Document doc;
    doc.Parse(s.data(), s.size());

    return {
        doc["i"].GetInt(),
        static_cast<float>(doc["x"].GetDouble())
    };
}

inline Numbers read_by_from_chars(std::string_view s)
{
    Numbers result{};

    const char* base = s.data();

    auto int_begin = base + cfg::int_pos;
    auto int_end = int_begin + cfg::int_digits;

    auto [int_ptr, int_ec] = std::from_chars(int_begin, int_end, result.i);

    if (int_ec != std::errc{} || int_ptr != int_end)
        std::abort();

    auto float_begin = base + cfg::float_pos;
    auto float_end = float_begin + cfg::float_len;

    auto [float_ptr, float_ec] = std::from_chars(
        float_begin,
        float_end,
        result.x,
        std::chars_format::fixed
    );

    if (float_ec != std::errc{} || float_ptr != float_end)
        std::abort();

    return result;
}

inline Numbers read_by_custom_parser(std::string_view s) noexcept
{
    const char* base = s.data();

    return {
        parse_uint_fixed<cfg::int_digits>(base + cfg::int_pos),
        parse_float_fixed<cfg::float_int_digits, cfg::float_frac_digits>(
            base + cfg::float_pos
        )
    };
}

inline std::string write_by_nlohmann(Numbers value)
{
    nlohmann::json json;

    json["i"] = value.i;
    json["x"] = value.x;

    return json.dump();
}

inline std::string write_by_rapidjson(Numbers value)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    writer.StartObject();

    writer.Key("i");
    writer.Int(value.i);

    writer.Key("x");
    writer.Double(static_cast<double>(value.x));

    writer.EndObject();

    return {buffer.GetString(), buffer.GetSize()};
}

inline void write_by_to_chars(std::string& out, Numbers value)
{
    char* base = out.data();

    auto int_begin = base + cfg::int_pos;
    auto int_end = int_begin + cfg::int_digits;

    auto [int_ptr, int_ec] = std::to_chars(int_begin, int_end, value.i);

    if (int_ec != std::errc{} || int_ptr != int_end)
        std::abort();

    auto float_begin = base + cfg::float_pos;
    auto float_end = float_begin + cfg::float_len;

    auto [float_ptr, float_ec] = std::to_chars(
        float_begin,
        float_end,
        value.x,
        std::chars_format::fixed,
        static_cast<int>(cfg::float_frac_digits)
    );

    if (float_ec != std::errc{} || float_ptr != float_end)
        std::abort();
}

inline void write_by_custom_algorithm(std::string& out, Numbers value) noexcept
{
    char* base = out.data();

    write_uint_fixed<cfg::int_digits>(
        base + cfg::int_pos,
        value.i
    );

    write_float_fixed<cfg::float_int_digits, cfg::float_frac_digits>(
        base + cfg::float_pos,
        value.x
    );
}

static const std::string input{cfg::json_sample};

static constexpr Numbers output_value{
    987654321,
    98.76543f
};

static void BM_Read_Nlohmann(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        auto value = read_by_nlohmann(input);
        benchmark::DoNotOptimize(value);
    }
}

static void BM_Read_RapidJSON(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        auto value = read_by_rapidjson(input);
        benchmark::DoNotOptimize(value);
    }
}

static void BM_Read_FromChars(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        auto value = read_by_from_chars(input);
        benchmark::DoNotOptimize(value);
    }
}

static void BM_Read_CustomParser(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        auto value = read_by_custom_parser(input);
        benchmark::DoNotOptimize(value);
    }
}

static void BM_Write_Nlohmann(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        auto text = write_by_nlohmann(output_value);
        benchmark::DoNotOptimize(text);
    }
}

static void BM_Write_RapidJSON(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        auto text = write_by_rapidjson(output_value);
        benchmark::DoNotOptimize(text);
    }
}

static void BM_Write_ToChars_InPlace(benchmark::State& state)
{
    std::string buffer{cfg::json_sample};

    for ([[maybe_unused]] auto _ : state)
    {
        write_by_to_chars(buffer, output_value);
        benchmark::DoNotOptimize(buffer.data());
        benchmark::ClobberMemory();
    }
}

static void BM_Write_Custom_InPlace(benchmark::State& state)
{
    std::string buffer{cfg::json_sample};

    for ([[maybe_unused]] auto _ : state)
    {
        write_by_custom_algorithm(buffer, output_value);
        benchmark::DoNotOptimize(buffer.data());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_Read_Nlohmann);
BENCHMARK(BM_Read_RapidJSON);
BENCHMARK(BM_Read_FromChars);
BENCHMARK(BM_Read_CustomParser);

BENCHMARK(BM_Write_Nlohmann);
BENCHMARK(BM_Write_RapidJSON);
BENCHMARK(BM_Write_ToChars_InPlace);
BENCHMARK(BM_Write_Custom_InPlace);

BENCHMARK_MAIN();