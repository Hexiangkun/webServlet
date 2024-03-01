//
// Created by 37496 on 2024/2/22.
//

#include <cstring>
#include "HttpMultiPart.h"


namespace Tiny_muduo::Http
{
    ReturnOption<std::string> HttpMultiPart::getValue(const std::string &name) const {
        return _form.get(name);
    }

    Part *HttpMultiPart::getFile(const std::string &name, size_t index) const {
        Part* p = nullptr;
        auto it = _files.get(name);
        if(it.exist()) {
            if(index >= it.value().size()) {
                return p;
            }
            p =&(it.value().at(index));
        }
        return p;
    }

    /**
POST http://www.example.com HTTP/1.1
Content-Type:multipart/form-data; boundary=----WebKitFormBoundaryrGKCBY7qhFd3TrwA

------WebKitFormBoundaryrGKCBY7qhFd3TrwA
Content-Disposition: form-data; name="text"

title
------WebKitFormBoundaryrGKCBY7qhFd3TrwA
Content-Disposition: form-data; name="file"; filename="chrome.png"
Content-Type: image/png

PNG ... content of chrome.png ...
------WebKitFormBoundaryrGKCBY7qhFd3TrwA--

     * */

    void HttpMultiPart::parse(std::string_view body) {
        size_t i = 0;
        size_t boundary_len = _boundary.size();
        size_t len = body.size();
        std::string name, filename,type,form_file_data;

        bool file_mark = false;
        const char* boundary_s = _boundary.data();

        State state = start_body;

        static constexpr const size_t buf_size = 4096;
        char buffer[buf_size];

        while(i < len) {
            switch (state) {
                case start_body:
                {
                    // boundary begin --boundary
                    if(i + 1 < len && body[i] == '-' && body[i+1] == '-') {
                        i+=2;
                        state = start_boundary;
                    }
                    break;
                }
                case start_boundary:
                {
                    if(::strncmp(body.data()+i, boundary_s, boundary_len) == 0) {
                        i += boundary_len;
                        // --boundary\r\n
                        if(i+1 < len && body[i] == CR && body[i+1] ==LF) {
                            i += 2;
                            state = end_boundary;
                        }
                            // --boundary--
                        else if(i+1 < len && body[i] == '-' && body[i+1] == '-') {
                            i +=2;
                            state = end_body;
                        }
                        else {
                            //bad body
                        }
                    }
                    break;
                }
                case end_boundary:
                {
                    if(::strncasecmp(body.data()+i, "Content-Disposition", 19) == 0) {
                        i += 19;    //skip Content-Disposition
                        state = start_content_disposition;
                    }
                    else {
                        //bad body
                    }
                    break;
                }
                case start_content_disposition:
                {
                    i += 13;        //// skip ": form-data; "

                    bool start_name_filename = false;
                    bool is_name = true;
                    while(i < len) {
                        if(i+1 < len && body[i] == CR && body[i+1] == LF) {
                            i += 2;
                            state = end_content_disposition;
                            break;
                        }
                        if(i+1 < len && body[i] == '=' && body[i+1] == '\"') {
                            i += 2;
                            start_name_filename = true;
                        }
                        else if(body[i] == '\"') {
                            start_name_filename = false;
                            is_name = !is_name;
                            i++;
                        }
                        else if(start_name_filename) {
                            if(is_name) {
                                name += body[i++];
                            }
                            else {
                                filename += body[i++];
                            }
                        }
                        else {
                            i++;
                        }
                    }
                    break;
                }
                case end_content_disposition:
                {
                    if(i+1 < len && body[i] == CR && body[i+1] == LF) {
                        i += 2;
                        file_mark = false;
                        state = start_content_data;
                    }
                    else {
                        if(::strncasecmp(body.data()+i, "Content-Type", 12) == 0) {
                            i += 14;    //skip "Content-Type: "
                            file_mark = true;
                            state = start_content_type;
                        }
                        else {
                            // bad body
                        }
                    }
                    break;
                }
                case start_content_type:
                {
                    while (i < len) {
                        if(i+1 < len && body[i] == CR && body[i+1] == LF) {
                            i += 2;
                            state = end_content_type;
                            break;
                        }
                        else {
                            type += body[i++];
                        }
                    }
                    break;
                }
                case end_content_type:
                {
                    if(i+1 < len && body[i] == CR && body[i+1] == LF) {
                        i += 2;
                        state = start_content_data;
                    }
                    else {
                        // bad body
                    }
                    break;
                }

                case start_content_data:
                {
                    form_file_data.reserve(buf_size);
                    size_t k = 0;
                    size_t ix = 1;
                    size_t pi = i;

                    while(i < len) {
                        if(i+4 < len && body[i] == CR && body[i+1] == LF &&
                                body[i+2] == '-' && body[i+3] == '-' &&
                                ::strncmp(body.data()+i+4, boundary_s, boundary_len) == 0) {
                            if(k != 0) {
                                form_file_data.append(buffer, k);
                            }
                            form_file_data.resize(i-pi);
                            i += 2;
                            state = end_content_data;
                            break;
                        }
                        else {
                            buffer[k++] = body[i++];
                            if(k >= buf_size) {
                                k = 0;
                                ix++;
                                form_file_data.append(buffer, buf_size);
                                form_file_data.reserve(buf_size*ix);
                            }
                        }
                    }
                    break;
                }

                case end_content_data:
                {
                    if(!file_mark) {
                        _form.add(name, form_file_data);
                    }
                    else {
                        auto x = _files.get(name);
                        if(x.exist()) {
                            x.value().emplace_back(std::move(name), std::move(filename), std::move(type), std::move(form_file_data));
                        }
                        else {
                            _files.add(name, std::move(std::vector{Part(name, filename, type, form_file_data)}));
                        }
                    }

                    name = filename = type = form_file_data = "";
                    file_mark = false;
                    state = start_body;
                    break;
                }
                case end_body:
                {
                    i += 2;
                    break;
                }
                default: {
                    i++;
                    break;
                }
            }
        }
    }
}