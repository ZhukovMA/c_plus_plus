#include <cstdio>

int main()
{
    const char* s = "#include <cstdio>%c%cint main()%c{%c    const char* s = %c%s%c;%c    std::printf(s, 10, 10, 10, 10, 34, s, 34, 10, 10, 10);%c}%c";
    std::printf(s, 10, 10, 10, 10, 34, s, 34, 10, 10, 10);
}