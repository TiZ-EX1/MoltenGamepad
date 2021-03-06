#pragma once
#include "../event_change.h"

class simple_chord : public advanced_event_translator {
public:
  std::vector<std::string> event_names;
  std::vector<int> event_ids;
  std::vector<int> event_vals;
  int output_cache = 0;
  input_source* owner = nullptr;
  event_translator* out_trans = nullptr;

  simple_chord(std::vector<std::string> event_names, event_translator* trans) : event_names(event_names), out_trans(trans) {};

  virtual ~simple_chord();

  virtual void init(input_source* source);
  virtual void attach(input_source* source);
  virtual bool set_mapped_events(const std::vector<std::string>& event_names);

  virtual bool claim_event(int id, mg_ev event);
  virtual advanced_event_translator* clone() {
    return new simple_chord(event_names, out_trans->clone());
  }

  static const char* decl;
  simple_chord(std::vector<MGField>& fields);
  virtual void fill_def(MGTransDef& def);
protected:
  simple_chord() {};
};
