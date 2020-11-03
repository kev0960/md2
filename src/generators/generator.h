#ifndef GENERATORS_GENERATOR_H
#define GENERATORS_GENERATOR_H

#include <string>

#include "parse_tree.h"

namespace md2 {

// Generates the converted markdown file in a target language.
class Generator {
 public:
  Generator(std::string_view md) : md_(md) { targets_.push_back(&target_); }
  std::string_view ShowOutput() const { return target_; }

  std::string_view Generate(const ParseTree& parse_tree);

  // Release the generated target.
  std::string&& ReleaseGeneratedTarget() && { return std::move(target_); }

 protected:
  virtual void HandleParseTreeNode(const ParseTreeNode& node) = 0;

  // For the elements that are not part of the child nodes, it runs the
  // default_action(g, index); Otherwise, it just calls the HandleParseTreeNode
  // of the child node.
  void GenerateWithDefaultAction(const ParseTreeNode& node,
                                 std::function<void(int index)> default_action);

  void GenerateWithDefaultActionSpan(
      const ParseTreeNode& node, std::function<void(int index)> default_action,
      int start, int end);

  std::string* GetCurrentTarget() { return targets_.back(); }

  std::string_view md_;
  std::string target_;

  // The last target (targets_.back()) is the current target that we are working
  // on. It will be merged into the top most target (=target_) in the end.
  std::vector<std::string*> targets_;
};

}  // namespace md2

#endif
