#include "generators/generator_context.h"
#include "generators/html_generator.h"
#include "generators/hwp_generator.h"
#include "generators/latex_generator.h"
#include "metadata_repo.h"
#include "parser.h"

extern "C" {
const char* convert_markdown_to_html(const char* md) {
  md2::Parser parser;
  const md2::ParseTree tree = parser.GenerateParseTree(md);

  md2::MetadataRepo metadata_repo;
  md2::GeneratorOptions options;
  options.server_mode = true;

  md2::GeneratorContext context(metadata_repo, "",
                                /*use_clang_server=*/false,
                                /*clang_server_port=*/-1,
                                /*context=*/nullptr, options);

  md2::HTMLGenerator generator("", md, context, tree);
  generator.Generate();

  std::string html = std::move(generator).ReleaseGeneratedTarget();

  char* c_html = new char[html.length() + 1];
  memcpy(c_html, html.c_str(), html.length() + 1);

  return c_html;
}

const char* convert_markdown_to_hwp(const char* md) {
  md2::Parser parser;
  const md2::ParseTree tree = parser.GenerateParseTree(md);

  md2::MetadataRepo metadata_repo;
  md2::GeneratorOptions options;
  options.server_mode = true;

  md2::GeneratorContext context(metadata_repo, "",
                                /*use_clang_server=*/false,
                                /*clang_server_port=*/-1,
                                /*context=*/nullptr, options);

  md2::HwpGenerator generator("", md, context, tree);
  generator.Generate();

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
