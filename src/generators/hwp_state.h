#ifndef GENERATORS_HWP_STATE_H
#define GENERATORS_HWP_STATE_H

#include <optional>
#include <unordered_map>
#include <vector>

namespace md2 {

struct HwpCharShape {
  int id = 0;
  int border_fill_id = 0;
  int height = 0;
  unsigned int shade_color = 0;
  unsigned int text_color = 0;

  bool is_bold = false;
  bool is_italic = false;

  std::vector<int> font_ids;
  std::vector<int> char_spacings;
  std::vector<int> ratios;
  std::vector<int> rel_sizes;
  std::vector<int> char_offsets;
};

enum HwpParaAlign { JUSTIFY, LEFT, RIGHT };
struct HwpParaShape {
  int id = 0;
  int style = 0;
  HwpParaAlign align = HwpParaAlign::JUSTIFY;
  std::optional<std::string> heading = std::nullopt;

  int margin_indent = 0;
  int margin_space = 0;
  int border_fill = 0;
};

class HwpStateManager {
 public:
  enum HwpCharShapeType { CHAR_REGULAR, BOLD, ITALIC, PROBLEM_START_CHAR };
  enum HwpParaShapeType { PARA_REGULAR, NUMBERING, PROBLEM_START_PARA };

  void AddDefaultMappings() {
    char_shapes_[CHAR_REGULAR] = HwpCharShape{
        .id = 1,
        .border_fill_id = 2,
        .height = 1000,
        .shade_color = 4294967295,
        .text_color = 0,
        .font_ids = {2, 2, 2, 2, 2, 2, 2},
        .char_spacings = {0, 0, 0, 0, 0, 0, 0},
        .ratios = {100, 100, 100, 100, 100, 100, 100},
        .rel_sizes = {100, 100, 100, 100, 100, 100, 100},
        .char_offsets = {0, 0, 0, 0, 0, 0, 0},
    };

    char_shapes_[BOLD] = HwpCharShape{
        .id = 7,
        .border_fill_id = 2,
        .height = 1000,
        .shade_color = 4294967295,
        .text_color = 0,
        .is_bold = true,
        .font_ids = {2, 2, 2, 2, 2, 2, 2},
        .char_spacings = {0, 0, 0, 0, 0, 0, 0},
        .ratios = {100, 100, 100, 100, 100, 100, 100},
        .rel_sizes = {100, 100, 100, 100, 100, 100, 100},
        .char_offsets = {0, 0, 0, 0, 0, 0, 0},
    };

    char_shapes_[ITALIC] = HwpCharShape{
        .id = 8,
        .border_fill_id = 2,
        .height = 1000,
        .shade_color = 4294967295,
        .text_color = 0,
        .is_italic = true,
        .font_ids = {2, 2, 2, 2, 2, 2, 2},
        .char_spacings = {0, 0, 0, 0, 0, 0, 0},
        .ratios = {100, 100, 100, 100, 100, 100, 100},
        .rel_sizes = {100, 100, 100, 100, 100, 100, 100},
        .char_offsets = {0, 0, 0, 0, 0, 0, 0},
    };

    para_shapes_[PARA_REGULAR] = HwpParaShape{.id = 0,
                                              .style = 0,
                                              .margin_indent = 0,
                                              .margin_space = 160,
                                              .border_fill = 2};
    para_shapes_[NUMBERING] = HwpParaShape{.id = 20,
                                           .style = 0,
                                           .heading = "Number",
                                           .margin_indent = 0,
                                           .margin_space = 160,
                                           .border_fill = 2};
  }

  void AddParaShape(HwpParaShapeType para_shape, int id, int style) {
    para_shapes_[para_shape] = HwpParaShape{.id = id,
                                            .style = style,
                                            .margin_indent = 0,
                                            .margin_space = 160,
                                            .border_fill = 2};
  }

  void AddTextShape(HwpCharShapeType char_shape, int id) {
    char_shapes_[char_shape] = HwpCharShape{
        .id = id,
        .border_fill_id = 2,
        .height = 1000,
        .shade_color = 4294967295,
        .text_color = 0,
        .font_ids = {2, 2, 2, 2, 2, 2, 2},
        .char_spacings = {0, 0, 0, 0, 0, 0, 0},
        .ratios = {100, 100, 100, 100, 100, 100, 100},
        .rel_sizes = {100, 100, 100, 100, 100, 100, 100},
        .char_offsets = {0, 0, 0, 0, 0, 0, 0},
    };
  }

  int GetRegularCharShape(int paragraph_nest_count) const {
    if (paragraph_nest_count == 0) {
      if (auto itr = char_shapes_.find(PROBLEM_START_CHAR);
          itr != char_shapes_.end()) {
        return itr->second.id;
      }
    }

    return GetCharShape(HwpCharShapeType::CHAR_REGULAR);
  }

  int GetCharShape(HwpCharShapeType char_shape) const {
    return char_shapes_.find(char_shape)->second.id;
  }

  std::pair<int, int> GetRegularParaShape(int paragraph_nest_count) const {
    if (paragraph_nest_count == 0) {
      if (auto itr = para_shapes_.find(PROBLEM_START_PARA);
          itr != para_shapes_.end()) {
        return std::make_pair(itr->second.id, itr->second.style);
      }
    }

    return GetParaShape(HwpParaShapeType::PARA_REGULAR);
  }

  std::pair<int, int> GetParaShape(HwpParaShapeType para_shape) const {
    const auto& shape = para_shapes_.find(para_shape)->second;
    return std::make_pair(shape.id, shape.style);
  }

 private:
  std::unordered_map<HwpCharShapeType, HwpCharShape> char_shapes_;
  std::unordered_map<HwpParaShapeType, HwpParaShape> para_shapes_;
};  // namespace md2

}  // namespace md2

#endif
