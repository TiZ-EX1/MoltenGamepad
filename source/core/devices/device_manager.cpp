#include "device.h"
#include "../parser.h"

device_manager::device_manager(moltengamepad* mg, manager_plugin plugin, void* plug_data) : mg(mg), plugin(plugin), name(plugin.name), log(name), plug_data(plug_data), opts([&] (std::string& name, MGField value) { return process_manager_option(name, value); }) {
  mapprofile->name = name;
  log.add_listener(mg->stdout);
  if (plugin.init)
    plugin.init(plug_data, this);
}

int device_manager::register_event(event_decl ev) {
  events.push_back(ev);
  std::vector<token> tokens = tokenize(std::string(ev.default_mapping));
  auto it = tokens.begin();
  event_translator* trans = MGparser::parse_trans(ev.type, tokens, it, nullptr);
  mapprofile->set_mapping(std::string(ev.name), trans ? trans : new event_translator(), ev.type, true);
  return 0;
}

int device_manager::register_device_option(option_decl opt) {
  mapprofile->register_option(opt);
  return 0;
}

int device_manager::register_manager_option(option_decl opt) {
  opts.register_option(opt);
  has_options = true;
  return 0;
}

int device_manager::process_manager_option(const std::string& name, MGField value) {
  if (plugin.process_manager_option)
    return plugin.process_manager_option(plug_data, name.c_str(), value);
  return FAILURE;
}

int device_manager::register_alias(const char* external, const char* local) {
  mapprofile->set_alias(std::string(external), std::string(local));
  return 0;
}

device_manager::~device_manager() {
  if (plugin.destroy)
    plugin.destroy(plug_data);
}

int device_manager::accept_device(struct udev* udev, struct udev_device* dev) {
  debug_print(DEBUG_INFO, 2, "\tchecking manager ",name.c_str());
  if (plugin.process_udev_event)
    return plugin.process_udev_event(plug_data, udev, dev);
  return DEVICE_UNCLAIMED;
}

int device_manager::accept_deferred_device(struct udev* udev, struct udev_device* dev) {
  if (plugin.process_deferred_udev_event)
    return plugin.process_deferred_udev_event(plug_data, udev, dev);
  return DEVICE_UNCLAIMED;
}

input_source* device_manager::add_device(device_plugin dev, void* dev_plug_data) {
  auto ptr = mg->add_device(this, dev, dev_plug_data);
  return ptr.get();
}

void device_manager::for_all_devices(std::function<void (const input_source*)> func) {
  mg->for_all_devices( [&, func] (std::shared_ptr<input_source> device) {
    if (device->get_manager_name() == name)
      func(device.get());
  });
}
