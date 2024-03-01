//
// Created by 37496 on 2024/2/22.
//

#include "HttpUtil.h"
#include "http/util/EncodeUtil.h"

namespace Tiny_muduo::Http
{
    /**
     * const char *formstr = "key1=dddd&key2=%E9%92%A6%E4%BD%A9&key3=%A3%B4%C8&key4=ha%B7&hfdhgsdsaf";
     * parseKeyValue(formstr, "=", "&", form);
     *
     * */
    void parseKeyValue(const std::string& src, const std::string& join, const std::string& split,
                       HttpMap<CASE_SENSITIVE::YES>& map, bool urlDecode)
    {
        std::string_view view;
        std::string decoded_src;
        if(urlDecode) {
            decoded_src = EncodeUtil::urlDecode(src);
            view = decoded_src;
        }
        else {
            view = src;
        }

        while(!view.empty()) {
            size_t pos = view.find(split);
            std::string_view kv = view.substr(0, pos);
            size_t pos_k = kv.find(join);
            if(pos_k == kv.npos) {  //解析失败
                return;
            }
            std::string_view k = kv.substr(0, pos_k);
            std::string_view v = kv.substr(pos_k+join.size());
            map.add(std::string(k), std::string(v));

            if(pos == view.npos) {
                break;  //解析完毕
            }
            else {
                pos += split.size();
                while(pos < view.size() && view[pos] == ' ') {
                    ++pos;
                }
            }
            view.remove_prefix(pos);
        }
    }
}