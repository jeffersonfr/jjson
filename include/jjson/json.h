#pragma once

#include <variant>
#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <optional>
#include <charconv>
#include <cctype>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <istream>
#include <iterator>
#include <unordered_map>

namespace jjson {
  class Json;
}

namespace jjson {

  enum class JsonType {
    Null,
    Bool,
    Integer,
    Decimal,
    Text,
    Array,
    Object
  };

  class Json;

  using jArray = std::vector<Json>;
  using jObject = std::unordered_map<std::string, Json>;
  using jValue = std::variant<std::nullptr_t, bool, int64_t, double, std::string, jArray, jObject>;

  template <typename T>
  concept JsonTypeConcept =
    std::same_as<T, std::nullptr_t> ||
    std::same_as<T, bool> ||
    std::convertible_to<T, int64_t> ||
    std::convertible_to<T, double> ||
    std::convertible_to<T, std::string> ||
    std::same_as<T, jArray> ||
    std::same_as<T, jObject>;

  struct ParseState {
    const char *p;
    const char *end;

    int peek() const {
      return p < end ? static_cast<unsigned char>(*p) : -1;
    }

    int get() {
      return p < end ? static_cast<unsigned char>(*p++) : -1;
    }

    void skip_space() {
      while (p < end && std::isspace(static_cast<unsigned char>(*p))) {
        ++p;
      }
    }
  };

  class Json {

    public:
      using null_type = std::nullptr_t;
      using bool_type = bool;
      using integer_type = int64_t;
      using decimal_type = double;
      using text_type = std::string;
      using array_type = jArray;
      using object_type = jObject;

      static std::optional<Json> parse(std::string_view data) {
        ParseState ps{data.data(), data.data() + data.size()};
        return _parse(ps);
      }

      static std::optional<Json> parse(std::istream &is) {
        std::string data(std::istreambuf_iterator<char>(is), {});
        return parse(std::string_view(data));
      }

      Json()
        : Json{nullptr} {
      }

      template <JsonTypeConcept T>
      Json(T const &value)
        : mValue{value} {
      }

      template <JsonTypeConcept T>
      Json(T &&value)
        : mValue{std::move(value)} {
      }

      template <JsonTypeConcept ...Args>
      Json(Args &&...args)
        : mValue{jArray{std::move(args)...}} {
      }

      Json(Json const &value)
        : mValue{value.mValue} {
      }

      Json(Json &&value)
        : mValue{std::move(value.mValue)} {
      }

      Json(std::initializer_list<std::pair<std::string, Json>> const &value)
        : Json{jObject{value.begin(), value.end()}} {
      }

      template <typename T>
        requires requires (Json &out, T const &value) {
          { json_from(out, value) };
        }
      Json(T const &t)
        : Json{}
      {
        json_from(*this, t);
      }

      template <typename T, template <typename ...Args> class Container>
        requires requires (Json &out, T const &value) {
          { json_from(out, value) };
        }
      Json(Container<T> const &values)
      {
        jArray array;
        array.reserve(values.size());

        for (auto const &v : values) {
          Json out;
          json_from(out, v);
          array.push_back(std::move(out));
        }

        mValue = std::move(array);
      }

      JsonType get_type() const {
        return static_cast<JsonType>(mValue.index());
      }

      bool is_null() const {
        return get_type() == JsonType::Null;
      }

      bool is_bool() const {
        return get_type() == JsonType::Bool;
      }

      bool is_integer() const {
        return get_type() == JsonType::Integer;
      }

      bool is_decimal() const {
        return get_type() == JsonType::Decimal;
      }

      bool is_text() const {
        return get_type() == JsonType::Text;
      }

      bool is_array() const {
        return get_type() == JsonType::Array;
      }

      bool is_object() const {
        return get_type() == JsonType::Object;
      }

      template <JsonTypeConcept T>
      Json & operator = (T const &value) {
        this->mValue = value;
        return *this;
      }

      template <JsonTypeConcept T>
      Json & operator = (T &&value) {
        this->mValue = std::move(value);
        return *this;
      }

      Json & operator = (Json const &value) {
        mValue = value.mValue;
        return *this;
      }

      Json & operator = (Json &&value) {
        mValue = std::move(value.mValue);
        return *this;
      }

      bool operator == (JsonType value) const {
        return get_type() == value;
      }

      bool operator == (std::nullptr_t) const {
        return get_type() == JsonType::Null;
      }

      bool operator == (bool value) const {
        return get_type() == JsonType::Bool && std::get<bool>(mValue) == value;
      }

      bool operator == (int64_t value) const {
        return get_type() == JsonType::Integer && std::get<int64_t>(mValue) == value;
      }

      bool operator == (double value) const {
        return get_type() == JsonType::Decimal && std::get<double>(mValue) == value;
      }

      bool operator == (const char *value) const {
        return get_type() == JsonType::Text && std::get<std::string>(mValue) == value;
      }

      bool operator == (std::string const &value) const {
        return get_type() == JsonType::Text && std::get<std::string>(mValue) == value;
      }

      bool operator == (jArray const &value) const {
        auto const *myValue = std::get_if<jArray>(&mValue);
        return myValue && std::equal(value.begin(), value.end(), myValue->begin(),
            [](auto const &lhs, auto const &rhs) {
              return lhs.mValue == rhs.mValue;
            });
      }

      Json & operator [] (std::size_t index) {
        if (auto *value = std::get_if<jArray>(&mValue)) {
          return (*value)[index];
        }
        throw std::runtime_error("invalid access");
      }

      Json const & operator [] (std::size_t index) const {
        if (auto *value = std::get_if<jArray>(&mValue)) {
          return (*value)[index];
        }
        throw std::runtime_error("invalid access");
      }

      Json & operator [] (std::string const &key) {
        if (auto *value = std::get_if<jObject>(&mValue)) {
          if (auto i = value->find(key); i != value->end()) {
            return i->second;
          }
        }
        throw std::runtime_error("invalid access");
      }

      Json const & operator [] (std::string const &key) const {
        if (auto *value = std::get_if<jObject>(&mValue)) {
          if (auto i = value->find(key); i != value->end()) {
            return i->second;
          }
        }
        throw std::runtime_error("invalid access");
      }

      bool has(std::string const &key) const {
        auto const *object = std::get_if<jObject>(&mValue);
        return object && object->contains(key);
      }

      template <typename T>
      std::optional<T> get() const {
        if constexpr (std::same_as<T, int>) {
          if (auto *v = std::get_if<int64_t>(&mValue)) {
            return static_cast<int>(*v);
          }
          return {};
        } else if constexpr (std::same_as<T, float>) {
          if (auto *v = std::get_if<double>(&mValue)) {
            return static_cast<float>(*v);
          }
          return {};
        } else if constexpr (std::same_as<T, std::nullptr_t>) {
          if (std::holds_alternative<std::nullptr_t>(mValue)) {
            return nullptr;
          }
          return {};
        } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double> ||
                             std::is_same_v<T, std::string> || std::is_same_v<T, jArray> ||
                             std::is_same_v<T, jObject> || std::is_same_v<T, bool>) {
          if (auto *v = std::get_if<T>(&mValue)) {
            return *v;
          }
          return {};
        } else {
          try {
            T value{};
            json_to(*this, value);
            return value;
          } catch (...) {
            return {};
          }
        }
      }

      template <typename T>
      T & get_or_throw() {
        return std::get<T>(mValue);
      }

      template <typename T>
      T const & get_or_throw() const {
        return std::get<T>(mValue);
      }

      std::string dump() const {
        std::ostringstream out;
        out << std::boolalpha;
        _dump(*this, out);
        return out.str();
      }

      jValue const & get_value() const {
        return mValue;
      }

    private:
      jValue mValue;

      static std::optional<Json> _parse(ParseState &ps) {
        ps.skip_space();
        int c = ps.peek();

        if (c == -1) {
          return Json{};
        } else if (c == 'n') {
          return _read_null(ps);
        } else if (c == 'f' || c == 't') {
          return _read_bool(ps);
        } else if (c == '+' || c == '-' || c == '.' || (c >= '0' && c <= '9')) {
          return _read_number(ps);
        } else if (c == '"') {
          auto str = _read_string(ps);
          if (str) return Json{std::move(str.value())};
          return {};
        } else if (c == '[') {
          return _read_array(ps);
        } else if (c == '{') {
          return _read_object(ps);
        }

        return {};
      }

      static std::optional<Json> _read_null(ParseState &ps) {
        if (ps.get() == 'n' && ps.get() == 'u' && ps.get() == 'l' && ps.get() == 'l') {
          return Json{};
        }
        return {};
      }

      static std::optional<Json> _read_bool(ParseState &ps) {
        int c = ps.get();
        if (c == 'f') {
          if (ps.get() == 'a' && ps.get() == 'l' && ps.get() == 's' && ps.get() == 'e') {
            return Json{false};
          }
        } else if (c == 't') {
          if (ps.get() == 'r' && ps.get() == 'u' && ps.get() == 'e') {
            return Json{true};
          }
        }
        return {};
      }

      static std::optional<Json> _read_number(ParseState &ps) {
        std::string token;
        char type = 'u';
        bool first = false;

        while (ps.p < ps.end) {
          int c = std::tolower(ps.peek());

          if (type == 'u') {
            ps.get(); // consume the first character

            if (c == '0') {
              c = std::tolower(ps.peek());

              if (c == 'x') {
                type = 'h';
                ps.get(); // consume 'x'
              } else if (c == 'b') {
                type = 'b';
                ps.get(); // consume 'b'
              } else if (c == '.') {
                type = 'f';
                token += '.';
                ps.get(); // consume '.'
              } else {
                type = 'o';
                token += '0';
              }
            } else if (c == '.') {
              type = 'f';
              token += '.';
            } else if (c == '-') {
              token += '-';
            } else if (c >= '0' && c <= '9') {
              type = 'i';
              token += static_cast<char>(c);
            } else {
              break;
            }
          } else {
            if (type == 'i') {
              if (c >= '0' && c <= '9') {
                token += static_cast<char>(c);
              } else if (c == '.') {
                type = 'f';
                token += '.';
              } else {
                break;
              }
            } else if (type == 'b') {
              if (c >= '0' && c <= '1') {
                token += static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'o') {
              if (c >= '0' && c <= '7') {
                token += static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'h') {
              if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
                token += static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'f') {
              if (c >= '0' && c <= '9') {
                token += static_cast<char>(c);
              } else if (c == 'e') {
                type = 'c';
                first = true;
                token += 'e';
              } else {
                break;
              }
            } else if (type == 'c') {
              if (first && (c == '-' || c == '+')) {
                token += static_cast<char>(c);
              } else if (c >= '0' && c <= '9') {
                token += static_cast<char>(c);
              } else {
                break;
              }
              first = false;
            }

            ps.get();
          }
        }

        int terminator = ps.peek();

        if (type == 'u' || (terminator != -1 && terminator != '}' && terminator != ']' &&
            terminator != ',' && !std::isspace(terminator))) {
          return {};
        }

        Json result;

        if (type == 'i') {
          int64_t v = 0;
          std::from_chars(token.data(), token.data() + token.size(), v);
          result = v;
        } else if (type == 'b') {
          int64_t v = 0;
          std::from_chars(token.data(), token.data() + token.size(), v, 2);
          result = v;
        } else if (type == 'o') {
          int64_t v = 0;
          std::from_chars(token.data(), token.data() + token.size(), v, 8);
          result = v;
        } else if (type == 'h') {
          int64_t v = 0;
          std::from_chars(token.data(), token.data() + token.size(), v, 16);
          result = v;
        } else if (type == 'f') {
          if (token.front() == '.') {
            token = '0' + token;
          }
          if (token.back() == '.') {
            token += '0';
          }
          double v = 0;
          std::from_chars(token.data(), token.data() + token.size(), v);
          result = v;
        } else if (type == 'c') {
          if (token.back() == 'e') {
            token += '+';
          }
          if (token.back() == '-' || token.back() == '+') {
            token += '0';
          }

          auto epos = token.find('e');
          std::string baseStr = token.substr(0, epos);
          std::string multStr = token.substr(epos + 1);

          double base = 0;
          int64_t mult = 0;
          std::from_chars(baseStr.data(), baseStr.data() + baseStr.size(), base);
          std::from_chars(multStr.data(), multStr.data() + multStr.size(), mult);

          result = base * std::pow(10, mult);
        }

        return result;
      }

      static std::optional<std::string> _read_string(ParseState &ps) {
        ps.get(); // skip leading '"'
        std::string result;
        bool escape = false;

        while (ps.p < ps.end) {
          int c = ps.get();

          if (c == '"' && !escape) {
            return result;
          }

          if (c == '\\' && !escape) {
            escape = true;
          } else {
            result += static_cast<char>(c);
            escape = false;
          }
        }

        return {};
      }

      static std::optional<Json> _read_array(ParseState &ps) {
        ps.get(); // skip '['
        jArray result;

        result.reserve(32);

        while (ps.p < ps.end) {
          ps.skip_space();
          int c = ps.peek();

          if (c == ']') {
            ps.get();
            return Json{std::move(result)};
          } else if (c == ',') {
            ps.get();
          } else {
            auto valueOpt = _parse(ps);
            if (valueOpt) {
              result.push_back(std::move(valueOpt.value()));
            } else {
              return {};
            }
          }
        }

        return {};
      }

      static std::optional<Json> _read_object(ParseState &ps) {
        ps.get(); // skip '{'
        jObject result;

        while (ps.p < ps.end) {
          ps.skip_space();
          int c = ps.peek();

          if (c == '}') {
            ps.get();
            return Json{std::move(result)};
          } else if (c == ',') {
            ps.get();
          } else {
            auto keyStr = _read_string(ps);
            if (!keyStr || keyStr->empty()) {
              return {};
            }

            ps.skip_space();
            if (ps.p >= ps.end || ps.get() != ':') {
              return {};
            }

            auto valueOpt = _parse(ps);
            if (!valueOpt) {
              return {};
            }

            result.emplace(std::move(keyStr.value()), std::move(valueOpt.value()));
          }
        }

        return {};
      }

      void _dump(Json const &value, std::ostringstream &out) const {
        switch (value.get_type()) {
          case JsonType::Null:
            out << "null";
            break;
          case JsonType::Bool:
            out << value.get_or_throw<bool>();
            break;
          case JsonType::Integer:
            out << value.get_or_throw<int64_t>();
            break;
          case JsonType::Decimal: {
            double d = value.get_or_throw<double>();
            out << d;
            if (d == static_cast<int64_t>(d)) {
              out << ".0";
            }
            break;
          }
          case JsonType::Text:
            out << std::quoted(value.get_or_throw<std::string>());
            break;
          case JsonType::Array: {
            auto const &array = value.get_or_throw<jArray>();
            out << "[";
            bool first = true;
            for (auto const &i : array) {
              if (!first) out << ",";
              first = false;
              _dump(i, out);
            }
            out << "]";
            break;
          }
          case JsonType::Object: {
            auto const &object = value.get_or_throw<jObject>();
            out << "{";
            bool first = true;
            for (auto const &[k, v] : object) {
              if (!first) out << ",";
              first = false;
              out << std::quoted(k) << ":";
              _dump(v, out);
            }
            out << "}";
            break;
          }
        }
      }

      friend bool operator == (Json const &lhs, Json const &rhs) {
        if (lhs.mValue.index() != rhs.mValue.index()) {
          return false;
        }

        switch (lhs.mValue.index()) {
          case 0: return true;
          case 1: return std::get<bool>(lhs.mValue) == std::get<bool>(rhs.mValue);
          case 2: return std::get<int64_t>(lhs.mValue) == std::get<int64_t>(rhs.mValue);
          case 3: return std::get<double>(lhs.mValue) == std::get<double>(rhs.mValue);
          case 4: return std::get<std::string>(lhs.mValue) == std::get<std::string>(rhs.mValue);
          case 5: {
            auto &a = std::get<jArray>(lhs.mValue);
            auto &b = std::get<jArray>(rhs.mValue);
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
          }
          case 6: {
            auto &a = std::get<jObject>(lhs.mValue);
            auto &b = std::get<jObject>(rhs.mValue);
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
          }
          default: return false;
        }
      }

      friend std::ostream & operator << (std::ostream &out, Json const &value) {
        out << value.dump();
        return out;
      }

  };

  template <typename T>
  void json_to(jjson::Json const &json, T &out) {
    auto const &values = std::get<jArray>(json.get_value());

    for (auto const &value: values) {
      auto item = value.get<typename T::value_type>();
      if (item) {
        out.emplace_back(std::move(item.value()));
      }
    }
  }

  template <JsonTypeConcept T>
  void json_to(jjson::Json const &json, T &out) {
    out = std::get<T>(json.get_value());
  }

  template <>
  void json_to(jjson::Json const &json, int &out) {
    out = static_cast<int>(std::get<int64_t>(json.get_value()));
  }

  template <>
  void json_to(jjson::Json const &json, float &out) {
    out = static_cast<float>(std::get<double>(json.get_value()));
  }

}
