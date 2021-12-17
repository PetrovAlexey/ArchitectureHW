#include "person.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <cppkafka/cppkafka.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void Person::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session_write();
            //*
            Statement drop_stmt(session);
            drop_stmt << "DROP TABLE IF EXISTS Person", now;
            //*/

            // (re)create table
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `Person` (`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`login` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL UNIQUE,"
                        << "`age` INTEGER NULL,"
                        << "PRIMARY KEY (`id`),KEY `lg` (`login`), KEY `fn` (`first_name`),KEY `ln` (`last_name`));",
                now;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr Person::toFullJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("login", _login);
        root->set("age", _age);

        return root;
    }

    Poco::JSON::Object::Ptr Person::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        //root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("login", _login);
        root->set("age", _age);

        return root;
    }

    Person Person::fromJSON(const std::string &str)
    {
        Person person;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        person.id() = object->getValue<long>("id");
        person.first_name() = object->getValue<std::string>("first_name");
        person.last_name() = object->getValue<std::string>("last_name");
        person.login() = object->getValue<std::string>("login");
        person.age() = object->getValue<unsigned int>("age");

        return person;
    }

    Person Person::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Poco::Data::Statement select(session);
            Person a;
            select << "SELECT id, first_name, last_name, login, age FROM Person where id=?",
                    into(a._id),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._login),
                    into(a._age),
                    use(id),
                    range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }

            return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Person Person::read_by_login(std::string login)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Poco::Data::Statement select(session);
            Person a;
            select << "SELECT first_name, last_name, login, age FROM Person where login=?",
                into(a._first_name),
                into(a._last_name),
                into(a._login),
                into(a._age),
                use(login),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }

            if (a.get_login().empty()) {
                throw Poco::Data::MySQL::StatementException("Data not found");
            }

            return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<Person> Person::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Statement select(session);
            std::vector<Person> result;
            Person a;
            select << "SELECT id, first_name, last_name, login, age FROM Person",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._login),
                into(a._age),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                select.execute();
                if (!a.get_login().empty()) {
                    result.push_back(a);
                }
            }

            if (result.empty()) {
                throw Poco::Data::MySQL::StatementException("Data not found");
            }

            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << e.message() << std::endl;
            throw;
        }
    }

    std::vector<Person> Person::search(std::string first_name, std::string last_name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session_read();
            Statement select(session);
            std::vector<Person> result;
            Person a;
            first_name+="%";
            last_name+="%";
            select << "SELECT id, first_name, last_name, login, age FROM Person where first_name LIKE ? and last_name LIKE ?",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._login),
                into(a._age),
                use(first_name),
                use(last_name),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                select.execute();
                if (!a.get_login().empty()) {
                    result.push_back(a);
                }
            }

            if (result.empty()) {
                throw Poco::Data::MySQL::StatementException("Data not found");
            }

            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

   
    void Person::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session_write();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Person (login, first_name,last_name,age) VALUES(?, ?, ?, ?)",
                use(_login),
                use(_first_name),
                use(_last_name),
                use(_age);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << e.message() << std::endl;
            throw;
        }
    }

    long Person::get_id() const
    {
        return _id;
    }

    const std::string &Person::get_first_name() const
    {
        return _first_name;
    }

    const std::string &Person::get_last_name() const
    {
        return _last_name;
    }

    const std::string &Person::get_login() const
    {
        return _login;
    }

    const unsigned int &Person::get_age() const
    {
        return _age;
    }

    long &Person::id()
    {
        return _id;
    }

    std::string &Person::first_name()
    {
        return _first_name;
    }

    std::string &Person::last_name()
    {
        return _last_name;
    }

    std::string &Person::login()
    {
        return _login;
    }

    unsigned int &Person::age()
    {
        return _age;
    }

    void Person::send_to_queue() {
        cppkafka::Configuration config = {
                {"metadata.broker.list", Config::get().get_queue_host()}};

        cppkafka::Producer producer(config);
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toFullJSON(), ss);
        std::string message = ss.str();
        producer.produce(cppkafka::MessageBuilder(Config::get().get_queue_topic()).partition(0).payload(message));
        producer.flush();
    }

}