#include "cpp-httplib-v0.7.15/httplib.h"
#include "searcher.hpp"
#include "util.hpp"

const std::string input = "data/raw_html/raw.txt";
const std::string root_path = "./wwwroot";

int main()
{
    ns_searcher::Searcher search;
    search.InitSearcher(input);

    httplib::Server svr;
    svr.set_base_dir(root_path.c_str());
    svr.Get("/s", [&search](const httplib::Request &req,  httplib::Response &rsp){
        if(!req.has_param("word"))
        {
            rsp.set_content("Please enter key-words to search", "text/plain; charset=utf-8");
            return;
        }
        std::string word = req.get_param_value("word");
        // std::cout << "user is searching: " << word << std::endl;
        LOG(NORMAL, "user searched: " + word);
        std::string json_string;
        search.Search(word, &json_string);
        rsp.set_content(json_string, "application/json");
    });

    LOG(NORMAL, "server started...");
    svr.listen("0.0.0.0", 8080);
    return 0;
}