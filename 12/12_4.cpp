/////////////////////////////////////////////////////////////////////

// chapter : Functions

/////////////////////////////////////////////////////////////////////

// section : Regular expressions

/////////////////////////////////////////////////////////////////////

// content : Extract all email addresses
//
// content : Extract email domains using group
//
// content : Raw string literals for tests
//
// content : Simplified email format

/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <print>
#include <regex>
#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////

struct email_info
{
    std::string address;
    std::string domain;
};

/////////////////////////////////////////////////////////////////////

std::vector<email_info> extract_emails(const std::string& text)
{
    const std::regex pattern
    {
        R"([A-Za-z0-9._%+-]+@([A-Za-z0-9.-]+\.[A-Za-z]{2,}))"
    };

    std::vector<email_info> result;

    auto begin = std::sregex_iterator(text.begin(), text.end(), pattern);
    auto end   = std::sregex_iterator();

    for (auto i = begin; i != end; ++i)
    {
        const std::smatch& match = *i;

        result.push_back
        ({
            .address = match.str(0),
            .domain  = match.str(1)
        });
    }

    return result;
}

/////////////////////////////////////////////////////////////////////

void test(const std::string& name, const std::string& text)
{
    std::println("{}", name);
    std::println("text:");
    std::println("{}\n", text);

    const auto emails = extract_emails(text);

    if (emails.empty())
    {
        std::println("emails : not found\n");
        return;
    }

    for (const auto& item : emails)
    {
        std::println("email  : {}", item.address);
        std::println("domain : {}", item.domain);
        std::println("");
    }
}

/////////////////////////////////////////////////////////////////////

int main()
{
    const std::string text_1 = R"(
        Write to support@example.com or admin@test.org.
        Also try user.name+tag@sub.domain.net.
    )";

    const std::string text_2 = R"(
        Invalid examples:
        user@
        @site.com
        hello@localhost

        Valid examples:
        maxim@mail.ru
        cpp.student_23@university.edu
    )";

    const std::string text_3 = R"(
        Contacts:
        sales@company.com, hr@company.com, dev.team@tools.io.
    )";

    test("test 1", text_1);
    test("test 2", text_2);
    test("test 3", text_3);
}

/////////////////////////////////////////////////////////////////////