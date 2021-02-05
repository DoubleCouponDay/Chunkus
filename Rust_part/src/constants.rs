use std::fmt;


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
    NotPngError
}

impl From<i32> for FfiResult
{
    fn from(num: i32) -> Self
    {
        match num
        {
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
            _ => FfiResult::AssumptionWrong
        }
    }
}

impl fmt::Display for FfiResult
{
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result
    {
        match self
        {
            FfiResult::SuccessCode => write!(f, "SuccessCode"),
            FfiResult::AssumptionWrong => write!(f, "AssumptionWrong"),
            FfiResult::TemplateFileNotFound => write!(f, "TemplateFileNotFound"),
            FfiResult::SvgSpaceError => write!(f, "SvgSpaceError"),
            FfiResult::ReadFileError => write!(f, "ReadFileError"),
            FfiResult::ArrayDiffSizeError => write!(f, "ArrayDiffSizeError"),
            FfiResult::NullArgumentError => write!(f, "NullArgumentError"),
            FfiResult::HashmapOom => write!(f, "HashmapOom"),
            FfiResult::OverflowError => write!(f, "OverflowError"),
            FfiResult::BadArgumentError => write!(f, "BadArgumentError"),
            FfiResult::NotPngError => write!(f, "NotPngError")
        }
    }
}