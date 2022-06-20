#include "generators/generator_context.h"
#include "generators/html_generator.h"
#include "generators/hwp_generator.h"
#include "generators/hwp_state.h"
#include "generators/latex_generator.h"
#include "metadata_repo.h"
#include "parser.h"

extern "C" {

typedef struct HtmlGenerateConfig {
  int inline_image_max_height;

  // If set, then always append '/' at front for the image path.
  bool use_absolute_image_path = false;
} HtmlGenerateConfig;

const char* convert_markdown_to_html(const char* md,
                                     HtmlGenerateConfig* render_config) {
  md2::Parser parser;
  const md2::ParseTree tree = parser.GenerateParseTree(md);

  md2::MetadataRepo metadata_repo;
  md2::GeneratorOptions options;
  options.server_mode = true;

  md2::GeneratorContext context(metadata_repo, "",
                                /*use_clang_server=*/false,
                                /*clang_server_port=*/-1,
                                /*context=*/nullptr, options);

  md2::HTMLGenerator generator(
      "", md, context, tree,
      md2::HtmlGeneratorOptions{
          .inline_image_max_height = render_config->inline_image_max_height,
          .use_absolute_image_path = render_config->use_absolute_image_path});

  generator.Generate();

  std::string html = std::move(generator).ReleaseGeneratedTarget();

  char* c_html = new char[html.length() + 1];
  memcpy(c_html, html.c_str(), html.length() + 1);

  return c_html;
}

typedef struct HwpGenerateConfig {
  const char* entry_name;
  int para_shape;
  int para_style;
  int char_shape;
} HwpGenerateConfig;

typedef struct HwpConversionStatus {
  int bin_item;
  int inst_id;
  int z_order;
} HwpConversionStatus;

const char* convert_markdown_to_hwp(const char* md,
                                    const HwpGenerateConfig* render_config,
                                    int render_config_len,
                                    bool should_wrap_paragraph,
                                    HwpConversionStatus* conversion_status) {
  md2::Parser parser;
  const md2::ParseTree tree = parser.GenerateParseTree(md);

  md2::MetadataRepo metadata_repo;
  md2::GeneratorOptions options;
  options.server_mode = true;

  md2::GeneratorContext context(metadata_repo, "",
                                /*use_clang_server=*/false,
                                /*clang_server_port=*/-1,
                                /*context=*/nullptr, options);

  md2::HwpGenerator generator("", md, context, tree,
                              md2::HwpGenerator::HwpStatus{
                                  .inst_id = conversion_status->inst_id,
                                  .z_order = conversion_status->z_order,
                                  .bin_item = conversion_status->bin_item,
                              },
                              should_wrap_paragraph);

  for (int i = 0; i < render_config_len; i++) {
    std::string entry_name = render_config[i].entry_name;
    if (entry_name == "problem_start") {
      generator.GetHwpStateManager().AddParaShape(
          md2::HwpStateManager::HwpParaShapeType::PROBLEM_START_PARA,
          render_config[i].para_shape, render_config[i].para_style);
      generator.GetHwpStateManager().AddTextShape(
          md2::HwpStateManager::HwpCharShapeType::PROBLEM_START_CHAR,
          render_config[i].char_shape);
    } else if (entry_name == "default") {
      generator.GetHwpStateManager().AddParaShape(
          md2::HwpStateManager::HwpParaShapeType::PARA_REGULAR,
          render_config[i].para_shape, render_config[i].para_style);
      generator.GetHwpStateManager().AddTextShape(
          md2::HwpStateManager::HwpCharShapeType::CHAR_REGULAR,
          render_config[i].char_shape);
    }
  }

  generator.Generate();

  conversion_status->bin_item = generator.GetHwpStatus().bin_item;
  conversion_status->z_order = generator.GetHwpStatus().z_order;
  conversion_status->inst_id = generator.GetHwpStatus().inst_id;

  std::string html = std::move(generator).ReleaseGeneratedTarget();

  char* c_html = new char[html.length() + 1];
  memcpy(c_html, html.c_str(), html.length() + 1);

  return c_html;
}

const char* convert_markdown_to_latex(const char* md,
                                      const char* image_dir_path,
                                      bool no_latex_image) {
  md2::Parser parser;
  const md2::ParseTree tree = parser.GenerateParseTree(md);

  md2::MetadataRepo metadata_repo;
  md2::GeneratorOptions options;
  options.server_mode = true;
  options.no_latex_image = no_latex_image;

  md2::GeneratorContext context(metadata_repo, image_dir_path,
                                /*use_clang_server=*/false,
                                /*clang_server_port=*/-1,
                                /*context=*/nullptr, options);

  md2::LatexGenerator generator("", md, context, tree);
  generator.Generate();

  std::string tex = std::move(generator).ReleaseGeneratedTarget();

  char* c_html = new char[tex.length() + 1];
  memcpy(c_html, tex.c_str(), tex.length() + 1);

  return c_html;
}

void deallocate(const char* s) { delete[] s; }
}
