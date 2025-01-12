#include <type/types.hh>

std::map<std::string, std::shared_ptr<Type>> types = {
    {"int8",  std::make_shared<IntegerType>(8)},
    {"int16", std::make_shared<IntegerType>(16)},
    {"int32", std::make_shared<IntegerType>(32)},
    {"int64", std::make_shared<IntegerType>(64)},
};