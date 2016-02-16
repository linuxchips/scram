#!/usr/bin/env python
#
# Copyright (C) 2014-2016 Olzhas Rakhimov
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Runs SCRAM with test inputs to examine the output visually."""

from __future__ import print_function

import subprocess
import sys


# Input that must be analyzed with the results printed.
PRINT_INPUTS = [
    "orphan_primary_event.xml",
    "unused_parameter.xml",
    "two_trees.xml",
    "two_top_events.xml",
    "two_top_through_formula.xml",
    "mixed_roles.xml",
    ]

# Correct corner case inputs without probability information
PASS_INPUTS = [
    "correct_tree_input.xml",
    "correct_formulas.xml",
    "component_definition.xml",
    "mixed_definitions.xml",
    "mixed_references.xml",
    "model_data_mixed_definitions.xml",
    "labels_and_attributes.xml",
    "very_long_mcs.xml",
    "unordered_structure.xml",
    "non_top_gate.xml",
    "nested_formula.xml"
    ]

# Correct corner cases with probability information
PASS_PROBS = [
    "correct_tree_input_with_probs.xml",
    "trailing_spaces.xml",
    "correct_expressions.xml",
    "flavored_types.xml",
    ]

# Incorrect input files without probability calculations
BAD_INPUTS = [
    "../xml_formatting_error.xml",
    "../schema_fail.xml",
    "../unsupported_feature.xml",
    "../unsupported_gate.xml",
    "../unsupported_expression.xml",
    "nonexistent_file.xml",
    "int_overflow.xml",
    "doubly_defined_gate.xml",
    "doubly_defined_house.xml",
    "doubly_defined_basic.xml",
    "doubly_defined_parameter.xml",
    "doubly_defined_ccf_group.xml",
    "extra_ccf_level_beta_factor.xml",
    "missing_gate_definition.xml",
    "missing_ccf_level_number.xml",
    "missing_ccf_members.xml",
    "undefined_event.xml",
    "undefined_basic_event.xml",
    "undefined_house_event.xml",
    "undefined_gate.xml",
    "undefined_parameter.xml",
    "reference_missing_fault_tree.xml",
    "reference_missing_component.xml",
    "wrong_parameter_unit.xml",
    "name_clash_two_trees.xml",
    "def_clash_basic_gate.xml",
    "def_clash_house_gate.xml",
    "def_clash_gate_primary.xml",
    "def_clash_basic_house.xml",
    "def_clash_house_basic.xml",
    "atleast_gate.xml",
    "cyclic_tree.xml",
    "cyclic_formula.xml",
    "cyclic_parameter.xml",
    "cyclic_expression.xml",
    "invalid_expression.xml",
    "repeated_child.xml",
    "alpha_ccf_level_error.xml",
    "beta_ccf_level_error.xml",
    "mgl_ccf_level_error.xml",
    "phi_ccf_wrong_sum.xml",
    "ccf_negative_factor.xml",
    "ccf_more_factors_than_needed.xml",
    "repeated_ccf_members.xml",
    ]

# Incorrect input files with probability calculations
BAD_PROBS = [
    "invalid_probability.xml",
    "missing_bool_constant.xml",
    "missing_expression.xml",
    "ccf_wrong_distribution.xml"
    ]

# Visual delimeters
BLOCK_DELIM = "=" * 80
FILE_DELIM = "-" * 80


def main():
    """Runs SCRAM with all registered inputs."""
    # Run correct inputs
    print("\nRUNNING CORRECT INPUTS WITH OUTPUT")
    print(BLOCK_DELIM)
    for i in PRINT_INPUTS:
        print("\nRUNNING : " + i)
        print(FILE_DELIM)
        args = ["scram", i]
        subprocess.call(args)
        print(FILE_DELIM)
    print(BLOCK_DELIM)

    # Run correct inputs
    print("\nVALIDATING CORRECT INPUTS WITHOUT PROBABILITY CALCULATION")
    print(BLOCK_DELIM)
    for i in PASS_INPUTS:
        print("\nVALIDATING : " + i)
        print(FILE_DELIM)
        args = ["scram", i, "--validate"]
        subprocess.call(args)
        print(FILE_DELIM)
    print(BLOCK_DELIM)

    # Run incorrect inputs
    print("\nVALIDATING INCORRECT INPUTS WITHOUT PROBABILITY CALCULATION")
    print(BLOCK_DELIM)
    for i in BAD_INPUTS:
        print("\nVALIDATING : " + i)
        print(FILE_DELIM)
        args = ["scram", i, "--validate"]
        try:
            subprocess.check_call(args)
        except subprocess.CalledProcessError:
            print(sys.exc_info()[0])
        print(FILE_DELIM)
    print(BLOCK_DELIM)

    # Run correct inputs with probabilities
    print("\nVALIDATING CORRECT PROBABILITY INPUTS")
    print(BLOCK_DELIM)
    for i in PASS_PROBS:
        print("\nVALIDATING : " + i)
        print(FILE_DELIM)
        args = ["scram", i, "--probability", "1", "--validate"]
        subprocess.call(args)
        print(FILE_DELIM)
    print(BLOCK_DELIM)

    # Run incorrect inputs with probability calculations
    print("\nVALIDATING INCORRECT PROBABILITY INPUTS")
    print(BLOCK_DELIM)
    for i in BAD_PROBS:
        print("\nVALIDATING : " + i)
        print(FILE_DELIM)
        args = ["scram", i, "--probability", "1", "--validate"]
        try:
            subprocess.check_call(args)
        except subprocess.CalledProcessError:
            print(sys.exc_info()[0])
        print(FILE_DELIM)
    print(BLOCK_DELIM)

if __name__ == "__main__":
    main()
