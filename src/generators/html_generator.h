#ifndef GENERATORS_HTML_GENERATOR_H
#define GENERATORS_HTML_GENERATOR_H

#include <vector>

#include "../string_util.h"
#include "generator.h"

namespace md2 {

struct HTMLLinkBuilder {
  std::string link_desc;
  std::string link_url;
};

struct HTMLImageBuilder {
  std::string alt;
  std::string caption;
  std::string size;
  std::string url;
};

class HTMLGenerator : public Generator {
 public:
  HTMLGenerator() { targets_.push_back(&target_); }

  std::string* GetCurrentTarget() { return targets_.back(); }

  void EmitPStart() override { GetCurrentTarget()->append("<p>"); }
  void EmitPEnd() override { GetCurrentTarget()->append("</p>"); }

  void EmitBoldStart() override {
    GetCurrentTarget()->append("<span class='font-weight-bold'>");
  }
  void EmitBoldEnd() override { GetCurrentTarget()->append("</span>"); }

  void EmitItalicStart() override {
    GetCurrentTarget()->append("<span class='font-italic'>");
  }
  void EmitItalicEnd() override { GetCurrentTarget()->append("</span>"); }

  void StartLink() override { links_.push_back(HTMLLinkBuilder()); }
  void EmitLinkUrlStart() override {
    targets_.push_back(&links_.back().link_url);
  }
  void EmitLinkUrlEnd() override { targets_.pop_back(); }

  void EmitLinkDescStart() override {
    targets_.push_back(&links_.back().link_desc);
  }
  void EmitLinkDescEnd() override { targets_.pop_back(); }

  void EndLink() override {
    const HTMLLinkBuilder& link = links_.back();
    GetCurrentTarget()->append(
        StrCat("<a href='", link.link_url, "'>", link.link_desc, "</a>"));
    links_.pop_back();
  }

  void StartImage() override { images_.push_back(HTMLImageBuilder()); }

  void EmitImageUrlStart() override { targets_.push_back(&images_.back().url); }

  void EmitImageUrlEnd() override { targets_.pop_back(); }

  void EmitImageAltStart() override { targets_.push_back(&images_.back().alt); }
  void EmitImageAltEnd() override { targets_.pop_back(); }
  void EmitImageCaptionStart() override {
    targets_.push_back(&images_.back().caption);
  }
  void EmitImageCaptionEnd() override { targets_.pop_back(); }
  void EmitImageSizeStart() override {
    targets_.push_back(&images_.back().size);
  }
  void EmitImageSizeEnd() override { targets_.pop_back(); }
  void EndImage() override {
    const HTMLImageBuilder& image = images_.back();
    GetCurrentTarget()->append(
        StrCat("<figure><picture><img class='content-img' src='", image.url,
               "' alt=", image.alt, "'>", "</picture><figcaption>",
               image.caption, "</figcaption></figure>"));
    images_.pop_back();
  }

  void EmitHeader(); 

 private:
  std::vector<HTMLLinkBuilder> links_;
  std::vector<HTMLImageBuilder> images_;
  std::vector<std::string*> targets_;
};

}  // namespace md2

#endif
