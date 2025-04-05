#include "linenoise-fltk.h"
#include <iostream>
#include <string>
#include <cstdlib>


typedef int (*PROCESS_CB)(char *str, void *clienData);

class Fl_Console : public Fl_Terminal {
private:
  int lastkey;
  struct linenoiseState l_state;
  char *buf;
  int buflen;
  std::string prompt;
  PROCESS_CB process_cb;
  void *process_cb_data;
  
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
    linenoiseHistoryLoad(&l_state, "history.txt"); /* Load the history at startup */
    return 0;
  }

  // Copy selected text or current line to clipboard
  void copy_to_clipboard() {
    if (selection_text_len() > 0) {
      // Get the selected text from the terminal
      const char *selected_text = selection_text();
      Fl::copy(selected_text, selection_text_len(), 1);
      free((void *) selected_text);
      clear_mouse_selection();
      redraw();
    }
  }

  // Paste from clipboard at current cursor position
  void paste_from_clipboard() {
    Fl::paste(*this, 1);
  }

  // Handle paste event
  void handle_paste(const char* text) {
    if (!text) return;
    
    // Insert each character from the pasted text
    for (const char* p = text; *p; p++) {
      if (*p != '\r' && *p != '\n') { // Skip newlines in pasted text
        lnHandleCharacter(&l_state, *p);
      }
    }
    redraw();
  }
  
};

Fl_Console::Fl_Console(int X,int Y,int W,int H, const char *L):
  Fl_Terminal(X,Y,W,H,L) {
  lastkey = -1;
  buflen = 4096;
  buf = new char[buflen];
  prompt = std::string("dlsh> ");
  process_cb = nullptr;
  process_cb_data = nullptr;
}

Fl_Console::~Fl_Console(void) {
  delete[] buf;
}

int Fl_Console::handle(int e) {
  
  switch (e) {
  case FL_PASTE:
    handle_paste(Fl::event_text());
    return 1;
    
  case FL_KEYDOWN:
    {
      const char *keybuf = Fl::event_text();
      lastkey = keybuf[0];
      //      std::cout << "keydown: " << Fl::event_key() << " alt_status (" << Fl::event_alt() << ")" << std::endl;

      if (Fl::event_alt()) return 0;
      
      switch (Fl::event_key()) {
      case FL_Meta_L:
      case FL_Meta_R:
      case FL_Alt_L:
      case FL_Alt_R:
      case FL_Shift_L:
      case FL_Shift_R:
      case FL_Control_L:
      case FL_Control_R:
      case FL_Caps_Lock:
        return 0;

      case 'c':
      case 'C':
        if (Fl::event_state(FL_COMMAND)) {
          copy_to_clipboard();
          return 1;
        }
        lnHandleCharacter(&l_state, (char) lastkey);
        redraw();
        return 1;

      case 'v':
      case 'V':
        if (Fl::event_state(FL_COMMAND)) {
          paste_from_clipboard();
          return 1;
        }
        lnHandleCharacter(&l_state, (char) lastkey);
        redraw();
        return 1;
        
      case FL_Enter:
        {
          int final_len = lnHandleCharacter(&l_state, (char) lastkey);
          append("\n");
          if (l_state.len) {
            linenoiseHistoryAdd(&l_state, buf); /* Add to the history. */
            linenoiseHistorySave(&l_state, "history.txt"); /* Save the history on disk. */
            do_callback(buf);
          }
          lnInitState(&l_state, buf, buflen, prompt.c_str());
          return 1;
        }
        break;
      case FL_Left:
        {
          lnHandleCharacter(&l_state, (char) CTRL_B);
          redraw();
          return 1;
        }
      case FL_Right:
        {
          lnHandleCharacter(&l_state, (char) CTRL_F);
          redraw();
          return 1;
        }
      case FL_Up:
        {
          lnHandleCharacter(&l_state, (char) CTRL_P);
          redraw();
          return 1;
        }
      case FL_Down:
        {
          lnHandleCharacter(&l_state, (char) CTRL_N);
          redraw();
          return 1;
        }
      default:
        lnHandleCharacter(&l_state, (char) lastkey);
        redraw();
        return 1;
      }
    }
  }
  return Fl_Terminal::handle(e);
}


