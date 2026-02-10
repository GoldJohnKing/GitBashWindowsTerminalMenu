#!/usr/bin/env python3
from __future__ import print_function
from shutil import copy
import os
import sys

# Configuration for different variants
VARIANT_CONFIG = {
    "gitbash": {
        "clsid_map": {
            "x86": "1A8B6C4F-4E13-5C7A-1D8F-3A4B5C6D7E8F",
            "x64": "8A7F5B3E-2C91-4A5F-9B8D-1E2C3D4A5F6B",
            "arm64": "9C8E6D4F-3D02-5B6A-0C9E-2F3D4E5F6A7B",
        },
        "PackageName": "GitBashWTContextMenu",
        "PackageDisplayName": "Git Bash Windows Terminal Context Menu",
        "PackageDLL": "GitBashWTContextMenu.dll",
        "MenuID": "OpenGitBashWindowsTerminal",
        "ApplicationIdShort": "GitBashWT",
        "Application": "GitBashWTContextMenu",
        "PublisherDisplayName": "GitBashWTContextMenu",
        "Publisher": "GitBashWTContextMenu",
        "PackageDescription": "Git Bash Windows Terminal Context Menu",
    },
    "wslubuntu": {
        "clsid_map": {
            "x86": "1A8B6C4F-4E13-5C7A-1D8F-3A4B5C6D7E8F",
            "x64": "8A7F5B3E-2C91-4A5F-9B8D-1E2C3D4A5F6B",
            "arm64": "9C8E6D4F-3D02-5B6A-0C9E-2F3D4E5F6A7B",
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
pkg_type = sys.argv[1]
arch = sys.argv[2]

# Parse variant from pkg_type (which matches the variant name)
variant = pkg_type.lower()
if variant not in VARIANT_CONFIG:
    # Fallback for backward compatibility
    variant = "gitbash"

config = VARIANT_CONFIG[variant]
pkg_dir = os.path.join(out_dir, pkg_type + "_explorer_pkg_" + arch)

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
