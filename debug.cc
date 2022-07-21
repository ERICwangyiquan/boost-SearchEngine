#include "searcher.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

const std::string input = "data/raw_html/raw.txt";

int main()
{
    // for test
    ns_searcher::Searcher *search = new ns_searcher::Searcher();
    search->InitSearcher(input);

    std::string query;
    std::string json_string;
    while(true)
    {
        std::cout << "Please Enter Your Search Query# ";
        getline(std::cin, query);
        search->Search(query, &json_string);
        std::cout << json_string<< std::endl;
    }
    return 0;
}