#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include "cache.h"

namespace database
{
    class Person{
        private:
        long _id;
            std::string _first_name;
            std::string _last_name;
            std::string _login;
            unsigned int _age{0};

        public:

            static Person fromJSON(const std::string & str);

            long             get_id() const;
            const std::string &get_first_name() const;
            const std::string &get_last_name() const;
            const std::string &get_login() const;
            const unsigned int&get_age() const;

            long&        id();
            std::string &first_name();
            std::string &last_name();
            std::string &login();
            unsigned int &age();

            static void init();
            static Person read_by_login(std::string login);
            static Person read_by_id(long id);
            static Person read_from_cache_by_login(std::string login);

            static std::vector<Person> read_all();
            static std::vector<Person> search(std::string first_name, std::string last_name);
            void save_to_mysql();
            void save_to_cache();

            Poco::JSON::Object::Ptr toJSON() const;


    };
}

#endif