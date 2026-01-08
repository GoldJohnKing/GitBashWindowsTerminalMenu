#!/usr/bin/env python3
from __future__ import print_function
from shutil import copy
import os
import sys

gitbash_windows_terminal_clsid_map = {
  'x86': '1A8B6C4F-4E13-5C7A-1D8F-3A4B5C6D7E8F',
  'x64': '8A7F5B3E-2C91-4A5F-9B8D-1E2C3D4A5F6B',
  'arm64': '9C8E6D4F-3D02-5B6A-0C9E-2F3D4E5F6A7B'
}

root = os.path.dirname(os.path.dirname(__file__))
out_dir = os.path.join(root, 'out')
pkg_type = sys.argv[1]
arch = sys.argv[2]
pkg_dir = os.path.join(out_dir, pkg_type + '_explorer_pkg_' + arch)

# Create output directory.
if not os.path.exists(pkg_dir):
    os.mkdir(pkg_dir)

# Update AppxManifest.
manifest = os.path.join(root, 'template', 'AppxManifest.xml')
with open(manifest, 'r') as f:
  content = f.read()
  content = content.replace('@@PublisherDisplayName@@', 'GitBashWTContextMenu')
  content = content.replace('@@Publisher@@', 'GitBashWTContextMenu')
  content = content.replace('@@PackageDescription@@', 'Git Bash Windows Terminal Context Menu')
  content = content.replace('@@PackageName@@', 'GitBashWTContextMenu')
  content = content.replace('@@PackageDisplayName@@', 'Git Bash Windows Terminal Context Menu')
  content = content.replace('@@Application@@', 'wt.exe')
  content = content.replace('@@ApplicationIdShort@@', 'GitBashWT')
  content = content.replace('@@MenuID@@', 'OpenGitBashWindowsTerminal')
  content = content.replace('@@CLSID@@', gitbash_windows_terminal_clsid_map[arch])
  content = content.replace('@@PackageDLL@@', 'GitBashWTContextMenu.dll')

# Copy AppxManifest file to the package directory.
manifest_output = os.path.join(pkg_dir, 'AppxManifest.xml')
with open(manifest_output, 'w+') as f:
  f.write(content)
