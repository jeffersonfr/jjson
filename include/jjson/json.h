#pragma once

#include <variant>
#include <vector>
#include <string>
#include <span>
#include <map>
#include <cassert>
#include <optional>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cmath>

namespace jjson {
  class Json;
}

namespace jjson {

  enum class JsonType {
    Null,
    Bool,
    Int,
    Float,
    String,
    Array,
    Object
  };

  class Json;

  using jArray = std::vector<Json>;

  using jObject = std::map<std::string, Json>;
      
  using jValue = std::variant<std::nullptr_t, bool, int64_t, double, std::string, jArray, jObject>;

  class Json {

    public:
      static std::optional<Json> parse(std::string const &data) {
        std::istringstream is{data};

        return parse(is);
      }

      static std::optional<Json> parse(std::istream &is) {
        Json result;

        while (is) {
          int c = std::tolower(is.peek());

          try {
            if (std::isspace(c)) {
              _skip_space(is);

              continue;
            } else if (c == -1) {
              return Json{};
            } else if (c == 'n') { // 'n'ull
              result = _read_null(is);
            } else if (c == 'f' or c == 't') { // 'f'alse or 't'rue
              result = _read_bool(is);
            } else if (c == '+' or c == '-' or c == '.' or c == '0' or c == '1' or c == '2' or 
                c == '3' or c == '4' or c == '5' or c == '6' or c == '7' or c == '8' or c == '9') {
              result = _read_number(is); // 42, 0x2a, 018, 0b1010, 3.1415+4
            } else if (c == '"') {
              result = _read_string(is);
            } else if (c == '[') {
              result = _read_array(is);
            } else if (c == '{') {
              result = _read_object(is);
            } else {
              return {};
            }
          } catch (...) {
            return {};
          }

          break;
        }

        return result;
      }

      Json()
        : Json{nullptr} {
      }

      template <typename T>
        requires 
          std::same_as<T, std::nullptr_t> ||
          std::same_as<T, bool> ||
          std::convertible_to<T, int64_t> ||
          std::convertible_to<T, double> ||
          std::convertible_to<T, std::string> ||
          std::same_as<T, jArray> ||
          std::same_as<T, jObject>
      Json(T const &value)
        : mValue{value} {
      }

      template <typename T>
        requires 
          std::same_as<T, std::nullptr_t> ||
          std::same_as<T, bool> ||
          std::convertible_to<T, int64_t> ||
          std::convertible_to<T, double> ||
          std::convertible_to<T, std::string> ||
          std::same_as<T, jArray> ||
          std::same_as<T, jObject>
      Json(T &&value)
        : mValue{std::move(value)} {
      }

      template <typename T, std::size_t N>
        Json(std::array<T, N> const &value)
          : Json{jArray{value.begin(), value.end()}}
        {
        }

      Json(Json const &value)
        : mValue{value.mValue} {
      }

      Json(Json &&value)
        : mValue{std::move(value.mValue)} {
      }

      Json(std::initializer_list<std::pair<std::string, Json>> const &value)
        : Json(jObject{value.begin(), value.end()}) {
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

      template <typename T>
        requires requires (Json &out, T const &value) {
          { json_from(out, value) };
        }
      Json(std::vector<T> const &values)
      {
        jArray array;

        for (auto &value : values) {
          Json out{};

          json_from(out, value);

          array.push_back(out);
        }

        mValue = array;
      }

      JsonType get_type() const {
        if (std::holds_alternative<bool>(mValue) == true) {
          return JsonType::Bool;
        } else if (std::holds_alternative<int64_t>(mValue) == true) {
          return JsonType::Int;
        } else if (std::holds_alternative<double>(mValue) == true) {
          return JsonType::Float;
        } else if (std::holds_alternative<std::string>(mValue) == true) {
          return JsonType::String;
        } else if (std::holds_alternative<jArray>(mValue) == true) {
          return JsonType::Array;
        } else if (std::holds_alternative<jObject>(mValue) == true) {
          return JsonType::Object;
        }

        return JsonType::Null;
      }

      template <typename T>
        requires 
          std::same_as<T, std::nullptr_t> ||
          std::same_as<T, bool> ||
          std::convertible_to<T, int64_t> ||
          std::convertible_to<T, double> ||
          std::convertible_to<T, std::string> ||
          std::same_as<T, jArray> ||
          std::same_as<T, jObject>
      Json & operator = (T const &value) {
        *this = Json{value};

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

      bool operator == (std::nullptr_t value) const {
        if (get_type() == JsonType::Null) {
          return true;
        }

        return false;
      }

      bool operator == (bool value) const {
        if (get_type() == JsonType::Bool) {
          return std::get<bool>(mValue) == value;
        }

        return false;
      }

      bool operator == (int64_t value) const {
        if (get_type() == JsonType::Int) {
          return std::get<int64_t>(mValue) == value;
        }

        return false;
      }

      bool operator == (double value) const {
        if (get_type() == JsonType::Float) {
          return std::get<double>(mValue) == value;
        }

        return false;
      }

      bool operator == (const char *value) const {
        if (get_type() == JsonType::String) {
          return std::get<std::string>(mValue) == value;
        }

        return false;
      }

      bool operator == (std::string const &value) const {
        if (get_type() == JsonType::String) {
          return std::get<std::string>(mValue) == value;
        }

        return false;
      }

      bool operator == (jArray value) const {
        if (get_type() == JsonType::Array) {
          std::optional<jArray> myValue = std::get<jArray>(mValue);

          return std::equal(value.begin(), value.end(), (*myValue).begin(),
              [](auto const &lhs, auto const &rhs) -> bool {
                return lhs.mValue == rhs.mValue;
              });
        }

        return false;
      }

      Json operator [] (std::size_t index) const {
        if (get_type() == JsonType::Array) {
          std::optional<jArray> value = get<jArray>();

          if (value and (*value).empty() == false and index < (*value).size()) {
            return (*value)[index];
          }
        }

        throw std::runtime_error("invalid access");
      }

      Json operator [] (std::string const &key) const {
        if (get_type() == JsonType::Object) {
          std::optional<jObject> value = get<jObject>();

          if (value and (*value).empty() == false) {
            auto i = (*value).find(key);

            if (i != (*value).end()) {
              return i->second;
            }
          }
        }

        throw std::runtime_error("invalid access");
      }

      bool has(std::string const &key) const {
        if (get_type() == JsonType::Object) {
          jObject object = std::get<jObject>(mValue);

          if (object.find(key) != object.end()) {
            return true;
          }
        }

        return false;
      }
      /**
       * \brief Returns an object of type T if it exists, or empty otherwise.
       *
       * \return type T, if exists.
       */
      template <typename T>
        std::optional<T> get() const {
          try {
            T value{};

            json_to<T>(*this, value);

            return value;
          } catch (...) {
          }

          return {};
        }

      /**
       * \brief Returns an object of type T if it exists, or throw otherwise.
       *
       * \return type T, if exists.
       */
      template <typename T>
        T get_or_throw() const {
          auto value = get<T>();

          if (value) {
            return *value;
          }

          throw std::runtime_error("unavailable type");
        }

      /**
       * \brief Dumps the content to string.
       *
       */
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

      static void _skip_space(std::istream &is) {
        while (std::isspace(is.peek())) {
          is.get();
        }
      }

      static Json _read_null(std::istream &is) {
        int a = is.get();
        int b = is.get();
        int c = is.get();
        int d = is.get();
        if (a == 'n' and b == 'u' and c == 'l' and d == 'l') {
          return {};
        }

        throw std::runtime_error("unable to parse null value");
      }

      static Json _read_bool(std::istream &is) {
        if (is.peek() == 'f') {
          if (is.get() == 'f' and is.get() == 'a' and is.get() == 'l' and is.get() == 's' and is.get() == 'e') {
            return false;
          }
        } else if (is.peek() == 't') {
          if (is.get() == 't' and is.get() == 'r' and is.get() == 'u' and is.get() == 'e') {
            return true;
          }
        }

        throw std::runtime_error("unable to parse bool value");
      }

      static Json _read_number(std::istream &is) {
        std::ostringstream out;
        std::string token;
        char type = 'u'; // u:unknown, i:int, b:binary, o:octal, h:hex, f:float, c: cientific notation
        bool first = false;
        int c = -1;

        while (is) {
          c = is.peek();

          if (type == 'u') {
            is.get();

            if (c == '0') {
              c = std::tolower(is.peek());

              if (c == 'x') {
                type = 'h';

                is.get();
              } else if (c == 'b') {
                type = 'b';

                is.get();
              } else if (c == '.') {
                type = 'f';
                token = token + '.';

                is.get();
              } else {
                type = 'o';
                token = token + '0';
              }
            } else if (c == '.') {
              type = 'f';
              token = token + '.';
            } else if (c == '-') {
              token = token + '-';
            } else if (c >= '0' and c <= '9') {
              type = 'i';
              token = token + static_cast<char>(c);
            } else {
              break;
            }
          } else {
            if (type == 'i') {
              if (c >= '0' and c <= '9') {
                token = token + static_cast<char>(c);
              } else if (c == '.') {
                type = 'f';
                token = token + static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'b') {
              if (c >= '0' and c <= '1') {
                token = token + static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'o') {
              if (c >= '0' and c <= '7') {
                token = token + static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'h') {
              if ((c >= '0' and c <= '9') or (c >= 'a' and c <= 'f')) {
                token = token + static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'f') {
              if (c >= '0' and c <= '9') {
                token = token + static_cast<char>(c);
              } else if (c == 'e') {
                type = 'c';
                first = true;
                token = token + static_cast<char>(c);
              } else {
                break;
              }
            } else if (type == 'c') {
              if (first == true and (c == '-' or c == '+')) {
                token = token + static_cast<char>(c);
              } else if (c >= '0' and c <= '9') {
                token = token + static_cast<char>(c);
              }
                  
              first = false;
            }

            is.get();
          }
        }
          
        if (c < 0 or c == '}' or c == ']' or c == ',' or std::isspace(c) != 0) {
          Json result;

          if (type == 'i') {
            result = static_cast<int64_t>(std::stoll(token, nullptr, 10));
          } else if (type == 'b') {
            result = static_cast<int64_t>(std::stoll(token, nullptr, 2));
          } else if (type == 'o') {
            result = static_cast<int64_t>(std::stoll(token, nullptr, 8));
          } else if (type == 'h') {
            result = static_cast<int64_t>(std::stoll(token, nullptr, 16));
          } else if (type == 'f') {
            if (token.front() == '.') {
              token = '0' + token;
            }

            if (token.back() == '.') {
              token = token + '0';
            }

            result = std::stod(token, nullptr);
          } else if (type == 'c') {
            if (token.back() == 'e') {
              token = token + '+';
            }

            if (token.back() == '-' or token.back() == '+') {
              token = token + '0';
            }

            std::string::size_type index = token.find("e");
            std::string baseStr = token.substr(0, index);
            std::string multStr = token.substr(index + 1);

            double base = std::stod(baseStr, nullptr);
            double mult = static_cast<int64_t>(std::stoll(multStr, nullptr, 10));

            result = static_cast<double>(base * std::pow(10, mult));
          }

          return result;
        }

        throw std::runtime_error("unable to parse number value");
      }

      static Json _read_string(std::istream &is) {
        std::ostringstream out;
        bool skip = false;

        is.get(); // remove first '"'

        while (is) {
          int c = is.get();

          if (c == '"' and skip == false) {
            return out.str();
          }

          if (c == '\\' and skip == false) {
            skip = true;
          } else {
            out << static_cast<char>(c);
          }
          
          skip = false;
        }

        throw std::runtime_error("unable to parse string value");
      }

      static Json _read_array(std::istream &is) {
        jArray result{};
 
        is.get(); // remove first '['

        while (is) {
          int c = is.peek();

          if (std::isspace(c)) {
            _skip_space(is);
          } else if (c == ']') {
            is.get();

            return result;
          } else if (c == ',') {
            is.get();
          } else {
            std::optional<Json> valueOpt = Json::parse(is);

            if (valueOpt) {
              result.push_back(std::move(valueOpt.value()));
            } else {
              is.get(); // skip char if no byte was read
            }
          }
        }

        return result;
      }

      static Json _read_object(std::istream &is) {
        jObject result{};
 
        is.get(); // remove first '['

        while (is) {
          int c = is.peek();

          if (std::isspace(c)) {
            _skip_space(is);
          } else if (c == '}') {
            is.get();

            return result;
          } else if (c == ',') {
            is.get();
          } else {
            std::string key = _read_string(is).get<std::string>().value();

            if (key.empty() == true) {
              throw std::runtime_error("unable to find object key");
            }

            _skip_space(is);

            if (!is) {
              throw std::runtime_error("unable to find object separator");
            }

            c = is.get();

            if (c != ':') {
              throw std::runtime_error("invalid object separator");
            }
            
            std::optional<Json> valueOpt = Json::parse(is);

            if (valueOpt) {
              result[key] = valueOpt.value();
            } else {
              throw std::runtime_error("unable to find object value");
            }
          }
        }

        return result;
      }

      void _dump(Json const &value, std::ostringstream &out) const {
        if (value.get_type() == JsonType::Null) {
          out << "null";
        } else if (value.get_type() == JsonType::Bool) {
          out << value.get_or_throw<bool>();
        } else if (value.get_type() == JsonType::Int) {
          out << value.get_or_throw<int64_t>();
        } else if (value.get_type() == JsonType::Float) {
          double d = value.get_or_throw<double>();

          out << d;

          if ((d - static_cast<int64_t>(d)) == 0) {
            out << ".0";
          }
        } else if (value.get_type() == JsonType::String) {
          out << std::quoted(value.get_or_throw<std::string>());
        } else if (value.get_type() == JsonType::Array) {
          jArray array = value.get_or_throw<jArray>();

          out << "[";

          bool first{true};

          for (auto &i : array) {
            if (first == false) {
              out << ",";
            }

            first = false;

            _dump(i, out);
          }

          out << "]";
        } else if (value.get_type() == JsonType::Object) {
          jObject object = value.get_or_throw<jObject>();

          out << "{";

          bool first{true};

          for (auto &i : object) {
            if (first == false) {
              out << ",";
            }

            first = false;

            out << std::quoted(i.first) << ":";
            
            _dump(i.second, out);
          }

          out << "}";
        }
      }

      friend bool operator == (Json const &lhs, Json const &rhs) {
        if (lhs.mValue.index() == rhs.mValue.index()) {
          if (std::holds_alternative<std::nullptr_t>(lhs.mValue) == true) {
            return true;
          } else if (std::holds_alternative<bool>(lhs.mValue) == true) {
            return std::get<bool>(lhs.mValue) == std::get<bool>(rhs.mValue);
          } else if (std::holds_alternative<int64_t>(lhs.mValue) == true) {
            return std::get<int64_t>(lhs.mValue) == std::get<int64_t>(rhs.mValue);
          } else if (std::holds_alternative<double>(lhs.mValue) == true) {
            return std::get<double>(lhs.mValue) == std::get<double>(rhs.mValue);
          } else if (std::holds_alternative<jArray>(lhs.mValue) == true) {
            auto &lhsValue = std::get<jArray>(lhs.mValue);
            auto &rhsValue = std::get<jArray>(rhs.mValue);

            return std::equal(lhsValue.begin(), lhsValue.end(), rhsValue.begin());
          } else if (std::holds_alternative<jObject>(lhs.mValue) == true) {
            auto &lhsValue = std::get<jObject>(lhs.mValue);
            auto &rhsValue = std::get<jObject>(rhs.mValue);

            return std::equal(lhsValue.begin(), lhsValue.end(), rhsValue.begin());
          }
        }

        return false;
      }

      friend std::ostream & operator << (std::ostream &out, Json const &value) {
        out << value.dump();

        return out;
      }

  };

  // INFO:: T is a std::vector, this method needs the specialization that following
  template <typename T>
    void json_to(jjson::Json const &json, T &out) {
      auto values = json.get_or_throw<jjson::jArray>();

      for (auto &value: values) {
        out.push_back(value.get_or_throw<typename T::value_type>());
      }
    }

  template <typename T>
      requires 
        std::same_as<T, std::nullptr_t> ||
        std::same_as<T, bool> ||
        std::same_as<T, int64_t> ||
        std::same_as<T, double> ||
        std::same_as<T, std::string> ||
        std::same_as<T, jjson::jArray> ||
        std::same_as<T, jjson::jObject>
    void json_to(jjson::Json const &json, T &out) {
      out = std::get<T>(json.get_value());
    }

  template <>
    void json_to(jjson::Json const &value, int &out) {
      out = static_cast<int>(std::get<int64_t>(value.get_value()));
    }

  template <>
    void json_to(jjson::Json const &value, float &out) {
      out = static_cast<float>(std::get<double>(value.get_value()));
    }

}
