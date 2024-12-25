#pragma once

#include "remote_base.h"

namespace esphome {
namespace remote_base {

struct Qx305Data {
  uint32_t address;
  uint32_t command;

  bool operator==(const Qx305Data &rhs) const { return address == rhs.address && command == rhs.command; }
};

class Qx305Protocol : public RemoteProtocol<Qx305Data> {
 public:
  void encode(RemoteTransmitData *dst, const Qx305Data &data) override;
  optional<Qx305Data> decode(RemoteReceiveData src) override;
  void dump(const Qx305Data &data) override;
};

DECLARE_REMOTE_PROTOCOL(Qx305)

template<typename... Ts> class Qx305Action : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint32_t, address)
  TEMPLATABLE_VALUE(uint32_t, command)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    Qx305Data data{};
    data.address = this->address_.value(x...);
    data.command = this->command_.value(x...);
    Qx305Protocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome
