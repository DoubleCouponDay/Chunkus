
pub const INPUTFILENAME: &str = "input.png";

pub const OUTPUT_SVG_FILE: &str = "output.svg";

pub const OUTPUTFILENAME: &str = "output.png";

pub const WAIT_FOR_MESSAGE_UPDATE_TIMEOUT_S: u64 = 5;


pub enum FfiResult {
    SuccessCode = 0,
    AssumptionWrong,
    TemplateFileNotFound,
    SvgSpaceError,
    ReadFileError,
    ArrayDiffSizeError,
    NullArgumentError,
    HashmapOom,
    OverflowError,
    BadArgumentError,
    NotPngError,
    NoBoundariesCreated,
    UnknownError
}

impl From<i32> for FfiResult {
    fn from(num: i32) -> Self {
        match num {
            0 => FfiResult::SuccessCode,
            1 => FfiResult::AssumptionWrong,
            2 => FfiResult::TemplateFileNotFound,
            3 => FfiResult::SvgSpaceError,
            4 => FfiResult::ReadFileError,
            5 => FfiResult::ArrayDiffSizeError,
            6 => FfiResult::NullArgumentError,
            7 => FfiResult::HashmapOom,
            8 => FfiResult::OverflowError,
            9 => FfiResult::BadArgumentError,
            10 => FfiResult::NotPngError,
            11 => FfiResult::NoBoundariesCreated,
            _ => FfiResult::UnknownError
        }
    }
}

impl Into<&'static str> for FfiResult {
    fn into(self) -> &'static str {
        ffiresult_to_string(&self)
    }
}

fn ffiresult_to_string(input: &FfiResult) -> &'static str {
    match input {
        FfiResult::SuccessCode => "SuccessCode",
        FfiResult::AssumptionWrong => "AssumptionWrong",
        FfiResult::TemplateFileNotFound => "TemplateFileNotFound",
        FfiResult::SvgSpaceError => "SvgSpaceError",
        FfiResult::ReadFileError => "ReadFileError",
        FfiResult::ArrayDiffSizeError => "ArrayDiffSizeError",
        FfiResult::NullArgumentError => "NullArgumentError",
        FfiResult::HashmapOom => "HashmapOom",
        FfiResult::OverflowError => "OverflowError",
        FfiResult::BadArgumentError => "BadArgumentError",
        FfiResult::NotPngError => "NotPngError",
        FfiResult::NoBoundariesCreated => "NoBoundariesCreated",
        FfiResult::UnknownError => "UnknownError"
    }
}
