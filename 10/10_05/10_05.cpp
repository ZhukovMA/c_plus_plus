#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

using HashFn = std::uint32_t(*)(const std::string&);

struct HashSpec {
    std::string name;
    HashFn fn;
};

std::uint32_t RSHash(const std::string& s)
{
    std::uint32_t b = 378551u;
    std::uint32_t a = 63689u;
    std::uint32_t hash = 0u;

    for (unsigned char ch : s) {
        hash = hash * a + ch;
        a = a * b;
    }

    return hash;
}

std::uint32_t JSHash(const std::string& s)
{
    std::uint32_t hash = 1315423911u;

    for (unsigned char ch : s) {
        hash ^= ((hash << 5u) + ch + (hash >> 2u));
    }

    return hash;
}

std::uint32_t PJWHash(const std::string& s)
{
    constexpr std::uint32_t bits_in_uint = 32u;
    constexpr std::uint32_t three_quarters = (bits_in_uint * 3u) / 4u;
    constexpr std::uint32_t one_eighth = bits_in_uint / 8u;
    constexpr std::uint32_t high_bits = 0xFFFFFFFFu << (bits_in_uint - one_eighth);

    std::uint32_t hash = 0u;

    for (unsigned char ch : s) {
        hash = (hash << one_eighth) + ch;
        const std::uint32_t test = hash & high_bits;
        if (test != 0u) {
            hash = (hash ^ (test >> three_quarters)) & (~high_bits);
        }
    }

    return hash;
}

std::uint32_t ELFHash(const std::string& s)
{
    std::uint32_t hash = 0u;

    for (unsigned char ch : s) {
        hash = (hash << 4u) + ch;
        const std::uint32_t x = hash & 0xF0000000u;
        if (x != 0u) {
            hash ^= (x >> 24u);
        }
        hash &= ~x;
    }

    return hash;
}

std::uint32_t BKDRHash(const std::string& s)
{
    constexpr std::uint32_t seed = 131u;
    std::uint32_t hash = 0u;

    for (unsigned char ch : s) {
        hash = hash * seed + ch;
    }

    return hash;
}

std::uint32_t SDBMHash(const std::string& s)
{
    std::uint32_t hash = 0u;

    for (unsigned char ch : s) {
        hash = ch + (hash << 6u) + (hash << 16u) - hash;
    }

    return hash;
}

std::uint32_t DJBHash(const std::string& s)
{
    std::uint32_t hash = 5381u;

    for (unsigned char ch : s) {
        hash = ((hash << 5u) + hash) + ch;
    }

    return hash;
}

std::uint32_t DEKHash(const std::string& s)
{
    std::uint32_t hash = static_cast<std::uint32_t>(s.size());

    for (unsigned char ch : s) {
        hash = ((hash << 5u) ^ (hash >> 27u)) ^ ch;
    }

    return hash;
}

std::uint32_t BPHash(const std::string& s)
{
    std::uint32_t hash = 0u;

    for (unsigned char ch : s) {
        hash = (hash << 7u) ^ ch;
    }

    return hash;
}

std::uint32_t FNVHash(const std::string& s)
{
    // Kept exactly as in the Partow library.
    constexpr std::uint32_t fnv_prime = 0x811C9DC5u;
    std::uint32_t hash = 0u;

    for (unsigned char ch : s) {
        hash *= fnv_prime;
        hash ^= ch;
    }

    return hash;
}

std::uint32_t APHash(const std::string& s)
{
    std::uint32_t hash = 0xAAAAAAAAu;

    for (std::size_t i = 0; i < s.size(); ++i) {
        const auto ch = static_cast<std::uint32_t>(static_cast<unsigned char>(s[i]));
        if ((i & 1u) == 0u) {
            hash ^= ((hash << 7u) ^ (ch * (hash >> 3u)));
        } else {
            hash ^= ~((hash << 11u) + (ch ^ (hash >> 5u)));
        }
    }

    return hash;
}

std::vector<HashSpec> classic9()
{
    return {
        {"RS", RSHash},
        {"JS", JSHash},
        {"PJW", PJWHash},
        {"ELF", ELFHash},
        {"BKDR", BKDRHash},
        {"SDBM", SDBMHash},
        {"DJB", DJBHash},
        {"DEK", DEKHash},
        {"AP", APHash},
    };
}

std::vector<HashSpec> all11()
{
    return {
        {"RS", RSHash},
        {"JS", JSHash},
        {"PJW", PJWHash},
        {"ELF", ELFHash},
        {"BKDR", BKDRHash},
        {"SDBM", SDBMHash},
        {"DJB", DJBHash},
        {"DEK", DEKHash},
        {"BP", BPHash},
        {"FNV", FNVHash},
        {"AP", APHash},
    };
}

std::string base62(std::uint64_t value)
{
    static constexpr char alphabet[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    if (value == 0u) {
        return "0";
    }

    std::string result;
    while (value > 0u) {
        result.push_back(alphabet[value % 62u]);
        value /= 62u;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::string make_random_string(std::uint64_t index, std::mt19937_64& rng)
{
    static constexpr char alphabet[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::uniform_int_distribution<int> length_dist(8, 40);
    std::uniform_int_distribution<int> char_dist(0, 61);

    std::string s = base62(index);
    s.push_back('_');
    const int extra = length_dist(rng);
    s.reserve(s.size() + static_cast<std::size_t>(extra));

    for (int i = 0; i < extra; ++i) {
        s.push_back(alphabet[char_dist(rng)]);
    }

    return s;
}

std::string make_sequential_string(std::uint64_t index)
{
    std::ostringstream out;
    out << "str_" << std::setw(8) << std::setfill('0') << index;
    return out.str();
}

struct ExperimentOptions {
    std::string dataset = "random";
    std::uint64_t count = 200000;
    std::uint64_t step = 5000;
    std::uint64_t seed = 42;
    std::string mode = "classic9";
    std::string csv_path = "collisions.csv";
};

ExperimentOptions parse_args(int argc, char** argv)
{
    ExperimentOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto take_value = [&](const std::string& prefix) -> std::string {
            if (arg.rfind(prefix, 0) == 0) {
                return arg.substr(prefix.size());
            }
            throw std::runtime_error("Unknown argument: " + arg);
        };

        if (arg.rfind("--dataset=", 0) == 0) {
            options.dataset = take_value("--dataset=");
        } else if (arg.rfind("--count=", 0) == 0) {
            options.count = std::stoull(take_value("--count="));
        } else if (arg.rfind("--step=", 0) == 0) {
            options.step = std::stoull(take_value("--step="));
        } else if (arg.rfind("--seed=", 0) == 0) {
            options.seed = std::stoull(take_value("--seed="));
        } else if (arg.rfind("--mode=", 0) == 0) {
            options.mode = take_value("--mode=");
        } else if (arg.rfind("--csv=", 0) == 0) {
            options.csv_path = take_value("--csv=");
        } else if (arg == "--help") {
            std::cout
                << "Usage:\n"
                << "  ./hash_study --dataset=random|sequential --count=N --step=S\n"
                << "               --seed=42 --mode=classic9|all11 --csv=result.csv\n";
            std::exit(0);
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    if (options.dataset != "random" && options.dataset != "sequential") {
        throw std::runtime_error("Only dataset=random and dataset=sequential are supported.");
    }
    if (options.mode != "classic9" && options.mode != "all11") {
        throw std::runtime_error("Only mode=classic9 and mode=all11 are supported.");
    }
    if (options.count == 0u || options.step == 0u) {
        throw std::runtime_error("count and step must be positive.");
    }

    return options;
}

struct SummaryRow {
    std::string name;
    std::uint64_t collisions = 0;
    double collision_rate = 0.0;
};

int run(const ExperimentOptions& options)
{
    const auto functions = (options.mode == "all11") ? all11() : classic9();
    std::vector<std::vector<std::uint64_t>> table(functions.size());
    std::vector<std::uint64_t> checkpoints;
    checkpoints.reserve(static_cast<std::size_t>((options.count + options.step - 1u) / options.step));

    std::vector<std::unordered_set<std::uint32_t>> seen(functions.size());
    for (auto& set : seen) {
        set.reserve(static_cast<std::size_t>(options.count * 1.3));
    }

    std::vector<std::uint64_t> collisions(functions.size(), 0u);
    std::mt19937_64 rng(options.seed);

    for (std::uint64_t i = 1; i <= options.count; ++i) {
        std::string value;
        if (options.dataset == "random") {
            value = make_random_string(i, rng);
        } else {
            value = make_sequential_string(i);
        }

        for (std::size_t fn_index = 0; fn_index < functions.size(); ++fn_index) {
            const std::uint32_t h = functions[fn_index].fn(value);
            auto [_, inserted] = seen[fn_index].insert(h);
            if (!inserted) {
                ++collisions[fn_index];
            }
        }

        if (i % options.step == 0u || i == options.count) {
            checkpoints.push_back(i);
            for (std::size_t fn_index = 0; fn_index < functions.size(); ++fn_index) {
                table[fn_index].push_back(collisions[fn_index]);
            }
        }
    }

    std::ofstream csv(options.csv_path);
    if (!csv) {
        throw std::runtime_error("Failed to open file: " + options.csv_path);
    }

    csv << "N";
    for (const auto& fn : functions) {
        csv << ',' << fn.name;
    }
    csv << '\n';

    for (std::size_t row = 0; row < checkpoints.size(); ++row) {
        csv << checkpoints[row];
        for (std::size_t fn_index = 0; fn_index < functions.size(); ++fn_index) {
            csv << ',' << table[fn_index][row];
        }
        csv << '\n';
    }

    std::vector<SummaryRow> summary;
    summary.reserve(functions.size());
    for (std::size_t fn_index = 0; fn_index < functions.size(); ++fn_index) {
        summary.push_back({
            functions[fn_index].name,
            collisions[fn_index],
            static_cast<double>(collisions[fn_index]) / static_cast<double>(options.count)
        });
    }

    std::sort(summary.begin(), summary.end(), [](const SummaryRow& lhs, const SummaryRow& rhs) {
        if (lhs.collisions != rhs.collisions) {
            return lhs.collisions < rhs.collisions;
        }
        return lhs.name < rhs.name;
    });

    std::cout << "dataset=" << options.dataset
              << ", count=" << options.count
              << ", step=" << options.step
              << ", seed=" << options.seed
              << ", mode=" << options.mode << "\n";
    std::cout << "CSV: " << options.csv_path << "\n\n";
    std::cout << std::left << std::setw(8) << "hash"
              << std::right << std::setw(14) << "collisions"
              << std::setw(18) << "collision_rate" << '\n';
    std::cout << std::string(40, '-') << '\n';
    for (const auto& row : summary) {
        std::cout << std::left << std::setw(8) << row.name
                  << std::right << std::setw(14) << row.collisions
                  << std::setw(17) << std::fixed << std::setprecision(8) << row.collision_rate
                  << '\n';
    }

    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        return run(parse_args(argc, argv));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}