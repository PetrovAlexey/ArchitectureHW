#ifndef PERSONHANDLER_H
#define PERSONHANDLER_H

#define STRTOLOWER(x) std::transform (x.begin(), x.end(), x.begin(), [](unsigned char c){ return std::tolower(c); })

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/person.h"

class AuthorHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at leas 3 signs";
            return false;
        }

        if (name.find(' ') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        if (name.find('\t') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        return true;
    };

    bool check_login(const std::string &email, std::string &reason)
    {
        if (email.find(' ') != std::string::npos)
        {
            reason = "Login can't contain spaces";
            return false;
        }

        if (email.find('\t') != std::string::npos)
        {
            reason = "Login can't contain spaces";
            return false;
        }

        return true;
    };

public:
    AuthorHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        std::cout << "Handle request: " << request.getMethod() << std::endl;

        HTMLForm form(request, request.stream());
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        std::ostream &ostr = response.send();
        auto method = request.getMethod();

        STRTOLOWER(method);

        if (method == "post")
        {
            if (form.has("first_name"))
                if (form.has("last_name"))
                    if (form.has("login"))
                        if (form.has("age"))
                        {
                            database::Person person;
                            person.first_name() = form.get("first_name");
                            person.last_name() = form.get("last_name");
                            person.login() = form.get("login");
                            person.age() = stoi(form.get("age"));

                            std::cout << "Person constructed" << std::endl;

                            bool check_result = true;
                            std::string message;
                            std::string reason;

                            if (!check_name(person.get_first_name(), reason))
                            {
                                check_result = false;
                                message += reason;
                                message += "<br>";
                            }

                            if (!check_name(person.get_last_name(), reason))
                            {
                                check_result = false;
                                message += reason;
                                message += "<br>";
                            }

                            if (!check_login(person.get_login(), reason))
                            {
                                check_result = false;
                                message += reason;
                                message += "<br>";
                            }

                            if (check_result)
                            {
                                try
                                {
                                    person.save_to_mysql();
                                    person.save_to_cache();
                                    ostr << "{ \"result\": true }";
                                    ostr.flush();
                                    return;
                                }
                                catch (...)
                                {
                                    ostr << "{ \"result\": false , \"reason\": \" database error\" }";
                                    return;
                                }
                            }
                            else
                            {
                                ostr << "{ \"result\": false , \"reason\": \"" << message << "\" }";
                                ostr.flush();
                                return;
                            }
                        }
        } else if (method == "get") {
            if (form.has("login"))
            {
                std::cout << "Get person by login " << std::endl;
                std::string login = form.get("login");
                try {
                    std::cout << "Get item from cache" << std::endl;
                    database::Person result = database::Person::read_from_cache_by_login(login);
                    if (!result.login().empty()) {
                        Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
                    }
                    return;
                }
                catch (...)
                {
                    std::cout << "Some error on cache" << std::endl;
                }

                try
                {
                    database::Person result = database::Person::read_by_login(login);
                    result.save_to_cache();
                    Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
                    return;
                }
                catch (...)
                {
                    ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                    return;
                }
            }
            else if (form.has("first_name") && form.has("last_name"))
            {
                std::cout << "Searching person " << std::endl;
                try
                {
                    std::string  fn = form.get("first_name");
                    std::string  ln = form.get("last_name");
                    auto results = database::Person::search(fn, ln);
                    Poco::JSON::Array arr;
                    for (auto s : results)
                        arr.add(s.toJSON());
                    Poco::JSON::Stringifier::stringify(arr, ostr);
                }
                catch (...)
                {
                    ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                    return;
                }
                return;
            }

            std::cout << "Read all persons" << std::endl;
            auto results = database::Person::read_all();
            Poco::JSON::Array arr;
            for (auto s : results)
                arr.add(s.toJSON());
            Poco::JSON::Stringifier::stringify(arr, ostr);
        } else {
            ostr << "{ \"result\": false , \"reason\": \"method not found\" }";
        }
    }

private:
    std::string _format;
};
#endif // !PERSON_H