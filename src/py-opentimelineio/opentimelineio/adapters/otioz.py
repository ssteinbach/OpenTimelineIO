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

"""OTIOZ adapter - bundles otio files linked to local media

Takes as input an OTIO file that has media references which are all relative
local paths (ie file:///foo.mov) and bundles those files and the otio file into
a single zip file with the suffix .otioz.  Can error out if files aren't
locally referenced or provide missing references

Can also extract the content.otio file from an otioz bundle for processing.
"""

import os
import copy
import zipfile

from .. import (
    exceptions,
)

from . import (
    file_bundle_utils as utils,
    otio_json
)


def read_from_file(filepath, extract_to_directory=None):
    if not zipfile.is_zipfile(filepath):
        raise exceptions.OTIOError("Not a zipfile: {}".format(filepath))

    if extract_to_directory:
        output_media_directory = os.path.join(
            extract_to_directory,
            utils.BUNDLE_DIR_NAME
        )

        if not os.path.exists(extract_to_directory):
            raise exceptions.OTIOError(
                "Directory '{}' does not exist, cannot unpack otioz "
                "there.".format(extract_to_directory)
            )

        if os.path.exists(output_media_directory):
            raise exceptions.OTIOError(
                "Error: '{}' already exists on disk, cannot overwrite while "
                " unpacking OTIOZ file '{}'.".format(
                    output_media_directory,
                    filepath
                )

            )

    with zipfile.ZipFile(filepath, 'r') as zi:
        result = otio_json.read_from_string(zi.read(utils.BUNDLE_PLAYLIST_PATH))

        if extract_to_directory:
            zi.extractall(extract_to_directory)

    return result


def write_to_file(
    input_otio,
    filepath,
    unreachable_media_policy=utils.MediaReferencePolicy.ErrorIfNotFile,
    dryrun=False
):
    input_otio = copy.deepcopy(input_otio)

    manifest = utils._file_bundle_manifest(
        input_otio,
        filepath,
        unreachable_media_policy,
        "OTIOZ"
    )

    # dryrun reports the total size of files
    if dryrun:
        fsize = 0
        for fn in manifest:
            fsize += os.path.getsize(fn)
        return fsize

    fmapping = {}

    # gather the files up in the staging_dir
    for fn in manifest:
        target = os.path.join(utils.BUNDLE_DIR_NAME, os.path.basename(fn))
        fmapping[fn] = target

    # relink the media reference
    for cl in input_otio.each_clip():
        try:
            source_fpath = cl.media_reference.target_url
        except AttributeError:
            continue

        cl.media_reference.target_url = "file://{}".format(
            fmapping[source_fpath.split("file://")[1]]
        )

    # write the otioz file to the temp directory
    otio_str = otio_json.write_to_string(input_otio)

    with zipfile.ZipFile(filepath, mode='w') as target:
        # write the media
        for src, dst in fmapping.items():
            target.write(src, dst)

        # write the OTIO
        target.writestr(utils.BUNDLE_PLAYLIST_PATH, otio_str)

    return