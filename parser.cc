#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "util.hpp"

// "data/input" has all html pages
const std::string src_path = "data/input";
const std::string output = "data/raw_html/raw.txt";

typedef struct DocInfo
{
    std::string title;   // doc's title
    std::string content; // doc's body text
    std::string url;     // doc's URL in the website
} DocInfo_t;

bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list);
bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results);
bool SaveHtml(const std::vector<DocInfo_t> &results, const std::string &output);

int main()
{
    std::vector<std::string> files_list;
    // 1.recursively put every .html's path into files_list for the later read
    if (!EnumFile(src_path, &files_list))
    {
        std::cerr << "enum file name error!" << std::endl;
        return 1;
    }

    // 2.read and parse every files in files_list
    std::vector<DocInfo_t> results;
    if (!ParseHtml(files_list, &results))
    {
        std::cerr << "parse html error!" << std::endl;
        return 2;
    }

    // 3.write each parsed DocInfo_t into output, use '\3' as stop sign
    if (!SaveHtml(results, output))
    {
        std::cerr << "save html error!" << std::endl;
        return 3;
    }

    return 0;
}

bool EnumFile(const std::string &src_path, std::vector<std::string> *files_list)
{
    namespace fs = boost::filesystem;
    fs::path root_path(src_path);

    // state if the path exists
    if (!fs::exists(root_path))
    {
        std::cerr << src_path << " not exists" << std::endl;
        return false;
    }

    // create a new iterator to state the end of the recursion
    fs::recursive_directory_iterator end;
    for (fs::recursive_directory_iterator iter(root_path); iter != end; iter++)
    {
        // state if a file is a regular file (.html is regular file)
        if (!fs::is_regular_file(*iter))
        {
            continue;
        }
        if (iter->path().extension() != ".html")
        { // see if its a .html file
            continue;
        }

        // std::cout << "debug: " << iter->path().string() << std::endl;
        // now it must be a regular && .html file
        files_list->push_back(iter->path().string());
    }

    return true;
}

static bool ParseTitle(const std::string &file, std::string *title)
{
    std::size_t begin = file.find("<title>");
    if (begin == std::string::npos)
    {
        return false;
    }
    std::size_t end = file.find("</title>");
    if (end == std::string::npos)
    {
        return false;
    }

    begin += std::string("<title>").size();

    if (begin > end)
    {
        return false;
    }
    *title = file.substr(begin, end - begin);
    return true;
}

static bool ParseContent(const std::string &file, std::string *content)
{
    // take off the label, create a simple finite-state machine
    enum status
    {
        LABLE,
        CONTENT
    };

    enum status s = LABLE;
    for (char c : file)
    {
        switch (s)
        {
        case LABLE:
            if (c == '>')
                s = CONTENT;
            break;
        case CONTENT:
            if (c == '<')
                s = LABLE;
            else
            {
                if (c == '\n')
                    c = ' ';
                content->push_back(c);
            }
            break;
        default:
            break;
        }
    }
    return true;
}

static bool ParseUrl(const std::string &file_path, std::string *url)
{
    std::string url_head = "http://www.boost.org/doc/libs/1_78_0/doc/html";
    std::string url_tail = file_path.substr(src_path.size());

    *url = url_head + url_tail;
    return true;
}

// for debug
static void ShowDoc(const DocInfo_t &doc)
{
    std::cout << "title: " << doc.title << std::endl;
    std::cout << "content: " << doc.content << std::endl;
    std::cout << "URL: " << doc.url << std::endl;
}

bool ParseHtml(const std::vector<std::string> &files_list, std::vector<DocInfo_t> *results)
{
    for (const std::string &file : files_list)
    {
        // 1. read file
        std::string result;
        if (!ns_util::FileUtil::ReadFile(file, &result))
        {
            continue;
        }
        DocInfo_t doc;
        // 2. parse and take title out
        if (!ParseTitle(result, &doc.title))
        {
            continue;
        }
        // 3. parse and take content out
        if (!ParseContent(result, &doc.content))
        {
            continue;
        }
        // 4. create URL
        if (!ParseUrl(file, &doc.url))
        {
            continue;
        }

        // done
        results->push_back(std::move(doc)); // or it will copy itself, low efficiency

        // for debug
        // ShowDoc(doc);
        // break;
    }
    return true;
}

bool SaveHtml(const std::vector<DocInfo_t> &results, const std::string &output)
{
#define SEP '\3'
    // write in binary
    std::ofstream out(output, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "open " << output << " failed!" << std::endl;
        return false;
    }

    for (auto &item : results)
    {
        std::string out_string;
        out_string = item.title;
        out_string += SEP;
        out_string += item.content;
        out_string += SEP;
        out_string += item.url;
        out_string += '\n';

        out.write(out_string.c_str(), out_string.size());
    }

    out.close();

    return true;
}