#[cfg(target_os = "windows")]
use winapi::shared::minwindef;

#[cfg(target_os = "windows")]
use ntapi::ntrtl::RtlNtStatusToDosError;

#[cfg(target_os = "windows")]
use winapi::um::{libloaderapi, winbase, winnt};

#[cfg(target_os = "linux")]
use sysexit;

#[cfg(target_os = "windows")]
fn to_wstring(value: &str) -> Vec<u16> {
    use std::os::windows::ffi::OsStrExt;

    std::ffi::OsStr::new(value)
        .encode_wide()
        .chain(std::iter::once(0))
        .collect()
}

#[cfg(target_os = "windows")]
pub unsafe fn pwstr_to_string(ptr: winnt::PWSTR) -> String {
    use std::slice::from_raw_parts;
    let len = (0_usize..)
        .find(|&n| *ptr.offset(n as isize) == 0)
        .expect("Couldn't find null terminator");
    let array: &[u16] = from_raw_parts(ptr, len);
    String::from_utf16_lossy(array)
}

#[cfg(target_os = "windows")]
fn error_string_wininet(errno: i32) -> String {
    let mut err_msg: winnt::LPWSTR = std::ptr::null_mut();
    let hmodule = unsafe {
        libloaderapi::LoadLibraryExW(
            to_wstring("wininet.dll").as_ptr(),
            std::ptr::null_mut(),
            libloaderapi::DONT_RESOLVE_DLL_REFERENCES,
        )
    };

    if hmodule != std::ptr::null_mut() {
        let ret = unsafe {
            winbase::FormatMessageW(
                winbase::FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | winbase::FORMAT_MESSAGE_FROM_HMODULE
                    | winbase::FORMAT_MESSAGE_FROM_SYSTEM
                    | winbase::FORMAT_MESSAGE_IGNORE_INSERTS
                    | winbase::FORMAT_MESSAGE_MAX_WIDTH_MASK,
                hmodule as minwindef::LPCVOID,
                errno as u32,
                winnt::MAKELANGID(winnt::LANG_ENGLISH, winnt::SUBLANG_DEFAULT) as u32,
                (&mut err_msg as *mut winnt::LPWSTR) as winnt::LPWSTR,
                0,
                std::ptr::null_mut(),
            )
        };

        unsafe {
            libloaderapi::FreeLibrary(hmodule);
        }

        if ret == 0 {
            String::from("Unknown.")
        } else {
            let ret = unsafe { pwstr_to_string(err_msg) };

            unsafe {
                winbase::LocalFree(err_msg as minwindef::HLOCAL);
            }

            ret
        }
    } else {
        String::from("Unknown.")
    }
}

#[cfg(target_os = "linux")]
pub fn error_string(errno:i32) -> String {
    sysexit::Code::from(errno).to_string()
}

#[cfg(target_os = "windows")]
pub fn error_string(errno: i32) -> String {
    let rtstatus: u32 = unsafe {
        RtlNtStatusToDosError(errno)
    };

    let mut err_msg: winnt::LPWSTR = std::ptr::null_mut();
    let ret = unsafe {
        winbase::FormatMessageW(
            winbase::FORMAT_MESSAGE_ALLOCATE_BUFFER
                | winbase::FORMAT_MESSAGE_FROM_SYSTEM
                | winbase::FORMAT_MESSAGE_IGNORE_INSERTS
                | winbase::FORMAT_MESSAGE_MAX_WIDTH_MASK,
            std::ptr::null_mut(),
            rtstatus,
            winnt::MAKELANGID(winnt::LANG_ENGLISH, winnt::SUBLANG_DEFAULT) as u32,
            (&mut err_msg as *mut winnt::LPWSTR) as winnt::LPWSTR,
            0,
            std::ptr::null_mut(),
        )
    };

    if ret == 0 {
        // Is it a network-related error?
        error_string_wininet(errno)
    } else {
        let ret = unsafe { pwstr_to_string(err_msg) };

        unsafe {
            winbase::LocalFree(err_msg as minwindef::HLOCAL);
        }

        ret
    }
}