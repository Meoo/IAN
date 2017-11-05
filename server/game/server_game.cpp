
#include <functional>

#include <modules/mod_headers.gen.h>


class ModuleEntry
{
public:
  ModuleEntry(const char * name, std::function<void*()> builder)
    : name(name), builder(builder) {}

  const char * name;
  std::function<void*()> builder;
};

const ModuleEntry MODULES[] =
{
# define IAN_MODULE_MACRO(mod) {#mod, []() { return new modules::mod; }},
# include <modules/mod_list.gen>
# undef IAN_MODULE_MACRO
  {nullptr, []() { return nullptr; }}
};


int main(int argc, char ** argv)
{
  return 0;
}
