#ifndef GENERATORS_GENERATOR_H
#define GENERATORS_GENERATOR_H

#include <string>

#include "generator_context.h"
#include "parse_tree.h"

namespace md2 {

// Generates the converted markdown file in a target language.
// This is *NOT* thread safe.
class Generator {
 public:
  Generator(std::string_view filename, std::string_view md,
            GeneratorContext& context, const ParseTree& parse_tree)
      : filename_(filename),
        md_(md),
        context_(&context),
        parse_tree_(parse_tree) {
    targets_.push_back(&target_);
  }
  std::string_view ShowOutput() const { return target_; }

  std::string_view Generate();

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

  std::string_view GetReferenceNodeGeneratedOutput(const std::string& ref_name);

  std::string_view GetFileTitle() const;

  std::string* GetCurrentTarget() { return targets_.back(); }

  template <typename Node>
  std::string_view GetStringInNode(Node* node, int prefix_offset = 0,
                                   int suffix_offset = 0) {
    return md_.substr(
        node->Start() + prefix_offset,
        node->End() - suffix_offset - node->Start() - prefix_offset);
  }

  std::string_view filename_;
  std::string_view md_;
  std::string target_;

  // The last target (targets_.back()) is the current target that we are working
  // on. It will be merged into the top most target (=target_) in the end.
  std::vector<std::string*> targets_;

  GeneratorContext* context_;

  const ParseTree& parse_tree_;

  // Mapping between the reference name and the generated data.
  std::unordered_map<std::string, std::string> ref_to_generated_;
};

}  // namespace md2

#endif
