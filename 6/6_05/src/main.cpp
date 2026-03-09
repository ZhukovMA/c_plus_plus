#include <iostream>
#include <string>

#include <boost/dll.hpp>

namespace dll = boost::dll;

using test_signature = void();

int main()
{
    const dll::fs::path exe_dir = dll::program_location().parent_path();

    std::cout << "Enter lib name. To exit enter: exit\n\n";

#if defined(_WIN32)
    std::cout << "Example: test_v1.dll or test_v2.dll\n";
#else
    std::cout << "Example: libtest_v1.so or libtest_v2.so\n";
#endif

    std::string library_name;

    while (true)
    {
        std::cout << "\nLibrary name: ";

        if (!(std::cin >> library_name))
        {
            std::cerr << "Error reading from stdin\n";
            return 1;
        }

        if (library_name == "exit")
        {
            break;
        }

        try
        {
            dll::fs::path library_path(library_name);

            if (library_path.is_relative())
            {
                library_path = exe_dir / library_path;
            }

            auto test_function =
                dll::import_symbol<test_signature>(library_path, "test");

            std::cout << "Library loaded: " << library_path.string() << '\n';
            test_function();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to load library or import test: " << e.what() << '\n';
        }
    }

    return 0;
}