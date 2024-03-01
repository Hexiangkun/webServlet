//
// Created by 37496 on 2024/2/22.
//

#ifndef WEBSERVER_HTTPMULTIPART_H
#define WEBSERVER_HTTPMULTIPART_H

#include <string>
#include <vector>
#include <memory>
#include "http/util/HttpMap.h"

namespace Tiny_muduo::Http
{

    class HttpMultiPart
    {
    public:
        struct Part
        {
            std::string name;
            std::string file_name;
            std::string file_type;
            std::string data;

            Part() = default;
            Part(std::string name, std::string fn, std::string ft, std::string data)
                    : name(std::move(name)), file_name(std::move(fn)),
                      file_type(std::move(ft)), data(std::move(data)) {}
        };
        typedef HttpBaseMap<std::string, std::vector<std::shared_ptr<Part>>> Files;
        typedef HttpBaseMap<std::string, std::string> Form;

        void setBoundary(const std::string &boundary) noexcept { _boundary = boundary; }
        const std::string &getBoundary() const noexcept { return _boundary; }

        ReturnOption<std::string> getValue(const std::string &name) const;

        std::shared_ptr<Part> getFile(const std::string &name, size_t index = 0) const;

        void parse(std::string_view body);

    private:
        enum State {
            start_body,
            start_boundary,
            end_boundary,
            start_content_disposition,
            end_content_disposition,
            start_content_type,
            end_content_type,
            start_content_data,
            end_content_data,
            end_body
        };

        std::string _boundary;
        Form _form;
        mutable Files _files;

        inline constexpr static char CR = '\r';
        inline constexpr static char LF = '\n';

    };
}


#endif //WEBSERVER_HTTPMULTIPART_H
