# SPDX-License-Identifier: Apache-2.0
# Copyright Contributors to the OpenTimelineIO project

import unittest
import os
import sys
import subprocess

from opentimelineio.console import (
    autogen_serialized_datamodel as asd,
    autogen_plugin_documentation as apd,
    autogen_version_map as avm
)


@unittest.skipIf(
    os.environ.get("OTIO_DISABLE_SERIALIZED_SCHEMA_TEST"),
    "Serialized schema test disabled because "
    "$OTIO_DISABLE_SERIALIZED_SCHEMA_TEST is set to something other than ''"
)
class SerializedSchemaTester(unittest.TestCase):
    def test_serialized_schema(self):
        """Test if the schema has changed since last time the serialized schema
        documentation was generated.
        """

        pt = os.path.dirname(os.path.dirname(__file__))
        fp = os.path.join(pt, "docs", "tutorials", "otio-serialized-schema.md")
        with open(fp) as fi:
            baseline_text = fi.read()

        test_text, _ = asd.generate_and_write_documentation()

        self.maxDiff = None
        self.longMessage = True
        self.assertMultiLineEqual(
            baseline_text,
            test_text,
            "\n The schema has changed and the autogenerated documentation in {}"
            " needs to be updated.  run: `make doc-model-update`".format(fp)
        )


@unittest.skipIf(
    os.environ.get("OTIO_DISABLE_SERIALIZED_SCHEMA_TEST"),
    "Plugin documentation test disabled because "
    "$OTIO_DISABLE_SERIALIZED_SCHEMA_TEST is set to something other than ''"
)
class PluginDocumentationTester(unittest.TestCase):
    def test_plugin_documentation(self):
        """Verify that the plugin manifest matches what is checked into the
        documentation.
        """

        pt = os.path.dirname(os.path.dirname(__file__))
        fp = os.path.join(pt, "docs", "tutorials", "otio-plugins.md")
        with open(fp) as fi:
            baseline_text = fi.read()

        test_text = apd.generate_and_write_documentation_plugins(
            public_only=True,
            sanitized_paths=True
        )

        self.maxDiff = None
        self.longMessage = True
        self.assertMultiLineEqual(
            baseline_text,
            test_text,
            "\n The schema has changed and the autogenerated documentation in {}"
            " needs to be updated.  run: `make doc-plugins-update`".format(fp)
        )


@unittest.skipIf(
    os.environ.get("OTIO_DISABLE_SERIALIZED_SCHEMA_TEST"),
    "CORE_VERSION_MAP generation test disabled because "
    "$OTIO_DISABLE_SERIALIZED_SCHEMA_TEST is set to something other than ''"
)
class CoreVersionMapGenerationTester(unittest.TestCase):
    def test_core_version_map_generator(self):
        """Verify the current CORE_VERSION_MAP matches the checked in one."""

        pt = os.path.dirname(os.path.dirname(__file__))
        root = os.path.join(pt, "src", "opentimelineio")
        template_fp = os.path.join(root, "CORE_VERSION_MAP.last.cpp")
        target_fp = os.path.join(root, "CORE_VERSION_MAP.cpp")

        with open(target_fp) as fi:
            baseline_text = fi.read()

        proc = subprocess.Popen(
            [
                sys.executable,
                avm.__file__,
                "-i",
                template_fp,
                "-d",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        stdout, _ = proc.communicate()

        test_text = stdout.decode("utf-8")[:-1]

        self.maxDiff = None
        self.longMessage = True
        self.assertMultiLineEqual(
            baseline_text,
            test_text,
            "\n The CORE_VERSION_MAP has changed and the autogenerated one in"
            " {} needs to be updated. run: `make version-map-update`".format(
                target_fp
            )
        )


if __name__ == '__main__':
    unittest.main()
