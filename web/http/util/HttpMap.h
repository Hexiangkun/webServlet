//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_HTTPMAP_H
#define WEBSERVER_HTTPMAP_H

#include <unordered_map>
#include "base/Util.h"
#include "http/util/ReturnOption.h"

namespace Tiny_muduo::Http
{
    enum class CASE_SENSITIVE{
        YES,        //键区分大小写
        NO          //键不区分大小写
    };


    template<typename K, typename V, CASE_SENSITIVE C = CASE_SENSITIVE::YES>
    class HttpBaseMap
    {
    public:
        typedef typename std::unordered_map<K, V>::iterator Iterator;
        typedef typename std::unordered_map<K, V>::const_iterator ConstIterator;
        
        virtual void add(const K& key, const V& val) {
            if(C == CASE_SENSITIVE::YES) {
                _map[key] = val;
            }
            else {
                _map[toLowers(key)] = val;
            }
        }
        
        virtual void del(const K& key) {
            ConstIterator it;
            if(C == CASE_SENSITIVE::YES) {
                it = _map.find(key);
            }
            else {
                it = _map.find(toLowers(key));
            }
            
            if(it != _map.end()) {
                _map.erase(it);
            }
        }
        
        virtual bool contain(const K& key) {
            ConstIterator it;
            if(C == CASE_SENSITIVE::YES) {
                it = _map.find(key);
            }
            else {
                it = _map.find(toLowers(key));
            }
            return it != _map.end();
        }
        
        virtual ReturnOption<V> get(const K& key) const {
            ConstIterator it;
            if(C == CASE_SENSITIVE::YES) {
                it = _map.find(key);
            }
            else {
                it = _map.find(toLowers(key));
            }
            if(it == _map.end()) {
                return {V(), false};
            }
            return {it->second, true};
        }
        
        ConstIterator begin() const { return _map.begin(); }
        Iterator begin() { return _map.begin(); }
        ConstIterator end() const { return _map.end(); }
        Iterator end() { return _map.end(); }
        
        virtual size_t size() const { return _map.size(); }
        virtual bool empty() const { return _map.empty(); }

    protected:
        std::unordered_map<K, V> _map;
    };
    
    template<CASE_SENSITIVE C = CASE_SENSITIVE::YES>
    using HttpMap = HttpBaseMap<std::string , std::string, C>;

    class HttpForm : public HttpMap<CASE_SENSITIVE::YES>
    {

    };
}
#endif //WEBSERVER_HTTPMAP_H
