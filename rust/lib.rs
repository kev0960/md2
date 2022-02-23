use libc::c_char;
use std::ffi::{CStr, CString};

#[link(name = "libmd2", kind = "static")]
extern "C" {
    fn convert_markdown_to_html(md: *const c_char) -> *const c_char;
    fn convert_markdown_to_hwp(md: *const c_char) -> *const c_char;
    fn convert_markdown_to_latex(md: *const c_char) -> *const c_char;
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

pub fn markdown_to_hwp(md: &str) -> Result<String, String> {
    convert_markdown(md, convert_markdown_to_hwp)
}

pub fn markdown_to_latex(md: &str) -> Result<String, String> {
    convert_markdown(md, convert_markdown_to_latex)
}

#[test]
fn convert_test() {
    assert_eq!(
        markdown_to_html("**a**").unwrap(),
        "<p><span class='font-weight-bold'>a</span></p>".to_string()
    );

    assert_eq!(
        markdown_to_html("**한글도 되나요**").unwrap(),
        "<p><span class='font-weight-bold'>한글도 되나요</span></p>".to_string()
    );

    assert_eq!(
        markdown_to_latex("**한글도 되나요**").unwrap(),
        "\n\\textbf{한글도 되나요}\n".to_string()
    );
}
