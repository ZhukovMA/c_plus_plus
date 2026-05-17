
//////////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <filesystem>
#include <format>
#include <print>
#include <regex>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////////////////

auto make_type(fs::file_status status) -> char
{
    if (fs::is_directory(status))    return 'd';
    if (fs::is_regular_file(status)) return 'f';
    if (fs::is_symlink(status))      return 'l';

    return '?';
}

//////////////////////////////////////////////////////////////////////////////////////

auto make_permissions(fs::perms permissions) -> std::string
{
    auto bit = [permissions](fs::perms value, char symbol)
    {
        return (permissions & value) == fs::perms::none ? '-' : symbol;
    };

    return
    {
        bit(fs::perms::owner_read,  'r'),
        bit(fs::perms::owner_write, 'w'),
        bit(fs::perms::owner_exec,  'x')
    };
}

//////////////////////////////////////////////////////////////////////////////////////

auto directory_size(fs::path const& path) -> std::uintmax_t
{
    std::uintmax_t result = 0;

    std::error_code error;

    fs::recursive_directory_iterator begin
    {
        path,
        fs::directory_options::skip_permission_denied,
        error
    };

    fs::recursive_directory_iterator end;

    for (auto iterator = begin; iterator != end; iterator.increment(error))
    {
        if (error)
        {
            error.clear();
            continue;
        }

        auto const& entry = *iterator;

        auto status = entry.symlink_status(error);

        if (error)
        {
            error.clear();
            continue;
        }

        if (fs::is_regular_file(status))
        {
            auto bytes = entry.file_size(error);

            if (!error)
            {
                result += bytes;
            }

            error.clear();
        }
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////

auto make_size(fs::directory_entry const& entry) -> std::string
{
    std::error_code error;

    std::uintmax_t bytes = 0;

    auto status = entry.symlink_status(error);

    if (error)
    {
        return "   ? (?)";
    }

    if (fs::is_regular_file(status))
    {
        bytes = entry.file_size(error);

        if (error)
        {
            return "   ? (?)";
        }
    }
    else if (fs::is_directory(status))
    {
        bytes = directory_size(entry.path());
    }

    std::vector<char> units { 'B', 'K', 'M', 'G', 'T' };

    std::size_t unit = 0;

    while (bytes >= 1024 && unit + 1 < units.size())
    {
        bytes /= 1024;
        ++unit;
    }

    return (std::stringstream {} << std::format("{:>4} ({})", bytes, units[unit])).str();
}

//////////////////////////////////////////////////////////////////////////////////////

auto make_time(fs::directory_entry const& entry) -> std::string
{
    std::error_code error;

    auto file_time = entry.last_write_time(error);

    if (error)
    {
        return "unknown time";
    }

    auto system_time = std::chrono::file_clock::to_sys(file_time);
    auto seconds = std::chrono::floor<std::chrono::seconds>(system_time);

    return std::format("{:%F %T}", seconds);
}

//////////////////////////////////////////////////////////////////////////////////////

auto name_matches(fs::directory_entry const& entry, std::regex const& pattern) -> bool
{
    auto name = entry.path().filename().string();

    return std::regex_search(name, pattern);
}

//////////////////////////////////////////////////////////////////////////////////////

void show(fs::path const& directory, std::regex const& pattern)
{
    std::error_code error;

    if (!fs::exists(directory, error) || !fs::is_directory(directory, error))
    {
        std::print("show : error : '{}' is not a directory\n", directory.string());
        return;
    }

    fs::directory_iterator begin
    {
        directory,
        fs::directory_options::skip_permission_denied,
        error
    };

    fs::directory_iterator end;

    if (error)
    {
        std::print("show : error : cannot open '{}'\n", directory.string());
        return;
    }

    for (auto iterator = begin; iterator != end; iterator.increment(error))
    {
        if (error)
        {
            error.clear();
            continue;
        }

        auto const& entry = *iterator;

        if (!name_matches(entry, pattern))
        {
            continue;
        }

        auto status = entry.symlink_status(error);

        if (error)
        {
            error.clear();
            continue;
        }

        std::print
        (
            "show : entry : {} | {} | {} | {} | {}\n",

            make_type(status),
            make_permissions(status.permissions()),
            make_size(entry),
            make_time(entry),
            entry.path().filename().string()
        );
    }
}

//////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::print("usage : {} <regex> [directory]\n", argv[0]);
        std::print("example : {} \"\\.cpp$\"\n", argv[0]);
        std::print("example : {} \"^(main|test).*\" ./src\n", argv[0]);

        return 1;
    }

    try
    {
        std::regex pattern
        {
            argv[1],
            std::regex::ECMAScript | std::regex::icase
        };

        fs::path directory = argc == 3 ? fs::path { argv[2] } : fs::current_path();

        show(directory, pattern);
    }
    catch (std::regex_error const& error)
    {
        std::print("regex error : {}\n", error.what());
        return 2;
    }
}

//////////////////////////////////////////////////////////////////////////////////////