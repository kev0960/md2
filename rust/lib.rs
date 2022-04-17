use libc::c_char;
use std::ffi::{CStr, CString};

#[link(name = "libmd2", kind = "static")]
extern "C" {
    fn convert_markdown_to_html(
        md: *const c_char,
        render_config: *const HtmlGenerateConfig,
    ) -> *const c_char;
    fn convert_markdown_to_hwp(
        md: *const c_char,
        config: *const HwpGenerateConfigInternal,
        render_config_len: i32,
        conversion_status: *mut HwpConversionStatus,
    ) -> *const c_char;
    fn convert_markdown_to_latex(
        md: *const c_char,
        image_dir_path: *const c_char,
        no_image: bool,
    ) -> *const c_char;
    fn deallocate(s: *const c_char);
}

pub fn markdown_to_html(md: &str, config: &HtmlGenerateConfig) -> Result<String, String> {
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

        let c_html_ptr = convert_markdown_to_html(c_str_md.as_ptr(), config);
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

#[repr(C)]
pub struct HtmlGenerateConfig {
    pub inline_image_max_height: i32,
}

#[repr(C)]
pub struct HwpGenerateConfigInternal {
    pub entry_name: *const c_char,
    pub para_shape: i32,
    pub para_style: i32,
    pub char_shape: i32,
}

#[repr(C)]
pub struct HwpConversionStatus {
    pub bin_item: i32,
    pub inst_id: i32,
    pub z_order: i32,
}

pub struct HwpGenerateConfig {
    pub entry_name: String,
    pub para_shape: i32,
    pub para_style: i32,
    pub char_shape: i32,
}

pub fn markdown_to_hwp(
    md: &str,
    render_configs: &Vec<HwpGenerateConfig>,
    hwp_conversion_status: &mut HwpConversionStatus,
) -> Result<String, String> {
    let html;
    unsafe {
        let c_str_md;

        let entry_names: Vec<Result<CString, std::ffi::NulError>> = render_configs
            .into_iter()
            .map(|config| CString::new(config.entry_name.clone()))
            .collect();

        for entry in &entry_names {
            if entry.is_err() {
                return Err(entry.as_ref().err().unwrap().to_string());
            }
        }

        let render_configs_len = render_configs.len();

        let mut c_render_configs = Vec::new();
        c_render_configs.reserve(render_configs_len);

        for i in 0..render_configs_len {
            c_render_configs.push(HwpGenerateConfigInternal {
                entry_name: entry_names.get(i).unwrap().as_ref().unwrap().as_ptr(),
                para_style: render_configs[i].para_style,
                para_shape: render_configs[i].para_shape,
                char_shape: render_configs[i].char_shape,
            });
        }

        match CString::new(md) {
            Ok(s) => {
                c_str_md = s;
            }
            Err(e) => {
                return Err(e.to_string());
            }
        }
        let c_html_ptr = convert_markdown_to_hwp(
            c_str_md.as_ptr(),
            c_render_configs.as_ptr(),
            render_configs_len as i32,
            hwp_conversion_status,
        );
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
        markdown_to_html(
            "**a**",
            &HtmlGenerateConfig {
                inline_image_max_height: 10
            }
        )
        .unwrap(),
        "<p><span class='font-weight-bold'>a</span></p>".to_owned()
    );

    assert_eq!(
        markdown_to_html(
            "**한글도 되나요**",
            &HtmlGenerateConfig {
                inline_image_max_height: 10
            }
        )
        .unwrap(),
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
    let mut hwp_conversion_status = HwpConversionStatus {
        bin_item: 3,
        inst_id: 1,
        z_order: 1,
    };

    assert_eq!(
        markdown_to_hwp(
            "a ![](123,456) \n\n b",
            &vec![HwpGenerateConfig {
                entry_name: "problem_start".to_owned(),
                para_shape: 17,
                para_style: 1,
                char_shape: 13
            }, HwpGenerateConfig {
                entry_name: "default".to_owned(),
                para_shape: 2,
                para_style: 3,
                char_shape: 4
            }],
            &mut hwp_conversion_status
        )
        .unwrap(),
        "<P ParaShape=\"17\" Style=\"1\"><TEXT CharShape=\"13\"><CHAR>a </CHAR></TEXT><TEXT CharShape=\"0\"><PICTURE Reverse=\"false\"><SHAPEOBJECT InstId=\"1\" Lock=\"false\" NumberingType=\"Figure\" ZOrder=\"1\"><SIZE Height=\"123\" HeightRelTo=\"Absolute\" Protect=\"false\" Width=\"456\" WidthRelTo=\"Absolute\"/><POSITION AffectLSpacing=\"false\" AllowOverlap=\"false\" FlowWithText=\"true\" HoldAnchorAndSO=\"false\" HorzAlign=\"Left\" HorzOffset=\"0\" HorzRelTo=\"Para\" TreatAsChar=\"true\" VertAlign=\"Top\" VertOffset=\"0\" VertRelTo=\"Para\"/><OUTSIDEMARGIN Bottom=\"0\" Left=\"0\" Right=\"0\" Top=\"0\"/><SHAPECOMMENT></SHAPECOMMENT></SHAPEOBJECT><SHAPECOMPONENT CurHeight=\"123\" CurWidth=\"456\" GroupLevel=\"0\" HorzFlip=\"false\" InstID=\"2\" OriHeight=\"123\" OriWidth=\"456\" VertFlip=\"false\" XPos=\"0\" YPos=\"0\"><ROTATIONINFO Angle=\"0\" CenterX=\"228\" CenterY=\"61\" Rotate=\"1\"/><RENDERINGINFO><TRANSMATRIX E1=\"1.00000\" E2=\"0.00000\" E3=\"0.00000\" E4=\"0.00000\" E5=\"1.00000\" E6=\"0.00000\"/><SCAMATRIX E1=\"0.80000\" E2=\"0.00000\" E3=\"0.00000\" E4=\"0.00000\" E5=\"0.80000\" E6=\"0.00000\"/><ROTMATRIX E1=\"1.00000\" E2=\"0.00000\" E3=\"0.00000\" E4=\"0.00000\" E5=\"1.00000\" E6=\"0.00000\"/></RENDERINGINFO></SHAPECOMPONENT><IMAGERECT X0=\"0\" X1=\"456\" X2=\"456\" X3=\"0\" Y0=\"0\" Y1=\"0\" Y2=\"123\" Y3=\"123\"/><IMAGECLIP Bottom=\"123\" Left=\"0\" Right=\"456\" Top=\"0\"/><INSIDEMARGIN Bottom=\"0\" Left=\"0\" Right=\"0\" Top=\"0\"/><IMAGEDIM Height=\"123\" Width=\"456\"/><IMAGE Alpha=\"0\" BinItem=\"3\" Bright=\"0\" Contrast=\"0\" Effect=\"RealPic\"/><EFFECTS/></PICTURE></TEXT><TEXT CharShape=\"13\"><CHAR> </CHAR></TEXT></P><P ParaShape=\"2\" Style=\"3\"><TEXT CharShape=\"4\"><CHAR> b</CHAR></TEXT></P>"
            .to_owned()
    );

    assert_eq!(hwp_conversion_status.bin_item, 4);
}
