#include "extra_info.h"
#include "LibFA.h"

namespace libfa {
namespace internal {

static int g_age = 0;
static int g_gender = 0;
static int g_source_type = SOURCE_RGB;

void setExtraInfo(int age, int gender, int source_type)
{
    g_age = age;
    g_gender = gender;
    g_source_type = source_type;
}

int extraAge() { return g_age; }
int extraGender() { return g_gender; }
int extraSourceType() { return g_source_type; }

} // namespace internal
} // namespace libfa
