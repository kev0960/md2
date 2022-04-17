#ifndef C_API_H
#define C_API_H

extern "C" {

struct HtmlGenerateConfig;
struct HwpGenerateConfig;
struct HwpConversionStatus;

// Convert markdown to html.
const char* convert_markdown_to_html(const char* md,
                                     HtmlGenerateConfig* render_config);

// Convert markdown to hwp.
// We need to pass vector<HwpGenerateConfig>
const char* convert_markdown_to_hwp(const char* md,
                                    const HwpGenerateConfig* render_config_start,
                                    int render_config_len,
                                    HwpConversionStatus* conversion_status);

// Convert markdown to hwp.
const char* convert_markdown_to_latex(const char* md,
                                      const char* image_dir_path,
                                      bool no_latex_image);

// Custom deallocator for const char*.
void deallocate(const char* s);
}

#endif

