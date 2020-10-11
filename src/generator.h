#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>

namespace md2 {

// Generates the converted markdown file in a target language.
class Generator {
 public:
  virtual void Emit(int index) { target_.push_back(md_[index]); }

  // Emit the paragraph start character.
  virtual void EmitPStart() {}
  virtual void EmitPEnd() {}

  virtual void EmitBoldStart() {}
  virtual void EmitBoldEnd() {}

  virtual void EmitItalicStart() {}
  virtual void EmitItalicEnd() {}

  virtual void EmitLink() {}

 protected:
  std::string_view md_;
  std::string target_;
};

}  // namespace md2

#endif
