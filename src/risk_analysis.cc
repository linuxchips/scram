/*
 * Copyright (C) 2014-2017 Olzhas Rakhimov
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

/// @file risk_analysis.cc
/// Implementation of risk analysis handler.

#include "risk_analysis.h"

#include "bdd.h"
#include "fault_tree.h"
#include "logger.h"
#include "mocus.h"
#include "random.h"
#include "zbdd.h"

namespace scram {
namespace core {

RiskAnalysis::RiskAnalysis(std::shared_ptr<const mef::Model> model,
                           const Settings& settings)
    : Analysis(settings),
      model_(std::move(model)) {}

void RiskAnalysis::Analyze() noexcept {
  assert(results_.empty() && "Rerunning the analysis.");
  // Set the seed for the pseudo-random number generator if given explicitly.
  // Otherwise it defaults to the implementation dependent value.
  if (Analysis::settings().seed() >= 0)
    Random::seed(Analysis::settings().seed());

  for (const mef::InitiatingEventPtr& initiating_event :
       model_->initiating_events()) {
    if (initiating_event->event_tree()) {
      LOG(INFO) << "Running event tree analysis: " << initiating_event->name();
      auto eta = std::make_unique<EventTreeAnalysis>(*initiating_event,
                                                     Analysis::settings());
      eta->Analyze();
      for (const EventTreeAnalysis::Result& result : eta->results()) {
        const mef::Sequence& sequence = result.sequence;
        LOG(INFO) << "Running analysis for sequence: " << sequence.name();
        RunAnalysis(*result.gate);
        LOG(INFO) << "Finished analysis for sequence: " << sequence.name();
      }
      event_tree_results_.push_back(std::move(eta));
      LOG(INFO) << "Finished event tree analysis: " << initiating_event->name();
    }
  }

  for (const mef::FaultTreePtr& ft : model_->fault_trees()) {
    for (const mef::Gate* target : ft->top_events()) {
      LOG(INFO) << "Running analysis for gate: " << target->id();
      RunAnalysis(*target);
      LOG(INFO) << "Finished analysis for gate: " << target->id();
    }
  }
}

void RiskAnalysis::RunAnalysis(const mef::Gate& target) noexcept {
  switch (Analysis::settings().algorithm()) {
    case Algorithm::kBdd:
      RunAnalysis<Bdd>(target);
      break;
    case Algorithm::kZbdd:
      RunAnalysis<Zbdd>(target);
      break;
    case Algorithm::kMocus:
      RunAnalysis<Mocus>(target);
  }
}

template <class Algorithm>
void RiskAnalysis::RunAnalysis(const mef::Gate& target) noexcept {
  auto fta =
      std::make_unique<FaultTreeAnalyzer<Algorithm>>(target,
                                                     Analysis::settings());
  fta->Analyze();
  Result result{target};
  if (Analysis::settings().probability_analysis()) {
    switch (Analysis::settings().approximation()) {
      case Approximation::kNone:
        RunAnalysis<Algorithm, Bdd>(fta.get(), &result);
        break;
      case Approximation::kRareEvent:
        RunAnalysis<Algorithm, RareEventCalculator>(fta.get(), &result);
        break;
      case Approximation::kMcub:
        RunAnalysis<Algorithm, McubCalculator>(fta.get(), &result);
    }
  }
  result.fault_tree_analysis = std::move(fta);
  results_.push_back(std::move(result));
}

template <class Algorithm, class Calculator>
void RiskAnalysis::RunAnalysis(FaultTreeAnalyzer<Algorithm>* fta,
                               Result* result) noexcept {
  auto pa = std::make_unique<ProbabilityAnalyzer<Calculator>>(
      fta, model_->mission_time().get());
  pa->Analyze();
  if (Analysis::settings().importance_analysis()) {
    auto ia = std::make_unique<ImportanceAnalyzer<Calculator>>(pa.get());
    ia->Analyze();
    result->importance_analysis = std::move(ia);
  }
  if (Analysis::settings().uncertainty_analysis()) {
    auto ua = std::make_unique<UncertaintyAnalyzer<Calculator>>(pa.get());
    ua->Analyze();
    result->uncertainty_analysis = std::move(ua);
  }
  result->probability_analysis = std::move(pa);
}

}  // namespace core
}  // namespace scram
