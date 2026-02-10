#!/usr/bin/env python3
from __future__ import print_function
from shutil import copy
import os
import sys

# Configuration for WSLUbuntu variant only
VARIANT_CONFIG = {
    "wslubuntu": {
        "clsid_map": {
            "x86": "C1D2E3F4-A5B6-7890-CDEF-1234567890AB",
            "x64": "D2E3F4A5-B6C7-8901-DEFA-2345678901BC",
            "arm64": "E3F4A5B6-C7D8-9012-EFAB-3456789012CD",
        },
        "PackageName": "WSLUbuntuWTContextMenu",
        "PackageDisplayName": "WSL Ubuntu Windows Terminal Context Menu",
        "PackageDLL": "WSLUbuntuWTContextMenu.dll",
        "MenuID": "WSLUbuntuContextMenu",
        "ApplicationIdShort": "WSLUbuntuWT",
        "Application": "WSLUbuntuWTContextMenu",
        "PublisherDisplayName": "WSLUbuntuWTContextMenu",
        "Publisher": "WSLUbuntuWTContextMenu",
        "PackageDescription": "WSL Ubuntu Windows Terminal Context Menu",
    },
}

root = os.path.dirname(os.path.dirname(__file__))
out_dir = os.path.join(root, "out")
arch = sys.argv[1]

variant = "wslubuntu"
config = VARIANT_CONFIG[variant]
pkg_dir = os.path.join(out_dir, variant + "_explorer_pkg_" + arch)

# Create output directory.
if not os.path.exists(pkg_dir):
    os.mkdir(pkg_dir)

# Update AppxManifest.
manifest = os.path.join(root, "template", "AppxManifest.xml")
with open(manifest, "r") as f:
    content = f.read()
    content = content.replace(
        "@@PublisherDisplayName@@", config["PublisherDisplayName"]
    )
    content = content.replace("@@Publisher@@", config["Publisher"])
    content = content.replace("@@PackageDescription@@", config["PackageDescription"])
    content = content.replace("@@PackageName@@", config["PackageName"])
    content = content.replace("@@PackageDisplayName@@", config["PackageDisplayName"])
    content = content.replace("@@Application@@", "wt.exe")
    content = content.replace("@@ApplicationIdShort@@", config["ApplicationIdShort"])
    content = content.replace("@@MenuID@@", config["MenuID"])
    content = content.replace("@@CLSID@@", config["clsid_map"][arch])
    content = content.replace("@@PackageDLL@@", config["PackageDLL"])

# Copy AppxManifest file to the package directory.
manifest_output = os.path.join(pkg_dir, "AppxManifest.xml")
with open(manifest_output, "w+") as f:
    f.write(content)
