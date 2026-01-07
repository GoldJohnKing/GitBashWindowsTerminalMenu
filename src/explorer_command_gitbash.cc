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

}

class __declspec(uuid(DLL_UUID)) ExplorerCommandHandler final : public RuntimeClass<RuntimeClassFlags<ClassicCom | InhibitRoOriginateError>, IExplorerCommand> {
 public:
  // IExplorerCommand implementation:
  IFACEMETHODIMP GetTitle(IShellItemArray* items, PWSTR* name) {
    const size_t kMaxStringLength = 1024;
    wchar_t value_w[kMaxStringLength] = L"";
    wchar_t expanded_value_w[kMaxStringLength] = L"";
    DWORD value_size_w = sizeof(value_w);
    const wchar_t kTitleRegkey[] = L"Software\\Classes\\GitBashWTContextMenu";
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
        return SHStrDup(value_w, name);
      }
    }

    return SHStrDup(L"Open in Git Bash", name);
  }

  IFACEMETHODIMP GetIcon(IShellItemArray* items, PWSTR* icon) {
    // Try to get Git Bash icon from Git installation
    std::filesystem::path git_icon_path = std::filesystem::path("C:\\Program Files\\Git\\mingw64\\share\\git\\git-for-windows.ico");
    if (std::filesystem::exists(git_icon_path)) {
      return SHStrDupW(git_icon_path.c_str(), icon);
    }

    // Fallback to wt.exe icon
    std::filesystem::path wt_path;
    PWSTR programFilesPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &programFilesPath);
    if (SUCCEEDED(hr)) {
      wt_path = std::filesystem::path(programFilesPath) / L"Microsoft\\WindowsApps\\wt.exe";
      CoTaskMemFree(programFilesPath);
      if (std::filesystem::exists(wt_path)) {
        return SHStrDupW(wt_path.c_str(), icon);
      }
    }

    // Final fallback to system32
    wt_path = L"C:\\Windows\\System32\\wt.exe";
    if (std::filesystem::exists(wt_path)) {
      return SHStrDupW(wt_path.c_str(), icon);
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
                      // Build command: wt.exe -p "Git Bash" -d "<path>"
                      std::wstring cmd = L"wt.exe -p \"Git Bash\" -d " + QuoteForCommandLineArg(path.get());

                      // Execute the command
                      HINSTANCE ret = ShellExecuteW(nullptr, L"open", L"wt.exe",
                          (L"-p \"Git Bash\" -d " + QuoteForCommandLineArg(path.get())).c_str(),
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
