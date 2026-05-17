

////////////////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <fstream>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>

////////////////////////////////////////////////////////////////////////////////////////////

class source_cleaner
{
public:
    explicit source_cleaner(std::string_view text)
        : source_(text)
    {
    }

    [[nodiscard]]
    std::string run()
    {
        for (std::size_t i = 0; i < source_.size();)
        {
            if (auto prefix = raw_prefix_size(i); prefix != 0)
            {
                i = copy_raw_string(i, prefix);
                continue;
            }

            auto ch = source_[i];

            if (ch == '"' || ch == '\'')
            {
                i = copy_quoted_literal(i);
                continue;
            }

            if (ch == '/' && i + 1 < source_.size())
            {
                if (source_[i + 1] == '/')
                {
                    i = skip_line_comment(i);
                    continue;
                }

                if (source_[i + 1] == '*')
                {
                    i = skip_block_comment(i);
                    continue;
                }
            }

            emit(ch);
            ++i;
        }

        finish_last_line();

        return result_;
    }

private:
    std::string_view source_;

    std::string result_;
    std::string line_;

    bool line_has_text_ = false;
    bool line_is_protected_ = false;

private:
    static bool is_horizontal_space(char ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f' || ch == '\r';
    }

    static bool is_raw_delimiter_char(char ch)
    {
        return ch != ' '  &&
               ch != '\t' &&
               ch != '\v' &&
               ch != '\f' &&
               ch != '\n' &&
               ch != '\r' &&
               ch != '('  &&
               ch != ')'  &&
               ch != '\\';
    }

    void emit(char ch, bool protected_line = false)
    {
        if (protected_line)
        {
            line_is_protected_ = true;
        }

        if (ch == '\n')
        {
            flush_line();
            return;
        }

        line_.push_back(ch);

        if (!is_horizontal_space(ch))
        {
            line_has_text_ = true;
        }
    }

    void flush_line()
    {
        if (line_has_text_ || line_is_protected_)
        {
            if (line_is_protected_)
            {
                result_ += line_;
            }
            else
            {
                auto last = line_.find_last_not_of(" \t\v\f\r");

                if (last != std::string::npos)
                {
                    result_.append(line_, 0, last + 1);
                }
            }

            result_ += '\n';
        }

        line_.clear();
        line_has_text_ = false;
        line_is_protected_ = false;
    }

    void finish_last_line()
    {
        if (line_has_text_ || line_is_protected_)
        {
            if (line_is_protected_)
            {
                result_ += line_;
            }
            else
            {
                auto last = line_.find_last_not_of(" \t\v\f\r");

                if (last != std::string::npos)
                {
                    result_.append(line_, 0, last + 1);
                }
            }
        }
    }

    [[nodiscard]]
    std::size_t raw_prefix_size(std::size_t i) const
    {
        if (source_.substr(i, 4) == "u8R\"")
        {
            return 4;
        }

        if (source_.substr(i, 3) == "LR\"" ||
            source_.substr(i, 3) == "uR\"" ||
            source_.substr(i, 3) == "UR\"")
        {
            return 3;
        }

        if (source_.substr(i, 2) == "R\"")
        {
            return 2;
        }

        return 0;
    }

    [[nodiscard]]
    std::size_t copy_raw_string(std::size_t i, std::size_t prefix_size)
    {
        auto delimiter_begin = i + prefix_size;
        auto delimiter_end = delimiter_begin;

        while (delimiter_end < source_.size() &&
               source_[delimiter_end] != '(' &&
               delimiter_end - delimiter_begin <= 16)
        {
            if (!is_raw_delimiter_char(source_[delimiter_end]))
            {
                emit(source_[i]);
                return i + 1;
            }

            ++delimiter_end;
        }

        if (delimiter_end >= source_.size() ||
            source_[delimiter_end] != '(' ||
            delimiter_end - delimiter_begin > 16)
        {
            emit(source_[i]);
            return i + 1;
        }

        std::string delimiter(source_.substr(delimiter_begin,
                                             delimiter_end - delimiter_begin));

        std::string closer = ")" + delimiter + "\"";

        auto body_begin = delimiter_end + 1;
        auto closer_begin = source_.find(closer, body_begin);

        auto raw_end = closer_begin == std::string_view::npos
            ? source_.size()
            : closer_begin + closer.size();

        while (i < raw_end)
        {
            emit(source_[i], true);
            ++i;
        }

        return i;
    }

    [[nodiscard]]
    std::size_t copy_quoted_literal(std::size_t i)
    {
        auto quote = source_[i];

        emit(source_[i]);
        ++i;

        while (i < source_.size())
        {
            auto ch = source_[i];

            emit(ch);
            ++i;

            if (ch == '\\' && i < source_.size())
            {
                emit(source_[i]);
                ++i;
                continue;
            }

            if (ch == quote)
            {
                break;
            }
        }

        return i;
    }

    [[nodiscard]]
    std::size_t skip_line_comment(std::size_t i) const
    {
        i += 2;

        while (i < source_.size() && source_[i] != '\n')
        {
            ++i;
        }

        return i;
    }

    [[nodiscard]]
    std::size_t skip_block_comment(std::size_t i)
    {
        emit(' ');

        i += 2;

        while (i + 1 < source_.size())
        {
            if (source_[i] == '*' && source_[i + 1] == '/')
            {
                return i + 2;
            }

            ++i;
        }

        return source_.size();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////

std::string read_file(std::filesystem::path const& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("cannot open input file");
    }

    return {
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };
}

////////////////////////////////////////////////////////////////////////////////////////////

void write_file(std::filesystem::path const& path, std::string_view text)
{
    std::ofstream file(path, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("cannot open output file");
    }

    file << text;
}

////////////////////////////////////////////////////////////////////////////////////////////

void transform(std::filesystem::path const& source_path,
               std::filesystem::path const& output_path)
{
    auto source = read_file(source_path);

    source_cleaner cleaner(source);

    write_file(output_path, cleaner.run());
}

////////////////////////////////////////////////////////////////////////////////////////////

int main()
try
{
    auto path_1 = std::filesystem::path("source.cpp");
    auto path_2 = std::filesystem::path("output.cpp");

    transform(path_1, path_2);

    std::println("main : file '{}' created", path_2.string());
}
catch (std::exception const& error)
{
    std::println(stderr, "error : {}", error.what());
    return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////