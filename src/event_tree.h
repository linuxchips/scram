/*
 * Copyright (C) 2017 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file event_tree.h
/// Event Tree facilities.

#ifndef SCRAM_SRC_EVENT_TREE_H_
#define SCRAM_SRC_EVENT_TREE_H_

#include <memory>
#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/variant.hpp>

#include "element.h"
#include "event.h"
#include "expression.h"
#include "ext/variant.h"

namespace scram {
namespace mef {

class InstructionVisitor;

/// Instructions and rules for event tree paths.
class Instruction : private boost::noncopyable {
 public:
  virtual ~Instruction() = default;

  /// Applies the visitor to the object.
  virtual void Accept(InstructionVisitor* visitor) const = 0;
};

/// Instructions are assumed not to be shared.
using InstructionPtr = std::unique_ptr<Instruction>;

/// A collection of instructions.
using InstructionContainer = std::vector<InstructionPtr>;

/// The operation of collecting expressions for event tree sequences.
class CollectExpression : public Instruction {
 public:
  /// @param[in] expression  The expression to multiply
  ///                        the current sequence probability.
  explicit CollectExpression(Expression* expression)
      : expression_(expression) {}

  /// @returns The collected expression for value extraction.
  Expression& expression() const { return *expression_; }

  void Accept(InstructionVisitor* visitor) const override;

 private:
  Expression* expression_;  ///< The probability expression to multiply.
};

/// The operation of connecting fault tree events into the event tree.
class CollectFormula : public Instruction {
 public:
  /// @param[in] formula  The valid formula to add into the sequence fault tree.
  explicit CollectFormula(FormulaPtr formula) : formula_(std::move(formula)) {}

  /// @returns The formula to include into the current product of the path.
  Formula& formula() const { return *formula_; }

  void Accept(InstructionVisitor* visitor) const override;

 private:
  FormulaPtr formula_;  ///< The valid single formula for the collection.
};

/// Conditional application of instructions.
class IfThenElse : public Instruction {
 public:
  /// @param[in] expression  The expression to evaluate for truth.
  /// @param[in] then_instruction  The required instruction to execute.
  /// @param[in] else_instruction  An optional instruction for the false case.
  IfThenElse(Expression* expression, InstructionPtr then_instruction,
             InstructionPtr else_instruction = nullptr)
      : expression_(expression),
        then_instruction_(std::move(then_instruction)),
        else_instruction_(std::move(else_instruction)) {}

  /// Forwards the visitor to the appropriate sub-instruction.
  void Accept(InstructionVisitor* visitor) const override;

 private:
  Expression* expression_;           ///< The condition source.
  InstructionPtr then_instruction_;  ///< The mandatory 'truth' instruction.
  InstructionPtr else_instruction_;  ///< The optional 'false' instruction.
};

/// Compound instructions.
class Block : public Instruction {
 public:
  /// @param[in] instructions  Instructions to be applied in this block.
  explicit Block(InstructionContainer instructions)
      : instructions_(std::move(instructions)) {}

  /// Applies the visitor to instructions in the block consecutively.
  void Accept(InstructionVisitor* visitor) const override;

 private:
  InstructionContainer instructions_;  ///< Zero or more instructions.
};

/// The base abstract class for instruction visitors.
class InstructionVisitor {
 public:
  virtual ~InstructionVisitor() = default;

  /// A set of required visitation functions for concrete visitors to implement.
  /// @{
  virtual void Visit(const CollectExpression*) = 0;
  virtual void Visit(const CollectFormula*) = 0;
  /// @}
};

/// Representation of sequences in event trees.
class Sequence : public Element, public Usage {
 public:
  using Element::Element;

  /// @param[in] instructions  Zero or more instructions for the sequence.
  void instructions(InstructionContainer instructions) {
    instructions_ = std::move(instructions);
  }

  /// @returns The instructions to be applied at this sequence.
  const InstructionContainer& instructions() const { return instructions_; }

 private:
  /// Instructions to execute with the sequence.
  InstructionContainer instructions_;
};

/// Sequences are defined in event trees but referenced in other constructs.
using SequencePtr = std::shared_ptr<Sequence>;

class EventTree;  // Manages the order assignment to functional events.

/// Representation of functional events in event trees.
class FunctionalEvent : public Element, public Usage {
  friend class EventTree;

 public:
  using Element::Element;

  /// @returns The order of the functional event in the event tree.
  /// @returns 0 if no order has been assigned.
  int order() const { return order_; }

 private:
  /// Sets the functional event order.
  void order(int order) { order_ = order; }

  int order_ = 0;  ///< The order of the functional event.
};

/// Functional events are defined in and unique to event trees.
using FunctionalEventPtr = std::unique_ptr<FunctionalEvent>;

class Fork;
class NamedBranch;

/// The branch representation in event trees.
class Branch {
 public:
  /// The types of possible branch end-points.
  using Target = boost::variant<Sequence*, Fork*, NamedBranch*>;

  /// Sets the instructions to execute at the branch.
  void instructions(InstructionContainer instructions) {
    instructions_ = std::move(instructions);
  }

  /// @returns The instructions to execute at the branch.
  const InstructionContainer& instructions() const { return instructions_; }

  /// Sets the target for the branch.
  void target(Target target) { target_ = std::move(target); }

  /// @returns The target semantics or end-points of the branch.
  ///
  /// @pre The target has been set.
  const Target& target() const {
    assert(ext::as<bool>(target_));
    return target_;
  }

 private:
  InstructionContainer instructions_;  ///< Zero or more instructions.
  Target target_;  ///< The target semantics of the branch.
};

/// Named branches that can be referenced and reused.
class NamedBranch : public Element,
                    public Branch,
                    public NodeMark,
                    public Usage {
 public:
  using Element::Element;
};

using NamedBranchPtr = std::unique_ptr<NamedBranch>;  ///< Unique in event tree.

/// Functional-event state paths in event trees.
class Path : public Branch {
 public:
  /// @param[in] state  State identifier string for functional event.
  ///
  /// @throws LogicError  The string is empty or malformed.
  explicit Path(std::string state);

  /// @returns The state of a functional event.
  const std::string& state() const { return state_; }

 private:
  std::string state_;  ///< The state identifier.
};

/// Functional event forks.
class Fork {
 public:
  /// @param[in] functional_event  The source functional event.
  /// @param[in] paths  The fork paths with functional event states.
  ///
  /// @throws ValidationError  The path states are duplicated.
  Fork(const FunctionalEvent& functional_event, std::vector<Path> paths);

  /// @returns The functional event of the fork.
  const FunctionalEvent& functional_event() const { return functional_event_; }

  /// @returns The fork paths with functional event states.
  /// @{
  const std::vector<Path>& paths() const { return paths_; }
  std::vector<Path>& paths() { return paths_; }
  /// @}

 private:
  const FunctionalEvent& functional_event_;  ///< The fork source.
  std::vector<Path> paths_;  ///< The non-empty collection of fork paths.
};

/// Event Tree representation with MEF constructs.
class EventTree : public Element, private boost::noncopyable {
 public:
  using Element::Element;

  /// @returns The initial state branch of the event tree.
  const Branch& initial_state() const { return initial_state_; }

  /// Sets the initial state of the event tree.
  void initial_state(Branch branch) { initial_state_ = std::move(branch); }

  /// @returns The container of event tree constructs of specific kind
  ///          with construct original names as keys.
  /// @{
  const ElementTable<FunctionalEventPtr>& functional_events() const {
    return functional_events_;
  }
  const ElementTable<NamedBranchPtr>& branches() const { return branches_; }
  /// @}

  /// Adds event tree constructs into the container.
  ///
  /// @param[in] element  A unique element defined in this event tree.
  ///
  /// @throws ValidationError  The element is already in this container.
  ///
  /// @{
  void Add(SequencePtr element);
  void Add(FunctionalEventPtr element);
  void Add(NamedBranchPtr element);
  void Add(std::unique_ptr<Fork> element) {
    forks_.push_back(std::move(element));
  }
  /// @}

 private:
  Branch initial_state_;  ///< The starting point.

  /// Containers for unique event tree constructs defined in this event tree.
  /// @{
  ElementTable<SequencePtr> sequences_;
  ElementTable<FunctionalEventPtr> functional_events_;
  ElementTable<NamedBranchPtr> branches_;
  /// @}
  std::vector<std::unique_ptr<Fork>> forks_;  ///< Lifetime management of forks.
};

using EventTreePtr = std::unique_ptr<EventTree>;  ///< Unique trees in a model.

/// Event-tree Initiating Event.
class InitiatingEvent : public Element {
 public:
  using Element::Element;

  /// Associates an event tree to the initiating event.
  ///
  /// @param[in] event_tree  Fully initialized event tree container.
  void event_tree(EventTree* event_tree) {
    assert(!event_tree_ && event_tree && "Resetting or un-setting event tree.");
    event_tree_ = event_tree;
  }

  /// @returns The event tree of the initiating event.
  ///          nullptr if the event tree is not set.
  /// @{
  EventTree* event_tree() const { return event_tree_; }
  EventTree* event_tree() { return event_tree_; }
  /// @}

 private:
  EventTree* event_tree_ = nullptr;  ///< The optional event tree specification.
};

/// Unique initiating events in a model.
using InitiatingEventPtr = std::unique_ptr<InitiatingEvent>;

}  // namespace mef
}  // namespace scram

#endif  // SCRAM_SRC_EVENT_TREE_H_
