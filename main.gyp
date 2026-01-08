{
  'target_defaults': {
    'conditions': [
      [ 'OS=="win"', {
        'include_dirs': [
          'vcpkg_installed/<(target_arch)-windows/include/fmt',
          'vcpkg_installed/<(target_arch)-windows/include/wil',
        ],
        'defines': [
          '_WINDLL',
          'WIN32_LEAN_AND_MEAN',
          '_UNICODE',
          'UNICODE',
          '_CRT_SECURE_NO_DEPRECATE',
          '_CRT_NONSTDC_NO_DEPRECATE',
        ],
        'msvs_settings': {
          'WindowsTargetPlatformVersion': '10',
          'LanguageStandard': 'stdcpp20',
          'VCLinkerTool': {
            'AdditionalOptions': [
              '/guard:cf',
            ],
            'OptimizeReferences': 2,             # /OPT:REF
            'EnableCOMDATFolding': 2,            # /OPT:ICF
          },
          'VCCLCompilerTool': {
            'AdditionalOptions': [
              '/Zc:__cplusplus',
              '/std:c++20',
              '/Qspectre',
              '/guard:cf',
              '/utf-8'
            ],
            'BufferSecurityCheck': 'true',
            'ExceptionHandling': 1,               # /EHsc
            'EnableFunctionLevelLinking': 'true',
            'Optimization': 3,              # /Ox, full optimization
          },
        },
        'libraries': [
          '-ladvapi32.lib',
          '-lruntimeobject.lib',
          '-lshlwapi.lib',
          '-lonecore.lib',
        ]
      }],
    ],
  },
  'targets': [{
    'target_name': 'GitBashWTContextMenu',
    'type': 'shared_library',
    'sources': [
      'src/explorer_command_gitbash.cc',
      'src/explorer_command_gitbash.def',
    ],
    'defines': [
      'EXE_NAME="wt.exe"',
      'DIR_NAME="Git"',
    ],
    'conditions': [
      [ 'OS=="win"', {
        'conditions': [
          ['target_arch=="x64"', {
            'TargetMachine' : 17,             # /MACHINE:X64
            'defines': [
              'DLL_UUID="8A7F5B3E-2C91-4A5F-9B8D-1E2C3D4A5F6B"',
            ],
          }],
          ['target_arch=="arm64"', {
            'TargetMachine' : 18,             # /MACHINE:ARM64
            'defines': [
              'DLL_UUID="9C8E6D4F-3D02-5B6A-0C9E-2F3D4E5F6A7B"',
            ],
          }],
        ],
      }],
    ],
  }],
}
