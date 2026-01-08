// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#include <fmt/core.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <filesystem>
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
#include <optional>
#include <mutex>

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
  // Thread-safe cached values
  std::optional<std::wstring> g_cachedTitle;
  std::optional<std::wstring> g_cachedModulePath;

  // Ensure one-time initialization
  std::once_flag g_titleCacheFlag;
  std::once_flag g_modulePathCacheFlag;

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

void InitializeTitleCache() {
  const size_t kMaxStringLength = 1024;
  wchar_t value_w[kMaxStringLength];
  wchar_t expanded_value_w[kMaxStringLength];
  DWORD value_size_w = sizeof(value_w);

  #if defined(INSIDER)
    const wchar_t kTitleRegkey[] = L"Software\\Classes\\CodeInsidersModernExplorerMenu";
  #else
    const wchar_t kTitleRegkey[] = L"Software\\Classes\\CodeModernExplorerMenu";
  #endif

  HKEY subhkey = nullptr;
  LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, kTitleRegkey, 0, KEY_READ, &subhkey);
  if (result != ERROR_SUCCESS) {
    result = RegOpenKeyEx(HKEY_CURRENT_USER, kTitleRegkey, 0, KEY_READ, &subhkey);
  }

  if (result == ERROR_SUCCESS) {
    DWORD type = REG_EXPAND_SZ;
    result = RegQueryValueEx(subhkey, L"Title", nullptr, &type,
                    reinterpret_cast<LPBYTE>(&value_w), &value_size_w);
    RegCloseKey(subhkey);

    if (result == ERROR_SUCCESS) {
      value_size_w = ExpandEnvironmentStrings(value_w, expanded_value_w, kMaxStringLength);
      if (value_size_w && value_size_w < kMaxStringLength) {
        g_cachedTitle = expanded_value_w;
        return;
      }
    }
  }

  g_cachedTitle = L"UnExpected Title";
}

void InitializeModulePathCache() {
  // Get DLL location and navigate to target
  std::filesystem::path module_path{
    wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle())
  };
  module_path = module_path.remove_filename().parent_path().parent_path();
  module_path = module_path / DIR_NAME / EXE_NAME;

  // Try primary path first
  if (std::filesystem::exists(module_path)) {
    g_cachedModulePath = module_path.native();
    return;
  }

  // Fallback to "C:\\Program Files"
  std::filesystem::path fallback_path =
    std::filesystem::path("C:\\Program Files") / DIR_NAME / EXE_NAME;
  if (std::filesystem::exists(fallback_path)) {
    g_cachedModulePath = fallback_path.native();
    return;
  }

  // No valid path found - leave empty
  g_cachedModulePath = std::nullopt;
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
    std::call_once(g_modulePathCacheFlag, InitializeModulePathCache);
    if (g_cachedModulePath) {
      return SHStrDupW(g_cachedModulePath->c_str(), icon);
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
      // Ensure module path is cached
      std::call_once(g_modulePathCacheFlag, InitializeModulePathCache);

      if (!g_cachedModulePath) {
        return E_FAIL;  // No valid module path
      }

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
                      HINSTANCE ret = ShellExecuteW(nullptr, L"open",
                          g_cachedModulePath->c_str(),
                          QuoteForCommandLineArg(path.get()).c_str(),
                          nullptr, SW_SHOW);
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
