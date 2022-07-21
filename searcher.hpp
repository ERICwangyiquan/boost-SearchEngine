#pragma once

#include "index.hpp"
#include "util.hpp"
#include "log.hpp"
#include <algorithm>
#include <unordered_map>
#include <json/json.h>


namespace ns_searcher
{

    struct InvertedElemPrint
    {
        uint64_t doc_id;
        int weight;
        std::vector<std::string> words;
        InvertedElemPrint():doc_id(0), weight(0){}
    };

    class Searcher
    {
    private:
        ns_index::Index *index; // for program to search
    public:
        Searcher() {}
        ~Searcher() {}

    public:
        void InitSearcher(const std::string &input)
        {
            // 1.get an Index instance
            index = ns_index::Index::GetInstance();
            // std::cout << "get index instance succeed" << std::endl;
            LOG(NORMAL, "get index instance success...");
            // 2.build index by instance
            index->BuildIndex(input);
            // std::cout << "build forward_index and inverted_index succeed" << std::endl;
            LOG(NORMAL, "build forward and inverted index success...");
        }

        // query: key word for searching
        // json_string: returns to user
        void Search(const std::string &query, std::string *json_string)
        {
            // 1. cut query
            std::vector<std::string> words;
            ns_util::JiebaUtil::CutString(query, &words);

            // 2.search words in inverted_index
            // ns_index::InvertedList inverted_list_all;
            std::vector<InvertedElemPrint> inverted_list_all;

            std::unordered_map<uint64_t, InvertedElemPrint> tokens_map;     //remove duplicates

            for (std::string word : words)
            {
                boost::to_lower(word);
                ns_index::InvertedList *inverted_list = index->GetInvertedList(word);
                if (nullptr == inverted_list)
                {
                    continue;
                }
                // inverted_list_all.insert(inverted_list_all.end(), inverted_list->begin(), inverted_list->end());

                for(const auto &elem : *inverted_list)  //remove duplicates
                {
                    auto &item = tokens_map[elem.doc_id];
                    item.doc_id = elem.doc_id;
                    item.weight += elem.weight;
                    item.words.push_back(elem.word);
                }
            }
            for(const auto &item : tokens_map)
            {
                inverted_list_all.push_back(std::move(item.second));
            }

            // 3.sort by weight         

            // std::sort(inverted_list_all.begin(), inverted_list_all.end(),
            //           [](const ns_index::InvertedElem &e1, const ns_index::InvertedElem &e2)        //TODO
            //           {
            //               return e1.weight > e2.weight;
            //           });

            std::sort(inverted_list_all.begin(), inverted_list_all.end(),\
                      [](const InvertedElemPrint &e1, const InvertedElemPrint &e2)
                      {
                          return e1.weight > e2.weight;
                      });

            // 4.construct Json string by jsoncpp-devel
            Json::Value root;
            Secret(&root);
            for (auto &item : inverted_list_all)
            {
                ns_index::DocInfo *doc = index->GetForwardIndex(item.doc_id);
                if(nullptr == doc)
                {
                    continue;
                }
                Json::Value elem;
                elem["title"] = doc->title;
                elem["desc"] = GetDesc(doc->content, item.words[0]); //part of whole content
                elem["url"] = doc->url;

                // for debug  for delete
                elem["id"] = (int)item.doc_id;
                elem["weight"] = item.weight;   //int -> string

                root.append(elem);
            }


            // Json::StyledWriter writer;
            Json::FastWriter writer;
            *json_string = writer.write(root);
        }

        std::string GetDesc(const std::string &html_content, const std::string &word)
        {
            // find first occur place of word, find 50 bytes before it (if not enough, from begin), 100 byte after it (if not enough, till end)
            const int prev_step = 50;
            const int next_step = 100;
            // 1. find it
            auto iter = std::search(html_content.begin(), html_content.end(), word.begin(), word.end(), [](int x, int y){
                return std::tolower(x) == std::tolower(y);
            });
            if(iter == html_content.end())
            {
                return "None1";
            }
            int pos = std::distance(html_content.begin(), iter);

            // 2.get start/end
            int start = 0;
            int end = html_content.size()-1;
            if(pos - prev_step > start) start = pos - prev_step;
            if(pos + next_step < end)   end = pos + next_step;

            // 3.return substr
            if(start >= end)
            {
                return "None2";
            }
            std::string desc = html_content.substr(start, end-start);
            desc += "...";
            return desc;
        }

        void Secret(Json::Value *root)
        {
            Json::Value secret;
            secret["title"] = "Eric's GitHub";
            secret["desc"] = "Click and see my other projects on GitHub"; //part of whole content
            secret["url"] = "https://github.com/ERICwangyiquan";
            secret["id"] = -1;
            secret["weight"] = -1; 
            (*root).append(secret);
        }
    };
}