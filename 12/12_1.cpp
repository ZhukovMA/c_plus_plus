#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <locale>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

std::string trim(std::string text) {
    auto is_not_space = [](unsigned char ch) {
        return !std::isspace(ch);
    };

    text.erase(text.begin(), std::find_if(text.begin(), text.end(), is_not_space));
    text.erase(std::find_if(text.rbegin(), text.rend(), is_not_space).base(), text.end());

    return text;
}

bool starts_with(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
}

bool ends_with(std::string_view text, std::string_view suffix) {
    return text.size() >= suffix.size()
        && text.substr(text.size() - suffix.size()) == suffix;
}

bool parse_money_value(
    const std::string& text,
    const std::locale& input_locale,
    long double& minor_units
) {
    std::stringstream input_stream(text);
    input_stream.imbue(input_locale);

    input_stream >> std::showbase >> std::get_money(minor_units, true);
    input_stream >> std::ws;

    return !input_stream.fail() && input_stream.eof();
}

std::optional<long double> read_rub_amount(
    const std::string& text,
    const std::locale& input_locale
) {
    const std::string currency_code = "RUB";
    const std::string source = trim(text);

    std::vector<std::string> variants;
    variants.push_back(source);

    if (starts_with(source, currency_code)) {
        std::string amount = trim(source.substr(currency_code.size()));
        variants.push_back(amount + " " + currency_code);
    }

    if (ends_with(source, currency_code)) {
        std::string amount = trim(source.substr(0, source.size() - currency_code.size()));
        variants.push_back(amount + " " + currency_code);
    }

    for (const auto& variant : variants) {
        long double minor_units{};

        if (parse_money_value(variant, input_locale, minor_units)) {
            return minor_units;
        }
    }

    return std::nullopt;
}

long double read_exchange_rate(std::string text) {
    text = trim(text);
    std::replace(text.begin(), text.end(), ',', '.');

    std::size_t parsed_chars{};
    long double exchange_rate = std::stold(text, &parsed_chars);

    if (parsed_chars != text.size() || exchange_rate <= 0) {
        throw std::invalid_argument("Invalid exchange rate");
    }

    return exchange_rate;
}

std::string format_usd_amount(
    long double minor_units,
    const std::locale& output_locale
) {
    std::stringstream output_stream;
    output_stream.imbue(output_locale);

    output_stream << std::showbase << std::put_money(minor_units, true);

    return output_stream.str();
}

int main() {
    try {
        const std::locale input_locale("ru_RU.utf8");
        const std::locale output_locale("en_US.utf8");

        std::string amount_text;
        std::string rate_text;

        std::cout << "Enter RUB amount: ";
        std::getline(std::cin, amount_text);

        std::cout << "Enter exchange rate, RUB per USD: ";
        std::getline(std::cin, rate_text);

        const auto rub_minor_units = read_rub_amount(amount_text, input_locale);

        if (!rub_minor_units.has_value()) {
            std::cerr << "Error: invalid RUB amount format\n";
            return 1;
        }

        const long double rub_per_usd = read_exchange_rate(rate_text);

        const long double usd_minor_units = std::round(*rub_minor_units / rub_per_usd);

        std::cout << "Result: "
                  << format_usd_amount(usd_minor_units, output_locale)
                  << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}