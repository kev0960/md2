#ifndef GENERATORS_HTML_GENERATOR_H
#define GENERATORS_HTML_GENERATOR_H

#include <iostream>
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
  HTMLGenerator(std::string_view content) : Generator(content) {
    targets_.push_back(&target_);
  }

  std::string* GetCurrentTarget() { return targets_.back(); }

  void Emit(int index) override { GetCurrentTarget()->push_back(md_[index]); }
  void Emit(int from, int end) override {
    GetCurrentTarget()->append(md_.substr(from, end - from));
  }

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
               "' alt='", image.alt, "'>", "</picture><figcaption>",
               image.caption, "</figcaption></figure>"));
    images_.pop_back();
  }

  void EmitHeader();

  void EmitTableStart() override { GetCurrentTarget()->append("<table>"); }
  void EmitTableEnd() override { GetCurrentTarget()->append("</table>"); }

  void EmitTableHeaderStart() override {
    GetCurrentTarget()->append("<thead>");
  }
  void EmitTableHeaderEnd() override { GetCurrentTarget()->append("</thead>"); }
  void EmitTableHeaderCellStart() override {
    GetCurrentTarget()->append("<th>");
  }
  void EmitTableHeaderCellEnd() override {
    GetCurrentTarget()->append("</th>");
  }

  void EmitTableBodyStart() override { GetCurrentTarget()->append("<tbody>"); }
  void EmitTableBodyEnd() override { GetCurrentTarget()->append("</tbody>"); }
  void EmitTableRowStart() override { GetCurrentTarget()->append("<tr>"); }
  void EmitTableRowEnd() override { GetCurrentTarget()->append("</tr>"); }
  void EmitTableCellStart() override { GetCurrentTarget()->append("<td>"); }
  void EmitTableCellEnd() override { GetCurrentTarget()->append("</td>"); }

  void EmitInlineVerbatimStart() override {
    GetCurrentTarget()->append("<code class='inline-code'>");
  }

  void EmitInlineVerbatimEnd() override {
    GetCurrentTarget()->append("</code>");
  }

  void EmitListStart() override { GetCurrentTarget()->append("<ul>"); }
  void EmitListEnd() override { GetCurrentTarget()->append("</ul>"); }
  void EmitListItemStart() override { GetCurrentTarget()->append("<li>"); }
  void EmitListItemEnd() override { GetCurrentTarget()->append("</li>"); }

  void EmitOrderedListStart() override { GetCurrentTarget()->append("<ol>"); }
  void EmitOrderedListEnd() override { GetCurrentTarget()->append("</ol>"); }
  void EmitOrderedListItemStart() override {
    GetCurrentTarget()->append("<li>");
  }
  void EmitOrderedListItemEnd() override {
    GetCurrentTarget()->append("</li>");
  }

 private:
  std::vector<HTMLLinkBuilder> links_;
  std::vector<HTMLImageBuilder> images_;
  std::vector<std::string*> targets_;
};

}  // namespace md2

#endif
