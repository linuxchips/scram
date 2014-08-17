/// @file fault_tree.cc
/// Implementation of fault tree analysis.
#include "fault_tree.h"

#include <iostream>
#include <iterator>
#include <typeinfo>

#include <boost/algorithm/string.hpp>

namespace scram {

FaultTree::FaultTree(std::string name)
    : name_(name),
      top_event_id_(""),
      lock_(false),
      warnings_("") {}

void FaultTree::AddGate(GatePtr& gate) {
  if (lock_) throw scram::Error("The tree is locked. No change is allowed.");

  if (top_event_id_ == "") {
    top_event_ = gate;
    top_event_id_ = gate->id();
  } else {
    if (inter_events_.count(gate->id()) || gate->id() == top_event_id_) {
      std::stringstream msg;
      msg << "Trying to doubly define a gate";
      throw scram::ValueError(msg.str());
    }
    inter_events_.insert(std::make_pair(gate->id(), gate));
  }
}

void FaultTree::GenerateLeafs_() {
  lock_ = true;  // Assumes that the tree is fully developed.

  FaultTree::ChildrenToLeafs_(top_event_);

  boost::unordered_map<std::string, GatePtr>::iterator git;
  for (git = inter_events_.begin(); git != inter_events_.end(); ++git) {

    FaultTree::ChildrenToLeafs_(git->second);
  }
}

void FaultTree::ChildrenToLeafs_(GatePtr& gate) {
  const std::map<std::string, EventPtr>* children = &gate->children();
  std::map<std::string, EventPtr>::const_iterator it;
  for (it = children->begin(); it != children->end(); ++it) {
    if (typeid(it->second) != typeid(top_event_)) {
      PrimaryEventPtr primary_event =
          boost::dynamic_pointer_cast<scram::PrimaryEvent>(it->second);
      assert(primary_event != 0);
      primary_events_.insert(std::make_pair(it->first, primary_event));
    }
  }
}

}  // namespace scram
