#
# Copyright 2019 Pixar Animation Studios
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
#

import ast
import sys

"""Utilities for OpenTimelineIO commandline modules."""


def arg_list_to_map(arg_list, label):
    """
    Convert an argument of the form -A foo=bar from the parsed result to a map.
    """

    argument_map = {}
    for pair in arg_list:
        if '=' in pair:
            key, val = pair.split('=', 1)  # only split on the 1st '='
            try:
                # Sometimes we need to pass a bool, int, list, etc.
                parsed_value = ast.literal_eval(val)
            except (ValueError, SyntaxError):
                # Fall back to a simple string
                parsed_value = val
            argument_map[key] = parsed_value
        else:
            print(
                "error: {} arguments must be in the form key=value"
                " got: {}".format(label, pair)
            )
            sys.exit(1)

    return argument_map
