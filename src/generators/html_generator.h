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
    std::string* link_desc = targets_.back();
    targets_.pop_back();

    std::string* link_url = targets_.back();
    targets_.pop_back();

    GetCurrentTarget()->append(
        StrCat("<a href='", *link_url, "'>", *link_desc, "</a>"));
    links_.pop_back();
  }

 private:
  std::vector<HTMLLinkBuilder> links_;
  std::vector<std::string*> targets_;
};

}  // namespace md2

#endif
