// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma execution_character_set("utf-8")
#include <fmt/core.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <shlwapi.h>
#include <shobjidl_core.h>
#include <userenv.h>
#include <wrl/module.h>
#include <wrl/implements.h>
#include <wrl/client.h>
#include "wil/stl.h"
#include "wil/filesystem.h"
#include "wil/win32_helpers.h"
#include <wil/cppwinrt.h>
#include <wil/resource.h>
#include <wil/com.h>

using Microsoft::WRL::ClassicCom;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::InhibitRoOriginateError;
using Microsoft::WRL::Module;
using Microsoft::WRL::ModuleType;
using Microsoft::WRL::RuntimeClass;
using Microsoft::WRL::RuntimeClassFlags;

extern "C" BOOL WINAPI DllMain(HINSTANCE instance,
                               DWORD reason,
                               LPVOID reserved) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }

  return true;
}

namespace {
  // Extracted from
  // https://source.chromium.org/chromium/chromium/src/+/main:base/command_line.cc;l=109-159

  std::wstring QuoteForCommandLineArg(const std::wstring& arg) {
  // We follow the quoting rules of CommandLineToArgvW.
  // http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
  std::wstring quotable_chars(L" \\\"");
  if (arg.find_first_of(quotable_chars) == std::wstring::npos) {
    // No quoting necessary.
    return arg;
  }

  std::wstring out;
  out.push_back('"');
  for (size_t i = 0; i < arg.size(); ++i) {
    if (arg[i] == '\\') {
      // Find the extent of this run of backslashes.
      size_t start = i, end = start + 1;
      for (; end < arg.size() && arg[end] == '\\'; ++end) {}
      size_t backslash_count = end - start;

      // Backslashes are escapes only if the run is followed by a double quote.
      // Since we also will end the string with a double quote, we escape for
      // either a double quote or the end of the string.
      if (end == arg.size() || arg[end] == '"') {
        // To quote, we need to output 2x as many backslashes.
        backslash_count *= 2;
      }
      for (size_t j = 0; j < backslash_count; ++j)
        out.push_back('\\');

      // Advance i to one before the end to balance i++ in loop.
      i = end - 1;
    } else if (arg[i] == '"') {
      out.push_back('\\');
      out.push_back('"');
    } else {
      out.push_back(arg[i]);
    }
  }
  out.push_back('"');

  return out;
}

// Thread-safe cache for menu metadata
std::optional<std::wstring> g_cachedTitle;
std::optional<std::wstring> g_cachedIconPath;
std::once_flag g_titleCacheFlag;
std::once_flag g_iconCacheFlag;

void InitializeTitleCache() {
  const size_t kMaxStringLength = 1024;
  wchar_t value_w[kMaxStringLength] = L"";
  DWORD value_size_w = sizeof(value_w);
  const wchar_t kTitleRegkey[] = L"Software\\Classes\\WSLUbuntuWTContextMenu";
  HKEY subhkey = nullptr;
  LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, kTitleRegkey, 0, KEY_READ, &subhkey);
  if (result != ERROR_SUCCESS) {
    result = RegOpenKeyEx(HKEY_CURRENT_USER, kTitleRegkey, 0, KEY_READ, &subhkey);
  }

  if (result == ERROR_SUCCESS) {
    DWORD type = 0;
    result = RegQueryValueExW(subhkey, L"Title", nullptr, &type,
                    reinterpret_cast<LPBYTE>(value_w), &value_size_w);
    RegCloseKey(subhkey);

    if (result == ERROR_SUCCESS && value_size_w > 0) {
      g_cachedTitle = value_w;
      return;
    }
  }

  g_cachedTitle = L"在 Ubuntu 中打开";
}

std::wstring FindUbuntuUuid() {
    const wchar_t kLxssRegKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Lxss";
    HKEY hKey = nullptr;
    
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kLxssRegKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return L"";
    }
    
    wchar_t subKeyName[256];
    DWORD subKeyNameSize = 256;
    DWORD index = 0;
    
    while (RegEnumKeyExW(hKey, index++, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        HKEY hSubKey = nullptr;
        if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
            wchar_t distroName[256];
            DWORD distroNameSize = sizeof(distroName);
            DWORD type = 0;
            
            if (RegQueryValueExW(hSubKey, L"DistributionName", nullptr, &type, 
                                 reinterpret_cast<LPBYTE>(distroName), &distroNameSize) == ERROR_SUCCESS) {
                if (wcscmp(distroName, L"Ubuntu") == 0) {
                    RegCloseKey(hSubKey);
                    RegCloseKey(hKey);
                    return std::wstring(subKeyName);
                }
            }
            RegCloseKey(hSubKey);
        }
        subKeyNameSize = 256;
    }
    
    RegCloseKey(hKey);
    return L"";
}

void InitializeIconCache() {
    // Try to get WSL Ubuntu icon
    std::wstring uuid = FindUbuntuUuid();
    if (!uuid.empty()) {
        wchar_t userProfile[MAX_PATH];
        if (GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH) > 0) {
            std::filesystem::path wslIconPath = std::filesystem::path(userProfile) / 
                L"AppData" / L"Local" / L"wsl" / uuid / L"shortcut.ico";
            if (std::filesystem::exists(wslIconPath)) {
                g_cachedIconPath = wslIconPath.wstring();
                return;
            }
        }
    }
    
    // Fall back to wt.exe icon
    std::filesystem::path wt_path;
    PWSTR programFilesPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &programFilesPath);
    if (SUCCEEDED(hr)) {
        wt_path = std::filesystem::path(programFilesPath) / L"Microsoft\\WindowsApps\\wt.exe";
        CoTaskMemFree(programFilesPath);
        if (std::filesystem::exists(wt_path)) {
            g_cachedIconPath = wt_path.wstring();
            return;
        }
    }
    
    std::filesystem::path wt_system32(L"C:\\Windows\\System32\\wt.exe");
    if (std::filesystem::exists(wt_system32)) {
        g_cachedIconPath = wt_system32.wstring();
        return;
    }
    
    g_cachedIconPath = std::nullopt;
}

}

class __declspec(uuid(DLL_UUID)) ExplorerCommandHandler final : public RuntimeClass<RuntimeClassFlags<ClassicCom | InhibitRoOriginateError>, IExplorerCommand> {
 public:
  // IExplorerCommand implementation:
  IFACEMETHODIMP GetTitle(IShellItemArray* items, PWSTR* name) {
    std::call_once(g_titleCacheFlag, InitializeTitleCache);
    return SHStrDup(g_cachedTitle->c_str(), name);
  }

  IFACEMETHODIMP GetIcon(IShellItemArray* items, PWSTR* icon) {
    std::call_once(g_iconCacheFlag, InitializeIconCache);
    if (g_cachedIconPath) {
      return SHStrDupW(g_cachedIconPath->c_str(), icon);
    }
    return E_FAIL;
  }

  IFACEMETHODIMP GetToolTip(IShellItemArray* items, PWSTR* infoTip) {
    *infoTip = nullptr;
    return E_NOTIMPL;
  }

  IFACEMETHODIMP GetCanonicalName(GUID* guidCommandName) {
    *guidCommandName = GUID_NULL;
    return S_OK;
  }

  IFACEMETHODIMP GetState(IShellItemArray* items, BOOL okToBeSlow, EXPCMDSTATE* cmdState) {
    *cmdState = ECS_ENABLED;
    return S_OK;
  }

  IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) {
    *flags = ECF_DEFAULT;
    return S_OK;
  }

  IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** enumCommands) {
    *enumCommands = nullptr;
    return E_NOTIMPL;
  }

  IFACEMETHODIMP Invoke(IShellItemArray* items, IBindCtx* bindCtx) {
      if (items) {
          DWORD count;
          RETURN_IF_FAILED(items->GetCount(&count));
          for (DWORD i = 0; i < count; ++i) {
              ComPtr<IShellItem> item;
              auto result = items->GetItemAt(i, &item);
              if (SUCCEEDED(result)) {
                  wil::unique_cotaskmem_string path;
                  result = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                  if (SUCCEEDED(result)) {
                       HINSTANCE ret = ShellExecuteW(nullptr, L"open", L"wt.exe",
                           (L"-p \"Ubuntu\" wsl.exe --cd " + QuoteForCommandLineArg(path.get())).c_str(), nullptr, SW_SHOW);
                      if ((INT_PTR)ret <= HINSTANCE_ERROR) {
                          RETURN_LAST_ERROR();
                      }
                  }
              }
          }
      }
      return S_OK;
  }
};

CoCreatableClass(ExplorerCommandHandler)
CoCreatableClassWrlCreatorMapInclude(ExplorerCommandHandler)

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
  if (ppv == nullptr)
    return E_POINTER;
  *ppv = nullptr;
  return Module<ModuleType::InProc>::GetModule().GetClassObject(rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void) {
  return Module<ModuleType::InProc>::GetModule().GetObjectCount() == 0 ? S_OK : S_FALSE;
}

STDAPI DllGetActivationFactory(HSTRING activatableClassId,
                               IActivationFactory** factory) {
  return Module<ModuleType::InProc>::GetModule().GetActivationFactory(activatableClassId, factory);
}
