#ifndef VIPER_TYPES_HXX
#define VIPER_TYPES_HXX
#include <types/type.hxx>
#include <types/integerType.hxx>
#include <types/voidType.hxx>
#include <types/arrayType.hxx>
#include <types/pointerType.hxx>
#include <types/structType.hxx>
#include <map>

namespace Viper
{
    extern std::map<std::string, std::shared_ptr<Type>> types;
    void InitDefaultTypes();
}

#endif