#include "jjson/json.h"

#include <gtest/gtest.h>

using namespace jjson;

struct MyRect {
  int x;
  int y;
  int w;
  int h;
};

namespace jjson {

  template <>
    auto json_to(Json const &value) -> MyRect {
      return MyRect{
        value["x"].get_or_throw<int>(),
        value["y"].get_or_throw<int>(),
        value["w"].get_or_throw<int>(),
        value["h"].get_or_throw<int>()};
    }

  template <>
    auto json_to(Json const &value) -> std::vector<MyRect> {
      auto values = value.get_or_throw<jArray>();
      std::vector<MyRect> result;

      for (auto &i: values) {
        result.push_back(json_to<MyRect>(i));
      }

      return result;
    }

  template <>
    auto json_from(MyRect const &value) -> Json {
      return {
        {"x", value.x},
        {"y", value.y},
        {"w", value.w},
        {"h", value.h}};
    }

  template <>
    auto json_from(std::vector<MyRect> const &value) -> Json {
      jArray result;

      for (auto &i: value) {
        result.push_back(json_from<MyRect>(i));
      }

      return result;
    }

}

TEST(JsonSuite, PrimitiveTypes) {
  Json j1;
  Json j2{};
  Json j3{nullptr};
  Json j4{true};
  Json j5{42};
  Json j6{3.14};
  Json j7{"Hello, world !"};
  Json j8{
      jArray{nullptr, true, 42, 3.14}
  };
  Json j9{
    {"key1", "value1"},
    {"key2", "value2"},
    {"key3", // "key3": [1234, 4321]
      jArray{1234, 4321}
    },
    {"key4", { // "key4": {"number1": 1234, "number2": 4321}
      {"number1", 1234},
      {"number2", 4321}}
    }
  };

  jObject object{
    {"key1", nullptr},
    {"key2", true},
    {"key3", 42},
    {"key4", 3.14},
    {"key5", jArray{1, 2, 3, 4, 5}},
    {"key6", {}}
  };

  Json j10{object};

  ASSERT_EQ(j1.get_type(), JsonType::Null);
  ASSERT_EQ(j2.get_type(), JsonType::Null);
  ASSERT_EQ(j3.get_type(), JsonType::Null);
  ASSERT_EQ(j4.get_type(), JsonType::Bool);
  ASSERT_EQ(j5.get_type(), JsonType::Int);
  ASSERT_EQ(j6.get_type(), JsonType::Float);
  ASSERT_EQ(j7.get_type(), JsonType::String);
  ASSERT_EQ(j8.get_type(), JsonType::Array);
  ASSERT_EQ(j9.get_type(), JsonType::Object);
  ASSERT_EQ(j10.get_type(), JsonType::Object);

  ASSERT_EQ((j1 = nullptr), JsonType::Null);
  ASSERT_EQ((j1 = true), JsonType::Bool);
  ASSERT_EQ((j1 = 42), JsonType::Int);
  ASSERT_EQ((j1 = 3.14), JsonType::Float);
  ASSERT_EQ((j1 = jArray{1, 2, 3}), JsonType::Array);
  ASSERT_EQ((j1 = jObject{{"key", "value"}}), JsonType::Object);

  Json j11{std::array{1, 2, 3, 4, 5}};
  
  ASSERT_EQ(Json{}.get<std::nullptr_t>(), nullptr);
  ASSERT_EQ(Json{true}.get<bool>(), true);
  ASSERT_EQ(Json{42}.get<int>(), 42);
  ASSERT_EQ(Json{1.2f}.get<float>(), 1.2f);
  ASSERT_EQ((*Json{jArray{1, 2, 3, 4, 5}}.get<jArray>()).size(), 5);
}

TEST(JsonSuite, PrimitiveOptional) {
  std::optional<Json> j1{nullptr};
  std::optional<Json> j2{true};
  std::optional<Json> j3{42};
  std::optional<Json> j4{3.14};
  std::optional<Json> j5{"Hello, world !"};
  std::optional<Json> j6{jArray{
    nullptr, true, 42, 3.14}};
  std::optional<Json> j7{jObject{
    {"key1", "value1"}}};

  ASSERT_EQ(j1.value(), JsonType::Null);
  ASSERT_EQ(j2.value(), JsonType::Bool);
  ASSERT_EQ(j3.value(), JsonType::Int);
  ASSERT_EQ(j4.value(), JsonType::Float);
  ASSERT_EQ(j5.value(), JsonType::String);
  ASSERT_EQ(j6.value(), JsonType::Array);
  ASSERT_EQ(j7.value(), JsonType::Object);
}

TEST(JsonSuite, CustomTypes) {
  Json j1{
    {"x", 10},
    {"y", 20},
    {"w", 30},
    {"h", 40}};

  auto rect1 = j1.get<MyRect>();

  ASSERT_TRUE(rect1);

  Json j2{jArray{
    {{"x", 0}, {"y", 0}, {"w", 10}, {"h", 10}},
    {{"x", 0}, {"y", 0}, {"w", 20}, {"h", 20}},
    {{"x", 0}, {"y", 0}, {"w", 30}, {"h", 30}}}};

  auto rect2 = j2.get<std::vector<MyRect>>();

  ASSERT_TRUE(rect2);
  ASSERT_EQ((*rect2).size(), 3);

  Json j3{*rect1};
  Json j4{*rect2};
}

TEST(JsonSuite, ParseFormated) {
  std::istringstream s1{"A"};
  std::istringstream s2{""};
  std::istringstream s3{"false"};
  std::istringstream s4{"true"};
  std::istringstream s5{"0"};
  std::istringstream s6{"1234"};
  std::istringstream s7{"0b1010"};
  std::istringstream s8{"01010"};
  std::istringstream s9{"0x1010"};
  std::istringstream s10{"."};
  std::istringstream s11{"0."};
  std::istringstream s12{".0"};
  std::istringstream s13{"1.0"};
  std::istringstream s14{"1.234e+2"};
  std::istringstream s15{"\"Hello, world\""};
  std::istringstream s16{R"("Hello, world")"};
  std::istringstream s17{"[]"};
  std::istringstream s18{"[1234]"};
  std::istringstream s19{"[null, 1234, true, \"Hello, world\"]"};
  std::istringstream s20{"{}"};
  std::istringstream s21{R"({"key": null})"};
  std::istringstream s22{R"({"key": false})"};
  std::istringstream s23{R"({"key": true})"};
  std::istringstream s24{R"({"key": 1234})"};
  std::istringstream s25{R"({"key": 3.14})"};
  std::istringstream s26{R"({"key": 1.23e2})"};
  std::istringstream s27{R"({"key": "Hello, world"})"};
  std::istringstream s28{R"({"key": []})"};
  std::istringstream s29{R"({"key": {}})"};
  std::istringstream s30{R"({"key": {"key": "value"}})"};

  ASSERT_EQ(Json::parse(s1), std::nullopt);
  ASSERT_EQ(Json::parse(s2), JsonType::Null);
  ASSERT_EQ(Json::parse(s3), JsonType::Bool);
  ASSERT_EQ(Json::parse(s4), JsonType::Bool);
  ASSERT_EQ(Json::parse(s5), JsonType::Int);
  ASSERT_EQ(Json::parse(s6), JsonType::Int);
  ASSERT_EQ(Json::parse(s7), JsonType::Int);
  ASSERT_EQ(Json::parse(s8), JsonType::Int);
  ASSERT_EQ(Json::parse(s9), JsonType::Int);
  ASSERT_EQ(Json::parse(s10), JsonType::Float);
  ASSERT_EQ(Json::parse(s11), JsonType::Float);
  ASSERT_EQ(Json::parse(s12), JsonType::Float);
  ASSERT_EQ(Json::parse(s13), JsonType::Float);
  ASSERT_EQ(Json::parse(s14), JsonType::Float);
  ASSERT_EQ(Json::parse(s15), JsonType::String);
  ASSERT_EQ(Json::parse(s16), JsonType::String);
  ASSERT_EQ(Json::parse(s17), JsonType::Array);
  ASSERT_EQ(Json::parse(s18), JsonType::Array);
  ASSERT_EQ(Json::parse(s19), JsonType::Array);
  ASSERT_EQ(Json::parse(s20), JsonType::Object);
  ASSERT_EQ(Json::parse(s21), JsonType::Object);
  ASSERT_EQ(Json::parse(s22), JsonType::Object);
  ASSERT_EQ(Json::parse(s23), JsonType::Object);
  ASSERT_EQ(Json::parse(s24), JsonType::Object);
  ASSERT_EQ(Json::parse(s25), JsonType::Object);
  ASSERT_EQ(Json::parse(s26), JsonType::Object);
  ASSERT_EQ(Json::parse(s27), JsonType::Object);
  ASSERT_EQ(Json::parse(s28), JsonType::Object);
  ASSERT_EQ(Json::parse(s29), JsonType::Object);
  ASSERT_EQ(Json::parse(s30), JsonType::Object);
}

TEST(JsonSuite, ParseSpaced) {
  std::istringstream s1{" A "};
  std::istringstream s2{"  "};
  std::istringstream s3{" false "};
  std::istringstream s4{" true "};
  std::istringstream s5{" 0 "};
  std::istringstream s6{" 1234 "};
  std::istringstream s7{" 0b1010 "};
  std::istringstream s8{" 01010 "};
  std::istringstream s9{" 0x1010 "};
  std::istringstream s10{" . "};
  std::istringstream s11{" 0. "};
  std::istringstream s12{" .0 "};
  std::istringstream s13{" 1.0 "};
  std::istringstream s14{" 1.234e+2 "};
  std::istringstream s15{" \"Hello, world\" "};
  std::istringstream s16{R"( "Hello, world" )"};
  std::istringstream s17{" [ ] "};
  std::istringstream s18{" [ 1234 ] "};
  std::istringstream s19{" [null , 1234 , true , \"Hello, world\" ] "};
  std::istringstream s20{" { } "};
  std::istringstream s21{R"( { "key" : null } )"};
  std::istringstream s22{R"( { "key" : false } )"};
  std::istringstream s23{R"( { "key" : true } )"};
  std::istringstream s24{R"( { "key" : 1234 } )"};
  std::istringstream s25{R"( { "key" : 3.14 } )"};
  std::istringstream s26{R"( { "key" : 1.23e2 } )"};
  std::istringstream s27{R"( { "key" : "Hello, world" } )"};
  std::istringstream s28{R"( { "key" : [ ] } )"};
  std::istringstream s29{R"( { "key" : { } } )"};
  std::istringstream s30{R"( { "key" : { "key" : "value" } } )"};

  ASSERT_EQ(Json::parse(s1), std::nullopt);
  ASSERT_EQ(Json::parse(s2), JsonType::Null);
  ASSERT_EQ(Json::parse(s3), JsonType::Bool);
  ASSERT_EQ(Json::parse(s4), JsonType::Bool);
  ASSERT_EQ(Json::parse(s5), JsonType::Int);
  ASSERT_EQ(Json::parse(s6), JsonType::Int);
  ASSERT_EQ(Json::parse(s7), JsonType::Int);
  ASSERT_EQ(Json::parse(s8), JsonType::Int);
  ASSERT_EQ(Json::parse(s9), JsonType::Int);
  ASSERT_EQ(Json::parse(s10), JsonType::Float);
  ASSERT_EQ(Json::parse(s11), JsonType::Float);
  ASSERT_EQ(Json::parse(s12), JsonType::Float);
  ASSERT_EQ(Json::parse(s13), JsonType::Float);
  ASSERT_EQ(Json::parse(s14), JsonType::Float);
  ASSERT_EQ(Json::parse(s15), JsonType::String);
  ASSERT_EQ(Json::parse(s16), JsonType::String);
  ASSERT_EQ(Json::parse(s17), JsonType::Array);
  ASSERT_EQ(Json::parse(s18), JsonType::Array);
  ASSERT_EQ(Json::parse(s19), JsonType::Array);
  ASSERT_EQ(Json::parse(s20), JsonType::Object);
  ASSERT_EQ(Json::parse(s21), JsonType::Object);
  ASSERT_EQ(Json::parse(s22), JsonType::Object);
  ASSERT_EQ(Json::parse(s23), JsonType::Object);
  ASSERT_EQ(Json::parse(s24), JsonType::Object);
  ASSERT_EQ(Json::parse(s25), JsonType::Object);
  ASSERT_EQ(Json::parse(s26), JsonType::Object);
  ASSERT_EQ(Json::parse(s27), JsonType::Object);
  ASSERT_EQ(Json::parse(s28), JsonType::Object);
  ASSERT_EQ(Json::parse(s29), JsonType::Object);
  ASSERT_EQ(Json::parse(s30), JsonType::Object);
}

TEST(JsonSuite, Huge) {
  std::istringstream s{R"({
     "items":{
        "item":[
           {
              "id":"0001",
              "type":"donut",
              "name":"Cake",
              "ppu":0.55,
              "batters":{
                 "batter":[
                    {
                       "id":"1001",
                       "type":"Regular"
                    },
                    {
                       "id":"1002",
                       "type":"Chocolate"
                    },
                    {
                       "id":"1003",
                       "type":"Blueberry"
                    },
                    {
                       "id":"1004",
                       "type":"Devil's Food"
                    }
                 ]
              },
              "topping":[
                 {
                    "id":"5001",
                    "type":"None"
                 },
                 {
                    "id":"5002",
                    "type":"Glazed"
                 },
                 {
                    "id":"5005",
                    "type":"Sugar"
                 },
                 {
                    "id":"5007",
                    "type":"Powdered Sugar"
                 },
                 {
                    "id":"5006",
                    "type":"Chocolate with Sprinkles"
                 },
                 {
                    "id":"5003",
                    "type":"Chocolate"
                 },
                 {
                    "id":"5004",
                    "type":"Maple"
                 }
              ]
           }
        ]
     }
  })"};

  auto valueOpt = Json::parse(s);

  ASSERT_TRUE(valueOpt);

  auto value = *valueOpt;

  ASSERT_EQ(value["items"]["item"][0]["batters"]["batter"].get<jArray>().value().size(), 4);
}

