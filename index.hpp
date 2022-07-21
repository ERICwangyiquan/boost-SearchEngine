#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include "util.hpp"
#include "log.hpp"

namespace ns_index
{

    struct DocInfo
    {
        std::string title;
        std::string content;
        std::string url;
        uint64_t doc_id; // doc's id
    };

    struct InvertedElem
    {
        uint64_t doc_id;
        std::string word;
        int weight;
        InvertedElem():weight(0){}
    };

    // inverted_list
    typedef std::vector<InvertedElem> InvertedList;

    class Index
    {
    private:
        // fowward_index uses the index of vector to be the index of the doc
        std::vector<DocInfo> forward_index;
        // inverted_index: one key to one/many InvertedElem
        std::unordered_map<std::string, InvertedList> inverted_index;

        static Index* instance;
        static std::mutex mtx;

    private:
        Index() {}
        Index(const Index&) = delete;
        Index& operator=(const Index&) = delete;

    public:
        ~Index() {}

    public:
        static Index* GetInstance()
        {
            if(nullptr == instance)
            {
                mtx.lock();
                if(nullptr == instance)
                {
                    instance = new Index();
                }
                mtx.unlock();
            }
            return instance;
        }

        // use doc_id to find doc
        DocInfo *GetForwardIndex(uint64_t doc_id)
        {
            if (doc_id >= forward_index.size())
            {
                std::cerr << "error : doc_id out of range" << std::endl;
                return nullptr;
            }
            return &forward_index[doc_id];
        }

        // use string to find inverted_list
        InvertedList *GetInvertedList(const std::string &word)
        {
            auto iter = inverted_index.find(word);
            if (iter == inverted_index.end())
            {
                std::cerr << word << "is not in InvertedList" << std::endl;
                return nullptr;
            }
            return &(iter->second);
        }

        // use off-labelled file (./data/raw_html/raw.txt) to build forward_index and inverted_index
        bool BuildIndex(const std::string &input) // input parsed data
        {
            std::ifstream in(input, std::ios::in | std::ios::binary);
            if (!in.is_open())
            {
                std::cerr << "sorry, " << input << " open error" << std::endl;
                return false;
            }

            std::string line;
            int count = 0;
            while (std::getline(in, line))
            {
                DocInfo *doc = BuildForwardIndex(line);
                if (nullptr == doc)
                {
                    std::cerr << "build " << line << "error" << std::endl; // for debug
                    continue;
                }

                BuildInvertedIndex(*doc);

                count++;
                // if(count % 50 == 0)
                // {
                //     std::cout << "already built index files: " << count << std::endl;
                // }
                LOG(NORMAL, "Currently built index docs: " + std::to_string(count));
            }
            return true;
        }

    private:
        DocInfo *BuildForwardIndex(const std::string &line)
        {
            // 1.parse line
            // line -> title content url
            std::vector<std::string> results;
            const std::string sep = "\3"; // seperator in line
            ns_util::StringUtil::Split(line, &results, sep);
            if (results.size() != 3)
            {
                return nullptr;
            }
            // 2.store in DocInfo
            DocInfo doc;
            doc.title = results[0];            // titile
            doc.content = results[1];          // content
            doc.url = results[2];              // url
            doc.doc_id = forward_index.size(); // set the id before insert
            // insert into forward index
            forward_index.push_back(std::move(doc));
            return &forward_index.back();
        }

        bool BuildInvertedIndex(const DocInfo &doc)
        {
            // DocInfo{titile, content, url, doc_id}
            // word -> inverted_index
            struct word_cnt
            {
                int title_cnt;
                int content_cnt;

                word_cnt() :title_cnt(0), content_cnt(0) {}
            };
            std::unordered_map<std::string, word_cnt> word_map; // store word <-> numbers it shows up

            // cut title
            std::vector<std::string> title_words;
            ns_util::JiebaUtil::CutString(doc.title, &title_words);

            // count words in title
            for(std::string s: title_words)
            {
                boost::to_lower(s); //for user search (hello/HELLO/Hello)
                (word_map[s].title_cnt)++;
            }


            // cut content
            std::vector<std::string> content_words;
            ns_util::JiebaUtil::CutString(doc.content, &content_words);


            // count words in content
            for(std::string s: content_words)
            {
                boost::to_lower(s);
                (word_map[s].content_cnt)++;
            }

#define X 10
#define Y 1
            for(auto &word_pair : word_map)
            {
                InvertedElem item;
                item.doc_id = doc.doc_id;
                item.word = word_pair.first;
                item.weight = X * word_pair.second.title_cnt + Y * word_pair.second.content_cnt;    //relativity
                inverted_index[word_pair.first].push_back(std::move(item));
            }

            return true;
        }
    };

    Index* Index::instance = nullptr;
    std::mutex Index::mtx;
}