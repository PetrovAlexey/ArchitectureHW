//
// Created by petrov on 10/26/21.
//

#include "../database/person.h"
#include "../config/config.h"
#include "gtest/gtest.h"
#include "Poco/UUIDGenerator.h"

class SkipFixture : public ::testing::Test {
protected:
    void SetUp() override {
        Config::get().login() = "petrov";
        Config::get().password() = "petrov";
        Config::get().port() = "3306";
        Config::get().host() = "localhost";
        Config::get().database() = "hw";

        database::Person test;

        Poco::UUIDGenerator generator;

        test.login() = generator.create().toString();
        test.age() = 20;
        test.first_name() = "test";
        test.last_name() = "test";

        test.save_to_mysql();
    }
};

TEST_F(SkipFixture, basic_test) {
    auto result = database::Person::read_by_id(1);
    ASSERT_TRUE(result.get_id() == 1);
}

TEST_F(SkipFixture, login_test) {
    auto result = database::Person::read_by_login("test");
    ASSERT_TRUE(result.get_login() == "test");
}

TEST_F(SkipFixture, search_test) {
    auto result = database::Person::search("tes_", "tes_");
    for (auto& row : result) {
        ASSERT_TRUE(row.first_name().find("tes") == 0);
        ASSERT_TRUE(row.last_name().find("tes") == 0);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
