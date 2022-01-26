#ifndef C_API_H
#define C_API_H

extern "C" {
  // Convert markdown to html.
  const char* convert_markdown_to_html(const char* md);

  // Convert markdown to hwp.
  const char* convert_markdown_to_hwp(const char* md);

  // Custom deallocator for const char*.
  void deallocate(const char* s);
}

#endif 

