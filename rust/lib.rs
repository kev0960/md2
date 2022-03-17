use libc::c_char;
use std::ffi::{CStr, CString};

#[link(name = "libmd2", kind = "static")]
extern "C" {
    fn convert_markdown_to_html(md: *const c_char) -> *const c_char;
    fn convert_markdown_to_hwp(
        md: *const c_char,
        config: *const HwpGenerateConfig,
    ) -> *const c_char;
    fn convert_markdown_to_latex(
        md: *const c_char,
        image_dir_path: *const c_char,
        no_image: bool,
    ) -> *const c_char;
    fn deallocate(s: *const c_char);
}

fn convert_markdown(
    md: &str,
    converter: unsafe extern "C" fn(*const c_char) -> *const c_char,
) -> Result<String, String> {
    let html;
    unsafe {
        let c_str_md;

        match CString::new(md) {
            Ok(s) => {
                c_str_md = s;
            }
            Err(e) => {
                return Err(e.to_string());
            }
        }

        let c_html_ptr = converter(c_str_md.as_ptr());
        let c_html = CStr::from_ptr(c_html_ptr);

        match c_html.to_str() {
            Ok(s) => {
                html = s.to_string();
            }
            Err(e) => {
                deallocate(c_html_ptr);
                return Err(e.to_string());
            }
        }

        deallocate(c_html_ptr);
    }

    Ok(html)
}

pub fn markdown_to_html(md: &str) -> Result<String, String> {
    convert_markdown(md, convert_markdown_to_html)
}

#[repr(C)]
pub struct HwpGenerateConfig {
    pub para_shape: i32,
    pub para_style: i32,
    pub char_shape: i32,
}

pub fn markdown_to_hwp(md: &str, render_config: &HwpGenerateConfig) -> Result<String, String> {
    let html;
    unsafe {
        let c_str_md;

        match CString::new(md) {
            Ok(s) => {
                c_str_md = s;
            }
            Err(e) => {
                return Err(e.to_string());
            }
        }

        let c_html_ptr = convert_markdown_to_hwp(c_str_md.as_ptr(), render_config);
        let c_html = CStr::from_ptr(c_html_ptr);

        match c_html.to_str() {
            Ok(s) => {
                html = s.to_string();
            }
            Err(e) => {
                deallocate(c_html_ptr);
                return Err(e.to_string());
            }
        }

        deallocate(c_html_ptr);
    }

    Ok(html)
}

pub fn markdown_to_latex(md: &str, image_dir_path: &str, no_image: bool) -> Result<String, String> {
    let html;
    unsafe {
        let c_str_md;

        match CString::new(md) {
            Ok(s) => {
                c_str_md = s;
            }
            Err(e) => {
                return Err(e.to_string());
            }
        }

        let c_str_image_dir_path;
        match CString::new(image_dir_path) {
            Ok(s) => {
                c_str_image_dir_path = s;
            }
            Err(e) => {
                return Err(e.to_string());
            }
        }

        let c_html_ptr =
            convert_markdown_to_latex(c_str_md.as_ptr(), c_str_image_dir_path.as_ptr(), no_image);
        let c_html = CStr::from_ptr(c_html_ptr);

        match c_html.to_str() {
            Ok(s) => {
                html = s.to_string();
            }
            Err(e) => {
                deallocate(c_html_ptr);
                return Err(e.to_string());
            }
        }

        deallocate(c_html_ptr);
    }

    Ok(html)
}

#[test]
fn convert_test() {
    assert_eq!(
        markdown_to_html("**a**").unwrap(),
        "<p><span class='font-weight-bold'>a</span></p>".to_owned()
    );

    assert_eq!(
        markdown_to_html("**한글도 되나요**").unwrap(),
        "<p><span class='font-weight-bold'>한글도 되나요</span></p>".to_owned()
    );
}

#[test]
fn latex_include_image() {
    assert_eq!(
        markdown_to_latex("a ![a](/image/hello)", "/home", false).unwrap(),
        "\na \n\\begin{figure}[H]\n\\centering\n\\includegraphics[max width=0.7\\linewidth]{/home/image/hello}\n\\end{figure}\n\n".to_owned()
    );
}

#[test]
fn latex_ignore_image() {
    assert_eq!(
        markdown_to_latex("a ![a](/image/hello)", "/home", /*no_image=*/ true).unwrap(),
        "\na \n".to_owned()
    );
}

#[test]
fn hwp_test() {
    assert_eq!(
        markdown_to_hwp(
            "a",
            &HwpGenerateConfig {
                para_shape: 17,
                para_style: 1,
                char_shape: 13
            }
        )
        .unwrap(),
        "<P ParaShape=\"17\" Style=\"1\"><TEXT CharShape=\"13\"><CHAR>a</CHAR></TEXT></P>"
            .to_owned()
    );
}

