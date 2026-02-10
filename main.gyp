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
    'target_name': 'WSLUbuntuWTContextMenu',
    'type': 'shared_library',
    'sources': [
      'src/explorer_command_wslubuntu.cc',
      'src/explorer_command_wslubuntu.def',
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
              'DLL_UUID="D2E3F4A5-B6C7-8901-DEFA-2345678901BC"',
            ],
          }],
          ['target_arch=="arm64"', {
            'TargetMachine' : 18,             # /MACHINE:ARM64
            'defines': [
              'DLL_UUID="E3F4A5B6-C7D8-9012-EFAB-3456789012CD"',
            ],
          }],
        ],
      }],
    ],
  }],
}
