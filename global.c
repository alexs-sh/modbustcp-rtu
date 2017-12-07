#include "global.h"

common_info_t* common_info = 0;

common_info_t* get_common_info() {
  // assert(common_info);
  return common_info;
}

void set_common_info(common_info_t* info) {
  if (!common_info) common_info = info;
}
