#pragma once

#include "linenoise-fltk.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

typedef int (*PROCESS_CB)(char *str, void *clienData);

class Fl_Console;
extern Fl_Console* s_console_instance;  // Static instance for callback

class Fl_Console : public Fl_Terminal {
private:
  int lastkey;
  struct linenoiseState l_state;
  char *buf;
  int buflen;
  std::string prompt;
  PROCESS_CB process_cb;
  void *process_cb_data;
  std::vector<std::string> available_commands;  // Store your command list
  
public:
  Fl_Console(int X,int Y,int W,int H, const char *L=0);
  ~Fl_Console(void);

  int handle(int e) FL_OVERRIDE;

  void resize(int X,int Y,int W,int H) FL_OVERRIDE {
    Fl_Terminal::resize(X,Y,W,H);
    // could update cols here but doesn't seem to work?
    // l_state.cols = display_columns();
    //    std::cout << "cols: " << l_state.cols << " w: " << W << " fontsize: " << textsize() << std::endl;
  }
  
  int getch(void) { return lastkey; }

  void set_callback(PROCESS_CB cb, void *cbdata) {
    process_cb = cb;
    process_cb_data = cbdata;
  }

  int do_callback(char *str)
  {
    if (!process_cb) return 0;
    return (process_cb(str, process_cb_data));
  }
  
  int init_linenoise(void)
  {
    lnInitState(&l_state, buf, buflen, prompt.c_str());
    l_state.cols = display_columns();
    l_state.mode = linenoiseState::ln_read_regular;
    l_state.smart_term_connected = true;  // Enable smart terminal features    
    linenoiseHistoryLoad(&l_state, "history.txt"); /* Load the history at startup */
    return 0;
  }

  std::string get_current_selection(void) {
    int srow,scol,erow,ecol;
    std::string sel;
    if (get_selection(srow,scol,erow,ecol)) {
      for (int row=srow; row<=erow; row++) {
	const Utf8Char *u8c = u8c_ring_row(row);
	int col_start = (row==srow) ? scol : 0;
	int col_end   = (row==erow) ? ecol : ring_cols();
	u8c += col_start;
	for (int col=col_start; col<col_end; col++,u8c++) { // Changed <= to 
	  sel += u8c->text_utf8();
	}
	if (row < erow) sel += '\n'; // Add newline between rows
      }
    }
    return sel;
  }  

  void copy_to_clipboard() {
    std::string sel = get_current_selection();
    if (!sel.empty()) {
      Fl::copy(sel.c_str(), sel.length(), 1);
    }
    clear_mouse_selection();
    redraw();
  }
  
  // Paste from clipboard at current cursor position
  void paste_from_clipboard() {
    Fl::paste(*this, 1);
  }

  void handle_paste(const char* text) {
    if (!text) return;
    
    // Debug: Print what we're trying to paste
    std::cout << "Pasting: '" << text << "' (length: " << strlen(text) << ")" << std::endl;
    
    for (const char* p = text; *p; p++) {
      if (*p != '\r' && *p != '\n') {
	lnHandleCharacter(&l_state, *p);
      }
    }
    redraw();
    
  }

  // Method to update the command list from your backend
  void update_command_list(const std::vector<std::string>& commands) {
    available_commands = commands;
  }
  // Static completion callback function (required by linenoise)
  static void completion_callback(const char *buf, linenoiseCompletions *lc);
};
