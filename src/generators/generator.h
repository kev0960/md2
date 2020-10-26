#ifndef GENERATORS_GENERATOR_H
#define GENERATORS_GENERATOR_H

#include <string>

namespace md2 {

// Generates the converted markdown file in a target language.
class Generator {
 public:
  Generator(std::string_view md) : md_(md) {}

  virtual void Emit(int index) { target_.push_back(md_[index]); }
  virtual void Emit(int from, int end) {
    target_.append(md_.substr(from, end - from));
  }

  // Emit the paragraph start character.
  virtual void EmitPStart() {}
  virtual void EmitPEnd() {}

  virtual void EmitBoldStart() {}
  virtual void EmitBoldEnd() {}

  virtual void EmitItalicStart() {}
  virtual void EmitItalicEnd() {}

  virtual void StartLink() {}
  virtual void EmitLinkUrlStart() {}
  virtual void EmitLinkUrlEnd() {}
  virtual void EmitLinkDescStart() {}
  virtual void EmitLinkDescEnd() {}
  virtual void EndLink() {}

  virtual void StartImage() {}
  virtual void EmitImageUrlStart() {}
  virtual void EmitImageUrlEnd() {}
  virtual void EmitImageAltStart() {}  // Alt tag of the image.
  virtual void EmitImageAltEnd() {}
  virtual void EmitImageCaptionStart() {}
  virtual void EmitImageCaptionEnd() {}
  virtual void EmitImageSizeStart() {}
  virtual void EmitImageSizeEnd() {}
  virtual void EndImage() {}

  virtual void EmitRegularHeader(int header_size) {}
  virtual void EmitLectureHeader(int start, int end) {}
  virtual void EmitTemplate() {}

  virtual void EmitTableStart() {}
  virtual void EmitTableEnd() {}

  virtual void EmitTableHeaderStart() {}
  virtual void EmitTableHeaderEnd() {}
  virtual void EmitTableHeaderCellStart() {}
  virtual void EmitTableHeaderCellEnd() {}

  virtual void EmitTableBodyStart() {}
  virtual void EmitTableBodyEnd() {}
  virtual void EmitTableRowStart() {}
  virtual void EmitTableRowEnd() {}
  virtual void EmitTableCellStart() {}
  virtual void EmitTableCellEnd() {}

  virtual void EmitInlineVerbatimStart() {}
  virtual void EmitInlineVerbatimEnd() {}

  virtual void EmitListStart() {}
  virtual void EmitListEnd() {}
  virtual void EmitListItemStart() {}
  virtual void EmitListItemEnd() {}

  virtual void EmitOrderedListStart() {}
  virtual void EmitOrderedListEnd() {}
  virtual void EmitOrderedListItemStart() {}
  virtual void EmitOrderedListItemEnd() {}

  std::string_view ShowOutput() const { return target_; }

 protected:
  std::string_view md_;
  std::string target_;
};

}  // namespace md2

#endif
