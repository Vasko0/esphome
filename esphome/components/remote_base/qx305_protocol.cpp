#include "qx305_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.qx305";

static const uint32_t HEADER_HIGH_US = 34;
static const uint32_t HEADER_LOW_US = 1020;
static const uint32_t BIT_ONE_HIGH_US = 102;
static const uint32_t BIT_ONE_LOW_US = 31;
static const uint32_t BIT_ZERO_HIGH_US = 34;
static const uint32_t BIT_ZERO_LOW_US = 100;

void Qx305Protocol::encode(RemoteTransmitData *dst, const Qx305Data &data) {
  dst->set_carrier_frequency(38000);

  // HEADER
  dst->item(HEADER_HIGH_US, HEADER_LOW_US);

  //  Typically a DISH device needs to get a command a total of
  //  at least 4 times to accept it.
  for (uint i = 0; i < 23; i++) {
    // COMMAND (function, in MSB)
    for (uint32_t mask = 1UL << 24; mask; mask >>= 1) {
      if (data.command & mask) {
        dst->item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US);
      } else {
        dst->item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US);
      }
    }

    // ADDRESS (unit code, in LSB)
    for (uint32_t mask = 1UL; mask < 1UL << 23; mask <<= 1) {
      if ((data.address - 1) & mask) {
        dst->item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US);
      } else {
        dst->item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US);
      }
    }
    // PADDING
    for (uint j = 0; j < 25; j++)
      dst->item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US);

    // FOOTER
    dst->item(HEADER_HIGH_US, HEADER_LOW_US);
  }
}
optional<Qx305Data> Qx305Protocol::decode(RemoteReceiveData src) {
  Qx305Data data{
      .address = 0,
      .command = 0,
  };
  if (!src.expect_item(HEADER_HIGH_US, HEADER_LOW_US))
    return {};

  for (uint32_t mask = 1UL << 20; mask != 0; mask >>= 1) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US)) {
      data.command |= mask;
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
      data.command &= ~mask;
    } else {
      return {};
    }
  }

  for (uint32_t mask = 1UL; mask < 1UL << 4; mask <<= 1) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US)) {
      data.address |= mask;
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
      data.address &= ~mask;
    } else {
      return {};
    }
  }
  // for (uint j = 0; j < 6; j++) {
  //   if (!src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
  //     return {};
  //   }
  // }
  data.address++;

  src.expect_item(HEADER_HIGH_US, HEADER_LOW_US);

  return data;
}

void Qx305Protocol::dump(const Qx305Data &data) {
  ESP_LOGI(TAG, "Received Dish: address=0x%02X, command=0x%02X", data.address, data.command);
}

}  // namespace remote_base
}  // namespace esphome
