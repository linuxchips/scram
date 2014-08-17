/// @file fault_tree.h
/// Fault Tree.
#ifndef SCRAM_FAULT_TREE_H_
#define SCRAM_FAULT_TREE_H_

#include <map>
#include <set>
#include <string>
#include <typeinfo>

#include <boost/pointer_cast.hpp>
#include <boost/serialization/map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "error.h"
#include "event.h"

// class FaultTreeTest;

typedef boost::shared_ptr<scram::Event> EventPtr;
typedef boost::shared_ptr<scram::Gate> GatePtr;
typedef boost::shared_ptr<scram::PrimaryEvent> PrimaryEventPtr;

namespace scram {

/// @class FaultTree
/// Fault tree representation.
class FaultTree {
  // friend class ::FaultTreeTest;

 public:
  /// The main constructor of the Fault Tree.
  /// @param[in] name The name identificator of this fault tree.
  FaultTree(std::string name);

  virtual ~FaultTree() {}

  /// New tree methhods.
  void AddGate(GatePtr& gate);

  /// @returns The name of this tree.
  const std::string& name() { return name_; }

  /// @returns The warnings generated by all the tree operations.
  const std::string& warnings() { return warnings_; }

  /// @returns The top gate.
  const GatePtr& top_event() { return top_event_; }

  /// @returns The container of intermediate events.
  const boost::unordered_map<std::string, GatePtr>& inter_events() {
    return inter_events_;
  }

  /// @returns The container of primary events of this tree.
  /// @note Assuming that the leafs are primary events, which means that the
  /// tree is fully developed without indefined gates.
  const std::map<std::string, PrimaryEventPtr>& primary_events() {
    if (primary_events_.empty()) GenerateLeafs_();
    return primary_events_;
  }

 private:
  /// Populates all non-gate events to the appropriate container.
  /// @note This does not check if the non-gate events are basic or house.
  void GenerateLeafs_();

  void ChildrenToLeafs_(GatePtr& gate);

  /// The name of this fault tree.
  std::string name_;
  /// This member is used to provide any warnings about assumptions,
  /// calculations, and settings. These warnings must be written into output
  /// file.
  std::string warnings_;

  /// Id of a top event.
  std::string top_event_id_;

  /// Top event.
  GatePtr top_event_;

  /// Holder for intermediate events.
  boost::unordered_map<std::string, GatePtr> inter_events_;

  /// Container for the primary events of the tree.
  std::map<std::string, PrimaryEventPtr> primary_events_;

  /// Locks any further changes to this tree.
  bool lock_;
};

}  // namespace scram

#endif  // SCRAM_FAULT_TREE_H_
