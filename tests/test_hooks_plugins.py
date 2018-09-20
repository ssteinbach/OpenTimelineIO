#
# Copyright 2018 Pixar Animation Studios
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
import unittest
import os
# import pkg_resources
# import sys

import opentimelineio as otio
import baseline_reader # unit test module
import utils

HOOKSCRIPT_PATH = "hookscript_example"

POST_RUN_NAME = "hook ran and did stuff"
TEST_METADATA = {'extra_data' : True}

class HookScriptTest(unittest.TestCase, otio.test_utils.OTIOAssertions):
    """Tests for the hook function plugins."""
    def setUp(self):

        self.jsn = baseline_reader.json_baseline_as_string(HOOKSCRIPT_PATH)
        self.hook_script = otio.adapters.read_from_string(
            self.jsn,
            'otio_json'
        )
        self.hook_script._json_path = os.path.join(
            baseline_reader.MODPATH,
            "baselines",
            HOOKSCRIPT_PATH
        )

    def test_plugin_hook(self):
        self.assertEqual(self.hook_script.name, "example hook")
        self.assertEqual(self.hook_script.execution_scope, "in process")
        self.assertEqual(self.hook_script.filepath, "example.py")

    def test_plugin_hook_runs(self):
        tl = otio.schema.Timeline()
        tl = self.hook_script.run(tl)
        self.assertEqual(tl.name,POST_RUN_NAME)

class TestPluginHookSystem(unittest.TestCase):
    """ Test the hook point definition system """
    def setUp(self):
        self.man = utils.create_manifest()
        self.jsn = baseline_reader.json_baseline_as_string(HOOKSCRIPT_PATH)
        self.hsf = otio.adapters.otio_json.read_from_string(self.jsn)
        self.hsf._json_path = os.path.join(
            baseline_reader.MODPATH,
            "baselines",
            HOOKSCRIPT_PATH
        )

        self.orig_manifest = otio.plugins.manifest._MANIFEST
        otio.plugins.manifest._MANIFEST = self.man

    def tearDown(self):
        utils.remove_manifest(self.man)
        otio.plugins.manifest._MANIFEST = self.orig_manifest

    def test_plugin_adapter(self):
        self.assertEqual(self.hsf.name, "example hook")
        self.assertEqual(self.hsf.execution_scope, "in process")
        self.assertEqual(self.hsf.filepath, "example.py")

    def test_load_adapter_module(self):
        target = os.path.join(
            baseline_reader.MODPATH,
            "baselines",
            "example.py"
        )

        self.assertEqual(self.hsf.module_abs_path(), target)
        self.assertTrue(hasattr(self.hsf.module(), "hook_function"))

    def test_run_hook_function(self):
        tl = otio.schema.Timeline()

        result = self.hsf.run(tl, TEST_METADATA)
        self.assertEqual(result.name, POST_RUN_NAME)
        self.assertEqual(result.metadata.get("extra_data"), True)

    def test_serialize(self):

        self.assertEqual(
            str(self.hsf),
            "HookScript({}, {}, {})".format(
                repr(self.hsf.name),
                repr(self.hsf.execution_scope),
                repr(self.hsf.filepath)
            )
        )
        self.assertEqual(
            repr(self.hsf),
            "otio.hooks.HookScript("
            "name={}, "
            "execution_scope={}, "
            "filepath={}"
            ")".format(
                repr(self.hsf.name),
                repr(self.hsf.execution_scope),
                repr(self.hsf.filepath)
            )
        )

    def test_available_hookscript_names(self):
        # for not just assert that it returns a non-empty list
        self.assertEqual(otio.hooks.available_hookscripts()[0].name, self.hsf.name)
        self.assertEqual(otio.hooks.available_hookscript_names(), [self.hsf.name])

    def test_manifest_hooks(self):
        self.assertEqual(
            otio.hooks.names(),
            [
                "post_adapter_read",
                "pre_adapter_write",
            ]
        )

        self.assertEqual(
            otio.hooks.attached_scripts_to("pre_adapter_write"),
            [
                self.hsf,
                self.hsf
            ]
        )

        self.assertEqual(
            otio.hook.attached_scripts_to("post_adapter_read"),
            []
        )

        tl = otio.schema.Timeline()
        result = otio.hook.run("pre_adapter_write", tl, TEST_METADATA)
        self.assertEqual(result.name, POST_RUN_NAME)
        self.assertEqual(result.metadata, TEST_METADATA)

        self.assertEqual(otio.media_linker.default_media_linker(), 'foo')
        with self.assertRaises(otio.exceptions.NoDefaultMediaLinkerError):
            del os.environ['OTIO_DEFAULT_MEDIA_LINKER']
            otio.media_linker.default_media_linker()

    def test_from_name_fail(self):
        with self.assertRaises(otio.exceptions.NotSupportedError):
            otio.media_linker.from_name("should not exist")



if __name__ == '__main__':
    unittest.main()
